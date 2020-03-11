/**
 * \file
 *
 * \brief In system programming to control security, memories and fuse bits
 *
 * Copyright (c) 2011 - 2014 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
 /**
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#include <common/services/isp/flip/module_config/conf_isp.h>
#include <common/services/isp/flip/isp.h>
#include "string.h"

#include "boot.h"
#include <Lib/sam.h>
#include <sam/utils/cmsis/sam4s/include/sam4s.h>
#include <sam/services/flash_efc/flash_efc.h>

/**
 * \ingroup isp
 * \defgroup isp_implementation In System Programming XMEGA implementation
 * This implementation links the XMEGA NVM functions to the common isp API.
 * Also, it manage :
 * - the flash access security
 * - the JTAG ID information storage
 * - the bootloader version storage
 * - the start appli operation through software reset
 *
 * @{
 */

/**
 * \name Memory APIs
 */
//@{

/**
 * \name Specific memories
 */
//@{

//! Memory signature that stores information about the device
static isp_mem_signature_t mem_signature;

//! Memory bootloader that stores the bootloader version
static isp_mem_bootloader_t mem_bootloader = {
	.version = BOOTLOADER_VERSION,
	.id1 = 0,
	.id2 = 0,
};
//@}

/**
 * \brief  Copy a flash memory section to a RAM buffer
 *
 * \param dst    Pointer to data destination.
 * \param src    Pointer to source flash.
 * \param nbytes Number of bytes to transfer.
 */
static void mem_flash_read(void *dst, isp_addr_t src, uint16_t nbytes)
{
	for (uint16_t i=0; i<nbytes; i++) {
		((uint8_t*)dst)[i] = ((uint8_t*)src)[i];
	}
}

/**
 * \brief  Copy a RAM buffer to a flash memory section
 *
 * \param dst    Pointer to flash destination.
 * \param src    Pointer to source data.
 * \param nbytes Number of bytes to transfer.
 */

// The rest of the array is composed of 64-Kbyte sectors of 128 pages, each page of 512 bytes
#define FLASH_SECTOR_SIZE 0x10000 //64K
#define LOCK_REGION_SIZE  0x2000 //8K

static void mem_flash_write(isp_addr_t dst, const void *src, uint16_t nbytes)
{
	uint32_t ul_rc;

	if (((dst - IFLASH0_ADDR) % LOCK_REGION_SIZE) == 0) {
		/* Unlock page */
		//ul_rc = flash_unlock(dst, dst+nbytes-1, 0, 0);
		ul_rc = flash_unlock(dst, dst+LOCK_REGION_SIZE-1, 0, 0);
		if (ul_rc != FLASH_RC_OK) {
			return;
		}

		/* The EWP command is not supported for non-8KByte sectors in all devices
		 *  SAM4 series, so an erase command is requried before the write operation.
		 */
		//ul_rc = flash_erase_sector(dst);
		ul_rc = flash_erase_page(dst, IFLASH_ERASE_PAGES_16);
		if (ul_rc != FLASH_RC_OK) {
			return;
		}
	}

	/* Write page */
	flash_write(dst, src, nbytes, 0);
}

/**
 * \brief  Copy a bootloader version memory section to a RAM buffer
 *
 * \param dst    Pointer to data destination.
 * \param src    Pointer to source memory.
 * \param nbytes Number of bytes to transfer.
 */
static void mem_bootloader_read(void *dst, isp_addr_t src, uint16_t nbytes)
{
	//memcpy(dst, (uint8_t*)&mem_bootloader + src, nbytes);
	for (uint16_t i=0; i<nbytes; i++) {
		((uint8_t*)dst)[i] = ((uint8_t*)src)[i];
	}
}

/**
 * \brief  Copy a signature memory section to a RAM buffer
 *
 * \param dst    Pointer to data destination.
 * \param src    Pointer to source memory.
 * \param nbytes Number of bytes to transfer.
 */
static void mem_signature_read(void *dst, isp_addr_t src, uint16_t nbytes)
{
	//memcpy(dst, (uint8_t*)&mem_signature + src, nbytes);
	for (uint16_t i=0; i<nbytes; i++) {
		((uint8_t*)dst)[i] = ((uint8_t*)src)[i];
	}
}

//! Interface for memory flash
const isp_mem_t isp_flash = {
	.size        = IFLASH0_ADDR + IFLASH_SIZE,
	.fnct_read   = mem_flash_read,
	.fnct_write  = mem_flash_write,
};

//! Interface for memory bootloader version
const isp_mem_t isp_bootloader = {
	.size        = sizeof(mem_bootloader),
	.fnct_read   = mem_bootloader_read,
	.fnct_write  = NULL,
};

//! Interface for memory signature
const isp_mem_t isp_signature = {
	.size        = sizeof(mem_signature),
	.fnct_read   = mem_signature_read,
	.fnct_write  = NULL,
};

//! Interface for memory no available
const isp_mem_t isp_no_available = {
	.size        = 0,
	.fnct_read   = NULL,
	.fnct_write  = NULL,
};

//! \name Memories list declaration
const isp_mems_t isp_memories = {
	.list = {
	.flash       = &isp_flash,
	.eeprom      = &isp_no_available,
	.security    = &isp_no_available,
	.conf        = &isp_no_available,
	.bootloader  = &isp_bootloader,
	.signature   = &isp_signature,
	.user        = &isp_no_available,
	.int_ram     = &isp_no_available,
	.ext_mem_cs0 = &isp_no_available,
	.ext_mem_cs1 = &isp_no_available,
	.ext_mem_cs2 = &isp_no_available,
	.ext_mem_cs3 = &isp_no_available,
	.ext_mem_cs4 = &isp_no_available,
	.ext_mem_cs5 = &isp_no_available,
	.ext_mem_cs6 = &isp_no_available,
	.ext_mem_cs7 = &isp_no_available,
	.ext_mem_df =  &isp_no_available,
	}
};

//@}


/**
 * \name Miscellaneous functions of the In System Programming module
 */
//@{

void isp_init(void)
{
	// These can be changed
	mem_signature.manufacture = 0x12;
	mem_signature.product_number_msb = 0x34;
	mem_signature.product_number_lsb = 0x56;
	mem_signature.product_revision = 0x78;
}


bool isp_is_security(void)
{
	return flash_is_security_bit_enabled();
}


bool isp_erase_chip(void)
{
	flash_unlock(IFLASH0_ADDR, IFLASH0_ADDR+IFLASH_SIZE, 0, 0);
	return flash_erase_all(IFLASH0_ADDR);
}

bool isp_erase_chip_split(void)
{
	flash_unlock(IFLASH0_ADDR, IFLASH0_ADDR+IFLASH_SIZE, 0, 0);
	uint32_t rc = flash_erase_all(IFLASH0_ADDR);
	return rc == FLASH_RC_OK;

	/*static uint16_t isp_page_number=0;
	uint8_t isp_page_number_split;

	uint32_t pul_flash_descriptor;
	flash_get_descriptor(IFLASH0_ADDR, &pul_flash_descriptor, IFLASH_SIZE);

	if (isp_page_number==0) {
		isp_page_number = flash_get_page_count(&pul_flash_descriptor);
		flash_lock(IFLASH0_ADDR, IFLASH0_ADDR+IFLASH_SIZE, 0, 0); //flash_api_lock_all_regions(false);
	}
	isp_page_number_split = 128;
	while (isp_page_number && isp_page_number_split) {
		flash_erase_page(--isp_page_number, false);
		isp_page_number_split--;
	}
	return (isp_page_number==0);*/
}


void isp_start_appli(void)
{
	for ( int pos = 0; pos <= 8; pos++ )
		GPBR->SYS_GPBR[ pos ] = 0x00000000;
	SOFTWARE_RESET();
}

/**
 * \brief Calculates the CRC-8-CCITT
 *
 * CRC-8-CCITT is defined to be x^8 + x^2 + x + 1
 * To use this function use the following template:
 * crc = isp_crc8( crc, data );
 */
/*static uint8_t isp_crc8(uint8_t inCrc, uint8_t inData)
{
	uint8_t i;
	uint16_t data;

	data = inCrc ^ inData;
	data <<= 8;

	for (i = 0; i < 8; i++) {
		if ((data & 0x8000) != 0) {
			data = data ^ (BOOT_CFG1_CRC8_POLYNOMIAL << 7);
		}
		data = data << 1;
	}
	return (uint8_t)(data >> 8);
}*/


void isp_force_boot_isp(bool force)
{
	for ( int pos = 0; pos <= sizeof(sys_reset_to_loader_magic)/4; pos++ )
                GPBR->SYS_GPBR[ pos ] = ((uint32_t*)sys_reset_to_loader_magic)[ pos ];
}

//@}


//@}

