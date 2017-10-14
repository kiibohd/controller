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
#include "../mchck.h"
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
	GPIOD_PDDR &= ~(1<<5);
	PORTD_PCR5 = PORT_PCR_PE | PORT_PCR_PFE | PORT_PCR_MUX(1);
	// Col1
	GPIOB_PDDR |= (1<<2);
	PORTB_PCR2 = PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOB_PSOR |= (1<<2);
}

// Called during each loop of the main bootloader sequence
void Device_process()
{
	// Check for S1 being pressed
	if ( GPIOD_PDIR & (1<<5) )
	{
		print( "Reset key pressed." NL );
		SOFTWARE_RESET();
	}
}

