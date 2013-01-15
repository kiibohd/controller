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
static uint8_t facom6684_ModifierMask[] = { 0x20, 0x2D, 0x2E, 0x30, 0x3E, 0x60 };

static uint8_t facom6684_DefaultMap[] = { 
				0, // 0x00
				0, // 0x01
				0, // 0x02
				0, // 0x03
				0, // 0x04
				0, // 0x05
				0, // 0x06
				0, // 0x07
				0, // 0x08
				0, // 0x09
				0, // 0x0A
				0, // 0x0B
				0, // 0x0C
				0, // 0x0D
				0, // 0x0E
				0, // 0x0F
				0, // 0x10
				0, // 0x11
				0, // 0x12
				0, // 0x13
				0, // 0x14
				0, // 0x15
				0, // 0x16
				0, // 0x17
				0, // 0x18
				0, // 0x19
				0, // 0x1A
				0, // 0x1B
				0, // 0x1C
				0, // 0x1D
				0, // 0x1E
				0, // 0x1F
				KEY_LEFT_SHIFT, // 0x20
				0, // 0x21
				KEY_Z, // 0x22
				KEY_X, // 0x23
				KEY_C, // 0x24
				KEY_V, // 0x25
				KEY_B, // 0x26
				KEY_N, // 0x27
				KEY_M, // 0x28
				KEY_COMMA, // 0x29
				KEY_PERIOD, // 0x2A
				KEY_SLASH, // 0x2B
				0, // 0x2C
				KEY_RIGHT_SHIFT, // 0x2D
				KEY_LEFT_ALT, // 0x2E
				KEY_SPACE, // 0x2F
				KEY_LEFT_CTRL, // 0x30
				KEY_A, // 0x31
				KEY_S, // 0x32
				KEY_D, // 0x33
				KEY_F, // 0x34
				KEY_G, // 0x35
				KEY_H, // 0x36
				KEY_J, // 0x37
				KEY_K, // 0x38
				KEY_L, // 0x39
				KEY_SEMICOLON, // 0x3A
				KEY_QUOTE, // 0x3B
				0, // 0x3C
				KEY_ENTER, // 0x3D
				KEY_RIGHT_GUI, // 0x3E
				KEY_LEFT, // 0x3F (KEYPAD_1)
				KEY_TAB, // 0x40
				KEY_Q, // 0x41
				KEY_W, // 0x42
				KEY_E, // 0x43
				KEY_R, // 0x44
				KEY_T, // 0x45
				KEY_Y, // 0x46
				KEY_U, // 0x47
				KEY_I, // 0x48
				KEY_O, // 0x49
				KEY_P, // 0x4A
				KEY_LEFT_BRACE, // 0x4B
				KEY_RIGHT_BRACE, // 0x4C
				KEY_BACKSLASH, // 0x4D
				KEY_DELETE, // 0x4E
				KEYPAD_4, // 0x4F
				KEY_ESC, // 0x50
				KEY_1, // 0x51
				KEY_2, // 0x52
				KEY_3, // 0x53
				KEY_4, // 0x54
				KEY_5, // 0x55
				KEY_6, // 0x56
				KEY_7, // 0x57
				KEY_8, // 0x58
				KEY_9, // 0x59
				KEY_0, // 0x5A
				KEY_MINUS, // 0x5B
				KEY_EQUAL, // 0x5C
				KEY_TILDE, // 0x5D
				KEY_BACKSPACE, // 0x5E
				KEY_NUM_LOCK, // 0x5F
				KEY_LEFT_GUI, // 0x60
				KEY_HOME, // 0x61
				KEY_END, // 0x62
				KEY_INSERT, // 0x63
				KEY_DELETE, // 0x64
				KEY_F1, // 0x65
				KEY_F2, // 0x66
				KEY_F3, // 0x67
				KEY_F4, // 0x68
				KEY_F5, // 0x69
				KEY_F6, // 0x6A
				KEY_F7, // 0x6B
				KEY_F8, // 0x6C
				KEY_F9, // 0x6D
				KEY_F10, // 0x6E
				KEY_PRINTSCREEN, // 0x6F
				KEY_PAGE_UP, // 0x70
				KEY_PAGE_DOWN, // 0x71
				KEY_F11, // 0x72
				KEYPAD_7, // 0x73
				KEYPAD_8, // 0x74
				KEYPAD_9, // 0x75
				KEY_UP, // 0x76 (KEYPAD_5)
				KEYPAD_6, // 0x77
				KEY_DOWN, // 0x78 (KEYPAD_2)
				KEY_RIGHT, // 0x79 (KEYPAD_3)
				KEYPAD_0, // 0x7A
				KEYPAD_00, // 0x7B
				KEYPAD_ENTER, // 0x7C
				0, // 0x7D
				0, // 0x7E
				0, // 0x7F
};

static uint8_t facom6684_ColemakMap[] = {
				0, // 0x00
				0, // 0x01
				0, // 0x02
				0, // 0x03
				0, // 0x04
				0, // 0x05
				0, // 0x06
				0, // 0x07
				0, // 0x08
				0, // 0x09
				0, // 0x0A
				0, // 0x0B
				0, // 0x0C
				0, // 0x0D
				0, // 0x0E
				0, // 0x0F
				0, // 0x10
				0, // 0x11
				0, // 0x12
				0, // 0x13
				0, // 0x14
				0, // 0x15
				0, // 0x16
				0, // 0x17
				0, // 0x18
				0, // 0x19
				0, // 0x1A
				0, // 0x1B
				0, // 0x1C
				0, // 0x1D
				0, // 0x1E
				0, // 0x1F
				KEY_LEFT_SHIFT, // 0x20
				0, // 0x21
				KEY_Z, // 0x22
				KEY_X, // 0x23
				KEY_C, // 0x24
				KEY_V, // 0x25
				KEY_B, // 0x26
				KEY_K, // 0x27
				KEY_M, // 0x28
				KEY_COMMA, // 0x29
				KEY_PERIOD, // 0x2A
				KEY_SLASH, // 0x2B
				0, // 0x2C
				KEY_RIGHT_SHIFT, // 0x2D
				KEY_LEFT_ALT, // 0x2E
				KEY_SPACE, // 0x2F
				KEY_LEFT_CTRL, // 0x30
				KEY_A, // 0x31
				KEY_R, // 0x32
				KEY_S, // 0x33
				KEY_T, // 0x34
				KEY_D, // 0x35
				KEY_H, // 0x36
				KEY_N, // 0x37
				KEY_E, // 0x38
				KEY_I, // 0x39
				KEY_O, // 0x3A
				KEY_QUOTE, // 0x3B
				0, // 0x3C
				KEY_ENTER, // 0x3D
				KEY_RIGHT_GUI, // 0x3E
				KEY_LEFT, // 0x3F (KEYPAD_1)
				KEY_TAB, // 0x40
				KEY_Q, // 0x41
				KEY_W, // 0x42
				KEY_F, // 0x43
				KEY_P, // 0x44
				KEY_G, // 0x45
				KEY_J, // 0x46
				KEY_L, // 0x47
				KEY_U, // 0x48
				KEY_Y, // 0x49
				KEY_SEMICOLON, // 0x4A
				KEY_LEFT_BRACE, // 0x4B
				KEY_RIGHT_BRACE, // 0x4C
				KEY_BACKSLASH, // 0x4D
				KEY_DELETE, // 0x4E
				KEYPAD_4, // 0x4F
				KEY_ESC, // 0x50
				KEY_1, // 0x51
				KEY_2, // 0x52
				KEY_3, // 0x53
				KEY_4, // 0x54
				KEY_5, // 0x55
				KEY_6, // 0x56
				KEY_7, // 0x57
				KEY_8, // 0x58
				KEY_9, // 0x59
				KEY_0, // 0x5A
				KEY_MINUS, // 0x5B
				KEY_EQUAL, // 0x5C
				KEY_TILDE, // 0x5D
				KEY_BACKSPACE, // 0x5E
				KEY_NUM_LOCK, // 0x5F
				KEY_LEFT_GUI, // 0x60
				KEY_HOME, // 0x61
				KEY_END, // 0x62
				KEY_INSERT, // 0x63
				KEY_DELETE, // 0x64
				KEY_F1, // 0x65
				KEY_F2, // 0x66
				KEY_F3, // 0x67
				KEY_F4, // 0x68
				KEY_F5, // 0x69
				KEY_F6, // 0x6A
				KEY_F7, // 0x6B
				KEY_F8, // 0x6C
				KEY_F9, // 0x6D
				KEY_F10, // 0x6E
				KEY_PRINTSCREEN, // 0x6F
				KEY_PAGE_UP, // 0x70
				KEY_PAGE_DOWN, // 0x71
				KEY_F11, // 0x72
				KEYPAD_7, // 0x73
				KEYPAD_8, // 0x74
				KEYPAD_9, // 0x75
				KEY_UP, // 0x76 (KEYPAD_5)
				KEYPAD_6, // 0x77
				KEY_DOWN, // 0x78 (KEYPAD_2)
				KEY_RIGHT, // 0x79 (KEYPAD_3)
				KEYPAD_0, // 0x7A
				KEYPAD_00, // 0x7B
				KEYPAD_ENTER, // 0x7C
				0, // 0x7D
				0, // 0x7E
				0, // 0x7F
};



#endif

