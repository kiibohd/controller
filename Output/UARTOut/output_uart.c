/* Copyright (C) 2014-2018 by Jacob Alexander
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

// UART Includes
#if defined(_avr_at_)
#elif defined(_kinetis_) || defined(_sam_)
#include "arm/uart_serial.h"
#endif

// KLL Include
#include <kll.h>

// Local Includes
#include <output_com.h>
#include "output_uart.h"



// ----- Function Declarations -----

// ----- Variables -----

// ----- Capabilities -----

// ----- Functions -----

// UART Module Setup
inline void UART_setup()
{
	// Setup UART
	uart_serial_setup();
}


// UART Data Poll
inline void UART_poll()
{
}


// UART Data Periodic
inline void UART_periodic()
{
}


// UART Data Ready
uint8_t UART_ready()
{
	return 1;
}


// Sets the device into firmware reload mode
inline void UART_firmwareReload()
{
	uart_device_reload();
}


// UART Input buffer available
inline unsigned int UART_availablechar()
{
	return uart_serial_available();
}


// UART Get Character from input buffer
inline int UART_getchar()
{
	// XXX Make sure to check output_availablechar() first! Information is lost with the cast (error codes) (AVR)
	return (int)uart_serial_getchar();
}


// UART Send Character to output buffer
inline int UART_putchar( char c )
{
	return uart_serial_putchar( c );
}


// UART Send String to output buffer, null terminated
inline int UART_putstr( char* str )
{
#if defined(_avr_at_) // AVR
	uint16_t count = 0;
#elif defined(_kinetis_) || defined(_sam_) // ARM
	uint32_t count = 0;
#endif
	// Count characters until NULL character, then send the amount counted
	while ( str[count] != '\0' )
		count++;

	return uart_serial_write( str, count );
}


// Soft Chip Reset
inline void UART_softReset()
{
#if defined(_avr_at_) // AVR
#elif defined(_kinetis_) || defined(_sam_) // ARM
	SOFTWARE_RESET();
#endif
}


// ----- CLI Command Functions -----

