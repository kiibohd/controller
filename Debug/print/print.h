/* Copyright (C) 2011-2013 by Jacob Alexander
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

#ifndef print_h__
#define print_h__

// ----- Includes -----

// Compiler Includes
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_)

#include <avr/pgmspace.h>
#include "avr/usb_keyboard_debug.h"

#elif defined(_mk20dx128_)

#include "arm/usb_serial.h"

#endif



// ----- Defines -----
#define NL "\r\n"



// ----- Functions and Corresponding Function Aliases -----

/* XXX
 * Note that all the variadic functions below, take comma separated string lists, they are purposely not printf style (simplicity)
 */

// Function Aliases
#define dPrint(c)         usb_debug_putstr(c)
#define dPrintStr(c)      usb_debug_putstr(c)
#define dPrintStrs(...)   usb_debug_putstrs(__VA_ARGS__, "\0\0\0")      // Convenience Variadic Macro
#define dPrintStrNL(c)    dPrintStrs       (c, NL)                      // Appends New Line Macro
#define dPrintStrsNL(...) usb_debug_putstrs(__VA_ARGS__, NL, "\0\0\0")  // Appends New Line Macro

// Special Msg Constructs (Uses VT100 tags)
#define dPrintMsg(colour_code_str,msg,...) \
                          usb_debug_putstrs("\033[", colour_code_str, "m", msg, "\033[0m - ", __VA_ARGS__, NL, "\0\0\0")
#define printMsg(colour_code_str,msg,str) \
                          print("\033[" colour_code_str "m" msg "\033[0m - " str NL)

// Info Messages
#define info_dPrint(...)  dPrintMsg        ("1;32",   "INFO",    __VA_ARGS__) // Info Msg
#define info_print(str)   printMsg         ("1;32",   "INFO",    str)         // Info Msg

// Warning Messages
#define warn_dPrint(...)  dPrintMsg        ("1;33",   "WARNING", __VA_ARGS__) // Warning Msg
#define warn_print(str)   printMsg         ("1;33",   "WARNING", str)         // Warning Msg

// Error Messages
#define erro_dPrint(...)  dPrintMsg        ("1;5;31", "ERROR",   __VA_ARGS__) // Error Msg
#define erro_print(str)   printMsg         ("1;5;31", "ERROR",   str)         // Error Msg

// Debug Messages
#define dbug_dPrint(...)  dPrintMsg        ("1;35",   "DEBUG",   __VA_ARGS__) // Debug Msg
#define dbug_print(str)   printMsg         ("1;35",   "DEBUG",   str)         // Debug Msg


// Static String Printing
#define print(s) _print(PSTR(s))

void _print(const char *s);
void usb_debug_putstr( char* s );
void usb_debug_putstrs( char* first, ... );


// Printing numbers
#define printHex(hex) printHex_op(hex, 1)

void printInt8  ( uint8_t  in );
void printInt16 ( uint16_t in );
void printHex_op( uint16_t in, uint8_t op );


// String Functions
#define hexToStr(hex, out) hexToStr_op(hex, out, 1)

void int8ToStr  ( uint8_t  in, char*  out );
void int16ToStr ( uint16_t in, char*  out );
void hexToStr_op( uint16_t in, char*  out, uint8_t op );
void revsStr    ( char*  in );
uint16_t lenStr ( char*  in );

#endif

