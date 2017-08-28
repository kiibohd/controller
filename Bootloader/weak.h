/* Copyright (C) 2017 by Jacob Alexander
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file.  If not, see <http://www.gnu.org/licenses/>.
 */

// ----- Includes -----

#include <inttypes.h>



// ----- Weak Functions -----

int8_t Chip_validation( uint8_t* key ) __attribute__ ((weak));

void   Chip_process()   __attribute__ ((weak));
void   Chip_reset()     __attribute__ ((weak));
void   Chip_setup()     __attribute__ ((weak));
void   Device_process() __attribute__ ((weak));
void   Device_reset()   __attribute__ ((weak));
void   Device_setup()   __attribute__ ((weak));

