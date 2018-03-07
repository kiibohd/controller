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

// ----- Compiler Includes -----

#include <sys/types.h>
#include <sys/cdefs.h>
#include <stdarg.h>



// ----- Project Includes -----

#include <mcu_compat.h>

#if defined(_kinetis_)
#include <kinetis.h>

#elif defined(_sam_)
#include <sam.h>

#endif



// ----- Local Includes -----

#include "mchck-cdefs.h"

extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss, _app_rom, _app_rom_end, _bootloader;

#include "ftfl.h"
#include "usbotg.h"
#include "sim.h"
#include "flash.h"
#include "usb.h"

