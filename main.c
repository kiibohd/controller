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
#include "usb_keyboard.h"

#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))

#define PRE_DRIVE_SLEEP
#define POST_DRIVE_SLEEP


#define DRIVE_reg_1 PORTB
#define DRIVE_reg_2 PORTB
#define DRIVE_reg_3 PORTB
#define DRIVE_reg_4 PORTC
#define DRIVE_reg_5 PORTE
#define DRIVE_reg_6 PORTE
#define DRIVE_reg_7 PORTF
#define DRIVE_reg_8 PORTF
#define DRIVE_reg_9 PORTF
#define DRIVE_reg_10 <blank>
#define DRIVE_reg_11 <blank>
#define DRIVE_reg_12 <blank>

#define DRIVE_pin_1 0
#define DRIVE_pin_2 1
#define DRIVE_pin_3 2
#define DRIVE_pin_4 7
#define DRIVE_pin_5 6
#define DRIVE_pin_6 7
#define DRIVE_pin_7 0
#define DRIVE_pin_8 4
#define DRIVE_pin_9 5
#define DRIVE_pin_10 <blank>
#define DRIVE_pin_11 <blank>
#define DRIVE_pin_12 <blank>

#define DETECT_group_1 0
#define DETECT_group_2 0
#define DETECT_group_3 0
#define DETECT_group_4 0
#define DETECT_group_5 0
#define DETECT_group_6 0
#define DETECT_group_7 0
#define DETECT_group_8 0
#define DETECT_group_9 0
#define DETECT_group_10 <blank>
#define DETECT_group_11 <blank>
#define DETECT_group_12 <blank>

// Change number of ORDs if number of lines differ
#define DD_LOOP \
			for ( int c = 0;; c++ ) { \
				switch ( c ) { \
					DD_CASE_ORD(1) \
					DD_CASE_ORD(2) \
					DD_CASE_ORD(3) \
					DD_CASE_ORD(4) \
					DD_CASE_ORD(5) \
					DD_CASE_ORD(6) \
					DD_CASE_ORD(7) \
					DD_CASE_ORD(8) \
					DD_CASE_END(9,c) \
				} \
			}

#define DRIVE_DETECT(reg,pin,group) \
			reg |= (1 << pin);\
			detection(group);\
			reg &= (0 << pin);

#define DD_CASE(number) \
			case number:\
				DRIVE_DETECT(DRIVE_reg##_##number, DRIVE_pin##_##number, DETECT_group##_##number)

#define DD_CASE_ORD(number) \
			DD_CASE(number) \
			break;

#define DD_CASE_END(number,var) \
			DD_CASE(number) \
			default: \
			var = -1; \
			break;

int main(void)
{
	// set for 16 MHz clock
	CPU_PRESCALE( 0 );

	// Configuring Pins

	// TODO

	// Initialize the USB, and then wait for the host to set configuration.
	// If the Teensy is powered without a PC connected to the USB port,
	// this will wait forever.
	usb_init();
	while ( !usb_configured() ) /* wait */ ;

	// Wait an extra second for the PC's operating system to load drivers
	// and do whatever it does to actually be ready for input
	_delay_ms(1000);

	// Main Detection Loop
	DD_LOOP

	// usb_keyboard_press(KEY_B, KEY_SHIFT);
	return 0;
}

