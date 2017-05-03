/* Copyright (C) 2011,2014 by Jacob Alexander
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

// Compiler Includes
#include <Lib/ScanLib.h>

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

#define UNSET_RESET()   RESET_DDR &= ~(1 << RESET_PIN)
#define   SET_RESET()   RESET_DDR |=  (1 << RESET_PIN)



// ----- Variables -----

// Buffer used to inform the macro processing module which keys have been detected as pressed
volatile uint8_t KeyIndex_Buffer[KEYBOARD_BUFFER];
volatile uint8_t KeyIndex_BufferUsed;


// Buffer Signals
volatile uint8_t BufferReadyToClear;



// ----- Functions -----

// Setup
inline void Scan_setup()
{
	// Setup the the USART interface for keyboard data input
	// NOTE: The input data signal needs to be inverted for the Teensy USART to properly work

	// Setup baud rate
	// 16 MHz / ( 16 * Baud ) = UBRR
	// Baud <- 0.82020 ms per bit, thus 1000 / 0.82020 = 1219.2
	// Thus baud setting = 820
	uint16_t baud = 820; // Max setting of 4095
	UBRR1H = (uint8_t)(baud >> 8);
	UBRR1L = (uint8_t)baud;

	// Enable the receiver, transitter, and RX Complete Interrupt
	UCSR1B = 0x98;

	// Set frame format: 8 data, no stop bits or parity
	// Asynchrounous USART mode
	// 8304 sends encoded scancodes (for example Alphanumeric 0-9 follows their keypad encoding scheme)
	// Separate line is for reset
	UCSR1C = 0x06;

	// Initially buffer doesn't need to be cleared (it's empty...)
	BufferReadyToClear = 0;

	// Reset the keyboard before scanning, we might be in a wierd state
	// Note: This should be run asap, but we need the USART setup to run this command on the 8304
	scan_resetKeyboard();
}


// Main Detection Loop
// Not needed for the Micro Switch 8304, this is just a busy loop
inline uint8_t Scan_loop()
{
	return 0;
}

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
				Macro_bufferAdd( keyValue );
				break;
			}

			// Key already in the buffer
			if ( KeyIndex_Buffer[c] == keyValue )
				break;
		}
		break;
	}
}

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

// Send data
//
// Keyboard Input Guide for Micro Switch 8304
// 0xBX is for LED F1,F2,Over Type,Lock
// 0xAX is for LED F3,F8,F9,F10
// 0x92 resets keyboard (LED off, echo scancode mode off)
// 0x9E sets echo scancode mode from (0x81 to 0xFF; translates to 0x01 to 0x7F)
// Other echos: 0x15~0x19 send 0x15~0x19, 0x40 sends 0x40 (as well as 0x44,0x45, 0x80)
// 0x8C Acks the keyboard and gets 0x70 sent back (delayed)
uint8_t Scan_sendData( uint8_t dataPayload )
{
	UDR1 = dataPayload;
	return 0;
}

// Signal KeyIndex_Buffer that it has been properly read
// In the case of the Micro Switch 8304, we leave the buffer alone until more scancode data comes in
void Scan_finishedWithBuffer( uint8_t sentKeys )
{
	// We received a Clear code from the 8304, clear the buffer now that we've used it
	if ( BufferReadyToClear )
	{
		KeyIndex_BufferUsed = 0;
		BufferReadyToClear = 0;
	}
}

// Signal that the keys have been properly sent over USB
void Scan_finishedWithUSBBuffer( uint8_t sentKeys )
{
}

// Reset/Hold keyboard
// Warning! This will cause the keyboard to not send any data, so you can't disable with a keypress
// The Micro Switch 8304 has a dedicated reset line
void Scan_lockKeyboard( void )
{
	UNSET_RESET();
}

void Scan_unlockKeyboard( void )
{
	SET_RESET();
}

// Reset Keyboard
void Scan_resetKeyboard( void )
{
	// Reset command for the 8304
	scan_sendData( 0x92 );
}

