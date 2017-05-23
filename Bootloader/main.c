/* Copyright (c) 2011,2012 Simon Schubert <2@0x2c.org>.
 * Modifications by Jacob Alexander 2014-2016 <haata@kiibohd.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// ----- Includes -----

// Project Includes
#include <delay.h>

// Local Includes
#include "mchck.h"
#include "dfu.desc.h"

#include "debug.h"

#include "usb-internal.h"



// ----- Variables -----

/**
 * Unfortunately we can't DMA directly to FlexRAM, so we'll have to stage here.
 */
static char staging[ USB_DFU_TRANSFER_SIZE ];



// ----- Functions -----

int sector_print( void* buf, size_t sector, size_t chunks )
{
	uint8_t* start = (uint8_t*)buf + sector * USB_DFU_TRANSFER_SIZE;
	uint8_t* end = (uint8_t*)buf + (sector + 1) * USB_DFU_TRANSFER_SIZE;
	uint8_t* pos = start;

	// Verify if sector erased
	FTFL.fccob.read_1s_section.fcmd = FTFL_FCMD_READ_1s_SECTION;
	FTFL.fccob.read_1s_section.addr = (uintptr_t)start;
	FTFL.fccob.read_1s_section.margin = FTFL_MARGIN_NORMAL;
	FTFL.fccob.read_1s_section.num_words = 250; // 2000 kB / 64 bits
	int retval = ftfl_submit_cmd();

#ifdef FLASH_DEBUG
	print( NL );
	print("Block ");
	printHex( sector );
	print(" ");
	printHex( (size_t)start );
	print(" -> ");
	printHex( (size_t)end );
	print(" Erased: ");
	printHex( retval );
	print( NL );
#endif

	// Display sector
	for ( size_t line = 0; pos < end - 24; line++ )
	{
		// Each Line
		printHex_op( (size_t)pos, 4 );
		print(": ");

		// Each 2 byte chunk
		for ( size_t chunk = 0; chunk < chunks; chunk++ )
		{
			// Print out the two bytes (second one first)
			printHex_op( *(pos + 1), 2 );
			printHex_op( *pos, 2 );
			print(" ");
			pos += 2;
		}

		print( NL );
	}

	return retval;
}

static enum dfu_status setup_read( size_t off, size_t *len, void **buf )
{
	// Calculate starting address from offset
	*buf = (void*)&_app_rom + off;

	// Calculate length of transfer
	*len = *buf + USB_DFU_TRANSFER_SIZE > (void*)(&_app_rom_end)
		? (void*)(&_app_rom_end) - *buf + 1
		: USB_DFU_TRANSFER_SIZE;

	return (DFU_STATUS_OK);
}

static enum dfu_status setup_write( size_t off, size_t len, void **buf )
{
	static int last = 0;

#ifdef FLASH_DEBUG
	// Debug
	print("Setup Write: offset(");
	printHex( off );
	print(") len(");
	printHex( len );
	print(") last(");
	printHex( last );
	printNL(")");
#endif

	if ( len > sizeof(staging) )
		return (DFU_STATUS_errADDRESS);

	// We only allow the last write to be less than one sector size.
	if ( off == 0 )
		last = 0;
	if ( last && len != 0 )
		return (DFU_STATUS_errADDRESS);
	if ( len != USB_DFU_TRANSFER_SIZE )
	{
		last = 1;
		memset( staging, 0xff, sizeof(staging) );
	}

	*buf = staging;
	return (DFU_STATUS_OK);
}

static enum dfu_status finish_write( void *buf, size_t off, size_t len )
{
	void *target;

	// If nothing left to flash, this is still ok
	if ( len == 0 )
		return (DFU_STATUS_OK);

	// If the binary is larger than the internal flash, error
	if ( off + (uintptr_t)&_app_rom + len > (uintptr_t)&_app_rom_end )
		return (DFU_STATUS_errADDRESS);

	target = flash_get_staging_area( off + (uintptr_t)&_app_rom, USB_DFU_TRANSFER_SIZE );
	if ( !target )
		return (DFU_STATUS_errADDRESS);
	memcpy( target, buf, len );

	// Depending on the error return a different status
	switch ( flash_program_sector( off + (uintptr_t)&_app_rom, USB_DFU_TRANSFER_SIZE ) )
	{
	/*
	case FTFL_FSTAT_RDCOLERR: // Flash Read Collision Error
	case FTFL_FSTAT_ACCERR:   // Flash Access Error
	case FTFL_FSTAT_FPVIOL:   // Flash Protection Violation Error
		return (DFU_STATUS_errADDRESS);
	case FTFL_FSTAT_MGSTAT0:  // Memory Controller Command Completion Error
		return (DFU_STATUS_errADDRESS);
	*/

	case 0:
	default: // No error
		return (DFU_STATUS_OK);
	}
}


static struct dfu_ctx dfu_ctx;

void init_usb_bootloader( int config )
{
	dfu_init( setup_read, setup_write, finish_write, &dfu_ctx );

#if defined(_mk20dx256vlh7_) // Kiibohd-dfu
	// Make sure SysTick counter is disabled
	SYST_CSR = 0;
#endif
}

void main()
{
#if defined(_mk20dx128vlf5_) // Kiibohd-dfu / Infinity
	// XXX McHCK uses B16 instead of A19

	// Enabling LED to indicate we are in the bootloader
	GPIOA_PDDR |= (1<<19);
	// Setup pin - A19 - See Lib/pin_map.mchck for more details on pins
	PORTA_PCR19 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOA_PSOR |= (1<<19);

#elif defined(_mk20dx256vlh7_) // Kiibohd-dfu
	// Enabling LED to indicate we are in the bootloader
	GPIOA_PDDR |= (1<<5);
	// Setup pin - A5 - See Lib/pin_map.mchck for more details on pins
	PORTA_PCR5 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOA_PSOR |= (1<<5);

	// TODO Add CMake configuration for disabling
	// Set LCD backlight on ICED to Red
	GPIOC_PDDR |= (1<<1);
	PORTC_PCR1 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOC_PCOR |= (1<<1);
#else
#error "Incompatible chip for bootloader"
#endif

	uart_serial_setup();
	printNL( NL "Bootloader DFU-Mode" );

	// Bootloader Enter Reasons
	print(" RCM_SRS0 - ");
	printHex( RCM_SRS0 & 0x60 );
	print( NL " RCM_SRS1 - ");
	printHex( RCM_SRS1 & 0x02 );
	print( NL " _app_rom - ");
	printHex( (uint32_t)_app_rom );
	print( NL " Soft Rst - " );
	printHex( memcmp( (uint8_t*)&VBAT, sys_reset_to_loader_magic, sizeof(sys_reset_to_loader_magic) ) == 0 );
	print( NL );

#ifdef FLASH_DEBUG
	for ( uint8_t sector = 0; sector < 3; sector++ )
		sector_print( &_app_rom, sector, 16 );
	print( NL );
#endif

	flash_prepare_flashing();
	usb_init( &dfu_device );

#if defined(_mk20dx256vlh7_) // Kiibohd-dfu
	// PTA4 - USB Swap
	// Start, disabled
	GPIOA_PDDR |= (1<<4);
	PORTA_PCR4 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOA_PCOR |= (1<<4);

	#define USBPortSwapDelay_ms 1000
	// For keyboards with dual usb ports, doesn't do anything on keyboards without them
	// If a USB connection is not detected in 2 seconds, switch to the other port to see if it's plugged in there
	uint32_t last_ms = systick_millis_count;
	uint8_t attempt = 0;
	for (;;)
	{
		usb_poll();

		// Only check for swapping after delay
		uint32_t wait_ms = systick_millis_count - last_ms;
		if ( wait_ms < USBPortSwapDelay_ms + attempt / 2 * USBPortSwapDelay_ms )
		{
			continue;
		}

		last_ms = systick_millis_count;

		// USB not initialized, attempt to swap
		if ( usb.state != USBD_STATE_ADDRESS )
		{
			print("USB not initializing, port swapping (if supported)");
			GPIOA_PTOR |= (1<<4);
			attempt++;
		}
	}
#else
	for (;;)
	{
		usb_poll();
	}
#endif
}

