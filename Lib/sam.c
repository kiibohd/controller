/* Copyright (C) 2017 by Jacob Alexander
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
#include "sam.h"



// ----- Variables -----

// ----- Function Declarations -----

extern int main();
void ResetHandler();



// ----- Interrupts -----

// ----- Flash Configuration -----

// ----- Functions -----

// ----- Chip Entry Point -----

// __attribute__ ((section(".startup"))) XXX (HaaTa) Necessary?
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

