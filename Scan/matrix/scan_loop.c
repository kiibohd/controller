/* Copyright (C) 2011 by Jacob Alexander
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

// AVR Includes
#include <avr/interrupt.h>

// Project Includes
#include <led.h>
#include <print.h>

// Local Includes
#include "scan_loop.h"
#include "matrix_scan.h"



// ----- Defines -----

// Debouncing Defines
#define SAMPLE_THRESHOLD 110
#define MAX_SAMPLES      127 // Max is 127, reaching 128 is very bad



// ----- Macros -----

// Loop over all of the sampled keys of the given array
// If the number of samples is higher than the sample threshold, flag the high bit, clear otherwise
// This should be resetting VERY quickly, cutting off a potentially valid keypress is not an issue
#define DEBOUNCE_ASSESS(table,size) \
			for ( uint8_t key = 1; key < size + 1; key++ ) \
				table[key] = ( table[key] & ~(1 << 7) ) > SAMPLE_THRESHOLD ? (1 << 7) : 0x00



// ----- Variables -----

// Keeps track of the number of scans, so we only do a debounce assess when it would be valid (as it throws away data)
uint8_t scan_count = 0;

// This is where the matrix scan data is held, as well as debouncing is evaluated to, which (depending on the read value) is handled
//  by the macro module
uint8_t KeyIndex_Array[KEYBOARD_SIZE + 1];



// ----- Functions -----

// Setup
inline void scan_setup()
{
	matrix_pinSetup( (uint8_t*)matrix_pinout );
}

// Main Detection Loop
inline uint8_t scan_loop()
{
	// Check count to see if the sample threshold may have been reached, otherwise collect more data
	if ( scan_count < MAX_SAMPLES )
	{
		matrix_scan( (uint8_t*)matrix_pinout, KeyIndex_Array );

		// scanDual requires 2 passes, and thus needs more memory per matrix_scan pass
#if scanMode == scanDual
		scan_count += 2;
#else
		scan_count++;
#endif

		// Signal Main Detection Loop to continue scanning
		return 0;
	}

	// Reset Sample Counter
	scan_count = 0;

	// Assess debouncing sample table
	DEBOUNCE_ASSESS( KeyIndex_Array, KeyIndex_Size );

	// Ready to allow for USB send
	return 1;
}

