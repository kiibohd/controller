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

#include <stdint.h>
#include "arm_cortex.h"

Cortex_IRQ get_current_isr()
{
	uint32_t ipsr;
	__asm volatile ("mrs %0, ipsr" : "=r" (ipsr) );

	return (Cortex_IRQ) (ipsr & 0xFF);
}

void __attribute__((naked)) debug_isr ( void ) {
	__asm volatile(
		"tst lr, #4\t\n"
		"ite eq\t\n"
		"mrseq r0, msp\t\n"
		"mrsne r0, psp\t\n"
		"b read_stacked_fault_frame\t\n"
		: // no output
		: // no input
		: "r0" // clobber
	);
}

void read_stacked_fault_frame( uint32_t *faultStackedAddress )
{
	/* These are volatile to try and prevent the compiler/linker optimising them
	 * away as the variables never actually get used.  If the debugger won't show the
	 * values of the variables, make them global my moving their declaration outside
	 * of this function. */
	volatile uint32_t    r0 __attribute__((unused));
	volatile uint32_t    r1 __attribute__((unused));
	volatile uint32_t    r2 __attribute__((unused));
	volatile uint32_t    r3 __attribute__((unused));
	volatile uint32_t   r12 __attribute__((unused));
	volatile uint32_t    lr __attribute__((unused));
	volatile uint32_t    pc __attribute__((unused));
	volatile uint32_t   psr __attribute__((unused));
	volatile   CFSR_t _CFSR __attribute__((unused));
	volatile   HFSR_t _HFSR __attribute__((unused));
	volatile   DFSR_t _DFSR __attribute__((unused));
	volatile uint32_t _AFSR __attribute__((unused));
	volatile uint32_t _BFAR __attribute__((unused));
	volatile uint32_t _MMAR __attribute__((unused));

	r0  = faultStackedAddress[ 0 ];
	r1  = faultStackedAddress[ 1 ];
	r2  = faultStackedAddress[ 2 ];
	r3  = faultStackedAddress[ 3 ];
	r12 = faultStackedAddress[ 4 ];
	lr  = faultStackedAddress[ 5 ];
	pc  = faultStackedAddress[ 6 ];
	psr = faultStackedAddress[ 7 ];

	// Configurable Fault Status Register
	// Consists of MMSR, BFSR and UFSR
	_CFSR = (*((volatile CFSR_t *)(0xE000ED28))) ;

	// Hard Fault Status Register
	_HFSR = (*((volatile HFSR_t *)(0xE000ED2C))) ;

	// Debug Fault Status Register
	_DFSR = (*((volatile DFSR_t *)(0xE000ED30))) ;

	// Auxiliary Fault Status Register ( Vendor Specific )
	_AFSR = (*((volatile unsigned long *)(0xE000ED3C))) ;

	// Fault Address Registers. These may not contain valid values.
	// Check BFARVALID/MMARVALID to see if they are valid values
	
	// MemManage Fault Address Register
	_MMAR = (*((volatile unsigned long *)(0xE000ED34))) ;
	// Bus Fault Address Register
	_BFAR = (*((volatile unsigned long *)(0xE000ED38))) ;

	/* gdb:
	 *      set output-radix 16
	 *      info locals
	 *      x/i _MMAR
	 *      x/i _BFAR
	 *      x/i pc
	 */
	__asm volatile("BKPT #01");
	for( ;; );
}
