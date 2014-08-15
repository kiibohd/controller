/* Copyright (c) 2011,2012 Simon Schubert <2@0x2c.org>.
 * Modifications by Jacob Alexander 2014 <haata@kiibohd.com>
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

#ifndef __FLASH_H
#define __FLASH_H

// ----- Defines -----

#define FLASH_SECTOR_SIZE 1024



// ----- Functions -----

__attribute__((section(".ramtext.ftfl_submit_cmd"), long_call))
int ftfl_submit_cmd(void);
int flash_prepare_flashing(void);
int flash_erase_sector(uintptr_t);
int flash_program_section(uintptr_t, size_t);
int flash_program_sector(uintptr_t, size_t);
void *flash_get_staging_area(uintptr_t, size_t);

#endif

