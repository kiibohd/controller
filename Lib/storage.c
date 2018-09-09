/**
 * Copyright (c) 2012 Atmel Corporation. All rights reserved.
 * Modified by Jacob Alexander 2018
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
 */

#include "mcu_compat.h"



#if defined(_sam_)

// ----- Includes -----

#include "storage.h"

// ASF Includes
#include <flash_efc.h>

// Local Includes
#include "sam.h"



// ----- Defines -----

#if (STORAGE_SIZE == 511)
#elif (STORAGE_SIZE == 255)
#elif (STORAGE_SIZE == 127)
#elif (STORAGE_SIZE == 63)
#else
#error Invalid STORAGE_SIZE
#endif

#if (STORAGE_FLASH_PAGE_SIZE != 512)
#error Page sizes other than 512 bytes are untested and will likely not work!
#endif

#if (STORAGE_PAGES != 16)
#error Page counts other than 16 are untested and will likely not work!
#endif



// ----- Variables -----

static int8_t current_storage_index = 0;
static uint8_t storage_buffer[STORAGE_SIZE];
static uint8_t current_page = 0;
static uint8_t erase_flag = 0;

/* status byte scheme
| X | X | X | X | X | X | X | V |
if V = 1, page/sub_page is next in line to be written, otherwise 0
*/



// ----- Functions -----

// Determine which block in page we are at
static int8_t get_storage_block( uint8_t * data )
{
#if (STORAGE_SIZE == 511)
	if ( data[0] & 0x01 )
	{
		return 0;
	}
#elif (STORAGE_SIZE == 255)
	if ( data[0] & 0x01 )
	{
		return 0;
	}
	else if ( data[256] & 0x01 )
	{
		return 1;
	}
#elif (STORAGE_SIZE == 127)
	if ( data[0] & 0x01 )
	{
		return 0;
	}
	else if ( data[128] & 0x01 )
	{
		return 1;
	}
	else if ( data[256] & 0x01 )
	{
		return 2;
	}
	else if ( data[384] & 0x01 )
	{
		return 3;
	}
#elif (STORAGE_SIZE == 63)
	if ( data[0] & 0x01 )
	{
		return 0;
	}
	else if ( data[64] & 0x01 )
	{
		return 1;
	}
	else if ( data[128] & 0x01 )
	{
		return 2;
	}
	else if ( data[192] & 0x01 )
	{
		return 3;
	}
	else if ( data[256] & 0x01 )
	{
		return 4;
	}
	else if ( data[320] & 0x01 )
	{
		return 5;
	}
	else if ( data[384] & 0x01 )
	{
		return 6;
	}
	else if ( data[448] & 0x01 )
	{
		return 7;
	}
#endif
	return -1;
}

uint8_t storage_init()
{
	uint8_t page_walker;

	// Flash incoming read buffer
	uint8_t page_buffer[STORAGE_FLASH_PAGE_SIZE];

	// Initialize in-memory buffer
	for ( int i = 0; i < STORAGE_SIZE; i++ )
	{
		storage_buffer[i] = 0xff;
	}

	// Find the page and block index for the valid writable page.
	for ( page_walker = 0; page_walker < STORAGE_PAGES; page_walker++ )
	{
		// Read flash into page_buffer
		memcpy(
			page_buffer,
			(const void *)(STORAGE_FLASH_START + page_walker * STORAGE_FLASH_PAGE_SIZE),
			sizeof(page_buffer)
		);

		// Check to see if block is available
		current_storage_index = get_storage_block(page_buffer);
		current_page = page_walker;

		// If available, stop walking
		if ( current_storage_index >= 0 )
		{
			break;
		}
	}

	// If no page found, then the last page was the valid one. select the last block and put the data into buffer
	if ( page_walker == STORAGE_PAGES )
	{
		for ( int i = 0; i < STORAGE_SIZE; i++ )
		{
#if (STORAGE_SIZE == 511)
			storage_buffer[i] = page_buffer[i + 1];
#elif (STORAGE_SIZE == 255)
			storage_buffer[i] = page_buffer[i + 257];
#elif (STORAGE_SIZE == 127)
			storage_buffer[i] = page_buffer[i + 385];
#elif (STORAGE_SIZE == 63)
			storage_buffer[i] = page_buffer[i + 449];
#endif
		}
		current_storage_index = 0;
		current_page = 0;

		// Since all pages are dirty, erase before the next write to page 0
		erase_flag = 1;
	}
	// If a page was found, move one block back and copy the data
	else
	{
		// First block of a page, this means the data to be read is in the last block of the previous page
		if ( current_storage_index == 0 )
		{
			memcpy(
				page_buffer,
				(const void *)(STORAGE_FLASH_START + ((current_page - 1) * STORAGE_FLASH_PAGE_SIZE)),
				sizeof(page_buffer)
			);

			for ( int i  = 0; i < STORAGE_SIZE; i++ )
			{
#if(STORAGE_SIZE == 511)
				storage_buffer[i] = page_buffer[i + 1];
#elif(STORAGE_SIZE == 255)
				storage_buffer[i] = page_buffer[i + 257];
#elif(STORAGE_SIZE == 127)
				storage_buffer[i] = page_buffer[i + 385];
#elif (STORAGE_SIZE == 63)
				storage_buffer[i] = page_buffer[i + 449];
#endif
			}
		}
		// If the block is within a page, go to previous block and fetch the data
		else
		{
			for ( int i = 0; i < STORAGE_SIZE; i++ )
			{
				storage_buffer[i] = page_buffer[((current_storage_index - 1) * (STORAGE_SIZE + 1)) + 1 + i];
			}
		}

	}

	return 1;
}

// Write storage block
// data    - Buffer to write from
// address - Starting address to write to
// size    - Number of bytes to write into storage
uint8_t storage_write(uint8_t* data, uint16_t address, uint16_t size )
{
	// Flashing buffer
	uint8_t page_buffer[STORAGE_FLASH_PAGE_SIZE];

	// Make sure there is enough address space in the storage
	if ( size + address > STORAGE_SIZE )
	{
		return 0;
	}

	// Set internal buffer to all 0xffs
	// This way it's possible to do partial writes to the page
	for ( int i = 0; i < STORAGE_FLASH_PAGE_SIZE; i++ )
	{
		page_buffer[i] = 0xff;
	}

	// If we have wrapped around the reserved 16 pages, delete everything
	if ( current_page == 0 && erase_flag && current_storage_index == 0 )
	{
		// Erase the first 8 pages
		flash_erase_page( STORAGE_FLASH_START, IFLASH_ERASE_PAGES_8 );
		// Erase the last 8 pages
		flash_erase_page( STORAGE_FLASH_START + (STORAGE_FLASH_PAGE_SIZE * 8), IFLASH_ERASE_PAGES_8 );
	}

	// Clear empty page flag
	page_buffer[(current_storage_index * (STORAGE_SIZE + 1))] = 0x00;

	// Prepare flashing and in-memory buffers so we can write to flash
	for ( int i = 0; i < size; i++ )
	{
		// Write to flashing buffer
		page_buffer[i + address + (current_storage_index * (STORAGE_SIZE + 1)) + 1] = data[i];

		// Write to in-memory storage
		storage_buffer[i + address] = data[i];
	}

	// Write flashing buffer to flash
	flash_write(
		(STORAGE_FLASH_START + current_page * STORAGE_FLASH_PAGE_SIZE),
		(const void *)page_buffer,
		sizeof(page_buffer), 0
	);

	// Handle page increment based on storage size
#if (STORAGE_SIZE == 511)
	current_page++;
#elif (STORAGE_SIZE == 255)
	current_storage_index++;
	if ( current_storage_index > 1 )
	{
		current_page++;
		current_storage_index = 0;
	}
#elif (STORAGE_SIZE == 127)
	current_storage_index++;
	if ( current_storage_index > 3 )
	{
		current_page++;
		current_storage_index = 0;
	}
#elif (STORAGE_SIZE == 63)
	current_storage_index++;
	if ( current_storage_index > 7 )
	{
		current_page++;
		current_storage_index = 0;
	}
#endif

	// Handle page rotation
	if ( current_page > 15 )
	{
		current_page = 0;
		erase_flag = 1;
		current_storage_index = 0;
	}
	return 1;
}

// Read storage block
// data    - Buffer to read into
// address - Starting address from storage
// size    - Number of bytes to read starting from address
uint8_t storage_read( uint8_t* data, uint16_t address, uint16_t size )
{
	// Make sure the data section fully exists
	if ( size + address > STORAGE_SIZE )
	{
		return 0;
	}

	// Read out from buffer
	for ( int i = 0; i < size; i++ )
	{
		data[i] = storage_buffer[address + i];
	}
	return 1;
}

#endif

