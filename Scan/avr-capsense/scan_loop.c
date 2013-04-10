/* Keyboard example with debug channel, for Teensy USB Development Board
 * http://www.pjrc.com/teensy/usb_keyboard.html
 * Copyright (c) 2008 PJRC.COM, LLC
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

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usb_keyboard_debug.h"
#include "print.h"

#define LED_CONFIG	(DDRD |= (1<<6))
#define LED_ON		(PORTD &= ~(1<<6))
#define LED_OFF		(PORTD |= (1<<6))
#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))

#define THRESHOLD 0x0a
#define BUMP_THRESHOLD 0x50
//((THRESHOLD) * 3)
#define BUMP_REST_US 1200

#define HYST 1
#define HYST_T 0x10

#define TEST_KEY_STROBE (0x05)
#define TEST_KEY_MASK (1 << 0)

#define ADHSM 7

/** Whether to use all of D and C, vs using E0, E1 instead of D6, D7,
 * or alternately all of D, and E0,E1 and C0,..5 */
//#define ALL_D_C
//#define SHORT_D
#define SHORT_C

// rough offset voltage: one diode drop, about 50mV = 0x3ff * 50/3560 = 20
//#define OFFSET_VOLTAGE 0x14
#define OFFSET_VOLTAGE 0x28

volatile uint8_t idle_count=1;

uint8_t blink=0;

volatile uint16_t full_av = 0;

#define RIGHT_JUSTIFY 0
#define LEFT_JUSTIFY (0xff)

// set left or right justification here:
#define JUSTIFY_ADC RIGHT_JUSTIFY

#define ADLAR_MASK (1 << ADLAR)
#ifdef JUSTIFY_ADC
#define ADLAR_BITS ((ADLAR_MASK) & (JUSTIFY_ADC))
#else // defaults to right justification.
#define ADLAR_BITS 0
#endif


// full muxmask
#define FULL_MUX_MASK ((1 << MUX0) | (1 << MUX1) | (1 << MUX2) | (1 << MUX3) | (1 << MUX4))

// F0-f7 pins only muxmask.
#define MUX_MASK ((1 << MUX0) | (1 << MUX1) | (1 << MUX2))

#define SET_MUX(X) ((ADMUX) = (((ADMUX) & ~(MUX_MASK)) | ((X) & (MUX_MASK))))
#define SET_FULL_MUX(X) ((ADMUX) = (((ADMUX) & ~(FULL_MUX_MASK)) | ((X) & (FULL_MUX_MASK))))

#define MUX_1_1 0x1e
#define MUX_GND 0x1f


	// set ADC clock prescale
#define PRESCALE_MASK ((1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2))
#define PRESCALE_SHIFT (ADPS0)
#define PRESCALE 3


/**/ uint8_t ze_strober = 0;

#ifdef EXTENDED_STROBE

#define STROBE_LINES 18

#else

#define STROBE_LINES 16

#endif

#define STROBE_LINES_XSHIFT 4
#define STROBE_LINES_MASK 0x0f
#define MUXES_COUNT 8
#define MUXES_COUNT_XSHIFT 3
#define MUXES_MASK 0x7

#define WARMUP_LOOPS ( 1024 )

#define RECOVERY_US 6

#define SAMPLES 10
int16_t samples [SAMPLES];

//int16_t gsamples [SAMPLES];

#define SAMPLE_OFFSET ((SAMPLES) - MUXES_COUNT)
//#define SAMPLE_OFFSET 9
#define STROBE_OFFSET 0

/**/ int16_t adc_mux_averages[MUXES_COUNT];
/**/ int16_t adc_strobe_averages[STROBE_LINES];


/**/ uint8_t cur_keymap[STROBE_LINES];
// /**/ int8_t last_keymap[STROBE_LINES];
/**/ uint8_t usb_keymap[STROBE_LINES];
uint8_t dirty;
uint8_t unstable;
uint8_t usb_dirty;

int16_t threshold = THRESHOLD;
uint16_t tests = 0;

uint8_t col_a=0;
uint8_t col_b=0;
uint8_t col_c=0;

uint8_t column=0;

#define KEY_COUNT ((STROBE_LINES) * (MUXES_COUNT))

int16_t keys_averages_acc[KEY_COUNT];
uint16_t keys_averages[KEY_COUNT];

uint8_t full_samples[KEY_COUNT];

/* viable starting biases for near 0.830V offset. and adc PRESCALE 3
0017 0016 001B 001A 0016 0016 000F 000E 001B 001E 001E 0018 0017 0015 000E 001D
001B 001A 0016 0016 000F 000E 001C 001B 001E 0018 0017 0015 000E 001D 0024 001F
0016 0016 000F 000E 001C 001B 001E 001E 0017 0015 000E 001D 0024 001F 0020 001F
000F 000E 001C 001B 001E 001E 0018 0017 000E 001D 0024 001F 0020 001F 0020 0017
001C 001B 001E 001E 0018 0017 0015 000E 0024 001F 0020 001F 0020 0017 0010 001D
001E 001E 0018 0017 0015 000E 001D 0024 0020 001F 0020 0017 0010 001D 0024 0021
0018 0017 0015 000E 001D 0024 001F 0020 0020 0017 0010 001D 0024 0021 0021 0021
0015 000E 001D 0024 001F 0020 001F 0020 0010 001D 0024 0021 0021 0021 0021 0018
*/

/*** starting bias relative to fixed offset estimate of 820mV (0x50)
 *   77 69 65 5B 50 4E 4C 45   66 53 4D 49 45 3F 3E 35
 *   68 54 4F 49 45 40 3F 34   74 66 5F 56 4E 4D 4C 3F
 *   6D 5D 53 4C 49 46 45 38   6D 5A 53 4E 49 48 45 3E
 *   6F 5D 56 4E 4B 48 48 3A   6D 5C 54 4E 48 48 45 37
 *   75 68 5F 57 4F 4D 4C 3F   60 4E 48 41 3C 3C 39 2F
 *   65 53 4E 49 41 3F 3E 34   65 54 4E 49 43 3F 3E 34
 *   60 51 4A 45 3F 3E 3C 30   57 4C 45 3E 3B 37 37 2E
 *   64 4E 48 44 3C 3B 39 2F   5D 4F 48 45 3E 3C 3B 30
 */

/*volatile*/ uint16_t count = 0;



/*volatile*/ uint8_t error = 0;
uint16_t error_data = 0;

void dump(void);
void dumpkeys(void);

static const uint8_t PROGMEM matrix122F_to_set3[] = {
0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x84, // (), npenter, np3, (), np+, np9, np*, np-
0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, // np0, np., np2, np5, np6, np8, numlck, np/
0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, //
0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
0x00, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, // 0x50 vanishes - is test key.
0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, // 0x48 vanishes - else roll back.
0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40,
0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
0x01, 0x83, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 0x02 is replaced with 0x83.
};

#define LX2FX

static const uint8_t PROGMEM page3_2_USB[133] = {
0x0,		// 00   00  // no key.

#ifndef LX2FX

0xe3,		// 01       Enl   Help -> windows. (e3)
0x0,		// 02		// no key.
0x80,	// 03   A4  ExSel SetUp print, -> paste 7d -> vol up 80
0x46,	// 04   A3  CrSel       Properties -> copy 7c -> mute 7f-> prt-scrn
0x29,	// 05   9A  Attn  SysRq // good place fer escape. (29)
0x47,		// 06   9C  Clear -> kp / 54 -> scroll-lock (47)
0x3a,		// 07   3A  F1
0x68,		// 08   68  F13
0x65,		// 09       Ctrl	// record/pause // -> kb application (65)
0x74,		// 0A       Copy  Test // play/test -> execute 74 -> kp + 57
0x75,		// 0B      // no key. help (75) -> kp - 56
0x48,		// 0C       Pause ErInp erase input. -> cut 7b -> kp * 55 -> pause 48

#else

/*
0x42,		// 01       L9
0x0,		// 02		// no key.
0x3e,		// 03		L5
0x3c,		// 04		L3
0x3a,		// 05      	L1
0x3b,		// 06      	L2
0x3a,		// 07   3A  F1
0x68,		// 08   68  F13
0x43,		// 09      	L10
0x41,		// 0A      	L8
0x3f,		// 0B      	L6
0x3d,		// 0C      	L4
*/
0x61,		// 01       L9 -> num 9  0x61
0x0,		// 02		// no key.
0x5d,		// 03		L5 -> num 5  0x5d
0x5b,		// 04		L3 -> num 3  0x5b
0x59,		// 05      	L1 -> num 1  0x59
0x5a,		// 06      	L2 -> num 2  0x5a
0x3a,		// 07   3A  F1
0x68,		// 08   68  F13
0x62,		// 09      	L10 -> num 0 0x62
0x60,		// 0A      	L8 -> num 8 0x60
0x5e,		// 0B      	L6 -> num 6 0x5e
0x5c,		// 0C      	L4 -> num 4 0x5c
#endif


0x2b,		// 0D   2B  Tab
0x29,		// 0E   35  ~ ` -> escape 29
0x3b,		// 0F   3B  F2
0x69,		// 10   69  F14
0xe0,		// 11   E0  Ctrl L
0xe1,		// 12   E1  Shift L
0xe1,		// 13   64  left of z. -> l shift e1
0x39,		// 14   39  Caps Lock
0x14,		// 15   14  Q
0x1e,		// 16   1E  ! 1
0x3c,		// 17   3C  F3
0x6a,		// 18   6A  F15
0xe2,		// 19   E2  Alt L
0x1d,		// 1A   1D  Z
0x16,		// 1B   16  S
0x04,		// 1C   04  A
0x1a,		// 1D   1A  W
0x1f,		// 1E   1F  @ 2
0x3d,		// 1F   3D  F4
0x6b,		// 20   6B  F16
0x06,		// 21   06  C
0x1b,		// 22   1B  X
0x07,		// 23   07  D
0x08,		// 24   08  E
0x21,		// 25   21  $ 4
0x20,		// 26   20  # 3
0x3e,		// 27   3E  F5
0x6c,		// 28   6C  F17
0x2c,		// 29   2C  Space
0x19,		// 2A   19  V
0x09,		// 2B   09  F
0x17,		// 2C   17  T
0x15,		// 2D   15  R
0x22,		// 2E   22  % 5
0x3f,		// 2F   3F  F6
0x6d,		// 30   6D  F18
0x11,		// 31   11  N
0x05,		// 32   05  B
0x0b,		// 33   0B  H
0x0a,		// 34   0A  G
0x1c,		// 35   1C  Y
0x23,		// 36   23  ^ 6
0x40,		// 37   40  F7
0x6e,		// 38   6E  F19
0xe6,		// 39   E6  Alt R
0x10,		// 3A   10  M
0x0d,		// 3B   0D  J
0x18,		// 3C   18  U
0x24,		// 3D   24  & 7
0x25,		// 3E   25  * 8
0x41,		// 3F   41  F8
0x6f,		// 40   6F  F20
0x36,		// 41   36  < ,
0x0e,		// 42   0E  K
0x0c,		// 43   0C  I
0x12,		// 44   12  O
0x27,		// 45   27  ) 0
0x26,		// 46   26  ( 9
0x42,		// 47   42  F9
0x70,		// 48   70  F21
0x37,		// 49   37  > .
0x38,		// 4A   38  ? /
0x0f,		// 4B   0F  L
0x33,		// 4C   33  : ;
0x13,		// 4D   13  P
0x2d,		// 4E   2D  _ -
0x43,		// 4F   43  F10
0x71,		// 50   71  F22
0xe5,		// 51   87  likely a shift - e.g. kp shift -> e5
0x34,		// 52   34  " '
0x31,		// 53      (INT 2) -> keypad enter. 58 -> |/\ (31)
0x2f,		// 54   2F  { [
0x2e,		// 55   2E  + =
0x44,		// 56   44  F11
0x72,		// 57   72  F23  -> vol up.
0xe4,		// 58   E4  Ctrl R
0xe5,		// 59   E5  Shift R
0x28,		// 5A   28  Enter
0x30,		// 5B   30  } ]
0x31,		// 5C   31  | '\'
0x35,		// 5D   -> kp = 67 -> ~` 35
0x45,		// 5E   45  F12
0x73,		// 5F   73  F24 -> vol down.
0x51,		// 60   51  Down CP
0x50,		// 61   50  Left CP
0x51, //0x0,// 62       Rule // centre cp.  //62   48  Pause/Bk
0x52,		// 63   52  Up CP
0x4c,		// 64   4C  Del CP
0x4d,		// 65   4D  End CP
0x2a,		// 66   2A  Back Space
0x49,		// 67   49  Ins CP
0x48,		// 68		// under kp0 => kp 0 (62) 48 -> pause (48)
0x59,		// 69   59  1 End KP
0x4f,		// 6A   4F  Right CP
0x5c,		// 6B   5C  4 Left KP
0x5f,		// 6C   5F  7 Home KP
0x4e,		// 6D   4E  PgDn CP
0x4a,		// 6E   4A  Home CP
0x4b,		// 6F   4B  PgUp CP
0x62,		// 70   62  0 Ins KP -> pause (+48) -> kp-0 (62)
0x63,		// 71   63  . Del KP
0x5a,		// 72   5A  2 Down KP
0x5d,		// 73   97  5 KP
0x5e,		// 74   5E  6 Right KP
0x60,		// 75   60  8 Up KP
0x53,		// 76   53  Num Lock
0x54,		// 77   54  / KP
0x47,		// 78       Undo // under enter -> scroll-lock 47
0x58,		// 79   58  Enter KP ->  enter kp
0x5b,		// 7A   5B  3 PgDn KP
0x46,		// 7B       (INT 5) // under + -> print screen (46
0x57,		// 7C   57  + KP
0x61,		// 7D   61  9 PgUp KP
0x55,		// 7E   55  * KP
0x0,		// 7F			no such key?
0x0,		// 80           no such key
0x0,		// 81              Paste?
0x0,		// 82              Find?
#ifndef LX2FX
0x81,		// 83   Print Ident -> undo 7a -> vol down 81
#else
//0x40,		// 83   L7 (f1) 3a
0x5f,		// 83   L7 (f1) 3a -> num 7 0x5f
#endif
0x56		// 84   56  - KP
};

void
_delay_loop(uint8_t __count)
{
	__asm__ volatile (
		"1: dec %0" "\n\t"
		"brne 1b"
		: "=r" (__count)
		: "0" (__count)
	);
}


void setup_ADC (void) {
	// disable adc digital pins.
	DIDR1 |= (1 << AIN0D) | (1<<AIN1D); // set disable on pins 1,0.
	//DIDR0 = 0xff; // disable all. (port F, usually). - testing w/o disable.
	DDRF = 0x0;
	PORTF = 0x0;
	uint8_t mux = 0 & 0x1f; // 0 == first. // 0x1e = 1.1V ref.

	// 0 = external aref 1,1 = 2.56V internal ref
	uint8_t aref = ((1 << REFS1) | (1 << REFS0)) & ((1 << REFS1) | (1 << REFS0));
//	uint8_t adlar = 0xff & (1 << ADLAR); // 1 := left justify bits, 0 := right
	uint8_t adate = (1 << ADATE) & (1 << ADATE); // trigger enable
	uint8_t trig = 0 & ((1 << ADTS0) | (1 << ADTS1) | (1 << ADTS2)); // 0 = free running
	// ps2, ps1 := /64 ( 2^6 ) ps2 := /16 (2^4), ps1 := 4, ps0 :=2, PS1,PS0 := 8 (2^8)
	uint8_t prescale = ( ((PRESCALE) << PRESCALE_SHIFT) & PRESCALE_MASK ); // 001 == 2^1 == 2
	uint8_t hispeed = (1 << ADHSM);
	uint8_t en_mux = (1 << ACME);

	//ADCSRA = (ADCSRA & ~PRESCALES) | ((1 << ADPS1) | (1 << ADPS2)); // 2, 1 := /64 ( 2^6 )
	//ADCSRA = (ADCSRA & ~PRESCALES) | ((1 << ADPS0) | (1 << ADPS2)); // 2, 0 := /32 ( 2^5 )
	//ADCSRA = (ADCSRA & ~PRESCALES) | ((1 << ADPS2)); // 2 := /16 ( 2^4 )

	ADCSRA = (1 << ADEN) | prescale; // ADC enable

	// select ref.
	//ADMUX |= ((1 << REFS1) | (1 << REFS0)); // 2.56 V internal.
	//ADMUX |= ((1 << REFS0) ); // Vcc with external cap.
	//ADMUX &= ~((1 << REFS1) | (1 << REFS0)); // 0,0 : aref.
	ADMUX = aref | mux | ADLAR_BITS;

	// enable MUX
	// ADCSRB |= (1 << ACME); 	// enable
	// ADCSRB &= ~(1 << ADEN); // ?

	// select first mux.
	//ADMUX = (ADMUX & ~MUXES); // start at 000 = ADC0

	// clear adlar to left justify data
	//ADMUX = ~();

	// set adlar to right justify data
	//ADMUX |= (1 << ADLAR);


	// set free-running
	ADCSRA |= adate; // trigger enable
	ADCSRB  = en_mux | hispeed | trig | (ADCSRB & ~((1 << ADTS0) | (1 << ADTS1) | (1 << ADTS2))); // trigger select free running

//	ADCSRA |= (1 << ADATE); // tiggger enable

	ADCSRA |= (1 << ADEN); // ADC enable
	ADCSRA |= (1 << ADSC); // start conversions q

}


#define RECOVERY_CONTROL 1

#define RECOVERY_SOURCE 0
#define RECOVERY_SINK 2
#define RECOVERY_MASK 0x03

void recovery(uint8_t on) {
	DDRB |= (1 << RECOVERY_CONTROL);

	PORTB &= ~(1 << RECOVERY_SINK);    // SINK always zero
	DDRB &= ~(1 << RECOVERY_SOURCE);  // SOURCE high imp

	if(on) {
		DDRB |= (1 << RECOVERY_SINK);	// SINK pull


		PORTB |= (1 << RECOVERY_CONTROL);

		PORTB |= (1 << RECOVERY_SOURCE); // SOURCE high
		DDRB |= (1 << RECOVERY_SOURCE);
	} else {
		_delay_loop(10);
		PORTB &= ~(1 << RECOVERY_CONTROL);

		DDRB &= ~(1 << RECOVERY_SOURCE);
		PORTB &= ~(1 << RECOVERY_SOURCE); // SOURCE low
		DDRB &= ~(1 << RECOVERY_SINK);	// SINK high-imp

		//DDRB &= ~(1 << RECOVERY_SINK);
	}
}

void strobe_w(uint8_t strobe_num) {

#ifdef ALL_D_C

#define D_MASK (0xff)
#define D_SHIFT 0

#define E_MASK (0x00)
#define E_SHIFT 0

#define C_MASK (0xff)
#define C_SHIFT 8

#else
#if defined(SHORT_D)

#define D_MASK (0x3f)
#define D_SHIFT 0

#define E_MASK (0x03)
#define E_SHIFT 6

#define C_MASK (0xff)
#define C_SHIFT 8

#else
#if defined(SHORT_C)

#define D_MASK (0xff)
#define D_SHIFT 0

#define E_MASK (0x03)
#define E_SHIFT 6

#define C_MASK (0xff)
#define C_SHIFT 8
#endif
#endif
#endif



#define STROBE_CASE(SC_CASE, SC_REG_A) case (SC_CASE): PORT##SC_REG_A = \
	(( (PORT##SC_REG_A) & ~(1 << (SC_CASE - SC_REG_A##_SHIFT)) ) | (1 << (SC_CASE - SC_REG_A##_SHIFT)))

	PORTC &= ~(D_MASK);
	PORTD &= ~(D_MASK);
	PORTE &= ~(E_MASK);

#ifdef SHORT_C
	strobe_num = 15 - strobe_num;
#endif

	switch(strobe_num) {

	case 0: PORTD |= (1 << 0); break;
	case 1: PORTD |= (1 << 1); break;
	case 2: PORTD |= (1 << 2); break;
	case 3: PORTD |= (1 << 3); break;
	case 4: PORTD |= (1 << 4); break;
	case 5: PORTD |= (1 << 5); break;

#ifdef ALL_D

	case 6: PORTD |= (1 << 6); break;
	case 7: PORTD |= (1 << 7); break;

	case 8:  PORTC |= (1 << 0); break;
	case 9:  PORTC |= (1 << 1); break;
	case 10: PORTC |= (1 << 2); break;
	case 11: PORTC |= (1 << 3); break;
	case 12: PORTC |= (1 << 4); break;
	case 13: PORTC |= (1 << 5); break;
	case 14: PORTC |= (1 << 6); break;
	case 15: PORTC |= (1 << 7); break;

	case 16: PORTE |= (1 << 0); break;
	case 17: PORTE |= (1 << 1); break;

#else
#ifdef SHORT_D

	case 6: PORTE |= (1 << 0); break;
	case 7: PORTE |= (1 << 1); break;

	case 8:  PORTC |= (1 << 0); break;
	case 9:  PORTC |= (1 << 1); break;
	case 10: PORTC |= (1 << 2); break;
	case 11: PORTC |= (1 << 3); break;
	case 12: PORTC |= (1 << 4); break;
	case 13: PORTC |= (1 << 5); break;
	case 14: PORTC |= (1 << 6); break;
	case 15: PORTC |= (1 << 7); break;

#else
#ifdef SHORT_C

	case 6: PORTD |= (1 << 6); break;
	case 7: PORTD |= (1 << 7); break;

	case 8: PORTE |= (1 << 0); break;
	case 9: PORTE |= (1 << 1); break;

	case 10:  PORTC |= (1 << 0); break;
	case 11:  PORTC |= (1 << 1); break;
	case 12: PORTC |= (1 << 2); break;
	case 13: PORTC |= (1 << 3); break;
	case 14: PORTC |= (1 << 4); break;
	case 15: PORTC |= (1 << 5); break;

	case 16: PORTC |= (1 << 6); break;
	case 17: PORTC |= (1 << 7); break;

#endif
#endif
#endif

	default:
		break;
	}

}


int sampleColumn_i(uint8_t column, uint8_t muxes, int16_t * buffer) {

	// ensure all probe lines are driven low, and chill for recovery delay.
	PORTC &= ~C_MASK;
	PORTD &= ~D_MASK;
	PORTE &= ~E_MASK;
	recovery(1);
	_delay_us(RECOVERY_US);
	recovery(0);

	uint8_t index = 0;

	for (uint8_t i=0; i<8; ++i) {
		if(muxes & (1 << i)) {
			buffer[index++] = i;
		}
	}

	SET_FULL_MUX(MUX_1_1); // crap sample will use this.
	ADCSRA |= (1 << ADEN) | (1 << ADSC); // enable and start conversions
	ADCSRA |= (1 << ADIF); // clear int flag by writing 1.

	uint16_t sample;

	while (! (ADCSRA & (1 << ADIF))); // wait until ready.
	sample = ADC; // 1st sample, icky.

	strobe_w(column);
	//recovery(0);

	/**
	 * we are running in continuous mode, so we must setup the next
	 * read _before_ the current read completes.
	 *
	 * setup 0,
	 * read garbage,
	 * do not store
	 *
	 * setup 1,
	 * read 0,
	 * store 0,
	 *
	 * ...
	 *
	 * setup junk,
	 * read n
	 * store n
	 *
	 * */


	ADCSRA |= (1 << ADIF); // clear int flag by writing 1.
	//wait for last read to complete.
	while (! (ADCSRA & (1 << ADIF)));
	sample = ADC; // throw away strobe'd value.

#if 0
	for (uint8_t i=0; i <= index; ++i) {

		// setup i'th read.
		SET_FULL_MUX(buffer[i]); // _next_ read will use this.
		// wait for i-1'th read to complete:
		ADCSRA |= (1 << ADIF); // clear int flag by writing 1.
		while (! (ADCSRA & (1 << ADIF)));

		// retrieve last (i-1'th) read.
		if (i) {
			buffer[i-1] = ADC - OFFSET_VOLTAGE;
		} /*else {
			buffer[0] = ADC - OFFSET_VOLTAGE;
		}*/

		//index++;
	}
#else
	for (uint8_t i=0; i < index; ++i) {

		// setup i'th read.
		SET_FULL_MUX(buffer[i]); // _next_ read will use this.

		ADCSRA |= (1 << ADIF); // clear int flag by writing 1.
		while (! (ADCSRA & (1 << ADIF)));
		sample = ADC; // throw away warmup value.



		/*
		ADCSRA |= (1 << ADIF); // clear int flag by writing 1.
		while (! (ADCSRA & (1 << ADIF)));
		sample = ADC; // throw away warmup value.
*/

		ADCSRA |= (1 << ADIF); // clear int flag by writing 1.
		while (! (ADCSRA & (1 << ADIF)));

		// retrieve current read.
		buffer[i] = ADC - OFFSET_VOLTAGE;


	}
#endif


	// turn off adc.
	ADCSRA &= ~(1 << ADEN);

	// pull all columns' probe-lines low.
	PORTC &= ~C_MASK;
	PORTD &= ~D_MASK;
	PORTE &= ~E_MASK;

	// test for humps. :/
	/*uint16_t delta = full_av;
	if(buffer[0] > BUMP_THRESHOLD + delta) {
		// ze horror.
		return 1;
	} else {
		return 0; //all good.
	}*/
	return 0;

}

int sampleColumn_k(uint8_t column, int16_t * buffer) {
	// ensure all probe lines are driven low, and chill for recovery delay.
	uint16_t sample;

	ADCSRA |= (1 << ADEN) | (1 << ADSC); // enable and start conversions
	ADCSRA |= (1 << ADIF); // clear int flag by writing 1.

	// sync up with adc clock:
	while (! (ADCSRA & (1 << ADIF))); // wait until ready.
	sample = ADC; // throw it away.

	for(uint8_t mux=0; mux < 8; ++mux) {

		PORTC &= ~C_MASK;
		PORTD &= ~D_MASK;
		PORTE &= ~E_MASK;

		SET_FULL_MUX(mux); // our sample will use this

		for(uint8_t i=0; i < 2; ++i) {
			ADCSRA |= (1 << ADIF); // clear int flag by writing 1.
			//wait for last read to complete.
			while (! (ADCSRA & (1 << ADIF)));
			sample = ADC; // throw away strobe'd value.
		}

		recovery(0);
		strobe_w(column);

		ADCSRA |= (1 << ADIF); // clear int flag by writing 1.
		//wait for last read to complete.
		while (! (ADCSRA & (1 << ADIF)));
		sample = ADC; // throw away strobe'd value.

		ADCSRA |= (1 << ADIF); // clear int flag by writing 1.
		while (! (ADCSRA & (1 << ADIF)));

		// retrieve current read.
		buffer[mux] = ADC - OFFSET_VOLTAGE;
		recovery(1);

	}

	// turn off adc.
	ADCSRA &= ~(1 << ADEN);

	// pull all columns' probe-lines low.
	PORTC &= ~C_MASK;
	PORTD &= ~D_MASK;
	PORTE &= ~E_MASK;
//		recovery(1);


	return 0;
}

int sampleColumn(uint8_t column) {
	int rval = 0;

	/*
	sampleColumn_i(column, 0x0f, samples+SAMPLE_OFFSET);
	sampleColumn_i(column, 0xf0, samples+SAMPLE_OFFSET + 4 );
*/

	rval = sampleColumn_k(column, samples+SAMPLE_OFFSET);

	for(uint8_t i=0; i<8; ++i) {
		if(samples[SAMPLE_OFFSET + i] - adc_mux_averages[i] > BUMP_THRESHOLD) {
			// was a hump

			_delay_us(BUMP_REST_US);
			rval++;
			error = 0x50;
			error_data = samples[SAMPLE_OFFSET +i]; //  | ((uint16_t)i << 8);
			return rval;
		}
	}

	return rval;
}



uint8_t testColumn(uint8_t strobe) {
    uint8_t column = 0;
    uint8_t bit = 1;
    for (uint8_t i=0; i < MUXES_COUNT; ++i) {
		uint16_t delta = keys_averages[(strobe << MUXES_COUNT_XSHIFT) + i];
		if ((int16_t)samples[SAMPLE_OFFSET + i] > threshold + delta) {
			column |= bit;
		}
		bit <<= 1;
	}
    return column;
}

int main(void) {
	// set for 16 MHz clock
	CPU_PRESCALE(0);

	// Initialize the USB, and then wait for the host to set configuration.
	// If the Teensy is powered without a PC connected to the USB port,
	// this will wait forever.
	usb_init();
	while (!usb_configured()) /* wait */ ;

	// Wait an extra second for the PC's operating system to load drivers
	// and do whatever it does to actually be ready for input
	_delay_ms(1000);

	LED_CONFIG;

	setup_ADC();

	// Configure timer 0 to generate a timer overflow interrupt every
	// 256*1024 clock cycles, or approx 61 Hz when using 16 MHz clock
	// This demonstrates how to use interrupts to implement a simple
	// inactivity timeout.
	//TCCR0A = 0x00;
	//TCCR0B = 0x05;
	//TIMSK0 = (1<<TOIE0);

	DDRC = C_MASK;
	PORTC = 0;
	DDRD = D_MASK;
	PORTD = 0;
	DDRE = E_MASK;
	PORTE = 0 ;

	//DDRC |= (1 << 6);
	//PORTC &= ~(1<< 6);

	//uint16_t strobe = 1;


	uint8_t strober = 0;
	uint32_t full_av_acc = 0;

	for (int i=0; i< STROBE_LINES; ++i) {
		cur_keymap[i] = 0;
		//last_keymap[i] = 0;
		usb_keymap[i] = 0;
	}

	int16_t mux_averages[MUXES_COUNT];
	for(int i=0; i < MUXES_COUNT; ++i) {
		adc_mux_averages[i] = 0x20; // experimentally determined.
	}
	int16_t strobe_averages[STROBE_LINES];
	for(int i=0; i < STROBE_LINES; ++i) {
		adc_strobe_averages[i] = 0x20; // yup.
	}

	for(int i=0; i< KEY_COUNT; ++i) {
		keys_averages[i] = 0x40;
		keys_averages_acc[i] = (0x400);
	}

	/** warm things up a bit before we start collecting data, taking real samples. */
	for(uint8_t i = 0; i< STROBE_LINES; ++i) {
		sampleColumn(i);
	}

	while(1) {

		for (strober = 0; strober < STROBE_LINES; ++strober) {

			uint8_t tries;
			tries = 1;
			while (tries++ && sampleColumn(strober)) { tries &= 0x7; } // don't waste this one just because the last one was poop.
			column = testColumn(strober);

			if( column != cur_keymap[strober] && (count >= WARMUP_LOOPS) ) {
				tests++;

				tries = 1;
				while (tries++ && sampleColumn(strober)) { tries &= 0x7; } // don't waste this one just because the last one was poop.
				col_a = testColumn(strober);

				tries = 1;
				while (tries++ && sampleColumn(strober)) { tries &= 0x7; } // don't waste this one just because the last one was poop.
				col_b = testColumn(strober);

				tries = 1;
				while (tries++ && sampleColumn(strober)) { tries &= 0x7; } // don't waste this one just because the last one was poop.
				col_c = testColumn(strober);

				if( (col_a == col_b) && (col_b == col_c) && (cur_keymap[strober] != col_a) ) {
					cur_keymap[strober] = col_a;
					usb_dirty = 1;
				}
			}

			if(error == 0x50) {
				error_data |= (((uint16_t)strober) << 12);
			}

			for(int i=0; i<MUXES_COUNT; ++i) {
				full_samples[(strober << MUXES_COUNT_XSHIFT) + i] = samples[SAMPLE_OFFSET + i];
			}

			strobe_averages[strober] = 0;
			for (uint8_t i = SAMPLE_OFFSET; i < (SAMPLE_OFFSET + MUXES_COUNT); ++i) {
				//samples[i] -= samples[i-SAMPLE_OFFSET]; // av; // + full_av); // -something.
				//samples[i] -= OFFSET_VOLTAGE; // moved to sampleColumn.

				full_av_acc += (samples[i]);
				mux_averages[i - SAMPLE_OFFSET] += samples[i];
				strobe_averages[strober] += samples[i];
				//samples[i] -= (full_av - HYST_T);

				//++count;
			}
			adc_strobe_averages[strober] += strobe_averages[strober] >> 3;
			adc_strobe_averages[strober] >>= 1;

			/** test if we went negative. */
			if ((adc_strobe_averages[strober] & 0xFF00) && (count
					>= WARMUP_LOOPS)) {
				//count = 0; // TODO : constrain properly.
				error = 0xf; error_data = adc_strobe_averages[strober];
			}

			uint8_t strobe_line = strober << MUXES_COUNT_XSHIFT;
			for (int i = 0; i < MUXES_COUNT; ++i) {
				keys_averages_acc[strobe_line + i]
						+= samples[SAMPLE_OFFSET + i];
			}

		} // for strober

		if (count < WARMUP_LOOPS) {
			error = 0x0C;
			error_data = count;
			count++;
		}

		// verify test key is not down.
		if((cur_keymap[TEST_KEY_STROBE] & TEST_KEY_MASK) ) {
			//count=0;
			error = 0x05;
			error_data = cur_keymap[TEST_KEY_STROBE] << 8;
			error_data += full_samples[TEST_KEY_STROBE * 8];
			//threshold++;
		}

		// calc mux averages.
		if (count < WARMUP_LOOPS) {
			full_av += (full_av_acc >> (7));
			full_av >>= 1;
			//full_av = full_av_acc / count;
			full_av_acc = 0;
			for (int i=0; i < MUXES_COUNT; ++i) {

// mix in 1/4 of the current average to the running average. -> (@mux_mix = 2)
#define MUX_MIX 2

				adc_mux_averages[i] = (adc_mux_averages[i] << MUX_MIX) - adc_mux_averages[i];
				adc_mux_averages[i] += (mux_averages[i] >> 4);
				adc_mux_averages[i] >>= MUX_MIX;

				mux_averages[i] = 0;
			}

		}

#define IDLE_COUNT_MASK 0xff
#define MAX_ICS 8

#define IDLE_COUNT_SHIFT 4
#define KEYS_AVERAGES_MIX 2

		idle_count++;
		idle_count &= IDLE_COUNT_MASK;

		if (/*usb_dirty &&*/ (count >= WARMUP_LOOPS) ) {
			for (int i=0; i<STROBE_LINES; ++i) {
				usb_keymap[i] = cur_keymap[i];
			}

			dumpkeys();
			usb_dirty=0;
			_delay_ms(2);
		}

		if (count < WARMUP_LOOPS) {
			for (uint8_t i = 0; i < KEY_COUNT; ++i) {
				uint16_t acc = keys_averages_acc[i];
				uint32_t av = keys_averages[i];

				av = av + av + av + acc;
				av >>= 2;

				keys_averages[i] = av;
				keys_averages_acc[i] = 0;
			}
		}


		if(!idle_count) {

			for (int i=0; i< KEY_COUNT; ++i) {
				keys_averages_acc[i] = 0;
			}

			sampleColumn(0x0); // to resync us if we dumped a mess 'o text.
		}
	}
}


void debug_println() {
	usb_debug_putchar('\r');
	usb_debug_putchar('\n');
}


// This interrupt routine is run approx 61 times per second.
// A very simple inactivity timeout is implemented, where we
// will send a space character and print a message to the
// hid_listen debug message window.
ISR(TIMER0_OVF_vect) {
	//idle_count++;
	/*
	if (idle_count > 61) {
		idle_count = 0;
	}*/

}

void dumpkeys(void) {
	//print(" \n");
	if(error) {
		for (uint8_t i=0; i < STROBE_LINES; ++i) {
				phex(usb_keymap[i]);
				usb_debug_putchar(' ');

				//print(" ");
		}
		if (count >= WARMUP_LOOPS && error) {
			dump();
		}

		print(" : ");
		phex(error);
		error = 0;
		print(" : ");
		phex16(error_data);
		error_data = 0;
		print(" : ");
		debug_println();
	}


	for(uint8_t i = 0; i < 6; ++i) {
		keyboard_keys[i] = 0;
	}
	keyboard_modifier_keys = 0;
	uint8_t usbkeycount = 0;
	for (uint8_t i=0; i < STROBE_LINES; ++i) {
		for(uint8_t j=0; j<MUXES_COUNT; ++j) {
			if ( usb_keymap[i] & (1 << j) ) {
				uint8_t key = pgm_read_byte(matrix122F_to_set3 + ( (i << MUXES_COUNT_XSHIFT) + j) );
				if(usb_dirty) phex( key );
				if(usbkeycount < 6) {
					uint8_t usbkey = pgm_read_byte(page3_2_USB + key);

					if ((usbkey >= 0xe0) && (usbkey <= 0xe7)) { // metas
						keyboard_modifier_keys |= (1 << (usbkey & 0x07));
					} else {
						keyboard_keys[usbkeycount++] =  usbkey;
					}
				}
				if (usb_dirty) usb_debug_putchar(' ');
			}
		}
	}
	if(usb_dirty) {
		debug_println();
	}

	/** shall we send actual keyboard events? */
#if 0
	usb_keyboard_send();
#endif

}

// taken from the datasheet.
uint8_t readEE(uint16_t offset) {
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE))
	;
	/* Set up address register */
	EEAR = offset;
	/* Start eeprom read by writing EERE */
	EECR |= (1<<EERE);
	/* Return data from Data Register */
	return EEDR;
}

// taken from the datasheet - note: writing is very slow. >1ms/byte.
void writeEE(uint16_t offset, uint8_t data) {
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE))
	;
	/* Set up address and Data Registers */
	EEAR = offset;
	EEDR = data;
	/* Write logical one to EEMPE */
	EECR |= (1<<EEMPE);
	/* Start eeprom write by setting EEPE */
	EECR |= (1<<EEPE);
}

uint8_t dump_count = 0;
void dump(void) {

#if 0
	static char once = 0;
	uint8_t eb;

	if(!once) {
		print("\nwriting ee");
		eb = readEE(0x10);
		eb++;
		writeEE(0x10, eb);

		once++;
	}


	print("\n ee: ");
	for(uint16_t i=0x10; i< 0x18; ++i) {
		phex(readEE(i));
		pchar(0x20);
	}
#endif

	if(!dump_count) {  // we don't want to debug-out during the measurements.

		for(int i =0; i< KEY_COUNT; ++i) {
			if(!(i & 0x0f)) {
				print("\n");
			} else if (!(i & 0x07)) {
				print("  ");
			}
			usb_debug_putchar(' ');
			//phex16 (keys_averages[(i >> MUXES_COUNT_XSHIFT) + (i & STROBE_LINES_MASK) ]);
			phex (keys_averages[i]);
		}

		print("\n");

		for(int i =0; i< KEY_COUNT; ++i) {
			if(!(i & 0x0f)) {
				print("\n");
			} else if (!(i & 0x07)) {
				print("  ");
			}
			usb_debug_putchar(' ');
			//phex16 (keys_averages[(i >> MUXES_COUNT_XSHIFT) + (i & STROBE_LINES_MASK) ]);
			//phex16 (keys_averages_acc[i]);
			phex(full_samples[i]);
		}
	}


	//}

//	uint8_t cur_strober = 0xe;
	uint8_t cur_strober = ze_strober;
	print("\n");

	phex(cur_strober);
	//print(":         ");
	print(": ");
#if 1
	print("\n");
	for (uint8_t i=0; i < MUXES_COUNT; ++i) {
		usb_debug_putchar(' ');
		phex16(full_samples[(cur_strober << MUXES_COUNT_XSHIFT) + i]);
	}

	print("\n");
//	phex(threshold);
//	print(": ");

	for (uint8_t i=0; i < MUXES_COUNT; ++i) {
		usb_debug_putchar(' ');
		phex16(keys_averages[(cur_strober << MUXES_COUNT_XSHIFT) + i]);
	}

#endif
	/*
	for (uint8_t i=0; i< SAMPLES; ++i) {
		print(" ");
		phex16(samples[i]);
		//phex16(ADC);
	}*/
	//print(" : ");
	//usb_debug_putchar((was_active)?' ':'*');

	//phex16(keymap[TEST_KEY_STROBE] & TEST_KEY_MASK);
	/*print(" "); */
	//phex(keymap[TEST_KEY_STROBE]);


	//print("\n");
	//print(":");
	//phex(full_av);
	//phex16(count);
	//print(" : ");
	print("\n");

	for (uint8_t i=0; i < STROBE_LINES; ++i) {
		phex(cur_keymap[i]);
		usb_debug_putchar(' ');

		//print(" ");
	}


	//print(": ");
	//phex(adc_strobe_averages[ze_strober]);
	//usb_debug_putchar(' ');



	for (uint8_t i=0; i < MUXES_COUNT; ++i) {
		usb_debug_putchar(' ');
		//phex16(adc_mux_averages[i] + adc_strobe_averages[ze_strober] - full_av);
		//phex16((adc_mux_averages[i] + adc_strobe_averages[ze_strober]) >> 1);
		//phex16((adc_mux_averages[i] * 3  + adc_strobe_averages[ze_strober]) >> 2);
		//phex16(adc_mux_averages[i] + threshold);
		//phex16(gsamples[i + SAMPLE_OFFSET] - (adc_mux_averages[i] + threshold) + 0x100);
		//phex16(keys_averages[(ze_strober << MUXES_COUNT_XSHIFT) + i] + (uint8_t)threshold);
		phex16(keys_averages[(ze_strober << MUXES_COUNT_XSHIFT) + i]);
	}

	if(error) {
		usb_debug_putchar(' ');
		phex (error);
		usb_debug_putchar(' ');
		phex16(error_data);
		error = 0;
		error_data = 0;
	}
	//print("\n");

	ze_strober++;
	ze_strober &= 0xf;


	dump_count++;
	dump_count &= 0x0f;



	//ze_strobe = (1 << (ze_strober ) );



	//phex(ADCSRA);
	//print(" ");
			//print("\n");
	//usb_keyboard_press(KEY_SPACE, 0);

//		if(blink) {
//			LED_ON;
//		} else {
//			LED_OFF;
//		}
//		blink ^= 1;
}


/*
int oldmain(void) {
	uint8_t b, d, mask, i, reset_idle;
	uint8_t b_prev=0xFF, d_prev=0xFF;

	// set for 16 MHz clock
	CPU_PRESCALE(0);

	// Configure all port B and port D pins as inputs with pullup resistors.
	// See the "Using I/O Pins" page for details.
	// http://www.pjrc.com/teensy/pins.html
	DDRD = 0x00;
	DDRB = 0x00;
	PORTB = 0xFF;
	PORTD = 0xFF;

	// Initialize the USB, and then wait for the host to set configuration.
	// If the Teensy is powered without a PC connected to the USB port,
	// this will wait forever.
	usb_init();
	while (!usb_configured())  ;

	// Wait an extra second for the PC's operating system to load drivers
	// and do whatever it does to actually be ready for input
	_delay_ms(1000);

	// Configure timer 0 to generate a timer overflow interrupt every
	// 256*1024 clock cycles, or approx 61 Hz when using 16 MHz clock
	// This demonstrates how to use interrupts to implement a simple
	// inactivity timeout.
	TCCR0A = 0x00;
	TCCR0B = 0x05;
	TIMSK0 = (1<<TOIE0);

	print("Begin keyboard example program\n");
	print("All Port B or Port D pins are inputs with pullup resistors.\n");
	print("Any connection to ground on Port B or D pins will result in\n");
	print("keystrokes sent to the PC (and debug messages here).\n");
	while (1) {
		// read all port B and port D pins
		b = PINB;
		d = PIND;
		// check if any pins are low, but were high previously
		mask = 1;
		reset_idle = 0;
		for (i=0; i<8; i++) {
			if (((b & mask) == 0) && (b_prev & mask) != 0) {
				usb_keyboard_press(KEY_B, KEY_SHIFT);
				usb_keyboard_press(number_keys[i], 0);
				print("Port B, bit ");
				phex(i);
				print("\n");
				reset_idle = 1;
			}
			if (((d & mask) == 0) && (d_prev & mask) != 0) {
				usb_keyboard_press(KEY_D, KEY_SHIFT);
				usb_keyboard_press(number_keys[i], 0);
				print("Port D, bit ");
				phex(i);
				print("\n");
				reset_idle = 1;
			}
			mask = mask << 1;
		}
		// if any keypresses were detected, reset the idle counter
		if (reset_idle) {
			// variables shared with interrupt routines must be
			// accessed carefully so the interrupt routine doesn't
			// try to use the variable in the middle of our access
			cli();
			idle_count = 0;
			sei();
		}
		// now the current pins will be the previous, and
		// wait a short delay so we're not highly sensitive
		// to mechanical "bounce".
		b_prev = b;
		d_prev = d;
		_delay_ms(2);
	}
}
*/

