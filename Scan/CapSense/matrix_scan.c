/* Copyright (C) 2016 by Jacob Alexander
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
#include <Lib/ScanLib.h>

// Project Includes
#include <cli.h>
#include <kll_defs.h>
#include <led.h>
#include <print.h>
#include <macro.h>
#include <Lib/delay.h>

// Local Includes
#include "matrix_scan.h"

// Matrix Configuration
//#include <matrix.h>



// ----- Defines -----

// ----- Function Declarations -----

// CLI Functions
void cliFunc_matrixDebug( char* args );
void cliFunc_matrixInfo( char* args );
void cliFunc_matrixState( char* args );



// ----- Variables -----

// Scan Module command dictionary
CLIDict_Entry( matrixDebug,  "Enables matrix debug mode, prints out each scan code." NL "\t\tIf argument \033[35mT\033[0m is given, prints out each scan code state transition." );
CLIDict_Entry( matrixInfo,   "Print info about the configured matrix." );
CLIDict_Entry( matrixState,  "Prints out the current scan table N times." NL "\t\t \033[1mO\033[0m - Off, \033[1;33mP\033[0m - Press, \033[1;32mH\033[0m - Hold, \033[1;35mR\033[0m - Release, \033[1;31mI\033[0m - Invalid" );

CLIDict_Def( matrixCLIDict, "Matrix Module Commands" ) = {
	CLIDict_Item( matrixDebug ),
	CLIDict_Item( matrixInfo ),
	CLIDict_Item( matrixState ),
	{ 0, 0, 0 } // Null entry for dictionary end
};



// ----- Functions -----

void Matrix_setup()
{
	// Register Matrix CLI dictionary
	CLI_registerDictionary( matrixCLIDict, matrixCLIDictName );
}

// Scan the matrix for keypresses
// NOTE: scanNum should be reset to 0 after a USB send (to reset all the counters)
void Matrix_scan( uint16_t scanNum )
{
}


// Called by parent scan module whenever the available current changes
// current - mA
void Matrix_currentChange( unsigned int current )
{
	// TODO - Any potential power savings?
}



// ----- CLI Command Functions -----

void cliFunc_matrixInfo( char* args )
{
}

void cliFunc_matrixDebug( char* args )
{
	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	// Set the matrix debug flag depending on the argument
	// If no argument, set to scan code only
	// If set to T, set to state transition
	switch ( arg1Ptr[0] )
	{
	// T as argument
	case 'T':
	case 't':
		break;

	// No argument
	case '\0':
		break;

	// Invalid argument
	default:
		return;
	}
}

void cliFunc_matrixState( char* args )
{
}

