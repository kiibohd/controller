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

// USB HID String Output
void usb_debug_putstr( char* s )
{
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
	while ( *s != '\0' )
		usb_debug_putchar( *s++ );
#elif defined(_mk20dx128_) || defined(_mk20dx256_) // ARM
	// Count characters until NULL character, then send the amount counted
	uint32_t count = 0;
	while ( s[count] != '\0' )
		count++;

	usb_serial_write( s, count );
#endif
}

// Multiple string Output
void usb_debug_putstrs( char* first, ... )
{
	// Initialize the variadic function parameter list
	va_list ap;

	// Get the first parameter
	va_start( ap, first );
	char *cur = first;

	// Loop through the variadic list until "\0\0\0" is found
	while ( !( cur[0] == '\0' && cur[1] == '\0' && cur[2] == '\0' ) )
	{
		// Print out the given string
		usb_debug_putstr( cur );

		// Get the next argument ready
		cur = va_arg( ap, char* );
	}

	va_end( ap ); // Not required, but good practice
}

// Print a constant string
void _print(const char *s)
{
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
	char c;

	// Acquire the character from flash, and print it, as long as it's not NULL
	// Also, if a newline is found, print a carrige return as well
	while ( ( c = pgm_read_byte(s++) ) != '\0' )
	{
		if ( c == '\n' )
			usb_debug_putchar('\r');
		usb_debug_putchar(c);
	}
#elif defined(_mk20dx128_) || defined(_mk20dx256_) // ARM
	usb_debug_putstr( (char*)s );
#endif
}



// Number Printing Functions
void printInt8( uint8_t in )
{
	// Max number of characters is 3 + 1 for null
	char tmpStr[4];

	// Convert number
	int8ToStr( in, tmpStr );

	// Print number
	dPrintStr( tmpStr );
}

void printInt16( uint16_t in )
{
	// Max number of characters is 5 + 1 for null
	char tmpStr[6];

	// Convert number
	int16ToStr( in, tmpStr );

	// Print number
	dPrintStr( tmpStr );
}

void printInt32( uint32_t in )
{
	// Max number of characters is 10 + 1 for null
	char tmpStr[11];

	// Convert number
	int32ToStr( in, tmpStr );

	// Print number
	dPrintStr( tmpStr );
}

void printHex_op( uint16_t in, uint8_t op )
{
	// With an op of 1, the max number of characters is 6 + 1 for null
	// e.g. "0xFFFF\0"
	// op 2 and 4 require fewer characters (2+1 and 4+1 respectively)
	char tmpStr[7];

	// Convert number
	hexToStr_op( in, tmpStr, op );

	// Print number
	dPrintStr( tmpStr );
}



// String Functions
void int8ToStr( uint8_t in, char* out )
{
	// Position and sign containers
	uint8_t pos;
	pos = 0;

	// Evaluate through digits as decimal
	do
	{
		out[pos++] = in % 10 + '0';
	}
	while ( (in /= 10) > 0 );

	// Append null
	out[pos] = '\0';

	// Reverse the string to the correct order
	revsStr(out);
}


void int16ToStr( uint16_t in, char* out )
{
	// Position and sign containers
	uint16_t pos;
	pos = 0;

	// Evaluate through digits as decimal
	do
	{
		out[pos++] = in % 10 + '0';
	}
	while ( (in /= 10) > 0 );

	// Append null
	out[pos] = '\0';

	// Reverse the string to the correct order
	revsStr(out);
}


void int32ToStr( uint32_t in, char* out )
{
	// Position and sign containers
	uint32_t pos;
	pos = 0;

	// Evaluate through digits as decimal
	do
	{
		out[pos++] = in % 10 + '0';
	}
	while ( (in /= 10) > 0 );

	// Append null
	out[pos] = '\0';

	// Reverse the string to the correct order
	revsStr(out);
}


void hexToStr_op( uint16_t in, char* out, uint8_t op )
{
	// Position container
	uint16_t pos = 0;

	// Evaluate through digits as hex
	do
	{
		uint16_t cur = in % 16;
		out[pos++] = cur + (( cur < 10 ) ? '0' : 'A' - 10);
	}
	while ( (in /= 16) > 0 );

	// Output formatting options
	switch ( op )
	{
	case 1: // Add 0x
		out[pos++] = 'x';
		out[pos++] = '0';
		break;
	case 2: //  8-bit padding
	case 4: // 16-bit padding
		while ( pos < op )
			out[pos++] = '0';
		break;
	}

	// Append null
	out[pos] = '\0';

	// Reverse the string to the correct order
	revsStr(out);
}


void revsStr( char* in )
{
	// Iterators
	int i, j;

	// Temp storage
	char c;

	// Loop through the string, and reverse the order of the characters
	for ( i = 0, j = lenStr( in ) - 1; i < j; i++, j-- )
	{
		c = in[i];
		in[i] = in[j];
		in[j] = c;
	}
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

