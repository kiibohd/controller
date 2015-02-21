/* Copyright (C) 2011-2015 by Jacob Alexander
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
// http://www.usb.org/developers/hidpage/Hut1_12v2.pdf (HID Usage Tables)

// List of Keycodes - USB HID 1.12v2 pg 53
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
#define KEYPAD_ASTERISK    0x55
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
#define KEY_ISO_SLASH      0x64
#define KEY_APP            0x65
#define KEYBOARD_STATUS    0x66 // Used for indicating keyboard is on/status/errors, not a key
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



// List of LED codes - USB HID 1.12v2 pg 61
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



// List of Mouse Buttons - USB HID 1.12v2 pg 67
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



// List of System Controls - USB HID 1.12v2 pg 32
// NKRO HID Supports 0x81 - 0xB7
#define SYS_POWER_DOWN           0x81
#define SYS_SLEEP                0x82
#define SYS_WAKE_UP              0x83
#define SYS_CONTEXT_MENU         0x84
#define SYS_MAIN_MENU            0x85
#define SYS_APP_MENU             0x86
#define SYS_MENU_HELP            0x87
#define SYS_MENU_EXIT            0x88
#define SYS_MENU_SELECT          0x89
#define SYS_MENU_RIGHT           0x8A
#define SYS_MENU_LEFT            0x8B
#define SYS_MENU_UP              0x8C
#define SYS_MENU_DOWN            0x8D
#define SYS_COLD_RESTART         0x8E
#define SYS_WARM_RESTART         0x8F
#define SYS_DPAD_UP              0x90
#define SYS_DPAD_DOWN            0x91
#define SYS_DPAD_RIGHT           0x92
#define SYS_DPAD_LEFT            0x93
// 0x94 - 0x9F Reserved
#define SYS_DOCK                 0xA0
#define SYS_UNDOCK               0xA1
#define SYS_SETUP                0xA2
#define SYS_BREAK                0xA3
#define SYS_DEBUGGER_BREAK       0xA4
#define SYS_APP_BREAK            0xA5
#define SYS_APP_DEBUGGER_BREAK   0xA6
#define SYS_SPEAKER_MUTE         0xA7
#define SYS_HIBERNATE            0xA8
// 0xA9 - 0xAF Reserved
#define SYS_DISP_INVERT          0xB0
#define SYS_DISP_INTERNAL        0xB1
#define SYS_DISP_EXTERNAL        0xB2
#define SYS_DISP_BOTH            0xB3
#define SYS_DISP_DUAL            0xB4
#define SYS_DISP_TOGGLE_INT_EXT  0xB5
#define SYS_DISP_SWAP_PRI_SEC    0xB6
#define SYS_DISP_LCD_AUTOSCALE   0xB7
// 0xB8 - 0xFFFF Reserved



// List of Consumer Codes - USB HID 1.12v2
// Only listing relevant ones, let me know if you need more -HaaTa
// NKRO HID Supports 0x020 - 0x29C
#define CONSUMER_10                      0x020
#define CONSUMER_100                     0x021
#define CONSUMER_AM_PM                   0x022
// 0x023 - 0x03F Reserved
#define CONSUMER_POWER                   0x030
#define CONSUMER_RESET                   0x031
#define CONSUMER_SLEEP                   0x032
#define CONSUMER_SLEEP_AFTER             0x033
#define CONSUMER_SLEEP_MODE              0x034
#define CONSUMER_ILLUMINATION            0x035

// 0x037 - 0x03F Reserved
#define CONSUMER_MENU                    0x040
#define CONSUMER_MENU_PICK               0x041
#define CONSUMER_MENU_UP                 0x042
#define CONSUMER_MENU_DOWN               0x043
#define CONSUMER_MENU_LEFT               0x044
#define CONSUMER_MENU_RIGHT              0x045
#define CONSUMER_MENU_ESCAPE             0x046
#define CONSUMER_MENU_VALUE_INCREASE     0x047
#define CONSUMER_MENU_VALUE_DECREASE     0x048
// 0x049 - 0x05F Reserved
#define CONSUMER_DATA_ON_SCREEN          0x060
#define CONSUMER_CLOSED_CAPTION          0x061
#define CONSUMER_CLOSED_CAPTION_SELECT   0x062
#define CONSUMER_VCR_TV                  0x063
#define CONSUMER_BROADCAST_MODE          0x064
#define CONSUMER_SNAPSHOT                0x065
#define CONSUMER_STILL                   0x066
// 0x067 - 0x07F Reserved

#define CONSUMER_ASSIGN_SELECTION        0x081
#define CONSUMER_MODE_STEP               0x082
#define CONSUMER_RECALL_LAST             0x083
#define CONSUMER_ENTER_CHANNEL           0x084
#define CONSUMER_ORDER_MOVIE             0x085

#define CONSUMER_MEDIA_COMPUTER          0x088
#define CONSUMER_MEDIA_TV                0x089
#define CONSUMER_MEDIA_WWW               0x08A
#define CONSUMER_MEDIA_DVD               0x08B
#define CONSUMER_MEDIA_TELEPHONE         0x08C
#define CONSUMER_MEDIA_PROGRAM_GUIDE     0x08D
#define CONSUMER_MEDIA_VIDEO_PHONE       0x08E
#define CONSUMER_MEDIA_SELECT_GAMES      0x08F
#define CONSUMER_MEDIA_SELECT_MESSAGES   0x090
#define CONSUMER_MEDIA_SELECT_CD         0x091
#define CONSUMER_MEDIA_SELECT_VCR        0x092
#define CONSUMER_MEDIA_SELECT_TUNER      0x093
#define CONSUMER_QUIT                    0x094
#define CONSUMER_HELP                    0x095
#define CONSUMER_MEDIA_SELECT_TAPE       0x096
#define CONSUMER_MEDIA_SELECT_CABLE      0x097
#define CONSUMER_MEDIA_SELECT_SATELLITE  0x098
#define CONSUMER_MEDIA_SELECT_SECURITY   0x099
#define CONSUMER_MEDIA_SELECT_HOME       0x09A
#define CONSUMER_MEDIA_SELECT_CALL       0x09B
#define CONSUMER_CHANNEL_INCREMENT       0x09C
#define CONSUMER_CAHNNEL_DECREMENT       0x09D
#define CONSUMER_MEDIA_SELECT_SAP        0x09E
// 0x09F Reserved
#define CONSUMER_VCR_PLUS                0x0A0
#define CONSUMER_ONCE                    0x0A1
#define CONSUMER_DAILY                   0x0A2
#define CONSUMER_WEEKLY                  0x0A3
#define CONSUMER_MONTHLY                 0x0A4
// 0x0A5 - 0x0AF Reserved
#define CONSUMER_PLAY                    0x0B0
#define CONSUMER_PAUSE                   0x0B1
#define CONSUMER_RECORD                  0x0B2
#define CONSUMER_FAST_FORWARD            0x0B3
#define CONSUMER_REWIND                  0x0B4
#define CONSUMER_SCAN_NEXT_TRACK         0x0B5
#define CONSUMER_SCAN_PREVIOUS_TRACK     0x0B6
#define CONSUMER_STOP                    0x0B7
#define CONSUMER_EJECT                   0x0B8
#define CONSUMER_RANDOM_PLAY             0x0B9

#define CONSUMER_REPEAT                  0x0BC

#define CONSUMER_TRACK_NORMAL            0x0BE

#define CONSUMER_FRAME_FORWARD           0x0C0
#define CONSUMER_FRAME_BACK              0x0C1
#define CONSUMER_MARK                    0x0C2
#define CONSUMER_CLEAR_MARK              0x0C3
#define CONSUMER_REPEAT_FROM_MARK        0x0C4
#define CONSUMER_RETURN_TO_MARK          0x0C5
#define CONSUMER_SEARCH_MARK_FORWARDS    0x0C6
#define CONSUMER_SEARCH_MARK_BACKWARDS   0x0C7
#define CONSUMER_COUNTER_RESET           0x0C8
#define CONSUMER_SHOW_COUNTER            0x0C9
#define CONSUMER_TRACKING_INCREMENT      0x0CA
#define CONSUMER_TRACKING_DECREMENT      0x0CB
#define CONSUMER_STOP_EJECT              0x0CC
#define CONSUMER_PAUSE_PLAY              0x0CD
#define CONSUMER_PLAY_SKIP               0x0CE
// 0x0CF - 0x0DF Reserved

#define CONSUMER_MUTE                    0x0E2

#define CONSUMER_BASS_BOOST              0x0E5
#define CONSUMER_SURROUND_MODE           0x0E6
#define CONSUMER_LOUDNESS                0x0E7
#define CONSUMER_MPX                     0x0E8
#define CONSUMER_VOLUME_UP               0x0E9
#define CONSUMER_VOLUME_DOWN             0x0EA
// 0x0EB - 0x0EF Reserved
#define CONSUMER_SPEED_SELECT            0x0F0
#define CONSUMER_STANDARD_PLAY           0x0F2
#define CONSUMER_LONG_PLAY               0x0F3
#define CONSUMER_EXTENDED_PLAY           0x0F4
#define CONSUMER_SLOW                    0x0F5
// 0x0F6 - 0x0FF
#define CONSUMER_FAN_ENABLE              0x100

#define CONSUMER_LIGHT_ENABLE            0x102

#define CONSUMER_CLIMATE_CONTROL_ENABLE  0x104

#define CONSUMER_SECURITY_ENABLE         0x106
#define CONSUMER_FIRE_ALARM              0x107

#define CONSUMER_MOTION                  0x10A
#define CONSUMER_DURESS_ALARM            0x10B
#define CONSUMER_HOLDUP_ALARM            0x10C
#define CONSUMER_MEDICAL_ALARM           0x10D
// 0x10E - 0x14F Reserved
#define CONSUMER_BALANCE_RIGHT           0x150
#define CONSUMER_BALANCE_LEFT            0x151
#define CONSUMER_BASS_INCR               0x152
#define CONSUMER_BASS_DECR               0x153
#define CONSUMER_TREBLE_INCR             0x154
#define CONSUMER_TREBLE_DECR             0x155
// 0x156 - 0x15F Reserved

#define CONSUMER_SUB_CHANNEL_INCREMENT   0x171
#define CONSUMER_SUB_CHANNEL_DECREMENT   0x172
#define CONSUMER_ALT_AUDIO_INCREMENT     0x173
#define CONSUMER_ALT_AUDIO_DECREMENT     0x174



// List of Consumer Codes - USB HID 1.12v2
// Application Launch Buttons pg 79
#define AL_LAUNCH_BUTTON_CONFIG_TOOL      0x181
#define AL_PROGRAMMABLE_BUTTON_CONFIG     0x182
#define AL_CONSUMER_CONTROL_CONFIG        0x183
#define AL_WORD_PROCESSOR                 0x184
#define AL_TEXT_EDITOR                    0x185
#define AL_SPREADSHEET                    0x186
#define AL_GRAPHICS_EDITOR                0x187
#define AL_PRESENTATION_APP               0x188
#define AL_DATABASE_APP                   0x189
#define AL_EMAIL_READER                   0x18A
#define AL_NEWSREADER                     0x18B
#define AL_VOICEMAIL                      0x18C
#define AL_CONTACTS_ADDRESS_BOOK          0x18D
#define AL_CALENDAR_SCHEDULE              0x18E
#define AL_TASK_PROJECT_MANAGER           0x18F
#define AL_LOG_JOURNAL_TIMECARD           0x190
#define AL_CHECKBOOK_FINANCE              0x191
#define AL_CALCULATOR                     0x192
#define AL_A_V_CAPTURE_PLAYBACK           0x193
#define AL_LOCAL_MACHINE_BROWSER          0x194
#define AL_LAN_WAN_BROWSER                0x195
#define AL_INTERNET_BROWSER               0x196
#define AL_REMOTE_NETWORKING_ISP_CONNECT  0x197
#define AL_NETWORK_CONFERENCE             0x198
#define AL_NETWORK_CHAT                   0x199
#define AL_TELEPHONY_DIALER               0x19A
#define AL_LOGON                          0x19B
#define AL_LOGOFF                         0x19C
#define AL_LOGON_LOGOFF                   0x19D
#define AL_TERMINAL_LOCK_SCREENSAVER      0x19E
#define AL_CONTROL_PANEL                  0x19F
#define AL_COMMAND_LINE_PROCESSOR_RUN     0x1A0
#define AL_PROCESS_TASK_MANAGER           0x1A1
#define AL_SELECT_TAST_APP                0x1A2
#define AL_NEXT_TASK_APP                  0x1A3
#define AL_PREVIOUS_TASK_APP              0x1A4
#define AL_PREEMPTIVE_HALT_TASK_APP       0x1A5
#define AL_INTEGRATED_HELP_CENTER         0x1A6
#define AL_DOCUMENTS                      0x1A7
#define AL_THESAURUS                      0x1A8
#define AL_DICTIONARY                     0x1A9
#define AL_DESKTOP                        0x1AA
#define AL_SPELL_CHECK                    0x1AB
#define AL_GRAMMAR_CHECK                  0x1AC
#define AL_WIRELESS_STATUS                0x1AD
#define AL_KEYBOARD_LAYOUT                0x1AE
#define AL_VIRUS_PROTECTION               0x1AF
#define AL_ENCRYPTION                     0x1B0
#define AL_SCREEN_SAVER                   0x1B1
#define AL_ALARMS                         0x1B2
#define AL_CLOCK                          0x1B3
#define AL_FILE_BROWSER                   0x1B4
#define AL_POWER_STATUS                   0x1B5
#define AL_IMAGE_BROWSER                  0x1B6
#define AL_AUDIO_BROWSER                  0x1B7
#define AL_MOVIE_BROWSER                  0x1B8
#define AL_DIGITAL_RIGHTS_MANAGER         0x1B9
#define AL_DIGITAL_WALLET                 0x1BA
// 0x1BB Reserved
#define AL_INSTANT_MESSAGING              0x1BC
#define AL_OEM_FEATURES_TIPS_TUTORIAL     0x1BD
#define AL_OEM_HELP                       0x1BE
#define AL_ONLINE_COMMUNITY               0x1BF
#define AL_ENTERTAINMENT_CONTENT          0x1C0
#define AL_ONLINE_SHOPPING                0x1C1
#define AL_SMARTCARD_INFO_HELP            0x1C2
#define AL_MARKET_MONITOR                 0x1C3
#define AL_CUSTOMIZED_CORP_NEWS           0x1C4
#define AL_ONLINE_ACTIVITY                0x1C5
#define AL_SEARCH_BROWSER                 0x1C6
#define AL_AUDIO_PLAYER                   0x1C7



// List of Consumer Codes - USB HID 1.12v2
// Generic GUI Application Controls pg 82
#define AC_NEW                      0x201
#define AC_OPEN                     0x202
#define AC_CLOSE                    0x203
#define AC_EXIT                     0x204
#define AC_MAXIMIZE                 0x205
#define AC_MINIMIZE                 0x206
#define AC_SAVE                     0x207
#define AC_PRINT                    0x208
#define AC_PROPERTIES               0x209
#define AC_UNDO                     0x21A
#define AC_COPY                     0x21B
#define AC_CUT                      0x21C
#define AC_PASTE                    0x21D
#define AC_SELECT_ALL               0x21E
#define AC_FIND                     0x21F
#define AC_FIND_AND_REPLACE         0x220
#define AC_SEARCH                   0x221
#define AC_GO_TO                    0x222
#define AC_HOME                     0x223
#define AC_BACK                     0x224
#define AC_FORWARD                  0x225
#define AC_STOP                     0x226
#define AC_REFRESH                  0x227
#define AC_PREVIOUS_LINK            0x228
#define AC_NEXT_LINK                0x229
#define AC_BOOKMARKS                0x22A
#define AC_HISTORY                  0x22B
#define AC_SUBSCRIPTIONS            0x22C
#define AC_ZOOM_IN                  0x22D
#define AC_ZOOM_OUT                 0x22E
#define AC_ZOOM                     0x22F
#define AC_FULL_SCREEN_VIEW         0x230
#define AC_NORMAL_VIEW              0x231
#define AC_VIEW_TOGGLE              0x232
#define AC_SCROLL_UP                0x233
#define AC_SCROLL_DOWN              0x234
#define AC_SCROLL                   0x235
#define AC_PAN_LEFT                 0x236
#define AC_PAN_RIGHT                0x237
#define AC_PAN                      0x238
#define AC_NEW_WINDOW               0x239
#define AC_TILE_HORIZONTALLY        0x23A
#define AC_TILE_VERTICALLY          0x23B
#define AC_FORMAT                   0x23C
#define AC_EDIT                     0x23D
#define AC_BOLD                     0x23E
#define AC_ITALICS                  0x23F
#define AC_UNDERLINE                0x240
#define AC_STRIKETHROUGH            0x241
#define AC_SUBSCRIPT                0x242
#define AC_SUPERSCRIPT              0x243
#define AC_ALL_CAPS                 0x244
#define AC_ROTATE                   0x245
#define AC_RESIZE                   0x246
#define AC_FILP_HORIZONTAL          0x247
#define AC_FILP_VERTICAL            0x248
#define AC_MIRROR_HORIZONTAL        0x249
#define AC_MIRROR_VERTICAL          0x24A
#define AC_FONT_SELECT              0x24B
#define AC_FONT_COLOR               0x24C
#define AC_FONT_SIZE                0x24D
#define AC_JUSTIFY_LEFT             0x24E
#define AC_JUSTIFY_CENTER_H         0x24F
#define AC_JUSTIFY_RIGHT            0x250
#define AC_JUSTIFY_BLOCK_H          0x251
#define AC_JUSTIFY_TOP              0x252
#define AC_JUSTIFY_CENTER_V         0x253
#define AC_JUSTIFY_BOTTOM           0x254
#define AC_JUSTIFY_BLOCK_V          0x255
#define AC_INDENT_DECREASE          0x256
#define AC_INDENT_INCREASE          0x257
#define AC_NUMBERED_LIST            0x258
#define AC_RESTART_NUMBERING        0x259
#define AC_BULLETED_LIST            0x25A
#define AC_PROMOTE                  0x25B
#define AC_DEMOTE                   0x25C
#define AC_YES                      0x25D
#define AC_NO                       0x25E
#define AC_CANCEL                   0x25F
#define AC_CATALOG                  0x260
#define AC_BUY_CHECKOUT             0x261
#define AC_ADD_TO_CART              0x262
#define AC_EXPAND                   0x263
#define AC_EXPAND_ALL               0x264
#define AC_COLLAPSE                 0x265
#define AC_COLLAPSE_ALL             0x266
#define AC_PRINT_PREVIEW            0x267
#define AC_PASTE_SPECIAL            0x268
#define AC_INSERT_MODE              0x269
#define AC_DELETE                   0x26A
#define AC_LOCK                     0x26B
#define AC_UNLOCK                   0x26C
#define AC_PROTECT                  0x26D
#define AC_UNPROTECT                0x26E
#define AC_ATTACH_COMMENT           0x26F
#define AC_DELETE_COMMENT           0x270
#define AC_VIEW_COMMENT             0x271
#define AC_SELECT_WORD              0x272
#define AC_SELECT_SENTENCE          0x273
#define AC_SELECT_PARAGRAPH         0x274
#define AC_SELECT_COLUMN            0x275
#define AC_SELECT_ROW               0x276
#define AC_SELECT_TABLE             0x277
#define AC_SELECT_OBJECT            0x278
#define AC_REDO_REPEAT              0x279
#define AC_SORT                     0x27A
#define AC_SORT_ASCENDING           0x27B
#define AC_SORT_DESCENDING          0x27C
#define AC_FILTER                   0x27D
#define AC_SET_CLOCK                0x27E
#define AC_VIEW_CLOCK               0x27F
#define AC_SELECT_TIME_ZONE         0x280
#define AC_EDIT_TIME_ZONE           0x281
#define AC_SET_ALARM                0x282
#define AC_CLEAR_ALARM              0x283
#define AC_SNOOZE_ALARM             0x284
#define AC_RESET_ALARM              0x285
#define AC_SYNCHRONIZE              0x286
#define AC_SEND_RECEIVE             0x287
#define AC_SEND_TO                  0x288
#define AC_REPLY                    0x289
#define AC_REPLY_ALL                0x28A
#define AC_FORWARD_MSG              0x28B
#define AC_SEND                     0x28C
#define AC_ATTACH_FILE              0x28D
#define AC_UPLOAD                   0x28E
#define AC_DOWNLOAD                 0x28F
#define AC_SET_BORDERS              0x290
#define AC_INSERT_ROW               0x291
#define AC_INSERT_COLUMN            0x292
#define AC_INSERT_FILE              0x293
#define AC_INSERT_PICTURE           0x294
#define AC_INSERT_OBJECT            0x295
#define AC_INSERT_SYMBOL            0x296
#define AC_SAVE_AND_CLOSE           0x297
#define AC_RENAME                   0x298
#define AC_MERGE                    0x299
#define AC_SPLIT                    0x29A
#define AC_DISTRIBUTE_HORIZONTALLY  0x29B
#define AC_DISTRIBUTE_VERTICALLY    0x29C
// 0x29D-0xFFFF Reserved



#endif

