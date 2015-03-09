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
static char staging[FLASH_SECTOR_SIZE];



// ----- Functions -----

static enum dfu_status setup_write(size_t off, size_t len, void **buf)
{
	static int last = 0;

	if (len > sizeof(staging))
		return (DFU_STATUS_errADDRESS);

	// We only allow the last write to be less than one sector size.
	if (off == 0)
		last = 0;
	if (last && len != 0)
		return (DFU_STATUS_errADDRESS);
	if (len != FLASH_SECTOR_SIZE) {
		last = 1;
		memset(staging, 0xff, sizeof(staging));
	}

	*buf = staging;
	return (DFU_STATUS_OK);
}

static enum dfu_status finish_write( void *buf, size_t off, size_t len )
{
	void *target;
	if (len == 0)
		return (DFU_STATUS_OK);

	target = flash_get_staging_area(off + (uintptr_t)&_app_rom, FLASH_SECTOR_SIZE);
	if (!target)
		return (DFU_STATUS_errADDRESS);
	memcpy(target, buf, len);
	if (flash_program_sector(off + (uintptr_t)&_app_rom, FLASH_SECTOR_SIZE) != 0)
		return (DFU_STATUS_errADDRESS);
	return (DFU_STATUS_OK);
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
	PORTA_PCR19 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOA_PSOR |= (1<<5);

#endif

	flash_prepare_flashing();

	usb_init( &dfu_device );
	for (;;)
	{
		usb_poll();
	}
}

