/* Copyright (C) 2013-2014 by Jacob Alexander
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


// ----- Macros -----



// ----- Variables -----

// Buffer used to inform the macro processing module which keys have been detected as pressed
volatile uint8_t KeyIndex_Buffer[KEYBOARD_BUFFER];
volatile uint8_t KeyIndex_BufferUsed;

volatile uint8_t KeyBufferRemove[6];
volatile uint8_t KeyBufferRemoveCount = 0;

static uint8_t KeyBuffer[3];
volatile static uint8_t KeyBufferCount = 0;



// ----- Function Declarations -----

void processKeyValue( uint8_t valueType, uint8_t keyValue );
void  removeKeyValue( uint8_t keyValue );



// ----- Interrupt Functions -----

// USART Receive Buffer Full Interrupt
ISR(USART1_RX_vect)
{
	cli(); // Disable Interrupts

	// Read part of the scan code (3 8bit chunks) from USART
	KeyBuffer[KeyBufferCount] = UDR1;

	if ( KeyBufferCount >= 2 )
	{
		// Debug
		for ( uint8_t c = 0; c <= 2; c++ )
		{
			// Debug
			char tmpStr[6];
			hexToStr( KeyBuffer[c], tmpStr );
			dPrintStrs( tmpStr, " " ); // Debug
		}
		print("\n");

		processKeyValue( KeyBuffer[1], KeyBuffer[2] );

		KeyBufferCount = 0;
	}
	else
	{
		KeyBufferCount++;
	}

	sei(); // Re-enable Interrupts
}



// ----- Functions -----

// Setup
inline void Scan_setup()
{
	// Setup the the USART interface for keyboard data input

	// Setup baud rate
	// 16 MHz / ( 16 * Baud ) = UBRR
	// Baud: 4817 -> 16 MHz / ( 16 * 4817 ) = 207.5981
	// Thus baud setting = 208
	uint16_t baud = 208; // Max setting of 4095
	UBRR1H = (uint8_t)(baud >> 8);
	UBRR1L = (uint8_t)baud;

	// Enable the receiver, transmitter, and RX Complete Interrupt
	UCSR1B = 0x98;

	// Set frame format: 8 data, 1 stop bit, odd parity
	// Asynchrounous USART mode
	UCSR1C = 0x36;

	// Reset the keyboard before scanning, we might be in a wierd state
	scan_resetKeyboard();
}


// Main Detection Loop
inline uint8_t Scan_loop()
{
	// Remove any "released keys", this is delayed due to buffer release synchronization issues
	for ( uint8_t c = 0; c < KeyBufferRemoveCount; c++ )
	{
		removeKeyValue( KeyBufferRemove[c] );
	}

	KeyBufferRemoveCount = 0;

	return 0;
}

void processKeyValue( uint8_t valueType, uint8_t keyValue )
{
	switch ( valueType )
	{
	// Single Key Press
	case 0x00:
		break;
	// Repeated Key Press
	case 0x01:
		break;
	// Modifier Key Release
	case 0x02:
		KeyBufferRemove[KeyBufferRemoveCount++] = keyValue;
		return;
	}

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
uint8_t Scan_sendData( uint8_t dataPayload )
{
	// Debug
	char tmpStr[6];
	hexToStr( dataPayload, tmpStr );
	info_dPrint( "Sending - ", tmpStr );

	UDR1 = dataPayload;

	return 0;
}

// Signal KeyIndex_Buffer that it has been properly read
void Scan_finishedWithBuffer( uint8_t sentKeys )
{
	// Make sure we aren't in the middle of a receiving a new scancode
	while ( KeyBufferCount != 0 );

	cli(); // Disable Interrupts

	// Count for number of modifiers to maintain in the buffer
	uint8_t filled = 0;
	uint8_t latched = 0;
	uint8_t latchBuffer[13]; // There are only 13 keys that can possibly be latched at the same time...
	uint8_t normal = 0;
	uint8_t prevBuffer = KeyIndex_BufferUsed;

	// Clean out all keys except "special" keys (designated modifiers)
	uint8_t key;
	for ( key = 0; key < sentKeys; key++ )
	{
		switch ( KeyIndex_Buffer[key] )
		{
		// Dedicated Modifier Keys
		// NOTE: Both shifts are represented as the same scan code
		case 0x04:
		case 0x05:
		case 0x12:
			KeyIndex_Buffer[filled++] = KeyIndex_Buffer[key];
			break;
		// Latched Keys, only released if a non-modifier is pressed along with it
		// NOTE: This keys do not have a built in repeating
		case 0x00:
		case 0x01:
		case 0x03:
		//case 0x0B: // XXX Being used as an alternate Enter, since it is labelled as such
		case 0x22:
		case 0x10:
		case 0x11:
		case 0x20:
		case 0x21:
		case 0x30:
		case 0x31:
		case 0x40:
		//case 0x41: // XXX Being used as ESC
			latchBuffer[latched++] = KeyIndex_Buffer[key];
			break;
		// Allow the scancode to be removed, normal keys
		default:
			normal++;
			break;
		}
	}

	// Reset the buffer counter
	KeyIndex_BufferUsed = filled;

	// Add back lost keys, so they are processed on the next USB send
	for ( ; key < prevBuffer; key++ )
	{
		Macro_bufferAdd( KeyIndex_Buffer[key] );
		info_print("Re-appending lost key after USB send...");
	}

	// Only "re-add" the latched keys if they weren't used
	if ( latched > 0 && normal == 0 )
	{
		for ( uint8_t c = 0; c < latched; c++ )
		{
			Macro_bufferAdd( latchBuffer[c] );
		}
	}

	sei(); // Re-enable Interrupts
}

// Signal that the keys have been properly sent over USB
void Scan_finishedWithUSBBuffer( uint8_t sentKeys )
{
}

// Reset/Hold keyboard
// NOTE: Does nothing with the FACOM6684
void Scan_lockKeyboard( void )
{
}

// NOTE: Does nothing with the FACOM6684
void Scan_unlockKeyboard( void )
{
}

// Reset Keyboard
void Scan_resetKeyboard( void )
{
	// Not a calculated valued...
	_delay_ms( 50 );

	KeyBufferCount = 0;
	KeyBufferRemoveCount = 0;
	KeyIndex_BufferUsed = 0;
}

