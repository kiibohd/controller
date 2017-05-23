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

// Compiler Includes
#include <Lib/ScanLib.h>

// Project Includes
#include <cli.h>
#include <led.h>
#include <print.h>
#include <matrix_scan.h>
#include <macro.h>

// KLL Include
#include <kll.h>

// Local Includes
#include "scan_loop.h"



// ----- Function Declarations -----

// CLI Functions
void cliFunc_echo( char* args );



// ----- Variables -----

// Scan Module command dictionary
CLIDict_Entry( echo,        "Example command, echos the arguments." );

CLIDict_Def( scanCLIDict, "Scan Module Commands" ) = {
	CLIDict_Item( echo ),
	{ 0, 0, 0 } // Null entry for dictionary end
};

// Number of scans since the last USB send
uint16_t Scan_scanCount = 0;

uint8_t Scan_strobe_position;



// ----- Functions -----

// Setup
inline void Scan_setup()
{
	// Register Scan CLI dictionary
	CLI_registerDictionary( scanCLIDict, scanCLIDictName );

	// Setup GPIO pins for matrix scanning
	Matrix_setup();

	// Reset scan count
	Scan_scanCount = 0;

	// Reset starting strobe position
	Scan_strobe_position = 0;
}


// Main Detection Loop
inline uint8_t Scan_loop()
{
	// Scan Matrix
	Matrix_scan( Scan_scanCount, &Scan_strobe_position, 4 );

	// Check if we are ready to leave the scan loop
	if ( Scan_strobe_position >= Matrix_totalColumns() - 1 )
	{
		Scan_strobe_position = 0;
		Scan_scanCount++;
		return 0;
	}

	return 1;
}


// Signal from Macro Module that all keys have been processed (that it knows about)
inline void Scan_finishedWithMacro( uint8_t sentKeys )
{
}


// Signal from Output Module that all keys have been processed (that it knows about)
inline void Scan_finishedWithOutput( uint8_t sentKeys )
{
	// Reset scan loop indicator (resets each key debounce state)
	// TODO should this occur after USB send or Macro processing?
	Scan_scanCount = 0;
}


// Signal from the Output Module that the available current has changed
// current - mA
void Scan_currentChange( unsigned int current )
{
	// Indicate to all submodules current change
	Matrix_currentChange( current );
}



// ----- CLI Command Functions -----

// XXX Just an example command showing how to parse arguments (more complex than generally needed)
void cliFunc_echo( char* args )
{
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Parse args until a \0 is found
	while ( 1 )
	{
		print( NL ); // No \r\n by default after the command is entered

		curArgs = arg2Ptr; // Use the previous 2nd arg pointer to separate the next arg from the list
		CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

		// Stop processing args if no more are found
		if ( *arg1Ptr == '\0' )
			break;

		// Print out the arg
		dPrint( arg1Ptr );
	}
}

