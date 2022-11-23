/* Copyright (C) 2017-2020 by Jacob Alexander
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
#include <Lib/sam.h>
#include <Lib/storage.h>
#include <swd/swd_host.h>
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

#if DFU_EXTRA_BLE_SWD_SUPPORT == 1
uint32_t swd_flash_size;
uint32_t swd_part;
uint32_t ble_uniqueid[2];
FirmwareInfo ble_firmware_info;
#endif



// ----- Functions -----

// Modify indicator string
static void modify_indicator_string(uint16_t *indicator_string, uint16_t replacement)
{
	for ( uint8_t pos = 0; indicator_string[pos] != u'\0'; pos++ )
	{
		// Looking for | character
		if ( indicator_string[pos] == u'|' )
		{
			indicator_string[pos] = replacement;

			// If shortening, also change length
			if ( replacement == u'\0' )
			{
				dfu_device_str_desc[4]->bLength = pos * 2 + 2;
			}
		}
	}
}

#if DFU_EXTRA_BLE_SWD_SUPPORT == 1
// Setup SWD interface
static void swd_setup()
{
	// Setup SWDIO pins
	const GPIO_ConfigPin swdio_pin = periph_io(B,6,Input);
	const GPIO_ConfigPin swclk_pin = periph_io(B,7,Input);

	// Setup DAP and SWD
	PIO_Setup(swclk_pin);
	PIO_Setup(swdio_pin);

	// Halt (to make sure this part is reliable)
	if (!swd_set_target_state_hw(HALT))
	{
		// SWD Halt failed
		printNL("HALT failed!");
		swd_part = 0;
		goto cleanup;
	}

	// Determine device part (nRF52)
	uint32_t tmp = 0;
	if (swd_read_word(0x10000100, &tmp))
	{
		print("SWD Part: ");
		printHex(tmp);
		printNL();
		swd_part = tmp;
	}

	// Determine flash size (nRF52)
	if (swd_read_word(0x10000110, &tmp))
	{
		swd_flash_size = tmp * 1024;
		print("SWD Flash: ");
		printHex(swd_flash_size);
		printNL();
	}

	// Retrieve unique id (8 bytes)
	if (swd_read_memory(0x10000060, (uint8_t*)ble_uniqueid, 8))
	{
		print("BLE UniqueId: ");
		printHex(ble_uniqueid[0]);
		print(" ");
		printHex(ble_uniqueid[1]);
		printNL();
	}

	// Retrieve ble firmware revision (UICR Customer)
	if (swd_read_memory(0x10001080, (uint8_t*)&ble_firmware_info, sizeof(FirmwareInfo)))
	{
		print("BLE Firmware Rev: ");
		printHex(ble_firmware_info.revision);
		printNL();
	}

	// Run (we may not want to reset the SWD device)
	if (!swd_set_target_state_hw(RUN))
	{
		// SWD Run failed
		printNL("RUN failed!");
	}

cleanup:
	Reset_CleanupExternal();
	swd_off();
}
#endif

// Called early-on during ResetHandler
void Chip_reset()
{
	// Generating Secure Key
	printNL("Generating Secure Key...");

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
		printNL( "Secure Key Bypassed.");
		Chip_secure1 = 0;
		Chip_secure2 = 0;

		// Replace with \0 to hide part of string
		replacement = u'\0';
	}

	// Modify iInterface delimiter
	modify_indicator_string(dfu_device_str_desc[4]->bString, replacement);
#if DFU_EXTRA_BLE_SWD_SUPPORT == 1
	modify_indicator_string(dfu_device_str_desc[6]->bString, replacement);
#endif

	printNL("Secure Key Generated.");
}

// Called during bootloader initialization
void Chip_setup()
{
	// Disable WDT
	WDT->WDT_MR = WDT_MR_WDDIS;

	// Initialize non-volatile storage
	storage_init();
}

// Called during bootloader initialization after Device_setup()
void Chip_setup_delayed(bool alt_device)
{
	// Make sure USB transceiver is reset (in case we didn't do a full reset)
	udc_stop();

	// Check if this is an alt device
	if ( alt_device )
	{
		udc_ptr_top_conf = &udc_config_alt;
	}
	else
	{
		udc_ptr_top_conf = &udc_config;
	}

	// Start USB stack
	udc_start();

#if DFU_EXTRA_BLE_SWD_SUPPORT == 1
	// Setup SWD link and gather some basic information
	swd_setup();
#endif
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
		printNL("Flash was not cleared.");
		break;
	default:
		printNL("Flashed cleared!");
		break;
	}
	print(" Page: ");
	printHex( storage_page_position() );
	print( " Block: ");
	printHex( storage_block_position() );
	print( NL );

	// Clear the User Signature
	EraseUserSignature();
}

// Serial Number retrieval
void Chip_serial_number_setup()
{
	// MCU Serial number
	uint32ToHex16(sam_UniqueId[0], &(dfu_device_str_desc[3]->bString[8]));
	uint32ToHex16(sam_UniqueId[1], &(dfu_device_str_desc[3]->bString[16]));
	uint32ToHex16(sam_UniqueId[2], &(dfu_device_str_desc[3]->bString[24]));
	uint32ToHex16(sam_UniqueId[3], &(dfu_device_str_desc[3]->bString[32]));

	// Read firmware revision from flash
	FirmwareInfo mcu_info = ReadUserSignature();
	print("MCU Revision: ");
	printHex(mcu_info.revision);
	printNL();

	// Set firmware revision
	uint32ToHex16(mcu_info.revision, &(dfu_device_str_desc[3]->bString[47]));

#if DFU_EXTRA_BLE_SWD_SUPPORT == 1
	// Set BLE unique id
	uint32ToHex16(ble_uniqueid[0], &(dfu_device_str_desc[3]->bString[58]));
	uint32ToHex16(ble_uniqueid[1], &(dfu_device_str_desc[3]->bString[66]));

	// Set BLE revision
	uint32ToHex16(ble_firmware_info.revision, &(dfu_device_str_desc[3]->bString[82]));
#endif
}
