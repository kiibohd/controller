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

// Project Includes
#include <Lib/entropy.h>

// Local Includes
#include "../weak.h"
#include "../device.h"
#include "../debug.h"
#include "../dfu.desc.h"



// ----- Defines -----

// ----- Variables -----

uint32_t Chip_secure1;
uint32_t Chip_secure2;



// ----- Functions -----

// Called early-on during ResetHandler
void Chip_reset()
{
}

// Called during bootloader initialization
void Chip_setup()
{
	//EFC0->EEFC_FMR = EEFC_FMR_FWS(0x#);
}

// Called during each loop of the main bootloader sequence
void Chip_process()
{
}

// Key validation
// Point to start of key
// Returns -1 if invalid
// Returns start-of-data offset if valid (may be unused until next block)
int8_t Chip_validation( uint8_t* key )
{
	return -1;
}

