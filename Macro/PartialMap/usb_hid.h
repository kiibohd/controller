/* Copyright (C) 2011-2014 by Jacob Alexander
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

#ifndef __usb_hid_h
#define __usb_hid_h

// ----- Defines -----

// The USB codes are all taken from the USB HID Spec
// http://www.usb.org/developers/devclass_docs/Hut1_11.pdf

// List of Keycodes - USB HID 1.11 pg 53
#define KEY_NOEVENT        0x00 // Event, not a physical key
#define KEY_ERRORROLLOVER  0x01 // Event, not a physical key
#define KEY_POSTFAIL       0x02 // Event, not a physical key
#define KEY_ERRORUNDEFINED 0x03 // Event, not a physical key
#define KEY_A              0x04
#define KEY_B              0x05
#define KEY_C              0x06
#define KEY_D              0x07
#define KEY_E              0x08
#define KEY_F              0x09
#define KEY_G              0x0A
#define KEY_H              0x0B
#define KEY_I              0x0C
#define KEY_J              0x0D
#define KEY_K              0x0E
#define KEY_L              0x0F
#define KEY_M              0x10
#define KEY_N              0x11
#define KEY_O              0x12
#define KEY_P              0x13
#define KEY_Q              0x14
#define KEY_R              0x15
#define KEY_S              0x16
#define KEY_T              0x17
#define KEY_U              0x18
#define KEY_V              0x19
#define KEY_W              0x1A
#define KEY_X              0x1B
#define KEY_Y              0x1C
#define KEY_Z              0x1D
#define KEY_1              0x1E
#define KEY_2              0x1F
#define KEY_3              0x20
#define KEY_4              0x21
#define KEY_5              0x22
#define KEY_6              0x23
#define KEY_7              0x24
#define KEY_8              0x25
#define KEY_9              0x26
#define KEY_0              0x27
#define KEY_ENTER          0x28
#define KEY_ESC            0x29
#define KEY_BACKSPACE      0x2A
#define KEY_TAB            0x2B
#define KEY_SPACE          0x2C
#define KEY_MINUS          0x2D
#define KEY_EQUAL          0x2E
#define KEY_LEFT_BRACE     0x2F
#define KEY_RIGHT_BRACE    0x30
#define KEY_BACKSLASH      0x31
#define KEY_NUMBER         0x32
#define KEY_SEMICOLON      0x33
#define KEY_QUOTE          0x34
#define KEY_TILDE          0x35 // TODO Removeme (old definition)
#define KEY_BACKTICK       0x35
#define KEY_COMMA          0x36
#define KEY_PERIOD         0x37
#define KEY_SLASH          0x38
#define KEY_CAPS_LOCK      0x39
#define KEY_F1             0x3A
#define KEY_F2             0x3B
#define KEY_F3             0x3C
#define KEY_F4             0x3D
#define KEY_F5             0x3E
#define KEY_F6             0x3F
#define KEY_F7             0x40
#define KEY_F8             0x41
#define KEY_F9             0x42
#define KEY_F10            0x43
#define KEY_F11            0x44
#define KEY_F12            0x45
#define KEY_PRINTSCREEN    0x46
#define KEY_SCROLL_LOCK    0x47
#define KEY_PAUSE          0x48
#define KEY_INSERT         0x49
#define KEY_HOME           0x4A
#define KEY_PAGE_UP        0x4B
#define KEY_DELETE         0x4C
#define KEY_END            0x4D
#define KEY_PAGE_DOWN      0x4E
#define KEY_RIGHT          0x4F
#define KEY_LEFT           0x50
#define KEY_DOWN           0x51
#define KEY_UP             0x52
#define KEY_NUM_LOCK       0x53
#define KEYPAD_SLASH       0x54
#define KEYPAD_ASTERIX     0x55
#define KEYPAD_MINUS       0x56
#define KEYPAD_PLUS        0x57
#define KEYPAD_ENTER       0x58
#define KEYPAD_1           0x59
#define KEYPAD_2           0x5A
#define KEYPAD_3           0x5B
#define KEYPAD_4           0x5C
#define KEYPAD_5           0x5D
#define KEYPAD_6           0x5E
#define KEYPAD_7           0x5F
#define KEYPAD_8           0x60
#define KEYPAD_9           0x61
#define KEYPAD_0           0x62
#define KEYPAD_PERIOD      0x63
#define KEY_ISO_BACKSLASH  0x64 // TODO Removeme (old definition)
#define KEY_ISO_SLASH      0x64
#define KEY_APP            0x65
#define KEYBOARD_STATUS    0x66 // Used for indicating status or errors, not a key
#define KEYPAD_EQUAL       0x67
#define KEY_F13            0x68
#define KEY_F14            0x69
#define KEY_F15            0x6A
#define KEY_F16            0x6B
#define KEY_F17            0x6C
#define KEY_F18            0x6D
#define KEY_F19            0x6E
#define KEY_F20            0x6F
#define KEY_F21            0x70
#define KEY_F22            0x71
#define KEY_F23            0x72
#define KEY_F24            0x73
#define KEY_EXEC           0x74
#define KEY_HELP           0x75
#define KEY_MENU           0x76
#define KEY_SELECT         0x77
#define KEY_STOP           0x78
#define KEY_AGAIN          0x79
#define KEY_UNDO           0x7A
#define KEY_CUT            0x7B
#define KEY_COPY           0x7C
#define KEY_PASTE          0x7D
#define KEY_FIND           0x7E
#define KEY_MUTE           0x7F
#define KEY_VOL_UP         0x80
#define KEY_VOL_DOWN       0x81
#define KEY_CAPS_TLOCK     0x82 // Toggle "Locking" Scroll Lock (Old keyboards with Locking Caps Lock)
#define KEY_NUM_TLOCK      0x83
#define KEY_SCROLL_TLOCK   0x84
#define KEYPAD_COMMA       0x85 // Brazillian (See spec)
#define KEYPAD_EQUAL_AS    0x86 // AS/400 Keyboard (See spec)
#define KEY_INTER1         0x87 // KANJI1 - Brazillian and Japanese "Ru" and "-"
#define KEY_INTER2         0x88 // KANJI2 - Japanese Katakana/Hiragana
#define KEY_INTER3         0x89 // KANJI3 - Japanese Yen
#define KEY_INTER4         0x8A // KANJI4 - Japanese Henkan
#define KEY_INTER5         0x8B // KANJI5 - Japanese Muhenkan
#define KEY_INTER6         0x8C // KANJI6 - PC0x62 Comma (Ka-m-ma)
#define KEY_INTER7         0x8D // KANJI7 - Double-Byte/Single-Byte Toggle
#define KEY_INTER8         0x8E // KANJI8 - Undefined
#define KEY_INTER9         0x8F // KANJI9 - Undefined
#define KEY_LANG1          0x90 // Korean Hangul/English Toggle
#define KEY_LANG2          0x91 // Korean Hanja Conversion - Japanese Eisu
#define KEY_LANG3          0x92 // Japanese Katakana Key (USB)
#define KEY_LANG4          0x93 // Japanese Hiragana Key (USB)
#define KEY_LANG5          0x94 // Japanese Zenkaku/Hankaku Key (USB)
#define KEY_LANG6          0x95 // Reserved (Application Specific)
#define KEY_LANG7          0x96 // Reserved (Application Specific)
#define KEY_LANG8          0x97 // Reserved (Application Specific)
#define KEY_LANG9          0x98 // Reserved (Application Specific)
#define KEY_ALT_ERASE      0x99 // Special Erase (See Spec)
#define KEY_SYSREQ_ATT     0x9A // Modifier Type
#define KEY_CANCEL         0x9B
#define KEY_CLEAR          0x9C
#define KEY_PRIOR          0x9D
#define KEY_RETURN         0x9E
#define KEY_SEPARATOR      0x9F
#define KEY_OUT            0xA0
#define KEY_OPER           0xA1
#define KEY_CLEAR_AGAIN    0xA2
#define KEY_CRSEL_PROPS    0xA3
#define KEY_EXSEL          0xA4
// 0xA5 - 0xAF Reserved
#define KEYPAD_00          0xB0
#define KEYPAD_000         0xB1
#define KEY_1000_SEP       0xB2
#define KEY_DECIMAL_SEP    0xB3
#define KEY_CURRENCY_MAIN  0xB4
#define KEY_CURRENCY_SUB   0xB5
#define KEYPAD_LPAREN      0xB6
#define KEYPAD_RPAREN      0xB7
#define KEYPAD_LBRACE      0xB8
#define KEYPAD_RBRACE      0xB9
#define KEYPAD_TAB         0xBA
#define KEYPAD_BACKSPACE   0xBB
#define KEYPAD_A           0xBC
#define KEYPAD_B           0xBD
#define KEYPAD_C           0xBE
#define KEYPAD_D           0xBF
#define KEYPAD_E           0xC0
#define KEYPAD_F           0xC1
#define KEYPAD_XOR         0xC2
#define KEYPAD_CHEVRON     0xC3
#define KEYPAD_PERCENT     0xC4
#define KEYPAD_LTHAN       0xC5
#define KEYPAD_GTHAN       0xC6
#define KEYPAD_BITAND      0xC7
#define KEYPAD_AND         0xC8
#define KEYPAD_BITOR       0xC9
#define KEYPAD_OR          0xCA
#define KEYPAD_COLON       0xCB
#define KEYPAD_POUND       0xCC
#define KEYPAD_SPACE       0xCD
#define KEYPAD_AT          0xCE
#define KEYPAD_EXCLAIM     0xCF
#define KEYPAD_MEM_STORE   0xD0
#define KEYPAD_MEM_RECALL  0xD1
#define KEYPAD_MEM_CLEAR   0xD2
#define KEYPAD_MEM_ADD     0xD3
#define KEYPAD_MEM_SUB     0xD4
#define KEYPAD_MEM_MULT    0xD5
#define KEYPAD_MEM_DIV     0xD6
#define KEYPAD_PLUS_MINUS  0xD7
#define KEYPAD_CLEAR       0xD8
#define KEYPAD_CLEAR_ENTRY 0xD9
#define KEYPAD_BINARY      0xDA
#define KEYPAD_OCTAL       0xDB
#define KEYPAD_DECIMAL     0xDC
#define KEYPAD_HEX         0xDD
// 0xDE - 0xDF Reserved
#define KEY_CTRL           0xE0 // Convenience
#define KEY_LCTRL          0xE0
#define KEY_SHIFT          0xE1 // Convenience
#define KEY_LSHIFT         0xE1
#define KEY_ALT            0xE2 // Convenience
#define KEY_LALT           0xE2
#define KEY_GUI            0xE3 // Convenience
#define KEY_LGUI           0xE3
#define KEY_RCTRL          0xE4
#define KEY_RSHIFT         0xE5
#define KEY_RALT           0xE6
#define KEY_RGUI           0xE7
// 0xE8 - 0xFFFF Reserved
// Except for 0xE0-0xE7 which are DV (Dynamic Flags), all Keycodes are Sel (Selectors).


// List of LED codes - USB HID 1.11 pg 61
// LED/Indicators are defined as:
//  OOC - On/Off Control
//  US  - Usage Indicator: 1 - In Use, 0 - Not In Use
//  UM  - Usage Multi Mode Indicator Collection of 1 or more indicators: On, Flash, Slow Blink, Fast Blink, Off
//  Sel - Selector
//  DV  - Dynamic Flag
#define LED_UNDEFINED      0x00
#define LED_NUM_LOCK       0x01 // OOC
#define LED_CAPS_LOCK      0x02 // OOC
#define LED_SCROLL_LOCK    0x03 // OOC
#define LED_COMPOSE        0x04 // OOC
#define LED_KANA           0x05 // OOC
#define LED_POWER          0x06 // OOC
#define LED_SHIFT          0x07 // OOC
#define LED_DO_NOT_DISTURB 0x08 // OOC
#define LED_MUTE           0x09 // OOC
#define LED_TONE_ENABLE    0x0A // OOC
#define LED_HIGHCUT_FILTER 0x0B // OOC
#define LED_LOWCUT_FILTER  0x0C // OOC
#define LED_EQL_ENABLE     0x0D // OOC
#define LED_SND_FLD_ON     0x0E // OOC
#define LED_SURROUND_ON    0x0F // OOC
#define LED_REPEAT         0x10 // OOC
#define LED_STEREO         0x11 // OOC
#define LED_SAMPLE_RT_DET  0x12 // OOC
#define LED_SPINNING       0x13 // OOC
#define LED_CAV            0x14 // OOC
#define LED_CLV            0x15 // OOC
#define LED_REC_FMT_DET    0x16 // OOC
#define LED_OFF_HOOK       0x17 // OOC
#define LED_RING           0x18 // OOC
#define LED_MSG_WAITING    0x19 // OOC
#define LED_DATA_MODE      0x1A // OOC
#define LED_BAT_OPERATION  0x1B // OOC
#define LED_BAT_OK         0x1C // OOC
#define LED_BAT_LOW        0x1D // OOC
#define LED_SPEAKER        0x1E // OOC
#define LED_HEAD_SET       0x1F // OOC
#define LED_HOLD           0x20 // OOC
#define LED_MICROPHONE     0x21 // OOC
#define LED_COVERAGE       0x22 // OOC
#define LED_NIGHT_MODE     0x23 // OOC
#define LED_SEND_CALLS     0x24 // OOC
#define LED_CALL_PICKUP    0x25 // OOC
#define LED_CONFERENCE     0x26 // OOC
#define LED_STAND_BY       0x27 // OOC
#define LED_CAMERA_ON      0x28 // OOC
#define LED_CAMERA_OFF     0x29 // OOC
#define LED_ON_LINE        0x2A // OOC
#define LED_OFF_LINE       0x2B // OOC
#define LED_BUSY           0x2C // OOC
#define LED_READY          0x2D // OOC
#define LED_PAPER_OUT      0x2E // OOC
#define LED_PAPER_JAM      0x2F // OOC
#define LED_REMOTE         0x30 // OOC
#define LED_FORWARD        0x31 // OOC
#define LED_REVERSE        0x32 // OOC
#define LED_STOP           0x33 // OOC
#define LED_REWIND         0x34 // OOC
#define LED_FAST_FORWARD   0x35 // OOC
#define LED_PLAY           0x36 // OOC
#define LED_PAUSE          0x37 // OOC
#define LED_RECORD         0x38 // OOC
#define LED_ERROR          0x39 // OOC
#define LED_USI            0x3A // US
#define LED_UIUI           0x3B // US
#define LED_UMMI           0x3C // UM
#define LED_IND_ON         0x3D // Sel
#define LED_IND_FLASH      0x3E // Sel
#define LED_IND_SLOW_BLNK  0x3F // Sel
#define LED_IND_FAST_BLNK  0x40 // Sel
#define LED_IND_OFF        0x41 // Sel
#define LED_FLASH_ON_TIME  0x42 // DV
#define LED_SLW_B_ON_TIME  0x43 // DV
#define LED_SLW_B_OFF_TIME 0x44 // DV
#define LED_FST_B_ON_TIME  0x45 // DV
#define LED_FST_B_OFF_TIME 0x46 // DV
#define LED_UIC            0x47 // UM
#define LED_IND_RED        0x48 // Sel
#define LED_IND_GREEN      0x49 // Sel
#define LED_IND_AMBER      0x4A // Sel
#define LED_GENERIC_IND    0x4B // OOC
#define LED_SYS_SUSPEND    0x4C // OOC
#define LED_EXT_PWR_CONN   0x4D // OOC
// 0x4E - 0xFFFF Reserved


// List of Mouse Buttons - USB HID 1.11 pg 67
#define MOUSE_NOPRESS      0x00
#define MOUSE_PRIMARY      0x01 // Button 1
#define MOUSE_SECONDARY    0x02 // Button 2
#define MOUSE_TERTIARY     0x03 // Button 3
#define MOUSE_BUTTON(x)       x
// Continues to 0xFFFF, the higher the Mouse code, the selector significance descreases
// Buttons can be defined as:
//  Sel - Selector
//  OOC - On/Off Control
//  MC  - Momentary Control
//  OSC - One-Shot Control
// depending on context.



// List of Consumer Codes - USB HID 1.11
// Only listing used ones, let me know if you need more -HaaTa
#define CONSUMER_SCAN_NEXT_TRACK                   0x0B5
#define CONSUMER_SCAN_PREVIOUS_TRACK               0x0B6
#define CONSUMER_STOP                              0x0B7
#define CONSUMER_EJECT                             0x0B8

#define CONSUMER_PAUSE_PLAY                        0x0CD

#define CONSUMER_MUTE                              0x0E2

#define CONSUMER_BASS_BOOST                        0x0E5

#define CONSUMER_LOUDNESS                          0x0E7

#define CONSUMER_VOLUME_UP                         0x0E9
#define CONSUMER_VOLUME_DOWN                       0x0EA

#define CONSUMER_BASS_INCR                         0x152
#define CONSUMER_BASS_DECR                         0x153
#define CONSUMER_TREBLE_INCR                       0x154
#define CONSUMER_TREBLE_DECR                       0x155

#define CONSUMER_AL_LAUNCH_BUTTON_CONFIG_TOOL      0x181
#define CONSUMER_AL_PROGRAMMABLE_BUTTON_CONFIG     0x182
#define CONSUMER_AL_CONSUMER_CONTROL_CONFIG        0x183
#define CONSUMER_AL_WORD_PROCESSOR                 0x184
#define CONSUMER_AL_TEXT_EDITOR                    0x185
#define CONSUMER_AL_SPREADSHEET                    0x186
#define CONSUMER_AL_GRAPHICS_EDITOR                0x187
#define CONSUMER_AL_PRESENTATION_APP               0x188
#define CONSUMER_AL_DATABASE_APP                   0x189
#define CONSUMER_AL_EMAIL_READER                   0x18A
#define CONSUMER_AL_NEWSREADER                     0x18B
#define CONSUMER_AL_VOICEMAIL                      0x18C
#define CONSUMER_AL_CONTACTS_ADDRESS_BOOK          0x18D
#define CONSUMER_AL_CALENDAR_SCHEDULE              0x18E
#define CONSUMER_AL_TASK_PROJECT_MANAGER           0x18F
#define CONSUMER_AL_LOG_JOURNAL_TIMECARD           0x190
#define CONSUMER_AL_CHECKBOOK_FINANCE              0x191
#define CONSUMER_AL_CALCULATOR                     0x192
#define CONSUMER_AL_A_V_CAPTURE_PLAYBACK           0x193
#define CONSUMER_AL_LOCAL_MACHINE_BROWSER          0x194
#define CONSUMER_AL_LAN_WAN_BROWSER                0x195
#define CONSUMER_AL_INTERNET_BROWSER               0x196
#define CONSUMER_AL_REMOTE_NETWORKING_ISP_CONNECT  0x197
#define CONSUMER_AL_NETWORK_CONFERENCE             0x198
#define CONSUMER_AL_NETWORK_CHAT                   0x199
#define CONSUMER_AL_TELEPHONY_DIALER               0x19A
#define CONSUMER_AL_LOGON                          0x19B
#define CONSUMER_AL_LOGOFF                         0x19C
#define CONSUMER_AL_LOGON_LOGOFF                   0x19D
#define CONSUMER_AL_TERMINAL_LOCK_SCREENSAVER      0x19E
#define CONSUMER_AL_CONTROL_PANEL                  0x19F
#define CONSUMER_AL_COMMAND_LINE_PROCESSOR_RUN     0x1A0
#define CONSUMER_AL_PROCESS_TASK_MANAGER           0x1A1
#define CONSUMER_AL_SELECT_TAST_APP                0x1A2
#define CONSUMER_AL_NEXT_TASK_APP                  0x1A3
#define CONSUMER_AL_PREVIOUS_TASK_APP              0x1A4
#define CONSUMER_AL_PREEMPTIVE_HALT_TASK_APP       0x1A5

#define CONSUMER_AC_NEW                            0x201
#define CONSUMER_AC_OPEN                           0x202
#define CONSUMER_AC_CLOSE                          0x203
#define CONSUMER_AC_EXIT                           0x204
#define CONSUMER_AC_MAXIMIZE                       0x205
#define CONSUMER_AC_MINIMIZE                       0x206
#define CONSUMER_AC_SAVE                           0x207
#define CONSUMER_AC_PRINT                          0x208
#define CONSUMER_AC_PROPERTIES                     0x209
#define CONSUMER_AC_UNDO                           0x21A
#define CONSUMER_AC_COPY                           0x21B
#define CONSUMER_AC_CUT                            0x21C
#define CONSUMER_AC_PASTE                          0x21D
#define CONSUMER_AC_SELECT_ALL                     0x21E
#define CONSUMER_AC_FIND                           0x21F
#define CONSUMER_AC_FIND_AND_REPLACE               0x220
#define CONSUMER_AC_SEARCH                         0x221
#define CONSUMER_AC_GO_TO                          0x222
#define CONSUMER_AC_HOME                           0x223
#define CONSUMER_AC_BACK                           0x224
#define CONSUMER_AC_FORWARD                        0x225
#define CONSUMER_AC_STOP                           0x226
#define CONSUMER_AC_REFRESH                        0x227
#define CONSUMER_AC_PREVIOUS_LINK                  0x228
#define CONSUMER_AC_NEXT_LINK                      0x229
#define CONSUMER_AC_BOOKMARKS                      0x22A
#define CONSUMER_AC_HISTORY                        0x22B
#define CONSUMER_AC_SUBSCRIPTIONS                  0x22C
#define CONSUMER_AC_ZOOM_IN                        0x22D
#define CONSUMER_AC_ZOOM_OUT                       0x22E
#define CONSUMER_AC_ZOOM                           0x22F
#define CONSUMER_AC_FULL_SCREEN_VIEW               0x230
#define CONSUMER_AC_NORMAL_VIEW                    0x231
#define CONSUMER_AC_VIEW_TOGGLE                    0x232
#define CONSUMER_AC_SCROLL_UP                      0x233
#define CONSUMER_AC_SCROLL_DOWN                    0x234
#define CONSUMER_AC_SCROLL                         0x235
#define CONSUMER_AC_PAN_LEFT                       0x236
#define CONSUMER_AC_PAN_RIGHT                      0x237
#define CONSUMER_AC_PAN                            0x238
#define CONSUMER_AC_NEW_WINDOW                     0x239
#define CONSUMER_AC_TILE_HORIZONTALLY              0x23A
#define CONSUMER_AC_TILE_VERTICALLY                0x23B
#define CONSUMER_AC_FORMAT                         0x23C



#endif

