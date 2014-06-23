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

// ----- Includes -----

// Compiler Includes
#include <Lib/MacroLib.h>

// Project Includes
#include <cli.h>
#include <led.h>
#include <print.h>
#include <scan_loop.h>
#include <output_com.h>

// Keymaps
#include "usb_hid.h"
#include <defaultMap.h>
#include "generatedKeymap.h" // TODO Use actual generated version

// Local Includes
#include "macro.h"



// ----- Function Declarations -----

void cliFunc_capList    ( char* args );
void cliFunc_capSelect  ( char* args );
void cliFunc_lookComb   ( char* args );
void cliFunc_lookDefault( char* args );
void cliFunc_lookPartial( char* args );
void cliFunc_macroDebug ( char* args );



// ----- Variables -----

// Macro Module command dictionary
char*       macroCLIDictName = "Macro Module Commands (Not all commands fully work yet...)";
CLIDictItem macroCLIDict[] = {
	{ "capList",     "Prints an indexed list of all non USB keycode capabilities.", cliFunc_capList },
	{ "capSelect",   "Triggers the specified capability." NL "\t\t\033[35mU10\033[0m USB Code 0x0A, \033[35mK11\033[0m Keyboard Capability 0x0B, \033[35mS12\033[0m Scancode 0x0C", cliFunc_capSelect },
	{ "lookComb",    "Do a lookup on the Combined map." NL "\t\t\033[35mS10\033[0m Scancode 0x0A, \033[35mU11\033[0m USB Code 0x0B", cliFunc_lookComb },
	{ "lookDefault", "Do a lookup on the Default map." NL "\t\t\033[35mS10\033[0m Scancode 0x0A", cliFunc_lookDefault },
	{ "lookPartial", "Do a lookup on the layered Partial maps." NL "\t\t\033[35mS10\033[0m Scancode 0x0A, \033[35mU11\033[0m USB Code 0x0B", cliFunc_lookPartial },
	{ "macroDebug",  "Disables/Enables sending USB keycodes to the Output Module and prints U/K codes.", cliFunc_macroDebug },
	{ 0, 0, 0 } // Null entry for dictionary end
};


// Macro debug flag - If set, clears the USB Buffers after signalling processing completion
uint8_t macroDebugMode = 0;

// Key Trigger List Buffer
//  * Item 1: scan code
//  * Item 2: state
//    ...
uint8_t macroTriggerListBuffer[0xFF * 2] = { 0 }; // Each key has a state to be cached (this can be decreased to save RAM)
uint8_t macroTriggerListBufferSize = 0;

// TODO, figure out a good way to scale this array size without wasting too much memory, but not rejecting macros
//       Possibly could be calculated by the KLL compiler
TriggerMacro *triggerMacroPendingList[30];



// ----- Functions -----

// Looks up the trigger list for the given scan code (from the active layer)
unsigned int *Macro_layerLookup( uint8_t scanCode )
{
	// TODO - No layer fallthrough lookup
	return default_scanMap[ scanCode ];
}


// Update the scancode key state
// States:
//   * 0x00 - Reserved
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
		macroTriggerListBuffer[ macroTriggerListBufferSize++ ] = scanCode;
		macroTriggerListBuffer[ macroTriggerListBufferSize++ ] = state;
		break;
	}
}


// Update the scancode analog state
// States:
//   * 0x00      - Reserved
//   * 0x01      - Released
//   * 0x02-0xFF - Analog value (low to high)
inline void Macro_analogState( uint8_t scanCode, uint8_t state )
{
	// TODO
}


// Update led state
// States:
//   * 0x00 - Reserved
//   * 0x01 - On
//   * 0x02 - Off
inline void Macro_ledState( uint8_t ledCode, uint8_t state )
{
	// TODO
}


// Evaluate/Update the TriggerMacro
void Macro_evalTriggerMacro( TriggerMacro *triggerMacro )
{
	// Which combo in the sequence is being evaluated
	unsigned int comboPos = triggerMacro->pos;

	// If combo length is more than 1, cancel trigger macro if an incorrect key is found
	uint8_t comboLength = triggerMacro->guide[ comboPos ];

	// Iterate over list of keys currently pressed
	for ( uint8_t keyPressed = 0; keyPressed < macroTriggerListBufferSize; keyPressed += 2 )
	{
		// Compare with keys in combo
		for ( unsigned int comboKey = 0; comboKey < comboLength; comboKey++ )
		{
			// Lookup key in combo
			uint8_t guideKey = triggerMacro->guide[ comboPos + comboKey + 2 ]; // TODO Only Press/Hold/Release atm

			// Sequence Case
			if ( comboLength == 1 )
			{
				// If key matches and only 1 key pressed, increment the TriggerMacro combo position
				if ( guideKey == macroTriggerListBuffer[ keyPressed ] && macroTriggerListBufferSize == 1 )
				{
					triggerMacro->pos += comboLength * 2 + 1;
					// TODO check if TriggerMacro is finished, register ResultMacro
					return;
				}

				// If key does not match or more than 1 key pressed, reset the TriggerMacro combo position
				triggerMacro->pos = 0;
				return;
			}
			// Combo Case
			else
			{
				// TODO
			}
		}
	}
}





inline void Macro_bufferAdd( uint8_t byte )
{
	// Make sure we haven't overflowed the key buffer
	// Default function for adding keys to the KeyIndex_Buffer, does a DefaultMap_Lookup
	if ( KeyIndex_BufferUsed < KEYBOARD_BUFFER )
	{
		uint8_t key = DefaultMap_Lookup[byte];
		for ( uint8_t c = 0; c < KeyIndex_BufferUsed; c++ )
		{
			// Key already in the buffer
			if ( KeyIndex_Buffer[c] == key )
				return;
		}

		// Add to the buffer
		KeyIndex_Buffer[KeyIndex_BufferUsed++] = key;
	}
}

inline void Macro_bufferRemove( uint8_t byte )
{
	uint8_t key = DefaultMap_Lookup[byte];

	// Check for the released key, and shift the other keys lower on the buffer
	for ( uint8_t c = 0; c < KeyIndex_BufferUsed; c++ )
	{
		// Key to release found
		if ( KeyIndex_Buffer[c] == key )
		{
			// Shift keys from c position
			for ( uint8_t k = c; k < KeyIndex_BufferUsed - 1; k++ )
				KeyIndex_Buffer[k] = KeyIndex_Buffer[k + 1];

			// Decrement Buffer
			KeyIndex_BufferUsed--;

			return;
		}
	}

	// Error case (no key to release)
	erro_msg("Could not find key to release: ");
	printHex( key );
}

inline void Macro_finishWithUSBBuffer( uint8_t sentKeys )
{
}

inline void Macro_process()
{
	// Only do one round of macro processing between Output Module timer sends
	if ( USBKeys_Sent != 0 )
		return;

	// Loop through macro trigger buffer
	for ( uint8_t index = 0; index < macroTriggerListBufferSize; index += 2 )
	{
		// Get scanCode, first item of macroTriggerListBuffer pairs
		uint8_t scanCode = macroTriggerListBuffer[ index ];

		// Lookup trigger list for this key
		unsigned int *triggerList = Macro_layerLookup( scanCode );

		// The first element is the length of the trigger list
		unsigned int triggerListSize = triggerList[0];

		// Loop through the trigger list
		for ( unsigned int trigger = 0; trigger < triggerListSize; trigger++ )
		{
			// Lookup TriggerMacro
			TriggerMacro *triggerMacro = (TriggerMacro*)triggerList[ trigger + 1 ];

			// Get triggered state of scan code, second item of macroTriggerListBuffer pairs
			uint8_t state = macroTriggerListBuffer[ index + 1 ];

			// Evaluate Macro
			Macro_evalTriggerMacro( triggerMacro );
		}
	}





	/* TODO
	// Loop through input buffer
	for ( uint8_t index = 0; index < KeyIndex_BufferUsed && !macroDebugMode; index++ )
	{
		//print(" KEYS: ");
		//printInt8( KeyIndex_BufferUsed );
		// Get the keycode from the buffer
		uint8_t key = KeyIndex_Buffer[index];

		// Set the modifier bit if this key is a modifier
		if ( (key & KEY_LCTRL) == KEY_LCTRL ) // AND with 0xE0
		{
			USBKeys_Modifiers |= 1 << (key ^ KEY_LCTRL); // Left shift 1 by key XOR 0xE0

			// Modifier processed, move on to the next key
			continue;
		}

		// Too many keys
		if ( USBKeys_Sent >= USBKeys_MaxSize )
		{
			warn_msg("USB Key limit reached");
			errorLED( 1 );
			break;
		}

		// Allow ignoring keys with 0's
		if ( key != 0 )
		{
			USBKeys_Array[USBKeys_Sent++] = key;
		}
		else
		{
			// Key was not mapped
			erro_msg( "Key not mapped... - " );
			printHex( key );
			errorLED( 1 );
		}
	}
	*/

	// Signal buffer that we've used it
	Scan_finishedWithBuffer( KeyIndex_BufferUsed );

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

	// Make sure macro trigger buffer is empty
	macroTriggerListBufferSize = 0;
}


// ----- CLI Command Functions -----

void cliFunc_capList( char* args )
{
	// TODO
}

void cliFunc_capSelect( char* args )
{
	// Parse code from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	// Depending on the first character, the lookup changes
	switch ( arg1Ptr[0] )
	{
	// Keyboard Capability
	case 'K':
		// TODO
		break;

	// Scancode
	case 'S':
		// Add to the USB Buffer using the DefaultMap lookup
		Macro_bufferAdd( decToInt( &arg1Ptr[1] ) );
		break;

	// USB Code
	case 'U':
		// Just add the key to the USB Buffer
		if ( KeyIndex_BufferUsed < KEYBOARD_BUFFER )
		{
			KeyIndex_Buffer[KeyIndex_BufferUsed++] = decToInt( &arg1Ptr[1] );
		}
		break;
	}
}

void cliFunc_lookComb( char* args )
{
	// Parse code from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	// Depending on the first character, the lookup changes
	switch ( arg1Ptr[0] )
	{
	// Scancode
	case 'S':
		// TODO
		break;

	// USB Code
	case 'U':
		// TODO
		break;
	}
}

void cliFunc_lookDefault( char* args )
{
	// Parse code from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	// Depending on the first character, the lookup changes
	switch ( arg1Ptr[0] )
	{
	// Scancode
	case 'S':
		print( NL );
		printInt8( DefaultMap_Lookup[decToInt( &arg1Ptr[1] )] );
		print(" ");
		printHex( DefaultMap_Lookup[decToInt( &arg1Ptr[1] )] );
		break;
	}
}

void cliFunc_lookPartial( char* args )
{
	// Parse code from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	// Depending on the first character, the lookup changes
	switch ( arg1Ptr[0] )
	{
	// Scancode
	case 'S':
		// TODO
		break;

	// USB Code
	case 'U':
		// TODO
		break;
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

