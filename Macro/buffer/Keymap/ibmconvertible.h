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

#ifndef __IBMCONV_H
#define __IBMCONV_H

// This file contains various key layouts for the IBM Convertible keyboard


// ----- Variables -----

static uint8_t  ibmconv_ModifierMask [] = { 0x2C, 0x38, 0x3A, 0x3B, 0x3C, 0x3E };

// Default 1-indexed key mappings
static uint8_t ibmconv_DefaultMap[] = {
				0, // 0x00
				KEY_TILDE, // 0x01
				KEY_1, // 0x02
				KEY_2, // 0x03
				KEY_3, // 0x04
				KEY_4, // 0x05
				KEY_5, // 0x06
				KEY_6, // 0x07
				KEY_7, // 0x08 (7)
				KEY_8, // 0x09 (8)
				KEY_9, // 0x0A (9)
				KEY_0, // 0x0B
				KEY_MINUS, // 0x0C (-)
				KEY_EQUAL, // 0x0D (+)
				KEY_BACKSLASH, // 0x0E
				KEY_BACKSPACE, // 0x0F
				KEY_TAB, // 0x10
				KEY_Q, // 0x11
				KEY_W, // 0x12
				KEY_E, // 0x13
				KEY_R, // 0x14
				KEY_T, // 0x15
				KEY_Y, // 0x16
				KEY_U, // 0x17 (4)
				KEY_I, // 0x18 (5)
				KEY_O, // 0x19 (6)
				KEY_P, // 0x1A
				KEY_LEFT_BRACE, // 0x1B
				KEY_RIGHT_BRACE, // 0x1C
				0, // 0x1D
				KEY_CAPS_LOCK, // 0x1E
				KEY_A, // 0x1F
				KEY_S, // 0x20
				KEY_D, // 0x21
				KEY_F, // 0x22
				KEY_G, // 0x23
				KEY_H, // 0x24
				KEY_J, // 0x25 (1)
				KEY_K, // 0x26 (2)
				KEY_L, // 0x27 (3)
				KEY_SEMICOLON, // 0x28
				KEY_QUOTE, // 0x29
				0, // 0x2A (1/4)
				KEY_ENTER, // 0x2B
				KEY_LEFT_SHIFT, // 0x2C
				0, // 0x2D
				KEY_Z, // 0x2E
				KEY_X, // 0x2F
				KEY_C, // 0x30
				KEY_V, // 0x31
				KEY_B, // 0x32
				KEY_N, // 0x33
				KEY_M, // 0x34 (0)
				KEY_COMMA, // 0x35
				KEY_PERIOD, // 0x36 (Decimal)
				KEY_SLASH, // 0x37 (/)
				KEY_RIGHT_SHIFT, // 0x38
				KEY_PRINTSCREEN, // 0x39 (*)
				KEY_LEFT_CTRL, // 0x3A
				KEY_LEFT_GUI, // 0x3B
				KEY_LEFT_ALT, // 0x3C
				KEY_SPACE, // 0x3D
				KEY_RIGHT_ALT, // 0x3E
				KEY_LEFT, // 0x3F (Home)
				KEY_UP, // 0x40 (PgUp)
				KEY_DOWN, // 0x41 (PgDn)
				KEY_RIGHT, // 0x42 (End)
				KEY_ESC, // 0x43
				KEY_F1, // 0x44
				KEY_F2, // 0x45
				KEY_F3, // 0x46
				KEY_F4, // 0x47
				KEY_F5, // 0x48
				KEY_F6, // 0x49
				KEY_F7, // 0x4A
				KEY_F8, // 0x4B
				KEY_F9, // 0x4C
				KEY_F10, // 0x4D
				KEY_NUM_LOCK, // 0x4E
				KEY_SCROLL_LOCK, // 0x4F
				KEY_INSERT, // 0x50
				KEY_DELETE, // 0x51
};

static uint8_t ibmconv_ColemakMap[] = {
				0, // 0x00
				KEY_TILDE, // 0x01
				KEY_1, // 0x02
				KEY_2, // 0x03
				KEY_3, // 0x04
				KEY_4, // 0x05
				KEY_5, // 0x06
				KEY_6, // 0x07
				KEY_7, // 0x08 (7)
				KEY_8, // 0x09 (8)
				KEY_9, // 0x0A (9)
				KEY_0, // 0x0B
				KEY_MINUS, // 0x0C (-)
				KEY_EQUAL, // 0x0D (+)
				KEY_BACKSLASH, // 0x0E
				KEY_BACKSPACE, // 0x0F
				KEY_TAB, // 0x10
				KEY_Q, // 0x11
				KEY_W, // 0x12
				KEY_F, // 0x13
				KEY_P, // 0x14
				KEY_G, // 0x15
				KEY_J, // 0x16
				KEY_L, // 0x17 (4)
				KEY_U, // 0x18 (5)
				KEY_Y, // 0x19 (6)
				KEY_SEMICOLON, // 0x1A
				KEY_LEFT_BRACE, // 0x1B
				KEY_RIGHT_BRACE, // 0x1C
				0, // 0x1D
				KEY_CAPS_LOCK, // 0x1E
				KEY_A, // 0x1F
				KEY_R, // 0x20
				KEY_S, // 0x21
				KEY_T, // 0x22
				KEY_D, // 0x23
				KEY_H, // 0x24
				KEY_N, // 0x25 (1)
				KEY_E, // 0x26 (2)
				KEY_I, // 0x27 (3)
				KEY_O, // 0x28
				KEY_QUOTE, // 0x29
				0, // 0x2A (1/4)
				KEY_ENTER, // 0x2B
				KEY_LEFT_SHIFT, // 0x2C
				0, // 0x2D
				KEY_Z, // 0x2E
				KEY_X, // 0x2F
				KEY_C, // 0x30
				KEY_V, // 0x31
				KEY_B, // 0x32
				KEY_K, // 0x33
				KEY_M, // 0x34 (0)
				KEY_COMMA, // 0x35
				KEY_PERIOD, // 0x36 (Decimal)
				KEY_SLASH, // 0x37 (/)
				KEY_RIGHT_SHIFT, // 0x38
				KEY_PRINTSCREEN, // 0x39 (*)
				KEY_LEFT_CTRL, // 0x3A
				KEY_LEFT_GUI, // 0x3B
				KEY_LEFT_ALT, // 0x3C
				KEY_SPACE, // 0x3D
				KEY_RIGHT_ALT, // 0x3E
				KEY_LEFT, // 0x3F (Home)
				KEY_UP, // 0x40 (PgUp)
				KEY_DOWN, // 0x41 (PgDn)
				KEY_RIGHT, // 0x42 (End)
				KEY_ESC, // 0x43
				KEY_F1, // 0x44
				KEY_F2, // 0x45
				KEY_F3, // 0x46
				KEY_F4, // 0x47
				KEY_F5, // 0x48
				KEY_F6, // 0x49
				KEY_F7, // 0x4A
				KEY_F8, // 0x4B
				KEY_F9, // 0x4C
				KEY_F10, // 0x4D
				KEY_NUM_LOCK, // 0x4E
				KEY_SCROLL_LOCK, // 0x4F
				KEY_INSERT, // 0x50
				KEY_DELETE, // 0x51
};



#endif

