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
#include <led.h>
#include <print.h>
#include <matrix_scan.h>
#include <macro.h>
#include <output_com.h>

// Local Includes
#include "scan_loop.h"



// ----- Function Declarations -----

// ----- Variables -----

// Number of scans since the last USB send
uint16_t Scan_scanCount = 0;



// ----- Functions -----

// Setup
inline void Scan_setup()
{
	// Setup cap sense matrix pins for scanning
	Matrix_setup();

	// Reset scan count
	Scan_scanCount = 0;
}


// Main Detection Loop
inline uint8_t Scan_loop()
{
	Matrix_scan( Scan_scanCount++ );

	return 0;
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



// ----- Capabilities -----

