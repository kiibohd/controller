/* Copyright (c) 2011,2012 Simon Schubert <2@0x2c.org>.
 * Modifications by Jacob Alexander 2014-2022 <haata@kiibohd.com>
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
#include <Lib/gpio.h>
#include <delay.h>

#if DFU_EXTRA_BLE_SWD_SUPPORT == 1
#include "swd/swd_host.h"
#endif

// Local Includes
#include "weak.h"
#include "device.h"
#include "debug.h"
#include "dfu.h"

#include "dfu.desc.h"

#if defined(_sam_)
#include <common/services/clock/osc.h>
#define WDT_TICK_US (128 * 1000000 / BOARD_FREQ_SLCK_XTAL)
#define WDT_MAX_VALUE 4095
#endif


// ----- Variables -----

/**
 * Unfortunately we can't DMA directly to FlexRAM, so we'll have to stage here.
 */
static uint8_t staging[USB_DFU_TRANSFER_SIZE];

// DFU State
struct dfu_ctx dfu_ctx;

extern uint32_t swd_flash_size;
extern uint32_t swd_part;



// ----- Functions -----

int sector_print( void* buf, size_t sector, size_t chunks )
{
	uint8_t* start = (uint8_t*)buf + sector * USB_DFU_TRANSFER_SIZE;
	uint8_t* end = (uint8_t*)buf + (sector + 1) * USB_DFU_TRANSFER_SIZE;
	uint8_t* pos = start;
	int retval = 0;

#if defined(_kinetis_)
	// Verify if sector erased
	FTFL.fccob.read_1s_section.fcmd = FTFL_FCMD_READ_1s_SECTION;
	FTFL.fccob.read_1s_section.addr = (uintptr_t)start;
	FTFL.fccob.read_1s_section.margin = FTFL_MARGIN_NORMAL;
	FTFL.fccob.read_1s_section.num_words = 250; // 2000 kB / 64 bits
	retval = ftfl_submit_cmd();
#endif

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

static enum dfu_status setup_read(size_t off, size_t *len, void **buf, uint8_t bAlternateSetting)
{
	switch (bAlternateSetting)
	{
	case 0: // DFU Upload for *this* MCU's flash
		// Calculate starting address from offset
		*buf = (void*)&_app_rom + off;

		// Calculate length of transfer
		*len = *buf + USB_DFU_TRANSFER_SIZE > (void*)(&_app_rom_end)
			? (void*)(&_app_rom_end) - *buf + 1
			: USB_DFU_TRANSFER_SIZE;
		break;

#if DFU_EXTRA_BLE_SWD_SUPPORT == 1
	case 1: // SWD Upload for BLE MCU
		// Don't even try if part is unknown
		if (!swd_part)
		{
			printNL("SWD Part Unknown!");
			return DFU_STATE_dfuERROR;
		}

		// Make sure we haven't already sent everything
		if (off >= swd_flash_size)
		{
			return DFU_STATE_dfuIDLE;
		}

		// Compute length of segment, the last segment may not be the full size
		*len = USB_DFU_TRANSFER_SIZE;
		if ((int32_t)swd_flash_size - off < USB_DFU_TRANSFER_SIZE)
		{
			*len = swd_flash_size - off + 1;
		}

		// Halt (to make sure this part is reliable)
		if (!swd_set_target_state_hw(HALT))
		{
			// SWD Halt failed
			printNL("HALT failed!");
			return DFU_STATE_dfuERROR;
		}

		// Read memory block
		if (!swd_read_memory(off, staging, *len))
		{
			printNL("Read failed!");
			return DFU_STATE_dfuERROR;
		}

		// Run
		if (!swd_set_target_state_hw(RUN))
		{
			// SWD Run failed
			printNL("RUN failed!");
			return DFU_STATE_dfuERROR;
		}

		// Set buffer
		*buf = staging;
		break;
#endif
	}
	return DFU_STATUS_OK;
}

static enum dfu_status setup_write(size_t off, size_t len, void **buf, uint8_t bAlternateSetting)
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
	print(") bAlternateSetting(");
	printHex(bAlternateSetting);
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

#if DFU_EXTRA_BLE_SWD_SUPPORT == 1
static void swd_wait_for_nvmc()
{
	uint32_t tmp = 0;
	while (tmp == 0)
	{
		swd_read_word(0x4001E400, &tmp); // Wait for 0x1 from NVMC_READY
	}
}
#endif

static enum dfu_status finish_write(void *buf, size_t off, size_t len, uint8_t bAlternateSetting)
{
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

	switch (bAlternateSetting)
	{
	case 0: // DFU Flashing *this* MCU's flash
#if defined(_sam_)
		// If this is the first block (or 2nd block after secure key), we might have the jump to SAM-BA bootloader sequence
		// This key is the chip unique id
		// It is also not allowed to jump to bootloader if the chip is in secure mode unless the one-time-key is prepended
		if (dfu_ctx.off == 0)
		{
			uint32_t *key = (uint32_t*)buf;
			bool full_reset = false;
			for (uint8_t pos = 0; pos < 4; pos++)
			{
				if (key[pos] == sam_UniqueId[pos] || __builtin_bswap32(key[pos]) == sam_UniqueId[pos])
				{
					if (pos == 3)
					{
						full_reset = true;
						break;
					}
					continue;
				}
				break;
			}

			if (full_reset)
			{
				// Reset GPNVM bits to jump back to SAM-BA
				print("Enabling ROM bootloader..." NL);
				EraseUserSignature(); // Make sure signature is erased to handle re-flash testing
				flash_clear_gpnvm(1);
				Reset_FullReset();
			}
		}
#endif

		// If the binary is larger than the internal flash, error
		if ( off + (uintptr_t)&_app_rom + len > (uintptr_t)&_app_rom_end )
		{
			return DFU_STATUS_errADDRESS;
		}

#if defined(_kinetis_)
		void *target = flash_get_staging_area( off + (uintptr_t)&_app_rom, USB_DFU_TRANSFER_SIZE );
		if ( !target )
		{
			return DFU_STATUS_errADDRESS;
		}
		memcpy( target, buf, len );

		// Depending on the error return a different status
		switch ( flash_program_sector( off + (uintptr_t)&_app_rom, USB_DFU_TRANSFER_SIZE ) )
		{
		case FTFL_FSTAT_RDCOLERR: // Flash Read Collision Error
		case FTFL_FSTAT_ACCERR:   // Flash Access Error
		case FTFL_FSTAT_FPVIOL:   // Flash Protection Violation Error
			return DFU_STATUS_errADDRESS;
		case FTFL_FSTAT_MGSTAT0:  // Memory Controller Command Completion Error
			return DFU_STATUS_errADDRESS;

		case 0:
		default: // No error
			break;
		}
#elif defined(_sam_)
		switch ( flash_program_sector( off + (uintptr_t)&_app_rom, staging, USB_DFU_TRANSFER_SIZE ) )
		{
		case FLASH_RC_OK:  // No error
			break;
		case FLASH_RC_ERROR:
		case FLASH_RC_INVALID:
		case FLASH_RC_NOT_SUPPORT:
		default:
			return DFU_STATUS_errADDRESS;
		}
#endif
		break;

#if DFU_EXTRA_BLE_SWD_SUPPORT == 1
	case 1: // SWD Flashing for BLE MCU
		// Don't even try if part is unknown
		if (!swd_part)
		{
			printNL("SWD Part Unknown!");
			return DFU_STATE_dfuERROR;
		}

		// If the binary is larger than the writable flash, error
		if (off + len > swd_flash_size)
		{
			return DFU_STATUS_errADDRESS;
		}

		// Halt (Needed for nRF52 flash reliability)
		if (!swd_set_target_state_hw(HALT))
		{
			// SWD Halt failed
			printNL("HALT failed!");
			return DFU_STATE_dfuERROR;
		}

		// If this is the first packet, erase the flash
		if (off == 0)
		{
			// Erase flash
			// Enable erase mode
			swd_write_word(0x4001E504, 0x2); // Write EEN to NVMC_CONFIG - Enables erase
			swd_wait_for_nvmc();

			// Erase all
			swd_write_word(0x4001E50C, 0x1); // Write 0x1 to NVMC_ERASEALL
			swd_wait_for_nvmc();
		}

		// Enable write mode
		swd_write_word(0x4001E504, 0x1); // Write WEN to NVMC_CONFIG - Enables write mode
		swd_wait_for_nvmc();

		// Write block
		swd_write_memory(off, buf, len);

		// Enable read mode
		swd_write_word(0x4001E504, 0x0); // Write REN to NVMC_CONFIG - Enables read mode
		swd_wait_for_nvmc();

		// Run
		if (!swd_set_target_state_hw(RUN))
		{
			// SWD Run failed
			printNL("RUN failed!");
			return DFU_STATE_dfuERROR;
		}
		break;
#endif
	}

	return DFU_STATUS_OK;
}

void init_usb_bootloader( int config )
{
	dfu_init( setup_read, setup_write, finish_write, &dfu_ctx );

#if defined(_kinetis_)
	// Make sure SysTick counter is disabled (dfu has issues otherwise)
	SYST_CSR = 0;
#endif

	// Clear verified status
	dfu_ctx.verified = DFU_VALIDATION_UNKNOWN;
}

// Code jump routine
__attribute__((noreturn))
static inline void jump_to_app( uintptr_t addr )
{
	// ARM-Cortex vector tables all begin with
	// the stack pointer, followed by reset handler

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
	// Bootloader Section
	extern uint32_t _app_rom;

	// Whether or not to enter the bootloader
	bool bootloader = false;

	// Prepared debug output (when supported)
	uart_serial_setup();
	printNL( NL "==> Bootloader" );

	// Early setup
	Chip_reset();
	Device_reset();

#if defined(_kinetis_)
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
#elif defined(_sam_)
	// Detected CPU
	print("CPU Id: ");
	printHex( SCB->CPUID );
	print( NL "Chip Id: ");
	printHex( CHIPID->CHIPID_CIDR );
	print( NL "Chip Ext: ");
	printHex( CHIPID->CHIPID_EXID );

	// Display DHCSR, see: https://developer.arm.com/documentation/ddi0337/e/CEGCJAHJ
	print( NL "DHCSR: ");
	printHex( *C_DHCSR );
	// Display DEMCR, see: https://developer.arm.com/documentation/ddi0337/e/CEGCHHJF
	print( NL "DEMCR: ");
	printHex( *C_DEMCR );

	// Bootloader Entry Reasons
	switch ( REG_RSTC_SR & RSTC_SR_RSTTYP_Msk ) {
	case RSTC_SR_RSTTYP_GeneralReset:
		// First power-up reset
		print( NL " GeneralReset");
		break;
	case RSTC_SR_RSTTYP_BackupReset:
		// Return from Backup Mode
		print( NL " BackupReset");
		break;
	case RSTC_SR_RSTTYP_WatchdogReset:
		// Watchdog fault occurred
		print( NL " WatchdogReset");

		// Check if we have the special reset to loader magic
		bootloader = memcmp( (uint8_t*)GPBR, sys_reset_to_loader_magic, sizeof(sys_reset_to_loader_magic) ) == 0;
		// Set the loader magic that we've already had a watchdog reset
		// Cleared by valid firmware
		for ( int pos = 0; pos <= sizeof(sys_reset_to_loader_magic)/4; pos++ )
			GPBR->SYS_GPBR[ pos ] = ((uint32_t*)sys_reset_to_loader_magic)[ pos ];
		break;
	case RSTC_SR_RSTTYP_SoftwareReset:
		// Processor reset required by the software
		print( NL " SoftwareReset");

		// Check if we have the special reset to loader magic
		bootloader = memcmp( (uint8_t*)GPBR, sys_reset_to_loader_magic, sizeof(sys_reset_to_loader_magic) ) == 0;
		break;
	case RSTC_SR_RSTTYP_UserReset:
		// NRST pin detected low
		print( NL " UserReset");
		bootloader = true;

		break;
	}
	print( NL " _app_rom - ");
	printHex( (uint32_t)_app_rom );
	// Check for unflashed firmware (always jump to bootloader)
	if ( _app_rom == 0xFFFFFFFF )
	{
		bootloader = true;
	}
	print( NL " Soft Rst - " );
	printHex( memcmp( (uint8_t*)GPBR, sys_reset_to_loader_magic, sizeof(sys_reset_to_loader_magic) ) == 0 );
	printNL();
#endif

#if defined(_kinetis_)
	// We treat _app_rom as pointer to directly read the stack
	// pointer and check for valid app code.  This is no fool
	// proof method, but it should help for the first flash.
	//
	// Rather than checking the watchdog signal, look for the sys_reset_to_loader_magic
	// sequence. If not set after a watchdog, try to boot the firmware again.
	// Otherwise if set, that means the firmware didn't fully initialize and go back to the bootloader
	//
	// Also checking for ARM lock-up signal (invalid firmware image)
	// RCM_SRS1 & 0x02
	if (    // PIN  (External Reset Pin/Switch)
		RCM_SRS0 & 0x40
		// LOCKUP (ARM Core LOCKUP event)
		|| RCM_SRS1 & 0x02
		// Blank flash check
		|| _app_rom == 0xffffffff
		// Software reset
		|| memcmp( (uint8_t*)&VBAT, sys_reset_to_loader_magic, sizeof(sys_reset_to_loader_magic) ) == 0
	)
	{
		printNL("-> DFU-Mode");
		// Bootloader mode
		memset( (uint8_t*)&VBAT, 0, sizeof(sys_reset_to_loader_magic) );
	}
	else
	{
		// Cleared by valid firmwre
		for ( int pos = 0; pos < sizeof(sys_reset_to_loader_magic); pos++ )
			(&VBAT)[ pos ] = sys_reset_to_loader_magic[ pos ];

		// Firmware mode
		print( NL "==> Booting Firmware..." NL );
		uint32_t addr = (uintptr_t)&_app_rom;
		SCB_VTOR = addr; // relocate vector table
		jump_to_app( addr );
	}
#elif defined(_sam_)
	if (bootloader)
	{
		printNL("-> DFU-Mode");
		// Bootloader mode
		for ( int pos = 0; pos <= sizeof(sys_reset_to_loader_magic)/sizeof(GPBR->SYS_GPBR[0]); pos++ )
			GPBR->SYS_GPBR[ pos ] = 0x00000000;
	}
	else
	{
		// Enable Watchdog before jumping
		/*
		// XXX (HaaTa) This watchdog cannot trigger an IRQ, as we're relocating the vector table
#if defined(DEBUG) && defined(JLINK)
		WDT->WDT_MR = WDT_MR_WDV(1000000 / WDT_TICK_US) | WDT_MR_WDD(WDT_MAX_VALUE) | WDT_MR_WDFIEN | WDT_MR_WDDBGHLT | WDT_MR_WDIDLEHLT;
		//WDT->WDT_MR = WDT_MR_WDDIS;
#else
		WDT->WDT_MR = WDT_MR_WDV(1000000 / WDT_TICK_US) | WDT_MR_WDD(WDT_MAX_VALUE) | WDT_MR_WDRSTEN | WDT_MR_WDRPROC | WDT_MR_WDDBGHLT | WDT_MR_WDIDLEHLT;
#endif
		*/
		WDT->WDT_MR = WDT_MR_WDDIS;

#if DFU_EXTRA_BLE_SWD_SUPPORT == 1
		// Cleanup external reset
		Reset_CleanupExternal();

		// Disable debug interface
		// Halt (to make sure this part is reliable)
		swd_set_target_state_hw(HALT);
		swd_write_dp(DP_SELECT, 0);
		swd_write_dp(DP_CTRL_STAT, 0);

		// Turn off SWD port
		swd_off();

		// Reset System Mux
		MATRIX->CCFG_SYSIO = 0;
#endif

		// Firmware mode
		printNL(NL "==> Booting Firmware...");
		uint32_t addr = (uintptr_t)&_app_rom;
		SCB->VTOR = ((uint32_t) addr); // relocate vector table
		jump_to_app( addr );
	}
#endif

	// Device/Chip specific setup
	Chip_setup();
	bool alt_device = false;
	Device_setup(&alt_device);
#if defined(_sam_)
	Chip_setup_delayed(alt_device);
#endif

#ifdef FLASH_DEBUG
	for ( uint8_t sector = 0; sector < 3; sector++ )
	{
		sector_print( &_app_rom, sector, 16 );
	}
	print( NL );
#endif

#if defined(_kinetis_)
	flash_prepare_flashing();
#endif
	dfu_usb_init(alt_device); // Initialize USB and dfu

	// Main Loop
	for (;;)
	{
#if defined(_kinetis_)
		// Stroke watchdog
		if ( WDOG_TMROUTL > 2 )
		{
			WDOG_REFRESH = WDOG_REFRESH_SEQ1;
			WDOG_REFRESH = WDOG_REFRESH_SEQ2;
		}

		dfu_usb_poll();
#endif

		// Device specific functions
		Chip_process();
		Device_process();
	}
}
