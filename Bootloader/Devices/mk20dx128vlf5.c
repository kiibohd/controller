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
// Kiibohd-dfu / Infinity 60% / Infinity 60% LED
//

// ----- Includes -----

// Local Includes
#include "../mchck.h"



// ----- Defines -----

// ----- Variables -----

// ----- Functions -----

// Called early-on during ResetHandler
inline void Chip_reset( uint8_t bootToFirmware )
{
}

// Called during bootloader initialization
inline void Chip_setup()
{
	// XXX McHCK uses B16 instead of A19

	// Enabling LED to indicate we are in the bootloader
	GPIOA_PDDR |= (1<<19);
	// Setup pin - A19 - See Lib/pin_map.mchck for more details on pins
	PORTA_PCR19 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOA_PSOR |= (1<<19);

}

// Called during each loop of the main bootloader sequence
inline void Chip_process()
{
}

