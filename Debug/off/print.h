/* Copyright (C) 2011 by Jacob Alexander
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

// ----- Disabler Defines -----

#define dPrint(c)
#define dPrintStr(c)
#define dPrintStrs(...)
#define dPrintStrNL(c)
#define dPrintStrsNL(...)

// Special Msg Constructs (Uses VT100 tags)
#define dPrintMsg(colour_code_str,msg,...)
#define printMsg(colour_code_str,msg,str)

// Info Messages
#define info_dPrint(...)
#define info_print(str)

// Warning Messages
#define warn_dPrint(...)
#define warn_print(str)

// Error Messages
#define erro_dPrint(...)
#define erro_print(str)

// Debug Messages
#define dbug_dPrint(...)
#define dbug_print(str)

// Static String Printing
#define print(s) _print(PSTR(s))

// Output Functions
#define _print(s)
#define usb_debug_putstr(s)
#define usb_debug_putstrs(s, ...)

// String Functions
#define hexToStr(hex, out)
#define int8ToStr(in, out)
#define int16ToStr(in, out)
#define hexToStr_op(in, out, op)
#define revsStr(in)
#define lenStr(in)

#endif

