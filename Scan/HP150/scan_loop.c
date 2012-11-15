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

// ----- Includes -----

// AVR Includes
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

// Project Includes
#include <led.h>
#include <print.h>

// Local Includes
#include "scan_loop.h"



// ----- Defines -----

// Pinout Defines
#define DATA_PORT PORTC
#define DATA_DDR   DDRC
#define DATA_PIN      7

#define CLOCK_PORT PORTC
#define CLOCK_DDR   DDRC
#define CLOCK_PIN      6

#define RESET_PORT PORTF
#define RESET_DDR   DDRF
#define RESET_PIN      7


// ----- Macros -----

// Make sure we haven't overflowed the buffer
#define bufferAdd(byte) \
		if ( KeyIndex_BufferUsed < KEYBOARD_BUFFER ) \
			KeyIndex_Buffer[KeyIndex_BufferUsed++] = byte



// ----- Variables -----

// Buffer used to inform the macro processing module which keys have been detected as pressed
volatile uint8_t KeyIndex_Buffer[KEYBOARD_BUFFER];
volatile uint8_t KeyIndex_BufferUsed;
volatile uint8_t KeyIndex_Add_InputSignal; // Used to pass the (click/input value) to the keyboard for the clicker

volatile uint8_t currentWaveState = 0;


// Buffer Signals
volatile uint8_t BufferReadyToClear;



// ----- Function Declarations -----

void processKeyValue( uint8_t keyValue );
void  removeKeyValue( uint8_t keyValue );



// ----- Interrupt Functions -----

// Generates a constant external clock
ISR( TIMER1_COMPA_vect )
{
	if ( currentWaveState )
	{
		CLOCK_PORT &= ~(1 << CLOCK_PIN);
		currentWaveState--;
	}
	else
	{
		CLOCK_PORT |=  (1 << CLOCK_PIN);
		currentWaveState++;
	}
}



// ----- Functions -----

// Setup
inline void scan_setup()
{
	// Setup Timer Pulse (16 bit)

	// TODO Clock can be adjusted to whatever (read chip datasheets for limits)
	// 16 MHz / (2 * Prescaler * (1 + OCR1A)) = 1200.1 baud
	// Prescaler is 1
	// Twice every 1200 baud (actually 1200.1, timer isn't accurate enough)
	// This is close to 820 us, but a bit slower
	cli();
	TCCR1B = 0x09;
	OCR1AH = 0x01;
	OCR1AL = 0x09;
	TIMSK1 = (1 << OCIE1A);
	CLOCK_DDR = (1 << CLOCK_PIN);
	sei();


	// Initially buffer doesn't need to be cleared (it's empty...)
	BufferReadyToClear = 0;

	// InputSignal is off by default
	KeyIndex_Add_InputSignal = 0x00;

	// Reset the keyboard before scanning, we might be in a wierd state
	scan_resetKeyboard();
}


// Main Detection Loop
// Since this function is non-interruptable, we can do checks here on what stage of the
//  output clock we are at (0 or 1)
// We are looking for a start of packet
// If detected, all subsequent bits are then logged into a variable
// Once the end of the packet has been detected (always the same length), decode the pressed keys
inline uint8_t scan_loop()
{
	return 0;
}

void processKeyValue( uint8_t keyValue )
{
	// Interpret scan code
	switch ( keyValue )
	{
	case 0x00: // Break code from input?
		break;
	default:
		// Make sure the key isn't already in the buffer
		for ( uint8_t c = 0; c < KeyIndex_BufferUsed + 1; c++ )
		{
			// Key isn't in the buffer yet
			if ( c == KeyIndex_BufferUsed )
			{
				bufferAdd( keyValue );

				// Only send data if enabled
				if ( KeyIndex_Add_InputSignal )
					scan_sendData( KeyIndex_Add_InputSignal );
				break;
			}

			// Key already in the buffer
			if ( KeyIndex_Buffer[c] == keyValue )
				break;
		}
		break;
	}
}

void removeKeyValue( uint8_t keyValue )
{
	// Check for the released key, and shift the other keys lower on the buffer
	uint8_t c;
	for ( c = 0; c < KeyIndex_BufferUsed; c++ )
	{
		// Key to release found
		if ( KeyIndex_Buffer[c] == keyValue )
		{
			// Shift keys from c position
			for ( uint8_t k = c; k < KeyIndex_BufferUsed - 1; k++ )
				KeyIndex_Buffer[k] = KeyIndex_Buffer[k + 1];

			// Decrement Buffer
			KeyIndex_BufferUsed--;

			break;
		}
	}

	// Error case (no key to release)
	if ( c == KeyIndex_BufferUsed + 1 )
	{
		errorLED( 1 );
		char tmpStr[6];
		hexToStr( keyValue, tmpStr );
		erro_dPrint( "Could not find key to release: ", tmpStr );
	}
}

// Send data 
uint8_t scan_sendData( uint8_t dataPayload )
{
	return 0;
}

// Signal KeyIndex_Buffer that it has been properly read
void scan_finishedWithBuffer( void )
{
}

// Signal that the keys have been properly sent over USB
void scan_finishedWithUSBBuffer( void )
{
}

// Reset/Hold keyboard
// NOTE: Does nothing with the BETKB
void scan_lockKeyboard( void )
{
}

// NOTE: Does nothing with the BETKB
void scan_unlockKeyboard( void )
{
}

// Reset Keyboard
void scan_resetKeyboard( void )
{
	// TODO Determine the scan period, and the interval to scan each bit
}

