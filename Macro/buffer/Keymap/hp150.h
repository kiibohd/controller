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

#ifndef __HP150_H
#define __HP150_H

// This file contains various key layouts for the HP150 Keyboard


// ----- Variables -----
static uint8_t hp150_ModifierMask[] = { 0x2F, 0x39, 0x3A, 0x3F, 0x45, 0x46, 0x76 };

static uint8_t hp150_DefaultMap[] = { 
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
				KEY_MENU, // 0x0C
				KEY_VOL_DOWN, // 0x0D
				KEY_VOL_UP, // 0x0E
				KEY_F8, // 0x0F
				KEY_F7, // 0x10
				KEY_F6, // 0x11
				KEY_F5, // 0x12
				KEY_PRINTSCREEN, // 0x13
				KEY_8, // 0x14
				KEY_END, // 0x15
				KEY_HOME, // 0x16
				KEY_BACKSPACE, // 0x17
				KEY_EQUAL, // 0x18
				KEY_MINUS, // 0x19
				KEY_0, // 0x1A
				KEY_9, // 0x1B
				KEY_I, // 0x1C
				KEY_DELETE, // 0x1D
				KEY_INSERT, // 0x1E
				KEY_BACKSLASH, // 0x1F
				KEY_RIGHT_BRACE, // 0x20
				KEY_LEFT_BRACE, // 0x21
				KEY_P, // 0x22
				KEY_O, // 0x23
				KEY_J, // 0x24
				KEY_PAGE_UP, // 0x25
				KEY_NUM_LOCK, // 0x26
				KEY_ENTER, // 0x27
				KEY_QUOTE, // 0x28
				KEY_SEMICOLON, // 0x29
				KEY_L, // 0x2A
				KEY_K, // 0x2B
				KEY_M, // 0x2C
				KEY_PAGE_DOWN, // 0x2D
				KEY_UP, // 0x2E
				KEY_RIGHT_GUI, // 0x2F
				KEY_1, // 0x30
				KEY_SLASH, // 0x31
				KEY_PERIOD, // 0x32
				KEY_COMMA, // 0x33
				0, // 0x34
				KEY_RIGHT, // 0x35
				KEY_DOWN, // 0x36
				KEY_LEFT, // 0x37
				KEY_RIGHT_GUI, // 0x38
				KEY_RIGHT_ALT, // 0x39
				KEY_LEFT_ALT, // 0x3A
				KEY_SPACE, // 0x3B
				KEY_F4, // 0x3C
				KEY_G, // 0x3D
				KEY_NUMBER, // 0x3E
				KEY_CTRL, // 0x3F
				KEY_TILDE, // 0x40
				KEY_F1, // 0x41
				KEY_F2, // 0x42
				KEY_F3, // 0x43
				KEY_7, // 0x44
				KEY_LEFT_SHIFT, // 0x45
				KEY_RIGHT_SHIFT, // 0x46
				KEY_CAPS_LOCK, // 0x47
				KEY_3, // 0x48
				KEY_4, // 0x49
				KEY_5, // 0x4A
				KEY_6, // 0x4B
				KEY_U, // 0x4C
				KEY_TAB, // 0x4D
				KEY_Q, // 0x4E
				KEY_W, // 0x4F
				KEY_E, // 0x50
				KEY_R, // 0x51
				KEY_T, // 0x52
				KEY_Y, // 0x53
				KEY_H, // 0x54
				KEY_2, // 0x55
				KEY_LEFT_GUI, // 0x56
				KEY_A, // 0x57
				KEY_S, // 0x58
				KEY_D, // 0x59
				KEY_F, // 0x5A
				KEY_ESC, // 0x5B
				KEYPAD_COMMA, // 0x5C
				KEYPAD_TAB, // 0x5D
				KEYPAD_6, // 0x5E
				KEYPAD_9, // 0x5F
				KEYPAD_5, // 0x60
				KEYPAD_8, // 0x61
				KEYPAD_4, // 0x62
				KEYPAD_7, // 0x63
				KEY_TAB, // 0x64
				KEYPAD_MINUS, // 0x65
				KEYPAD_3, // 0x66
				KEYPAD_PLUS, // 0x67
				KEYPAD_2, // 0x68
				KEYPAD_SLASH, // 0x69
				KEYPAD_1, // 0x6A
				KEYPAD_ASTERIX, // 0x6B
				0, // 0x6C
				KEY_F12, // 0x6D
				KEYPAD_PERIOD, // 0x6E
				KEY_F11, // 0x6F
				0, // 0x70
				KEY_F10, // 0x71
				KEYPAD_0, // 0x72
				KEY_F9, // 0x73
				KEY_N, // 0x74
				KEY_ESC, // 0x75
				KEY_GUI, // 0x76
				KEY_Z, // 0x77
				KEY_X, // 0x78
				KEY_C, // 0x79
				KEY_V, // 0x7A
				KEY_B, // 0x7B
				0, // 0x7C
				0, // 0x7D
				0, // 0x7E
				0, // 0x7F
};

static uint8_t hp150_ColemakMap[] = {
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
				KEY_MENU, // 0x0C
				KEY_VOL_DOWN, // 0x0D
				KEY_VOL_UP, // 0x0E
				KEY_F8, // 0x0F
				KEY_F7, // 0x10
				KEY_F6, // 0x11
				KEY_F5, // 0x12
				KEY_PRINTSCREEN, // 0x13
				KEY_8, // 0x14
				KEY_END, // 0x15
				KEY_HOME, // 0x16
				KEY_BACKSPACE, // 0x17
				KEY_EQUAL, // 0x18
				KEY_MINUS, // 0x19
				KEY_0, // 0x1A
				KEY_9, // 0x1B
				KEY_U, // 0x1C
				KEY_DELETE, // 0x1D
				KEY_INSERT, // 0x1E
				KEY_BACKSLASH, // 0x1F
				KEY_RIGHT_BRACE, // 0x20
				KEY_LEFT_BRACE, // 0x21
				KEY_SEMICOLON, // 0x22
				KEY_Y, // 0x23
				KEY_N, // 0x24
				KEY_PAGE_UP, // 0x25
				KEY_NUM_LOCK, // 0x26
				KEY_ENTER, // 0x27
				KEY_QUOTE, // 0x28
				KEY_O, // 0x29
				KEY_I, // 0x2A
				KEY_E, // 0x2B
				KEY_M, // 0x2C
				KEY_PAGE_DOWN, // 0x2D
				KEY_UP, // 0x2E
				KEY_RIGHT_GUI, // 0x2F
				KEY_1, // 0x30
				KEY_SLASH, // 0x31
				KEY_PERIOD, // 0x32
				KEY_COMMA, // 0x33
				0, // 0x34
				KEY_RIGHT, // 0x35
				KEY_DOWN, // 0x36
				KEY_LEFT, // 0x37
				KEY_RIGHT_GUI, // 0x38
				KEY_RIGHT_ALT, // 0x39
				KEY_LEFT_ALT, // 0x3A
				KEY_SPACE, // 0x3B
				KEY_F4, // 0x3C
				KEY_D, // 0x3D
				KEY_NUMBER, // 0x3E
				KEY_CTRL, // 0x3F
				KEY_TILDE, // 0x40
				KEY_F1, // 0x41
				KEY_F2, // 0x42
				KEY_F3, // 0x43
				KEY_7, // 0x44
				KEY_LEFT_SHIFT, // 0x45
				KEY_RIGHT_SHIFT, // 0x46
				KEY_CAPS_LOCK, // 0x47
				KEY_3, // 0x48
				KEY_4, // 0x49
				KEY_5, // 0x4A
				KEY_6, // 0x4B
				KEY_L, // 0x4C
				KEY_TAB, // 0x4D
				KEY_Q, // 0x4E
				KEY_W, // 0x4F
				KEY_F, // 0x50
				KEY_P, // 0x51
				KEY_G, // 0x52
				KEY_J, // 0x53
				KEY_H, // 0x54
				KEY_2, // 0x55
				KEY_LEFT_GUI, // 0x56
				KEY_A, // 0x57
				KEY_R, // 0x58
				KEY_S, // 0x59
				KEY_T, // 0x5A
				KEY_ESC, // 0x5B
				KEYPAD_COMMA, // 0x5C
				KEYPAD_TAB, // 0x5D
				KEYPAD_6, // 0x5E
				KEYPAD_9, // 0x5F
				KEYPAD_5, // 0x60
				KEYPAD_8, // 0x61
				KEYPAD_4, // 0x62
				KEYPAD_7, // 0x63
				KEY_TAB, // 0x64
				KEYPAD_MINUS, // 0x65
				KEYPAD_3, // 0x66
				KEYPAD_PLUS, // 0x67
				KEYPAD_2, // 0x68
				KEYPAD_SLASH, // 0x69
				KEYPAD_1, // 0x6A
				KEYPAD_ASTERIX, // 0x6B
				0, // 0x6C
				KEY_F12, // 0x6D
				KEYPAD_PERIOD, // 0x6E
				KEY_F11, // 0x6F
				0, // 0x70
				KEY_F10, // 0x71
				KEYPAD_0, // 0x72
				KEY_F9, // 0x73
				KEY_K, // 0x74
				KEY_ESC, // 0x75
				KEY_GUI, // 0x76
				KEY_Z, // 0x77
				KEY_X, // 0x78
				KEY_C, // 0x79
				KEY_V, // 0x7A
				KEY_B, // 0x7B
				0, // 0x7C
				0, // 0x7D
				0, // 0x7E
				0, // 0x7F
};



#endif

