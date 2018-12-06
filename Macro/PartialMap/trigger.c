/* Copyright (C) 2014-2019 by Jacob Alexander
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
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
#include "result.h"
#include "layer.h"
#include "kll.h"



// ----- Enums -----

// Bit positions are important, passes (correct key) always trump incorrect key votes
typedef enum TriggerMacroVote {
	TriggerMacroVote_Release          = 0x80, // Correct key
	TriggerMacroVote_PassRelease      = 0xC0, // Correct key (both pass and release)
	TriggerMacroVote_Pass             = 0x40, // Correct key
	TriggerMacroVote_SustainEvent     = 0x20, // Keep event around for the next processing cycle
	TriggerMacroVote_Reserved2        = 0x10,
	TriggerMacroVote_Reserved1        = 0x8,
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

// Incoming Trigger Event Buffer
extern TriggerEvent macroTriggerEventBuffer[];
extern var_uint_t macroTriggerEventBufferSize;
extern var_uint_t macroTriggerEventLayerCache[];

// Debug Variables
extern uint8_t triggerPendingDebugMode;
extern uint8_t voteDebugMode;

// Pending Trigger Macro Index List
//  * Any trigger macros that need processing from a previous macro processing loop
// TODO, figure out a good way to scale this array size without wasting too much memory, but not rejecting macros
//       Possibly could be calculated by the KLL compiler
// XXX It may be possible to calculate the worst case using the KLL compiler
#if TriggerMacroNum == 0
#undef TriggerMacroNum
#define TriggerMacroNum 1
#endif
index_uint_t macroTriggerMacroPendingList[ TriggerMacroNum ] = { 0 };
index_uint_t macroTriggerMacroPendingListSize = 0;



// ----- Protected Macro Functions -----

extern uint8_t Macro_scheduleLookup( state_uint_t state, Schedule **schedule );
extern ScheduleParam Macro_determineGenericTrigger( state_uint_t state );



// ----- Functions -----

// -- Debug --

// Show TriggerMacroVote
void Trigger_showTriggerMacroVote( TriggerMacroVote vote, uint8_t long_trigger_macro )
{
	const char *result = "";

	// Long Macro
	if ( long_trigger_macro )
	{
		print("l");
	}
	// Short Macro
	else
	{
		print("s");
	}

	// Static voting
	switch ( vote )
	{
	case TriggerMacroVote_Invalid:
		result = "V:I";
		break;

	case TriggerMacroVote_Release:
		result = "V:R";
		break;

	case TriggerMacroVote_Pass:
		result = "V:P";
		break;

	case TriggerMacroVote_PassRelease:
		result = "V:PR";
		break;

	default:
		print("V:");
		if ( vote & TriggerMacroVote_Fail )
		{
			print("F");
		}
		if ( vote & TriggerMacroVote_DoNothingRelease )
		{
			print("NR");
		}
		else if ( vote & TriggerMacroVote_DoNothing )
		{
			print("N");
		}
		return;
	}

	print( result );
}


// -- General --

// Determine if long ResultMacro (more than 1 seqence element)
uint8_t Trigger_isLongResultMacro( const ResultMacro *macro )
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
uint8_t Trigger_isLongTriggerMacro( const TriggerMacro *macro )
{
	// Check the second sequence combo length
	// If non-zero return non-zero (long sequence)
	// 0 otherwise (short sequence)
	return macro->guide[ macro->guide[0] * TriggerGuideSize + 1 ];
}


// Determine if combo element is state-scheduled
uint8_t Trigger_isStateScheduled( TriggerGuide *guide )
{
	Schedule *schedule = NULL;
	print(" A ");
	// If the schedule is invalid, or 0 length, handle as a generic state
	if ( !Macro_scheduleLookup( guide->state, &schedule ) || schedule->count == 0 )
	{
		// Invalid lookup, use generic state
		return 0;
	}
	print(" B ");

	// Generic triggers only contain a single Generic state in the schedule
	// and may (or may not) have a time scheduled
	if ( schedule->count == 1 && schedule->params[0].state & ScheduleType_Gen )
	{
		switch ( guide->type )
		{
		// Normal State Type
		case TriggerType_Switch1:
		case TriggerType_Switch2:
		case TriggerType_Switch3:
		case TriggerType_Switch4:
		// LED State Type
		case TriggerType_LED1:
		// Layer State Type
		case TriggerType_Layer1:
		case TriggerType_Layer2:
		case TriggerType_Layer3:
		case TriggerType_Layer4:
		// Activity State Types
		case TriggerType_Sleep1:
		case TriggerType_Resume1:
		case TriggerType_Inactive1:
		case TriggerType_Active1:
		// Animation State Type
		case TriggerType_Animation1:
		case TriggerType_Animation2:
		case TriggerType_Animation3:
		case TriggerType_Animation4:
			// Generic state
			print(" C ");
			return 0;

		// Analog State Type
		case TriggerType_Analog1:
		case TriggerType_Analog2:
		case TriggerType_Analog3:
		case TriggerType_Analog4:
		// Rotation State Type
		case TriggerType_Rotation1:
		default:
			// Should be state scheduled even though state is set to Generic
			print(" D ");
			break;
		}
	}

	// Otherwise, this guide is state scheduled
	print(" E ");
	return 1;
}


// Evaluate timing
// TODO, be able to mark dual states
//       - Should evaluate timing after state
//       - If bad, mask out any pass criteria
//       - If not set to auto state (guide), long triggers do not fail out immediately
TriggerMacroVote Trigger_evalTiming( Time elapsed, Time schedule )
{
	// Compare times
	// Not enough time has elapsed
	if ( Time_compare( schedule, elapsed ) == -1 )
	{
		return TriggerMacroVote_Fail;
	}
	// Exact, or sufficient time has elapsed
	return TriggerMacroVote_Pass;
}

// Handle short trigger PHRO/AODO state transitions
TriggerMacroVote Trigger_evalShortTriggerMacroVote_PHRO_(
	ScheduleState in_state,
	Schedule guide_state
)
{
	// TODO
	// 1) Use the voting stage to determine the state of the schedule
	// 2) If a stage has been passed, indicate the new stage
	// 3) If all stages have been passed, indicate a pass
	// -- At the end of voting if there
	// Continued triggers must be claimed each cycle in order to stay around, otherwise they are ejected
	// Claiming: Continued triggers are run through the layer mapping using the incoming triggers mappings
	//  - This must be done through the same layer as the incoming trigger mapping (may have to store during the cycle?)
	return TriggerMacroVote_Pass;
}

// Handle short trigger PHRO/AODO state transitions
TriggerMacroVote Trigger_evalShortTriggerMacroVote_PHRO( ScheduleState state )
{
	switch ( state & 0x0F )
	{
	// Correct key, pressed, possible passing
	case ScheduleType_P:
		return TriggerMacroVote_Pass;

	// Correct key, held, possible passing or release
	case ScheduleType_H:
		return TriggerMacroVote_PassRelease;

	// Correct key, released, possible release
	case ScheduleType_R:
		return TriggerMacroVote_Release;

	// Invalid state, fail
	default:
		return TriggerMacroVote_Fail;
	}
}


// Handle short trigger DRO state transitions
TriggerMacroVote Trigger_evalShortTriggerMacroVote_DRO( ScheduleState state )
{
	switch ( state )
	{
	// Correct event, possible passing
	case ScheduleType_Done:
	case ScheduleType_Repeat:
		return TriggerMacroVote_Pass;

	// Invalid state, fail
	default:
		return TriggerMacroVote_Fail;
	}
}


// Votes on the given key vs. guide, short macros
TriggerMacroVote Trigger_evalShortTriggerMacroVote(
	Time *last_event,
	TriggerEvent *event,
	TriggerGuide *guide,
	TriggerMacroVote *cur_vote
)
{
	// Lookup full index
	var_uint_t guide_index = KLL_TriggerIndex_loopkup( guide->type, guide->scanCode );
	var_uint_t event_index = KLL_TriggerIndex_loopkup( event->type, event->index );

	// Lookup generic schedule
	ScheduleParam gen_schedule = Macro_determineGenericTrigger( guide->state );

	// Return value
	TriggerMacroVote vote = TriggerMacroVote_Invalid;

	// Depending on key type
	switch ( guide->type )
	{
	// Normal State Type
	case TriggerType_Switch1:
	case TriggerType_Switch2:
	case TriggerType_Switch3:
	case TriggerType_Switch4:
	// LED State Type
	case TriggerType_LED1:
	// Layer State Type
	case TriggerType_Layer1:
	case TriggerType_Layer2:
	case TriggerType_Layer3:
	case TriggerType_Layer4:
	// Activity State Types
	case TriggerType_Sleep1:
	case TriggerType_Resume1:
	case TriggerType_Inactive1:
	case TriggerType_Active1:
		// For short TriggerMacros completely ignore incorrect keys
		// Only monitor 0x70 bits if set in the guide, otherwise ensure they are 0x00
		// Used for Layer state information
		if (
			guide_index == event_index &&
			guide->type == event->type &&
			(
				(gen_schedule.state & 0x70) == (event->state & 0x70) ||
				(gen_schedule.state & 0x70) == 0x00
			)
		)
		{
			// Make sure timing has been satisfied
			Time elapsed = Time_duration( *last_event );
			if ( Trigger_evalTiming( elapsed, gen_schedule.time ) == TriggerMacroVote_Pass )
			{
				vote = Trigger_evalShortTriggerMacroVote_PHRO( event->state );
				break;
			}

			// Otherwise do nothing
		}

		vote = TriggerMacroVote_DoNothing;
		break;

	// Analog State Type
	case TriggerType_Analog1:
	case TriggerType_Analog2:
	case TriggerType_Analog3:
	case TriggerType_Analog4:
		erro_print("Analog State Type - Generic not implemented...");
		break;

	// Animation State Type
	case TriggerType_Animation1:
	case TriggerType_Animation2:
	case TriggerType_Animation3:
	case TriggerType_Animation4:
		erro_print("Animation State Type - Generic not implemented...");
		break;

	// Rotation State Type
	case TriggerType_Rotation1:
		erro_print("Rotation State Type - Generic is invalid, this is a bug.");
		break;

	// Invalid State Type
	default:
		erro_print("Invalid State Type. This is a bug.");
		break;
	}

	// If this is a combo macro, make a preference for TriggerMacroVote_Pass instead of TriggerMacroVote_PassRelease
	if ( *cur_vote != TriggerMacroVote_Invalid && event_index == guide_index )
	{
		// Make sure the votes are different and one of them are Pass
		if ( *cur_vote != vote
			&& ( *cur_vote == TriggerMacroVote_Pass || vote == TriggerMacroVote_Pass )
			&& ( *cur_vote == TriggerMacroVote_PassRelease || vote == TriggerMacroVote_PassRelease )
		)
		{
			*cur_vote = TriggerMacroVote_Pass;
			vote = TriggerMacroVote_Pass;
		}
	}

	return vote;
}


// Handle long trigger PHRO/AODO state transitions
TriggerMacroVote Trigger_evalLongTriggerMacroVote_PHRO( ScheduleState state, uint8_t correct )
{
	// Correct scancode match
	if ( correct )
	{
		switch ( state )
		{
		// Correct key, pressed, possible passing
		case ScheduleType_P:
			return TriggerMacroVote_Pass;

		// Correct key, held, possible passing or release
		case ScheduleType_H:
			return TriggerMacroVote_PassRelease;

		// Correct key, released, possible release
		case ScheduleType_R:
			return TriggerMacroVote_Release;

		// Invalid state, fail
		default:
			return TriggerMacroVote_Fail;
		}
	}
	// Incorrect scancode match
	else
	{
		switch ( state )
		{
		// Wrong key, pressed, fail
		case ScheduleType_P:
			return TriggerMacroVote_Fail;

		// Wrong key, held, do not pass (no effect)
		case ScheduleType_H:
			return TriggerMacroVote_DoNothing;

		// Wrong key released, fail out if pos == 0
		case ScheduleType_R:
			return TriggerMacroVote_DoNothing | TriggerMacroVote_DoNothingRelease;

		// Invalid state, fail
		default:
			return TriggerMacroVote_Fail;
		}
	}
}


// Handle long trigger DRO state transitions
TriggerMacroVote Trigger_evalLongTriggerMacroVote_DRO( ScheduleState state, uint8_t correct )
{
	// Correct match
	if ( correct )
	{
		switch ( state )
		{
		// Correct event, possible passing
		case ScheduleType_Done:
		case ScheduleType_Repeat:
			return TriggerMacroVote_Pass;

		// Invalid state, fail
		default:
			return TriggerMacroVote_Fail;
		}
	}
	// Incorrect match
	else
	{
		return TriggerMacroVote_Fail;
	}
}


// Votes on the given key vs. guide, long macros
// A long macro is defined as a guide with more than 1 combo
TriggerMacroVote Trigger_evalLongTriggerMacroVote(
	Time *last_event,
	TriggerEvent *event,
	TriggerGuide *guide,
	TriggerMacroVote *cur_vote
)
{
	// Lookup full index
	var_uint_t guide_index = KLL_TriggerIndex_loopkup( guide->type, guide->scanCode );
	var_uint_t event_index = KLL_TriggerIndex_loopkup( event->type, event->index );

	// Lookup generic schedule
	ScheduleParam gen_schedule = Macro_determineGenericTrigger( guide->state );

	// Depending on key type
	switch ( guide->type )
	{
	// Normal State Type
	case TriggerType_Switch1:
	case TriggerType_Switch2:
	case TriggerType_Switch3:
	case TriggerType_Switch4:
	// LED State Type
	case TriggerType_LED1:
	// Layer State Type
	case TriggerType_Layer1:
	case TriggerType_Layer2:
	case TriggerType_Layer3:
	case TriggerType_Layer4:
	// Activity State Types
	case TriggerType_Sleep1:
	case TriggerType_Resume1:
	case TriggerType_Inactive1:
	case TriggerType_Active1:
		// Depending on the state of the buffered key, make voting decision
		// Only monitor 0x70 bits if set in the guide, otherwise ensure they are 0x00
		// Used for Layer state information
		// Correct key
		if (
			guide_index == event_index &&
			guide->type == event->type &&
			(
				(gen_schedule.state & 0x70) == (event->state & 0x70) ||
				(gen_schedule.state & 0x70) == 0x00
			)
		)
		{
			// Make sure timing has been satisfied
			Time elapsed = Time_duration( *last_event );
			if ( Trigger_evalTiming( elapsed, gen_schedule.time ) == TriggerMacroVote_Pass )
			{
				return Trigger_evalLongTriggerMacroVote_PHRO( event->state, 1 );
			}
		}

		// Incorrect key
		return Trigger_evalLongTriggerMacroVote_PHRO( event->state, 0 );

	// Analog State Type
	case TriggerType_Analog1:
	case TriggerType_Analog2:
	case TriggerType_Analog3:
	case TriggerType_Analog4:
		erro_print("Analog State Type (long) - Generic not implemented...");
		break;

	// Animation State Type
	case TriggerType_Animation1:
	case TriggerType_Animation2:
	case TriggerType_Animation3:
	case TriggerType_Animation4:
		erro_print("Animation State Type (long) - Generic not implemented...");
		break;

	// Rotation State Type
	case TriggerType_Rotation1:
		// Rotation triggers use state as the index, rather than encoding a type of action
		// There is only "activated" state for rotations, which is only sent once
		// This makes rotations not so useful for long macros
		// (though it may be possible to implement it if there is demand)
		erro_print("Rotation State Type (Long Macros) - Not implemented/Invalid...");
		break;

	// Invalid State Type
	default:
		erro_print("Invalid State Type. This is a bug.");
		break;
	}

	// XXX Shouldn't reach here
	return TriggerMacroVote_Invalid;
}


// Votes on the given key vs. guide, state scheduled (long or short macro)
TriggerMacroVote Trigger_evalStateScheduledTriggerMacroVote(
	Time *last_event,
	uint8_t *schedule_index,
	TriggerEvent *event,
	TriggerGuide *guide,
	TriggerMacroVote *cur_vote
)
{
	// Lookup full index
	var_uint_t guide_index = KLL_TriggerIndex_loopkup( guide->type, guide->scanCode );
	var_uint_t event_index = KLL_TriggerIndex_loopkup( event->type, event->index );

	// Lookup schedule
	// For state scheduling, the schedule must be valid
	Schedule *schedule = NULL;
	if ( !Macro_scheduleLookup( guide->state, &schedule ) )
	{
		return TriggerMacroVote_Fail;
	}
	ScheduleParam *cur_schedule = &schedule->params[ *schedule_index ];

	// Return value
	TriggerMacroVote vote = TriggerMacroVote_Invalid;

	// Handle special cases
	// default handles usual case
	switch ( guide->type )
	{
	// Layer State Type
	case TriggerType_Layer1:
	case TriggerType_Layer2:
	case TriggerType_Layer3:
	case TriggerType_Layer4:
		// Match index, type and state
		// Only monitor 0x70 bits if set in the guide, otherwise ensure they are 0x00
		// Used for Layer state information
		if (
			guide_index == event_index &&
			guide->type == event->type &&
			(cur_schedule->state & 0x0F) == (event->state & 0x0F) &&
			(
				(cur_schedule->state & 0x70) == (event->state & 0x70) ||
				(cur_schedule->state & 0x70) == 0x00
			)
		)
		{
			// Make sure timing has been satisfied
			// Timing parameter always exists, but it may be set to 0 (where it will always pass)
			Time elapsed = Time_duration( *last_event );
			if ( Trigger_evalTiming( elapsed, cur_schedule->time ) == TriggerMacroVote_Pass )
			{
				// If this is an old event, update last event
				(*schedule_index)++;

				// Since this event was valid, indicate that we should sustain the event
				if ( *schedule_index < schedule->count )
				{
					vote = TriggerMacroVote_SustainEvent;
					break;
				}

				// Unless this is the final scheduled item, in which case this should be a pass release
				vote = TriggerMacroVote_PassRelease;
				break;
			}
		}

		// Otherwise, ignore
		vote = TriggerMacroVote_DoNothing;
		break;

	// Analog State Type
	case TriggerType_Analog1:
	case TriggerType_Analog2:
	case TriggerType_Analog3:
	case TriggerType_Analog4:
		// TODO
		erro_print("Analog State Type - Not implemented...");
		break;

	// Default Case
	default:
		// Match index, type and state
		if (
			guide_index == event_index &&
			guide->type == event->type &&
			cur_schedule->state == event->state
		)
		{
			// Make sure timing has been satisfied
			// Timing parameter always exists, but it may be set to 0 (where it will always pass)
			Time elapsed = Time_duration( *last_event );
			if ( Trigger_evalTiming( elapsed, cur_schedule->time ) == TriggerMacroVote_Pass )
			{
				// If this is an old event, update last event
				(*schedule_index)++;

				// Since this event was valid, indicate that we should sustain the event
				if ( *schedule_index < schedule->count )
				{
					vote = TriggerMacroVote_SustainEvent;
					break;
				}

				// Unless this is the final scheduled item, in which case this should be a pass release
				vote = TriggerMacroVote_PassRelease;
				break;
			}
		}

		// Otherwise, ignore
		vote = TriggerMacroVote_DoNothing;
		break;
	}

	return vote;
}


// Iterate over combo, voting on the key state
TriggerMacroVote Trigger_overallVote(
	const TriggerMacro *macro,
	TriggerMacroRecord *record,
	uint8_t long_trigger_macro,
	var_uint_t pos
)
{
	// Length of the combo being processed
	uint8_t comboLength = macro->guide[ pos ] * TriggerGuideSize;

	// The macro is waiting for input when in the TriggerMacro_Waiting state
	// Once all keys have been pressed/held (only those keys), entered TriggerMacro_Press state (passing)
	// Transition to the next combo (if it exists) when a single key is released (TriggerMacro_Release state)
	// On scan after position increment, change to TriggerMacro_Waiting state
	// TODO Add support for 0x00 Key state (not pressing a key, not all that useful in general)
	TriggerMacroVote overallVote = TriggerMacroVote_Invalid;
	for ( uint8_t comboItem = pos + 1; comboItem < pos + comboLength + 1; comboItem += TriggerGuideSize )
	{
		// Assign TriggerGuide element (key type, state and scancode)
		TriggerGuide *guide = (TriggerGuide*)(&macro->guide[ comboItem ]);

		// Determine the last event time (starts with trigger macro record)
		// However, if there is an old event (from a prior periodic loop), then that will take
		// precedence (override)
		Time last_event = record->last_pos_event;

		// For state-scheduled elements we'll need to keep track of which schedule element
		// we're currently processing
		uint8_t schedule_index = 0;

		TriggerMacroVote vote = TriggerMacroVote_Invalid;
		// Iterate through the key buffer, comparing to each key in the combo
		for ( var_uint_t key = 0; key < macroTriggerEventBufferSize; key++ )
		{
			// Lookup key information
			TriggerEvent *triggerInfo = &macroTriggerEventBuffer[ key ];

			// Determine if this element is state scheduled
			if ( Trigger_isStateScheduled( guide ) )
			{
				vote |= Trigger_evalStateScheduledTriggerMacroVote(
					&last_event,
					&schedule_index,
					triggerInfo,
					guide,
					&overallVote
				);

				// Check for sustained vote
				// Used to determine whether an event is sustained
				if ( vote & TriggerMacroVote_SustainEvent )
				{
					// Sustain event
					triggerInfo->status = TriggerEventStatus_Hold;

					// Sustained vote is cleared after reading
					vote &= ~(TriggerMacroVote_SustainEvent);
				}
				continue;
			}

			// Otherwise determine what to do based on state and context
			// Vote on triggers
			vote |= long_trigger_macro
				? Trigger_evalLongTriggerMacroVote(
					&last_event,
					triggerInfo,
					guide,
					&overallVote
				)
				: Trigger_evalShortTriggerMacroVote(
					&last_event,
					triggerInfo,
					guide,
					&overallVote
				);
		}

		// Mask out incorrect votes, if anything indicates a pass
		if ( vote >= TriggerMacroVote_Pass )
		{
			vote &= TriggerMacroVote_Release | TriggerMacroVote_PassRelease | TriggerMacroVote_Pass;
		}

		// If no pass vote was found after scanning all of the keys
		// Fail the combo, if this is a short macro (long macros already will have a fail vote)
		if ( !long_trigger_macro && vote < TriggerMacroVote_Pass )
		{
			vote |= TriggerMacroVote_Fail;
		}

		// After voting, append to overall vote
		overallVote |= vote;
	}

	return overallVote;
}


// Evaluate/Update TriggerMacro
TriggerMacroEval Trigger_evalTriggerMacro( var_uint_t triggerMacroIndex )
{
	// Lookup TriggerMacro
	const TriggerMacro *macro = &TriggerMacroList[ triggerMacroIndex ];
	TriggerMacroRecord *record = &TriggerMacroRecordList[ triggerMacroIndex ];

	// Check if this is a long Trigger Macro
	uint8_t long_trigger_macro = Trigger_isLongTriggerMacro( macro );

	// Long Macro
	if ( long_trigger_macro )
	{
		// Check if macro has finished and should be incremented sequence elements
		if ( record->state != TriggerMacro_Waiting )
		{
			record->prevPos = record->pos;
			record->pos = record->pos + macro->guide[ record->pos ] * TriggerGuideSize + 1;
			record->last_pos_event = Time_now();
		}

		// Current Macro position
		var_uint_t pos = record->pos;

		// Length of the combo being processed
		uint8_t comboLength = macro->guide[ pos ] * TriggerGuideSize;

		TriggerMacroVote overallVote = TriggerMacroVote_Invalid;
		// Iterate through the items in the combo, voting the on the key state
		// If any of the pressed keys do not match, fail the macro
		if ( comboLength != 0 )
		{
			overallVote |= Trigger_overallVote( macro, record, long_trigger_macro, pos );
		}

		// If this is a sequence, and have processed at least one vote already
		// then we need to keep track of releases
		// TODO - Different for state scheduling (only for generic triggers)
		if ( pos != 0 )
		{
			overallVote |= Trigger_overallVote( macro, record, long_trigger_macro, record->prevPos );
		}

		// If no pass vote was found after scanning the entire combo
		// And this is the first position in the combo, just remove it (nothing important happened)
		if ( overallVote & TriggerMacroVote_DoNothingRelease && pos == 0 )
		{
			overallVote |= TriggerMacroVote_Fail;
		}

		// Vote Debug
		switch ( voteDebugMode )
		{
		case 1:
			Trigger_showTriggerMacroVote( overallVote, long_trigger_macro );
			print(" TriggerMacroList[");
			printInt16( triggerMacroIndex );
			print("]");
			break;
		}

		// Decide new state of macro after voting
		// Fail macro, remove from pending list
		if ( overallVote & TriggerMacroVote_Fail )
		{
			return TriggerMacroEval_Remove;
		}
		// Do nothing, incorrect key is being held or released
		else if ( overallVote & TriggerMacroVote_DoNothing )
		{
			record->state = TriggerMacro_Waiting;

			// Just doing nothing :)
		}
		// If ready for release state but we get both release and the next press at the same time
		// This is unlikely, but possible
		else if ( ( overallVote & TriggerMacroVote_PassRelease ) == TriggerMacroVote_PassRelease )
		{
			record->state = TriggerMacro_PressRelease;

			// If this is the last combo in the sequence, trigger result
			if ( macro->guide[ pos + comboLength + 1 ] == 0 )
			{
				return TriggerMacroEval_DoResultAndRemove;
			}
		}
		// If ready for transition and in Press state, increment combo position
		else if ( overallVote & TriggerMacroVote_Release && record->state & TriggerMacro_Press )
		{
			record->state = TriggerMacro_Release;

			// If this is the last combo in the sequence, trigger result
			// Or, the final release of a sequence
			if ( comboLength == 0 || macro->guide[ pos + comboLength + 1 ] == 0 )
			{
				return TriggerMacroEval_DoResultAndRemove;
			}
		}
		// If passing and in Waiting state, set macro state to Press
		else if ( overallVote & TriggerMacroVote_Pass
			&& ( record->state == TriggerMacro_Waiting || record->state & TriggerMacro_Press ) )
		{
			record->state = TriggerMacro_Press;

			// If this is the last combo in the sequence, trigger result
			if ( macro->guide[ pos + comboLength + 1 ] == 0 )
			{
				return TriggerMacroEval_DoResultAndRemove;
			}
		}
	}
	// Short Macro
	else
	{
		// Current Macro position
		var_uint_t pos = record->pos;

		// Iterate through the items in the combo, voting the on the key state
		// If any of the pressed keys do not match, fail the macro
		TriggerMacroVote overallVote = Trigger_overallVote( macro, record, long_trigger_macro, pos );

		// Vote Debug
		switch ( voteDebugMode )
		{
		case 1:
			Trigger_showTriggerMacroVote( overallVote, long_trigger_macro );
			print(" TriggerMacroList[");
			printInt16( triggerMacroIndex );
			print("]");
			break;
		}

		// Decide new state of macro after voting
		// Fail macro, remove from pending list
		if ( overallVote & TriggerMacroVote_Fail )
		{
			return TriggerMacroEval_Remove;
		}
		// If passing and in Waiting state, set macro state to Press
		// And trigger result
		else if ( overallVote & TriggerMacroVote_Pass
			&& ( record->state == TriggerMacro_Waiting || record->state == TriggerMacro_Press )
		)
		{
			record->state = TriggerMacro_Press;
			print("RIGHTHERE");

			// Long result macro (more than 1 combo)
			// TODO - If a 1 combo, and 1 element is state-scheduled, also considered a long result macro
			if ( Trigger_isLongResultMacro( &ResultMacroList[ macro->result ] ) )
			{
				print("-FORME");
				// Only ever trigger result once, on press
				if ( overallVote == TriggerMacroVote_Pass )
				{
					return TriggerMacroEval_DoResultAndRemove;
				}
			}
			// State Scheduled Trigger
			else if ( Trigger_isStateScheduled( (TriggerGuide*)&macro->guide[1] ) )
			{
				print("YSSSSS");
				// Only ever trigger result once, on press
				if ( overallVote == TriggerMacroVote_PassRelease )
				{
					print("MEEE");
					return TriggerMacroEval_DoResultAndRemove;
				}
			}
			// State Scheduled Result
			// TODO
			// Short result macro
			else
			{
				print("FOR YOU");
				// Trigger result continuously
				return TriggerMacroEval_DoResult;
			}
		}
		// Otherwise, just remove the macro on key release
		else if ( overallVote & TriggerMacroVote_Release )
		{
			print("DITSSS");
			// Long result macro (more than 1 combo) are ignored (only on press)
			if ( !Trigger_isLongResultMacro( &ResultMacroList[ macro->result ] ) )
			{
				record->state = TriggerMacro_Release;

				return TriggerMacroEval_DoResultAndRemove;
			}
		}

		// This is a short macro, just remove it
		// The state can be rebuilt on the next iteration
		return TriggerMacroEval_Remove;
	}

	return TriggerMacroEval_DoNothing;
}


// Update pending trigger list
void Trigger_updateTriggerMacroPendingList()
{
	// Iterate over the macroTriggerEventBuffer to add any new Trigger Macros to the pending list
	for ( var_uint_t key = 0; key < macroTriggerEventBufferSize; key++ )
	{
		TriggerEvent *event = &macroTriggerEventBuffer[ key ];

		// If this is a release case, indicate to layer lookup for possible latch expiry
		uint8_t latch_expire = event->state == ScheduleType_R;

		// Lookup Trigger List
		LayerTrigger lookup = Layer_layerLookup( event, latch_expire );

		// If there was an error during lookup, skip
		if ( lookup.trigger_list == 0 )
			continue;

		// Number of Triggers in list
		nat_ptr_t triggerListSize = lookup.trigger_list[0];

		// Iterate over triggerList to see if any TriggerMacros need to be added
		// First item is the number of items in the TriggerList
		for ( var_uint_t macro = 1; macro < triggerListSize + 1; macro++ )
		{
			// Lookup trigger macro index
			var_uint_t triggerMacroIndex = lookup.trigger_list[ macro ];

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
				TriggerMacroRecordList[ triggerMacroIndex ].pos            = 0;
				TriggerMacroRecordList[ triggerMacroIndex ].prevPos        = 0;
				TriggerMacroRecordList[ triggerMacroIndex ].state          = TriggerMacro_Waiting;
				TriggerMacroRecordList[ triggerMacroIndex ].last_pos_event = Time_now();
				TriggerMacroRecordList[ triggerMacroIndex ].layer          = lookup.layer;
			}
		}
	}
}


// Determines whether or not a scancode is used on a trigger
// index -> index within trigger list
uint8_t Trigger_DetermineScanCodeOnTrigger( const Layer *layer, uint8_t index )
{
	// Check all triggers
	for ( uint8_t trigger = 1; trigger <= layer->triggerMap[index][0]; trigger++ )
	{
		// Trigger element
		nat_ptr_t elem = layer->triggerMap[index][trigger];

		// Lookup trigger type
		const uint8_t *pos = TriggerMacroList[elem].guide;

		// If there are no elements, ignore
		if ( pos[0] == 0 )
		{
			continue;
		}

		// Only look at first type, no need to go further
		// XXX (HaaTa) This may cause bugs, but it's not as likely
		//             It's also much faster to only check the first type
		switch ( pos[1] )
		{
		case TriggerType_Switch1:
		case TriggerType_Switch2:
		case TriggerType_Switch3:
		case TriggerType_Switch4:
		case TriggerType_Analog1:
		case TriggerType_Analog2:
		case TriggerType_Analog3:
		case TriggerType_Analog4:
			return 1;
		default:
			break;
		}
	}

	// No triggers
	return 0;
}


void Trigger_setup()
{
	// Initialize TriggerMacro states
	for ( var_uint_t macro = 0; macro < TriggerMacroNum; macro++ )
	{
		TriggerMacroRecordList[ macro ].pos            = 0;
		TriggerMacroRecordList[ macro ].prevPos        = 0;
		TriggerMacroRecordList[ macro ].state          = TriggerMacro_Waiting;
		TriggerMacroRecordList[ macro ].last_pos_event = Time_init();
		TriggerMacroRecordList[ macro ].layer          = 0;
	}
}


void Trigger_process()
{
	// Update pending trigger list, before processing TriggerMacros
	Trigger_updateTriggerMacroPendingList();

	// Tail pointer for macroTriggerMacroPendingList
	// Macros must be explicitly re-added
	var_uint_t macroTriggerMacroPendingListTail = 0;

	// Display trigger information before processing
	if ( triggerPendingDebugMode )
	{
		print("\033[1;30mTPe\033[0m");
		for ( var_uint_t macro = 0; macro < macroTriggerMacroPendingListSize; macro++ )
		{
			print(" ");
			printInt8( macroTriggerMacroPendingList[ macro ] );
		}
		print(NL);
	}

	// Iterate through the pending TriggerMacros, processing each of them
	for ( var_uint_t macro = 0; macro < macroTriggerMacroPendingListSize; macro++ )
	{
		index_uint_t cur_macro = macroTriggerMacroPendingList[ macro ];
		switch ( Trigger_evalTriggerMacro( cur_macro ) )
		{
		// Trigger Result Macro (purposely falling through)
		case TriggerMacroEval_DoResult:
			if ( voteDebugMode )
			{
				print(" DR");
			}
			// Append ResultMacro to PendingList
			Result_appendResultMacroToPendingList(
				&TriggerMacroList[ cur_macro ],
				&TriggerMacroRecordList[ cur_macro ],
				ResultMacroAction_Tracking
			);

		default:
			if ( voteDebugMode )
			{
				print(" _" NL);
			}
			macroTriggerMacroPendingList[ macroTriggerMacroPendingListTail++ ] = cur_macro;
			break;

		// Trigger Result Macro and Remove (purposely falling through)
		case TriggerMacroEval_DoResultAndRemove:
			if ( voteDebugMode )
			{
				print(" DRaR");
			}
			// Append ResultMacro to PendingList
			Result_appendResultMacroToPendingList(
				&TriggerMacroList[ cur_macro ],
				&TriggerMacroRecordList[ cur_macro ],
				ResultMacroAction_OneShot
			);

		// Remove Macro from Pending List, nothing to do, removing by default
		case TriggerMacroEval_Remove:
			if ( voteDebugMode )
			{
				print(" R" NL);
			}
			break;
		}
	}

	// Update the macroTriggerMacroPendingListSize with the tail pointer
	macroTriggerMacroPendingListSize = macroTriggerMacroPendingListTail;
}

