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
// Infinity Ergodox
//

// ----- Includes -----

// Local Includes
#include "../mchck.h"



// ----- Defines -----

// ----- Variables -----

// ----- Functions -----

// Called early-on during ResetHandler
inline void Device_reset( uint8_t bootToFirmware )
{
}

// Called during bootloader initialization
inline void Device_setup()
{
	// Set LCD backlight on ICED to Red
	GPIOC_PDDR |= (1<<1);
	PORTC_PCR1 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOC_PCOR |= (1<<1);
}

// Called during each loop of the main bootloader sequence
inline void Device_process()
{
}

