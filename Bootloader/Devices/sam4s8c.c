/* Copyright (C) 2017-2019 by Jacob Alexander
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

//
// Kiibohd-dfu
//

// ----- Includes -----

// Project Includes
#include <Lib/entropy.h>
#include <Lib/gpio.h>
#include <Lib/storage.h>
#include "debug.h"

// ASF Includes
#include <sam/services/flash_efc/flash_efc.h>
#include <common/services/usb/udc/udc.h>

// Local Includes
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
		|| (Chip_secure1 == 0 && Chip_secure2 == 0)
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
	// Disable WDT
	WDT->WDT_MR = WDT_MR_WDDIS;

	// Initialize non-volatile storage
	storage_init();

	// Make sure USB transceiver is reset (in case we didn't do a full reset)
	udc_stop();

	// Start USB stack
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

// Called after firmware download has completed, but before reseting and jumping to firmware
void Chip_download_complete()
{
	// Make sure the last page of non-volatile memory has been cleared
	switch ( storage_clear_page() )
	{
	case 0:
		print("Flash was not cleared." NL);
		break;
	default:
		print("Flashed cleared!" NL);
		break;
	}
	print(" Page: ");
	printHex( storage_page_position() );
	print( " Block: ");
	printHex( storage_block_position() );
	print( NL );
}

