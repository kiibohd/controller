/* Copyright (C) 2012 by Jacob Alexander
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

#ifndef __BETKB_H
#define __BETKB_H

// This file contains various key layouts for the Burroughs Ergonomic Terminal Keyboard (Kokusai)


// ----- Variables -----
static uint8_t betkb_ModifierMask[] = { 0x48, 0x49, 0x4C, 0x4D, 0x45, 0x14 };

static uint8_t betkb_DefaultMap[] = { 
				KEY_TILDE, // 0x00
				KEY_UP, // 0x01
				0, // 0x02
				0, // 0x03
				KEY_PRINTSCREEN, // 0x04
				KEY_PAGE_DOWN, // 0x05
				KEY_LEFT_BRACE, // 0x06
				KEY_F11, // 0x07
				KEY_BACKSPACE, // 0x08
				KEY_TAB, // 0x09
				KEY_ENTER, // 0x0A
				KEY_DOWN, // 0x0B
				KEY_PAGE_UP, // 0x0C
				KEYPAD_ENTER, // 0x0D
				KEY_LEFT, // 0x0E
				KEY_RIGHT, // 0x0F
				0, // 0x10
				KEY_PAGE_UP, // 0x11
				KEY_F12, // 0x12
				KEY_PAGE_DOWN, // 0x13
				KEY_GUI, // 0x14
				KEY_F1, // 0x15
				KEY_F2, // 0x16
				KEY_F3, // 0x17
				KEY_F4, // 0x18
				KEY_F5, // 0x19
				KEY_F6, // 0x1A
				KEY_ESC, // 0x1B
				KEY_F7, // 0x1C
				KEY_F8, // 0x1D
				KEY_F9, // 0x1E
				KEY_F10, // 0x1F
				KEY_SPACE, // 0x20
				KEYPAD_9, // 0x21
				0, // 0x22
				0, // 0x23
				0, // 0x24
				0, // 0x25
				0, // 0x26
				KEY_QUOTE, // 0x27
				0, // 0x28
				0, // 0x29
				0, // 0x2A
				KEY_EQUAL, // 0x2B
				KEY_COMMA, // 0x2C
				KEY_MINUS, // 0x2D
				KEY_PERIOD, // 0x2E
				KEY_SLASH, // 0x2F
				KEY_0, // 0x30
				KEY_1, // 0x31
				KEY_2, // 0x32
				KEY_3, // 0x33
				KEY_4, // 0x34
				KEY_5, // 0x35
				KEY_6, // 0x36
				KEY_7, // 0x37
				KEY_8, // 0x38
				KEY_9, // 0x39
				0, // 0x3A
				KEY_SEMICOLON, // 0x3B
				0, // 0x3C
				KEY_EQUAL, // 0x3D
				0, // 0x3E
				0, // 0x3F
				0, // 0x40
				KEYPAD_6, // 0x41
				KEYPAD_MINUS, // 0x42
				KEY_PAUSE, // 0x43
				KEY_INSERT, // 0x44
				KEY_CTRL, // 0x45
				KEYPAD_2, // 0x46
				KEYPAD_3, // 0x47
				KEY_LEFT_SHIFT, // 0x48
				KEY_RIGHT_SHIFT, // 0x49
				KEYPAD_0, // 0x4A
				KEYPAD_PERIOD, // 0x4B
				KEY_LEFT_ALT, // 0x4C
				KEY_RIGHT_ALT, // 0x4D
				0, // 0x4E
				0, // 0x4F
				0, // 0x50
				0, // 0x51
				0, // 0x52
				0, // 0x53
				0, // 0x54
				0, // 0x55
				0, // 0x56
				0, // 0x57
				0, // 0x58
				0, // 0x59
				0, // 0x5A
				KEY_RIGHT_BRACE, // 0x5B
				KEYPAD_7, // 0x5C
				KEY_BACKSLASH, // 0x5D
				KEY_ESC, // 0x5E
				0, // 0x5F
				KEYPAD_1, // 0x60
				KEY_A, // 0x61
				KEY_B, // 0x62
				KEY_C, // 0x63
				KEY_D, // 0x64
				KEY_E, // 0x65
				KEY_F, // 0x66
				KEY_G, // 0x67
				KEY_H, // 0x68
				KEY_I, // 0x69
				KEY_J, // 0x6A
				KEY_K, // 0x6B
				KEY_L, // 0x6C
				KEY_M, // 0x6D
				KEY_N, // 0x6E
				KEY_O, // 0x6F
				KEY_P, // 0x70
				KEY_Q, // 0x71
				KEY_R, // 0x72
				KEY_S, // 0x73
				KEY_T, // 0x74
				KEY_U, // 0x75
				KEY_V, // 0x76
				KEY_W, // 0x77
				KEY_X, // 0x78
				KEY_Y, // 0x79
				KEY_Z, // 0x7A
				KEYPAD_4, // 0x7B
				KEYPAD_8, // 0x7C
				KEYPAD_5, // 0x7D
				0, // 0x7E
				KEY_DELETE, // 0x7F
};

static uint8_t betkb_ColemakMap[] = {
				KEY_TILDE, // 0x00
				KEY_UP, // 0x01
				0, // 0x02
				0, // 0x03
				KEY_PRINTSCREEN, // 0x04
				KEY_PAGE_DOWN, // 0x05
				KEY_LEFT_BRACE, // 0x06
				KEY_F11, // 0x07
				KEY_BACKSPACE, // 0x08
				KEY_TAB, // 0x09
				KEY_ENTER, // 0x0A
				KEY_DOWN, // 0x0B
				KEY_PAGE_UP, // 0x0C
				KEYPAD_ENTER, // 0x0D
				KEY_LEFT, // 0x0E
				KEY_RIGHT, // 0x0F
				0, // 0x10
				KEY_PAGE_UP, // 0x11
				KEY_F12, // 0x12
				KEY_PAGE_DOWN, // 0x13
				KEY_GUI, // 0x14
				KEY_F1, // 0x15
				KEY_F2, // 0x16
				KEY_F3, // 0x17
				KEY_F4, // 0x18
				KEY_F5, // 0x19
				KEY_F6, // 0x1A
				KEY_ESC, // 0x1B
				KEY_F7, // 0x1C
				KEY_F8, // 0x1D
				KEY_F9, // 0x1E
				KEY_F10, // 0x1F
				KEY_SPACE, // 0x20
				KEYPAD_9, // 0x21
				0, // 0x22
				0, // 0x23
				0, // 0x24
				0, // 0x25
				0, // 0x26
				KEY_QUOTE, // 0x27
				0, // 0x28
				0, // 0x29
				0, // 0x2A
				KEY_EQUAL, // 0x2B
				KEY_COMMA, // 0x2C
				KEY_MINUS, // 0x2D
				KEY_PERIOD, // 0x2E
				KEY_SLASH, // 0x2F
				KEY_0, // 0x30
				KEY_1, // 0x31
				KEY_2, // 0x32
				KEY_3, // 0x33
				KEY_4, // 0x34
				KEY_5, // 0x35
				KEY_6, // 0x36
				KEY_7, // 0x37
				KEY_8, // 0x38
				KEY_9, // 0x39
				0, // 0x3A
				KEY_O, // 0x3B
				0, // 0x3C
				KEY_EQUAL, // 0x3D
				0, // 0x3E
				0, // 0x3F
				0, // 0x40
				KEYPAD_6, // 0x41
				KEYPAD_MINUS, // 0x42
				KEY_PAUSE, // 0x43
				KEY_INSERT, // 0x44
				KEY_CTRL, // 0x45
				KEYPAD_2, // 0x46
				KEYPAD_3, // 0x47
				KEY_LEFT_SHIFT, // 0x48
				KEY_RIGHT_SHIFT, // 0x49
				KEYPAD_0, // 0x4A
				KEYPAD_PERIOD, // 0x4B
				KEY_LEFT_ALT, // 0x4C
				KEY_RIGHT_ALT, // 0x4D
				0, // 0x4E
				0, // 0x4F
				0, // 0x50
				0, // 0x51
				0, // 0x52
				0, // 0x53
				0, // 0x54
				0, // 0x55
				0, // 0x56
				0, // 0x57
				0, // 0x58
				0, // 0x59
				0, // 0x5A
				KEY_RIGHT_BRACE, // 0x5B
				KEYPAD_7, // 0x5C
				KEY_BACKSLASH, // 0x5D
				KEY_ESC, // 0x5E
				0, // 0x5F
				KEYPAD_1, // 0x60
				KEY_A, // 0x61
				KEY_B, // 0x62
				KEY_C, // 0x63
				KEY_S, // 0x64
				KEY_F, // 0x65
				KEY_T, // 0x66
				KEY_D, // 0x67
				KEY_H, // 0x68
				KEY_U, // 0x69
				KEY_N, // 0x6A
				KEY_E, // 0x6B
				KEY_I, // 0x6C
				KEY_M, // 0x6D
				KEY_K, // 0x6E
				KEY_Y, // 0x6F
				KEY_SEMICOLON, // 0x70
				KEY_Q, // 0x71
				KEY_P, // 0x72
				KEY_R, // 0x73
				KEY_G, // 0x74
				KEY_L, // 0x75
				KEY_V, // 0x76
				KEY_W, // 0x77
				KEY_X, // 0x78
				KEY_J, // 0x79
				KEY_Z, // 0x7A
				KEYPAD_4, // 0x7B
				KEYPAD_8, // 0x7C
				KEYPAD_5, // 0x7D
				0, // 0x7E
				KEY_DELETE, // 0x7F
};



#endif

