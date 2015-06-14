/* Copyright (C) 2011-2012,2014-2015 by Jacob Alexander
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

// ----- Includes -----

// Compiler Includes
#include <stdint.h>

// Local Includes
#include "matrix_scan.h"

// Matrix Configuration
#include <matrix.h>



// ----- Defines -----

#define KEYBOARD_BUFFER 24 // Max number of key signals to buffer



// ----- Variables -----

// NOTE: Highest Bit: Valid keypress (0x80 is valid keypress)
//        Other Bits: Pressed state sample counter
extern                       uint8_t KeyIndex_Array [KEYBOARD_KEYS + 1];
		static const uint8_t KeyIndex_Size = KEYBOARD_KEYS;

extern volatile              uint8_t KeyIndex_Buffer[KEYBOARD_BUFFER];
extern volatile              uint8_t KeyIndex_BufferUsed;
extern volatile     uint8_t KeyIndex_Add_InputSignal;



// ----- Functions -----

void    Scan_setup( void );
uint8_t Scan_loop( void );

// Callbacks from the Macro and Output modules (useful with difficult protocols)
void Scan_finishedWithBuffer( uint8_t sentKeys );
void Scan_finishedWithUSBBuffer( uint8_t sentKeys );

