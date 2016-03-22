/* Copyright (C) 2011-2014 by Jacob Alexander
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

// ----- Includes -----

// Compiler Includes
#include <stdarg.h>

// Project Includes
#include "print.h"



// ----- Functions -----

// Multiple string Output
void printstrs( char* first, ... )
{
}

// Print a constant string
void _print( const char* s )
{
}



// Number Printing Functions
void printInt8( uint8_t in )
{
}

void printInt16( uint16_t in )
{
}

void printInt32( uint32_t in )
{
}

void printHex_op( uint16_t in, uint8_t op )
{
}

void printHex32_op( uint32_t in, uint8_t op )
{
}



// String Functions
void int8ToStr( uint8_t in, char* out )
{
}


void int16ToStr( uint16_t in, char* out )
{
}


void int32ToStr( uint32_t in, char* out )
{
}


void hexToStr_op( uint16_t in, char* out, uint8_t op )
{
}


void hex32ToStr_op( uint32_t in, char* out, uint8_t op )
{
}


void revsStr( char* in )
{
}


uint16_t lenStr( char* in )
{
	// Iterator
	char *pos;

	// Loop until null is found
	for ( pos = in; *pos; pos++ );

	// Return the difference between the pointers of in and pos (which is the string length)
	return (pos - in);
}


int16_t eqStr( char* str1, char* str2 )
{
	// Scan each string for NULLs and whether they are the same
	while( *str1 != '\0' && *str1++ == *str2++ );

	// If the strings are still identical (i.e. both NULL), then return -1, otherwise current *str1
	// If *str1 is 0, then str1 ended (and str1 is "like" str2), otherwise strings are different
	return *--str1 == *--str2 ? -1 : *++str1;
}

int numToInt( char* in )
{
	return 0;
}

