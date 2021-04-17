/* Copyright (C) 2011-2020 by Jacob Alexander
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



// ----- Macros -----

// ----- Function Declarations -----

// ----- Variables -----

// ----- Capabilities -----

// ----- Functions -----

// Output Module Setup
void Output_setup()
{
	// Initialize Interface module
	OutputGen_setup();
}


// Output Module Data Poll
void Output_poll()
{
}


// Output Module Data Periodic
void Output_periodic()
{
}


// Sets the device into firmware reload mode
void Output_firmwareReload()
{
}


// Soft Chip Reset
void Output_softReset()
{
}


// UART Input buffer available
unsigned int Output_availablechar()
{
	return 0;
}


// UART Get Character from input buffer
int Output_getchar()
{
	return 0;
}


// UART Send Character to output buffer
int Output_putchar( char c )
{
	erro_printNL("Output_putchar not implemented!");
	return 0;
}

void output_putstr(const char *buf);

// UART Send String to output buffer, null terminated
int Output_putstr(char* str)
{
	output_putstr((const char*)str);
	return 0;
}


// UART RawIO buffer available
unsigned int Output_rawio_availablechar()
{
	return 0;
}


// UART RawIO get buffer
int Output_rawio_getbuffer( char* buffer )
{
	return 0;
}


// UART RawIO send buffer
int Output_rawio_sendbuffer( char* buffer )
{
	return 0;
}

