/* Copyright (C) 2014-2017 by Jacob Alexander
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file.  If not, see <http://www.gnu.org/licenses/>.
 */

// ----- Includes -----

// Compiler Includes
#include <Lib/MacroLib.h>

// Project Includes
#include <cli.h>
#include <led.h>
#include <print.h>
#include <scan_loop.h>

// Keymaps
#include "usb_hid.h"
#include <generatedKeymap.h> // Generated using kll at compile time, in build directory

// Connect Includes
#if defined(ConnectEnabled_define)
#include <connect_scan.h>
#endif

// PixelMap Includes
#if defined(Pixel_MapEnabled_define)
#include <pixel.h>
#endif

// Local Includes
#include "trigger.h"
#include "result.h"
#include "macro.h"



// ----- Function Declarations -----

void cliFunc_capList   ( char* args );
void cliFunc_capSelect ( char* args );
void cliFunc_keyHold   ( char* args );
void cliFunc_keyPress  ( char* args );
void cliFunc_keyRelease( char* args );
void cliFunc_layerDebug( char* args );
void cliFunc_layerList ( char* args );
void cliFunc_layerState( char* args );
void cliFunc_macroDebug( char* args );
void cliFunc_macroList ( char* args );
void cliFunc_macroProc ( char* args );
void cliFunc_macroShow ( char* args );
void cliFunc_macroStep ( char* args );
void cliFunc_posList   ( char* args );



// ----- Variables -----

// Macro Module command dictionary
CLIDict_Entry( capList,     "Prints an indexed list of all non USB keycode capabilities." );
CLIDict_Entry( capSelect,   "Triggers the specified capabilities. First two args are state and stateType." NL "\t\t\033[35mK11\033[0m Keyboard Capability 0x0B" );
CLIDict_Entry( keyHold,     "Send key-hold events to the macro module. Duplicates have undefined behaviour." NL "\t\t\033[35mS10\033[0m Scancode 0x0A" );
CLIDict_Entry( keyPress,    "Send key-press events to the macro module. Duplicates have undefined behaviour." NL "\t\t\033[35mS10\033[0m Scancode 0x0A" );
CLIDict_Entry( keyRelease,  "Send key-release event to macro module. Duplicates have undefined behaviour." NL "\t\t\033[35mS10\033[0m Scancode 0x0A" );
CLIDict_Entry( layerDebug,  "Layer debug mode. Shows layer stack and any changes." );
CLIDict_Entry( layerList,   "List available layers." );
CLIDict_Entry( layerState,  "Modify specified indexed layer state <layer> <state byte>." NL "\t\t\033[35mL2\033[0m Indexed Layer 0x02" NL "\t\t0 Off, 1 Shift, 2 Latch, 4 Lock States" );
CLIDict_Entry( macroDebug,  "Disables/Enables sending USB keycodes to the Output Module and prints U/K codes." );
CLIDict_Entry( macroList,   "List the defined trigger and result macros." );
CLIDict_Entry( macroProc,   "Pause/Resume macro processing." );
CLIDict_Entry( macroShow,   "Show the macro corresponding to the given index." NL "\t\t\033[35mT16\033[0m Indexed Trigger Macro 0x10, \033[35mR12\033[0m Indexed Result Macro 0x0C" );
CLIDict_Entry( macroStep,   "Do N macro processing steps. Defaults to 1." );
CLIDict_Entry( posList,     "List physical key positions by ScanCode." );

CLIDict_Def( macroCLIDict, "Macro Module Commands" ) = {
	CLIDict_Item( capList ),
	CLIDict_Item( capSelect ),
	CLIDict_Item( keyHold ),
	CLIDict_Item( keyPress ),
	CLIDict_Item( keyRelease ),
	CLIDict_Item( layerDebug ),
	CLIDict_Item( layerList ),
	CLIDict_Item( layerState ),
	CLIDict_Item( macroDebug ),
	CLIDict_Item( macroList ),
	CLIDict_Item( macroProc ),
	CLIDict_Item( macroShow ),
	CLIDict_Item( macroStep ),
	CLIDict_Item( posList ),
	{ 0, 0, 0 } // Null entry for dictionary end
};


// Layer debug flag - If set, displays any changes to layers and the full layer stack on change
uint8_t layerDebugMode = 0;

// Macro debug flag - If set, clears the USB Buffers after signalling processing completion
uint8_t macroDebugMode = 0;

// Macro pause flag - If set, the macro module pauses processing, unless unset, or the step counter is non-zero
uint8_t macroPauseMode = 0;

// Macro step counter - If non-zero, the step counter counts down every time the macro module does one processing loop
uint16_t macroStepCounter = 0;


// Key Trigger List Buffer and Layer Cache
// The layer cache is set on press only, hold and release events refer to the value set on press
TriggerGuide macroTriggerListBuffer[ MaxScanCode ];
var_uint_t macroTriggerListBufferSize = 0;
var_uint_t macroTriggerListLayerCache[ MaxScanCode ];

// Layer Index Stack
//  * When modifying layer state and the state is non-0x0, the stack must be adjusted
index_uint_t macroLayerIndexStack[ LayerNum + 1 ] = { 0 };
index_uint_t macroLayerIndexStackSize = 0;

// TODO REMOVE when dependency no longer exists
extern ResultsPending macroResultMacroPendingList;
extern index_uint_t macroTriggerMacroPendingList[];
extern index_uint_t macroTriggerMacroPendingListSize;

// Interconnect ScanCode Cache
#if defined(ConnectEnabled_define) || defined(PressReleaseCache_define)
// TODO This can be shrunk by the size of the max node 0 ScanCode
TriggerGuide macroInterconnectCache[ MaxScanCode ];
uint8_t macroInterconnectCacheSize = 0;
#endif



// ----- Capabilities -----

#if defined(Pixel_MapEnabled_define) && defined(animation_test_layout_define)
uint8_t Pixel_addDefaultAnimation( uint32_t index );
#endif

// Sets the given layer with the specified layerState
void Macro_layerState( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint16_t layer, uint8_t layerState )
{
	// Ignore if layer does not exist or trying to manipulate layer 0/Default layer
	if ( layer >= LayerNum || layer == 0 )
		return;

#if defined(Pixel_MapEnabled_define) && defined(animation_test_layout_define)
	// TODO TODO TODO TODO
	// TODO (HaaTa) Add as an event
	Pixel_addDefaultAnimation( Animation__lock_event );
#endif

	// Is layer in the LayerIndexStack?
	uint8_t inLayerIndexStack = 0;
	uint16_t stackItem = 0;
	while ( stackItem < macroLayerIndexStackSize )
	{
		// Flag if layer is already in the LayerIndexStack
		if ( macroLayerIndexStack[ stackItem ] == layer )
		{
			inLayerIndexStack = 1;
			break;
		}

		// Increment to next item
		stackItem++;
	}

	// Toggle Layer State Byte
	if ( LayerState[ layer ] & layerState )
	{
		// Unset
		LayerState[ layer ] &= ~layerState;
	}
	else
	{
		// Set
		LayerState[ layer ] |= layerState;
	}

	// If the layer was not in the LayerIndexStack add it
	if ( !inLayerIndexStack )
	{
		macroLayerIndexStack[ macroLayerIndexStackSize++ ] = layer;
	}

	// If the layer is in the LayerIndexStack and the state is 0x00, remove
	if ( LayerState[ layer ] == 0x00 && inLayerIndexStack )
	{
		// Remove the layer from the LayerIndexStack
		// Using the already positioned stackItem variable from the loop above
		while ( stackItem < macroLayerIndexStackSize )
		{
			macroLayerIndexStack[ stackItem ] = macroLayerIndexStack[ stackItem + 1 ];
			stackItem++;
		}

		// Reduce LayerIndexStack size
		macroLayerIndexStackSize--;
	}

	// Layer Debug Mode
	if ( layerDebugMode )
	{
		dbug_msg("Layer ");

		// Iterate over each of the layers displaying the state as a hex value
		for ( index_uint_t index = 0; index < LayerNum; index++ )
		{
			printHex_op( LayerState[ index ], 0 );
		}

		// Always show the default layer (it's always 0)
		print(" 0");

		// Iterate over the layer stack starting from the bottom of the stack
		for ( index_uint_t index = macroLayerIndexStackSize; index > 0; index-- )
		{
			print(":");
			printHex_op( macroLayerIndexStack[ index - 1 ], 0 );
		}

		print( NL );
	}
}

// Modifies the specified Layer control byte
// Argument #1: Layer Index -> uint16_t
// Argument #2: Layer State -> uint8_t
void Macro_layerState_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Macro_layerState(layerIndex,layerState)");
		return;
	}

	// Only use capability on press or release
	// TODO Analog
	// XXX This may cause issues, might be better to implement state table here to decide -HaaTa
	if ( stateType == 0x00 && state == 0x02 ) // Hold condition
		return;

	// Get layer index from arguments
	// Cast pointer to uint8_t to uint16_t then access that memory location
	uint16_t layer = *(uint16_t*)(&args[0]);

	// Get layer toggle byte
	uint8_t layerState = args[ sizeof(uint16_t) ];

	Macro_layerState( trigger, state, stateType, layer, layerState );
}


// Latches given layer
// Argument #1: Layer Index -> uint16_t
void Macro_layerLatch_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Macro_layerLatch(layerIndex)");
		return;
	}

	// Only use capability on press
	// TODO Analog
	if ( stateType == 0x00 && state != 0x03 ) // Only on release
		return;

	// Get layer index from arguments
	// Cast pointer to uint8_t to uint16_t then access that memory location
	uint16_t layer = *(uint16_t*)(&args[0]);

	Macro_layerState( trigger, state, stateType, layer, 0x02 );
}


// Locks given layer
// Argument #1: Layer Index -> uint16_t
void Macro_layerLock_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Macro_layerLock(layerIndex)");
		return;
	}

	// Only use capability on press
	// TODO Analog
	// XXX Could also be on release, but that's sorta dumb -HaaTa
	if ( stateType == 0x00 && state != 0x01 ) // All normal key conditions except press
		return;

	// Get layer index from arguments
	// Cast pointer to uint8_t to uint16_t then access that memory location
	uint16_t layer = *(uint16_t*)(&args[0]);

	Macro_layerState( trigger, state, stateType, layer, 0x04 );
}


// Shifts given layer
// Argument #1: Layer Index -> uint16_t
void Macro_layerShift_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Macro_layerShift(layerIndex)");
		return;
	}

	// Only use capability on press or release
	// TODO Analog
	if ( stateType == 0x00 && ( state == 0x00 || state == 0x02 ) ) // Only pass press or release conditions
		return;

	// Get layer index from arguments
	// Cast pointer to uint8_t to uint16_t then access that memory location
	uint16_t layer = *(uint16_t*)(&args[0]);

	// Only set the layer if it is disabled
	if ( LayerState[ layer ] != 0x00 && state == 0x01 )
		return;

	// Only unset the layer if it is enabled
	if ( LayerState[ layer ] == 0x00 && state == 0x03 )
		return;

	Macro_layerState( trigger, state, stateType, layer, 0x01 );
}


// Rotate layer to next/previous
// Uses state variable to keep track of the current layer position
// Layers are still evaluated using the layer stack
uint16_t Macro_rotationLayer;
void Macro_layerRotate_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Macro_layerRotate(previous)");
		return;
	}

	// Only use capability on press
	// TODO Analog
	// XXX Could also be on release, but that's sorta dumb -HaaTa
	if ( stateType == 0x00 && state != 0x01 ) // All normal key conditions except press
		return;

	// Unset previous rotation layer if not 0
	if ( Macro_rotationLayer != 0 )
	{
		Macro_layerState( trigger, state, stateType, Macro_rotationLayer, 0x04 );
	}

	// Get direction of rotation, 0, next, non-zero previous
	uint8_t direction = *args;

	// Next
	if ( !direction )
	{
		Macro_rotationLayer++;

		// Invalid layer
		if ( Macro_rotationLayer >= LayerNum )
			Macro_rotationLayer = 0;
	}
	// Previous
	else
	{
		Macro_rotationLayer--;

		// Layer wrap
		if ( Macro_rotationLayer >= LayerNum )
			Macro_rotationLayer = LayerNum - 1;
	}

	// Toggle the computed layer rotation
	Macro_layerState( trigger, state, stateType, Macro_rotationLayer, 0x04 );
}



// ----- Functions -----

// Looks up the trigger list for the given scan code (from the active layer)
// NOTE: Calling function must handle the NULL pointer case
nat_ptr_t *Macro_layerLookup( TriggerGuide *guide, uint8_t latch_expire )
{
	uint8_t scanCode = guide->scanCode;

	// TODO Analog
	// If a normal key, and not pressed, do a layer cache lookup
	if ( guide->type == 0x00 && guide->state != 0x01 )
	{
		// Cached layer
		var_uint_t cachedLayer = macroTriggerListLayerCache[ scanCode ];

		// Lookup map, then layer
		nat_ptr_t **map = (nat_ptr_t**)LayerIndex[ cachedLayer ].triggerMap;
		const Layer *layer = &LayerIndex[ cachedLayer ];

		// Cache trigger list before attempting to expire latch
		nat_ptr_t *trigger_list = map[ scanCode - layer->first ];

		// Check if latch has been pressed for this layer
		uint8_t latch = LayerState[ cachedLayer ] & 0x02;
		if ( latch && latch_expire )
		{
			Macro_layerState( 0, 0, 0, cachedLayer, 0x02 );
#if defined(ConnectEnabled_define) && defined(LCDEnabled_define)
			// Evaluate the layerStack capability if available (LCD + Interconnect)
			extern void LCD_layerStack_capability(
				TriggerMacro *trigger,
				uint8_t state,
				uint8_t stateType,
				uint8_t *args
			);
			LCD_layerStack_capability( 0, 0, 0, 0 );
#endif
		}

		return trigger_list;
	}

	// If no trigger macro is defined at the given layer, fallthrough to the next layer
	for ( uint16_t layerIndex = macroLayerIndexStackSize; layerIndex != 0xFFFF; layerIndex-- )
	{
		// Lookup Layer
		const Layer *layer = &LayerIndex[ macroLayerIndexStack[ layerIndex ] ];

		// Check if latch has been pressed for this layer
		// XXX Regardless of whether a key is found, the latch is removed on first lookup
		uint8_t latch = LayerState[ macroLayerIndexStack[ layerIndex ] ] & 0x02;
		if ( latch && latch_expire )
		{
			Macro_layerState( 0, 0, 0, macroLayerIndexStack[ layerIndex ], 0x02 );
		}

		// Only use layer, if state is valid
		// XOR each of the state bits
		// If only two are enabled, do not use this state
		if ( (LayerState[ macroLayerIndexStack[ layerIndex ] ] & 0x01) ^ (latch>>1) ^ ((LayerState[ macroLayerIndexStack[ layerIndex ] ] & 0x04)>>2) )
		{
			// Lookup layer
			nat_ptr_t **map = (nat_ptr_t**)layer->triggerMap;

			// Determine if layer has key defined
			// Make sure scanCode is between layer first and last scancodes
			if ( map != 0
				&& scanCode <= layer->last
				&& scanCode >= layer->first
				&& *map[ scanCode - layer->first ] != 0 )
			{
				// Set the layer cache
				macroTriggerListLayerCache[ scanCode ] = macroLayerIndexStack[ layerIndex ];

				return map[ scanCode - layer->first ];
			}
		}
	}

	// Do lookup on default layer
	nat_ptr_t **map = (nat_ptr_t**)LayerIndex[0].triggerMap;

	// Lookup default layer
	const Layer *layer = &LayerIndex[0];

	// Make sure scanCode is between layer first and last scancodes
	if ( map != 0
		&& scanCode <= layer->last
		&& scanCode >= layer->first
		&& *map[ scanCode - layer->first ] != 0 )
	{
		// Set the layer cache to default map
		macroTriggerListLayerCache[ scanCode ] = 0;

		return map[ scanCode - layer->first ];
	}

	// Otherwise no defined Trigger Macro
	erro_msg("Scan Code has no defined Trigger Macro: ");
	printHex( scanCode );
	print( NL );
	return 0;
}


// Add an interconnect ScanCode
// These are handled differently (less information is sent, hold/off states must be assumed)
// Returns 1 if added, 0 if the ScanCode is already in the buffer
// Returns 2 if there's an error
#if defined(ConnectEnabled_define) || defined(PressReleaseCache_define)
uint8_t Macro_pressReleaseAdd( void *trigger_ptr )
{
	TriggerGuide *trigger = (TriggerGuide*)trigger_ptr;

	// Error checking
	uint8_t error = 0;
	switch ( trigger->type )
	{
	case 0x00: // Normal key
		switch ( trigger->state )
		{
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
			break;
		default:
			erro_msg("Invalid key state - ");
			error = 1;
			break;
		}
		break;

	// Invalid TriggerGuide type
	default:
		erro_msg("Invalid type - ");
		error = 1;
		break;
	}

	// Check if ScanCode is out of range
	if ( trigger->scanCode > MaxScanCode )
	{
		warn_msg("ScanCode is out of range/not defined - ");
		error = 1;
	}

	// Display TriggerGuide
	if ( error )
	{
		printHex( trigger->type );
		print(" ");
		printHex( trigger->state );
		print(" ");
		printHex( trigger->scanCode );
		print( NL );
		return 2;
	}

	// Add trigger to the Interconnect Cache
	// During each processing loop, a scancode may be re-added depending on it's state
	for ( var_uint_t c = 0; c < macroInterconnectCacheSize; c++ )
	{
		// Check if the same ScanCode
		if ( macroInterconnectCache[ c ].scanCode == trigger->scanCode )
		{
			// Update the state
			macroInterconnectCache[ c ].state = trigger->state;
			return 0;
		}
	}

	// If not in the list, add it
	macroInterconnectCache[ macroInterconnectCacheSize++ ] = *trigger;

	return 1;
}
#endif


// Update the scancode key state
// States:
//   * 0x00 - Off
//   * 0x01 - Pressed
//   * 0x02 - Held
//   * 0x03 - Released
//   * 0x04 - Unpressed (this is currently ignored)
inline void Macro_keyState( uint8_t scanCode, uint8_t state )
{
#if defined(ConnectEnabled_define)
	// Only compile in if a Connect node module is available
	if ( !Connect_master )
	{
		// ScanCodes are only added if there was a state change (on/off)
		switch ( state )
		{
		case 0x00: // Off
		case 0x02: // Held
			return;
		}
	}
#endif

	// Only add to macro trigger list if one of three states
	switch ( state )
	{
	case 0x01: // Pressed
	case 0x02: // Held
	case 0x03: // Released
		// Check if ScanCode is out of range
		if ( scanCode > MaxScanCode )
		{
			warn_msg("ScanCode is out of range/not defined: ");
			printHex( scanCode );
			print( NL );
			return;
		}

		macroTriggerListBuffer[ macroTriggerListBufferSize ].scanCode = scanCode;
		macroTriggerListBuffer[ macroTriggerListBufferSize ].state    = state;
		macroTriggerListBuffer[ macroTriggerListBufferSize ].type     = 0x00; // Normal key
		macroTriggerListBufferSize++;
		break;
	}
}


// Update the scancode analog state
// States:
//   * 0x00      - Off
//   * 0x01      - Released
//   * 0x02-0xFF - Analog value (low to high)
inline void Macro_analogState( uint8_t scanCode, uint8_t state )
{
	// Only add to macro trigger list if non-off
	// TODO Handle change for interconnect
	if ( state != 0x00 )
	{
		// Check if ScanCode is out of range
		if ( scanCode > MaxScanCode )
		{
			warn_msg("ScanCode is out of range/not defined: ");
			printHex( scanCode );
			print( NL );
			return;
		}

		macroTriggerListBuffer[ macroTriggerListBufferSize ].scanCode = scanCode;
		macroTriggerListBuffer[ macroTriggerListBufferSize ].state    = state;
		macroTriggerListBuffer[ macroTriggerListBufferSize ].type     = 0x02; // Analog key
		macroTriggerListBufferSize++;
	}
}


// Update led state
// States:
//   * 0x00 - Off
//   * 0x01 - On
inline void Macro_ledState( uint8_t ledCode, uint8_t state )
{
	// Only add to macro trigger list if non-off
	// TODO Handle change for interconnect
	if ( state != 0x00 )
	{
		// Check if LedCode is out of range
		// TODO

		macroTriggerListBuffer[ macroTriggerListBufferSize ].scanCode = ledCode;
		macroTriggerListBuffer[ macroTriggerListBufferSize ].state    = state;
		macroTriggerListBuffer[ macroTriggerListBufferSize ].type     = 0x01; // LED key
		macroTriggerListBufferSize++;
	}
}


// Append result macro to pending list, checking for duplicates
// Do nothing if duplicate
void Macro_appendResultMacroToPendingList( const TriggerMacro *triggerMacro )
{
	// Lookup result macro index
	var_uint_t resultMacroIndex = triggerMacro->result;

	// Iterate through result macro pending list, making sure this macro hasn't been added yet
	for ( var_uint_t macro = 0; macro < macroResultMacroPendingList.size; macro++ )
	{
		// If duplicate found, do nothing
		if ( macroResultMacroPendingList.data[ macro ].index == resultMacroIndex )
			return;
	}

	// No duplicates found, add to pending list
	macroResultMacroPendingList.data[ macroResultMacroPendingList.size ].trigger = (TriggerMacro*)triggerMacro;
	macroResultMacroPendingList.data[ macroResultMacroPendingList.size++ ].index = resultMacroIndex;

	// Lookup scanCode of the last key in the last combo
	var_uint_t pos = 0;
	for ( uint8_t comboLength = triggerMacro->guide[0]; comboLength > 0; )
	{
		pos += TriggerGuideSize * comboLength + 1;
		comboLength = triggerMacro->guide[ pos ];
	}

	uint8_t scanCode = ((TriggerGuide*)&triggerMacro->guide[ pos - TriggerGuideSize ])->scanCode;

	// Lookup scanCode in buffer list for the current state and stateType
	for ( var_uint_t keyIndex = 0; keyIndex < macroTriggerListBufferSize; keyIndex++ )
	{
		if ( macroTriggerListBuffer[ keyIndex ].scanCode == scanCode )
		{
			ResultMacroRecordList[ resultMacroIndex ].state     = macroTriggerListBuffer[ keyIndex ].state;
			ResultMacroRecordList[ resultMacroIndex ].stateType = macroTriggerListBuffer[ keyIndex ].type;
		}
	}

	// Reset the macro position
	ResultMacroRecordList[ resultMacroIndex ].pos = 0;
}


// Macro Procesing Loop
// Called once per USB buffer send
inline void Macro_process()
{
#if defined(ConnectEnabled_define)
	// Only compile in if a Connect node module is available
	// If this is a interconnect slave node, send all scancodes to master node
	if ( !Connect_master )
	{
		if ( macroTriggerListBufferSize > 0 )
		{
			Connect_send_ScanCode( Connect_id, macroTriggerListBuffer, macroTriggerListBufferSize );
			macroTriggerListBufferSize = 0;
		}
		return;
	}
#endif

#if defined(ConnectEnabled_define) || defined(PressReleaseCache_define)
#if defined(ConnectEnabled_define)
	// Check if there are any ScanCodes in the interconnect cache to process
	if ( Connect_master && macroInterconnectCacheSize > 0 )
#endif
	{
		// Iterate over all the cache ScanCodes
		uint8_t currentInterconnectCacheSize = macroInterconnectCacheSize;
		macroInterconnectCacheSize = 0;
		for ( uint8_t c = 0; c < currentInterconnectCacheSize; c++ )
		{
			// Add to the trigger list
			macroTriggerListBuffer[ macroTriggerListBufferSize++ ] = macroInterconnectCache[ c ];

			// TODO Handle other TriggerGuide types (e.g. analog)
			switch ( macroInterconnectCache[ c ].type )
			{
			// Normal (Press/Hold/Release)
			case 0x00:
				// Decide what to do based on the current state
				switch ( macroInterconnectCache[ c ].state )
				{
				// Re-add to interconnect cache in hold state
				case 0x01: // Press
				//case 0x02: // Hold // XXX Why does this not work? -HaaTa
					macroInterconnectCache[ c ].state = 0x02;
					macroInterconnectCache[ macroInterconnectCacheSize++ ] = macroInterconnectCache[ c ];
					break;
				case 0x03: // Remove
					break;
				// Otherwise, do not re-add
				}
			}
		}
	}
#endif

	// If the pause flag is set, only process if the step counter is non-zero
	if ( macroPauseMode )
	{
		if ( macroStepCounter == 0 )
			return;

		// Proceed, decrementing the step counter
		macroStepCounter--;
		dbug_print("Macro Step");
	}

	// Process Trigger Macros
	Trigger_process();


	// Process result macros
	Result_process();

	// Signal buffer that we've used it
	Scan_finishedWithMacro( macroTriggerListBufferSize );

	// Reset TriggerList buffer
	macroTriggerListBufferSize = 0;

	// If Macro debug mode is set, clear the USB Buffer
	if ( macroDebugMode )
	{
		USBKeys_Modifiers = 0;
		USBKeys_Sent = 0;
	}
}


inline void Macro_setup()
{
	// Register Macro CLI dictionary
	CLI_registerDictionary( macroCLIDict, macroCLIDictName );

	// Disable Macro debug mode
	macroDebugMode = 0;

	// Disable Macro pause flag
	macroPauseMode = 0;

	// Set Macro step counter to zero
	macroStepCounter = 0;

	// Make sure macro trigger buffer is empty
	macroTriggerListBufferSize = 0;

	// Set the current rotated layer to 0
	Macro_rotationLayer = 0;

	// Setup Triggers
	Trigger_setup();

	// Setup Results
	Result_setup();
}


// ----- CLI Command Functions -----

void cliFunc_capList( char* args )
{
	print( NL );
	info_msg("Capabilities List ");
	printHex( CapabilitiesNum );

	// Iterate through all of the capabilities and display them
	for ( var_uint_t cap = 0; cap < CapabilitiesNum; cap++ )
	{
		print( NL "\t" );
		printHex( cap );
		print(" - ");

		// Display/Lookup Capability Name (utilize debug mode of capability)
		void (*capability)(TriggerMacro*, uint8_t, uint8_t, uint8_t*) = \
			(void(*)(TriggerMacro*, uint8_t, uint8_t, uint8_t*))(CapabilitiesList[ cap ].func);
		capability( 0, 0xFF, 0xFF, 0 );
	}
}

void cliFunc_capSelect( char* args )
{
	// Parse code from argument
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Total number of args to scan (must do a lookup if a keyboard capability is selected)
	var_uint_t totalArgs = 2; // Always at least two args
	var_uint_t cap = 0;

	// Arguments used for keyboard capability function
	var_uint_t argSetCount = 0;
	uint8_t *argSet = (uint8_t*)args;

	// Process all args
	for ( var_uint_t c = 0; argSetCount < totalArgs; c++ )
	{
		curArgs = arg2Ptr;
		CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

		// Stop processing args if no more are found
		// Extra arguments are ignored
		if ( *arg1Ptr == '\0' )
			break;

		// For the first argument, choose the capability
		if ( c == 0 ) switch ( arg1Ptr[0] )
		{
		// Keyboard Capability
		case 'K':
			// Determine capability index
			cap = numToInt( &arg1Ptr[1] );

			// Lookup the number of args
			totalArgs += CapabilitiesList[ cap ].argCount;
			continue;
		}

		// Because allocating memory isn't doable, and the argument count is arbitrary
		// The argument pointer is repurposed as the argument list (much smaller anyways)
		argSet[ argSetCount++ ] = (uint8_t)numToInt( arg1Ptr );

		// Once all the arguments are prepared, call the keyboard capability function
		if ( argSetCount == totalArgs )
		{
			// Indicate that the capability was called
			print( NL );
			info_msg("K");
			printInt8( cap );
			print(" - ");
			printHex( argSet[0] );
			print(" - ");
			printHex( argSet[1] );
			print(" - ");
			printHex( argSet[2] );
			print( "..." NL );

			// Make sure this isn't the reload capability
			// If it is, and the remote reflash define is not set, ignore
			if ( flashModeEnabled_define == 0 ) for ( uint32_t cap = 0; cap < CapabilitiesNum; cap++ )
			{
				if ( CapabilitiesList[ cap ].func == (const void*)Output_flashMode_capability )
				{
					print( NL );
					warn_print("flashModeEnabled not set, cancelling firmware reload...");
					info_msg("Set flashModeEnabled to 1 in your kll configuration.");
					return;
				}
			}

			void (*capability)(TriggerMacro*, uint8_t, uint8_t, uint8_t*) = \
				(void(*)(TriggerMacro*, uint8_t, uint8_t, uint8_t*))(CapabilitiesList[ cap ].func);
			capability( 0, argSet[0], argSet[1], &argSet[2] );
		}
	}
}

void cliFunc_keyHold( char* args )
{
	// Parse codes from arguments
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Process all args
	for ( ;; )
	{
		curArgs = arg2Ptr;
		CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

		// Stop processing args if no more are found
		if ( *arg1Ptr == '\0' )
			break;

		// Ignore non-Scancode numbers
		switch ( arg1Ptr[0] )
		{
		// Scancode
		case 'S':
			Macro_keyState( (uint8_t)numToInt( &arg1Ptr[1] ), 0x02 ); // Hold scancode
			break;
		}
	}
}

void cliFunc_keyPress( char* args )
{
	// Parse codes from arguments
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Process all args
	for ( ;; )
	{
		curArgs = arg2Ptr;
		CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

		// Stop processing args if no more are found
		if ( *arg1Ptr == '\0' )
			break;

		// Ignore non-Scancode numbers
		switch ( arg1Ptr[0] )
		{
		// Scancode
		case 'S':
			Macro_keyState( (uint8_t)numToInt( &arg1Ptr[1] ), 0x01 ); // Press scancode
			break;
		}
	}
}

void cliFunc_keyRelease( char* args )
{
	// Parse codes from arguments
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Process all args
	for ( ;; )
	{
		curArgs = arg2Ptr;
		CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

		// Stop processing args if no more are found
		if ( *arg1Ptr == '\0' )
			break;

		// Ignore non-Scancode numbers
		switch ( arg1Ptr[0] )
		{
		// Scancode
		case 'S':
			Macro_keyState( (uint8_t)numToInt( &arg1Ptr[1] ), 0x03 ); // Release scancode
			break;
		}
	}
}

void cliFunc_layerDebug( char *args )
{
	// Toggle layer debug mode
	layerDebugMode = layerDebugMode ? 0 : 1;

	print( NL );
	info_msg("Layer Debug Mode: ");
	printInt8( layerDebugMode );
}

void cliFunc_layerList( char* args )
{
	print( NL );
	info_msg("Layer List");

	// Iterate through all of the layers and display them
	for ( uint16_t layer = 0; layer < LayerNum; layer++ )
	{
		print( NL "\t" );
		printHex( layer );
		print(" - ");

		// Display layer name
		dPrint( (char*)LayerIndex[ layer ].name );

		// Default map
		if ( layer == 0 )
			print(" \033[1m(default)\033[0m");

		// Layer State
		print( NL "\t\t Layer State: " );
		printHex( LayerState[ layer ] );

		// First -> Last Indices
		print(" First -> Last Indices: ");
		printHex( LayerIndex[ layer ].first );
		print(" -> ");
		printHex( LayerIndex[ layer ].last );
	}
}

void cliFunc_layerState( char* args )
{
	// Parse codes from arguments
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	uint8_t arg1 = 0;
	uint8_t arg2 = 0;

	// Process first two args
	for ( uint8_t c = 0; c < 2; c++ )
	{
		curArgs = arg2Ptr;
		CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

		// Stop processing args if no more are found
		if ( *arg1Ptr == '\0' )
			break;

		switch ( c )
		{
		// First argument (e.g. L1)
		case 0:
			if ( arg1Ptr[0] != 'L' )
				return;

			arg1 = (uint8_t)numToInt( &arg1Ptr[1] );
			break;
		// Second argument (e.g. 4)
		case 1:
			arg2 = (uint8_t)numToInt( arg1Ptr );

			// Display operation (to indicate that it worked)
			print( NL );
			info_msg("Setting Layer L");
			printInt8( arg1 );
			print(" to - ");
			printHex( arg2 );

			// Set the layer state
			LayerState[ arg1 ] = arg2;
			break;
		}
	}
}

void cliFunc_macroDebug( char* args )
{
	// Toggle macro debug mode
	macroDebugMode = macroDebugMode ? 0 : 1;

	print( NL );
	info_msg("Macro Debug Mode: ");
	printInt8( macroDebugMode );
}

void cliFunc_macroList( char* args )
{
	// Show pending key events
	print( NL );
	info_msg("Pending Key Events: ");
	printInt16( (uint16_t)macroTriggerListBufferSize );
	print(" : ");
	for ( var_uint_t key = 0; key < macroTriggerListBufferSize; key++ )
	{
		printHex( macroTriggerListBuffer[ key ].scanCode );
		print(" ");
	}

	// Show pending trigger macros
	print( NL );
	info_msg("Pending Trigger Macros: ");
	printInt16( (uint16_t)macroTriggerMacroPendingListSize );
	print(" : ");
	for ( var_uint_t macro = 0; macro < macroTriggerMacroPendingListSize; macro++ )
	{
		printHex( macroTriggerMacroPendingList[ macro ] );
		print(" ");
	}

	// Show pending result macros
	print( NL );
	info_msg("Pending Result Macros: ");
	printInt16( (uint16_t)macroResultMacroPendingList.size );
	print(" : ");
	for ( var_uint_t macro = 0; macro < macroResultMacroPendingList.size; macro++ )
	{
		printHex( macroResultMacroPendingList.data[ macro ].index );
		print(" ");
	}

	// Show available trigger macro indices
	print( NL );
	info_msg("Trigger Macros Range: T0 -> T");
	printInt16( (uint16_t)TriggerMacroNum - 1 ); // Hopefully large enough :P (can't assume 32-bit)

	// Show available result macro indices
	print( NL );
	info_msg("Result  Macros Range: R0 -> R");
	printInt16( (uint16_t)ResultMacroNum - 1 ); // Hopefully large enough :P (can't assume 32-bit)

	// Show Trigger to Result Macro Links
	print( NL );
	info_msg("Trigger : Result Macro Pairs");
	for ( var_uint_t macro = 0; macro < TriggerMacroNum; macro++ )
	{
		print( NL );
		print("\tT");
		printInt16( (uint16_t)macro ); // Hopefully large enough :P (can't assume 32-bit)
		print(" : R");
		printInt16( (uint16_t)TriggerMacroList[ macro ].result ); // Hopefully large enough :P (can't assume 32-bit)
	}
}

void cliFunc_macroProc( char* args )
{
	// Toggle macro pause mode
	macroPauseMode = macroPauseMode ? 0 : 1;

	print( NL );
	info_msg("Macro Processing Mode: ");
	printInt8( macroPauseMode );
}

void macroDebugShowTrigger( var_uint_t index )
{
	// Only proceed if the macro exists
	if ( index >= TriggerMacroNum )
		return;

	// Trigger Macro Show
	const TriggerMacro *macro = &TriggerMacroList[ index ];
	TriggerMacroRecord *record = &TriggerMacroRecordList[ index ];

	print( NL );
	info_msg("Trigger Macro Index: ");
	printInt16( (uint16_t)index ); // Hopefully large enough :P (can't assume 32-bit)
	print( NL );

	// Read the comboLength for combo in the sequence (sequence of combos)
	var_uint_t pos = 0;
	uint8_t comboLength = macro->guide[ pos ];

	// Iterate through and interpret the guide
	while ( comboLength != 0 )
	{
		// Initial position of the combo
		var_uint_t comboPos = ++pos;

		// Iterate through the combo
		while ( pos < comboLength * TriggerGuideSize + comboPos )
		{
			// Assign TriggerGuide element (key type, state and scancode)
			TriggerGuide *guide = (TriggerGuide*)(&macro->guide[ pos ]);

			// Display guide information about trigger key
			printHex( guide->scanCode );
			print("|");
			printHex( guide->type );
			print("|");
			printHex( guide->state );

			// Increment position
			pos += TriggerGuideSize;

			// Only show combo separator if there are combos left in the sequence element
			if ( pos < comboLength * TriggerGuideSize + comboPos )
				print("+");
		}

		// Read the next comboLength
		comboLength = macro->guide[ pos ];

		// Only show sequence separator if there is another combo to process
		if ( comboLength != 0 )
			print(";");
	}

	// Display current position
	print( NL "Position: " );
	printInt16( (uint16_t)record->pos ); // Hopefully large enough :P (can't assume 32-bit)

	// Display result macro index
	print( NL "Result Macro Index: " );
	printInt16( (uint16_t)macro->result ); // Hopefully large enough :P (can't assume 32-bit)

	// Display trigger macro state
	print( NL "Trigger Macro State: " );
	switch ( record->state )
	{
	case TriggerMacro_Press:   print("Press");   break;
	case TriggerMacro_Release: print("Release"); break;
	case TriggerMacro_Waiting: print("Waiting"); break;
	}
}

void macroDebugShowResult( var_uint_t index )
{
	// Only proceed if the macro exists
	if ( index >= ResultMacroNum )
		return;

	// Trigger Macro Show
	const ResultMacro *macro = &ResultMacroList[ index ];
	ResultMacroRecord *record = &ResultMacroRecordList[ index ];

	print( NL );
	info_msg("Result Macro Index: ");
	printInt16( (uint16_t)index ); // Hopefully large enough :P (can't assume 32-bit)
	print( NL );

	// Read the comboLength for combo in the sequence (sequence of combos)
	var_uint_t pos = 0;
	uint8_t comboLength = macro->guide[ pos++ ];

	// Iterate through and interpret the guide
	while ( comboLength != 0 )
	{
		// Function Counter, used to keep track of the combos processed
		var_uint_t funcCount = 0;

		// Iterate through the combo
		while ( funcCount < comboLength )
		{
			// Assign TriggerGuide element (key type, state and scancode)
			ResultGuide *guide = (ResultGuide*)(&macro->guide[ pos ]);

			// Display Function Index
			printHex( guide->index );
			print("|");

			// Display Function Ptr Address
			printHex( (nat_ptr_t)CapabilitiesList[ guide->index ].func );
			print("|");

			// Display/Lookup Capability Name (utilize debug mode of capability)
			void (*capability)(TriggerMacro*, uint8_t, uint8_t, uint8_t*) = \
				(void(*)(TriggerMacro*, uint8_t, uint8_t, uint8_t*))(CapabilitiesList[ guide->index ].func);
			capability( 0, 0xFF, 0xFF, 0 );

			// Display Argument(s)
			print("(");
			for ( var_uint_t arg = 0; arg < CapabilitiesList[ guide->index ].argCount; arg++ )
			{
				// Arguments are only 8 bit values
				printHex( (&guide->args)[ arg ] );

				// Only show arg separator if there are args left
				if ( arg + 1 < CapabilitiesList[ guide->index ].argCount )
					print(",");
			}
			print(")");

			// Increment position
			pos += ResultGuideSize( guide );

			// Increment function count
			funcCount++;

			// Only show combo separator if there are combos left in the sequence element
			if ( funcCount < comboLength )
				print("+");
		}

		// Read the next comboLength
		comboLength = macro->guide[ pos++ ];

		// Only show sequence separator if there is another combo to process
		if ( comboLength != 0 )
			print(";");
	}

	// Display current position
	print( NL "Position: " );
	printInt16( (uint16_t)record->pos ); // Hopefully large enough :P (can't assume 32-bit)

	// Display final trigger state/type
	print( NL "Final Trigger State (State/Type): " );
	printHex( record->state );
	print("/");
	printHex( record->stateType );
}

void cliFunc_macroShow( char* args )
{
	// Parse codes from arguments
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Process all args
	for ( ;; )
	{
		curArgs = arg2Ptr;
		CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

		// Stop processing args if no more are found
		if ( *arg1Ptr == '\0' )
			break;

		// Ignore invalid codes
		switch ( arg1Ptr[0] )
		{
		// Indexed Trigger Macro
		case 'T':
			macroDebugShowTrigger( numToInt( &arg1Ptr[1] ) );
			break;
		// Indexed Result Macro
		case 'R':
			macroDebugShowResult( numToInt( &arg1Ptr[1] ) );
			break;
		}
	}
}

void cliFunc_macroStep( char* args )
{
	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	// Default to 1, if no argument given
	var_uint_t count = (var_uint_t)numToInt( arg1Ptr );

	if ( count == 0 )
		count = 1;

	// Set the macro step counter, negative int's are cast to uint
	macroStepCounter = count;
}

// Convenience Macro
#define Key_PositionPrint( key, name ) \
	printInt16( Key_Position[ key ].name.i ); \
	print("."); \
	printInt16( Key_Position[ key ].name.f )

void cliFunc_posList( char* args )
{
	print( NL );

	/* TODO Add printFloat function
	// List out physical key positions by scan code
	for ( uint8_t key = 0; key < MaxScanCode; key++ )
	{
		printInt8( key + 1 );
		print(": [");
		Key_PositionPrint( key, x );
		print(", ");
		Key_PositionPrint( key, y );
		print(", ");
		Key_PositionPrint( key, z );
		print("] r[");
		Key_PositionPrint( key, rx );
		print(", ");
		Key_PositionPrint( key, ry );
		print(", ");
		Key_PositionPrint( key, rz );
		print("]");
		print( NL );
	}
	*/
}

