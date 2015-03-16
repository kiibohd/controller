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

#ifndef __usb_keys_h
#define __usb_keys_h

// ----- Defines -----

// List of Modifiers
#define KEY_CTRL          0x01
#define KEY_SHIFT         0x02
#define KEY_ALT           0x04
#define KEY_GUI           0x08
#define KEY_LEFT_CTRL     0x01
#define KEY_LEFT_SHIFT    0x02
#define KEY_LEFT_ALT      0x04
#define KEY_LEFT_GUI      0x08
#define KEY_RIGHT_CTRL    0x10
#define KEY_RIGHT_SHIFT   0x20
#define KEY_RIGHT_ALT     0x40
#define KEY_RIGHT_GUI     0x80

// List of Keycodes
#define KEY_NOEVENT          0 // Event, not a physical key
#define KEY_ERRORROLLOVER    1 // Event, not a physical key
#define KEY_POSTFAIL         2 // Event, not a physical key
#define KEY_ERRORUNDEFINED   3 // Event, not a physical key
#define KEY_A                4
#define KEY_B                5
#define KEY_C                6
#define KEY_D                7
#define KEY_E                8
#define KEY_F                9
#define KEY_G               10
#define KEY_H               11
#define KEY_I               12
#define KEY_J               13
#define KEY_K               14
#define KEY_L               15
#define KEY_M               16
#define KEY_N               17
#define KEY_O               18
#define KEY_P               19
#define KEY_Q               20
#define KEY_R               21
#define KEY_S               22
#define KEY_T               23
#define KEY_U               24
#define KEY_V               25
#define KEY_W               26
#define KEY_X               27
#define KEY_Y               28
#define KEY_Z               29
#define KEY_1               30
#define KEY_2               31
#define KEY_3               32
#define KEY_4               33
#define KEY_5               34
#define KEY_6               35
#define KEY_7               36
#define KEY_8               37
#define KEY_9               38
#define KEY_0               39
#define KEY_ENTER           40
#define KEY_ESC             41
#define KEY_BACKSPACE       42
#define KEY_TAB             43
#define KEY_SPACE           44
#define KEY_MINUS           45
#define KEY_EQUAL           46
#define KEY_LEFT_BRACE      47
#define KEY_RIGHT_BRACE     48
#define KEY_BACKSLASH       49
#define KEY_NUMBER          50
#define KEY_SEMICOLON       51
#define KEY_QUOTE           52
#define KEY_TILDE           53
#define KEY_COMMA           54
#define KEY_PERIOD          55
#define KEY_SLASH           56
#define KEY_CAPS_LOCK       57
#define KEY_F1              58
#define KEY_F2              59
#define KEY_F3              60
#define KEY_F4              61
#define KEY_F5              62
#define KEY_F6              63
#define KEY_F7              64
#define KEY_F8              65
#define KEY_F9              66
#define KEY_F10             67
#define KEY_F11             68
#define KEY_F12             69
#define KEY_PRINTSCREEN     70
#define KEY_SCROLL_LOCK     71
#define KEY_PAUSE           72
#define KEY_INSERT          73
#define KEY_HOME            74
#define KEY_PAGE_UP         75
#define KEY_DELETE          76
#define KEY_END             77
#define KEY_PAGE_DOWN       78
#define KEY_RIGHT           79
#define KEY_LEFT            80
#define KEY_DOWN            81
#define KEY_UP              82
#define KEY_NUM_LOCK        83
#define KEYPAD_SLASH        84
#define KEYPAD_ASTERIX      85
#define KEYPAD_MINUS        86
#define KEYPAD_PLUS         87
#define KEYPAD_ENTER        88
#define KEYPAD_1            89
#define KEYPAD_2            90
#define KEYPAD_3            91
#define KEYPAD_4            92
#define KEYPAD_5            93
#define KEYPAD_6            94
#define KEYPAD_7            95
#define KEYPAD_8            96
#define KEYPAD_9            97
#define KEYPAD_0            98
#define KEYPAD_PERIOD       99
#define KEY_ISO_BACKSLASH  100
#define KEY_APP            101
#define KEYBOARD_ERROR     102 // See spec
#define KEYPAD_EQUAL       103
#define KEY_F13            104
#define KEY_F14            105
#define KEY_F15            106
#define KEY_F16            107
#define KEY_F17            108
#define KEY_F18            109
#define KEY_F19            110
#define KEY_F20            111
#define KEY_F21            112
#define KEY_F22            113
#define KEY_F23            114
#define KEY_F24            115
#define KEY_EXEC           116
#define KEY_HELP           117
#define KEY_MENU           118
#define KEY_SELECT         119
#define KEY_STOP           120
#define KEY_AGAIN          121
#define KEY_UNDO           122
#define KEY_CUT            123
#define KEY_COPY           124
#define KEY_PASTE          125
#define KEY_FIND           126
#define KEY_MUTE           127
#define KEY_VOL_UP         128
#define KEY_VOL_DOWN       129
#define KEY_CAPS_LLOCK     130 // "Locking" Scroll Lock (Old keyboards with Locking Caps Lock)
#define KEY_NUM_LLOCK      131
#define KEY_SCROLL_LLOCK   132
#define KEYPAD_COMMA       133 // Brazillian (See spec)
#define KEYPAD_EQUAL_AS    134 // AS/400 Keyboard (See spec)
#define KEY_INTER1         135 // KANJI1 - Brazillian and Japanese "Ru" and "-"
#define KEY_INTER2         136 // KANJI2 - Japanese Katakana/Hiragana
#define KEY_INTER3         137 // KANJI3 - Japanese Yen
#define KEY_INTER4         138 // KANJI4 - Japanese Henkan
#define KEY_INTER5         139 // KANJI5 - Japanese Muhenkan
#define KEY_INTER6         140 // KANJI6 - PC98 Comma (Ka-m-ma)
#define KEY_INTER7         141 // KANJI7 - Double-Byte/Single-Byte Toggle
#define KEY_INTER8         142 // KANJI8 - Undefined
#define KEY_INTER9         143 // KANJI9 - Undefined
#define KEY_LANG1          144 // Korean Hangul/English Toggle
#define KEY_LANG2          145 // Korean Hanja Conversion - Japanese Eisu
#define KEY_LANG3          146 // Japanese Katakana Key (USB)
#define KEY_LANG4          147 // Japanese Hiragana Key (USB)
#define KEY_LANG5          148 // Japanese Zenkaku/Hankaku Key (USB)
#define KEY_LANG6          149 // Reserved (Application Specific)
#define KEY_LANG7          150 // Reserved (Application Specific)
#define KEY_LANG8          151 // Reserved (Application Specific)
#define KEY_LANG9          152 // Reserved (Application Specific)
#define KEY_ALT_ERASE      153 // Special Erase (See Spec)
#define KEY_SYSREQ_ATT     154 // Modifier Type
#define KEY_CANCEL         155
#define KEY_CLEAR          156
#define KEY_PRIOR          157
#define KEY_RETURN         158
#define KEY_SEPARATOR      159
#define KEY_OUT            160
#define KEY_OPER           161
#define KEY_CLEAR_AGAIN    162
#define KEY_CRSEL_PROPS    163
#define KEY_EXSEL          164
// 165 - 175 Reserved
#define KEYPAD_00          176
#define KEYPAD_000         177
#define KEY_1000_SEP       178
#define KEY_DECIMAL_SEP    179
#define KEY_CURRENCY_MAIN  180
#define KEY_CURRENCY_SUB   181
#define KEYPAD_LPAREN      182
#define KEYPAD_RPAREN      183
#define KEYPAD_LBRACE      184
#define KEYPAD_RBRACE      185
#define KEYPAD_TAB         186
#define KEYPAD_BACKSPACE   187
#define KEYPAD_A           188
#define KEYPAD_B           189
#define KEYPAD_C           190
#define KEYPAD_D           191
#define KEYPAD_E           192
#define KEYPAD_F           193
#define KEYPAD_XOR         194
#define KEYPAD_CHEVRON     195
#define KEYPAD_PERCENT     196
#define KEYPAD_LTHAN       197
#define KEYPAD_GTHAN       198
#define KEYPAD_AND         199
#define KEYPAD_AND_AND     200
#define KEYPAD_OR          201
#define KEYPAD_OR_OR       202
#define KEYPAD_COLON       203
#define KEYPAD_POUND       204
#define KEYPAD_SPACE       205
#define KEYPAD_AT          206
#define KEYPAD_EXCLAIM     207
#define KEYPAD_MEM_STORE   208
#define KEYPAD_MEM_RECALL  209
#define KEYPAD_MEM_CLEAR   210
#define KEYPAD_MEM_ADD     211
#define KEYPAD_MEM_SUB     212
#define KEYPAD_MEM_MULT    213
#define KEYPAD_MEM_DIV     214
#define KEYPAD_PLUS_MINUS  215
#define KEYPAD_CLEAR       216
#define KEYPAD_CLEAR_ENTRY 217
#define KEYPAD_BINARY      218
#define KEYPAD_OCTAL       219
#define KEYPAD_DECIMAL     220
#define KEYPAD_HEX         221
// 222 - 223 Reserved
#define KEYS_LCTRL         224
#define KEYS_LSHIFT        225
#define KEYS_LALT          226
#define KEYS_LGUI          227
#define KEYS_RCTRL         228
#define KEYS_RSHIFT        229
#define KEYS_RALT          230
#define KEYS_RGUI          231
// 232 - 65535 Reserved

#endif

