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
//#include "usb_keys.h"
#include "scan_loop.h"
//#include "layouts.h"
//#include "usb_keyboard.h"

// TEMP INCLUDES
#include "usb_keyboard_debug.h"
#include "print.h"

#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))




// Verified Keypress Defines
#define USB_TRANSFER_DIVIDER 10 // 1024 == 1 Send of keypresses per second, 1 == 1 Send of keypresses per ~1 millisecond

// Error LED Control
void errorLED( uint8_t on )
{
	// Error LED On
	if ( on ) {
		PORTD |= (1<<6);
	}
	// Error LED Off
	else {
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

	uint16_t led = 0;
	// Main Detection Loop
	while ( 1 ) {
		//scan_loop();

		// Loop should never get here (indicate error)
		errorLED( 1 );

		// HID Debug Error message
		erro_print("Detection loop error, this is very bad...bug report!");
	}
}

// Timer Interrupt for flagging a send of the sampled key detection data to the USB host
uint16_t sendKeypressCounter = 0;

ISR( TIMER0_OVF_vect )
{
	sendKeypressCounter++;
	if ( sendKeypressCounter > USB_TRANSFER_DIVIDER ) {
		sendKeypressCounter = 0;
		sendKeypresses = 1;
	}
}

