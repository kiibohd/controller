/* Copyright (C) 2015-2017 by Jacob Alexander
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

// Project Includes
#include <Lib/mcu_compat.h>



// ----- Defines -----

#define NL "\r\n"
//#define FLASH_DEBUG // XXX: Use when there are USB flashing issues



// ----- Macros -----

#define print(str)   Output_putstr(str)
#define printNL(str) Output_putstr(str NL)



// ----- Functions -----

#if defined(_kii_v1_) || defined(_kii_v2_)
int Output_putstr( char* str );

int uart_serial_write( const void *buffer, uint32_t size );

void uart_serial_setup();

// Convenience
#define printHex(hex) printHex_op(hex, 1)
#define printHex32(hex) printHex_op(hex, 1)
void printHex_op( uint32_t in, uint8_t op );

#else
#define Output_putstr(str)
#define uart_serial_write(buf,size)
#define uart_serial_setup()
#define printHex(hex)
#define printHex32(hex)
#define printHex_op(in,op)
#endif

