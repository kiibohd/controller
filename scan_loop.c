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


#include <stdint.h>
#include "usb_keyboard_debug.h"
#include "keymap.h"
// Debouncing Defines
#define SAMPLE_THRESHOLD 110
#define MAX_SAMPLES 127 // Max is 127, reaching 128 is very bad
// Loop over all of the sampled keys of the given array
// If the number of samples is higher than the sample threshold, flag the high bit, clear otherwise
// This should be resetting VERY quickly, cutting off a potentially valid keypress is not an issue
#define DEBOUNCE_ASSESS(table,size) \
			for ( uint8_t key = 1; key < size + 1; key++ ) {\
				table[key] = ( table[key] & ~(1 << 7) ) > SAMPLE_THRESHOLD ? (1 << 7) : 0x00; \
			} \

// NOTE: Highest Bit: Valid keypress (0x80 is valid keypress)
//        Other Bits: Pressed state sample counter
#define KEYBOARD_SIZE 23
uint8_t keyboardDetectArray[KEYBOARD_SIZE + 1];

// Interrupt Variable
volatile uint8_t sendKeypresses = 0;

// USB Data Send
void usb_send( uint8_t validKeys )
{
		// TODO undo potentially old keys
		for ( uint8_t c = validKeys; c < 6; c++ )
			keyboard_keys[c] = 0;

		// Send keypresses
		usb_keyboard_send();

		// Clear sendKeypresses Flag
		sendKeypresses = 0;

		// Clear modifiers
		keyboard_modifier_keys = 0;
}


// Given a sampling array, and the current number of detected keypress
// Add as many keypresses from the sampling array to the USB key send array as possible.
void keyPressDetection( uint8_t *keys, uint8_t *validKeys, uint8_t numberOfKeys, uint8_t *modifiers, uint8_t numberOfModifiers, uint8_t *map ) {
	for ( uint8_t key = 0; key < numberOfKeys + 1; key++ ) {
		if ( keys[key] & (1 << 7) ) {
			pint8( key );
			//print(" ");
			uint8_t modFound = 0;

			// Determine if the key is a modifier
			for ( uint8_t mod = 0; mod < numberOfModifiers; mod++ ) {
				// Modifier found
				if ( modifiers[mod] == key ) {
					keyboard_modifier_keys |= map[key];
					modFound = 1;
					break;
				}
			}
			if ( modFound )
				continue;

			// Too many keys
			if ( *validKeys == 6 )
				break;

			// Allow ignoring keys with 0's
			if ( map[key] != 0 )
				keyboard_keys[(*validKeys)++] = map[key];
		}
	}
}


// Main Detection Loop
void scan_loop( void )
{
	//matrix_pinSetup( matrix_pinout );
	uint8_t count = 0;

	for ( ;; ) {
		//matrix_scan( matrix_pinout, keyboardDetectArray );

		// Check count to see if the sample threshold may have been reached, otherwise collect more data
		if ( count++ < MAX_SAMPLES )
			continue;

		// Reset Sample Counter
		count = 0;

		// Assess debouncing sample table
		//DEBOUNCE_ASSESS(keyDetectArray,KEYBOARD_SIZE)

		// Send keypresses over USB if the ISR has signalled that it's time
		if ( !sendKeypresses )
			continue;

		// Layout Setup
		uint8_t validKeys = 0;

		uint8_t *keyboard_MODMASK = keyboard_modifierMask;
		uint8_t  keyboard_NUMMODS = MODIFIERS_KEYBOARD;
		uint8_t *keyboard_MAP     = defaultMap;

		// TODO Layout Switching

		// TODO Macro Processing

		// Debounce Sampling Array to USB Data Array
		keyPressDetection( keyboardDetectArray, &validKeys, KEYBOARD_SIZE, keyboard_MODMASK, keyboard_NUMMODS, keyboard_MAP );

		// Send USB Data
		usb_send( validKeys );
	}
}

