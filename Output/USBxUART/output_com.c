/* Copyright (C) 2014-2017 by Jacob Alexander
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

// USB Module Setup
inline void Output_setup()
{
	// Initialize Interface module
	OutputGen_setup();

	// Setup UART
	UART_setup();

	// Setup USB
	USB_setup();
}


// USB Data Poll
inline void Output_poll()
{
	// UART Poll Routine
	UART_poll();

	// USB Poll Routine
	USB_poll();
}


// USB Data Periodic
inline void Output_periodic()
{
	// UART Periodic Routine
	UART_periodic();

	// USB Periodic Routine
	USB_periodic();
}


// Sets the device into firmware reload mode
void Output_firmwareReload()
{
	USB_firmwareReload();
}


// USB Input buffer available
inline unsigned int Output_availablechar()
{
#if enableVirtualSerialPort_define == 1
	return USB_availablechar() + UART_availablechar();
#else
	return UART_availablechar();
#endif
}


// USB Get Character from input buffer
inline int Output_getchar()
{
#if enableVirtualSerialPort_define == 1
	// XXX Make sure to check output_availablechar() first! Information is lost with the cast (error codes) (AVR)
	if ( USB_availablechar() > 0 )
	{
		return USB_getchar();
	}
#endif

	if ( UART_availablechar() > 0 )
	{
		return UART_getchar();
	}

	return -1;
}


// USB Send Character to output buffer
inline int Output_putchar( char c )
{
#if enableVirtualSerialPort_define == 1
	// First send to UART
	UART_putchar( c );

	// Then send to USB
	return USB_putchar( c );
#else
	return UART_putchar( c );
#endif
}


// USB Send String to output buffer, null terminated
inline int Output_putstr( char* str )
{
#if enableVirtualSerialPort_define == 1
	// First send to UART
	UART_putstr( str );

	// Then send to USB
	return USB_putstr( str );
#else
	return UART_putstr( str );
#endif
}


// Soft Chip Reset
inline void Output_softReset()
{
	USB_softReset();
}


// USB RawIO buffer available
unsigned int Output_rawio_availablechar()
{
#if enableRawIO_define == 1
	return USB_rawio_availablechar();
#else
	return 0;
#endif
}


// USB RawIO get buffer
int Output_rawio_getbuffer( char* buffer )
{
#if enableRawIO_define == 1
	return USB_rawio_getbuffer( buffer );
#else
	return 0;
#endif
}


// USB RawIO send buffer
int Output_rawio_sendbuffer( char* buffer )
{
#if enableRawIO_define == 1
	return USB_rawio_sendbuffer( buffer );
#else
	return 0;
#endif
}

