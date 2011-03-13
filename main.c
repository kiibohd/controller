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

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
//#include "usb_keyboard.h"

// TEMP INCLUDES
#include "usb_keyboard_debug.h"
#include <print.h>

#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))

// Sleep defined in milliseconds
#define PRE_DRIVE_SLEEP  10
#define POST_DRIVE_SLEEP 10


// Drive Pin Defines
#define DRIVE_reg_1 PORTD
#define DRIVE_reg_2 PORTD
#define DRIVE_reg_3 PORTD
#define DRIVE_reg_4 PORTD
#define DRIVE_reg_5 PORTD
#define DRIVE_reg_6 PORTD
#define DRIVE_reg_7 PORTE
#define DRIVE_reg_8 PORTE
#define DRIVE_reg_9 PORTE
#define DRIVE_reg_10 <blank>
#define DRIVE_reg_11 <blank>
#define DRIVE_reg_12 <blank>

#define DRIVE_pin_1 2
#define DRIVE_pin_2 3
#define DRIVE_pin_3 4
#define DRIVE_pin_4 5
#define DRIVE_pin_5 6
#define DRIVE_pin_6 7
#define DRIVE_pin_7 0
#define DRIVE_pin_8 1
#define DRIVE_pin_9 6
#define DRIVE_pin_10 <blank>
#define DRIVE_pin_11 <blank>
#define DRIVE_pin_12 <blank>

// Detect Pin/Group Defines
#define DETECT_group_1 1
#define DETECT_group_2 2
#define DETECT_group_3 3
#define DETECT_group_4 4
#define DETECT_group_5 5
#define DETECT_group_6 6
#define DETECT_group_7 7
#define DETECT_group_8 8
#define DETECT_group_9 9
#define DETECT_group_10 <blank>
#define DETECT_group_11 <blank>
#define DETECT_group_12 <blank>

#define DETECT_group_size_1 4
#define DETECT_group_size_2 8
#define DETECT_group_size_3 8
#define DETECT_group_size_4 7
#define DETECT_group_size_5 7
#define DETECT_group_size_6 8
#define DETECT_group_size_7 6
#define DETECT_group_size_8 7
#define DETECT_group_size_9 7
#define DETECT_group_size_10 <blank>
#define DETECT_group_size_11 <blank>
#define DETECT_group_size_12 <blank>

#define DETECT_group_array_1 {{KEY_ESC,KEY_CTRL,KEY_CAPS_LOCK,KEY_SHIFT},{0,1,0,1}}
#define DETECT_group_array_2 {{KEY_BACKSPACE,KEY_UP,KEY_DOWN,KEY_A,KEY_INSERT,KEY_ALT,KEY_Z,KEY_RIGHT},{0,0,0,0,0,1,0,0}}
#define DETECT_group_array_3 {{KEY_TILDE,KEY_DELETE,KEY_LEFT,KEY_SPACE,KEY_X,KEY_S,KEY_TAB,KEY_1},{0,0,0,0,0,0,0,0}}
#define DETECT_group_array_4 {{KEY_SLASH,KEY_RIGHT_BRACE,KEY_ENTER,KEY_D,KEY_2,KEY_Q,KEY_C},{0,0,0,0,0,0,0}}
#define DETECT_group_array_5 {{KEY_EQUAL,KEY_LEFT_BRACE,KEY_QUOTE,KEY_F,KEY_3,KEY_W,KEY_V},{0,0,0,0,0,0,0}}
#define DETECT_group_array_6 {{KEY_MINUS,KEY_P,KEY_SEMICOLON,KEY_G,KEY_4,KEY_E,KEY_B,KEY_BACKSLASH},{0,0,0,0,0,0,0,0}}
#define DETECT_group_array_7 {{KEY_8,KEY_U,KEY_K,KEY_7,KEY_Y,KEY_COMMA},{0,0,0,0,0,0}}
#define DETECT_group_array_8 {{KEY_9,KEY_I,KEY_PERIOD,KEY_J,KEY_6,KEY_T,KEY_M},{0,0,0,0,0,0,0}}
#define DETECT_group_array_9 {{KEY_0,KEY_O,KEY_L,KEY_H,KEY_5,KEY_R,KEY_N},{0,0,0,0,0,0,0}}
#define DETECT_group_array_10 <blank>
#define DETECT_group_array_11 <blank>
#define DETECT_group_array_12 <blank>



// Drive Macros (Generally don't need to be changed), except for maybe DRIVE_DETECT
#define DRIVE_DETECT(reg,pin,group) \
			reg &= ~(1 << pin); \
			detection(group); \
			reg |= (1 << pin); \
			_delay_ms(POST_DRIVE_SLEEP);

#define DD_CASE(number) \
			case number:\
				DRIVE_DETECT(DRIVE_reg##_##number, DRIVE_pin##_##number, DETECT_group##_##number)

#define DD_CASE_ORD(number) \
			DD_CASE(number) \
			break;

#define DD_CASE_END(number,var) \
			DD_CASE(number) \
			var = -1; \
			break;


// Detection Macros (Probably don't need to be changed, but depending the matrix, may have to be)
// Determine if key is either normal or a modifier
#define DET_GROUP_CHECK(index) \
			{ \
			if ( groupArray[1][index] ) \
				curDetect.modifiers |= groupArray[0][index]; \
			else \
				curDetect.keyDetectArray[curDetect.keyDetectCount++] = groupArray[0][index]; \
			}


// XXX - Detection Groups
// Checks each of the specified pins, and then if press detected, determine if the key is normal or a modifier
// Inverse logic applies for the PINs

// Used for 1 detection group
#define DET_GROUP_1 \
			if ( !( PINC & (1 << 0) ) ) \
				DET_GROUP_CHECK(3) \
			if ( !( PINE & (1 << 1) ) ) \
				DET_GROUP_CHECK(2) \
			if ( !( PINE & (1 << 0) ) ) \
				DET_GROUP_CHECK(1) \
			if ( !( PINB & (1 << 7) ) ) \
				DET_GROUP_CHECK(0)

// Used for 4 detection groups
#define DET_GROUP_2 \
			if ( !( PINC & (1 << 0) ) ) \
				DET_GROUP_CHECK(0) \
			if ( !( PINC & (1 << 1) ) ) \
				DET_GROUP_CHECK(1) \
			if ( !( PINC & (1 << 2) ) ) \
				DET_GROUP_CHECK(2) \
			if ( !( PINC & (1 << 3) ) ) \
				DET_GROUP_CHECK(3) \
			if ( !( PINC & (1 << 4) ) ) \
				DET_GROUP_CHECK(4) \
			if ( !( PINC & (1 << 5) ) ) \
				DET_GROUP_CHECK(5) \
			if ( !( PINC & (1 << 6) ) ) \
				DET_GROUP_CHECK(6) \

// Used for 1 detection group
#define DET_GROUP_3 \
			if ( !( PINC & (1 << 0) ) ) \
				DET_GROUP_CHECK(0) \
			if ( !( PINC & (1 << 1) ) ) \
				DET_GROUP_CHECK(1) \
			if ( !( PINC & (1 << 3) ) ) \
				DET_GROUP_CHECK(2) \
			if ( !( PINC & (1 << 4) ) ) \
				DET_GROUP_CHECK(3) \
			if ( !( PINC & (1 << 5) ) ) \
				DET_GROUP_CHECK(4) \
			if ( !( PINC & (1 << 6) ) ) \
				DET_GROUP_CHECK(5) \

// Used for 3 detection groups
#define DET_GROUP_4 \
			if ( !( PINC & (1 << 0) ) ) \
				DET_GROUP_CHECK(0) \
			if ( !( PINC & (1 << 1) ) ) \
				DET_GROUP_CHECK(1) \
			if ( !( PINC & (1 << 2) ) ) \
				DET_GROUP_CHECK(2) \
			if ( !( PINC & (1 << 3) ) ) \
				DET_GROUP_CHECK(3) \
			if ( !( PINC & (1 << 4) ) ) \
				DET_GROUP_CHECK(4) \
			if ( !( PINC & (1 << 5) ) ) \
				DET_GROUP_CHECK(5) \
			if ( !( PINC & (1 << 6) ) ) \
				DET_GROUP_CHECK(6) \
			if ( !( PINE & (1 << 1) ) ) \
				DET_GROUP_CHECK(7) \

// Combines the DET_GROUP_Xs above for the given groupArray
#define DET_GROUP(group,det_group) \
			case group: \
				{ \
					uint8_t groupArray[2][DETECT_group_size##_##group] = DETECT_group_array##_##group; \
					DET_GROUP##_##det_group \
				} \
				break;

struct keys {
	uint8_t keyDetectCount;
	uint8_t keyDetectArray[40];
	uint8_t modifiers;
} curDetect, prevDetect;

void detection( int group )
{
	_delay_ms(PRE_DRIVE_SLEEP);

	// XXX Modify for different detection groups <-> groupArray mappings
	switch ( group ) {
		DET_GROUP(1,1)
		/*
		DET_GROUP(2,4)
		DET_GROUP(3,4)
		DET_GROUP(4,1)
		DET_GROUP(5,4)
		DET_GROUP(6,2)
		DET_GROUP(7,2)
		DET_GROUP(8,3)
		DET_GROUP(9,2)
		*/
	}


	// Print out the current keys pressed
	if ( curDetect.keyDetectCount > 0 ) {
		print("Keys: ");
		for ( int c = 0; c < curDetect.keyDetectCount; c++ ) {
			phex( curDetect.keyDetectArray[c] );
			print(" ");
		}
		print("\n");
	}
	if ( curDetect.modifiers ) {
		print("Modifiers: ");
		phex( curDetect.modifiers );
		print("\n");
	}
}



// XXX This part is configurable
void pinSetup(void)
{
	// For each pin, 0=input, 1=output
	DDRA = 0x00;
	DDRB = 0x07;
	DDRC = 0x80;
	DDRD = 0x00;
	DDRE = 0xC0;
	DDRF = 0x31;

	// Setting pins to either high or pull-up resistor
	PORTA = 0x00;
	PORTB = 0x0F;
	PORTC = 0xFF;
	PORTD = 0x00;
	PORTE = 0xC2;
	PORTF = 0x3F;
}

int main( void )
{
	// set for 16 MHz clock
	CPU_PRESCALE( 0 );

	// Configuring Pins
	pinSetup();

	// Initialize the USB, and then wait for the host to set configuration.
	// If the Teensy is powered without a PC connected to the USB port,
	// this will wait forever.
	usb_init();
	while ( !usb_configured() ) /* wait */ ;

	// Wait an extra second for the PC's operating system to load drivers
	// and do whatever it does to actually be ready for input
	_delay_ms(1000);

	// Make sure variables are properly initialized
	curDetect.keyDetectCount = 0;
	curDetect.modifiers = 0;

	// Main Detection Loop
	// XXX Change number of ORDs if number of lines differ
	for ( int group = 1;;group++ ) {
		// Determine which keys are being pressed
		switch ( group ) {
			/*
			DD_CASE_ORD(1)
			DD_CASE_ORD(2)
			DD_CASE_ORD(3)
			DD_CASE_ORD(4)
			DD_CASE_ORD(5)
			DD_CASE_ORD(6)
			DD_CASE_ORD(7)
			DD_CASE_ORD(8)
			*/
			DD_CASE_END(9,group)
		}

		if ( group != -1 )
			continue;

		// After going through each of the key groups, send the detected keys and modifiers
		// Currently limited to the USB spec (6 keys + modifiers)
		// Making sure to pass zeros when there are no keys being pressed
		for ( int c = 0; c < 6 && c < curDetect.keyDetectCount; c++ )
			keyboard_keys[c] = c < curDetect.keyDetectCount ? curDetect.keyDetectArray[c] : 0;

		// Modifiers
		keyboard_modifier_keys = curDetect.modifiers;

		// Send keypresses
		usb_keyboard_send();

		// Cleanup
		curDetect.keyDetectCount = 0;
		curDetect.modifiers = 0;
	}

	// usb_keyboard_press(KEY_B, KEY_SHIFT);
	return 0;
}

