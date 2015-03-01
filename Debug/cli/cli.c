/* Copyright (C) 2014-2015 by Jacob Alexander
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
CLIDict_Entry( cliDebug, "Enables/Disables hex output of the most recent cli input." );
CLIDict_Entry( help,     "You're looking at it :P" );
CLIDict_Entry( led,      "Enables/Disables indicator LED. Try a couple times just in case the LED is in an odd state.\r\n\t\t\033[33mWarning\033[0m: May adversely affect some modules..." );
CLIDict_Entry( reload,   "Signals microcontroller to reflash/reload." );
CLIDict_Entry( reset,    "Resets the terminal back to initial settings." );
CLIDict_Entry( restart,  "Sends a software restart, should be similar to powering on the device." );
CLIDict_Entry( version,  "Version information about this firmware." );

CLIDict_Def( basicCLIDict, "General Commands" ) = {
	CLIDict_Item( cliDebug ),
	CLIDict_Item( help ),
	CLIDict_Item( led ),
	CLIDict_Item( reload ),
	CLIDict_Item( reset ),
	CLIDict_Item( restart ),
	CLIDict_Item( version ),
	{ 0, 0, 0 } // Null entry for dictionary end
};



// ----- Functions -----

inline void prompt()
{
	print("\033[2K\r"); // Erases the current line and resets cursor to beginning of line
	print("\033[1;34m:\033[0m "); // Blue bold prompt
}

// Initialize the CLI
inline void CLI_init()
{
	// Reset the Line Buffer
	CLILineBufferCurrent = 0;

	// Set prompt
	prompt();

	// Register first dictionary
	CLIDictionariesUsed = 0;
	CLI_registerDictionary( basicCLIDict, basicCLIDictName );

	// Initialize main LED
	init_errorLED();
	CLILEDState = 0;

	// Hex debug mode is off by default
	CLIHexDebugMode = 0;
}

// Query the serial input buffer for any new characters
void CLI_process()
{
	// Current buffer position
	uint8_t prev_buf_pos = CLILineBufferCurrent;

	// Process each character while available
	while ( 1 )
	{
		// No more characters to process
		if ( Output_availablechar() == 0 )
			break;

		// Retrieve from output module
		char cur_char = (char)Output_getchar();

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

	// Display Hex Key Input if enabled
	if ( CLIHexDebugMode && CLILineBufferCurrent > prev_buf_pos )
	{
		print("\033[s\r\n"); // Save cursor position, and move to the next line
		print("\033[2K");    // Erases the current line

		uint8_t pos = prev_buf_pos;
		while ( CLILineBufferCurrent > pos )
		{
			printHex( CLILineBuffer[pos++] );
			print(" ");
		}

		print("\033[u"); // Restore cursor position
	}

	// If buffer has changed, output to screen while there are still characters in the buffer not displayed
	while ( CLILineBufferCurrent > prev_buf_pos )
	{
		// Check for control characters
		switch ( CLILineBuffer[prev_buf_pos] )
		{
		case 0x0D: // Enter
			CLILineBuffer[CLILineBufferCurrent - 1] = ' '; // Replace Enter with a space (resolves a bug in args)

			// Remove the space if there is no command
			if ( CLILineBufferCurrent == 1 )
				CLILineBufferCurrent--;

			// Process the current line buffer
			CLI_commandLookup();

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
			CLI_tabCompletion();

			CLILineBufferCurrent--; // Remove the Tab

			// XXX There is a potential bug here when resetting the buffer (losing valid keypresses)
			//     Doesn't look like it will happen *that* often, so not handling it for now -HaaTa
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
	}
}

// Takes a string, returns two pointers
//  One to the first non-space character
//  The second to the next argument (first NULL if there isn't an argument). delimited by a space
//  Places a NULL at the first space after the first argument
void CLI_argumentIsolation( char* string, char** first, char** second )
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

// Scans the CLILineBuffer for any valid commands
void CLI_commandLookup()
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
	CLI_argumentIsolation( CLILineBuffer, &cmdPtr, &argPtr );

	// Scan array of dictionaries for a valid command match
	for ( uint8_t dict = 0; dict < CLIDictionariesUsed; dict++ )
	{
		// Parse each cmd until a null command entry is found, or an argument match
		for ( uint8_t cmd = 0; CLIDict[dict][cmd].name != 0; cmd++ )
		{
			// Compare the first argument and each command entry
			if ( eqStr( cmdPtr, (char*)CLIDict[dict][cmd].name ) == -1 )
			{
				// Run the specified command function pointer
				//   argPtr is already pointing at the first character of the arguments
				(*(void (*)(char*))CLIDict[dict][cmd].function)( argPtr );

				return;
			}
		}
	}

	// No match for the command...
	print( NL );
	erro_dPrint("\"", CLILineBuffer, "\" is not a valid command...type \033[35mhelp\033[0m");
}

// Registers a command dictionary with the CLI
void CLI_registerDictionary( const CLIDictItem *cmdDict, const char* dictName )
{
	// Make sure this max limit of dictionaries hasn't been reached
	if ( CLIDictionariesUsed >= CLIMaxDictionaries )
	{
		erro_print("Max number of dictionaries defined already...");
		return;
	}

	// Add dictionary
	CLIDictNames[CLIDictionariesUsed] = (char*)dictName;
	CLIDict[CLIDictionariesUsed++] = (CLIDictItem*)cmdDict;
}

inline void CLI_tabCompletion()
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
	CLI_argumentIsolation( CLILineBuffer, &cmdPtr, &argPtr );

	// Tab match pointer
	char* tabMatch = 0;
	uint8_t matches = 0;

	// Scan array of dictionaries for a valid command match
	for ( uint8_t dict = 0; dict < CLIDictionariesUsed; dict++ )
	{
		// Parse each cmd until a null command entry is found, or an argument match
		for ( uint8_t cmd = 0; CLIDict[dict][cmd].name != 0; cmd++ )
		{
			// Compare the first argument piece to each command entry to see if it is "like"
			// NOTE: To save on processing, we only care about the commands and ignore the arguments
			//       If there are arguments, and a valid tab match is found, buffer is cleared (args lost)
			//       Also ignores full matches
			if ( eqStr( cmdPtr, (char*)CLIDict[dict][cmd].name ) == 0 )
			{
				// TODO Make list of commands if multiple matches
				matches++;
				tabMatch = (char*)CLIDict[dict][cmd].name;
			}
		}
	}

	// Only tab complete if there was 1 match
	if ( matches == 1 )
	{
		// Reset the buffer
		CLILineBufferCurrent = 0;

		// Reprint the prompt (automatically clears the line)
		prompt();

		// Display the command
		dPrint( tabMatch );

		// There are no index counts, so just copy the whole string to the input buffer
		while ( *tabMatch != '\0' )
		{
			CLILineBuffer[CLILineBufferCurrent++] = *tabMatch++;
		}
	}
}



// ----- CLI Command Functions -----

void cliFunc_cliDebug( char* args )
{
	// Toggle Hex Debug Mode
	if ( CLIHexDebugMode )
	{
		print( NL );
		info_print("Hex debug mode disabled...");
		CLIHexDebugMode = 0;
	}
	else
	{
		print( NL );
		info_print("Hex debug mode enabled...");
		CLIHexDebugMode = 1;
	}
}

void cliFunc_help( char* args )
{
	// Scan array of dictionaries and print every description
	//  (no alphabetical here, too much processing/memory to sort...)
	for ( uint8_t dict = 0; dict < CLIDictionariesUsed; dict++ )
	{
		// Print the name of each dictionary as a title
		print( NL "\033[1;32m" );
		_print( CLIDictNames[dict] ); // This print is requride by AVR (flash)
		print( "\033[0m" NL );

		// Parse each cmd/description until a null command entry is found
		for ( uint8_t cmd = 0; CLIDict[dict][cmd].name != 0; cmd++ )
		{
			dPrintStrs(" \033[35m", CLIDict[dict][cmd].name, "\033[0m");

			// Determine number of spaces to tab by the length of the command and TabAlign
			uint8_t padLength = CLIEntryTabAlign - lenStr( (char*)CLIDict[dict][cmd].name );
			while ( padLength-- > 0 )
				print(" ");

			_print( CLIDict[dict][cmd].description ); // This print is required by AVR (flash)
			print( NL );
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
	Output_firmwareReload();
}

void cliFunc_reset( char* args )
{
	print("\033c"); // Resets the terminal
}

void cliFunc_restart( char* args )
{
	// Trigger an overall software reset
	Output_softReset();
}

void cliFunc_version( char* args )
{
	print( NL );
	print( " \033[1mRevision:\033[0m      " CLI_Revision       NL );
	print( " \033[1mBranch:\033[0m        " CLI_Branch         NL );
	print( " \033[1mTree Status:\033[0m   " CLI_ModifiedStatus CLI_ModifiedFiles NL );
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

