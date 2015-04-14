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

// ----- Includes -----

// Local Includes
#include "mchck.h"
#include "dfu.desc.h"



// ----- Variables -----

/**
 * Unfortunately we can't DMA directly to FlexRAM, so we'll have to stage here.
 */
static char staging[ FLASH_SECTOR_SIZE ];



// ----- Functions -----

static enum dfu_status setup_write( size_t off, size_t len, void **buf )
{
	GPIOA_PCOR |= (1<<5);
	static int last = 0;

	if ( len > sizeof(staging) )
		return (DFU_STATUS_errADDRESS);

	// We only allow the last write to be less than one sector size.
	if ( off == 0 )
		last = 0;
	if ( last && len != 0 )
		return (DFU_STATUS_errADDRESS);
	if ( len != FLASH_SECTOR_SIZE )
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
	if ( len == 0 )
		return (DFU_STATUS_OK);

	target = flash_get_staging_area(off + (uintptr_t)&_app_rom, FLASH_SECTOR_SIZE);
	if ( !target )
		return (DFU_STATUS_errADDRESS);
	memcpy( target, buf, len );

	// Depending on the error return a different status
	switch ( flash_program_sector(off + (uintptr_t)&_app_rom, FLASH_SECTOR_SIZE) )
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
	dfu_init(setup_write, finish_write, &dfu_ctx);
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
#else
#error "Incompatible chip for bootloader"
#endif

	//for (uint8_t c = 0; c < 20; c++)
	/*
	while( 1 )
	{
		GPIOA_PTOR |= (1<<5);
		for (uint32_t d = 0; d < 7200000; d++ );
	}
	*/

	// XXX REMOVEME
	/*
	GPIOB_PDDR |= (1<<16);
	PORTB_PCR16 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOB_PSOR |= (1<<16);
	*/
	// RST
	GPIOC_PDDR |= (1<<8);
	PORTC_PCR8 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOC_PSOR |= (1<<8);
	/*
	// CS1B
	GPIOC_PDDR |= (1<<4);
	PORTC_PCR4 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOC_PCOR |= (1<<4);
	*/
	// Backlight
	GPIOC_PDDR |= (1<<1);
	PORTC_PCR1 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOC_PCOR |= (1<<1);
	GPIOC_PDDR |= (1<<2);
	PORTC_PCR2 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOC_PCOR |= (1<<2);
	GPIOC_PDDR |= (1<<3);
	PORTC_PCR3 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOC_PCOR |= (1<<3);



	flash_prepare_flashing();

	uint32_t *position = &_app_rom;
	usb_init( &dfu_device );
	for (;;)
	{
		usb_poll();

		/*
		for ( ; position < &_app_rom + 0x201; position++ )
		//for ( ; position < &_app_rom + 0x800; position++ )
		{
			if ( *position != 0xFFFFFFFF )
			{
			while( 1 )
			{
				GPIOA_PTOR |= (1<<5);
				for (uint32_t d = 0; d < 7200000; d++ );
			}
			}
		}
		*/

	}
}

