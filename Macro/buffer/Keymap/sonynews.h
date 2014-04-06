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

#ifndef __SONYNEWS_H
#define __SONYNEWS_H

// This file contains various key layouts for the Sony NEWS Topre Keyboard
// TODO



// ----- Variables -----
static uint8_t sonynews_ModifierMask[] = { 0x28, 0x36, 0x42, 0x43, 0x4A };

static uint8_t sonynews_DefaultMap[] = { 
				0x00, // 0x00
				KEY_F1, // 0x01
				KEY_F2, // 0x02
				KEY_F3, // 0x03
				KEY_F4, // 0x04
				KEY_F5, // 0x05
				KEY_F6, // 0x06
				KEY_F7, // 0x07
				KEY_F8, // 0x08
				KEY_F9, // 0x09
				KEY_F10, // 0x0A
				KEY_ESC, // 0x0B
				KEY_1, // 0x0C
				KEY_2, // 0x0D
				KEY_3, // 0x0E
				KEY_4, // 0x0F
				KEY_5, // 0x10
				KEY_6, // 0x11
				KEY_7, // 0x12
				KEY_8, // 0x13
				KEY_9, // 0x14
				KEY_0, // 0x15
				KEY_MINUS, // 0x16
				KEY_EQUAL, // 0x17
				KEY_INTER3, // 0x18 - Yen
				KEY_BACKSPACE, // 0x19
				KEY_TAB, // 0x1A
				KEY_Q, // 0x1B
				KEY_W, // 0x1C
				KEY_E, // 0x1D
				KEY_R, // 0x1E
				KEY_T, // 0x1F
				KEY_Y, // 0x20
				KEY_U, // 0x21
				KEY_I, // 0x22
				KEY_O, // 0x23
				KEY_P, // 0x24
				KEY_LEFT_BRACE, // 0x25
				KEY_RIGHT_BRACE, // 0x26
				KEY_DELETE, // 0x27
				KEY_CTRL, // 0x28
				KEY_A, // 0x29
				KEY_S, // 0x2A
				KEY_D, // 0x2B
				KEY_F, // 0x2C
				KEY_G, // 0x2D
				KEY_H, // 0x2E
				KEY_J, // 0x2F
				KEY_K, // 0x30
				KEY_L, // 0x31
				KEY_SEMICOLON, // 0x32
				KEY_QUOTE, // 0x33
				KEY_BACKSLASH, // 0x34
				KEY_ENTER, // 0x35
				KEY_LEFT_SHIFT, // 0x36
				KEY_Z, // 0x37
				KEY_X, // 0x38
				KEY_C, // 0x39
				KEY_V, // 0x3A
				KEY_B, // 0x3B
				KEY_N, // 0x3C
				KEY_M, // 0x3D
				KEY_COMMA, // 0x3E
				KEY_PERIOD, // 0x3F
				KEY_SLASH, // 0x40
				KEY_INTER1, // 0x41 - "Ru" and "-"
				KEY_RIGHT_SHIFT, // 0x42
				KEY_ALT, // 0x43
				KEY_CAPS_LOCK, // 0x44
				KEY_INTER5, // 0x45 - Muhenkan
				KEY_SPACE, // 0x46
				KEY_INTER4, // 0x47 - Henkan
				KEY_LANG2, // 0x48 - Eisu (English/Numbers) (I've seen references to this mapping)
				KEY_INTER2, // 0x49 - Kana
				KEY_EXEC, // 0x4A - Jikkou XXX Which means Execute, and since there is no Language key
				KEYPAD_7, // 0x4B
				KEYPAD_8, // 0x4C
				KEYPAD_9, // 0x4D
				KEYPAD_MINUS, // 0x4E
				KEYPAD_4, // 0x4F
				KEYPAD_5, // 0x50
				KEYPAD_6, // 0x51
				KEYPAD_PLUS, // 0x52
				KEYPAD_1, // 0x53
				KEYPAD_2, // 0x54
				KEYPAD_3, // 0x55
				KEYPAD_COMMA, // 0x56
				KEYPAD_0, // 0x57
				KEY_UP, // 0x58
				KEYPAD_PERIOD, // 0x59
				KEYPAD_ENTER, // 0x5A
				KEY_LEFT, // 0x5B
				KEY_DOWN, // 0x5C
				KEY_RIGHT, // 0x5D
				0, // 0x5E
				0, // 0x5F
				0, // 0x60
				0, // 0x61
				0, // 0x62
				0, // 0x63
				KEYPAD_ASTERIX, // 0x64
				KEYPAD_SLASH, // 0x65
				KEYPAD_TAB, // 0x66
				0, // 0x67
				KEY_F11, // 0x68
				KEY_F12, // 0x69
				KEY_HELP, // 0x6A
				KEY_INSERT, // 0x6B
				KEY_CLEAR, // 0x6C
				KEY_PAGE_UP, // 0x6D
				KEY_PAGE_DOWN, // 0x6E
				0, // 0x6F
				0, // 0x70
				0, // 0x71
				0, // 0x72
				0, // 0x73
				0, // 0x74
				0, // 0x75
				0, // 0x76
				0, // 0x77
				0, // 0x78
				0, // 0x79
				KEY_F13, // 0x7A
				0, // 0x7B
				0, // 0x7C
				0, // 0x7D
				0, // 0x7E
				0, // 0x7F
};

static uint8_t sonynews_ColemakMap[] = {
				0x00, // 0x00
				KEY_F1, // 0x01
				KEY_F2, // 0x02
				KEY_F3, // 0x03
				KEY_F4, // 0x04
				KEY_F5, // 0x05
				KEY_F6, // 0x06
				KEY_F7, // 0x07
				KEY_F8, // 0x08
				KEY_F9, // 0x09
				KEY_F10, // 0x0A
				KEY_ESC, // 0x0B
				KEY_1, // 0x0C
				KEY_2, // 0x0D
				KEY_3, // 0x0E
				KEY_4, // 0x0F
				KEY_5, // 0x10
				KEY_6, // 0x11
				KEY_7, // 0x12
				KEY_8, // 0x13
				KEY_9, // 0x14
				KEY_0, // 0x15
				KEY_MINUS, // 0x16
				KEY_EQUAL, // 0x17
				KEY_INTER3, // 0x18 - Yen
				KEY_BACKSPACE, // 0x19
				KEY_TAB, // 0x1A
				KEY_Q, // 0x1B
				KEY_W, // 0x1C
				KEY_F, // 0x1D
				KEY_P, // 0x1E
				KEY_G, // 0x1F
				KEY_J, // 0x20
				KEY_L, // 0x21
				KEY_U, // 0x22
				KEY_Y, // 0x23
				KEY_SEMICOLON, // 0x24
				KEY_LEFT_BRACE, // 0x25
				KEY_RIGHT_BRACE, // 0x26
				KEY_DELETE, // 0x27
				KEY_CTRL, // 0x28
				KEY_A, // 0x29
				KEY_R, // 0x2A
				KEY_S, // 0x2B
				KEY_T, // 0x2C
				KEY_D, // 0x2D
				KEY_H, // 0x2E
				KEY_N, // 0x2F
				KEY_E, // 0x30
				KEY_I, // 0x31
				KEY_O, // 0x32
				KEY_QUOTE, // 0x33
				KEY_BACKSLASH, // 0x34
				KEY_ENTER, // 0x35
				KEY_LEFT_SHIFT, // 0x36
				KEY_Z, // 0x37
				KEY_X, // 0x38
				KEY_C, // 0x39
				KEY_V, // 0x3A
				KEY_B, // 0x3B
				KEY_K, // 0x3C
				KEY_M, // 0x3D
				KEY_COMMA, // 0x3E
				KEY_PERIOD, // 0x3F
				KEY_SLASH, // 0x40
				KEY_INTER1, // 0x41 - "Ru" and "-"
				KEY_RIGHT_SHIFT, // 0x42
				KEY_ALT, // 0x43
				KEY_CAPS_LOCK, // 0x44
				KEY_INTER5, // 0x45 - Muhenkan
				KEY_SPACE, // 0x46
				KEY_INTER4, // 0x47 - Henkan
				KEY_LANG2, // 0x48 - Eisu (English/Numbers) (I've seen references to this mapping)
				KEY_INTER2, // 0x49 - Kana
				//KEY_EXEC, // 0x4A - Jikkou XXX Which means Execute, and since there is no Language key
				KEY_GUI, // 0x4A - Using Jikkou as Windows Key
				KEYPAD_7, // 0x4B
				KEYPAD_8, // 0x4C
				KEYPAD_9, // 0x4D
				KEYPAD_MINUS, // 0x4E
				KEYPAD_4, // 0x4F
				KEYPAD_5, // 0x50
				KEYPAD_6, // 0x51
				KEYPAD_PLUS, // 0x52
				KEYPAD_1, // 0x53
				KEYPAD_2, // 0x54
				KEYPAD_3, // 0x55
				KEYPAD_COMMA, // 0x56
				KEYPAD_0, // 0x57
				KEY_UP, // 0x58
				KEYPAD_PERIOD, // 0x59
				KEYPAD_ENTER, // 0x5A
				KEY_LEFT, // 0x5B
				KEY_DOWN, // 0x5C
				KEY_RIGHT, // 0x5D
				0, // 0x5E
				0, // 0x5F
				0, // 0x60
				0, // 0x61
				0, // 0x62
				0, // 0x63
				KEYPAD_ASTERIX, // 0x64
				KEYPAD_SLASH, // 0x65
				KEYPAD_TAB, // 0x66
				0, // 0x67
				KEY_F11, // 0x68
				KEY_F12, // 0x69
				KEY_HELP, // 0x6A
				KEY_INSERT, // 0x6B
				KEY_CLEAR, // 0x6C
				KEY_PAGE_UP, // 0x6D
				KEY_PAGE_DOWN, // 0x6E
				0, // 0x6F
				0, // 0x70
				0, // 0x71
				0, // 0x72
				0, // 0x73
				0, // 0x74
				0, // 0x75
				0, // 0x76
				0, // 0x77
				0, // 0x78
				0, // 0x79
				KEY_F13, // 0x7A
				0, // 0x7B
				0, // 0x7C
				0, // 0x7D
				0, // 0x7E
				0, // 0x7F
};



#endif

