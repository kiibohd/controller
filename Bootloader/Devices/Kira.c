/* Copyright (C) 2017 by Jacob Alexander
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

//
// WhiteFox
//

// ----- Includes -----

// Local Includes
#include "../device.h"
#include "../debug.h"



// ----- Defines -----

// ----- Variables -----

// ----- Functions -----

// Called early-on during ResetHandler
void Device_reset()
{
}

// Called during bootloader initialization
void Device_setup()
{
	// Setup scanning for S1
	// Row1
	PIOA->PIO_PUDR = (1 << 26);
	PIOA->PIO_PPDER = (1 << 26);
	PIOA->PIO_IFER = (1 << 26);
	PIOA->PIO_ODR = (1 << 26);
	PIOA->PIO_PER = (1 << 26);

	// Col1
	PIOB->PIO_OER = (1 << 1);
	PIOB->PIO_PER = (1 << 1);
}

// Called during each loop of the main bootloader sequence
void Device_process()
{
	// Check for S1 being pressed
	// FIXME: Causing random resets
	/*if ( PIOA->PIO_PDSR & (1<<26) )
	{
		print( "Reset key pressed." NL );
		SOFTWARE_RESET();
	}*/
}

