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



// ----- Interrupt Functions -----

// USART Receive Buffer Full Interrupt
ISR(USART1_RX_vect)
{
	cli(); // Disable Interrupts

	uint8_t keyValue = 0x00;

	// The interrupt is always for the first item of the packet set, reset the buffer
	KeyIndex_BufferUsed = 0;

	// Only the first 7 bits have scancode data
	// The last packet of the packet set has the 8th bit high, all the others are low
	//
	// Interrupts are too slow for the rest of the packet set, poll for the rest
	while ( 1 )
	{
		// Read the raw packet from the USART
		keyValue = UDR1;

		// Debug
		char tmpStr[6];
		hexToStr( keyValue, tmpStr );
		dPrintStrs( tmpStr, " " );

		// Process the scancode
		processKeyValue( keyValue );

		// Last packet of the set
		if ( keyValue & 0x80 )
		{
			dPrintStrs( "**" );
			break;
		}

		// Delay enough so we don't run into the same packet (or the previous buffered packet)
		_delay_us(10000);
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
	// Finalize output buffer
	// Mask 8th bit
	keyValue &= 0x7F;

	// Interpret scan code
	switch ( keyValue )
	{
	case 0x40: // Clear buffer command
		info_print("CLEAR!");

		BufferReadyToClear = 1;
		break;
	case 0x7F:
		scan_lockKeyboard();
		_delay_ms(3000);
		scan_unlockKeyboard();

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
	/*
	uint8_t foundModifiers = 0;

	// Look for all of the modifiers present, there is a max of 8 (but only keys for 5 on the HASCI version)
	for ( uint8_t c = 0; c < KeyIndex_BufferUsed; c++ )
	{
		// The modifier range is from 0x80 to 0x8F (well, the last bit is the ON/OFF signal, but whatever...)
		if ( KeyIndex_Buffer[c] <= 0x8F && KeyIndex_Buffer[c] >= 0x80 )
		{
			// Add the modifier back into the the Key Buffer
			KeyIndex_Buffer[foundModifiers] = KeyIndex_Buffer[c];
			foundModifiers++;
		}
	}

	// Adjust the size of the new Key Buffer
	KeyIndex_BufferUsed = foundModifiers;
	*/
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

