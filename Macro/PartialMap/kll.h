/* Copyright (C) 2014 by Jacob Alexander
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

#ifndef __kll_h
#define __kll_h

// ----- Includes -----

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
// TODO Make selectable from layout variable
//typedef uint32_t var_uint_t;
//typedef uint16_t var_uint_t;
typedef uint8_t  var_uint_t;

// - NOTE -
// Native pointer length
// This needs to be defined per microcontroller
// e.g. mk20s  -> 32 bit
//      atmega -> 16 bit
typedef uint32_t nat_ptr_t;
//typedef uint16_t nat_ptr_t;



// ----- Structs -----

// -- Result Macro
// Defines the sequence of combinations to as the Result of Trigger Macro
//
// Capability + args per USB send
// Default Args (always sent): key state/analog of last key
// Combo Length of 0 signifies end of sequence
//
// ResultMacro.guide     -> [<combo length>|<capability index>|<arg1>|<argn>|<capability index>|...|<combo length>|...|0]
// ResultMacro.pos       -> <current combo position>
// ResultMacro.state     -> <last key state>
// ResultMacro.stateType -> <last key state type>

// ResultMacro struct, one is created per ResultMacro, no duplicates
typedef struct ResultMacro {
	const uint8_t *guide;
	var_uint_t pos;
	uint8_t  state;
	uint8_t  stateType;
} ResultMacro;

// Guide, key element
#define ResultGuideSize( guidePtr ) sizeof( ResultGuide ) - 1 + CapabilitiesList[ (guidePtr)->index ].argCount
typedef struct ResultGuide {
	uint8_t index;
	uint8_t args; // This is used as an array pointer (but for packing purposes, must be 8 bit)
} ResultGuide;



// -- Trigger Macro
// Defines the sequence of combinations to Trigger a Result Macro
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
// TriggerMacro.pos    -> <current combo position>
// TriggerMacro.state  -> <status of the macro pos>

// TriggerMacro states
typedef enum TriggerMacroState {
	TriggerMacro_Press,   // Combo in sequence is passing
	TriggerMacro_Release, // Move to next combo in sequence (or finish if at end of sequence)
	TriggerMacro_Waiting, // Awaiting user input
} TriggerMacroState;

// TriggerMacro struct, one is created per TriggerMacro, no duplicates
typedef struct TriggerMacro {
	const uint8_t *guide;
	var_uint_t result;
	var_uint_t pos;
	TriggerMacroState state;
} TriggerMacro;

// Guide, key element
#define TriggerGuideSize sizeof( TriggerGuide )
typedef struct TriggerGuide {
	uint8_t type;
	uint8_t state;
	uint8_t scanCode;
} TriggerGuide;



// ----- Capabilities -----

// Capability
typedef struct Capability {
	void *func;
	uint8_t argCount;
} Capability;

// Total Number of Capabilities
#define CapabilitiesNum sizeof( CapabilitiesList ) / sizeof( Capability )


// -- Result Macros

// Guide_RM / Define_RM Pair
// Guide_RM( index ) = result;
//  * index  - Result Macro index number
//  * result - Result Macro guide (see ResultMacro)
// Define_RM( index );
//  * index  - Result Macro index number
//  Must be used after Guide_RM
#define Guide_RM( index ) const uint8_t rm##index##_guide[]
#define Define_RM( index ) { rm##index##_guide, 0, 0, 0 }


// -- Result Macro List

// Total number of result macros (rm's)
// Used to create pending rm's table
#define ResultMacroNum sizeof( ResultMacroList ) / sizeof( ResultMacro )


// -- Trigger Macros

// Guide_TM / Define_TM Trigger Setup
// Guide_TM( index ) = trigger;
//  * index   - Trigger Macro index number
//  * trigger - Trigger Macro guide (see TriggerMacro)
// Define_TM( index, result );
//  * index   - Trigger Macro index number
//  * result  - Result Macro index number which is triggered by this Trigger Macro
#define Guide_TM( index ) const uint8_t tm##index##_guide[]
#define Define_TM( index, result ) { tm##index##_guide, result, 0, TriggerMacro_Waiting }


// -- Trigger Macro List

// Total number of trigger macros (tm's)
// Used to create pending tm's table
#define TriggerMacroNum sizeof( TriggerMacroList ) / sizeof( TriggerMacro )



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
//
// Except for Off, all states an exist simultaneously for each layer
// For example:
// state -> 0x04 + 0x01 = 0x05 (Shift + Lock), which is effectively Off (0x00)
//
// Max defines the maximum number of keys in the map, maximum of 0xFF
//  - Compiler calculates this
//
// The name is defined for cli debugging purposes (Null terminated string)

typedef struct Layer {
	const nat_ptr_t **triggerMap;
	const char *name;
	const uint8_t max;
	uint8_t state;
} Layer;


// Layer_IN( map, name );
//  * map  - Trigger map
//  * name - Name of the trigger map
#define Layer_IN( map, name ) { map, name, sizeof( map ) / 4 - 1, 0 }

// Total number of layers
#define LayerNum sizeof( LayerIndex ) / sizeof( Layer )



#endif // __kll_h

