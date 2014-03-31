/* Copyright (C) 2011-2012,2014 by Jacob Alexander
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

#ifndef __SCAN_LOOP_H
#define __SCAN_LOOP_H

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

void scan_setup( void );
uint8_t scan_loop( void );


// Functions available to macro.c
uint8_t scan_sendData( uint8_t dataPayload );

void scan_finishedWithBuffer( uint8_t sentKeys );
void scan_finishedWithUSBBuffer( uint8_t sentKeys );
void scan_lockKeyboard( void );
void scan_unlockKeyboard( void );
void scan_resetKeyboard( void );


#endif // __SCAN_LOOP_H

