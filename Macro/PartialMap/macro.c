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

void cliFunc_capList   ( char* args );
void cliFunc_capSelect ( char* args );
void cliFunc_keyPress  ( char* args );
void cliFunc_keyRelease( char* args );
void cliFunc_layerList ( char* args );
void cliFunc_layerState( char* args );
void cliFunc_macroDebug( char* args );
void cliFunc_macroList ( char* args );
void cliFunc_macroProc ( char* args );
void cliFunc_macroShow ( char* args );
void cliFunc_macroStep ( char* args );



// ----- Variables -----

// Macro Module command dictionary
char*       macroCLIDictName = "Macro Module Commands";
CLIDictItem macroCLIDict[] = {
	{ "capList",     "Prints an indexed list of all non USB keycode capabilities.", cliFunc_capList },
	{ "capSelect",   "Triggers the specified capabilities. First two args are state and stateType." NL "\t\t\033[35mK11\033[0m Keyboard Capability 0x0B", cliFunc_capSelect },
	{ "keyPress",    "Send key-presses to the macro module. Held until released. Duplicates have undefined behaviour." NL "\t\t\033[35mS10\033[0m Scancode 0x0A", cliFunc_keyPress },
	{ "keyRelease",  "Release a key-press from the macro module. Duplicates have undefined behaviour." NL "\t\t\033[35mS10\033[0m Scancode 0x0A", cliFunc_keyRelease },
	{ "layerList",   "List available layers.", cliFunc_layerList },
	{ "layerState",  "Modify specified indexed layer state <layer> <state byte>." NL "\t\t\033[35mL2\033[0m Indexed Layer 0x02" NL "\t\t0 Off, 1 Shift, 2 Latch, 4 Lock States", cliFunc_layerState },
	{ "macroDebug",  "Disables/Enables sending USB keycodes to the Output Module and prints U/K codes.", cliFunc_macroDebug },
	{ "macroList",   "List the defined trigger and result macros.", cliFunc_macroList },
	{ "macroProc",   "Pause/Resume macro processing.", cliFunc_macroProc },
	{ "macroShow",   "Show the macro corresponding to the given index." NL "\t\t\033[35mT16\033[0m Indexed Trigger Macro 0x10, \033[35mR12\033[0m Indexed Result Macro 0x0C", cliFunc_macroShow },
	{ "macroStep",   "Do N macro processing steps. Defaults to 1.", cliFunc_macroStep },
	{ 0, 0, 0 } // Null entry for dictionary end
};


// Macro debug flag - If set, clears the USB Buffers after signalling processing completion
uint8_t macroDebugMode = 0;

// Macro pause flag - If set, the macro module pauses processing, unless unset, or the step counter is non-zero
uint8_t macroPauseMode = 0;

// Macro step counter - If non-zero, the step counter counts down every time the macro module does one processing loop
unsigned int macroStepCounter = 0;


// Key Trigger List Buffer
//  * Item 1: scan code
//  * Item 2: state
//    ...
uint8_t macroTriggerListBuffer[MaxScanCode * 2] = { 0 }; // Each key has a state to be cached
uint8_t macroTriggerListBufferSize = 0;

// TODO, figure out a good way to scale this array size without wasting too much memory, but not rejecting macros
//       Possibly could be calculated by the KLL compiler
// XXX It may be possible to calculate the worst case using the KLL compiler
TriggerMacro *triggerMacroPendingList[TriggerMacroNum];



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




/*
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
*/

inline void Macro_finishWithUSBBuffer( uint8_t sentKeys )
{
}

inline void Macro_process()
{
	// Only do one round of macro processing between Output Module timer sends
	if ( USBKeys_Sent != 0 )
		return;

	// If the pause flag is set, only process if the step counter is non-zero
	if ( macroPauseMode && macroStepCounter == 0 )
	{
		return;
	}
	// Proceed, decrementing the step counter
	else
	{
		macroStepCounter--;
	}

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

	// Disable Macro pause flag
	macroPauseMode = 0;

	// Set Macro step counter to zero
	macroStepCounter = 0;

	// Make sure macro trigger buffer is empty
	macroTriggerListBufferSize = 0;
}


// ----- CLI Command Functions -----

void cliFunc_capList( char* args )
{
	print( NL );
	info_msg("Capabilities List");

	// Iterate through all of the capabilities and display them
	for ( unsigned int cap = 0; cap < CapabilitiesNum; cap++ )
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
	unsigned int totalArgs = 2; // Always at least two args
	unsigned int cap = 0;

	// Arguments used for keyboard capability function
	unsigned int argSetCount = 0;
	uint8_t *argSet = (uint8_t*)args;

	// Process all args
	for ( unsigned int c = 0; argSetCount < totalArgs; c++ )
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
			cap = decToInt( &arg1Ptr[1] );

			// Lookup the number of args
			totalArgs += CapabilitiesList[ cap ].argCount;
			continue;
		}

		// Because allocating memory isn't doable, and the argument count is arbitrary
		// The argument pointer is repurposed as the argument list (much smaller anyways)
		argSet[ argSetCount++ ] = (uint8_t)decToInt( arg1Ptr );

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
			Macro_keyState( (uint8_t)decToInt( &arg1Ptr[1] ), 0x01 ); // Press scancode
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
			Macro_keyState( (uint8_t)decToInt( &arg1Ptr[1] ), 0x03 ); // Release scancode
			break;
		}
	}
}

void cliFunc_layerList( char* args )
{
	print( NL );
	info_msg("Layer List");

	// Iterate through all of the layers and display them
	for ( unsigned int layer = 0; layer < LayerNum; layer++ )
	{
		print( NL "\t" );
		printHex( layer );
		print(" - ");

		// Display layer name
		dPrint( LayerIndex[ layer ].name );

		// Default map
		if ( layer == 0 )
			print(" \033[1m(default)\033[0m");

		// Layer State
		print( NL "\t\t Layer State: " );
		printHex( LayerIndex[ layer ].state );

		// Max Index
		print(" Max Index: ");
		printHex( LayerIndex[ layer ].max );
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

			arg1 = (uint8_t)decToInt( &arg1Ptr[1] );
			break;
		// Second argument (e.g. 4)
		case 1:
			arg2 = (uint8_t)decToInt( arg1Ptr );

			// Display operation (to indicate that it worked)
			print( NL );
			info_msg("Setting Layer L");
			printInt8( arg1 );
			print(" to - ");
			printHex( arg2 );

			// Set the layer state
			LayerIndex[ arg1 ].state = arg2;
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
	for ( unsigned int macro = 0; macro < TriggerMacroNum; macro++ )
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

void macroDebugShowTrigger( unsigned int index )
{
	// Only proceed if the macro exists
	if ( index >= TriggerMacroNum )
		return;

	// Trigger Macro Show
	TriggerMacro *macro = &TriggerMacroList[ index ];

	print( NL );
	info_msg("Trigger Macro Index: ");
	printInt16( (uint16_t)index ); // Hopefully large enough :P (can't assume 32-bit)
	print( NL );

	// Read the comboLength for combo in the sequence (sequence of combos)
	unsigned int pos = 0;
	uint8_t comboLength = macro->guide[ pos ];

	// Iterate through and interpret the guide
	while ( comboLength != 0 )
	{
		// Initial position of the combo
		unsigned int comboPos = ++pos;

		// Iterate through the combo
		while ( pos < comboLength * TriggerGuideSize + comboPos )
		{
			// Assign TriggerGuide element (key type, state and scancode)
			TriggerGuide *guide = (TriggerGuide*)(&macro->guide[ pos ]);

			// Display guide information about trigger key
			printHex( guide->scancode );
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
	printInt16( (uint16_t)macro->pos ); // Hopefully large enough :P (can't assume 32-bit)

	// Display result macro index
	print( NL "Result Macro Index: " );
	printInt16( (uint16_t)macro->result ); // Hopefully large enough :P (can't assume 32-bit)
}

void macroDebugShowResult( unsigned int index )
{
	// Only proceed if the macro exists
	if ( index >= ResultMacroNum )
		return;

	// Trigger Macro Show
	ResultMacro *macro = &ResultMacroList[ index ];

	print( NL );
	info_msg("Result Macro Index: ");
	printInt16( (uint16_t)index ); // Hopefully large enough :P (can't assume 32-bit)
	print( NL );

	// Read the comboLength for combo in the sequence (sequence of combos)
	unsigned int pos = 0;
	uint8_t comboLength = macro->guide[ pos++ ];

	// Iterate through and interpret the guide
	while ( comboLength != 0 )
	{
		// Function Counter, used to keep track of the combos processed
		unsigned int funcCount = 0;

		// Iterate through the combo
		while ( funcCount < comboLength )
		{
			// Assign TriggerGuide element (key type, state and scancode)
			ResultGuide *guide = (ResultGuide*)(&macro->guide[ pos ]);

			// Display Function Index
			printHex( guide->index );
			print("|");

			// Display Function Ptr Address
			printHex( (unsigned int)CapabilitiesList[ guide->index ].func );
			print("|");

			// Display/Lookup Capability Name (utilize debug mode of capability)
			void (*capability)(uint8_t, uint8_t, uint8_t*) = (void(*)(uint8_t, uint8_t, uint8_t*))(CapabilitiesList[ guide->index ].func);
			capability( 0xFF, 0xFF, 0 );

			// Display Argument(s)
			print("(");
			for ( unsigned int arg = 0; arg < CapabilitiesList[ guide->index ].argCount; arg++ )
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
	printInt16( (uint16_t)macro->pos ); // Hopefully large enough :P (can't assume 32-bit)

	// Display final trigger state/type
	print( NL "Final Trigger State (State/Type): " );
	printHex( macro->state );
	print("/");
	printHex( macro->stateType );
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
			macroDebugShowTrigger( decToInt( &arg1Ptr[1] ) );
			break;
		// Indexed Result Macro
		case 'R':
			macroDebugShowResult( decToInt( &arg1Ptr[1] ) );
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

	// Set the macro step counter, negative int's are cast to uint
	macroStepCounter = (unsigned int)decToInt( arg1Ptr );
}

