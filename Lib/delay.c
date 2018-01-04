/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2013 PJRC.COM, LLC.
 * Modifications by Jacob Alexander 2013-2017
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * 2. If the Software is incorporated into a build system that allows
 * selection among a list of target devices, then similar target
 * devices manufactured by PJRC.COM must be included in the list of
 * target devices and selectable in the same manner.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// ----- Local Includes -----

#include "delay.h"
#include "mcu_compat.h"

#if defined(_kinetis_)
#include "kinetis.h"
#elif defined(_sam_)
#include "sam.h"
#endif



// ----- Variables -----

// The systick interrupt is supposed to increment this at 1 kHz rate
volatile uint32_t systick_millis_count = 0;



// ----- Functions -----

// - Delay Functions -

// Delay by specified cycles
void delay_cycles( uint32_t cycles )
{
	uint32_t start = cycle_now();

	// Loop and yield until the cycles count has expired
	while ( cycle_now() - start <= cycles )
	{
		yield();
	}
}

// Delay by specified usecs
// XXX (HaaTa) Possibly not accurate for low us values (for low F_CPU values)
void delay_us( uint32_t us )
{
	uint32_t start = us_now();

	// Loop and yield until the us count has expired
	while ( us_now() - start <= us )
	{
		yield();
	}
}

// Delay by specified ms
void delay_ms( uint32_t ms )
{
	uint32_t start = us_now();

	// Loop and yield until the ms count has expired
	while ( ms_now() - start <= ms )
	{
		yield();
	}
}


// - Current Count Functions -

// Current cycle count for CPU
inline uint32_t cycle_now()
{
#if defined(_kinetis_)
	return SYST_CVR;
#elif defined(_sam_)
	return SCB->SHCSR & SCB_SHCSR_SYSTICKACT_Msk;
#else
#warning "cycle_now not implemented"
	return 0;
#endif
}

// Current us count for CPU
// Uses both cycle count and systick
// TODO (HaaTa) - Make it possible to use dynamic F_CPU
//                Possibly using a table of supported frequencies?
uint32_t us_now()
{
#if defined(_kinetis_) || defined(_sam_)
	uint32_t count;
	uint32_t current;
	uint32_t pending;

	// Snapshot both the cycle count and ms counter
	__disable_irq();

	count = systick_millis_count;
	current  = cycle_now();

	#if defined(_kinetis_)
	pending = SCB_ICSR & SCB_ICSR_PENDSTSET; // bit 26 indicates if systick exception pending
	#elif defined(_sam_)
	pending = SCB->ICSR & SCB_ICSR_PENDSTSET_Msk;
	#endif

	__enable_irq();

	// Check for pending systick, and increment if one is it was
	if ( pending && current > ( ( F_CPU / 1000 ) - 50 ) )
	{
		count++;
	}
	// Determine cycles since systick (approx.)
	current = ( ( F_CPU / 1000 ) - 1 ) - current;

	// Add ms and cycles (since systick), converted as us
	return count * 1000 + current / ( F_CPU / 1000000 );
#else
#warning "us_now not implemented"
	return 0;
#endif
}

inline uint32_t ms_now()
{
	return systick_millis_count; // single aligned 32 bit is atomic;
}


// - Misc Functions -

void yield()
{
}

