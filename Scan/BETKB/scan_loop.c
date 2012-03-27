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
#define RESET_PORT PORTB
#define RESET_DDR   DDRD
#define RESET_PIN      0


// ----- Macros -----

// Make sure we haven't overflowed the buffer
#define bufferAdd(byte) \
		if ( KeyIndex_BufferUsed < KEYBOARD_BUFFER ) \
			KeyIndex_Buffer[KeyIndex_BufferUsed++] = byte



// ----- Variables -----

// Buffer used to inform the macro processing module which keys have been detected as pressed
volatile uint8_t KeyIndex_Buffer[KEYBOARD_BUFFER];
volatile uint8_t KeyIndex_BufferUsed;


// Buffer Signals
volatile uint8_t BufferReadyToClear;



// ----- Function Declarations -----

void processKeyValue( uint8_t keyValue );
void  removeKeyValue( uint8_t keyValue );



// ----- Interrupt Functions -----

// USART Receive Buffer Full Interrupt
ISR(USART1_RX_vect)
{
	cli(); // Disable Interrupts

	uint8_t keyValue = 0x00;
	uint8_t keyState = 0x00;

	// Read the scancode packet from the USART (1st to 8th bits)
	keyValue = UDR1;

	// Read the release/press bit (9th bit) XXX Unnecessary, and wrong it seems, parity bit? or something else?
	keyState = UCSR1B & 0x02;

	// High bit of keyValue, also represents press/release
	keyState = keyValue & 0x80 ? 0x00 : 0x02;

	// Debug
	char tmpStr[6];
	hexToStr( keyValue & 0x7F, tmpStr );

	// Process the scancode
	switch ( keyState )
	{
	case 0x00: // Released
		dPrintStrs( tmpStr, "R  " ); // Debug

		// Remove key from press buffer
		removeKeyValue( keyValue & 0x7F );
		break;

	case 0x02: // Pressed
		dPrintStrs( tmpStr, "P " ); // Debug

		// New key to process
		processKeyValue( keyValue & 0x7F );
		break;
	}

	sei(); // Re-enable Interrupts
}



// ----- Functions -----

// Setup
inline void scan_setup()
{
	// Setup the the USART interface for keyboard data input
	// NOTE: The input data signal needs to be inverted for the Teensy USART to properly work
	
	// Setup baud rate
	// 16 MHz / ( 16 * Baud ) = UBRR
	// Baud <- 0.823284 ms per bit, thus 1000 / 0.823284 = 1214.65004 -> 823.2824
	// Thus baud setting = 823
	uint16_t baud = 823; // Max setting of 4095
	UBRR1H = (uint8_t)(baud >> 8);
	UBRR1L = (uint8_t)baud;

	// Enable the receiver, transitter, and RX Complete Interrupt as well as 9 bit data
	UCSR1B = 0x9C;

	// Set frame format: 9 data, 1 stop bit, no parity
	// Asynchrounous USART mode
	UCSR1C = 0x06;

	// Initially buffer doesn't need to be cleared (it's empty...)
	BufferReadyToClear = 0;

	// Reset the keyboard before scanning, we might be in a wierd state
	scan_resetKeyboard();
}


// Main Detection Loop
// Not needed for the BETKB, this is just a busy loop
inline uint8_t scan_loop()
{
	return 0;
}

// TODO
void processKeyValue( uint8_t keyValue )
{
	// Interpret scan code
	switch ( keyValue )
	{
	default:
		// Make sure the key isn't already in the buffer
		for ( uint8_t c = 0; c < KeyIndex_BufferUsed + 1; c++ )
		{
			// Key isn't in the buffer yet
			if ( c == KeyIndex_BufferUsed )
			{
				bufferAdd( keyValue );
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
	UDR1 = dataPayload;
	return 0;
}

// Signal KeyIndex_Buffer that it has been properly read
// TODO
void scan_finishedWithBuffer( void )
{
	return;
}

// Signal that the keys have been properly sent over USB
// TODO
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
// TODO?
// - Holds the input read line high to flush the buffer
// - This does not actually reset the keyboard, but always seems brings it to a sane state
// - Won't work fully if keys are being pressed done at the same time
void scan_resetKeyboard( void )
{
	// Initiate data request line, but don't read the incoming data
	//REQUEST_DATA(); TODO

	// Not a calculated valued...
	_delay_ms( 50 );

	// Stop request line
	//STOP_DATA(); TODO
}

