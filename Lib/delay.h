/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2013 PJRC.COM, LLC.
 * Modifications by Jacob Alexander 2013-2015
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

#pragma once

// ----- System Includes -----

#include <stdint.h>



// ----- Macros -----

// Convenience Macros, for delay compatibility with AVR-GCC
#define _delay_ms(val) delay( val )
#define _delay_us(val) delayMicroseconds( val )



// ----- Functions -----

// the systick interrupt is supposed to increment this at 1 kHz rate
extern volatile uint32_t systick_millis_count;

static inline uint32_t millis(void) __attribute__((always_inline, unused));
static inline uint32_t millis(void)
{
	return systick_millis_count; // single aligned 32 bit is atomic;
}


#ifndef _host_
static inline void delayMicroseconds(uint32_t) __attribute__((always_inline, unused));
static inline void delayMicroseconds(uint32_t usec)
{
#if F_CPU == 96000000
	uint32_t n = usec << 5;
#elif F_CPU == 72000000
	uint32_t n = usec << 5; // XXX Not accurate, assembly snippet needs to be updated
#elif F_CPU == 48000000
	uint32_t n = usec << 4;
#elif F_CPU == 24000000
	uint32_t n = usec << 3;
#endif
	asm volatile(
		"L_%=_delayMicroseconds:"               "\n\t"
		"subs   %0, #1"                         "\n\t"
		"bne    L_%=_delayMicroseconds"         "\n"
		: "+r" (n) :
	);
}
#endif


void yield(void) __attribute__ ((weak));

uint32_t micros(void);

void delay(uint32_t ms);

