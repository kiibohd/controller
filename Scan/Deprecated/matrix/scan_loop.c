/* Copyright (C) 2011-2012,2014 by Jacob Alexander
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
#include <Lib/ScanLib.h>

// Project Includes
#include <led.h>
#include <macro.h>
#include <print.h>

// Local Includes
#include "scan_loop.h"
#include "matrix_scan.h"



// ----- Defines -----

// Debouncing Defines
// Old
//#define SAMPLE_THRESHOLD 110
//#define MAX_SAMPLES      127 // Max is 127, reaching 128 is very bad
#define SAMPLE_THRESHOLD 6
#define MAX_SAMPLES      10 // Max is 127, reaching 128 is very bad



// ----- Macros -----



// ----- Variables -----

// Buffer used to inform the macro processing module which keys have been detected as pressed
volatile uint8_t KeyIndex_Buffer[KEYBOARD_BUFFER];
volatile uint8_t KeyIndex_BufferUsed;
volatile uint8_t KeyIndex_Add_InputSignal; // Used to pass the (click/input value) to the keyboard for the clicker


// Keeps track of the number of scans, so we only do a debounce assess when it would be valid (as it throws away data)
uint8_t Scan_count = 0;

// This is where the matrix scan data is held, as well as debouncing is evaluated to, which (depending on the read value) is handled
//  by the macro module
uint8_t KeyIndex_Array[KEYBOARD_KEYS + 1];



// ----- Functions -----

// Setup
inline void Scan_setup()
{
	matrix_pinSetup( (uint8_t*)matrix_pinout, scanMode );
}

// Main Detection Loop
inline uint8_t Scan_loop()
{
	// Check count to see if the sample threshold may have been reached, otherwise collect more data
	if ( Scan_count < MAX_SAMPLES )
	{
		matrix_scan( (uint8_t*)matrix_pinout, KeyIndex_Array );

		// scanDual requires 2 passes, and thus needs more memory per matrix_scan pass
#if scanMode == scanDual
		Scan_count += 2;
#else
		Scan_count++;
#endif

		// Signal Main Detection Loop to continue scanning
		return 0;
	}

	// Reset Sample Counter
	Scan_count = 0;

	// Assess debouncing sample table
	// Loop over all of the sampled keys of the given array
	// If the number of samples is higher than the sample threshold, flag the high bit, clear otherwise
	// This should be resetting VERY quickly, cutting off a potentially valid keypress is not an issue
	for ( uint8_t key = 1; key < KeyIndex_Size + 1; key++ ) if ( ( KeyIndex_Array[key] & ~(1 << 7) ) > SAMPLE_THRESHOLD )
	{
		// Debug output (keypress detected)
		printHex( key );
		print(" ");

		// Add the key to the buffer, if it isn't already in the current Key Buffer
		for ( uint8_t c = 0; c < KeyIndex_BufferUsed + 1; c++ )
		{
			// Key isn't in the buffer yet
			if ( c == KeyIndex_BufferUsed )
			{
				Macro_bufferAdd( key );
				break;
			}

			// Key already in the buffer
			if ( KeyIndex_Buffer[c] == key )
				break;
		}

		KeyIndex_Array[key] = (1 << 7);
	}
	else
	{
		// Remove the key from the buffer only if it was previously known to be pressed
		if ( KeyIndex_Array[key] & (1 << 7 ) )
		{
			// Check for the released key, and shift the other keys lower on the buffer
			for ( uint8_t c = 0; c < KeyIndex_BufferUsed; c++ )
			{
				// Key to release found
				if ( KeyIndex_Buffer[c] == key )
				{
					// Shift keys from c position
					for ( uint8_t k = c; k < KeyIndex_BufferUsed - 1; k++ )
						KeyIndex_Buffer[k] = KeyIndex_Buffer[k + 1];

					// Decrement Buffer
					KeyIndex_BufferUsed--;

					break;
				}
			}
		}

		KeyIndex_Array[key] = 0x00;
	}

	// Ready to allow for USB send
	return 1;
}


// Signal that the USB keycodes have been properly sent through the Output Module
inline void Scan_finishedWithUSBBuffer( uint8_t sentKeys )
{
	return;
}


// Signal KeyIndex_Buffer that it has been fully processed using the macro module
inline void Scan_finishedWithBuffer( uint8_t sentKeys )
{
	return;
}

