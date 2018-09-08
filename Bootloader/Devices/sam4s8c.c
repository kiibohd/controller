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
#include "flash_efc.h"
#include "udc.h"


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
	/* Disable WDT */
	WDT->WDT_MR = WDT_MR_WDDIS;

	/* Enable Debug LED */
	PIOB->PIO_PER = (1<<0);
	PIOB->PIO_OER = (1<<0);
	PIOB->PIO_SODR = (1<<0);

	/* Initialize flash: 6 wait states for flash writing. */
	uint32_t ul_rc;
	ul_rc = flash_init(FLASH_ACCESS_MODE_128, 6);
	if (ul_rc != FLASH_RC_OK) {
		return;
	}

	/* Start USB stack */
	udc_start();
}

// Called during each loop of the main bootloader sequence
void Chip_process()
{
	// Not locked up... Reset the watchdog timer
	WDT->WDT_CR = WDT_CR_KEY_PASSWD | WDT_CR_WDRSTT;
}

// Key validation
// Point to start of key
// Returns -1 if invalid
// Returns start-of-data offset if valid (may be unused until next block)
int8_t Chip_validation( uint8_t* key )
{
	// TODO
	return 0;
}

