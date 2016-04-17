/* Copyright (C) 2014-2016 by Jacob Alexander
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

#pragma once

// ----- Includes -----

// Project Includes
#include <matrix_setup.h>



// ----- Matrix Definition -----

// CapTest
// (TODO)
//
// Strobe
//  PTB0..3,16,17
//  PTC4,5
//  PTD0
//
// Sense
//  PTD1..7


// -- Strobes --
// Format
//  gpio( <port letter>, <port #> )
// Freescale ARM MK20's support GPIO PTA, PTB, PTC, PTD and PTE 0..31
// Not all chips have access to all of these pins (most don't have 160 pins :P)
//

GPIO_Pin Matrix_strobe[] = { gpio(B,0), gpio(B,1), gpio(B,2), gpio(B,3), gpio(B,16), gpio(B,17), gpio(C,4), gpio(C,5), gpio(D,0) };


// -- Sense --
// Format
//  sense( <port letter>, <port #>, <adc #>, <adc channel #> )
// Freescale ARM MK20's support 32 ADC channels
// However, not every channel is useful for reading from an input pin.
//
// NOTE: Be careful that you are not using a strobe and a sense at the same time!
//

ADC_Pin Matrix_sense[] = { sense(B,4,0,5) };

// TODO
// Misc pins required for control

