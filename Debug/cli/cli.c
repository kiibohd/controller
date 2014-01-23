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
#include <buildvars.h>
#include "cli.h"
#include <led.h>
#include <print.h>



// ----- Variables -----

// Basic command dictionary
CLIDictItem basicCLIDict[] = {
	{ "cliDebug", "Enables/Disables hex output of the most recent cli input.", cliFunc_cliDebug },
	{ "help",     "You're looking at it :P", cliFunc_help },
	{ "led",      "Enables/Disables indicator LED. Try a couple times just in case the LED is in an odd state.\r\n\t\t\033[33mWarning\033[0m: May adversely affect some modules...", cliFunc_led },
	{ "reload",   "Signals microcontroller to reflash/reload.", cliFunc_reload },
	{ "reset",    "Sends a software reset, should be similar to powering on the device.", cliFunc_reset },
	{ "version",  "Version information about this firmware.", cliFunc_version },
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

	// Initialize main LED
	init_errorLED();
	CLILEDState = 0;
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

// Takes a string, returns two pointers
//  One to the first non-space character
//  The second to the next argument (first NULL if there isn't an argument). delimited by a space
//  Places a NULL at the first space after the first argument
inline void argumentIsolation_cli( char* string, char** first, char** second )
{
	// Mark out the first argument
	// This is done by finding the first space after a list of non-spaces and setting it NULL
	char* cmdPtr = string - 1;
	while ( *++cmdPtr == ' ' ); // Skips leading spaces, and points to first character of cmd

	// Locates first space delimiter
	char* argPtr = cmdPtr + 1;
	while ( *argPtr != ' ' && *argPtr != '\0' )
		argPtr++;

	// Point to the first character of args or a NULL (no args) and set the space delimiter as a NULL
	(++argPtr)[-1] = '\0';

	// Set return variables
	*first = cmdPtr;
	*second = argPtr;
}

void commandLookup_cli()
{
	// Ignore command if buffer is 0 length
	if ( CLILineBufferCurrent == 0 )
		return;

	// Set the last+1 character of the buffer to NULL for string processing
	CLILineBuffer[CLILineBufferCurrent] = '\0';

	// Retrieve pointers to command and beginning of arguments
	// Places a NULL at the first space after the command
	char* cmdPtr;
	char* argPtr;
	argumentIsolation_cli( CLILineBuffer, &cmdPtr, &argPtr );

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
	erro_dPrint("\"", CLILineBuffer, "\" is not a valid command...type \033[35mhelp\033[0m");
}

inline void registerDictionary_cli( CLIDictItem *cmdDict )
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

void cliFunc_cliDebug( char* args )
{
}

void cliFunc_help( char* args )
{
	// Scan array of dictionaries and print every description
	//  (no alphabetical here, too much processing/memory to sort...)
	for ( uint8_t dict = 0; dict < CLIDictionariesUsed; dict++ )
	{
		print( NL "\033[1;32mCOMMAND SET\033[0m " );
		printInt8( dict + 1 );
		print( NL );

		// Parse each cmd/description until a null command entry is found
		for ( uint8_t cmd = 0; CLIDict[dict][cmd].name != 0; cmd++ )
		{
			dPrintStrs(" \033[35m", CLIDict[dict][cmd].name, "\033[0m");

			// Determine number of spaces to tab by the length of the command and TabAlign
			uint8_t padLength = CLIEntryTabAlign - lenStr( CLIDict[dict][cmd].name );
			while ( padLength-- > 0 )
				print(" ");

			dPrintStrNL( CLIDict[dict][cmd].description );
		}
	}
}

void cliFunc_led( char* args )
{
	CLILEDState ^= 1 << 1; // Toggle between 0 and 1
	errorLED( CLILEDState ); // Enable/Disable error LED
}

void cliFunc_reload( char* args )
{
	// Request to output module to be set into firmware reload mode
	output_firmwareReload();
}

void cliFunc_reset( char* args )
{
	// Trigger an overall software reset
	SOFTWARE_RESET();
}

void cliFunc_version( char* args )
{
	print( NL );
	print( " \033[1mRevision:\033[0m      " CLI_Revision       NL );
	print( " \033[1mBranch:\033[0m        " CLI_Branch         NL );
	print( " \033[1mTree Status:\033[0m   " CLI_ModifiedStatus NL );
	print( " \033[1mRepo Origin:\033[0m   " CLI_RepoOrigin     NL );
	print( " \033[1mCommit Date:\033[0m   " CLI_CommitDate     NL );
	print( " \033[1mCommit Author:\033[0m " CLI_CommitAuthor   NL );
	print( " \033[1mBuild Date:\033[0m    " CLI_BuildDate      NL );
	print( " \033[1mBuild OS:\033[0m      " CLI_BuildOS        NL );
	print( " \033[1mArchitecture:\033[0m  " CLI_Arch           NL );
	print( " \033[1mChip:\033[0m          " CLI_Chip           NL );
	print( " \033[1mCPU:\033[0m           " CLI_CPU            NL );
	print( " \033[1mDevice:\033[0m        " CLI_Device         NL );
	print( " \033[1mModules:\033[0m       " CLI_Modules        NL );
}

