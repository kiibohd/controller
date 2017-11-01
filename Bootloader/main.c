/* Copyright (c) 2011,2012 Simon Schubert <2@0x2c.org>.
 * Modifications by Jacob Alexander 2014-2017 <haata@kiibohd.com>
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
#include "weak.h"
#include "mchck.h"
#include "dfu.desc.h"

#include "debug.h"



// ----- Variables -----

/**
 * Unfortunately we can't DMA directly to FlexRAM, so we'll have to stage here.
 */
static char staging[ USB_DFU_TRANSFER_SIZE ];

// DFU State
static struct dfu_ctx dfu_ctx;



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

	return DFU_STATUS_OK;
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
	{
		return DFU_STATUS_errADDRESS;
	}

	// We only allow the last write to be less than one sector size.
	if ( off == 0 )
	{
		last = 0;
	}
	if ( last && len != 0 )
	{
		return DFU_STATUS_errADDRESS;
	}
	if ( len != USB_DFU_TRANSFER_SIZE )
	{
		last = 1;
		memset( staging, 0xff, sizeof(staging) );
	}

	*buf = staging;
	return DFU_STATUS_OK;
}

static enum dfu_status finish_write( void *buf, size_t off, size_t len )
{
	void *target;

	// If nothing left to flash, this is still ok
	if ( len == 0 )
	{
		return DFU_STATUS_OK;
	}

	if ( off == 0 && dfu_ctx.verified == DFU_VALIDATION_UNKNOWN )
	{
		// Reset offset
		dfu_ctx.off = 0;

		// First block, if using Chip_validation, skip flashing this block and use for key validation
		// When key disabled, we supported a key'd file OR a non-key'd file
		switch ( Chip_validation( (uint8_t*)buf ) )
		{
		// Key disabled, no key
		case 0:
			dfu_ctx.verified = DFU_VALIDATION_OK;
			break;

		// Invalid key
		case -1:
			dfu_ctx.verified = DFU_VALIDATION_FAILED;
			return DFU_STATUS_errFILE;

		// Valid key, or Key disabled and a key.
		default:
			dfu_ctx.verified = DFU_VALIDATION_PENDING;
			print( "Valid firmware key" NL );

			// Do not use this block
			return DFU_STATUS_OK;
		}
	}

	// If the binary is larger than the internal flash, error
	if ( off + (uintptr_t)&_app_rom + len > (uintptr_t)&_app_rom_end )
	{
		return DFU_STATUS_errADDRESS;
	}

	target = flash_get_staging_area( off + (uintptr_t)&_app_rom, USB_DFU_TRANSFER_SIZE );
	if ( !target )
	{
		return DFU_STATUS_errADDRESS;
	}
	memcpy( target, buf, len );

	// Depending on the error return a different status
	switch ( flash_program_sector( off + (uintptr_t)&_app_rom, USB_DFU_TRANSFER_SIZE ) )
	{
	/*
	case FTFL_FSTAT_RDCOLERR: // Flash Read Collision Error
	case FTFL_FSTAT_ACCERR:   // Flash Access Error
	case FTFL_FSTAT_FPVIOL:   // Flash Protection Violation Error
		return DFU_STATUS_errADDRESS;
	case FTFL_FSTAT_MGSTAT0:  // Memory Controller Command Completion Error
		return DFU_STATUS_errADDRESS;
	*/

	case 0:
	default: // No error
		return DFU_STATUS_OK;
	}
}

void init_usb_bootloader( int config )
{
	dfu_init( setup_read, setup_write, finish_write, &dfu_ctx );

	// Make sure SysTick counter is disabled (dfu has issues otherwise)
	SYST_CSR = 0;

	// Clear verified status
	dfu_ctx.verified = DFU_VALIDATION_UNKNOWN;
}

// Code jump routine
__attribute__((noreturn))
static inline void jump_to_app( uintptr_t addr )
{
	// addr is in r0
	__asm__("ldr sp, [%[addr], #0]\n"
		"ldr pc, [%[addr], #4]"
		:: [addr] "r" (addr));
	// NOTREACHED
	__builtin_unreachable();
}

// Main entry point
// NOTE: Code does not start here, see Lib/mk20dx.c
void main()
{
	// Prepared debug output (when supported)
	uart_serial_setup();
	printNL( NL "Bootloader DFU-Mode" );

	// Early setup
	Chip_reset();
	Device_reset();

	// Bootloader Section
	extern uint32_t _app_rom;

	// We treat _app_rom as pointer to directly read the stack
	// pointer and check for valid app code.  This is no fool
	// proof method, but it should help for the first flash.
	//
	// Purposefully disabling the watchdog *after* the reset check this way
	// if the chip goes into an odd state we'll reset to the bootloader (invalid firmware image)
	// RCM_SRS0 & 0x20
	//
	// Also checking for ARM lock-up signal (invalid firmware image)
	// RCM_SRS1 & 0x02
	if (    // PIN  (External Reset Pin/Switch)
		RCM_SRS0 & 0x40
		// WDOG (Watchdog timeout)
		|| RCM_SRS0 & 0x20
		// LOCKUP (ARM Core LOCKUP event)
		|| RCM_SRS1 & 0x02
		// Blank flash check
		|| _app_rom == 0xffffffff
		// Software reset
		|| memcmp( (uint8_t*)&VBAT, sys_reset_to_loader_magic, sizeof(sys_reset_to_loader_magic) ) == 0
	)
	{
		// Bootloader mode
		memset( (uint8_t*)&VBAT, 0, sizeof(sys_reset_to_loader_magic) );
	}
	else
	{
		// Enable Watchdog before jumping
		// XXX (HaaTa) This watchdog cannot trigger an IRQ, as we're relocating the vector table
		WDOG_UNLOCK = WDOG_UNLOCK_SEQ1;
		WDOG_UNLOCK = WDOG_UNLOCK_SEQ2;
		WDOG_TOVALH = 0;
		WDOG_TOVALL = 1000;
		WDOG_STCTRLH |= WDOG_STCTRLH_WDOGEN;

		// Firmware mode
		uint32_t addr = (uintptr_t)&_app_rom;
		SCB_VTOR = addr; // relocate vector table
		jump_to_app( addr );
	}

	// Detected CPU
	print("CPU Id: ");
	printHex( SCB_CPUID );
	print( NL "Device Id: ");
	printHex( SIM_SDID );
	print( NL "Flash CFG: ");
	printHex( SIM_FCFG1 & 0xFFFFFFF0 );
	print( NL "RAM: ");
	printHex( SIM_SOPT1_RAMSIZE );

	// Bootloader Entry Reasons
	print( NL " RCM_SRS0 - ");
	printHex( RCM_SRS0 & 0x60 );
	print( NL " RCM_SRS1 - ");
	printHex( RCM_SRS1 & 0x02 );
	print( NL " _app_rom - ");
	printHex( (uint32_t)_app_rom );
	print( NL " Soft Rst - " );
	printHex( memcmp( (uint8_t*)&VBAT, sys_reset_to_loader_magic, sizeof(sys_reset_to_loader_magic) ) == 0 );
	print( NL );

	// Device/Chip specific setup
	Chip_setup();
	Device_setup();

#ifdef FLASH_DEBUG
	for ( uint8_t sector = 0; sector < 3; sector++ )
	{
		sector_print( &_app_rom, sector, 16 );
	}
	print( NL );
#endif

	flash_prepare_flashing();
	dfu_usb_init(); // Initialize USB and dfu

	// Main Loop
	for (;;)
	{
		dfu_usb_poll();

		// Device specific functions
		Chip_process();
		Device_process();
	}
}

