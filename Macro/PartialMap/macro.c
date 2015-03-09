/* Copyright (C) 2014-2015 by Jacob Alexander
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

// Local Includes
#include "macro.h"



// ----- Function Declarations -----

void cliFunc_capList   ( char* args );
void cliFunc_capSelect ( char* args );
void cliFunc_keyHold   ( char* args );
void cliFunc_keyPress  ( char* args );
void cliFunc_keyRelease( char* args );
void cliFunc_layerList ( char* args );
void cliFunc_layerState( char* args );
void cliFunc_macroDebug( char* args );
void cliFunc_macroList ( char* args );
void cliFunc_macroProc ( char* args );
void cliFunc_macroShow ( char* args );
void cliFunc_macroStep ( char* args );



// ----- Enums -----

// Bit positions are important, passes (correct key) always trump incorrect key votes
typedef enum TriggerMacroVote {
	TriggerMacroVote_Release          = 0x10, // Correct key
	TriggerMacroVote_PassRelease      = 0x18, // Correct key (both pass and release)
	TriggerMacroVote_Pass             = 0x8,  // Correct key
	TriggerMacroVote_DoNothingRelease = 0x4,  // Incorrect key
	TriggerMacroVote_DoNothing        = 0x2,  // Incorrect key
	TriggerMacroVote_Fail             = 0x1,  // Incorrect key
	TriggerMacroVote_Invalid          = 0x0,  // Invalid state
} TriggerMacroVote;

typedef enum TriggerMacroEval {
	TriggerMacroEval_DoNothing,
	TriggerMacroEval_DoResult,
	TriggerMacroEval_DoResultAndRemove,
	TriggerMacroEval_Remove,
} TriggerMacroEval;

typedef enum ResultMacroEval {
	ResultMacroEval_DoNothing,
	ResultMacroEval_Remove,
} ResultMacroEval;



// ----- Variables -----

// Macro Module command dictionary
CLIDict_Entry( capList,     "Prints an indexed list of all non USB keycode capabilities." );
CLIDict_Entry( capSelect,   "Triggers the specified capabilities. First two args are state and stateType." NL "\t\t\033[35mK11\033[0m Keyboard Capability 0x0B" );
CLIDict_Entry( keyHold,     "Send key-hold events to the macro module. Duplicates have undefined behaviour." NL "\t\t\033[35mS10\033[0m Scancode 0x0A" );
CLIDict_Entry( keyPress,    "Send key-press events to the macro module. Duplicates have undefined behaviour." NL "\t\t\033[35mS10\033[0m Scancode 0x0A" );
CLIDict_Entry( keyRelease,  "Send key-release event to macro module. Duplicates have undefined behaviour." NL "\t\t\033[35mS10\033[0m Scancode 0x0A" );
CLIDict_Entry( layerList,   "List available layers." );
CLIDict_Entry( layerState,  "Modify specified indexed layer state <layer> <state byte>." NL "\t\t\033[35mL2\033[0m Indexed Layer 0x02" NL "\t\t0 Off, 1 Shift, 2 Latch, 4 Lock States" );
CLIDict_Entry( macroDebug,  "Disables/Enables sending USB keycodes to the Output Module and prints U/K codes." );
CLIDict_Entry( macroList,   "List the defined trigger and result macros." );
CLIDict_Entry( macroProc,   "Pause/Resume macro processing." );
CLIDict_Entry( macroShow,   "Show the macro corresponding to the given index." NL "\t\t\033[35mT16\033[0m Indexed Trigger Macro 0x10, \033[35mR12\033[0m Indexed Result Macro 0x0C" );
CLIDict_Entry( macroStep,   "Do N macro processing steps. Defaults to 1." );

CLIDict_Def( macroCLIDict, "Macro Module Commands" ) = {
	CLIDict_Item( capList ),
	CLIDict_Item( capSelect ),
	CLIDict_Item( keyHold ),
	CLIDict_Item( keyPress ),
	CLIDict_Item( keyRelease ),
	CLIDict_Item( layerList ),
	CLIDict_Item( layerState ),
	CLIDict_Item( macroDebug ),
	CLIDict_Item( macroList ),
	CLIDict_Item( macroProc ),
	CLIDict_Item( macroShow ),
	CLIDict_Item( macroStep ),
	{ 0, 0, 0 } // Null entry for dictionary end
};


// Macro debug flag - If set, clears the USB Buffers after signalling processing completion
uint8_t macroDebugMode = 0;

// Macro pause flag - If set, the macro module pauses processing, unless unset, or the step counter is non-zero
uint8_t macroPauseMode = 0;

// Macro step counter - If non-zero, the step counter counts down every time the macro module does one processing loop
uint16_t macroStepCounter = 0;


// Key Trigger List Buffer and Layer Cache
// The layer cache is set on press only, hold and release events refer to the value set on press
TriggerGuide macroTriggerListBuffer[ MaxScanCode ];
uint8_t macroTriggerListBufferSize = 0;
var_uint_t macroTriggerListLayerCache[ MaxScanCode ];

// Pending Trigger Macro Index List
//  * Any trigger macros that need processing from a previous macro processing loop
// TODO, figure out a good way to scale this array size without wasting too much memory, but not rejecting macros
//       Possibly could be calculated by the KLL compiler
// XXX It may be possible to calculate the worst case using the KLL compiler
uint16_t macroTriggerMacroPendingList[ TriggerMacroNum ] = { 0 };
uint16_t macroTriggerMacroPendingListSize = 0;

// Layer Index Stack
//  * When modifying layer state and the state is non-0x0, the stack must be adjusted
uint16_t macroLayerIndexStack[ LayerNum + 1 ] = { 0 };
uint16_t macroLayerIndexStackSize = 0;

// Pending Result Macro Index List
//  * Any result macro that needs processing from a previous macro processing loop
uint16_t macroResultMacroPendingList[ ResultMacroNum ] = { 0 };
uint16_t macroResultMacroPendingListSize = 0;



// ----- Capabilities -----

// Sets the given layer with the specified layerState
void Macro_layerState( uint8_t state, uint8_t stateType, uint16_t layer, uint8_t layerState )
{
	// Ignore if layer does not exist
	if ( layer >= LayerNum )
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
}

// Modifies the specified Layer control byte
// Argument #1: Layer Index -> uint16_t
// Argument #2: Layer State -> uint8_t
void Macro_layerState_capability( uint8_t state, uint8_t stateType, uint8_t *args )
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

	Macro_layerState( state, stateType, layer, layerState );
}


// Latches given layer
// Argument #1: Layer Index -> uint16_t
void Macro_layerLatch_capability( uint8_t state, uint8_t stateType, uint8_t *args )
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

	Macro_layerState( state, stateType, layer, 0x02 );
}


// Locks given layer
// Argument #1: Layer Index -> uint16_t
void Macro_layerLock_capability( uint8_t state, uint8_t stateType, uint8_t *args )
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

	Macro_layerState( state, stateType, layer, 0x04 );
}


// Shifts given layer
// Argument #1: Layer Index -> uint16_t
void Macro_layerShift_capability( uint8_t state, uint8_t stateType, uint8_t *args )
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

	Macro_layerState( state, stateType, layer, 0x01 );
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

		return map[ scanCode - layer->first ];
	}

	// If no trigger macro is defined at the given layer, fallthrough to the next layer
	for ( uint16_t layerIndex = 0; layerIndex < macroLayerIndexStackSize; layerIndex++ )
	{
		// Lookup Layer
		const Layer *layer = &LayerIndex[ macroLayerIndexStack[ layerIndex ] ];

		// Check if latch has been pressed for this layer
		// XXX Regardless of whether a key is found, the latch is removed on first lookup
		uint8_t latch = LayerState[ macroLayerIndexStack[ layerIndex ] ] & 0x02;
		if ( latch && latch_expire )
		{
			Macro_layerState( 0, 0, macroLayerIndexStack[ layerIndex ], 0x02 );
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
	return 0;
}


// Update the scancode key state
// States:
//   * 0x00 - Off
//   * 0x01 - Pressed
//   * 0x02 - Held
//   * 0x03 - Released
//   * 0x04 - Unpressed (this is currently ignored)
inline void Macro_keyState( uint8_t scanCode, uint8_t state )
{
	// Only add to macro trigger list if one of three states
	switch ( state )
	{
	case 0x01: // Pressed
	case 0x02: // Held
	case 0x03: // Released
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
	if ( state != 0x00 )
	{
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
	if ( state != 0x00 )
	{
		macroTriggerListBuffer[ macroTriggerListBufferSize ].scanCode = ledCode;
		macroTriggerListBuffer[ macroTriggerListBufferSize ].state    = state;
		macroTriggerListBuffer[ macroTriggerListBufferSize ].type     = 0x01; // LED key
		macroTriggerListBufferSize++;
	}
}


// Append result macro to pending list, checking for duplicates
// Do nothing if duplicate
inline void Macro_appendResultMacroToPendingList( const TriggerMacro *triggerMacro )
{
	// Lookup result macro index
	var_uint_t resultMacroIndex = triggerMacro->result;

	// Iterate through result macro pending list, making sure this macro hasn't been added yet
	for ( var_uint_t macro = 0; macro < macroResultMacroPendingListSize; macro++ )
	{
		// If duplicate found, do nothing
		if ( macroResultMacroPendingList[ macro ] == resultMacroIndex )
			return;
	}

	// No duplicates found, add to pending list
	macroResultMacroPendingList[ macroResultMacroPendingListSize++ ] = resultMacroIndex;

	// Lookup scanCode of the last key in the last combo
	var_uint_t pos = 0;
	for ( uint8_t comboLength = triggerMacro->guide[0]; comboLength > 0; )
	{
		pos += TriggerGuideSize * comboLength + 1;
		comboLength = triggerMacro->guide[ pos ];
	}

	uint8_t scanCode = ((TriggerGuide*)&triggerMacro->guide[ pos - TriggerGuideSize ])->scanCode;

	// Lookup scanCode in buffer list for the current state and stateType
	for ( uint8_t keyIndex = 0; keyIndex < macroTriggerListBufferSize; keyIndex++ )
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


// Determine if long ResultMacro (more than 1 seqence element)
inline uint8_t Macro_isLongResultMacro( const ResultMacro *macro )
{
	// Check the second sequence combo length
	// If non-zero return non-zero (long sequence)
	// 0 otherwise (short sequence)
	var_uint_t position = 1;
	for ( var_uint_t result = 0; result < macro->guide[0]; result++ )
		position += ResultGuideSize( (ResultGuide*)&macro->guide[ position ] );
	return macro->guide[ position ];
}


// Determine if long TriggerMacro (more than 1 sequence element)
inline uint8_t Macro_isLongTriggerMacro( const TriggerMacro *macro )
{
	// Check the second sequence combo length
	// If non-zero return non-zero (long sequence)
	// 0 otherwise (short sequence)
	return macro->guide[ macro->guide[0] * TriggerGuideSize + 1 ];
}


// Votes on the given key vs. guide, short macros
inline TriggerMacroVote Macro_evalShortTriggerMacroVote( TriggerGuide *key, TriggerGuide *guide )
{
	// Depending on key type
	switch ( guide->type )
	{
	// Normal State Type
	case 0x00:
		// For short TriggerMacros completely ignore incorrect keys
		if ( guide->scanCode == key->scanCode )
		{
			switch ( key->state )
			{
			// Correct key, pressed, possible passing
			case 0x01:
				return TriggerMacroVote_Pass;

			// Correct key, held, possible passing or release
			case 0x02:
				return TriggerMacroVote_PassRelease;

			// Correct key, released, possible release
			case 0x03:
				return TriggerMacroVote_Release;
			}
		}

		return TriggerMacroVote_DoNothing;

	// LED State Type
	case 0x01:
		erro_print("LED State Type - Not implemented...");
		break;

	// Analog State Type
	case 0x02:
		erro_print("Analog State Type - Not implemented...");
		break;

	// Invalid State Type
	default:
		erro_print("Invalid State Type. This is a bug.");
		break;
	}

	// XXX Shouldn't reach here
	return TriggerMacroVote_Invalid;
}


// Votes on the given key vs. guide, long macros
// A long macro is defined as a guide with more than 1 combo
inline TriggerMacroVote Macro_evalLongTriggerMacroVote( TriggerGuide *key, TriggerGuide *guide )
{
	// Depending on key type
	switch ( guide->type )
	{
	// Normal State Type
	case 0x00:
		// Depending on the state of the buffered key, make voting decision
		// Incorrect key
		if ( guide->scanCode != key->scanCode )
		{
			switch ( key->state )
			{
			// Wrong key, pressed, fail
			case 0x01:
				return TriggerMacroVote_Fail;

			// Wrong key, held, do not pass (no effect)
			case 0x02:
				return TriggerMacroVote_DoNothing;

			// Wrong key released, fail out if pos == 0
			case 0x03:
				return TriggerMacroVote_DoNothing | TriggerMacroVote_DoNothingRelease;
			}
		}

		// Correct key
		else
		{
			switch ( key->state )
			{
			// Correct key, pressed, possible passing
			case 0x01:
				return TriggerMacroVote_Pass;

			// Correct key, held, possible passing or release
			case 0x02:
				return TriggerMacroVote_PassRelease;

			// Correct key, released, possible release
			case 0x03:
				return TriggerMacroVote_Release;
			}
		}

		break;

	// LED State Type
	case 0x01:
		erro_print("LED State Type - Not implemented...");
		break;

	// Analog State Type
	case 0x02:
		erro_print("Analog State Type - Not implemented...");
		break;

	// Invalid State Type
	default:
		erro_print("Invalid State Type. This is a bug.");
		break;
	}

	// XXX Shouldn't reach here
	return TriggerMacroVote_Invalid;
}


// Evaluate/Update TriggerMacro
TriggerMacroEval Macro_evalTriggerMacro( var_uint_t triggerMacroIndex )
{
	// Lookup TriggerMacro
	const TriggerMacro *macro = &TriggerMacroList[ triggerMacroIndex ];
	TriggerMacroRecord *record = &TriggerMacroRecordList[ triggerMacroIndex ];

	// Check if macro has finished and should be incremented sequence elements
	if ( record->state == TriggerMacro_Release )
	{
		record->state = TriggerMacro_Waiting;
		record->pos = record->pos + macro->guide[ record->pos ] * TriggerGuideSize + 1;
	}

	// Current Macro position
	var_uint_t pos = record->pos;

	// Length of the combo being processed
	uint8_t comboLength = macro->guide[ pos ] * TriggerGuideSize;

	// If no combo items are left, remove the TriggerMacro from the pending list
	if ( comboLength == 0 )
	{
		return TriggerMacroEval_Remove;
	}

	// Check if this is a long Trigger Macro
	uint8_t longMacro = Macro_isLongTriggerMacro( macro );

	// Iterate through the items in the combo, voting the on the key state
	// If any of the pressed keys do not match, fail the macro
	//
	// The macro is waiting for input when in the TriggerMacro_Waiting state
	// Once all keys have been pressed/held (only those keys), entered TriggerMacro_Press state (passing)
	// Transition to the next combo (if it exists) when a single key is released (TriggerMacro_Release state)
	// On scan after position increment, change to TriggerMacro_Waiting state
	// TODO Add support for system LED states (NumLock, CapsLock, etc.)
	// TODO Add support for analog key states
	// TODO Add support for 0x00 Key state (not pressing a key, not all that useful in general)
	// TODO Add support for Press/Hold/Release differentiation when evaluating (not sure if useful)
	TriggerMacroVote overallVote = TriggerMacroVote_Invalid;
	for ( uint8_t comboItem = pos + 1; comboItem < pos + comboLength + 1; comboItem += TriggerGuideSize )
	{
		// Assign TriggerGuide element (key type, state and scancode)
		TriggerGuide *guide = (TriggerGuide*)(&macro->guide[ comboItem ]);

		TriggerMacroVote vote = TriggerMacroVote_Invalid;
		// Iterate through the key buffer, comparing to each key in the combo
		for ( uint8_t key = 0; key < macroTriggerListBufferSize; key++ )
		{
			// Lookup key information
			TriggerGuide *keyInfo = &macroTriggerListBuffer[ key ];

			// If vote is a pass (>= 0x08, no more keys in the combo need to be looked at)
			// Also mask all of the non-passing votes
			vote |= longMacro
				? Macro_evalLongTriggerMacroVote( keyInfo, guide )
				: Macro_evalShortTriggerMacroVote( keyInfo, guide );
			if ( vote >= TriggerMacroVote_Pass )
			{
				vote &= TriggerMacroVote_Release | TriggerMacroVote_PassRelease | TriggerMacroVote_Pass;
				break;
			}
		}

		// If no pass vote was found after scanning all of the keys
		// Fail the combo, if this is a short macro (long macros already will have a fail vote)
		if ( !longMacro && vote < TriggerMacroVote_Pass )
			vote |= TriggerMacroVote_Fail;

		// After voting, append to overall vote
		overallVote |= vote;
	}

	// If no pass vote was found after scanning the entire combo
	// And this is the first position in the combo, just remove it (nothing important happened)
	if ( longMacro && overallVote & TriggerMacroVote_DoNothingRelease && pos == 0 )
		overallVote |= TriggerMacroVote_Fail;

	// Decide new state of macro after voting
	// Fail macro, remove from pending list
	if ( overallVote & TriggerMacroVote_Fail )
	{
		return TriggerMacroEval_Remove;
	}
	// Do nothing, incorrect key is being held or released
	else if ( overallVote & TriggerMacroVote_DoNothing && longMacro )
	{
		// Just doing nothing :)
	}
	// If ready for transition and in Press state, set to Waiting and increment combo position
	// Position is incremented (and possibly remove the macro from the pending list) on the next iteration
	else if ( overallVote & TriggerMacroVote_Release && record->state == TriggerMacro_Press )
	{
		record->state = TriggerMacro_Release;

		// If this is the last combo in the sequence, remove from the pending list
		if ( macro->guide[ record->pos + macro->guide[ record->pos ] * TriggerGuideSize + 1 ] == 0 )
			return TriggerMacroEval_DoResultAndRemove;
	}
	// If passing and in Waiting state, set macro state to Press
	else if ( overallVote & TriggerMacroVote_Pass
	     && ( record->state == TriggerMacro_Waiting || record->state == TriggerMacro_Press ) )
	{
		record->state = TriggerMacro_Press;

		// If in press state, and this is the final combo, send request for ResultMacro
		// Check to see if the result macro only has a single element
		// If this result macro has more than 1 key, only send once
		// TODO Add option to have long macro repeat rate
		if ( macro->guide[ pos + comboLength + 1 ] == 0 )
		{
			// Long result macro (more than 1 combo)
			if ( Macro_isLongResultMacro( &ResultMacroList[ macro->result ] ) )
			{
				// Only ever trigger result once, on press
				if ( overallVote == TriggerMacroVote_Pass )
				{
					return TriggerMacroEval_DoResultAndRemove;
				}
			}
			// Short result macro
			else
			{
				// Only trigger result once, on press, if long trigger (more than 1 combo)
				if ( Macro_isLongTriggerMacro( macro ) )
				{
					return TriggerMacroEval_DoResultAndRemove;
				}
				// Otherwise, trigger result continuously
				else
				{
					return TriggerMacroEval_DoResult;
				}
			}
		}
	}
	// Otherwise, just remove the macro on key release
	// One more result has to be called to indicate to the ResultMacro that the key transitioned to the release state
	else if ( overallVote & TriggerMacroVote_Release )
	{
		return TriggerMacroEval_DoResultAndRemove;
	}

	// If this is a short macro, just remove it
	// The state can be rebuilt on the next iteration
	if ( !longMacro )
		return TriggerMacroEval_Remove;

	return TriggerMacroEval_DoNothing;
}


// Evaluate/Update ResultMacro
inline ResultMacroEval Macro_evalResultMacro( var_uint_t resultMacroIndex )
{
	// Lookup ResultMacro
	const ResultMacro *macro = &ResultMacroList[ resultMacroIndex ];
	ResultMacroRecord *record = &ResultMacroRecordList[ resultMacroIndex ];

	// Current Macro position
	var_uint_t pos = record->pos;

	// Length of combo being processed
	uint8_t comboLength = macro->guide[ pos ];

	// Function Counter, used to keep track of the combo items processed
	var_uint_t funcCount = 0;

	// Combo Item Position within the guide
	var_uint_t comboItem = pos + 1;

	// Iterate through the Result Combo
	while ( funcCount < comboLength )
	{
		// Assign TriggerGuide element (key type, state and scancode)
		ResultGuide *guide = (ResultGuide*)(&macro->guide[ comboItem ]);

		// Do lookup on capability function
		void (*capability)(uint8_t, uint8_t, uint8_t*) = (void(*)(uint8_t, uint8_t, uint8_t*))(CapabilitiesList[ guide->index ].func);

		// Call capability
		capability( record->state, record->stateType, &guide->args );

		// Increment counters
		funcCount++;
		comboItem += ResultGuideSize( (ResultGuide*)(&macro->guide[ comboItem ]) );
	}

	// Move to next item in the sequence
	record->pos = comboItem;

	// If the ResultMacro is finished, remove
	if ( macro->guide[ comboItem ] == 0 )
	{
		record->pos = 0;
		return ResultMacroEval_Remove;
	}

	// Otherwise leave the macro in the list
	return ResultMacroEval_DoNothing;
}


// Update pending trigger list
inline void Macro_updateTriggerMacroPendingList()
{
	// Iterate over the macroTriggerListBuffer to add any new Trigger Macros to the pending list
	for ( uint8_t key = 0; key < macroTriggerListBufferSize; key++ )
	{
		// TODO LED States
		// TODO Analog Switches
		// Only add TriggerMacro to pending list if key was pressed (not held, released or off)
		if ( macroTriggerListBuffer[ key ].state == 0x00 && macroTriggerListBuffer[ key ].state != 0x01 )
			continue;

		// TODO Analog
		// If this is a release case, indicate to layer lookup for possible latch expiry
		uint8_t latch_expire = macroTriggerListBuffer[ key ].state == 0x03;

		// Lookup Trigger List
		nat_ptr_t *triggerList = Macro_layerLookup( &macroTriggerListBuffer[ key ], latch_expire );

		// Number of Triggers in list
		nat_ptr_t triggerListSize = triggerList[0];

		// Iterate over triggerList to see if any TriggerMacros need to be added
		// First item is the number of items in the TriggerList
		for ( var_uint_t macro = 1; macro < triggerListSize + 1; macro++ )
		{
			// Lookup trigger macro index
			var_uint_t triggerMacroIndex = triggerList[ macro ];

			// Iterate over macroTriggerMacroPendingList to see if any macro in the scancode's
			//  triggerList needs to be added
			var_uint_t pending = 0;
			for ( ; pending < macroTriggerMacroPendingListSize; pending++ )
			{
				// Stop scanning if the trigger macro index is found in the pending list
				if ( macroTriggerMacroPendingList[ pending ] == triggerMacroIndex )
					break;
			}

			// If the triggerMacroIndex (macro) was not found in the macroTriggerMacroPendingList
			// Add it to the list
			if ( pending == macroTriggerMacroPendingListSize )
			{
				macroTriggerMacroPendingList[ macroTriggerMacroPendingListSize++ ] = triggerMacroIndex;

				// Reset macro position
				TriggerMacroRecordList[ triggerMacroIndex ].pos   = 0;
				TriggerMacroRecordList[ triggerMacroIndex ].state = TriggerMacro_Waiting;
			}
		}
	}
}


// Macro Procesing Loop
// Called once per USB buffer send
inline void Macro_process()
{
	// Only do one round of macro processing between Output Module timer sends
	if ( USBKeys_Sent != 0 )
		return;

	// If the pause flag is set, only process if the step counter is non-zero
	if ( macroPauseMode )
	{
		if ( macroStepCounter == 0 )
			return;

		// Proceed, decrementing the step counter
		macroStepCounter--;
		dbug_print("Macro Step");
	}

	// Update pending trigger list, before processing TriggerMacros
	Macro_updateTriggerMacroPendingList();

	// Tail pointer for macroTriggerMacroPendingList
	// Macros must be explicitly re-added
	var_uint_t macroTriggerMacroPendingListTail = 0;

	// Iterate through the pending TriggerMacros, processing each of them
	for ( var_uint_t macro = 0; macro < macroTriggerMacroPendingListSize; macro++ )
	{
		switch ( Macro_evalTriggerMacro( macroTriggerMacroPendingList[ macro ] ) )
		{
		// Trigger Result Macro (purposely falling through)
		case TriggerMacroEval_DoResult:
			// Append ResultMacro to PendingList
			Macro_appendResultMacroToPendingList( &TriggerMacroList[ macroTriggerMacroPendingList[ macro ] ] );

		default:
			macroTriggerMacroPendingList[ macroTriggerMacroPendingListTail++ ] = macroTriggerMacroPendingList[ macro ];
			break;

		// Trigger Result Macro and Remove (purposely falling through)
		case TriggerMacroEval_DoResultAndRemove:
			// Append ResultMacro to PendingList
			Macro_appendResultMacroToPendingList( &TriggerMacroList[ macroTriggerMacroPendingList[ macro ] ] );

		// Remove Macro from Pending List, nothing to do, removing by default
		case TriggerMacroEval_Remove:
			break;
		}
	}

	// Update the macroTriggerMacroPendingListSize with the tail pointer
	macroTriggerMacroPendingListSize = macroTriggerMacroPendingListTail;


	// Tail pointer for macroResultMacroPendingList
	// Macros must be explicitly re-added
	var_uint_t macroResultMacroPendingListTail = 0;

	// Iterate through the pending ResultMacros, processing each of them
	for ( var_uint_t macro = 0; macro < macroResultMacroPendingListSize; macro++ )
	{
		switch ( Macro_evalResultMacro( macroResultMacroPendingList[ macro ] ) )
		{
		// Re-add macros to pending list
		case ResultMacroEval_DoNothing:
		default:
			macroResultMacroPendingList[ macroResultMacroPendingListTail++ ] = macroResultMacroPendingList[ macro ];
			break;

		// Remove Macro from Pending List, nothing to do, removing by default
		case ResultMacroEval_Remove:
			break;
		}
	}

	// Update the macroResultMacroPendingListSize with the tail pointer
	macroResultMacroPendingListSize = macroResultMacroPendingListTail;

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

	// Initialize TriggerMacro states
	for ( var_uint_t macro = 0; macro < TriggerMacroNum; macro++ )
	{
		TriggerMacroRecordList[ macro ].pos   = 0;
		TriggerMacroRecordList[ macro ].state = TriggerMacro_Waiting;
	}

	// Initialize ResultMacro states
	for ( var_uint_t macro = 0; macro < ResultMacroNum; macro++ )
	{
		ResultMacroRecordList[ macro ].pos       = 0;
		ResultMacroRecordList[ macro ].state     = 0;
		ResultMacroRecordList[ macro ].stateType = 0;
	}
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
		void (*capability)(uint8_t, uint8_t, uint8_t*) = (void(*)(uint8_t, uint8_t, uint8_t*))(CapabilitiesList[ cap ].func);
		capability( 0xFF, 0xFF, 0 );
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

			void (*capability)(uint8_t, uint8_t, uint8_t*) = (void(*)(uint8_t, uint8_t, uint8_t*))(CapabilitiesList[ cap ].func);
			capability( argSet[0], argSet[1], &argSet[2] );
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
	for ( uint8_t key = 0; key < macroTriggerListBufferSize; key++ )
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
	printInt16( (uint16_t)macroResultMacroPendingListSize );
	print(" : ");
	for ( var_uint_t macro = 0; macro < macroResultMacroPendingListSize; macro++ )
	{
		printHex( macroResultMacroPendingList[ macro ] );
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
			void (*capability)(uint8_t, uint8_t, uint8_t*) = (void(*)(uint8_t, uint8_t, uint8_t*))(CapabilitiesList[ guide->index ].func);
			capability( 0xFF, 0xFF, 0 );

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

