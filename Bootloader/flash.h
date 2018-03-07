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

#pragma once

// ----- Defines -----

#if defined(_mk20dx128vlf5_) || defined(_mk20dx128vlh7_)
#define FLASH_SECTOR_SIZE 1024
#elif defined(_mk20dx256vlh7_)
#define FLASH_SECTOR_SIZE 2048
#elif defined(_mk22fx512avlh12_)
#define FLASH_SECTOR_SIZE 4096
#elif defined(_sam4sd32_)
#define FLASH_SECTOR_SIZE 65536ul
#define FLASH_PAGE_SIZE 512
#endif



// ----- Functions -----

#if defined(_kinetis_)
__attribute__((section(".ramtext.ftfl_submit_cmd"), long_call))
int ftfl_submit_cmd(void);
#elif defined(_sam_)
int ftfl_submit_cmd(uint8_t cmd, uint16_t args);
#endif

int flash_prepare_flashing(void);
int flash_erase_sector(uintptr_t);
int flash_program_sector(uintptr_t, size_t);
int flash_program_longword(uintptr_t, uint8_t*);
void *flash_get_staging_area(uintptr_t, size_t);

