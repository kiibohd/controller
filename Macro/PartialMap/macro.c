/* Copyright (C) 2014-2018 by Jacob Alexander
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
#include <latency.h>
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
void cliFunc_voteDebug ( char* args );



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
CLIDict_Entry( voteDebug,   "Show results of TriggerEvent voting." );

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
	CLIDict_Item( voteDebug ),
	{ 0, 0, 0 } // Null entry for dictionary end
};


// Layer debug flag - If set, displays any changes to layers and the full layer stack on change
uint8_t layerDebugMode;

// Macro debug flag - If set, clears the USB Buffers after signalling processing completion
// 1 - Disable USB output, show debug
// 2 - Enabled USB output, show debug
// 3 - Disable USB output
uint8_t macroDebugMode;

// Vote debug flag - If set show the result of each
uint8_t voteDebugMode;

// Trigger pending debug flag - If set show pending triggers before evaluating
uint8_t triggerPendingDebugMode;

// Macro pause flag - If set, the macro module pauses processing, unless unset, or the step counter is non-zero
uint8_t macroPauseMode;

// Macro step counter - If non-zero, the step counter counts down every time the macro module does one processing loop
uint16_t macroStepCounter;


// Latency resource
static uint8_t macroLatencyResource;


// Incoming Trigger Event Buffer
TriggerEvent macroTriggerEventBuffer[ MaxScanCode + 1 ];
var_uint_t macroTriggerEventBufferSize;
var_uint_t macroTriggerEventLayerCache[ MaxScanCode + 1 ];

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
TriggerEvent macroInterconnectCache[ MaxScanCode + 1 ];
uint8_t macroInterconnectCacheSize = 0;
#endif

// Dynamically Sized Type Widths
#if defined(_host_)
const uint8_t StateWordSize = StateWordSize_define;
const uint8_t IndexWordSize = IndexWordSize_define;
const uint8_t ScheduleStateSize = ScheduleStateSize_define;
#endif



// ----- Capabilities -----

// Sets the given layer with the specified layerState
void Macro_layerState( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint16_t layer, uint8_t layerState )
{
	// Ignore if layer does not exist or trying to manipulate layer 0/Default layer
	if ( layer >= LayerNum || layer == 0 )
		return;

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
		print("\033[1;36mL\033[0m ");

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



// ----- Debug Functions -----

// Shows a ScheduleType
void Macro_showScheduleType( ScheduleState state )
{
	// State types
	switch ( state )
	{
	case ScheduleType_P:
	//case ScheduleType_A:
		print("\033[1;33mP\033[0m");
		break;

	case ScheduleType_H:
	//case ScheduleType_On:
		print("\033[1;32mH\033[0m");
		break;

	case ScheduleType_R:
	//case ScheduleType_D:
		print("\033[1;35mR\033[0m");
		break;

	case ScheduleType_O:
	//case ScheduleType_Off:
		print("\033[1mO\033[0m");
		break;

	case ScheduleType_UP:
		print("UP");
		break;

	case ScheduleType_UR:
		print("UR");
		break;

	case ScheduleType_Done:
		print("Done");
		break;

	case ScheduleType_Repeat:
		print("Repeat");
		break;

	case ScheduleType_Debug:
		print("Debug");
		break;

	default:
		print("\033[1;31mINVALID\033[0m");
		break;
	}
}

// Shows a ScheduleParam
void Macro_showScheduleParam( ScheduleParam *param, uint8_t analog )
{
	// Analog
	if ( analog )
	{
		printInt8( param->analog );
	}
	// Everything else
	else
	{
		Macro_showScheduleType( param->state );
	}

	// Time
	print(":");
	printInt32( param->time.ms );
	print(".");
	printInt32( param->time.ticks );
}

// Shows a Schedule
void Macro_showSchedule( Schedule *schedule, uint8_t analog )
{
	// Show first element
	Macro_showScheduleParam( &schedule->params[0], analog );

	// Iterate over each additional parameter of the schedule
	for ( uint8_t c = 1; c < schedule->count; c++ )
	{
		print(",");
		Macro_showScheduleParam( &schedule->params[c], analog );
	}
}

// Shows a TriggerType
void Macro_showTriggerType( TriggerType type )
{
	// Type
	switch ( type )
	{
	// Switches
	case TriggerType_Switch1:
	case TriggerType_Switch2:
	case TriggerType_Switch3:
	case TriggerType_Switch4:
		print("Sw");
		break;

	// LEDs
	case TriggerType_LED1:
		print("LED");
		break;

	// Analog
	case TriggerType_Analog1:
	case TriggerType_Analog2:
	case TriggerType_Analog3:
	case TriggerType_Analog4:
		print("An");
		break;

	// Layer
	case TriggerType_Layer1:
	case TriggerType_Layer2:
	case TriggerType_Layer3:
	case TriggerType_Layer4:
		print("Layer");
		break;

	// Invalid
	default:
		print("INVALID");
		break;

	// Debug
	case TriggerType_Debug:
		print("Debug");
		break;
	}
}

// Shows a TriggerEvent
void Macro_showTriggerEvent( TriggerEvent *event )
{
	// Decode type
	Macro_showTriggerType( event->type );
	print(" ");

	// Show state
	Macro_showScheduleType( event->state );
	print(" ");

	// Show index number
	printInt8( event->type );
	print(":");
	printInt8( event->index );
}

// Shows a TriggerGuide
void Macro_showTriggerGuide( TriggerGuide *guide )
{
}



// ----- Functions -----

#if defined(_host_)
// Callback for host-side kll
extern int Output_callback( char* command, char* args );
const uint32_t LayerNum_host = LayerNum;
#endif

// Clears the current layer state
void Macro_clearLayers()
{
	// Clear layer stack
	macroLayerIndexStackSize = 0;

	// Clear layer states
	memset( &LayerState, 0, sizeof(LayerStateType) * LayerNum );
}

// Looks up the trigger list for the given scan code (from the active layer)
// NOTE: Calling function must handle the NULL pointer case
nat_ptr_t *Macro_layerLookup( TriggerEvent *event, uint8_t latch_expire )
{
	uint8_t index = event->index;

	// TODO Analog, LED, Layer, Animation
	// If a normal key, and not pressed, do a layer cache lookup
	if ( event->type == 0x00 && event->state != 0x01 )
	{
		// Cached layer
		var_uint_t cachedLayer = macroTriggerEventLayerCache[ index ];

		// Lookup map, then layer
		nat_ptr_t **map = (nat_ptr_t**)LayerIndex[ cachedLayer ].triggerMap;
		const Layer *layer = &LayerIndex[ cachedLayer ];

		// Cache trigger list before attempting to expire latch
		nat_ptr_t *trigger_list = map[ index - layer->first ];

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
			// Make sure index is between layer first and last scancodes
			if ( map != 0
				&& index <= layer->last
				&& index >= layer->first
				&& *map[ index - layer->first ] != 0 )
			{
				// Set the layer cache
				macroTriggerEventLayerCache[ index ] = macroLayerIndexStack[ layerIndex ];

				return map[ index - layer->first ];
			}
		}
	}

	// Do lookup on default layer
	nat_ptr_t **map = (nat_ptr_t**)LayerIndex[0].triggerMap;

	// Lookup default layer
	const Layer *layer = &LayerIndex[0];

	// Make sure index is between layer first and last scancodes
	if ( map != 0
		&& index <= layer->last
		&& index >= layer->first
		&& *map[ index - layer->first ] != 0 )
	{
		// Set the layer cache to default map
		macroTriggerEventLayerCache[ index ] = 0;

		return map[ index - layer->first ];
	}

	// Otherwise no defined Trigger Macro
	erro_msg("Index has no defined Trigger Macro: ");
	printHex( index );
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
	TriggerEvent *trigger = (TriggerEvent*)trigger_ptr;

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
	if ( trigger->index > MaxScanCode )
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
		printHex( trigger->index );
		print( NL );
		return 2;
	}

	// Add trigger to the Interconnect Cache
	// During each processing loop, a scancode may be re-added depending on it's state
	for ( var_uint_t c = 0; c < macroInterconnectCacheSize; c++ )
	{
		// Check if the same ScanCode
		if ( macroInterconnectCache[ c ].index == trigger->index )
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
void Macro_keyState( uint16_t scanCode, uint8_t state )
{
#if defined(ConnectEnabled_define)
	// Only compile in if a Connect node module is available
	if ( !Connect_master )
	{
		// ScanCodes are only added if there was a state change (on/off)
		switch ( state )
		{
		case ScheduleType_O: // Off
		case ScheduleType_H: // Held
			return;
		}
	}
#endif

	// Lookup done based on size of scanCode
	uint8_t index = 0;
	TriggerType type = TriggerType_Switch1;

	// Only add to macro trigger list if one of three states
	switch ( state )
	{
	case ScheduleType_P: // Pressed
	case ScheduleType_H: // Held
	case ScheduleType_R: // Released
		// Check if ScanCode is out of range
		if ( scanCode > MaxScanCode )
		{
			warn_msg("ScanCode is out of range/not defined: ");
			printInt16( scanCode );
			print( NL );
			return;
		}

		// Determine which type
		if ( scanCode < 256 )
		{
			index = scanCode;
		}
		else if ( scanCode < 512 )
		{
			index = scanCode - 256;
			type = TriggerType_Switch2;
		}
		else if ( scanCode < 768 )
		{
			index = scanCode - 512;
			type = TriggerType_Switch3;
		}
		else if ( scanCode < 1024 )
		{
			index = scanCode - 768;
			type = TriggerType_Switch4;
		}

		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].index = index;
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].state = state;
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].type  = type;
		macroTriggerEventBufferSize++;
		break;
	}
}


// Update the scancode analog state
// States:
//   * 0x00      - Off
//   * 0x01      - Released
//   * 0x02-0xFF - Analog value (low to high)
void Macro_analogState( uint16_t scanCode, uint8_t state )
{
	// Only add to macro trigger list if non-off
	if ( state == 0x00 )
		return;

	// Lookup done based on size of scanCode
	uint8_t index = 0;
	TriggerType type = TriggerType_Analog1;

	// Determine which type
	if ( scanCode < 256 )
	{
		index = scanCode;
	}
	else if ( scanCode < 512 )
	{
		index = scanCode - 256;
		type = TriggerType_Analog2;
	}
	else if ( scanCode < 768 )
	{
		index = scanCode - 512;
		type = TriggerType_Analog3;
	}
	else if ( scanCode < 1024 )
	{
		index = scanCode - 768;
		type = TriggerType_Analog4;
	}

	macroTriggerEventBuffer[ macroTriggerEventBufferSize ].index = index;
	macroTriggerEventBuffer[ macroTriggerEventBufferSize ].state = state;
	macroTriggerEventBuffer[ macroTriggerEventBufferSize ].type  = type;
	macroTriggerEventBufferSize++;
}


// Update led state
// States:
//   * 0x00 - Off
//   * 0x01 - Activate
//   * 0x02 - On
//   * 0x03 - Deactivate
void Macro_ledState( uint16_t ledCode, uint8_t state )
{
	// TODO Handle change for interconnect

	// Lookup done based on size of scanCode
	uint8_t index = ledCode;
	TriggerType type = TriggerType_LED1;

	// Only add to macro trigger list if one of three states
	switch ( state )
	{
	case ScheduleType_A:  // Activate
	case ScheduleType_On: // On
	case ScheduleType_D:  // Deactivate
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].index = index;
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].state = state;
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].type  = type;
		macroTriggerEventBufferSize++;
		break;
	}
}


// Update animation state
// States:
//   * 0x00 - Off
//   * 0x06 - Done
//   * 0x07 - Repeat
void Macro_animationState( uint16_t animationIndex, uint8_t state )
{
	// TODO Handle change for interconnect

	// Lookup done based on size of layerIndex
	uint8_t index = 0;
	TriggerType type = TriggerType_Animation1;

	// Only add to macro trigger list if one of three states
	switch ( state )
	{
	case ScheduleType_Done:   // Activate
	case ScheduleType_Repeat: // On
		// Check if layer is out of range
		// TODO check total animatinos
		/*
		if ( animationIndex > AnimationNum_KLL )
		{
			warn_msg("AnimationIndex is out of range/not defined: ");
			printInt16( animationIndex );
			print( NL );
			return;
		}
		*/

		// Determine which type
		if ( animationIndex < 256 )
		{
			index = animationIndex;
		}
		else if ( animationIndex < 512 )
		{
			index = animationIndex - 256;
			type = TriggerType_Animation2;
		}
		else if ( animationIndex < 768 )
		{
			index = animationIndex - 512;
			type = TriggerType_Animation3;
		}
		else if ( animationIndex < 1024 )
		{
			index = animationIndex - 768;
			type = TriggerType_Animation4;
		}

		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].index = index;
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].state = state;
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].type  = type;
		macroTriggerEventBufferSize++;
		break;
	}
}


/* TODO Merge with Macro_layerState
// Update layer state
// States:
//   * 0x00 - Off
//   * 0x01 - Activate
//   * 0x02 - On
//   * 0x03 - Deactivate
void Macro_layerState( uint16_t layerIndex, uint8_t state )
{
	// TODO Handle change for interconnect

	// Lookup done based on size of layerIndex
	uint8_t index = 0;
	TriggerType type = TriggerType_Layer1;

	// Only add to macro trigger list if one of three states
	switch ( state )
	{
	case ScheduleType_A:  // Activate
	case ScheduleType_On: // On
	case ScheduleType_D:  // Deactivate
		// Check if layer is out of range
		if ( layerIndex > LayerNum_KLL )
		{
			warn_msg("LayerIndex is out of range/not defined: ");
			printInt16( layerIndex );
			print( NL );
			return;
		}

		// Determine which type
		if ( layerIndex < 256 )
		{
			index = layerIndex;
		}
		else if ( layerIndex < 512 )
		{
			index = layerIndex - 256;
			type = TriggerType_Layer2;
		}
		else if ( layerIndex < 768 )
		{
			index = layerIndex - 512;
			type = TriggerType_Layer3;
		}
		else if ( layerIndex < 1024 )
		{
			index = layerIndex - 768;
			type = TriggerType_Layer4;
		}

		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].index = index;
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].state = state;
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].type  = type;
		macroTriggerEventBufferSize++;
		break;
	}
}
*/


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
	for ( var_uint_t keyIndex = 0; keyIndex < macroTriggerEventBufferSize; keyIndex++ )
	{
		if ( macroTriggerEventBuffer[ keyIndex ].index == scanCode )
		{
			ResultMacroRecordList[ resultMacroIndex ].state     = macroTriggerEventBuffer[ keyIndex ].state;
			ResultMacroRecordList[ resultMacroIndex ].stateType = macroTriggerEventBuffer[ keyIndex ].type;
		}
	}

	// Reset the macro position
	ResultMacroRecordList[ resultMacroIndex ].prevPos = 0;
	ResultMacroRecordList[ resultMacroIndex ].pos = 0;
}


// Macro Processing Loop, called often
// Generally takes care of thread-unsafe calls to capabilities that must be synchronized
void Macro_poll()
{
	// Process delayed capabilities
	Result_process_delayed();
}


// Macro Processing Loop, called from the periodic execution thread
// Called once per USB buffer send
void Macro_periodic()
{
	// Latency measurement
	Latency_start_time( macroLatencyResource );

#if defined(ConnectEnabled_define)
	// Only compile in if a Connect node module is available
	// If this is a interconnect slave node, send all scancodes to master node
	if ( !Connect_master )
	{
		if ( macroTriggerEventBufferSize > 0 )
		{
			Connect_send_ScanCode( Connect_id, macroTriggerEventBuffer, macroTriggerEventBufferSize );
			macroTriggerEventBufferSize = 0;
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
			macroTriggerEventBuffer[ macroTriggerEventBufferSize++ ] = macroInterconnectCache[ c ];

			// TODO Handle other TriggerGuide types (e.g. analog)
			switch ( macroInterconnectCache[ c ].type )
			{
			// Normal (Press/Hold/Release)
			case TriggerType_Switch1:
				// Decide what to do based on the current state
				switch ( macroInterconnectCache[ c ].state )
				{
				// Re-add to interconnect cache in hold state
				case ScheduleType_P: // Press
				//case ScheduleType_H: // Hold // XXX Why does this not work? -HaaTa
					macroInterconnectCache[ c ].state = 0x02;
					macroInterconnectCache[ macroInterconnectCacheSize++ ] = macroInterconnectCache[ c ];
					break;

				case ScheduleType_R: // Release
					break;

				// Otherwise, do not re-add
				default:
					break;
				}
				break;

			// Not implemented
			default:
				erro_msg("Interconnect Trigger Event Type - Not Implemented ");
				printInt8( macroInterconnectCache[ c ].type );
				print( NL );
				break;
			}
		}
	}
#endif
	// Macro incoming state debug
	switch ( macroDebugMode )
	{
	case 1:
	case 2:
		// Iterate over incoming triggers
		for ( uint16_t trigger = 0; trigger < macroTriggerEventBufferSize; trigger++ )
		{
			// Show debug info about incoming trigger
			Macro_showTriggerEvent( &macroTriggerEventBuffer[trigger] );
			print( NL );
		}

	case 3:
	default:
		break;
	}

	// Check macroTriggerEventBufferSize to make sure no overflow
	if ( macroTriggerEventBufferSize >= MaxScanCode )
	{
		// No scancodes defined
		if ( MaxScanCode == 0 )
		{
			warn_print("No scancodes defined! Check your BaseMap!");
		}
		// Bug!
		else
		{
			erro_msg("Macro Trigger Event Overflow! Serious Bug! ");
			printInt16( macroTriggerEventBufferSize );
			print( NL );
			macroTriggerEventBufferSize = 0;
		}
	}

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
	Scan_finishedWithMacro( macroTriggerEventBufferSize );

	// Reset TriggerList buffer
	macroTriggerEventBufferSize = 0;

#if defined(_host_)
	// Signal host to read layer state
	Output_callback( "layerState", "" );
#endif

	// Latency measurement
	Latency_end_time( macroLatencyResource );

	// If Macro debug mode is set, clear the USB Buffer
#if defined(Output_USBEnabled_define)
	if ( macroDebugMode == 1 || macroDebugMode == 3 )
	{
		USBKeys_primary.changed = 0;
	}
#endif
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

	// Disable Macro Vote debug mode
	voteDebugMode = 0;

	// Disable Trigger Pending debug mode
	triggerPendingDebugMode = 0;

	// Make sure macro trigger event buffer is empty
	macroTriggerEventBufferSize = 0;

	// Set the current rotated layer to 0
	Macro_rotationLayer = 0;

	// Layer debug mode
	layerDebugMode = 0;

	// Setup Triggers
	Trigger_setup();

	// Setup Results
	Result_setup();

	// Allocate resource for latency measurement
	macroLatencyResource = Latency_add_resource("PartialMap", LatencyOption_Ticks);
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
	for ( index_uint_t layer = 0; layer < LayerNum; layer++ )
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
	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	// Set the macro debug flag depending on the argument
	switch ( arg1Ptr[0] )
	{
	// 3 as argument
	case '3':
		macroDebugMode = macroDebugMode != 3 ? 3 : 0;
		break;

	// 2 as argument
	case '2':
		macroDebugMode = macroDebugMode != 2 ? 2 : 0;
		break;

	// No argument
	case '1':
	case '\0':
		macroDebugMode = macroDebugMode != 1 ? 1 : 0;
		break;

	// Invalid argument
	default:
		return;
	}

	print( NL );
	info_msg("Macro Debug Mode: ");
	printInt8( macroDebugMode );
}

void cliFunc_macroList( char* args )
{
	// Show pending key events
	print( NL );
	info_msg("Pending Key Events: ");
	printInt16( (uint16_t)macroTriggerEventBufferSize );
	print(" : ");
	for ( var_uint_t key = 0; key < macroTriggerEventBufferSize; key++ )
	{
		printHex( macroTriggerEventBuffer[ key ].index );
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
	print(" ");
	printInt16( (uint16_t)record->prevPos );

	// Display result macro index
	print( NL "Result Macro Index: " );
	printInt16( (uint16_t)macro->result ); // Hopefully large enough :P (can't assume 32-bit)

	// Display trigger macro state
	print( NL "Trigger Macro State: " );
	switch ( record->state )
	{
	case TriggerMacro_Press:        print("Press");   break;
	case TriggerMacro_Release:      print("Release"); break;
	case TriggerMacro_PressRelease: print("Press|Release"); break;
	case TriggerMacro_Waiting:      print("Waiting"); break;
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
	print(" ");
	printInt16( (uint16_t)record->prevPos );

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

void cliFunc_voteDebug( char* args )
{
	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	// Set the vote debug flag depending on the argument
	switch ( arg1Ptr[0] )
	{
	// No argument
	case 1:
	case '\0':
		voteDebugMode = voteDebugMode != 1 ? 1 : 0;
		break;

	// Invalid argument
	default:
		return;
	}

	print( NL );
	info_msg("Vote Debug Mode: ");
	printInt8( voteDebugMode );
}

