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

#pragma once

// ----- Includes -----

// KLL Generated Defines
#include <kll_defs.h>

// Project Includes
#include <print.h>
#include <scan_loop.h>
#include <macro.h>
#include <output_com.h>

// USB HID Keymap list
#include <usb_hid.h>



// ----- Types -----

// - NOTE -
// It is possible to change the maximum state and indexing positions of the state machine.
// This usually affects the SRAM usage quite a bit, so it can be used to fit the code on smaller uCs
// Or to allow for nearly infinite states.
#if StateWordSize_define == 32
typedef uint32_t var_uint_t;
#elif StateWordSize_define == 16
typedef uint16_t var_uint_t;
#elif StateWordSize_define == 8
typedef uint8_t  var_uint_t;
#else
#error "Invalid StateWordSize, possible values: 32, 16 and 8."
#endif

// - NOTE -
// It is possible to change the maximum number of trigger/result index sizes
// This will affect SRAM and flash usage, so it can be used to fit code on smaller uCs.
// Also allows for over 4 billion triggers and results (triggers and results have separate indices)
#if IndexWordSize_define == 32
typedef uint32_t index_uint_t;
#elif IndexWordSize_define == 16
typedef uint16_t index_uint_t;
#elif IndexWordSize_define == 8
typedef uint8_t index_uint_t;
#else
#error "Invalid IndexWordSize, possible values: 32, 16 and 8."
#endif

// - NOTE -
// Native pointer length
// This needs to be defined per microcontroller
// e.g. mk20s  -> 32 bit
//      atmega -> 16 bit
// Default to whatever is detected
#if defined(_mk20dx128_) || defined(_mk20dx128vlf5_) || defined(_mk20dx256_) || defined(_mk20dx256vlh7_) // ARM
typedef uint32_t nat_ptr_t;
#elif defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
typedef uint16_t nat_ptr_t;
#else
typedef uintptr_t nat_ptr_t;
#endif



// ----- Structs -----

// -- Scheduling

typedef enum ScheduleState {
	ScheduleType_P,   // Press
	ScheduleType_H,   // Hold
	ScheduleType_R,   // Release
	ScheduleType_O,   // Off
	ScheduleType_UP,  // Unique Press
	ScheduleType_UR,  // Unique Release

	ScheduleType_A,   // Activate
	ScheduleType_D,   // Deactivate
	ScheduleType_On,  // On
	ScheduleType_Off, // Off
} ScheduleState;

// TODO - Do we need 64-bits for time here? It's the easiest to implement -HaaTa
typedef struct Schedule {
	uint32_t systick;   // ms systick
	uint32_t cycletick; // Usually in the ns range, e.g. 13.889 ns per tick
	union {
		ScheduleState state;
		uint8_t analog;
	};
} Schedule;



// -- Result Macro
// Defines the sequence of combinations to as the Result of Trigger Macro
// For RAM optimization reasons, ResultMacro has been split into ResultMacro and ResultMacroRecord structures
//
// Capability + args per USB send
// Default Args (always sent): key state/analog of last key
// Combo Length of 0 signifies end of sequence
//
// ResultMacro.guide -> [<combo length>|<capability index>|<arg1>|<argn>|<capability index>|...|<combo length>|...|0]
//
// ResultMacroRecord.pos       -> <current combo position>
// ResultMacroRecord.state     -> <last key state>
// ResultMacroRecord.stateType -> <last key state type>

// ResultMacro struct, one is created per ResultMacro, no duplicates
typedef struct ResultMacro {
	const uint8_t *guide;
} ResultMacro;

typedef struct ResultMacroRecord {
	var_uint_t pos;
	uint8_t  state;
	uint8_t  stateType;
} ResultMacroRecord;

// Guide, key element
#define ResultGuideSize( guidePtr ) sizeof( ResultGuide ) - 1 + CapabilitiesList[ (guidePtr)->index ].argCount
typedef struct ResultGuide {
	uint8_t index;
	uint8_t args; // This is used as an array pointer (but for packing purposes, must be 8 bit)
} ResultGuide;



// -- Trigger Macro
// Defines the sequence of combinations to Trigger a Result Macro
// For RAM optimization reasons TriggerMacro has been split into TriggerMacro and TriggerMacroRecord
// Key Types:
//   * 0x00 Normal (Press/Hold/Release)
//   * 0x01 LED State (On/Off)
//   * 0x02 Analog (Threshold)
//   * 0x03-0xFE Reserved
//   * 0xFF Debug State
//
// Key State:
//   * Off                - 0x00 (all flag states)
//   * On                 - 0x01
//   * Press/Hold/Release - 0x01/0x02/0x03
//   * Threshold (Range)  - 0x01 (Released), 0x10 (Light press), 0xFF (Max press)
//   * Debug              - 0xFF (Print capability name)
//
// Combo Length of 0 signifies end of sequence
//
// TriggerMacro.guide  -> [<combo length>|<key1 type>|<key1 state>|<key1>...<keyn type>|<keyn state>|<keyn>|<combo length>...|0]
// TriggerMacro.result -> <index to result macro>
//
// TriggerMacroRecord.pos   -> <current combo position>
// TriggerMacroRecord.state -> <status of the macro pos>

// TriggerMacro states
typedef enum TriggerMacroState {
	TriggerMacro_Press,   // Combo in sequence is passing
	TriggerMacro_Release, // Move to next combo in sequence (or finish if at end of sequence)
	TriggerMacro_Waiting, // Awaiting user input
} TriggerMacroState;

// TriggerMacro struct, one is created per TriggerMacro, no duplicates
typedef struct TriggerMacro {
	const uint8_t *guide;
	const var_uint_t result;
} TriggerMacro;

typedef struct TriggerMacroRecord {
	var_uint_t pos;
	TriggerMacroState state;
} TriggerMacroRecord;

// Guide, key element
// Used for storing Trigger elements
#define TriggerGuideSize sizeof( TriggerGuide )
typedef struct TriggerGuide {
	uint8_t type;
	uint8_t state;
	uint8_t scanCode;
} TriggerGuide;

// Same as a TriggerGuide, but is used for incoming events rather than event comparisons
typedef struct TriggerBuffer {
	uint8_t type;
	uint8_t state;
	uint8_t scanCode;
} TriggerBuffer;



// -- List Structs

// Result pending list struct
typedef struct ResultPendingElem {
	TriggerMacro *trigger;
	index_uint_t  index;
} ResultPendingElem;

// Results Pending - Ring-buffer definition
typedef struct ResultsPending {
	ResultPendingElem data[ ResultMacroNum_KLL ];
	index_uint_t      size;
} ResultsPending;



// ----- Capabilities -----

// Capability
typedef struct Capability {
	const void *func;
	const uint8_t argCount;
} Capability;

// Total Number of Capabilities
// (generated by KLL)
#define CapabilitiesNum CapabilitiesNum_KLL


// -- Result Macros

// Guide_RM / Define_RM Pair
// Guide_RM( index ) = result;
//  * index  - Result Macro index number
//  * result - Result Macro guide (see ResultMacro)
// Define_RM( index );
//  * index  - Result Macro index number
//  Must be used after Guide_RM
#define Guide_RM( index ) const uint8_t rm##index##_guide[]
#define Define_RM( index ) { rm##index##_guide }


// -- Result Macro List

// Total number of result macros (rm's)
// Used to create pending rm's table
// (generated by KLL)
#define ResultMacroNum ResultMacroNum_KLL


// -- Trigger Macros

// Guide_TM / Define_TM Trigger Setup
// Guide_TM( index ) = trigger;
//  * index   - Trigger Macro index number
//  * trigger - Trigger Macro guide (see TriggerMacro)
// Define_TM( index, result );
//  * index   - Trigger Macro index number
//  * result  - Result Macro index number which is triggered by this Trigger Macro
#define Guide_TM( index ) const uint8_t tm##index##_guide[]
#define Define_TM( index, result ) { tm##index##_guide, result }


// -- Trigger Macro List

// Total number of trigger macros (tm's)
// Used to create pending tm's table
// (generated by KLL)
#define TriggerMacroNum TriggerMacroNum_KLL



// ----- Trigger Maps -----

// Define_TL( layer, scanCode ) = triggerList;
//  * layer       - basename of the layer
//  * scanCode    - Hex value of the scanCode
//  * triggerList - Trigger List (see Trigger Lists)
#define Define_TL( layer, scanCode ) const nat_ptr_t layer##_tl_##scanCode[]



// ----- Layer Index -----

// Defines each map of trigger macro lists
// Layer 0 is always the default map
// Layer States:
//   * Off   - 0x00
//   * Shift - 0x01
//   * Latch - 0x02
//   * Lock  - 0x04
// Layer states are stored in the LayerState array
//
// Except for Off, all states an exist simultaneously for each layer
// For example:
// state -> 0x04 + 0x01 = 0x05 (Shift + Lock), which is effectively Off (0x00)
//
// First defines the first used scan code (most keyboards start at 0, some start higher e.g. 0x40)
//  - Compiler calculates this
//
// Last defines the last scan code used (helps reduce RAM usage)
//
// The name is defined for cli debugging purposes (Null terminated string)

typedef struct Layer {
	const nat_ptr_t **triggerMap;
	const char *name;
	const uint8_t first;
	const uint8_t last;
} Layer;

// Layer_IN( map, name, first );
//  * map   - Trigger map
//  * name  - Name of the trigger map
//  * first - First scan code used (most keyboards start at 0, some start higher e.g. 0x40)
#define Layer_IN( map, name, first ) { map, name, first, sizeof( map ) / sizeof( nat_ptr_t ) - 1 + first }

// Total number of layers (generated by KLL)
#define LayerNum LayerNum_KLL



// ----- Key Positions -----

// Each positions has 6 dimensions
// Units are in mm
#define PositionEntry( x, y, z, rx, ry, rz ) \
	{ x, y, z, rx, ry, rz }
typedef struct Position {
	float x;
	float y;
	float z;
	float rx;
	float ry;
	float rz;
} Position;

