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

#ifndef __EPSONQX10_H
#define __EPSONQX10_H

// This file contains various key layouts for the Epson QX-10 Series of keyboards
// NOTE: Modifiers can't be remapped to non-modifer keys due to how the signals are interpretted
//       Remapping between modifier keys is perfectly fine (or even a modifier to a normal key)



// ----- Variables -----
static uint8_t epsonqx10_ModifierMask[] = { 0x81, 0x83, 0x85, 0x87, 0x89, 0x8B, 0x8D, 0x8F };

static uint8_t epsonqx10_DefaultMap[] = { 
				0x00, // 0x00
				KEY_F4, // 0x01
				0, // 0x02
				KEY_F5, // 0x03
				KEY_F6, // 0x04
				KEY_F7, // 0x05
				KEY_F8, // 0x06
				KEY_F9, // 0x07
				0, // 0x08
				KEY_F10, // 0x09
				KEY_F11, // 0x0A
				KEY_F12, // 0x0B
				KEY_F13, // 0x0C
				0, // 0x0D
				KEY_F14, // 0x0E
				KEY_F15, // 0x0F
				0, // 0x10
				0, // 0x11
				0, // 0x12
				0, // 0x13
				0, // 0x14
				KEYPAD_ENTER, // 0x15
				KEYPAD_PERIOD, // 0x16
				KEYPAD_0, // 0x17
				KEYPAD_EQUAL, // 0x18
				KEYPAD_6, // 0x19
				KEYPAD_5, // 0x1A
				KEYPAD_4, // 0x1B
				0, // 0x1C
				0, // 0x1D
				KEY_F17, // 0x1E
				KEY_F16, // 0x1F
				0, // 0x20
				0, // 0x21
				0, // 0x22
				0, // 0x23
				0, // 0x24
				KEYPAD_3, // 0x25
				KEYPAD_2, // 0x26
				KEYPAD_1, // 0x27
				KEYPAD_PLUS, // 0x28
				KEYPAD_9, // 0x29
				KEYPAD_8, // 0x2A
				KEYPAD_7, // 0x2B
				KEYPAD_MINUS, // 0x2C
				KEYPAD_ASTERIX, // 0x2D
				KEYPAD_SLASH, // 0x2E
				KEY_NUM_LOCK, // 0x2F
				0, // 0x30
				0, // 0x31
				KEY_SPACE, // 0x32
				KEY_Z, // 0x33
				KEY_X, // 0x34
				KEY_C, // 0x35
				KEY_V, // 0x36
				KEY_B, // 0x37
				KEY_N, // 0x38
				KEY_M, // 0x39
				KEY_COMMA, // 0x3A
				KEY_PERIOD, // 0x3B
				KEY_UP, // 0x3C
				KEY_LEFT, // 0x3D
				KEY_RIGHT, // 0x3E
				KEY_DOWN, // 0x3F
				0, // 0x40
				KEY_F19, // 0x41
				KEY_CAPS_LOCK, // 0x42
				KEY_A, // 0x43
				KEY_S, // 0x44
				KEY_D, // 0x45
				KEY_F, // 0x46
				KEY_G, // 0x47
				KEY_H, // 0x48
				KEY_J, // 0x49
				KEY_K, // 0x4A
				KEY_L, // 0x4B
				KEY_SEMICOLON, // 0x4C
				KEY_QUOTE, // 0x4D
				KEY_ENTER, // 0x4E
				KEY_SLASH, // 0x4F
				0, // 0x50
				KEY_Q, // 0x51
				KEY_W, // 0x52
				KEY_E, // 0x53
				KEY_R, // 0x54
				KEY_T, // 0x55
				KEY_Y, // 0x56
				KEY_U, // 0x57
				KEY_I, // 0x58
				KEY_O, // 0x59
				KEY_P, // 0x5A
				KEY_LEFT_BRACE, // 0x5B
				KEY_RIGHT_BRACE, // 0x5C
				KEY_BACKSLASH, // 0x5D
				KEY_INSERT, // 0x5E
				KEY_PAGE_DOWN, // 0x5F
				0, // 0x60
				KEY_2, // 0x61
				KEY_3, // 0x62
				KEY_4, // 0x63
				KEY_5, // 0x64
				KEY_6, // 0x65
				KEY_7, // 0x66
				KEY_8, // 0x67
				KEY_9, // 0x68
				KEY_0, // 0x69
				KEY_MINUS, // 0x6A
				KEY_EQUAL, // 0x6B
				KEY_TILDE, // 0x6C
				KEY_BACKSPACE, // 0x6D
				KEY_DELETE, // 0x6E
				KEY_PAGE_UP, // 0x6F
				0, // 0x70
				KEY_F3, // 0x71
				KEY_F2, // 0x72
				KEY_F1, // 0x73
				KEY_F18, // 0x74
				KEY_ESC, // 0x75
				KEY_1, // 0x76
				KEY_TAB, // 0x77
				KEY_F19, // 0x78
				0, // 0x79
				0, // 0x7A
				0, // 0x7B
				0, // 0x7C
				0, // 0x7D
				0, // 0x7E
				0, // 0x7F
				0, // 0x80
				0, // 0x81
				0, // 0x82
				0, // 0x83
				0, // 0x84
				KEY_RIGHT_SHIFT, // 0x85
				0, // 0x86
				KEY_LEFT_SHIFT, // 0x87
				0, // 0x88
				0, // 0x89
				0, // 0x8A
				KEY_LEFT_CTRL, // 0x8B
				0, // 0x8C
				KEY_GUI, // 0x8D
				0, // 0x8E
				KEY_RIGHT_CTRL, // 0x8F
};

static uint8_t epsonqx10_ColemakMap[] = {
				0x00, // 0x00
				KEY_F4, // 0x01
				0, // 0x02
				KEY_F5, // 0x03
				KEY_F6, // 0x04
				KEY_F7, // 0x05
				KEY_F8, // 0x06
				KEY_F9, // 0x07
				0, // 0x08
				KEY_F10, // 0x09
				KEY_F11, // 0x0A
				KEY_F12, // 0x0B
				KEY_F13, // 0x0C
				0, // 0x0D
				KEY_F14, // 0x0E
				KEY_F15, // 0x0F
				0, // 0x10
				0, // 0x11
				0, // 0x12
				0, // 0x13
				0, // 0x14
				KEYPAD_ENTER, // 0x15
				KEYPAD_PERIOD, // 0x16
				KEYPAD_0, // 0x17
				KEYPAD_EQUAL, // 0x18
				KEYPAD_6, // 0x19
				KEYPAD_5, // 0x1A
				KEYPAD_4, // 0x1B
				0, // 0x1C
				0, // 0x1D
				KEY_F17, // 0x1E
				KEY_F16, // 0x1F
				0, // 0x20
				0, // 0x21
				0, // 0x22
				0, // 0x23
				0, // 0x24
				KEYPAD_3, // 0x25
				KEYPAD_2, // 0x26
				KEYPAD_1, // 0x27
				KEYPAD_PLUS, // 0x28
				KEYPAD_9, // 0x29
				KEYPAD_8, // 0x2A
				KEYPAD_7, // 0x2B
				KEYPAD_MINUS, // 0x2C
				KEYPAD_ASTERIX, // 0x2D
				KEYPAD_SLASH, // 0x2E
				KEY_NUM_LOCK, // 0x2F
				0, // 0x30
				0, // 0x31
				KEY_SPACE, // 0x32
				KEY_Z, // 0x33
				KEY_X, // 0x34
				KEY_C, // 0x35
				KEY_V, // 0x36
				KEY_B, // 0x37
				KEY_K, // 0x38
				KEY_M, // 0x39
				KEY_COMMA, // 0x3A
				KEY_PERIOD, // 0x3B
				KEY_UP, // 0x3C
				KEY_LEFT, // 0x3D
				KEY_RIGHT, // 0x3E
				KEY_DOWN, // 0x3F
				0, // 0x40
				KEY_F19, // 0x41
				KEY_CAPS_LOCK, // 0x42
				KEY_A, // 0x43
				KEY_R, // 0x44
				KEY_S, // 0x45
				KEY_T, // 0x46
				KEY_D, // 0x47
				KEY_H, // 0x48
				KEY_N, // 0x49
				KEY_E, // 0x4A
				KEY_I, // 0x4B
				KEY_O, // 0x4C
				KEY_QUOTE, // 0x4D
				KEY_ENTER, // 0x4E
				KEY_SLASH, // 0x4F
				0, // 0x50
				KEY_Q, // 0x51
				KEY_W, // 0x52
				KEY_F, // 0x53
				KEY_P, // 0x54
				KEY_G, // 0x55
				KEY_J, // 0x56
				KEY_L, // 0x57
				KEY_U, // 0x58
				KEY_Y, // 0x59
				KEY_SEMICOLON, // 0x5A
				KEY_LEFT_BRACE, // 0x5B
				KEY_RIGHT_BRACE, // 0x5C
				KEY_BACKSLASH, // 0x5D
				KEY_INSERT, // 0x5E
				KEY_PAGE_DOWN, // 0x5F
				0, // 0x60
				KEY_2, // 0x61
				KEY_3, // 0x62
				KEY_4, // 0x63
				KEY_5, // 0x64
				KEY_6, // 0x65
				KEY_7, // 0x66
				KEY_8, // 0x67
				KEY_9, // 0x68
				KEY_0, // 0x69
				KEY_MINUS, // 0x6A
				KEY_EQUAL, // 0x6B
				KEY_TILDE, // 0x6C
				KEY_BACKSPACE, // 0x6D
				KEY_DELETE, // 0x6E
				KEY_PAGE_UP, // 0x6F
				0, // 0x70
				KEY_F3, // 0x71
				KEY_F2, // 0x72
				KEY_F1, // 0x73
				KEY_F18, // 0x74
				KEY_ESC, // 0x75
				KEY_1, // 0x76
				KEY_TAB, // 0x77
				KEY_F19, // 0x78
				0, // 0x79
				0, // 0x7A
				0, // 0x7B
				0, // 0x7C
				0, // 0x7D
				0, // 0x7E
				0, // 0x7F
				0, // 0x80
				0, // 0x81
				0, // 0x82
				0, // 0x83
				0, // 0x84
				KEY_RIGHT_SHIFT, // 0x85
				0, // 0x86
				KEY_LEFT_SHIFT, // 0x87
				0, // 0x88
				0, // 0x89
				0, // 0x8A
				KEY_LEFT_CTRL, // 0x8B
				0, // 0x8C
				KEY_GUI, // 0x8D
				0, // 0x8E
				KEY_ALT, // 0x8F
};



#endif

