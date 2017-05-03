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
#define CLOCK_PORT PORTB
#define CLOCK_DDR   DDRB
#define CLOCK_PIN      0


// ----- Macros -----

#define setLED(id, status) \
		status = status ? 0 : 1; \
		scan_setLED( id, status )



// ----- Variables -----

// Buffer used to inform the macro processing module which keys have been detected as pressed
volatile uint8_t KeyIndex_Buffer[KEYBOARD_BUFFER];
volatile uint8_t KeyIndex_BufferUsed;

volatile uint8_t currentWaveState = 0;

volatile uint8_t calcLED      = 0;
volatile uint8_t insertLED    = 0;
volatile uint8_t shiftLockLED = 0;
volatile uint8_t schedLED     = 0;
volatile uint8_t drawLED      = 0;



// ----- Function Declarations -----

void Scan_diagnostics( void );
void processKeyValue( uint8_t keyValue );
void Scan_diagnostics( void );
void Scan_setRepeatStart( uint8_t n );
void Scan_readSwitchStatus( void );
void Scan_repeatControl( uint8_t on );
void Scan_enableKeyboard( uint8_t enable );
void Scan_setRepeatRate( uint8_t n );
void Scan_setLED( uint8_t ledNumber, uint8_t on );
void Scan_readLED( void );



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

// USART Receive Buffer Full Interrupt
ISR(USART1_RX_vect)
{
	cli(); // Disable Interrupts

	uint8_t keyValue = 0x00;

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



// ----- Functions -----

// Setup
inline void Scan_setup()
{
	// Setup Timer Pulse (16 bit)
	// 16 MHz / (2 * Prescaler * (1 + OCR1A)) = 1204.8 baud (820 us)
	// Prescaler is 1
	/*
	TCCR1B = 0x09;
	OCR1AH = 0x19;
	OCR1AL = 0xEF;
	TIMSK1 = (1 << OCIE1A);
	CLOCK_DDR = (1 << CLOCK_PIN);
	*/
	// 16 MHz / (2 * Prescaler * (1 + OCR1A)) = 1200.1 baud
	// Prescaler is 1
	// Twice every 1200 baud (actually 1200.1, timer isn't accurate enough)
	// This is close to 820 us, but a bit slower
	cli();
	TCCR1B = 0x09;
	OCR1AH = 0x1A;
	OCR1AL = 0x09;
	TIMSK1 = (1 << OCIE1A);
	CLOCK_DDR = (1 << CLOCK_PIN);


	// Setup the the USART interface for keyboard data input

	// Setup baud rate
	// 16 MHz / ( 16 * Baud ) = UBRR
	// Baud <- 1200 as per the spec (see datasheet archives), rounding to 1200.1 (as that's as accurate as the timer can be)
	// Thus UBRR = 833.26 -> round to 833
	uint16_t baud = 833; // Max setting of 4095
	UBRR1H = (uint8_t)(baud >> 8);
	UBRR1L = (uint8_t)baud;

	// Enable the receiver, transitter, and RX Complete Interrupt
	UCSR1B = 0x98;

	// Set frame format: 8 data, no stop bits or parity
	// Synchrounous USART mode
	// Tx Data on Falling Edge, Rx on Rising
	UCSR1C = 0x47;
	sei();

	// Reset the keyboard before scanning, we might be in a wierd state
	_delay_ms( 50 );
	scan_resetKeyboard();

	_delay_ms( 5000 ); // Wait for the reset command to finish enough for new settings to take hold afterwards
	scan_setRepeatRate( 0x00 ); // Set the fastest repeat rate
}


// Main Detection Loop
// Nothing is required here with the Epson QX-10 Keyboards as the interrupts take care of the inputs
inline uint8_t Scan_loop()
{
	return 0;
}

// TODO
void processKeyValue( uint8_t keyValue )
{
	// Detect LED Status
	uint8_t inputType = keyValue & 0xC0;

	// Determine the input type
	switch ( inputType )
	{
	// LED Status
	case 0xC0:
		// Binary Representation: 1100 llln
		// Hex Range: 0xC0 to 0xCF
		// - First 3 bits determine which LED (0 to 7)
		// - Last bit is whether the LED is On (1) or Off (0)
		// 000 - N/A (A)
		// 001 - N/A (B)
		// 010 - INSERT
		// 011 - SHIFT LOCK
		// 100 - N/A (C)
		// 101 - DRAW
		// 110 - SCHED
		// 111 - CALC
		break;

	// SW (Switch) Status
	case 0x80:
	{
		// Binary Representation: 1000 dddn
		// Hex Range: 0x80 to 0x8F
		// - First 3 bits determine which DB (KRTN) (See datasheet)
		// - Last bit is whether the key is enabled
		// 000 - N/A?
		// 001 - N/A?
		// 010 - Right SHIFT
		// 011 - Left SHIFT
		// 100 - N/A?
		// 101 - Left CTRL
		// 110 - GRPH SHIFT
		// 111 - Right CTRL

		// Detect Modifier Press/Release
		uint8_t press = keyValue & 0x01;

		// Modifier Press Detected
		if ( press )
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
		// Modifier Release Detected
		else
		{
			uint8_t actualKeyValue = keyValue | 0x01;

			// Check for the released key, and shift the other keys lower on the buffer
			uint8_t c;
			for ( c = 0; c < KeyIndex_BufferUsed; c++ )
			{
				// Key to release found
				if ( KeyIndex_Buffer[c] == actualKeyValue )
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
		break;
	}

	// Key code
	default:
		// Binary Representation: 0ddd pppp
		// Hex Range: 0x00 to 0x7F
		// - First 3 bits determine which DB (KRTN) (See datasheet)
		// - Last 4 bits corresond to the KSC signals (P13, P12, P11, P10 respectively)
		// Or, that can be read as, each key has it's own keycode (with NO release code)
		// Modifiers are treated differently

		// Add the key to the buffer, if it isn't already in the current Key Buffer
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
		// Special Internal Key Mapping/Functions
		switch ( keyValue )
		{
		// LED Test
		case 0x0A: // CALC
			setLED( 0x07, calcLED ); // 0x4F
			break;
		case 0x0B: // SCHED
			setLED( 0x0E, schedLED ); // 0x5D
			break;
		case 0x0C: // DRAW
			setLED( 0x0D, drawLED ); // 0x5B
			break;
		case 0x42: // SHIFT LOCK
			setLED( 0x0B, shiftLockLED ); // 0x57
			break;
		case 0x5E: // INSERT
			setLED( 0x02, insertLED ); // 0x45
			break;

		/*
		// TEST
		case 0x51:
			scan_resetKeyboard();
			break;
		case 0x52:
			scan_diagnostics();
			break;
		case 0x53:
			scan_setRepeatStart( 0x00 );
			break;
		case 0x54:
			scan_readSwitchStatus();
			break;
		case 0x55:
			scan_repeatControl( 0x00 );
			break;
		case 0x56:
			scan_repeatControl( 0x01 );
			break;
		case 0x57:
			scan_enableKeyboard( 0x00 );
			break;
		case 0x58:
			scan_enableKeyboard( 0x01 );
			break;
		case 0x59:
			scan_setRepeatRate( 0x00 );
			break;
		case 0x5A:
			scan_readLED();
			break;
		*/
		}
		break;
	}
}

// Send data
// See below functions for the input sequences for the Epson QX-10 Keyboard
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
inline void Scan_finishedWithBuffer( uint8_t sentKeys )
{
	return;
}

// Signal that the keys have been properly sent over USB
// For the Epson QX-10 only the modifier keys have release signals
// Therefore, only 5 keys could possibly be assigned as a modifiers
// The rest of the keys are single press (like the Kaypro keyboards)
//
// However, this differentiation causes complications on how the key signals are discarded and used
// The single keypresses must be discarded immediately, while the modifiers must be kept
inline void Scan_finishedWithUSBBuffer( uint8_t sentKeys )
{
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

	/* Non-working, too slow (too much traffic on the bus)
	// Poll the modifiers using an input command
	uint8_t oldBuffer = KeyIndex_BufferUsed;
	KeyIndex_BufferUsed = 0;
	if ( oldBuffer )
		scan_readSwitchStatus();
	*/
}

// Reset/Hold keyboard
// Warning! This will cause the keyboard to not send any data, so you can't disable with a keypress
// The Epson QX-10 Keyboards have a command used to lock the keyboard output
void Scan_lockKeyboard( void )
{
	scan_enableKeyboard( 0x00 );
}

void Scan_unlockKeyboard( void )
{
	scan_enableKeyboard( 0x01 );
}

// Reset Keyboard
// Does the following
// - Clears the keycode buffer (32 characters)
// - Validates repeat function (what does this do?)
// - Sets repeat start time (500 ms)
// - Sets repeat interval (50 ms)
// - Turns off all LEDs
void Scan_resetKeyboard( void )
{
	// Reset command for the QX-10 Keyboard
	scan_sendData( 0xE0 );

	// Empty buffer, now that keyboard has been reset
	KeyIndex_BufferUsed = 0;
}

// TODO Check
// Runs Diagnostics on the keyboard
// - First does a reset (see Scan_resetKeyboard)
// - Blinks all of the LEDs one after another
// - Outputs 0x00 if no keys are pressed
// - Outputs 0xFF if any keys are being pressed
void Scan_diagnostics( void )
{
	// Send reset command with diagnositics
	scan_sendData( 0xE7 );
}

// TODO Check
// Set Repeat Interval Start
// 300 ms + n * 25 ms
// Interval after which to start the repeated keys
void Scan_setRepeatStart( uint8_t n )
{
	// Send command
	// Binary Representation: 000n nnnn
	// Hex boundaries 0x00 to 0x1F
	// 300 ms to 1075 ms (intervals of 25 ms)
	scan_sendData( n );
}

// Read Switch Status (preferential to actual keypress outputs)
// 000 - N/A?
// 001 - N/A?
// 010 - Right SHIFT
// 011 - Left SHIFT
// 100 - N/A?
// 101 - Left CTRL
// 110 - GRPH SHIFT
// 111 - Right CTRL
void Scan_readSwitchStatus( void )
{
	scan_sendData( 0x80 );
}

// TODO Check
// Repeat Control
// 0x00 Stops repeat function
// 0x01 Enables repeat function
void Scan_repeatControl( uint8_t on )
{
	// Send command
	// Binary Representation: 101X XXXn
	// Hex options: 0xA0 or 0xA1
	scan_sendData( 0xA0 | on );
}

// TODO Check
// Enable Sending Keyboard Data
// 0x00 Stops keycode transmission
// 0x01 Enables keycode transmission
void Scan_enableKeyboard( uint8_t enable )
{
	// Send command
	// Binary Representation: 110X XXXn
	// Hex options: 0xC0 or 0xC1
	scan_sendData( 0xC0 | enable );
}

// Set Repeat Interval
// 30 ms + n * 5 ms
// Period between sending each repeated key after the initial interval
void Scan_setRepeatRate( uint8_t n )
{
	// Send command
	// Binary Representation: 001n nnnn
	// Hex options: 0x00 to 0x1F
	// 30 ms to 185 ms (intervals of 5 ms)
	scan_sendData( 0x20 | n );
}

// Turn On/Off LED
// 0x00 LED Off
// 0x01 LED On
//
// 8 LEDs max (Note: 5 connected on my board, there is 1 position empty on the PCB for a total of 6)
// 0 to 7 (0x0 to 0x7)
void Scan_setLED( uint8_t ledNumber, uint8_t on )
{
	// Send command
	// Binary Representation: 010l llln
	// Hex options: 0x40 to 0x4F
	// The spec is NOT accurate (especially about the "don't care" bit)
	// llll n - Usage
	// 0000 X - N/A (1)
	// 0001 X - N/A (2)
	// 0010 1 - INSERT On
	// 0011 0 - SHIFT LOCK Off
	// 0100 X - N/A (3)
	// 0101 0 - DRAW Off
	// 0110 0 - SCHED Off
	// 0111 1 - CALC On
	// 1000 X - N/A (1)
	// 1001 X - N/A (2)
	// 1010 0 - INSERT Off
	// 1011 1 - SHIFT LOCK On
	// 1100 X - N/A (3)
	// 1101 1 - DRAW On
	// 1110 1 - SCHED On
	// 1111 0 - CALC Off

	uint8_t off = 0;
	if ( !on )
	{
		off = 0x10;
	}
	scan_sendData( ( 0x40 | (ledNumber << 1) | on ) ^ off );
}

// Read LED Status
// High priority data output (may overwrite some keycode data)
void Scan_readLED( void )
{
	scan_sendData( 0x7F );
}

