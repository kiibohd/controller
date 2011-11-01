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

// ----- Includes -----

// AVR Includes
#include <avr/interrupt.h>
#include <avr/io.h>

// Project Includes
#include <led.h>
#include <print.h>

// Local Includes
#include "scan_loop.h"



// ----- Defines -----



// ----- Macros -----



// ----- Variables -----

uint8_t KeyIndex_Array[KEYBOARD_SIZE + 1];


// Known signals
static uint8_t cmd_clickOFF  = 0x0A; // Short beep, turns off clicker
static uint8_t cmd_clickON   = 0x04; // Long beep, turns on clicker
static uint8_t cmd_ACK_AA    = 0x10; // Keyboard will send ack (0xAA) back to PC

// Other known signals
// 0x02 turns on clicker but with short beep



// ----- Functions -----

// Setup
inline void scan_setup()
{
	// Setup the the USART interface for keyboard data input
	
	// Setup baud rate
	// 16 MHz / ( 16 * Baud ) = UBRR
	// Baud <- 3.358 ms per bit, thus 1000 / 3.358 = 297.80
	// Thus baud = 3357
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
inline uint8_t scan_loop()
{
	/*
	// Packet Read
	if ( packet_index == 8 )
	{
		// Disable Error LED, proper key found
		errorLED( 0 );

//#ifdef MAX_DEBUG
		// Crazy Debug (Read the Scan Code)
		char tmpStr[3];
		hexToStr_op( inputData, tmpStr, 2 );
		dPrintStrsNL( "Read Data: 0x", tmpStr );
//#endif
		// - Map the scan code to the index array -
		// If the 8th bit is high, remove the keypress, else, add the keypress
		// The lower 7 bits are the array index
		KeyIndex_Array[(inputData & 0x7F)] = (inputData & 0x80) ? 0x00 : 0x80;

		// Reset Containers
		packet_index = 0;
		inputData = 0xFF;
	}
	// Bad Packet
	else if ( packet_index > 8 )
	{
		// Signal Error
		errorLED( 1 );

		char tmpStr[3];
		int8ToStr( packet_index, tmpStr );
		erro_dPrint( "Big packet? Mismatched... ", tmpStr );

		packet_index = 0;
		inputData = 0xFF;
	}
	*/
	/*
	// Disable keyboard interrupt (does nothing if already off)
	UNSET_INTR();

	// Read the clock 8 times
	if ( READ_CLK )
	{
		// Mis-read packet, set back to 0
		if ( packet_index == -1 )
			packet_index = 0;

		// Append 1 bit of data
		inputData &= ~(READ_DATA << packet_index);
		packet_index++;

		// 8 Bits have been read
		if ( packet_index == 8 )
		{
			// Wait till clock edge falls
			while ( READ_CLK );

			// Sample both lines to make sure this is not a data value
			//  and definitely the end of packet data blip
			uint16_t badDataCounter = 0;
			while ( !( READ_DATA ) && !( READ_CLK ) )
					badDataCounter++;

			if ( badDataCounter < 25 )
			{
//#ifdef MAX_DEBUG
				// Crazy Debug (Read the Scan Code)
				char tmpStr[3];
				hexToStr_op( inputData, tmpStr, 2 );
				dbug_dPrint( "Read Data: 0x", tmpStr );
//#endif
				// - Map the scan code to the index array -
				// If the 8th bit is high, remove the keypress, else, add the keypress
				// The lower 7 bits are the array index
				KeyIndex_Array[(inputData & 0x7F)] = (inputData & 0x80) ? 0x00 : 0x80;
			}
			// Even though this is a mis-read packet, we still know what the value is
			else
			{
				// Signal Error
				errorLED( 1 );
				char tmpStr[3];
				hexToStr_op( inputData, tmpStr, 2 );
				erro_dPrint( "Bad packet? Mismatched... 0x", tmpStr );
			}

			// Reset Containers
			inputData = 0xFF;
			packet_index = 0;

			// Interrupt the keyboard, so we don't get packet pieces...
			SET_INTR();

			// Do not wait for next clock, let USB do it's thing (if desired)
			return packet_index;
		}

		// Wait till clock edge falls
		while ( READ_CLK );
	}

	// Interrupt keyboard if there is no pending packet
	SET_INTR();
	*/
	return 0;
}

// USART Receive Buffer Full Interrupt
ISR(USART1_RX_vect)
{
	cli(); // Disable Interrupts

	uint8_t keyValue = UDR1;
	char tmpStr1[6];
	hexToStr( keyValue, tmpStr1 );
	dPrintStrs( tmpStr1, " " );

	// Special keys - For communication to the keyboard
	// TODO Try to push this functionality into the macros...somehow
	switch ( keyValue )
	{
	case 0xC3: // Keypad Enter
		print("\n");
		info_print("BEEEEP! - Clicker on");
		UDR1 = cmd_clickON;
		break;

	case 0xB2: // Keypad Decimal
		print("\n");
		info_print("BEEP! - Clicker off");
		UDR1 = cmd_clickOFF;
		break;

	case 0x0A: // Line Feed
		print("\n");
		info_print("ACK!!");
		UDR1 = cmd_ACK_AA;
		break;
	}

	// Add key to processing buffer

	sei(); // Re-enable Interrupts
}

