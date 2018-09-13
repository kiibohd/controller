/* Copyright (C) 2017-2018 by Jacob Alexander
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
// Kira
//

// ----- Includes -----

// Project Includes
#include <Lib/entropy.h>
#include "debug.h"

// Local Includes
#include "flash_efc.h"
#include "udc.h"
#include "../dfu.desc.h"


// ----- Defines -----

// ----- Variables -----

uint32_t Chip_secure1;
uint32_t Chip_secure2;



// ----- Functions -----

// Called early-on during ResetHandler
void Chip_reset()
{
	// Generating Secure Key
	print( "Generating Secure Key..." NL );

	// Read current 64 bit secure number
	Chip_secure1 = GPBR_SECURE1;
	Chip_secure2 = GPBR_SECURE2;

	// Generate 64 bit random numbers
	while ( !rand_available() );
	GPBR_SECURE1 = rand_value32();
	while ( !rand_available() );
	GPBR_SECURE2 = rand_value32();

	// Disable rand generation
	rand_disable();

	// Secure indicator string (lsusb), iInterface
	uint16_t *indicator_string = dfu_device_str_desc[4]->bString;
	uint16_t replacement = u' '; // Replace with space in secure mode

	// If using an external reset, disable secure validation
	// Or if the flash is blank
	if (    // PIN  (External Reset Pin/Switch)
                (REG_RSTC_SR & RSTC_SR_RSTTYP_Msk) == RSTC_SR_RSTTYP_UserReset
                // WDOG (Watchdog timeout)
                || (REG_RSTC_SR & RSTC_SR_RSTTYP_Msk) == RSTC_SR_RSTTYP_WatchdogReset
                // Blank flash check
                || _app_rom == 0xffffffff
        )
	{
		print( "Secure Key Bypassed." NL );
		Chip_secure1 = 0;
		Chip_secure2 = 0;

		// Replace with \0 to hide part of string
		replacement = u'\0';
	}

	// Modify iInterface delimiter
	for ( uint8_t pos = 0; indicator_string[ pos ] != u'\0'; pos++ )
	{
		// Looking for | character
		if ( indicator_string[ pos ] == u'|' )
		{
			indicator_string[ pos ] = replacement;

			// If shortening, also change length
			if ( replacement == u'\0' )
			{
				dfu_device_str_desc[4]->bLength = pos * 2 + 2;
			}
		}
	}

	print( "Secure Key Generated." NL );
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
	/*uint32_t ul_rc;
	ul_rc = flash_init(FLASH_ACCESS_MODE_128, 6);
	if (ul_rc != FLASH_RC_OK) {
		return;
	}*/

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
	// Ignore check if set to 0s
	if ( Chip_secure1 == 0 && Chip_secure2 == 0 )
	{
		// Check to see if there's a key set, start after the key section
		// Block size is 1024 (0x400)
		uint8_t key_section = 8;
		for ( uint16_t byte = key_section; byte < 1024; byte++ )
		{
			// If anything isn't zero, this is a data section
			if ( key[byte] != 0 )
			{
				key_section = 0;
			}
		}

		return key_section;
	}

	// Check first 32 bits, then second 32 bits of incoming key
	if (
		*(uint32_t*)&key[0] == Chip_secure1
		&& *(uint32_t*)&key[4] == Chip_secure2
	)
	{
		return 8;
	}

	// Otherwise, an invalid key
	print( "Invalid firmware key!" NL );
	return -1;
}

