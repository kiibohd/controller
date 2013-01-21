/* Copyright (C) 2013 by Jacob Alexander
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

#ifndef __FACOM6684_H
#define __FACOM6684_H

// This file contains various key layouts for the Fujitsu FACOM 6684KC3 Supercomputer Terminal Keyboard


// ----- Variables -----
static uint8_t facom6684_ModifierMask[] = { 0x22, 0x12, 0x05, 0x04, 0x03 };

static uint8_t facom6684_DefaultMap[] = { 
				0, // 0x00
				0, // 0x01
				KEY_SPACE, // 0x02
				KEY_LEFT_ALT, // 0x03
				KEY_RIGHT_ALT, // 0x04
				KEY_LEFT_GUI, // 0x05
				0, // 0x06
				0, // 0x07
				KEY_BACKSPACE, // 0x08
				KEY_ENTER, // 0x09
				0, // 0x0A
				0, // 0x0B
				KEYPAD_0, // 0x0C
				KEYPAD_00, // 0x0D
				KEYPAD_000, // 0x0E
				KEYPAD_DECIMAL, // 0x0F
				0, // 0x10
				0, // 0x11
				KEY_LEFT_SHIFT, // 0x12
				0, // 0x13
				KEY_Z, // 0x14
				KEY_X, // 0x15
				KEY_C, // 0x16
				KEY_V, // 0x17
				KEY_B, // 0x18
				KEY_N, // 0x19
				KEY_M, // 0x1A
				KEY_COMMA, // 0x1B
				KEYPAD_1, // 0x1C
				KEYPAD_2, // 0x1D
				KEYPAD_3, // 0x1E
				KEYPAD_ENTER, // 0x1F
				0, // 0x20
				0, // 0x21
				KEY_LEFT_CTRL, // 0x22
				KEY_A, // 0x23
				KEY_S, // 0x24
				KEY_D, // 0x25
				KEY_F, // 0x26
				KEY_G, // 0x27
				KEY_H, // 0x28
				KEY_J, // 0x29
				KEY_K, // 0x2A
				KEY_L, // 0x2B
				KEYPAD_4, // 0x2C
				KEYPAD_5, // 0x2D
				KEYPAD_6, // 0x2E
				KEYPAD_MINUS, // 0x2F
				0, // 0x30
				0, // 0x31
				KEY_TAB, // 0x32
				KEY_Q, // 0x33
				KEY_W, // 0x34
				KEY_E, // 0x35
				KEY_R, // 0x36
				KEY_T, // 0x37
				KEY_Y, // 0x38
				KEY_U, // 0x39
				KEY_I, // 0x3A
				KEY_O, // 0x3B
				KEYPAD_7, // 0x3C
				KEYPAD_8, // 0x3D
				KEYPAD_9, // 0x3E
				KEYPAD_TAB, // 0x3F
				0, // 0x40
				KEY_TILDE, // 0x41
				KEY_ESC, // 0x42
				KEY_1, // 0x43
				KEY_2, // 0x44
				KEY_3, // 0x45
				KEY_4, // 0x46
				KEY_5, // 0x47
				KEY_6, // 0x48
				KEY_7, // 0x49
				KEY_8, // 0x4A
				KEY_9, // 0x4B
				0, // 0x4C
				0, // 0x4D
				KEYPAD_COMMA, // 0x4E
				KEYPAD_SPACE, // 0x4F
				KEY_F1, // 0x50
				KEY_F2, // 0x51
				KEY_F3, // 0x52
				KEY_F4, // 0x53
				KEY_F5, // 0x54
				KEY_F6, // 0x55
				KEY_F7, // 0x56
				KEY_F8, // 0x57
				KEY_0, // 0x58
				KEY_P, // 0x59
				KEY_SEMICOLON, // 0x5A
				KEY_PERIOD, // 0x5B
				KEY_INSERT, // 0x5C
				KEY_LEFT, // 0x5D
				0, // 0x5E
				KEY_DELETE, // 0x5F
				KEY_F13, // 0x60
				KEY_F14, // 0x61
				KEY_F15, // 0x62
				KEY_F16, // 0x63
				KEY_F17, // 0x64
				KEY_F18, // 0x65
				KEY_F9, // 0x66
				KEY_F10, // 0x67
				KEY_MINUS, // 0x68
				KEY_LEFT_BRACE, // 0x69
				KEY_QUOTE, // 0x6A
				KEY_SLASH, // 0x6B
				KEY_HOME, // 0x6C
				KEY_DOWN, // 0x6D
				0, // 0x6E
				KEY_END, // 0x6F
				KEY_F19, // 0x70
				KEY_F20, // 0x71
				KEY_F21, // 0x72
				KEY_F22, // 0x73
				KEY_F23, // 0x74
				KEY_F24, // 0x75
				KEY_F11, // 0x76
				KEY_F12, // 0x77
				KEY_EQUAL, // 0x78
				KEY_RIGHT_BRACE, // 0x79
				KEY_BACKSLASH, // 0x7A
				KEY_UP, // 0x7B
				KEY_PAGE_UP, // 0x7C
				KEY_RIGHT, // 0x7D
				0, // 0x7E
				KEY_PAGE_DOWN, // 0x7F
};

static uint8_t facom6684_ColemakMap[] = {
				0, // 0x00
				0, // 0x01
				KEY_SPACE, // 0x02
				KEY_LEFT_ALT, // 0x03
				KEY_RIGHT_ALT, // 0x04
				KEY_LEFT_GUI, // 0x05
				0, // 0x06
				0, // 0x07
				KEY_BACKSPACE, // 0x08
				KEY_ENTER, // 0x09
				0, // 0x0A
				0, // 0x0B
				KEYPAD_0, // 0x0C
				KEYPAD_00, // 0x0D
				KEYPAD_000, // 0x0E
				KEYPAD_DECIMAL, // 0x0F
				0, // 0x10
				0, // 0x11
				KEY_LEFT_SHIFT, // 0x12
				0, // 0x13
				KEY_Z, // 0x14
				KEY_X, // 0x15
				KEY_C, // 0x16
				KEY_V, // 0x17
				KEY_B, // 0x18
				KEY_K, // 0x19
				KEY_M, // 0x1A
				KEY_COMMA, // 0x1B
				KEYPAD_1, // 0x1C
				KEYPAD_2, // 0x1D
				KEYPAD_3, // 0x1E
				KEYPAD_ENTER, // 0x1F
				0, // 0x20
				0, // 0x21
				KEY_LEFT_CTRL, // 0x22
				KEY_A, // 0x23
				KEY_R, // 0x24
				KEY_S, // 0x25
				KEY_T, // 0x26
				KEY_D, // 0x27
				KEY_H, // 0x28
				KEY_N, // 0x29
				KEY_E, // 0x2A
				KEY_I, // 0x2B
				KEYPAD_4, // 0x2C
				KEYPAD_5, // 0x2D
				KEYPAD_6, // 0x2E
				KEYPAD_MINUS, // 0x2F
				0, // 0x30
				0, // 0x31
				KEY_TAB, // 0x32
				KEY_Q, // 0x33
				KEY_W, // 0x34
				KEY_F, // 0x35
				KEY_P, // 0x36
				KEY_G, // 0x37
				KEY_J, // 0x38
				KEY_L, // 0x39
				KEY_U, // 0x3A
				KEY_Y, // 0x3B
				KEYPAD_7, // 0x3C
				KEYPAD_8, // 0x3D
				KEYPAD_9, // 0x3E
				KEYPAD_TAB, // 0x3F
				0, // 0x40
				KEY_TILDE, // 0x41
				KEY_ESC, // 0x42
				KEY_1, // 0x43
				KEY_2, // 0x44
				KEY_3, // 0x45
				KEY_4, // 0x46
				KEY_5, // 0x47
				KEY_6, // 0x48
				KEY_7, // 0x49
				KEY_8, // 0x4A
				KEY_9, // 0x4B
				0, // 0x4C
				0, // 0x4D
				KEYPAD_COMMA, // 0x4E
				KEYPAD_SPACE, // 0x4F
				KEY_F1, // 0x50
				KEY_F2, // 0x51
				KEY_F3, // 0x52
				KEY_F4, // 0x53
				KEY_F5, // 0x54
				KEY_F6, // 0x55
				KEY_F7, // 0x56
				KEY_F8, // 0x57
				KEY_0, // 0x58
				KEY_SEMICOLON, // 0x59
				KEY_O, // 0x5A
				KEY_PERIOD, // 0x5B
				KEY_INSERT, // 0x5C
				KEY_LEFT, // 0x5D
				0, // 0x5E
				KEY_DELETE, // 0x5F
				KEY_F13, // 0x60
				KEY_F14, // 0x61
				KEY_F15, // 0x62
				KEY_F16, // 0x63
				KEY_F17, // 0x64
				KEY_F18, // 0x65
				KEY_F9, // 0x66
				KEY_F10, // 0x67
				KEY_MINUS, // 0x68
				KEY_LEFT_BRACE, // 0x69
				KEY_QUOTE, // 0x6A
				KEY_SLASH, // 0x6B
				KEY_HOME, // 0x6C
				KEY_DOWN, // 0x6D
				0, // 0x6E
				KEY_END, // 0x6F
				KEY_F19, // 0x70
				KEY_F20, // 0x71
				KEY_F21, // 0x72
				KEY_F22, // 0x73
				KEY_F23, // 0x74
				KEY_F24, // 0x75
				KEY_F11, // 0x76
				KEY_F12, // 0x77
				KEY_EQUAL, // 0x78
				KEY_RIGHT_BRACE, // 0x79
				KEY_BACKSLASH, // 0x7A
				KEY_UP, // 0x7B
				KEY_PAGE_UP, // 0x7C
				KEY_RIGHT, // 0x7D
				0, // 0x7E
				KEY_PAGE_DOWN, // 0x7F
};



#endif

