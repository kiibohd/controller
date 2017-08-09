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
// Kiibohd-dfu
// Infinity Ergodox / WhiteFox / K-Type
//

// ----- Includes -----

// Local Includes
#include "../mchck.h"
#include "../debug.h"



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
	// Enabling LED to indicate we are in the bootloader
	GPIOA_PDDR |= (1<<5);
	// Setup pin - A5 - See Lib/pin_map.mchck for more details on pins
	PORTA_PCR5 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOA_PSOR |= (1<<5);

	// TODO Disable this by default
	print( " Secure Code - ");
	uint8_t *vbat_reg = (uint8_t*)&VBAT;
	// Start at byte 24 and 28
	uint32_t *secure1 = (uint32_t*)&vbat_reg[24];
	uint32_t *secure2 = (uint32_t*)&vbat_reg[28];
	printHex_op( *secure1, 8 );
	printHex_op( *secure2, 8 );
	print( NL );
}

// Called during each loop of the main bootloader sequence
inline void Chip_process()
{
}

