/* Copyright (C) 2011-2017 by Jacob Alexander
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
#include <Lib/OutputLib.h>

// Project Includes
#include <print.h>

// KLL
#include <kll.h>

// Interface Includes
#include <output_com.h>



// ----- Function Declarations -----

// ----- Variables -----

// ----- Capabilities -----

// ----- Functions -----

// Output Module Setup
inline void Output_setup()
{
	// Initialize Interface module
	OutputGen_setup();

	// Initialize the USB module
	USB_setup();
}


// Output Module Data Poll
inline void Output_poll()
{
	USB_poll();
}


// Output Module Data Periodic
inline void Output_periodic()
{
	USB_periodic();
}


// Sets the device into firmware reload mode
inline void Output_firmwareReload()
{
	USB_firmwareReload();
}


// Soft Chip Reset
inline void Output_softReset()
{
	USB_softReset();
}


// USB Input buffer available
inline unsigned int Output_availablechar()
{
	return USB_availablechar();
}


// USB Get Character from input buffer
inline int Output_getchar()
{
	return USB_getchar();
}


// USB Send Character to output buffer
inline int Output_putchar( char c )
{
	return USB_putchar( c );
}


// USB Send String to output buffer, null terminated
inline int Output_putstr( char* str )
{
	return USB_putstr( str );
}


// USB RawIO buffer available
unsigned int Output_rawio_availablechar()
{
	return USB_rawio_availablechar();
}


// USB RawIO get buffer
int Output_rawio_getbuffer( char* buffer )
{
	return USB_rawio_getbuffer( buffer );
}


// USB RawIO send buffer
int Output_rawio_sendbuffer( char* buffer )
{
	return USB_rawio_sendbuffer( buffer );
}

