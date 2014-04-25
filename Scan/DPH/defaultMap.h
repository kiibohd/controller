/* Copyright (C) 2011-2013 by Joseph Makuch
 * Additions by Jacob Alexander (2013-2014)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __KEYMAP_H
#define __KEYMAP_H

// This file contains the a basic US ANSI-like layout for the Kishsaver using the AVR DPH Capsense Controller


// ----- Variables -----

// Default 1-indexed key mappings
static uint8_t DefaultMap_Lookup[] = {
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
				0, // 0x20
				0, // 0x21
				0, // 0x22
				0, // 0x23
				0, // 0x24
				0, // 0x25
				0, // 0x26
				0, // 0x27
				0, // 0x28
				0, // 0x29
				0, // 0x2A
				0, // 0x2B
				0, // 0x2C
				0, // 0x2D
				0, // 0x2E
				0, // 0x2F
				0, // 0x30
				0, // 0x31
				0, // 0x32
				0, // 0x33
				0, // 0x34
				0, // 0x35
				0, // 0x36
				0, // 0x37
				0, // 0x38
				0, // 0x39
				0, // 0x3A
				0, // 0x3B
				0, // 0x3C
				0, // 0x3D
				0, // 0x3E
				0, // 0x3F
				KEY_BACKSPACE, // 0x40
				0, // 0x41
				KEY_RIGHT_BRACE, // 0x42
				KEY_DELETE, // 0x43
				KEY_ENTER, // 0x44
				0, // 0x45
				KEY_RSHIFT, // 0x46
				KEY_RCTRL, // 0x47
				KEY_EQUAL, // 0x48
				KEY_MINUS, // 0x49
				KEY_LEFT_BRACE, // 0x4A
				KEY_BACKSLASH, // 0x4B
				KEY_QUOTE, // 0x4C
				KEY_SLASH, // 0x4D
				KEY_RGUI, // 0x4E
				KEY_RALT, // 0x4F
				KEY_0, // 0x50
				KEY_9, // 0x51
				KEY_P, // 0x52
				KEY_O, // 0x53
				KEY_SEMICOLON, // 0x54
				KEY_L, // 0x55
				KEY_PERIOD, // 0x56
				KEY_COMMA, // 0x57
				KEY_8, // 0x58
				KEY_7, // 0x59
				KEY_I, // 0x5A
				KEY_U, // 0x5B
				KEY_J, // 0x5C
				KEY_K, // 0x5D
				KEY_N, // 0x5E
				KEY_M, // 0x5F
				KEY_6, // 0x60
				KEY_5, // 0x61
				KEY_Y, // 0x62
				KEY_T, // 0x63
				KEY_H, // 0x64
				KEY_G, // 0x65
				KEY_B, // 0x66
				KEY_SPACE, // 0x67
				KEY_4, // 0x68
				KEY_3, // 0x69
				KEY_R, // 0x6A
				KEY_E, // 0x6B
				KEY_F, // 0x6C
				KEY_D, // 0x6D
				KEY_C, // 0x6E
				KEY_V, // 0x6F
				KEY_2, // 0x70
				KEY_Q, // 0x71
				KEY_W, // 0x72
				KEY_A, // 0x73
				KEY_S, // 0x74
				KEY_X, // 0x75
				KEY_Z, // 0x76
				KEY_LALT, // 0x77
				KEY_1, // 0x78
				KEY_TILDE, // 0x79
				KEY_TAB, // 0x7A
				KEY_CAPS_LOCK, // 0x7B
				KEY_LSHIFT, // 0x7C
				KEY_INTER1, // 0x7D
				KEY_LGUI, // 0x7E
				KEY_LCTRL, // 0x7F
				0, // 0x80
				0, // 0x81
				0, // 0x82
				0, // 0x83
				0, // 0x84
				0, // 0x85
				0, // 0x86
				0, // 0x87
				0, // 0x88
				0, // 0x89
				0, // 0x8A
				0, // 0x8B
				0, // 0x8C
				0, // 0x8D
				0, // 0x8E
				0, // 0x8F
				0, // 0x90
				0, // 0x91
				0, // 0x92
				0, // 0x93
				0, // 0x94
				0, // 0x95
				0, // 0x96
				0, // 0x97
				0, // 0x98
				0, // 0x99
				0, // 0x9A
				0, // 0x9B
				0, // 0x9C
				0, // 0x9D
				0, // 0x9E
				0, // 0x9F
				0, // 0xA0
				0, // 0xA1
				0, // 0xA2
				0, // 0xA3
				0, // 0xA4
				0, // 0xA5
				0, // 0xA6
				0, // 0xA7
				0, // 0xA8
				0, // 0xA9
				0, // 0xAA
				0, // 0xAB
				0, // 0xAC
				0, // 0xAD
				0, // 0xAE
				0, // 0xAF
				0, // 0xB0
				0, // 0xB1
				0, // 0xB2
				0, // 0xB3
				0, // 0xB4
				0, // 0xB5
				0, // 0xB6
				0, // 0xB7
				0, // 0xB8
				0, // 0xB9
				0, // 0xBA
				0, // 0xBB
				0, // 0xBC
				0, // 0xBD
				0, // 0xBE
				0, // 0xBF
				0, // 0xC0
				0, // 0xC1
				0, // 0xC2
				0, // 0xC3
				0, // 0xC4
				0, // 0xC5
				0, // 0xC6
				0, // 0xC7
				0, // 0xC8
				0, // 0xC9
				0, // 0xCA
				0, // 0xCB
				0, // 0xCC
				0, // 0xCD
				0, // 0xCE
				0, // 0xCF
				0, // 0xD0
				0, // 0xD1
				0, // 0xD2
				0, // 0xD3
				0, // 0xD4
				0, // 0xD5
				0, // 0xD6
				0, // 0xD7
				0, // 0xD8
				0, // 0xD9
				0, // 0xDA
				0, // 0xDB
				0, // 0xDC
				0, // 0xDD
				0, // 0xDE
				0, // 0xDF
				0, // 0xE0
				0, // 0xE1
				0, // 0xE2
				0, // 0xE3
				0, // 0xE4
				0, // 0xE5
				0, // 0xE6
				0, // 0xE7
				0, // 0xE8
				0, // 0xE9
				0, // 0xEA
				0, // 0xEB
				0, // 0xEC
				0, // 0xED
				0, // 0xEE
				0, // 0xEF
				0, // 0xF0
				0, // 0xF1
				0, // 0xF2
				0, // 0xF3
				0, // 0xF4
				0, // 0xF5
				0, // 0xF6
				0, // 0xF7
				0, // 0xF8
				0, // 0xF9
				0, // 0xFA
				0, // 0xFB
				0, // 0xFC
				0, // 0xFD
				0, // 0xFE
				0, // 0xFF
};

#endif

