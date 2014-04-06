/* Copyright (C) 2011-2014 by Jacob Alexander
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


// Known signals
static uint8_t cmd_clickOFF  = 0x0A; // Short beep, turns off clicker
static uint8_t cmd_clickON   = 0x04; // Long beep, turns on clicker
static uint8_t cmd_ACK_AA    = 0x10; // Keyboard will send ack (0xAA) back to PC

// Other known signals
// 0x02 turns on clicker but with short beep



// ----- Functions -----

// Setup
inline void Scan_setup()
{
	// Setup the the USART interface for keyboard data input

	// Setup baud rate
	// 16 MHz / ( 16 * Baud ) = UBRR
	// Baud <- 3.358 ms per bit, thus 1000 / 3.358 = 297.80
	// Thus USBRR = 3357
	uint16_t baud = 3357; // Max setting of 4095
	UBRR1H = (uint8_t)(baud >> 8);
	UBRR1L = (uint8_t)baud;

	// Enable the receiver, transitter, and RX Complete Interrupt
	UCSR1B = 0x98;

	// Set frame format: 8 data, no stop bits or parity
	// Asynchrounous USART mode
	// Kaypro sends ASCII codes (mostly standard) with 1 start bit and 8 data bits, with no trailing stop or parity bits
	UCSR1C = 0x06;
}


// Main Detection Loop
// Nothing is needed here for the Kaypro, but the function is available as part of the api to be called in a polling fashion
// TODO
//  - Add songs :D
inline uint8_t Scan_loop()
{
	// We *could* do extra offline processing here, but, it's not really needed for the Kaypro 1 keyboard
	return 0;
}

// USART Receive Buffer Full Interrupt
ISR(USART1_RX_vect)
{
	cli(); // Disable Interrupts

	// Get key from USART
	uint8_t keyValue = UDR1;

//#ifdef MAX_DEBUG
	// Debug print key
	char tmpStr1[6];
	hexToStr( keyValue, tmpStr1 );
	dPrintStrs( tmpStr1, " " );
//#endif

	// Add key(s) to processing buffer
	// First split out Shift and Ctrl
	//  Reserved Codes:
	//   Shift - 0xF5
	//   Ctrl  - 0xF6
	switch ( keyValue )
	{
	// - Ctrl Keys -
	// Exception keys
	case 0x08: // ^H
	case 0x09: // ^I
	case 0x0D: // ^M
	case 0x1B: // ^[
		Macro_bufferAdd( keyValue );
		break;
	// 0x40 Offset Keys
	// Add Ctrl key and offset to the lower alphabet
	case 0x00: // ^@
	case 0x1C: // "^\"
	case 0x1D: // ^]
	case 0x1E: // ^^
	case 0x1F: // ^_
		Macro_bufferAdd( 0xF6 );
		Macro_bufferAdd( keyValue + 0x40 );
		break;

	// - Add Shift key and offset to non-shifted key -
	// 0x10 Offset Keys
	case 0x21: // !
	case 0x23: // #
	case 0x24: // $
	case 0x25: // %
		Macro_bufferAdd( 0xF5 );
		Macro_bufferAdd( keyValue + 0x10 );
		break;
	// 0x11 Offset Keys
	case 0x26: // &
	case 0x28: // (
		Macro_bufferAdd( 0xF5 );
		Macro_bufferAdd( keyValue + 0x11 );
		break;
	// 0x07 Offset Keys
	case 0x29: // )
		Macro_bufferAdd( 0xF5 );
		Macro_bufferAdd( keyValue + 0x07 );
		break;
	// -0x0E Offset Keys
	case 0x40: // @
		Macro_bufferAdd( 0xF5 );
		Macro_bufferAdd( keyValue - 0x0E );
		break;
	// 0x0E Offset Keys
	case 0x2A: // *
		Macro_bufferAdd( 0xF5 );
		Macro_bufferAdd( keyValue + 0x0E );
		break;
	// 0x12 Offset Keys
	case 0x2B: // +
		Macro_bufferAdd( 0xF5 );
		Macro_bufferAdd( keyValue + 0x12 );
		break;
	// 0x05 Offset Keys
	case 0x22: // "
		Macro_bufferAdd( 0xF5 );
		Macro_bufferAdd( keyValue + 0x05 );
		break;
	// 0x01 Offset Keys
	case 0x3A: // :
		Macro_bufferAdd( 0xF5 );
		Macro_bufferAdd( keyValue + 0x01 );
		break;
	// -0x10 Offset Keys
	case 0x3C: // <
	case 0x3E: // >
	case 0x3F: // ?
		Macro_bufferAdd( 0xF5 );
		Macro_bufferAdd( keyValue - 0x10 );
		break;
	// -0x28 Offset Keys
	case 0x5E: // ^
		Macro_bufferAdd( 0xF5 );
		Macro_bufferAdd( keyValue - 0x28 );
		break;
	// -0x32 Offset Keys
	case 0x5F: // _
		Macro_bufferAdd( 0xF5 );
		Macro_bufferAdd( keyValue - 0x32 );
		break;
	// -0x20 Offset Keys
	case 0x7B: // {
	case 0x7C: // |
	case 0x7D: // }
		Macro_bufferAdd( 0xF5 );
		Macro_bufferAdd( keyValue - 0x20 );
		break;
	// -0x1E Offset Keys
	case 0x7E: // ~
		Macro_bufferAdd( 0xF5 );
		Macro_bufferAdd( keyValue - 0x1E );
		break;
	// All other keys
	default:
		// Ctrl Characters are from 0x00 to 0x1F, excluding:
		//  0x08 - Backspace
		//  0x09 - [Horizontal] Tab
		//  0x0D - [Carriage] Return
		//  0x1B - Escape
		//  0x7F - Delete (^?) (Doesn't need to be split out)

		// 0x60 Offset Keys
		// Add Ctrl key and offset to the lower alphabet
		if ( keyValue >= 0x00 && keyValue <= 0x1F )
		{
			Macro_bufferAdd( 0xF6 );
			Macro_bufferAdd( keyValue + 0x60 );
		}

		// Shift Characters are from 0x41 to 0x59
		//  No exceptions here :D
		// Add Shift key and offset to the lower alphabet
		else if ( keyValue >= 0x41 && keyValue <= 0x5A )
		{
			Macro_bufferAdd( 0xF5 );
			Macro_bufferAdd( keyValue + 0x20 );
		}

		// Everything else
		else
		{
			Macro_bufferAdd( keyValue );
		}
		break;
	}

	// Special keys - For communication to the keyboard
	// TODO Try to push this functionality into the macros...somehow
	switch ( keyValue )
	{
	case 0xC3: // Keypad Enter
		print("\n");
		info_print("BEEEEP! - Clicker on");
		scan_sendData( cmd_clickON );
		break;

	case 0xB2: // Keypad Decimal
		print("\n");
		info_print("BEEP! - Clicker off");
		scan_sendData( cmd_clickOFF );
		break;

	case 0x0A: // Line Feed
		print("\n");
		info_print("ACK!!");
		scan_sendData( cmd_ACK_AA );
		break;
	}

	sei(); // Re-enable Interrupts
}

// Send data
uint8_t Scan_sendData( uint8_t dataPayload )
{
	UDR1 = dataPayload;
	return 0;
}

// Signal KeyIndex_Buffer that it has been properly read
void Scan_finishedWithBuffer( uint8_t sentKeys )
{
}

// Signal that the keys have been properly sent over USB
void Scan_finishedWithUSBBuffer( uint8_t sentKeys )
{
}

// Reset/Hold keyboard
// NOTE: Does nothing with the BETKB
void Scan_lockKeyboard( void )
{
}

// NOTE: Does nothing with the BETKB
void Scan_unlockKeyboard( void )
{
}

// Reset Keyboard
void Scan_resetKeyboard( void )
{
}

