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

#ifndef __TANDY1000_H
#define __TANDY1000_H

// This file contains various key layouts for the Tandy 1000 keyboard


// ----- Variables -----

static uint8_t tandy1000_modifierMask[] = { 0x1D, 0x2A, 0x36, 0x38, 0x46 };

static uint8_t tandy1000_map[] = { 0,
				KEY_ESC,
				KEY_1,
				KEY_2,
				KEY_3,
				KEY_4,
				KEY_5,
				KEY_6,
				KEY_7,
				KEY_8,
				KEY_9,
				KEY_0,
				KEY_MINUS,
				KEY_EQUAL,
				KEY_BACKSPACE,
				KEY_TAB,
				KEY_Q,
				KEY_W,
				KEY_E,
				KEY_R,
				KEY_T,
				KEY_Y,
				KEY_U,
				KEY_I,
				KEY_O,
				KEY_P,
				KEY_LEFT_BRACE,
				KEY_RIGHT_BRACE,
				KEY_ENTER,
				KEY_CTRL, // 0x1D
				KEY_A,
				KEY_S,
				KEY_D,
				KEY_F,
				KEY_G,
				KEY_H,
				KEY_J,
				KEY_K,
				KEY_L,
				KEY_SEMICOLON,
				KEY_QUOTE,
				KEY_UP,
				KEY_LEFT_SHIFT, // 0x2A
				KEY_LEFT,
				KEY_Z,
				KEY_X,
				KEY_C,
				KEY_V,
				KEY_B,
				KEY_N,
				KEY_M,
				KEY_COMMA,
				KEY_PERIOD,
				KEY_SLASH,
				KEY_RIGHT_SHIFT, // 0x36
				KEY_PRINTSCREEN,
				KEY_ALT, // 0x38
				KEY_SPACE,
				KEY_CAPS_LOCK,
				KEY_F1,
				KEY_F2,
				KEY_F3,
				KEY_F4,
				KEY_F5,
				KEY_F6,
				KEY_F7,
				KEY_F8,
				KEY_F9,
				KEY_F10,
				KEY_NUM_LOCK,
				KEY_GUI, // Actually Hold... 0x48
				KEY_BACKSLASH, // Also KEYPAD_7
				KEY_TILDE, // Also KEYPAD_8
				KEYPAD_9,
				KEY_UP,
				KEY_BACKSLASH, // Actually | and KEYPAD_4
				KEYPAD_5,
				KEYPAD_6,
				KEY_RIGHT,
				KEYPAD_1,
				KEY_TILDE, // Actually ` and KEYPAD_2
				KEYPAD_3,
				KEYPAD_0,
				KEY_DELETE,
				KEY_PAUSE,
				KEY_INSERT,
				KEYPAD_PERIOD,
				KEYPAD_ENTER,
				KEY_HOME,
				KEY_F11,
				KEY_F12, // 0x5A
};

static uint8_t tandy1000_colemak[] = { 0,
				KEY_ESC,
				KEY_1,
				KEY_2,
				KEY_3,
				KEY_4,
				KEY_5,
				KEY_6,
				KEY_7,
				KEY_8,
				KEY_9,
				KEY_0,
				KEY_MINUS,
				KEY_EQUAL,
				KEY_BACKSPACE,
				KEY_TAB,
				KEY_Q,
				KEY_W,
				KEY_F,
				KEY_P,
				KEY_G,
				KEY_J,
				KEY_L,
				KEY_U,
				KEY_Y,
				KEY_SEMICOLON,
				KEY_LEFT_BRACE,
				KEY_RIGHT_BRACE,
				KEY_ENTER,
				KEY_CTRL, // 0x1D
				KEY_A,
				KEY_R,
				KEY_S,
				KEY_T,
				KEY_D,
				KEY_H,
				KEY_N,
				KEY_E,
				KEY_I,
				KEY_O,
				KEY_QUOTE,
				KEY_UP,
				KEY_LEFT_SHIFT, // 0x2A
				KEY_LEFT,
				KEY_Z,
				KEY_X,
				KEY_C,
				KEY_V,
				KEY_B,
				KEY_K,
				KEY_M,
				KEY_COMMA,
				KEY_PERIOD,
				KEY_SLASH,
				KEY_RIGHT_SHIFT, // 0x36
				KEY_PRINTSCREEN,
				KEY_ALT, // 0x38
				KEY_SPACE,
				0, //KEY_CAPS_LOCK,
				KEY_F1,
				KEY_F2,
				KEY_F3,
				KEY_F4,
				KEY_F5,
				KEY_F6,
				KEY_F7,
				KEY_F8,
				KEY_F9,
				KEY_F10,
				0, //KEY_NUM_LOCK,
				KEY_GUI, // Actually Hold... 0x48
				KEY_BACKSLASH, // Also KEYPAD_7
				KEY_TILDE, // Also KEYPAD_8
				KEYPAD_9,
				KEY_DOWN,
				KEY_BACKSLASH, // Actually | and KEYPAD_4
				KEYPAD_5,
				KEYPAD_6,
				KEY_RIGHT,
				KEYPAD_1,
				KEY_TILDE, // Actually ` and KEYPAD_2
				KEYPAD_3,
				KEYPAD_0,
				KEY_DELETE,
				KEY_PAUSE,
				KEY_INSERT,
				KEYPAD_PERIOD,
				KEYPAD_ENTER,
				KEY_HOME,
				KEY_F11,
				KEY_F12, // 0x5A
};



#endif

