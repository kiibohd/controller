/* Copyright (C) 2018 by Jacob Alexander
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

// Debug Includes
#if defined(_bootloader_)
#include <inttypes.h>
#include <debug.h>
#else
#include <print.h>
#endif

// Local Includes
#include "entropy.h"
#include "nrf5.h"



// ----- Variables -----

/* Initialize segments */
extern uint32_t _sfixed;
extern uint32_t _efixed;
extern uint32_t _etext;
extern uint32_t _srelocate;
extern uint32_t _erelocate;
extern uint32_t _szero;
extern uint32_t _ezero;
extern uint32_t _sstack;
extern uint32_t _estack;

uintptr_t __stack_chk_guard = 0xdeadbeef;

//__attribute__((__aligned__(TRACE_BUFFER_SIZE * sizeof(uint32_t)))) uint32_t mtb[TRACE_BUFFER_SIZE];

// ----- Function Declarations -----

extern int main();
void ResetHandler();



// ----- Interrupts -----

// NVIC - Default ISR
void fault_isr()
{
	print("Fault!" NL );

#if defined(DEBUG) && defined(JLINK)
	asm volatile("BKPT #01");
#else
	while ( 1 )
	{
		// keep polling some communication while in fault
		// mode, so we don't completely die.
		/*if ( SIM_SCGC4 & SIM_SCGC4_USBOTG ) usb_isr();
		if ( SIM_SCGC4 & SIM_SCGC4_UART0 )  uart0_status_isr();
		if ( SIM_SCGC4 & SIM_SCGC4_UART1 )  uart1_status_isr();
		if ( SIM_SCGC4 & SIM_SCGC4_UART2 )  uart2_status_isr();*/
	}
#endif
}


// Stack Overflow Interrupt
void __stack_chk_fail(void)
{
	print("Segfault!" NL );
#if defined(DEBUG) && defined(JLINK)
	asm volatile("BKPT #01");
#else
	fault_isr();
#endif
}



// ----- Flash Configuration -----

// ----- Functions -----

void *memset( void *addr, int val, unsigned int len )
{
	char *buf = addr;

	for (; len > 0; --len, ++buf)
		*buf = val;
	return (addr);
}

int memcmp( const void *a, const void *b, unsigned int len )
{
	const uint8_t *ap = a, *bp = b;
	int val = 0;

	for (; len > 0 && (val = *ap - *bp) == 0; --len, ++ap, ++bp)
		/* NOTHING */;
	return (val);
}

void *memcpy( void *dst, const void *src, unsigned int len )
{
// Fast memcpy (Cortex-M4 only), adapted from:
// https://cboard.cprogramming.com/c-programming/154333-fast-memcpy-alternative-32-bit-embedded-processor-posted-just-fyi-fwiw.html#post1149163
// Cortex-M4 can do unaligned accesses, even for 32-bit values
// Uses 32-bit values to do copies instead of 8-bit (should effectively speed up memcpy by 3x)
#if defined(_cortex_m4_)
	uint32_t i;
	uint32_t *pLongSrc;
	uint32_t *pLongDest;
	uint32_t numLongs = len / 4;
	uint32_t endLen = len & 0x03;

	// Convert byte addressing to long addressing
	pLongSrc = (uint32_t*) src;
	pLongDest = (uint32_t*) dst;

	// Copy long values, disregarding any 32-bit alignment issues
	for ( i = 0; i < numLongs; i++ )
	{
		*pLongDest++ = *pLongSrc++;
	}

	// Convert back to byte addressing
	uint8_t *srcbuf = (uint8_t*) pLongSrc;
	uint8_t *dstbuf = (uint8_t*) pLongDest;

	// Copy trailing bytes byte-by-byte
	for (; endLen > 0; --endLen, ++dstbuf, ++srcbuf)
		*dstbuf = *srcbuf;

	return (dst);
#else
	char *dstbuf = dst;
	const char *srcbuf = src;

	for (; len > 0; --len, ++dstbuf, ++srcbuf)
		*dstbuf = *srcbuf;
	return (dst);
#endif
}


// ----- Chip Entry Point -----

void ResetHandler()
{
	// TODO (HaaTa) - Add initialization code, this should be the firmware entry point

	// Enable IRQs
	__enable_irq();

	// Intialize entropy for random numbers
	rand_initialize();

	// Start main
	main();
	while ( 1 ); // Shouldn't get here...
}



// ----- RAM Setup -----

// ----- Interrupt Execution Priority -----

