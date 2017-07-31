/* Copyright (C) 2014-2017 by Jacob Alexander
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

// Project Includes
#include <Lib/entropy.h>
#include <Lib/time.h>

// General Includes
#include <buildvars.h>
#include <led.h>
#include <print.h>

// KLL Includes
#include <kll_defs.h>

// Local Includes
#include "cli.h"



// ----- Variables -----

// Basic command dictionary
CLIDict_Entry( clear,     "Clear the screen.");
CLIDict_Entry( cliDebug,  "Enables/Disables hex output of the most recent cli input." );
CLIDict_Entry( colorTest, "Displays a True Color ANSI test sequence to test terminal. If it displays in color, you're good." );
#if defined(_host_)
CLIDict_Entry( exit,      "Host KLL Only - Exits cli." );
#endif
CLIDict_Entry( help,      "You're looking at it :P" );
CLIDict_Entry( led,       "Enables/Disables indicator LED. Try a couple times just in case the LED is in an odd state.\r\n\t\t\033[33mWarning\033[0m: May adversely affect some modules..." );
CLIDict_Entry( rand,      "If entropy available, print a random 32-bit number." );
CLIDict_Entry( reload,    "Signals microcontroller to reflash/reload." );
CLIDict_Entry( reset,     "Resets the terminal back to initial settings." );
CLIDict_Entry( restart,   "Sends a software restart, should be similar to powering on the device." );
CLIDict_Entry( tick,      "Displays the fundamental tick size, and current ticks since last systick." );
CLIDict_Entry( version,   "Version information about this firmware." );

CLIDict_Def( basicCLIDict, "General Commands" ) = {
	CLIDict_Item( clear ),
	CLIDict_Item( cliDebug ),
	CLIDict_Item( colorTest ),
#if defined(_host_)
	CLIDict_Item( exit ),
#endif
	CLIDict_Item( help ),
	CLIDict_Item( led ),
	CLIDict_Item( rand ),
	CLIDict_Item( reload ),
	CLIDict_Item( reset ),
	CLIDict_Item( restart ),
	CLIDict_Item( tick ),
	CLIDict_Item( version ),
	{ 0, 0, 0 } // Null entry for dictionary end
};

#if defined(_host_)
int CLI_exit = 0; // When 1, cli signals library to exit (Host-side KLL only)
#endif



// ----- Functions -----

void prompt()
{
	print("\033[2K\r"); // Erases the current line and resets cursor to beginning of line
	print("\033[1;34m:\033[0m "); // Blue bold prompt
}

// Initialize the CLI
inline void CLI_init()
{
	// Reset the Line Buffer
	CLILineBufferCurrent = 0;

	// History starts empty
	CLIHistoryHead = 0;
	CLIHistoryCurrent = 0;
	CLIHistoryTail = 0;

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

#if defined(_host_)
	// Make sure we're not exiting right away in Host-side KLL mode
	CLI_exit = 0;
#endif
}

// Query the serial input buffer for any new characters
int CLI_process()
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

			return 0;
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
		// Enter
		case 0x0A: // LF
		case 0x0D: // CR
			CLILineBuffer[CLILineBufferCurrent - 1] = ' '; // Replace Enter with a space (resolves a bug in args)

			// Remove the space if there is no command
			if ( CLILineBufferCurrent == 1 )
			{
				CLILineBufferCurrent--;
			}
			else
			{
				// Add the command to the history
				CLI_saveHistory( CLILineBuffer );

				// Process the current line buffer
				CLI_commandLookup();

				// Keep the array circular, discarding the older entries
				if ( CLIHistoryTail < CLIHistoryHead )
					CLIHistoryHead = ( CLIHistoryHead + 1 ) % CLIMaxHistorySize;
				CLIHistoryTail++;
				if ( CLIHistoryTail == CLIMaxHistorySize )
				{
					CLIHistoryTail = 0;
					CLIHistoryHead = 1;
				}

				CLIHistoryCurrent = CLIHistoryTail; // 'Up' starts at the last item
				CLI_saveHistory( NULL ); // delete the old temp buffer

			}

			// Reset the buffer
			CLILineBufferCurrent = 0;

			// Reset the prompt after processing has finished
			print( NL );
			prompt();

			// Check if we need to exit right away
#if defined(_host_)
			if ( CLI_exit )
			{
				CLI_exit = 0;
				return 1;
			}
#endif

			// XXX There is a potential bug here when resetting the buffer (losing valid keypresses)
			//     Doesn't look like it will happen *that* often, so not handling it for now -HaaTa
			return 0;

		case 0x09: // Tab
			// Tab completion for the current command
			CLI_tabCompletion();

			CLILineBufferCurrent--; // Remove the Tab

			// XXX There is a potential bug here when resetting the buffer (losing valid keypresses)
			//     Doesn't look like it will happen *that* often, so not handling it for now -HaaTa
			return 0;

		case 0x1B: // Esc / Escape codes
			// Check for other escape sequence

			// \e[ is an escape code in vt100 compatible terminals
			if ( CLILineBufferCurrent >= prev_buf_pos + 3
				&& CLILineBuffer[ prev_buf_pos ] == 0x1B
				&& CLILineBuffer[ prev_buf_pos + 1] == 0x5B )
			{
				// Arrow Keys: A (0x41) = Up, B (0x42) = Down, C (0x43) = Right, D (0x44) = Left

				if ( CLILineBuffer[ prev_buf_pos + 2 ] == 0x41 ) // Hist prev
				{
					if ( CLIHistoryCurrent == CLIHistoryTail )
					{
						// Is first time pressing arrow. Save the current buffer
						CLILineBuffer[ prev_buf_pos ] = '\0';
						CLI_saveHistory( CLILineBuffer );
					}

					// Grab the previus item from the history if there is one
					if ( RING_PREV( CLIHistoryCurrent ) != RING_PREV( CLIHistoryHead ) )
						CLIHistoryCurrent = RING_PREV( CLIHistoryCurrent );
					CLI_retreiveHistory( CLIHistoryCurrent );
				}
				if ( CLILineBuffer[ prev_buf_pos + 2 ] == 0x42 ) // Hist next
				{
					// Grab the next item from the history if it exists
					if ( RING_NEXT( CLIHistoryCurrent ) != RING_NEXT( CLIHistoryTail ) )
						CLIHistoryCurrent = RING_NEXT( CLIHistoryCurrent );
					CLI_retreiveHistory( CLIHistoryCurrent );
				}
			}
			return 0;

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

	return 0;
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

inline int CLI_wrap( int kX, int const kLowerBound, int const kUpperBound )
{
	int range_size = kUpperBound - kLowerBound + 1;

	if ( kX < kLowerBound )
		kX += range_size * ((kLowerBound - kX) / range_size + 1);

	return kLowerBound + (kX - kLowerBound) % range_size;
}

inline void CLI_saveHistory( char *buff )
{
	if ( buff == NULL )
	{
		//clear the item
		CLIHistoryBuffer[ CLIHistoryTail ][ 0 ] = '\0';
		return;
	}

        // Don't write empty lines to the history
        const char *cursor = buff;
        while (*cursor == ' ') { cursor++; } // advance past the leading whitespace
        if (*cursor == '\0') { return ; }

	// Copy the line to the history
	int i;
	for (i = 0; i < CLILineBufferCurrent; i++)
	{
		CLIHistoryBuffer[ CLIHistoryTail ][ i ] = CLILineBuffer[ i ];
	}
}

void CLI_retreiveHistory( int index )
{
	char *histMatch = CLIHistoryBuffer[ index ];

	// Reset the buffer
	CLILineBufferCurrent = 0;

	// Reprint the prompt (automatically clears the line)
	prompt();

	// Display the command
	dPrint( histMatch );

	// There are no index counts, so just copy the whole string to the input buffe
	CLILineBufferCurrent = 0;
	while ( *histMatch != '\0' )
	{
		CLILineBuffer[ CLILineBufferCurrent++ ] = *histMatch++;
	}
}



// ----- CLI Command Functions -----

void cliFunc_clear( char* args)
{
	print("\033[2J\033[H\r"); // Erases the whole screen
}

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

void cliFunc_colorTest( char* args )
{
	print( NL );
	print("\x1b[38;2;255;100;0mTRUECOLOR\x1b[0m");
}

#if defined(_host_)
void cliFunc_exit( char* args )
{
	CLI_exit = 1;
}
#endif

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

void cliFunc_rand( char* args )
{
	print( NL );

	// Check if entropy available
	if ( !rand_available() )
	{
		warn_print("No entropy available!");
		return;
	}

	info_msg("Rand: ");
	printHex32( rand_value32() );
}

void cliFunc_reload( char* args )
{
	if ( flashModeEnabled_define == 0 )
	{
		print( NL );
		warn_print("flashModeEnabled not set, cancelling firmware reload...");
		info_msg("Set flashModeEnabled to 1 in your kll configuration.");
		return;
	}

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

void cliFunc_tick( char* args )
{
	print( NL );

	// Get current time
	Time now = Time_now();

	// Display <systick>:<cycleticks since systick>
	info_msg("ns per cycletick: ");
	print( Time_ticksPer_ns_str );
	print( NL );
	info_print("<systick ms>:<cycleticks since systick>");
	printInt32( now.ms );
	print(":");
	printInt32( now.ticks );
	print( NL );
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
#if defined(_mk20dx128_) || defined(_mk20dx128vlf5_) || defined(_mk20dx256_) || defined(_mk20dx256vlh7_)
	print( " \033[1mUnique Id:\033[0m     " );
	printHex32_op( SIM_UIDH, 8 );
	printHex32_op( SIM_UIDMH, 8 );
	printHex32_op( SIM_UIDML, 8 );
	printHex32_op( SIM_UIDL, 8 );
#elif defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_)
#elif defined(_host_)
#else
#error "No unique id defined."
#endif
}

