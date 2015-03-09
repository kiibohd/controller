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
#define SPKR_PORT PORTD
#define SPKR_DDR   DDRD
#define SPKR_POS      1

#define POWR_PORT PORTC
#define POWR_DDR   DDRC
#define POWR_POS      7



// ----- Macros -----



// ----- Variables -----

// Buffer used to inform the macro processing module which keys have been detected as pressed
volatile uint8_t KeyIndex_Buffer[KEYBOARD_BUFFER];
volatile uint8_t KeyIndex_BufferUsed;



// ----- Functions -----

// Setup
inline void Scan_setup()
{
	// Setup the the USART interface for keyboard data input

	// Setup baud rate
	// 16 MHz / ( 16 * Baud ) = UBRR
	// Baud <- 0.10450 ms per bit, thus 1000 / 0.10450 = 9569.4
	// Thus UBRR = 104.50
	// To deal with the 0.5, setting to double speed, which means UBRR = 209
	uint16_t baud = 209; // Max setting of 4095
	UBRR1H = (uint8_t)(baud >> 8);
	UBRR1L = (uint8_t)baud;

	// Enable Double Read Speed
	UCSR1A = 0x02;

	// Enable the receiver, transitter, and RX Complete Interrupt
	UCSR1B = 0x98;

	// Set frame format: 8 data, no stop bits or parity
	// Asynchrounous USART mode
	UCSR1C = 0x06;

	// Set Speaker Pin to Pull-Up gives a low-volume click (XXX no other setting does, why?)
	SPKR_DDR  &= ~(1 << SPKR_POS);
	SPKR_PORT |= (1 << SPKR_POS);

	// Set Power Pin (I've traced this back to the "Power On" Switch, but I don't really know what it's for)
	// Configured as a Pull-up Input - This pin "can" be read as well, it will go to GND when the "Power On" switch is pressed, and will read ~5V otherwise
	// XXX Currently not used by the controller
	POWR_DDR  &= ~(1 << POWR_POS);
	POWR_PORT |= (1 << POWR_POS);

	// Reset the keyboard before scanning, we might be in a wierd state
	scan_resetKeyboard();
}

// Main Detection Loop
// Not needed for the Sony NEWS, this is just a busy loop
inline uint8_t Scan_loop()
{
	return 0;
}

void processKeyValue( uint8_t keyValue )
{
	// Detect release condition
	uint8_t release = keyValue & 0x80;

	// Finalize output buffer
	// Mask 8th bit
	keyValue &= 0x7F;

	// Key Release
	if ( release )
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
	// Press or Repeated Key
	else
	{
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
	}
}

// USART Receive Buffer Full Interrupt
ISR(USART1_RX_vect)
{
	cli(); // Disable Interrupts

	uint8_t keyValue = 0x00;

	// One scancode at a time (fastest interval ~3.95 ms - recorded, should still be ok for interrupt polling)
	// Read the raw packet from the USART
	keyValue = UDR1;

	// Debug
	char tmpStr[6];
	hexToStr( keyValue, tmpStr );
	dPrintStrs( tmpStr, " " );

	// Process the scancode
	if ( keyValue != 0x00 )
		processKeyValue( keyValue );

	sei(); // Re-enable Interrupts
}

// Send data to keyboard
uint8_t Scan_sendData( uint8_t dataPayload )
{
	// Debug
	char tmpStr[6];
	hexToStr( dataPayload, tmpStr );
	info_dPrint( tmpStr, " " );

	UDR1 = dataPayload;
	return 0;
}

// Signal KeyIndex_Buffer that it has been properly read
// Not needed as a signal is sent to remove key-presses
void Scan_finishedWithBuffer( uint8_t sentKeys )
{
	return;
}

// Reset/Hold keyboard TODO
// Warning! This will cause the keyboard to not send any data, so you can't disable with a keypress
void Scan_lockKeyboard( void )
{
}

void Scan_unlockKeyboard( void )
{
}

// Reset Keyboard
void Scan_resetKeyboard( void )
{
	// Empty buffer, now that keyboard has been reset
	KeyIndex_BufferUsed = 0;
}

void Scan_finishedWithUSBBuffer( uint8_t sentKeys )
{
	return;
}

