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
#include "usb_keys.h"
#include "scan_loop.h"
//#include "layouts.h"
//#include "usb_keyboard.h"

// TEMP INCLUDES
#include "usb_keyboard_debug.h"
#include <print.h>

#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))


// Debouncing Defines
#define SAMPLE_THRESHOLD 110
#define MAX_SAMPLES 127 // Max is 127, reaching 128 is very bad


// Verified Keypress Defines
#define USB_TRANSFER_DIVIDER 10 // 1024 == 1 Send of keypresses per second, 1 == 1 Send of keypresses per ~1 millisecond

/*
// Number of keys
#define KEYBOARD_SIZE 63
#define KEYPAD_SIZE 16

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

#define DETECT_group_size_1 7
#define DETECT_group_size_2 7
#define DETECT_group_size_3 6
#define DETECT_group_size_4 8
#define DETECT_group_size_5 7
#define DETECT_group_size_6 7
#define DETECT_group_size_7 8
#define DETECT_group_size_8 8
#define DETECT_group_size_9 4
#define DETECT_group_size_10 <blank>
#define DETECT_group_size_11 <blank>
#define DETECT_group_size_12 <blank>

// Switch Codes
#define DETECT_group_array_1 {55,22,6 ,40,43,27,11}
#define DETECT_group_array_2 {56,23,7 ,41,58,26,10}
#define DETECT_group_array_3 {57,24,8 ,42,25,9}
#define DETECT_group_array_4 {54,21,5 ,39,44,28,12,59}
#define DETECT_group_array_5 {53,20,4 ,38,45,29,13}
#define DETECT_group_array_6 {52,19,3 ,37,46,30,14}
#define DETECT_group_array_7 {51,18,2 ,36,61,31,15,63}
#define DETECT_group_array_8 {50,17,1 ,35,47,32,16,62}
#define DETECT_group_array_9 {48,49,33,34} // 49/60 are the same line
#define DETECT_group_array_10 <blank>
#define DETECT_group_array_11 <blank>
#define DETECT_group_array_12 <blank>



// Drive Macros (Generally don't need to be changed), except for maybe DRIVE_DETECT
// Sleep is for signal debouncing
#define DRIVE_DETECT(reg,pin,group) \
			reg &= ~(1 << pin); \
			detection(group); \
			reg |= (1 << pin); \
			_delay_us(1);

#define DD_CASE(number) \
			case number:\
				DRIVE_DETECT(DRIVE_reg_##number, DRIVE_pin_##number, DETECT_group_##number)

#define DD_CASE_ORD(number) \
			DD_CASE(number) \
			break;

#define DD_CASE_END(number,var) \
			DD_CASE(number) \
			var = -1; \
			break;


// Updates the current detection sample and last sample bit
// Detection Macros (Probably don't need to be changed, but depending the matrix, may have to be)
// Determine if key is either normal or a modifier
#define DET_GROUP_CHECK(index,test) \
			if ( test ) { \
				keyDetectArray[groupArray[index]]++; \
			}


// XXX - Detection Groups
// Checks each of the specified pins, and then if press detected, determine if the key is normal or a modifier
// Inverse logic applies for the PINs

// Used for 1 detection group (Special group)
#define DET_GROUP_1 \
			DET_GROUP_CHECK(0,!( PINB & (1 << 7) )) \
			DET_GROUP_CHECK(1,!( PINC & (1 << 0) )) \
			DET_GROUP_CHECK(2,!( PIND & (1 << 0) )) \
			DET_GROUP_CHECK(3,!( PIND & (1 << 1) )) \

// Used for 4 detection groups (Skips J1 P9)
#define DET_GROUP_2 \
			DET_GROUP_CHECK(0,!( PINE & (1 << 7) )) \
			DET_GROUP_CHECK(1,!( PINB & (1 << 0) )) \
			DET_GROUP_CHECK(2,!( PINB & (1 << 1) )) \
			DET_GROUP_CHECK(3,!( PINB & (1 << 2) )) \
			DET_GROUP_CHECK(4,!( PINB & (1 << 3) )) \
			DET_GROUP_CHECK(5,!( PINB & (1 << 4) )) \
			DET_GROUP_CHECK(6,!( PINB & (1 << 5) )) \

// Used for 1 detection group (Skips J1 P6 and J1 P9)
#define DET_GROUP_3 \
			DET_GROUP_CHECK(0,!( PINE & (1 << 7) )) \
			DET_GROUP_CHECK(1,!( PINB & (1 << 0) )) \
			DET_GROUP_CHECK(2,!( PINB & (1 << 1) )) \
			DET_GROUP_CHECK(3,!( PINB & (1 << 2) )) \
			DET_GROUP_CHECK(4,!( PINB & (1 << 4) )) \
			DET_GROUP_CHECK(5,!( PINB & (1 << 5) )) \

// Used for 3 detection groups (No skips, except special group 1)
#define DET_GROUP_4 \
			DET_GROUP_CHECK(0,!( PINE & (1 << 7) )) \
			DET_GROUP_CHECK(1,!( PINB & (1 << 0) )) \
			DET_GROUP_CHECK(2,!( PINB & (1 << 1) )) \
			DET_GROUP_CHECK(3,!( PINB & (1 << 2) )) \
			DET_GROUP_CHECK(4,!( PINB & (1 << 3) )) \
			DET_GROUP_CHECK(5,!( PINB & (1 << 4) )) \
			DET_GROUP_CHECK(6,!( PINB & (1 << 5) )) \
			DET_GROUP_CHECK(7,!( PINB & (1 << 6) )) \

// Combines the DET_GROUP_Xs above for the given groupArray
#define DET_GROUP(group,det_group) \
			case group: \
				{ \
					uint8_t groupArray[DETECT_group_size_##group] = DETECT_group_array_##group; \
					_delay_us(1); \
					DET_GROUP_##det_group \
				} \
				break;


// Loop over all of the sampled keys of the given array
// If the number of samples is higher than the sample threshold, flag the high bit, clear otherwise
// This should be resetting VERY quickly, cutting off a potentially valid keypress is not an issue
#define DEBOUNCE_ASSESS(table,size) \
			for ( uint8_t key = 1; key < size + 1; key++ ) {\
				table[key] = ( table[key] & ~(1 << 7) ) > SAMPLE_THRESHOLD ? (1 << 7) : 0x00; \
			} \


// Keypad detection
// Each switch has it's own detection line, inverse logic
#define KEYPAD_DETECT(test,switch_code) \
			if ( !(test) ) { \
				keypadDetectArray[switch_code]++; \
			} \


// NOTE: Highest Bit: Valid keypress (0x80 is valid keypress)
//        Other Bits: Pressed state sample counter
uint8_t keyboardDetectArray[KEYBOARD_SIZE + 1];

// Interrupt Variables
uint16_t sendKeypressCounter = 0;
volatile uint8_t sendKeypresses = 0;


void detection( int group )
{
	// XXX Modify for different detection groups <-> groupArray mappings
	switch ( group ) {
		DET_GROUP(1,2)
		DET_GROUP(2,2)
		DET_GROUP(3,3)
		DET_GROUP(4,4)
		DET_GROUP(5,2)
		DET_GROUP(6,2)
		DET_GROUP(7,4)
		DET_GROUP(8,4)
		DET_GROUP(9,1)
	}
}
*/

// Error LED Control
void errorLED( uint8_t on )
{
	// Error LED On
	if ( on ) {
		DDRD  |= (1<<6);
		PORTD |= (1<<6);
	}
	// Error LED Off
	else {
		DDRD  &= ~(1<<6);
		PORTD &= ~(1<<6);
	}
}



// Initial Pin Setup
// If the matrix is properly set, this function does not need to be changed
inline void pinSetup(void)
{
	// For each pin, 0=input, 1=output
	DDRA = 0x00;
	DDRB = 0x00;
	DDRC = 0x00;
	DDRD = 0x40; // LED Setup
	DDRE = 0x00;
	DDRF = 0x00;


	// Setting pins to either high or pull-up resistor
	PORTA = 0x00;
	PORTB = 0x00;
	PORTC = 0x00;
	PORTD = 0x40; // LED Enable
	PORTE = 0x00;
	PORTF = 0x00;
}
/*
// Given a sampling array, and the current number of detected keypress
// Add as many keypresses from the sampling array to the USB key send array as possible.
void keyPressDetection( uint8_t *keys, uint8_t *validKeys, uint8_t numberOfKeys, uint8_t *modifiers, uint8_t numberOfModifiers, uint8_t *map ) {
	for ( uint8_t key = 0; key < numberOfKeys + 1; key++ ) {
		if ( keys[key] & (1 << 7) ) {
			pint8( key );
			//print(" ");
			uint8_t modFound = 0;

			// Determine if the key is a modifier
			for ( uint8_t mod = 0; mod < numberOfModifiers; mod++ ) {
				// Modifier found
				if ( modifiers[mod] == key ) {
					keyboard_modifier_keys |= map[key];
					modFound = 1;
					break;
				}
			}
			if ( modFound )
				continue;

			// Too many keys
			if ( *validKeys == 6 )
				break;

			// Allow ignoring keys with 0's
			if ( map[key] != 0 )
				keyboard_keys[(*validKeys)++] = map[key];
		}
	}
}
*/

int main( void )
{
	// Setup with 16 MHz clock
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

	// Setup ISR Timer for flagging a kepress send to USB
	// Set to 256 * 1024 (8 bit timer with Clock/1024 prescalar) timer
	TCCR0A = 0x00;
	TCCR0B = 0x03;
	TIMSK0 = (1 << TOIE0);

	// Main Detection Loop
	while ( 1 ) {
		scan_loop();

		// Loop should never get here (indicate error)
		errorLED( 1 );

		// TODO HID Debug message
	}
}
/*
	int8_t group = 1;
	uint8_t count = 0;
	for ( ;;group++ ) {
		// XXX Change number of ORDs if number of lines (RowsxColumns) differ
		// Determine which keys are being pressed
		switch ( group ) {
			DD_CASE_ORD(1)
			DD_CASE_ORD(2)
			DD_CASE_ORD(3)
			DD_CASE_ORD(4)
			DD_CASE_ORD(5)
			DD_CASE_ORD(6)
			DD_CASE_ORD(7)
			DD_CASE_ORD(8)
			DD_CASE_END(9,group)
		}

		// Check all Keyboard keys first
		if ( group != -1 )
			continue;

		// Check Keypad keys
		KEYPAD_DETECT(PINA & (1 << 0),11)
		KEYPAD_DETECT(PINA & (1 << 1),3)
		KEYPAD_DETECT(PINA & (1 << 2),7)
		KEYPAD_DETECT(PINA & (1 << 3),4)
		KEYPAD_DETECT(PINA & (1 << 4),15)
		KEYPAD_DETECT(PINA & (1 << 5),6)
		KEYPAD_DETECT(PINA & (1 << 6),2)
		KEYPAD_DETECT(PINA & (1 << 7),10)
		KEYPAD_DETECT(PINF & (1 << 0),8)
		KEYPAD_DETECT(PINF & (1 << 1),12)
		KEYPAD_DETECT(PINF & (1 << 2),16)
		KEYPAD_DETECT(PINF & (1 << 3),13)
		KEYPAD_DETECT(PINF & (1 << 4),1)
		KEYPAD_DETECT(PINF & (1 << 5),5)
		KEYPAD_DETECT(PINF & (1 << 6),9)
		KEYPAD_DETECT(PINF & (1 << 7),14)

		// Check count to see if the sample threshold may have been reached, otherwise collect more data
		count++;
		if ( count < MAX_SAMPLES )
			continue;

		// Reset Sample Counter
		count = 0;

		// Assess debouncing sample table
		DEBOUNCE_ASSESS(keyDetectArray,KEYBOARD_SIZE)
		DEBOUNCE_ASSESS(keypadDetectArray,KEYPAD_SIZE)

		// Send keypresses over USB if the ISR has signalled that it's time
		if ( !sendKeypresses )
			continue;

		// Detect Valid Keypresses - TODO
		uint8_t validKeys = 0;

		uint8_t *keyboard_MODMASK = keyboard_modifierMask;
		uint8_t  keyboard_NUMMODS = MODIFIERS_KEYBOARD;
		uint8_t *keyboard_MAP     = defaultMap;
		uint8_t *keypad_MODMASK   = keypad_modifierMask;
		uint8_t  keypad_NUMMODS   = MODIFIERS_KEYPAD;
		uint8_t *keypad_MAP       = keypadDefaultMap;

		// Map selection - CapsLock FN
		if ( keyDetectArray[34] & (1 << 7) ) { // CapsLock FN Modifier
				keyboard_MAP     = colemakMap;
				keyboard_MODMASK = alternate_modifierMask;
				keyboard_NUMMODS = 5;

			// Function Key
			if ( keyDetectArray[61] & (1 << 7) ) {
				keyboard_MAP     = navigationMap;
			}
		}

		keyPressDetection( keyDetectArray, &validKeys, KEYBOARD_SIZE, keyboard_MODMASK, keyboard_NUMMODS, keyboard_MAP );
		keyPressDetection( keypadDetectArray, &validKeys, KEYPAD_SIZE, keypad_MODMASK, keypad_NUMMODS, keypad_MAP );
		//print(":\n");

		// TODO undo potentially old keys
		for ( uint8_t c = validKeys; c < 6; c++ )
			keyboard_keys[c] = 0;

		// Send keypresses
		usb_keyboard_send();

		// Clear sendKeypresses Flag
		sendKeypresses = 0;

		// Clear modifiers
		keyboard_modifier_keys = 0;
	}

	return 0;
}
*/

// Timer Interrupt for flagging a send of the sampled key detection data to the USB host
ISR( TIMER0_OVF_vect )
{
	sendKeypressCounter++;
	if ( sendKeypressCounter > USB_TRANSFER_DIVIDER ) {
		sendKeypressCounter = 0;
		sendKeypresses = 1;
	}
}

