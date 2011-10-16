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

#ifndef __BUDKEYPAD_H
#define __BUDKEYPAD_H

// This file contains various key layouts for the Bud Hall Effect Keypad (16 key)


// ----- Variables -----

static uint8_t budkeypad_ModifierMask[] = {};

// Default 1-indexed key mappings
static uint8_t budkeypad_DefaultMap[] = { 0,
				KEYPAD_7,
				KEYPAD_8,
				KEYPAD_9,
				KEYPAD_SLASH,
				KEYPAD_4,
				KEYPAD_5,
				KEYPAD_6,
				KEYPAD_ASTERIX,
				KEYPAD_1,
				KEYPAD_2,
				KEYPAD_3,
				KEYPAD_MINUS,
				KEYPAD_ENTER,
				KEYPAD_0,
				KEYPAD_PERIOD,
				KEYPAD_PLUS,
};

static uint8_t budkeypad_TheProfosistMap[] = { 0,
				KEY_7,
				KEY_8,
				KEY_9,
				KEYPAD_SLASH,
				KEY_4,
				KEY_5,
				KEY_6,
				KEYPAD_ASTERIX,
				KEY_1,
				KEY_2,
				KEY_3,
				KEYPAD_MINUS,
				KEY_0,
				KEYPAD_PERIOD,
				KEYPAD_ENTER,
				KEYPAD_PLUS,
};



#endif

