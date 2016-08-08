/* Copyright (C) 2014-2016 by Jacob Alexander
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
#include <led.h>
#include <print.h>

// Local Includes
#include "trigger.h"
#include "kll.h"



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



// ----- Generated KLL Variables -----

extern const Capability CapabilitiesList[];

extern const TriggerMacro TriggerMacroList[];
extern TriggerMacroRecord TriggerMacroRecordList[];

extern const ResultMacro ResultMacroList[];



// ----- Variables -----

// Key Trigger List Buffer and Layer Cache
// The layer cache is set on press only, hold and release events refer to the value set on press
extern TriggerGuide macroTriggerListBuffer[];
extern var_uint_t macroTriggerListBufferSize;
extern var_uint_t macroTriggerListLayerCache[];

// Pending Trigger Macro Index List
//  * Any trigger macros that need processing from a previous macro processing loop
// TODO, figure out a good way to scale this array size without wasting too much memory, but not rejecting macros
//       Possibly could be calculated by the KLL compiler
// XXX It may be possible to calculate the worst case using the KLL compiler
index_uint_t macroTriggerMacroPendingList[ TriggerMacroNum ] = { 0 };
index_uint_t macroTriggerMacroPendingListSize = 0;



// ----- Protected Macro Functions -----

extern nat_ptr_t *Macro_layerLookup( TriggerGuide *guide, uint8_t latch_expire );

extern void Macro_appendResultMacroToPendingList( const TriggerMacro *triggerMacro );



// ----- Functions -----

// Determine if long ResultMacro (more than 1 seqence element)
uint8_t Macro_isLongResultMacro( const ResultMacro *macro )
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
uint8_t Macro_isLongTriggerMacro( const TriggerMacro *macro )
{
	// Check the second sequence combo length
	// If non-zero return non-zero (long sequence)
	// 0 otherwise (short sequence)
	return macro->guide[ macro->guide[0] * TriggerGuideSize + 1 ];
}


// Votes on the given key vs. guide, short macros
TriggerMacroVote Macro_evalShortTriggerMacroVote( TriggerGuide *key, TriggerGuide *guide )
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
TriggerMacroVote Macro_evalLongTriggerMacroVote( TriggerGuide *key, TriggerGuide *guide )
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
		for ( var_uint_t key = 0; key < macroTriggerListBufferSize; key++ )
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


// Update pending trigger list
void Macro_updateTriggerMacroPendingList()
{
	// Iterate over the macroTriggerListBuffer to add any new Trigger Macros to the pending list
	for ( var_uint_t key = 0; key < macroTriggerListBufferSize; key++ )
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

		// If there was an error during lookup, skip
		if ( triggerList == 0 )
			continue;

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



void Trigger_state( uint8_t type, uint8_t state, uint8_t index )
{
}


uint8_t Trigger_update( uint8_t type, uint8_t state, uint8_t index )
{
	return 0;
}


void Trigger_setup()
{
	// Initialize TriggerMacro states
	for ( var_uint_t macro = 0; macro < TriggerMacroNum; macro++ )
	{
		TriggerMacroRecordList[ macro ].pos   = 0;
		TriggerMacroRecordList[ macro ].state = TriggerMacro_Waiting;
	}
}


void Trigger_process()
{
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
}

