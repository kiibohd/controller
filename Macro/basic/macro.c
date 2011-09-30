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

// Project Includes
#include <usb_com.h>
#include <scan_loop.h>
#include <print.h>

// Keymaps
#include <keymap.h>
#include <usb_keys.h>

// Local Includes
#include "macro.h"



// ----- Functions -----

// Given a sampling array, and the current number of detected keypress
// Add as many keypresses from the sampling array to the USB key send array as possible.
void keyPressDetection( uint8_t *keys, uint8_t *validKeys, uint8_t numberOfKeys, uint8_t *modifiers, uint8_t numberOfModifiers, uint8_t *map )
{
	for ( uint8_t key = 0; key < numberOfKeys + 1; key++ ) {
		if ( keys[key] & (1 << 7) ) {
			// TODO Debug Out
			uint8_t modFound = 0;

			// Determine if the key is a modifier
			for ( uint8_t mod = 0; mod < numberOfModifiers; mod++ ) {
				// Modifier found
				if ( modifiers[mod] == key ) {
					USBKeys_Modifiers |= map[key];
					modFound = 1;
					break;
				}
			}
			if ( modFound )
				continue;

			// Too many keys
			if ( *validKeys >= USBKeys_MaxSize )
				break;

			// Allow ignoring keys with 0's
			if ( map[key] != 0 )
				USBKeys_Array[(*validKeys)++] = map[key];
		}
	}
}

void process_macros(void)
{
	// Layout Setup
	uint8_t validKeys = 0;

	uint8_t *keyboard_MODMASK = keyboard_modifierMask;
	uint8_t  keyboard_NUMMODS = MODIFIERS_KEYBOARD;
	uint8_t *keyboard_MAP     = defaultMap;

	// Debounce Sampling Array to USB Data Array
	keyPressDetection( KeyIndex_Array, &validKeys, KeyIndex_Size, keyboard_MODMASK, keyboard_NUMMODS, keyboard_MAP );
}

