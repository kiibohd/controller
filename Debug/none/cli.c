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

// Project Includes
#include <buildvars.h>
#include "cli.h"
#include <led.h>
#include <print.h>
#include <kll_defs.h>



// ----- Variables -----

// Basic command dictionary
CLIDict_Entry( clear, "Clear the screen.");
CLIDict_Entry( cliDebug, "Enables/Disables hex output of the most recent cli input." );
CLIDict_Entry( help,     "You're looking at it :P" );
CLIDict_Entry( led,      "Enables/Disables indicator LED. Try a couple times just in case the LED is in an odd state.\r\n\t\t\033[33mWarning\033[0m: May adversely affect some modules..." );
CLIDict_Entry( reload,   "Signals microcontroller to reflash/reload." );
CLIDict_Entry( reset,    "Resets the terminal back to initial settings." );
CLIDict_Entry( restart,  "Sends a software restart, should be similar to powering on the device." );
CLIDict_Entry( version,  "Version information about this firmware." );

CLIDict_Def( basicCLIDict, "General Commands" ) = {
	CLIDict_Item( clear ),
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
}

// Initialize the CLI
inline void CLI_init()
{
}

// Query the serial input buffer for any new characters
void CLI_process()
{
}

// Takes a string, returns two pointers
//  One to the first non-space character
//  The second to the next argument (first NULL if there isn't an argument). delimited by a space
//  Places a NULL at the first space after the first argument
void CLI_argumentIsolation( char* string, char** first, char** second )
{
}

// Scans the CLILineBuffer for any valid commands
void CLI_commandLookup()
{
}

// Registers a command dictionary with the CLI
void CLI_registerDictionary( const CLIDictItem *cmdDict, const char* dictName )
{
}

inline void CLI_tabCompletion()
{
}

inline int CLI_wrap( int kX, int const kLowerBound, int const kUpperBound )
{
	return 0;
}

inline void CLI_saveHistory( char *buff )
{
}

void CLI_retreiveHistory( int index )
{
}



// ----- CLI Command Functions -----

void cliFunc_clear( char* args)
{
}

void cliFunc_cliDebug( char* args )
{
}

void cliFunc_help( char* args )
{
}

void cliFunc_led( char* args )
{
}

void cliFunc_reload( char* args )
{
}

void cliFunc_reset( char* args )
{
}

void cliFunc_restart( char* args )
{
}

void cliFunc_version( char* args )
{
}

