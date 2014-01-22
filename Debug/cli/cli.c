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
//#include <stdarg.h>

// Project Includes
#include "cli.h"
#include <print.h>



// ----- Variables -----

// Basic command dictionary
CLIDictItem basicCLIDict[] = {
	{ "help",    "This command :P", cliFunc_help },
	{ "version", "Version information about this firmware.", cliFunc_version },
	{ 0, 0, 0 } // Null entry for dictionary end
};



// ----- Functions -----

inline void prompt()
{
	print(": ");
}

inline void init_cli()
{
	// Reset the Line Buffer
	CLILineBufferCurrent = 0;

	// Set prompt
	prompt();

	// Register first dictionary
	CLIDictionariesUsed = 0;
	registerDictionary_cli( basicCLIDict );
}

void process_cli()
{
	// Current buffer position
	uint8_t prev_buf_pos = CLILineBufferCurrent;

	// Process each character while available
	int result = 0;
	while ( 1 )
	{
		// No more characters to process
		result = usb_serial_getchar(); // Retrieve from serial module // TODO Make USB agnostic
		if ( result == -1 )
			break;

		char cur_char = (char)result;

		// Make sure buffer isn't full
		if ( CLILineBufferCurrent >= CLILineBufferMaxSize )
		{
			print( NL );
			erro_print("Serial line buffer is full, dropping character and resetting...");

			// Clear buffer
			CLILineBufferCurrent = 0;

			// Reset the prompt
			prompt();

			return;
		}

		// Place into line buffer
		CLILineBuffer[CLILineBufferCurrent++] = cur_char;
	}

	// If buffer has changed, output to screen while there are still characters in the buffer not displayed
	while ( CLILineBufferCurrent > prev_buf_pos )
	{
		// Check for control characters
		switch ( CLILineBuffer[prev_buf_pos] )
		{
		case 0x0D: // Enter
			CLILineBufferCurrent--; // Remove the Enter

			// Process the current line buffer
			commandLookup_cli();

			// Reset the buffer
			CLILineBufferCurrent = 0;

			// Reset the prompt after processing has finished
			print( NL );
			prompt();

			// XXX There is a potential bug here when resetting the buffer (losing valid keypresses)
			//     Doesn't look like it will happen *that* often, so not handling it for now -HaaTa
			return;

		case 0x09: // Tab
			// Tab completion for the current command
			// TODO
			return;

		case 0x1B: // Esc
			// Check for escape sequence
			// TODO
			return;

		case 0x08:
		case 0x7F: // Backspace
			// TODO - Does not handle case for arrow editing (arrows disabled atm)
			CLILineBufferCurrent--; // Remove the backspace

			// If there are characters in the buffer
			if ( CLILineBufferCurrent > 0 )
			{
				// Remove character from current position in the line buffer
				CLILineBufferCurrent--;

				// Remove character from tty
				print("\b \b");
			}

			break;

		default:
			// Place a null on the end (to use with string print)
			CLILineBuffer[CLILineBufferCurrent] = '\0';

			// Output buffer to screen
			dPrint( &CLILineBuffer[prev_buf_pos] );

			// Buffer reset
			prev_buf_pos++;

			break;
		}

		/* TODO Enable via option
		uint8_t pos = prev_buf_pos;
		while ( CLILineBuffer[pos] != 0 )
		{
			printHex( CLILineBuffer[pos++] );
			print(" ");
		}

		print( NL );
		*/
	}
}

void commandLookup_cli()
{
	// Ignore command if buffer is 0 length
	if ( CLILineBufferCurrent == 0 )
		return;

	// Set the last+1 character of the buffer to NULL for string processing
	CLILineBuffer[CLILineBufferCurrent] = '\0';

	// Mark out the first argument
	// This is done by finding the first space after a list of non-spaces and setting it NULL
	char* cmdPtr = CLILineBuffer - 1;
	while ( *++cmdPtr == ' ' ); // Skips leading spaces, and points to first character of cmd

	// Locates first space delimiter, and points to first character of args or a NULL (no args)
	char* argPtr = cmdPtr;
	do {
		argPtr++;
	} while ( *argPtr != ' ' && *argPtr != '\0' );

	// Set the space delimiter as a NULL
	argPtr[-1] = '\0';

	// Scan array of dictionaries for a valid command match
	for ( uint8_t dict = 0; dict < CLIDictionariesUsed; dict++ )
	{
		// Parse each cmd until a null command entry is found, or an argument match
		for ( uint8_t cmd = 0; CLIDict[dict][cmd].name != 0; cmd++ )
		{
			// Compare the first argument and each command entry
			if ( eqStr( cmdPtr, CLIDict[dict][cmd].name ) )
			{
				// Run the specified command function pointer
				//   argPtr is already pointing at the first character of the arguments
				(*CLIDict[dict][cmd].function)( argPtr );

				return;
			}
		}
	}

	// No match for the command...
	print( NL );
	erro_dPrint("\"", CLILineBuffer, "\" is not a valid command...try help");
}

void registerDictionary_cli( CLIDictItem *cmdDict )
{
	// Make sure this max limit of dictionaries hasn't been reached
	if ( CLIDictionariesUsed >= CLIMaxDictionaries )
	{
		erro_print("Max number of dictionaries defined already...");
		return;
	}

	// Add dictionary
	CLIDict[CLIDictionariesUsed++] = cmdDict;
}



// ----- CLI Command Functions -----

void cliFunc_help( char* args )
{
	print( NL );
	print("Help!");
	dPrint( args );
}

void cliFunc_version( char* args )
{
	print( NL );
	print("Version!");
	dPrint( args );
}

