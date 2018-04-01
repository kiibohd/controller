/* Copyright (C) 2018 by Rowan Decker
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

// RTT Includes
#include "SEGGER_RTT.h"

// KLL Include
#include <kll.h>

// Local Includes
#include <output_com.h>



// ----- Function Declarations -----

// ----- Defines -----
#define rtt_buffer_size 64
#define rtt_channel 0

// ----- Variables -----

volatile uint8_t rtt_buffer_head = 0;
volatile uint8_t rtt_buffer_tail = 0;
volatile uint8_t rtt_buffer_items = 0;
volatile uint8_t rtt_buffer[rtt_buffer_size];

// ----- Capabilities -----

// ----- Functions -----

// RTT Module Setup
inline void RTT_setup()
{
	// No setup needed
}


// RTT Data Poll
inline void RTT_poll()
{
	int c;
	while ( (c = SEGGER_RTT_GetKey()) > 0 )
	{
		rtt_buffer[rtt_buffer_tail++] = c;
		rtt_buffer_items++;

		// Wrap-around of tail pointer
		if ( rtt_buffer_tail >= rtt_buffer_size )
		{
			rtt_buffer_tail = 0;
		}

		// Make sure the head pointer also moves if circular buffer is overwritten
		if ( rtt_buffer_head == rtt_buffer_tail )
		{
			rtt_buffer_head++;
		}

		// Wrap-around of head pointer
		if ( rtt_buffer_head >= rtt_buffer_size )
		{
			rtt_buffer_head = 0;
		}
	}
}


// RTT Data Periodic
inline void RTT_periodic()
{
}


// RTT Data Ready
uint8_t RTT_ready()
{
	return 1;
}


// Sets the device into firmware reload mode
inline void RTT_firmwareReload()
{
	// TODO
}


// RTT Input buffer available
inline unsigned int RTT_availablechar()
{
	return rtt_buffer_items;
}


// Get the next character, or -1 if nothing received
inline int RTT_getchar()
{
	// XXX Make sure to check output_availablechar() first!

	unsigned int value = -1;

	// Check to see if the FIFO has characters
	if ( rtt_buffer_items > 0 )
	{
		value = rtt_buffer[rtt_buffer_head++];
		rtt_buffer_items--;

		// Wrap-around of head pointer
		if ( rtt_buffer_head >= rtt_buffer_size )
		{
			rtt_buffer_head = 0;
		}
	}

	return value;
}


// RTT Send Character to output buffer
inline int RTT_putchar( char c )
{
	return SEGGER_RTT_PutChar(rtt_channel, c);
}


// RTT Send String to output buffer, null terminated
inline int RTT_putstr( char* str )
{
	return SEGGER_RTT_WriteString(rtt_channel, str);
}


// Soft Chip Reset
inline void RTT_softReset()
{
#if defined(_kinetis_) || defined(_sam_)
	SOFTWARE_RESET();
#endif
}
