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
#include <led.h>
#include <print.h>

// Local Includes
#include "result.h"
#include "kll.h"



// ----- Enums -----

typedef enum ResultMacroEval {
	ResultMacroEval_DoNothing,
	ResultMacroEval_Remove,
} ResultMacroEval;




// ----- KLL Generated Variables -----

extern const Capability CapabilitiesList[];

extern const ResultMacro ResultMacroList[];
extern ResultMacroRecord ResultMacroRecordList[];



// ----- Variables -----

// Pending Result Macro Index List
//  * Any result macro that needs processing from a previous macro processing loop
ResultsPending macroResultMacroPendingList;



// ----- Functions -----

// Evaluate/Update ResultMacro
ResultMacroEval Macro_evalResultMacro( ResultPendingElem resultElem )
{
	// Lookup ResultMacro
	const ResultMacro *macro = &ResultMacroList[ resultElem.index ];
	ResultMacroRecord *record = &ResultMacroRecordList[ resultElem.index ];

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
		void (*capability)(TriggerMacro*, uint8_t, uint8_t, uint8_t*) = \
			(void(*)(TriggerMacro*, uint8_t, uint8_t, uint8_t*))(CapabilitiesList[ guide->index ].func);

		// Call capability
		capability( resultElem.trigger, record->state, record->stateType, &guide->args );

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


void Result_add( uint32_t index )
{
}


void Result_setup()
{
	// Initialize macroResultMacroPendingList
	macroResultMacroPendingList.size = 0;

	// Initialize ResultMacro states
	for ( var_uint_t macro = 0; macro < ResultMacroNum; macro++ )
	{
		ResultMacroRecordList[ macro ].pos       = 0;
		ResultMacroRecordList[ macro ].state     = 0;
		ResultMacroRecordList[ macro ].stateType = 0;
	}
}


void Result_process()
{
	// Tail pointer for macroResultMacroPendingList
	// Macros must be explicitly re-added
	index_uint_t macroResultMacroPendingListTail = 0;

	// Iterate through the pending ResultMacros, processing each of them
	for ( index_uint_t macro = 0; macro < macroResultMacroPendingList.size; macro++ )
	{
		switch ( Macro_evalResultMacro( macroResultMacroPendingList.data[ macro ] ) )
		{
		// Re-add macros to pending list
		case ResultMacroEval_DoNothing:
		default:
			memcpy( &macroResultMacroPendingList.data[ macroResultMacroPendingListTail++ ],
				&macroResultMacroPendingList.data[ macro ],
				sizeof( ResultPendingElem )
			);
			break;

		// Remove Macro from Pending List, nothing to do, removing by default
		case ResultMacroEval_Remove:
			break;
		}
	}

	// Update the macroResultMacroPendingListSize with the tail pointer
	macroResultMacroPendingList.size = macroResultMacroPendingListTail;
}

