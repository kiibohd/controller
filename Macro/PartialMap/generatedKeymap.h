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

// Generated MSG /w timestamp and compiler information

#ifndef __generatedKeymap_h
#define __generatedKeymap_h

// ----- Includes -----

// Project Includes
#include <print.h>
#include <scan_loop.h>
#include <macro.h>
#include <output_com.h>

// USB HID Keymap list
#include <usb_hid.h>



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
	unsigned int pos;
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
	unsigned int result;
	unsigned int pos;
	TriggerMacroState state;
} TriggerMacro;

// Guide, key element
#define TriggerGuideSize sizeof( TriggerGuide )
typedef struct TriggerGuide {
	uint8_t type;
	uint8_t state;
	uint8_t scanCode;
} TriggerGuide;



// ----- Macros -----

void debugPrint_capability( uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("debugPrint(arg)");
		return;
	}

	dbug_msg("Capability Print: ");
	print(" statetype( ");
	printHex( stateType );
	print(" )  state ( ");
	printHex( state );
	print(" )  arg ( ");
	printHex( args[0] );
	print(" )");
}

void debugPrint2_capability( uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("debugPrint2(arg1,arg2)");
		return;
	}

	dbug_msg("Capability Print: ");
	print(" statetype( ");
	printHex( stateType );
	print(" )  state ( ");
	printHex( state );
	print(" )  arg1 ( ");
	printHex( args[0] );
	print(" )  arg2 ( ");
	printHex( args[1] );
	print(" )");
}

// Capability
typedef struct Capability {
	void *func;
	uint8_t argCount;
} Capability;

// Total Number of Capabilities
#define CapabilitiesNum sizeof( CapabilitiesList ) / sizeof( Capability )

// Indexed Capabilities Table
// TODO Generated from .kll files in each module
const Capability CapabilitiesList[] = {
	{ debugPrint_capability, 1 },
	{ debugPrint2_capability, 2 },
	{ Macro_layerStateToggle_capability, sizeof(unsigned int) + 1 },
	{ Output_usbCodeSend_capability, 1 },
};


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

Guide_RM( 0 ) = { 1, 0, 0xDA, 0 };
Guide_RM( 1 ) = { 1, 0, 0xBE, 1, 0, 0xEF, 0 };
Guide_RM( 2 ) = { 2, 0, 0xFA, 0, 0xAD, 0 };
Guide_RM( 3 ) = { 1, 1, 0xCA, 0xFE, 0 };
Guide_RM( 4 ) = { 1, 0, 0xDA, 0 };


// -- Result Macro List

// Total number of result macros (rm's)
// Used to create pending rm's table
#define ResultMacroNum sizeof( ResultMacroList ) / sizeof( ResultMacro )

// Indexed Table of Result Macros
ResultMacro ResultMacroList[] = {
	Define_RM( 0 ),
	Define_RM( 1 ),
	Define_RM( 2 ),
	Define_RM( 3 ),
	Define_RM( 4 ),
};


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

Guide_TM( 0 ) = { 1, 0x00, 0x01, 0x73, 0 };
Guide_TM( 1 ) = { 1, 0x00, 0x01, 0x73, 1, 0x00, 0x01, 0x75, 0 };
Guide_TM( 2 ) = { 2, 0x00, 0x01, 0x73, 0x00, 0x01, 0x74, 0 };
Guide_TM( 3 ) = { 1, 0x00, 0x01, 0x76, 0 };
Guide_TM( 4 ) = { 1, 0x00, 0x01, 0x77, 0 };
Guide_TM( 5 ) = { 1, 0x00, 0x01, 0x2E, 0 };
Guide_TM( 6 ) = { 1, 0x00, 0x01, 0x2D, 0 };
Guide_TM( 7 ) = { 1, 0x00, 0x01, 0x2C, 0 };
Guide_TM( 8 ) = { 2, 0x00, 0x01, 0x20, 0x00, 0x01, 0x21, 0 };
Guide_TM( 9 ) = { 1, 0x00, 0x01, 0x20, 1, 0x00, 0x01, 0x22, 0 };
Guide_TM( 10 ) = { 1, 0x00, 0x01, 0x2B, 0 };


// -- Trigger Macro List

// Total number of trigger macros (tm's)
// Used to create pending tm's table
#define TriggerMacroNum sizeof( TriggerMacroList ) / sizeof( TriggerMacro )

// Indexed Table of Trigger Macros
TriggerMacro TriggerMacroList[] = {
	Define_TM( 0, 0 ),
	Define_TM( 1, 1 ),
	Define_TM( 2, 2 ),
	Define_TM( 3, 3 ),
	Define_TM( 4, 0 ),
	Define_TM( 5, 0 ),
	Define_TM( 6, 1 ),
	Define_TM( 7, 2 ),
	Define_TM( 8, 0 ),
	Define_TM( 9, 0 ), // TODO
	Define_TM( 10, 4 ),
};



// ----- Trigger Maps -----

// MaxScanCode
// - This is retrieved from the KLL configuration
// - Should be corollated with the max scan code in the scan module
// - Maximum value is 0x100 (0x0 to 0xFF)
// - Increasing it beyond the keyboard's capabilities is just a waste of ram...
#define MaxScanCode 0x100

// Define_TL( layer, scanCode ) = triggerList;
//  * layer       - basename of the layer
//  * scanCode    - Hex value of the scanCode
//  * triggerList - Trigger List (see Trigger Lists)
#define Define_TL( layer, scanCode ) const unsigned int layer##_tl_##scanCode[]

// -- Trigger Lists
//
// Index 0: # of triggers in list
// Index n: pointer to trigger macro - use tm() macro

// Default Layer
Define_TL( default, 0x00 ) = { 0 };
Define_TL( default, 0x01 ) = { 0 };
Define_TL( default, 0x02 ) = { 0 };
Define_TL( default, 0x03 ) = { 0 };
Define_TL( default, 0x04 ) = { 0 };
Define_TL( default, 0x05 ) = { 0 };
Define_TL( default, 0x06 ) = { 0 };
Define_TL( default, 0x07 ) = { 0 };
Define_TL( default, 0x08 ) = { 0 };
Define_TL( default, 0x09 ) = { 0 };
Define_TL( default, 0x0A ) = { 0 };
Define_TL( default, 0x0B ) = { 0 };
Define_TL( default, 0x0C ) = { 0 };
Define_TL( default, 0x0D ) = { 0 };
Define_TL( default, 0x0E ) = { 0 };
Define_TL( default, 0x0F ) = { 0 };
Define_TL( default, 0x10 ) = { 0 };
Define_TL( default, 0x11 ) = { 0 };
Define_TL( default, 0x12 ) = { 0 };
Define_TL( default, 0x13 ) = { 0 };
Define_TL( default, 0x14 ) = { 0 };
Define_TL( default, 0x15 ) = { 0 };
Define_TL( default, 0x16 ) = { 0 };
Define_TL( default, 0x17 ) = { 0 };
Define_TL( default, 0x18 ) = { 0 };
Define_TL( default, 0x19 ) = { 0 };
Define_TL( default, 0x1A ) = { 0 };
Define_TL( default, 0x1B ) = { 0 };
Define_TL( default, 0x1C ) = { 0 };
Define_TL( default, 0x1D ) = { 0 };
Define_TL( default, 0x1E ) = { 0 };
Define_TL( default, 0x1F ) = { 0 };
Define_TL( default, 0x20 ) = { 1, 8 };
Define_TL( default, 0x21 ) = { 1, 8 };
Define_TL( default, 0x22 ) = { 0 };
Define_TL( default, 0x23 ) = { 0 };
Define_TL( default, 0x24 ) = { 0 };
Define_TL( default, 0x25 ) = { 0 };
Define_TL( default, 0x26 ) = { 0 };
Define_TL( default, 0x27 ) = { 0 };
Define_TL( default, 0x28 ) = { 0 };
Define_TL( default, 0x29 ) = { 0 };
Define_TL( default, 0x2A ) = { 0 };
Define_TL( default, 0x2B ) = { 1, 10 };
Define_TL( default, 0x2C ) = { 1, 7 };
Define_TL( default, 0x2D ) = { 1, 6 };
Define_TL( default, 0x2E ) = { 1, 5 };
Define_TL( default, 0x2F ) = { 0 };
Define_TL( default, 0x30 ) = { 0 };
Define_TL( default, 0x31 ) = { 0 };
Define_TL( default, 0x32 ) = { 0 };
Define_TL( default, 0x33 ) = { 0 };
Define_TL( default, 0x34 ) = { 0 };
Define_TL( default, 0x35 ) = { 0 };
Define_TL( default, 0x36 ) = { 0 };
Define_TL( default, 0x37 ) = { 0 };
Define_TL( default, 0x38 ) = { 0 };
Define_TL( default, 0x39 ) = { 0 };
Define_TL( default, 0x3A ) = { 0 };
Define_TL( default, 0x3B ) = { 0 };
Define_TL( default, 0x3C ) = { 0 };
Define_TL( default, 0x3D ) = { 0 };
Define_TL( default, 0x3E ) = { 0 };
Define_TL( default, 0x3F ) = { 0 };
Define_TL( default, 0x40 ) = { 0 };
Define_TL( default, 0x41 ) = { 0 };
Define_TL( default, 0x42 ) = { 0 };
Define_TL( default, 0x43 ) = { 0 };
Define_TL( default, 0x44 ) = { 0 };
Define_TL( default, 0x45 ) = { 0 };
Define_TL( default, 0x46 ) = { 0 };
Define_TL( default, 0x47 ) = { 0 };
Define_TL( default, 0x48 ) = { 0 };
Define_TL( default, 0x49 ) = { 0 };
Define_TL( default, 0x4A ) = { 0 };
Define_TL( default, 0x4B ) = { 0 };
Define_TL( default, 0x4C ) = { 0 };
Define_TL( default, 0x4D ) = { 0 };
Define_TL( default, 0x4E ) = { 0 };
Define_TL( default, 0x4F ) = { 0 };
Define_TL( default, 0x50 ) = { 0 };
Define_TL( default, 0x51 ) = { 0 };
Define_TL( default, 0x52 ) = { 0 };
Define_TL( default, 0x53 ) = { 0 };
Define_TL( default, 0x54 ) = { 0 };
Define_TL( default, 0x55 ) = { 0 };
Define_TL( default, 0x56 ) = { 0 };
Define_TL( default, 0x57 ) = { 0 };
Define_TL( default, 0x58 ) = { 0 };
Define_TL( default, 0x59 ) = { 0 };
Define_TL( default, 0x5A ) = { 0 };
Define_TL( default, 0x5B ) = { 0 };
Define_TL( default, 0x5C ) = { 0 };
Define_TL( default, 0x5D ) = { 0 };
Define_TL( default, 0x5E ) = { 0 };
Define_TL( default, 0x5F ) = { 0 };
Define_TL( default, 0x60 ) = { 0 };
Define_TL( default, 0x61 ) = { 0 };
Define_TL( default, 0x62 ) = { 0 };
Define_TL( default, 0x63 ) = { 0 };
Define_TL( default, 0x64 ) = { 0 };
Define_TL( default, 0x65 ) = { 0 };
Define_TL( default, 0x66 ) = { 0 };
Define_TL( default, 0x67 ) = { 0 };
Define_TL( default, 0x68 ) = { 0 };
Define_TL( default, 0x69 ) = { 0 };
Define_TL( default, 0x6A ) = { 0 };
Define_TL( default, 0x6B ) = { 0 };
Define_TL( default, 0x6C ) = { 0 };
Define_TL( default, 0x6D ) = { 0 };
Define_TL( default, 0x6E ) = { 0 };
Define_TL( default, 0x6F ) = { 0 };
Define_TL( default, 0x70 ) = { 0 };
Define_TL( default, 0x71 ) = { 0 };
Define_TL( default, 0x72 ) = { 0 };
Define_TL( default, 0x73 ) = { 3, 0, 1, 2 };
Define_TL( default, 0x74 ) = { 1, 2 };
Define_TL( default, 0x75 ) = { 1, 1 };
Define_TL( default, 0x76 ) = { 1, 3 };
Define_TL( default, 0x77 ) = { 1, 4 };
Define_TL( default, 0x78 ) = { 0 };
Define_TL( default, 0x79 ) = { 0 };
Define_TL( default, 0x7A ) = { 0 };
Define_TL( default, 0x7B ) = { 0 };
Define_TL( default, 0x7C ) = { 0 };
Define_TL( default, 0x7D ) = { 0 };
Define_TL( default, 0x7E ) = { 0 };
Define_TL( default, 0x7F ) = { 0 };
Define_TL( default, 0x80 ) = { 0 };
Define_TL( default, 0x81 ) = { 0 };
Define_TL( default, 0x82 ) = { 0 };
Define_TL( default, 0x83 ) = { 0 };
Define_TL( default, 0x84 ) = { 0 };
Define_TL( default, 0x85 ) = { 0 };
Define_TL( default, 0x86 ) = { 0 };
Define_TL( default, 0x87 ) = { 0 };
Define_TL( default, 0x88 ) = { 0 };
Define_TL( default, 0x89 ) = { 0 };
Define_TL( default, 0x8A ) = { 0 };
Define_TL( default, 0x8B ) = { 0 };
Define_TL( default, 0x8C ) = { 0 };
Define_TL( default, 0x8D ) = { 0 };
Define_TL( default, 0x8E ) = { 0 };
Define_TL( default, 0x8F ) = { 0 };
Define_TL( default, 0x90 ) = { 0 };
Define_TL( default, 0x91 ) = { 0 };
Define_TL( default, 0x92 ) = { 0 };
Define_TL( default, 0x93 ) = { 0 };
Define_TL( default, 0x94 ) = { 0 };
Define_TL( default, 0x95 ) = { 0 };
Define_TL( default, 0x96 ) = { 0 };
Define_TL( default, 0x97 ) = { 0 };
Define_TL( default, 0x98 ) = { 0 };
Define_TL( default, 0x99 ) = { 0 };
Define_TL( default, 0x9A ) = { 0 };
Define_TL( default, 0x9B ) = { 0 };
Define_TL( default, 0x9C ) = { 0 };
Define_TL( default, 0x9D ) = { 0 };
Define_TL( default, 0x9E ) = { 0 };
Define_TL( default, 0x9F ) = { 0 };
Define_TL( default, 0xA0 ) = { 0 };
Define_TL( default, 0xA1 ) = { 0 };
Define_TL( default, 0xA2 ) = { 0 };
Define_TL( default, 0xA3 ) = { 0 };
Define_TL( default, 0xA4 ) = { 0 };
Define_TL( default, 0xA5 ) = { 0 };
Define_TL( default, 0xA6 ) = { 0 };
Define_TL( default, 0xA7 ) = { 0 };
Define_TL( default, 0xA8 ) = { 0 };
Define_TL( default, 0xA9 ) = { 0 };
Define_TL( default, 0xAA ) = { 0 };
Define_TL( default, 0xAB ) = { 0 };
Define_TL( default, 0xAC ) = { 0 };
Define_TL( default, 0xAD ) = { 0 };
Define_TL( default, 0xAE ) = { 0 };
Define_TL( default, 0xAF ) = { 0 };
Define_TL( default, 0xB0 ) = { 0 };
Define_TL( default, 0xB1 ) = { 0 };
Define_TL( default, 0xB2 ) = { 0 };
Define_TL( default, 0xB3 ) = { 0 };
Define_TL( default, 0xB4 ) = { 0 };
Define_TL( default, 0xB5 ) = { 0 };
Define_TL( default, 0xB6 ) = { 0 };
Define_TL( default, 0xB7 ) = { 0 };
Define_TL( default, 0xB8 ) = { 0 };
Define_TL( default, 0xB9 ) = { 0 };
Define_TL( default, 0xBA ) = { 0 };
Define_TL( default, 0xBB ) = { 0 };
Define_TL( default, 0xBC ) = { 0 };
Define_TL( default, 0xBD ) = { 0 };
Define_TL( default, 0xBE ) = { 0 };
Define_TL( default, 0xBF ) = { 0 };
Define_TL( default, 0xC0 ) = { 0 };
Define_TL( default, 0xC1 ) = { 0 };
Define_TL( default, 0xC2 ) = { 0 };
Define_TL( default, 0xC3 ) = { 0 };
Define_TL( default, 0xC4 ) = { 0 };
Define_TL( default, 0xC5 ) = { 0 };
Define_TL( default, 0xC6 ) = { 0 };
Define_TL( default, 0xC7 ) = { 0 };
Define_TL( default, 0xC8 ) = { 0 };
Define_TL( default, 0xC9 ) = { 0 };
Define_TL( default, 0xCA ) = { 0 };
Define_TL( default, 0xCB ) = { 0 };
Define_TL( default, 0xCC ) = { 0 };
Define_TL( default, 0xCD ) = { 0 };
Define_TL( default, 0xCE ) = { 0 };
Define_TL( default, 0xCF ) = { 0 };
Define_TL( default, 0xD0 ) = { 0 };
Define_TL( default, 0xD1 ) = { 0 };
Define_TL( default, 0xD2 ) = { 0 };
Define_TL( default, 0xD3 ) = { 0 };
Define_TL( default, 0xD4 ) = { 0 };
Define_TL( default, 0xD5 ) = { 0 };
Define_TL( default, 0xD6 ) = { 0 };
Define_TL( default, 0xD7 ) = { 0 };
Define_TL( default, 0xD8 ) = { 0 };
Define_TL( default, 0xD9 ) = { 0 };
Define_TL( default, 0xDA ) = { 0 };
Define_TL( default, 0xDB ) = { 0 };
Define_TL( default, 0xDC ) = { 0 };
Define_TL( default, 0xDD ) = { 0 };
Define_TL( default, 0xDE ) = { 0 };
Define_TL( default, 0xDF ) = { 0 };
Define_TL( default, 0xE0 ) = { 0 };
Define_TL( default, 0xE1 ) = { 0 };
Define_TL( default, 0xE2 ) = { 0 };
Define_TL( default, 0xE3 ) = { 0 };
Define_TL( default, 0xE4 ) = { 0 };
Define_TL( default, 0xE5 ) = { 0 };
Define_TL( default, 0xE6 ) = { 0 };
Define_TL( default, 0xE7 ) = { 0 };
Define_TL( default, 0xE8 ) = { 0 };
Define_TL( default, 0xE9 ) = { 0 };
Define_TL( default, 0xEA ) = { 0 };
Define_TL( default, 0xEB ) = { 0 };
Define_TL( default, 0xEC ) = { 0 };
Define_TL( default, 0xED ) = { 0 };
Define_TL( default, 0xEE ) = { 0 };
Define_TL( default, 0xEF ) = { 0 };
Define_TL( default, 0xF0 ) = { 0 };
Define_TL( default, 0xF1 ) = { 0 };
Define_TL( default, 0xF2 ) = { 0 };
Define_TL( default, 0xF3 ) = { 0 };
Define_TL( default, 0xF4 ) = { 0 };
Define_TL( default, 0xF5 ) = { 0 };
Define_TL( default, 0xF6 ) = { 0 };
Define_TL( default, 0xF7 ) = { 0 };
Define_TL( default, 0xF8 ) = { 0 };
Define_TL( default, 0xF9 ) = { 0 };
Define_TL( default, 0xFA ) = { 0 };
Define_TL( default, 0xFB ) = { 0 };
Define_TL( default, 0xFC ) = { 0 };
Define_TL( default, 0xFD ) = { 0 };
Define_TL( default, 0xFE ) = { 0 };
Define_TL( default, 0xFF ) = { 0 };


// myname Layer
Define_TL( myname, 0x05 ) = { 0 };
Define_TL( myname, 0x06 ) = { 0 };
Define_TL( myname, 0x07 ) = { 0 };


// myname2 Layer
Define_TL( myname2, 0x04 ) = { 0 };
Define_TL( myname2, 0x05 ) = { 0 };
Define_TL( myname2, 0x06 ) = { 0 };


// -- ScanCode Indexed Maps
// Maps to a trigger list of macro pointers
//                 _
// <scan code> -> |T|
//                |r| -> <trigger macro pointer 1>
//                |i|
//                |g| -> <trigger macro pointer 2>
//                |g|
//                |e| -> <trigger macro pointer 3>
//                |r|
//                |s| -> <trigger macro pointer n>
//                 -

// Default Map for ScanCode Lookup
const unsigned int *default_scanMap[] = {
default_tl_0x00, default_tl_0x01, default_tl_0x02, default_tl_0x03, default_tl_0x04, default_tl_0x05, default_tl_0x06, default_tl_0x07, default_tl_0x08, default_tl_0x09, default_tl_0x0A, default_tl_0x0B, default_tl_0x0C, default_tl_0x0D, default_tl_0x0E, default_tl_0x0F, default_tl_0x10, default_tl_0x11, default_tl_0x12, default_tl_0x13, default_tl_0x14, default_tl_0x15, default_tl_0x16, default_tl_0x17, default_tl_0x18, default_tl_0x19, default_tl_0x1A, default_tl_0x1B, default_tl_0x1C, default_tl_0x1D, default_tl_0x1E, default_tl_0x1F, default_tl_0x20, default_tl_0x21, default_tl_0x22, default_tl_0x23, default_tl_0x24, default_tl_0x25, default_tl_0x26, default_tl_0x27, default_tl_0x28, default_tl_0x29, default_tl_0x2A, default_tl_0x2B, default_tl_0x2C, default_tl_0x2D, default_tl_0x2E, default_tl_0x2F, default_tl_0x30, default_tl_0x31, default_tl_0x32, default_tl_0x33, default_tl_0x34, default_tl_0x35, default_tl_0x36, default_tl_0x37, default_tl_0x38, default_tl_0x39, default_tl_0x3A, default_tl_0x3B, default_tl_0x3C, default_tl_0x3D, default_tl_0x3E, default_tl_0x3F, default_tl_0x40, default_tl_0x41, default_tl_0x42, default_tl_0x43, default_tl_0x44, default_tl_0x45, default_tl_0x46, default_tl_0x47, default_tl_0x48, default_tl_0x49, default_tl_0x4A, default_tl_0x4B, default_tl_0x4C, default_tl_0x4D, default_tl_0x4E, default_tl_0x4F, default_tl_0x50, default_tl_0x51, default_tl_0x52, default_tl_0x53, default_tl_0x54, default_tl_0x55, default_tl_0x56, default_tl_0x57, default_tl_0x58, default_tl_0x59, default_tl_0x5A, default_tl_0x5B, default_tl_0x5C, default_tl_0x5D, default_tl_0x5E, default_tl_0x5F, default_tl_0x60, default_tl_0x61, default_tl_0x62, default_tl_0x63, default_tl_0x64, default_tl_0x65, default_tl_0x66, default_tl_0x67, default_tl_0x68, default_tl_0x69, default_tl_0x6A, default_tl_0x6B, default_tl_0x6C, default_tl_0x6D, default_tl_0x6E, default_tl_0x6F, default_tl_0x70, default_tl_0x71, default_tl_0x72, default_tl_0x73, default_tl_0x74, default_tl_0x75, default_tl_0x76, default_tl_0x77, default_tl_0x78, default_tl_0x79, default_tl_0x7A, default_tl_0x7B, default_tl_0x7C, default_tl_0x7D, default_tl_0x7E, default_tl_0x7F, default_tl_0x80, default_tl_0x81, default_tl_0x82, default_tl_0x83, default_tl_0x84, default_tl_0x85, default_tl_0x86, default_tl_0x87, default_tl_0x88, default_tl_0x89, default_tl_0x8A, default_tl_0x8B, default_tl_0x8C, default_tl_0x8D, default_tl_0x8E, default_tl_0x8F, default_tl_0x90, default_tl_0x91, default_tl_0x92, default_tl_0x93, default_tl_0x94, default_tl_0x95, default_tl_0x96, default_tl_0x97, default_tl_0x98, default_tl_0x99, default_tl_0x9A, default_tl_0x9B, default_tl_0x9C, default_tl_0x9D, default_tl_0x9E, default_tl_0x9F, default_tl_0xA0, default_tl_0xA1, default_tl_0xA2, default_tl_0xA3, default_tl_0xA4, default_tl_0xA5, default_tl_0xA6, default_tl_0xA7, default_tl_0xA8, default_tl_0xA9, default_tl_0xAA, default_tl_0xAB, default_tl_0xAC, default_tl_0xAD, default_tl_0xAE, default_tl_0xAF, default_tl_0xB0, default_tl_0xB1, default_tl_0xB2, default_tl_0xB3, default_tl_0xB4, default_tl_0xB5, default_tl_0xB6, default_tl_0xB7, default_tl_0xB8, default_tl_0xB9, default_tl_0xBA, default_tl_0xBB, default_tl_0xBC, default_tl_0xBD, default_tl_0xBE, default_tl_0xBF, default_tl_0xC0, default_tl_0xC1, default_tl_0xC2, default_tl_0xC3, default_tl_0xC4, default_tl_0xC5, default_tl_0xC6, default_tl_0xC7, default_tl_0xC8, default_tl_0xC9, default_tl_0xCA, default_tl_0xCB, default_tl_0xCC, default_tl_0xCD, default_tl_0xCE, default_tl_0xCF, default_tl_0xD0, default_tl_0xD1, default_tl_0xD2, default_tl_0xD3, default_tl_0xD4, default_tl_0xD5, default_tl_0xD6, default_tl_0xD7, default_tl_0xD8, default_tl_0xD9, default_tl_0xDA, default_tl_0xDB, default_tl_0xDC, default_tl_0xDD, default_tl_0xDE, default_tl_0xDF, default_tl_0xE0, default_tl_0xE1, default_tl_0xE2, default_tl_0xE3, default_tl_0xE4, default_tl_0xE5, default_tl_0xE6, default_tl_0xE7, default_tl_0xE8, default_tl_0xE9, default_tl_0xEA, default_tl_0xEB, default_tl_0xEC, default_tl_0xED, default_tl_0xEE, default_tl_0xEF, default_tl_0xF0, default_tl_0xF1, default_tl_0xF2, default_tl_0xF3, default_tl_0xF4, default_tl_0xF5, default_tl_0xF6, default_tl_0xF7, default_tl_0xF8, default_tl_0xF9, default_tl_0xFA, default_tl_0xFB, default_tl_0xFC, default_tl_0xFD, default_tl_0xFE, default_tl_0xFF,
};

// Layer <name> for ScanCode Lookup
const unsigned int *myname_scanMap[] = {
0, 0, 0, 0, myname_tl_0x05, myname_tl_0x06, myname_tl_0x07
};

// Layer <name> for ScanCode Lookup
const unsigned int *myname2_scanMap[] = {
0, 0, 0, myname2_tl_0x04, myname2_tl_0x05, myname2_tl_0x06
};



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
	const unsigned int **triggerMap;
	const char *name;
	const uint8_t max;
	uint8_t state;
} Layer;


// Layer_IN( map, name );
//  * map  - Trigger map
//  * name - Name of the trigger map
#define Layer_IN( map, name ) { map, name, sizeof( map ) / 4 - 1, 0 }

// -- Layer Index List
//
// Index 0: Default map
// Index n: Additional layers
Layer LayerIndex[] = {
	Layer_IN( default_scanMap, "DefaultMap" ),
	Layer_IN( myname_scanMap, "myname" ),
	Layer_IN( myname2_scanMap, "myname2" ),
};

// Total number of layers
#define LayerNum sizeof( LayerIndex ) / sizeof( Layer )



#endif // __generatedKeymap_h

