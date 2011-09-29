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

// Compiler Includes
#include <stdarg.h>

// AVR Includes
#include <avr/io.h>
#include <avr/pgmspace.h>

// Project Includes
#include "print.h"

// Defines


// USB HID String Output
void usb_debug_putstr( char* s )
{
	while ( *s != '\0' )
		usb_debug_putchar( *s++ );
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
	char c;

	// Acquire the character from flash, and print it, as long as it's not NULL
	// Also, if a newline is found, print a carrige return as well
	while ( ( c = pgm_read_byte(s++) ) != '\0' )
	{
		if ( c == '\n' )
			usb_debug_putchar('\r');
		usb_debug_putchar(c);
	}
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

