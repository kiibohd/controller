/* Copyright (c) 2011,2012 Simon Schubert <2@0x2c.org>.
 * Modifications by Jacob Alexander 2014-2015 <haata@kiibohd.com>
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

// ----- Local Includes -----

#include "mchck.h"
#include "debug.h"



// ----- Variables -----

uint32_t flash_ALLOW_BRICKABLE_ADDRESSES;



// ----- Functions -----

/* This will have to live in SRAM. */
__attribute__((section(".ramtext.ftfl_submit_cmd"), long_call))
int ftfl_submit_cmd()
{
	FTFL.fstat.raw = ((struct FTFL_FSTAT_t){
			.ccif = 1,
			//.rdcolerr = 1,
			.accerr = 1,
			.fpviol = 1
		}).raw;

	// Wait for the operation to complete
	struct FTFL_FSTAT_t stat;
	while (!(stat = FTFL.fstat).ccif); // XXX maybe WFI?

	// Mask error bits
	return stat.raw & (FTFL_FSTAT_RDCOLERR | FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL | FTFL_FSTAT_MGSTAT0);
	//return (!!stat.mgstat0);
}

int flash_prepare_flashing()
{
	/* switch to FlexRAM */
	if ( !FTFL.fcnfg.ramrdy )
	{
		FTFL.fccob.set_flexram.fcmd = FTFL_FCMD_SET_FLEXRAM;
		FTFL.fccob.set_flexram.flexram_function = FTFL_FLEXRAM_RAM;
		return (ftfl_submit_cmd());
	}
	return (0);
}

int flash_read_1s_sector( uintptr_t addr, size_t num )
{
	FTFL.fccob.read_1s_section.fcmd = FTFL_FCMD_READ_1s_SECTION;
	FTFL.fccob.read_1s_section.addr = addr;
	FTFL.fccob.read_1s_section.margin = FTFL_MARGIN_NORMAL;
	FTFL.fccob.read_1s_section.num_words = num;

	return ftfl_submit_cmd();
}

int flash_erase_sector( uintptr_t addr )
{
#ifdef FLASH_DEBUG
	// Debug
	print("Erasing Sector: address(");
	printHex( addr );
	printNL(")");
#endif

	if ( addr < (uintptr_t)&_app_rom && flash_ALLOW_BRICKABLE_ADDRESSES != 0x00023420 )
		return (-1);
	FTFL.fccob.erase.fcmd = FTFL_FCMD_ERASE_SECTOR;
	FTFL.fccob.erase.addr = addr;

	return ftfl_submit_cmd();
}

int flash_program_section( uintptr_t addr, size_t num )
{
#ifdef FLASH_DEBUG
	// Debug
	print("Programming Sector: address(");
	printHex( addr );
	print(") units (");
	printHex( num );
	printNL(")");
#endif

	FTFL.fccob.program_section.fcmd = FTFL_FCMD_PROGRAM_SECTION;
	FTFL.fccob.program_section.addr = addr;
	FTFL.fccob.program_section.num_words = num;

	return ftfl_submit_cmd();
}

int flash_program_sector( uintptr_t addr, size_t len )
{
	if ( len != USB_DFU_TRANSFER_SIZE )
		return 1;

#if defined(_mk20dx128vlf5_) || defined(_mk20dx128vlh7_)
	// Check if this is the beginning of a sector
	// Only erase if necessary
	if ( (addr & (FLASH_SECTOR_SIZE - 1)) == 0
		&& flash_read_1s_sector( addr, FLASH_SECTOR_SIZE / 4 )
		&& flash_erase_sector( addr ) )
			return 1;

	// Program sector (longword)
	return flash_program_section( addr, FLASH_SECTOR_SIZE / 4 );
#elif defined(_mk20dx256vlh7_)
	// Check if beginning of sector and erase if not empty
	// Each sector is 2 kB in length, but we can only write to half a sector at a time
	// We can only erase an entire sector at a time
	if ( (addr & (FLASH_SECTOR_SIZE - 1)) == 0
		&& flash_read_1s_sector( addr, FLASH_SECTOR_SIZE / 8 )
		&& flash_erase_sector( addr ) )
			return 1;

	// Program half-sector (phrases)
	return flash_program_section( addr, FLASH_SECTOR_SIZE / 16 );
#elif defined(_mk22fx512avlh12_)
	// Check if beginning of sector and erase if not empty
	// Each sector is 4 kB in length, but we can only write to half a sector at a time
	// We can only erase an entire sector at a time
	if ( (addr & (FLASH_SECTOR_SIZE - 1)) == 0
		&& flash_read_1s_sector( addr, FLASH_SECTOR_SIZE / 16 )
		&& flash_erase_sector( addr ) )
			return 1;

	// Program half-sector (double phrases)
	return flash_program_section( addr, FLASH_SECTOR_SIZE / 32 );
#endif
}

int flash_prepare_reading()
{
	return (0);
}

int flash_read_sector( uintptr_t addr, size_t len )
{
	return (0);
}

void *flash_get_staging_area( uintptr_t addr, size_t len )
{
	if ( (addr & (USB_DFU_TRANSFER_SIZE - 1)) != 0 || len != USB_DFU_TRANSFER_SIZE )
		return (NULL);
	return (FlexRAM);
}

