/* Copyright (C) 2013 by Jacob Alexander
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

// Make sure we haven't overflowed the buffer
#define bufferAdd(byte) \
		if ( KeyIndex_BufferUsed < KEYBOARD_BUFFER ) \
			KeyIndex_Buffer[KeyIndex_BufferUsed++] = byte



// ----- Variables -----

// Buffer used to inform the macro processing module which keys have been detected as pressed
volatile uint8_t KeyIndex_Buffer[KEYBOARD_BUFFER];
volatile uint8_t KeyIndex_BufferUsed;



// ----- Function Declarations -----

void processKeyValue( uint8_t valueType );
void  removeKeyValue( uint8_t keyValue );



// ----- Interrupt Functions -----

// UART Receive Buffer Full Interrupt
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
ISR(USART1_RX_vect)
#elif defined(_mk20dx128_) // ARM
void uart0_status_isr(void)
#endif
{
	cli(); // Disable Interrupts

	// Read part of the scan code (3 8bit chunks) from USART
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
	uint8_t tmp = UDR1;
#elif defined(_mk20dx128_) // ARM
	// TODO
	uint8_t tmp = 0;
#endif

	// Debug
	char tmpStr[6];
	hexToStr( tmp, tmpStr );
	dPrintStrs( tmpStr, " " ); // Debug

	// TODO

	sei(); // Re-enable Interrupts
}



// ----- Functions -----

// Setup
inline void scan_setup()
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
{
	// Setup the the USART interface for keyboard data input

	// TODO
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
#elif defined(_mk20dx128_) // ARM
{
	// Setup the the UART interface for keyboard data input

	// Setup baud rate
	// TODO

	// Reset the keyboard before scanning, we might be in a wierd state
	scan_resetKeyboard();
}
#endif


// Main Detection Loop
inline uint8_t scan_loop()
{
	return 0;
}

void processKeyValue( uint8_t keyValue )
{
	// TODO Process ASCII

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
	// Debug
	char tmpStr[6];
	hexToStr( dataPayload, tmpStr );
	info_dPrint( "Sending - ", tmpStr );

#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
	UDR1 = dataPayload;
#elif defined(_mk20dx128_) // ARM
	// TODO
#endif

	return 0;
}

// Signal KeyIndex_Buffer that it has been properly read
void scan_finishedWithBuffer( uint8_t sentKeys )
{
}

// Signal that the keys have been properly sent over USB
void scan_finishedWithUSBBuffer( uint8_t sentKeys )
{
}

// Reset/Hold keyboard
// NOTE: Does nothing with the FACOM6684
void scan_lockKeyboard( void )
{
}

// NOTE: Does nothing with the FACOM6684
void scan_unlockKeyboard( void )
{
}

// Reset Keyboard
void scan_resetKeyboard( void )
{
	// Not a calculated valued...
	_delay_ms( 50 );

	KeyIndex_BufferUsed = 0;
}

