/* Copyright (C) 2014 by Jacob Alexander
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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

// Output Module command dictionary
char*       macroCLIDictName = "Macro Module Commands";
CLIDictItem macroCLIDict[] = {
	{ "capList",     "Prints an indexed list of all non USB keycode capabilities.", cliFunc_capList },
	{ "capSelect",   "Triggers the specified capability. U10 - USB Code 0x0A. K11 - Keyboard Capability 0x0B. S10 - Scancode 0x0A", cliFunc_capSelect },
	{ "lookComb",    "Do a lookup on the Combined map. S10 - Scancode 0x0A. U10 - USB Code 0x0A.", cliFunc_lookComb },
	{ "lookDefault", "Do a lookup on the Default map. S10 - Scancode 0x0A.", cliFunc_lookDefault },
	{ "lookPartial", "Do a lookup on the layered Partial maps. S10 - Scancode 0x0A. U10 - USB Code 0x0A.", cliFunc_lookPartial },
	{ "macroDebug",  "Disables/Enables sending USB keycodes to the Output Module and prints U/K codes.", cliFunc_macroDebug },
	{ 0, 0, 0 } // Null entry for dictionary end
};


// Macro debug flag - If set, clears the USB Buffers after signalling processing completion
uint8_t macroDebugMode = 0;



// ----- Functions -----

inline void Macro_bufferAdd( uint8_t byte )
{
	// Make sure we haven't overflowed the key buffer
	// Default function for adding keys to the KeyIndex_Buffer, does a DefaultMap_Lookup
	if ( KeyIndex_BufferUsed < KEYBOARD_BUFFER )
	{
		KeyIndex_Buffer[KeyIndex_BufferUsed++] = DefaultMap_Lookup[byte];
	}
}

inline void Macro_finishWithUSBBuffer( uint8_t sentKeys )
{
}

inline void Macro_process()
{
	// Only do one round of macro processing between Output Module timer sends
	if ( USBKeys_Sent != 0 )
		return;

	// Loop through input buffer
	for ( uint8_t index = 0; index < KeyIndex_BufferUsed; index++ )
	{
		// Get the keycode from the buffer
		uint8_t key = KeyIndex_Buffer[index];

		// Set the modifier bit if this key is a modifier
		if ( key & KEY_LCTRL ) // AND with 0xE0
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

