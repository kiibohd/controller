/* Copyright (C) 2012,2014 by Jacob Alexander
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
#define DATA_PORT PORTC
#define DATA_DDR   DDRC
#define DATA_PIN      7
#define DATA_OUT   PINC

#define CLOCK_PORT PORTC
#define CLOCK_DDR   DDRC
#define CLOCK_PIN      6

#define RESET_PORT PORTF
#define RESET_DDR   DDRF
#define RESET_PIN      7


// ----- Macros -----



// ----- Variables -----

// Buffer used to inform the macro processing module which keys have been detected as pressed
volatile uint8_t KeyIndex_Buffer[KEYBOARD_BUFFER];
volatile uint8_t KeyIndex_BufferUsed;
volatile uint8_t KeyIndex_Add_InputSignal; // Used to pass the (click/input value) to the keyboard for the clicker

volatile uint8_t currentWaveState = 0;
volatile uint8_t positionCounter = 0;

volatile uint8_t statePositionCounter = 0;
volatile uint16_t stateSamplesTotal = 0;
volatile uint16_t stateSamples = 0;


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
		currentWaveState--; // Keeps track of the clock value (for direct clock output)
		statePositionCounter = positionCounter;
		positionCounter++;  // Counts the number of falling edges, reset is done by the controlling section (reset, or main scan)
	}
	else
	{
		CLOCK_PORT |=  (1 << CLOCK_PIN);
		currentWaveState++;
	}
}



// ----- Functions -----

// Setup
inline void Scan_setup()
{
	// Setup Timer Pulse (16 bit)

	// TODO Clock can be adjusted to whatever (read chip datasheets for limits)
	// This seems like a good scan speed, as there don't seem to be any periodic
	//  de-synchronization events, and is fast enough for scanning keys
	// Anything much more (100k baud), tends to cause a lot of de-synchronization
	// 16 MHz / (2 * Prescaler * (1 + OCR1A)) = 10k baud
	// Prescaler is 1
	cli();
	TCCR1B = 0x09;
	OCR1AH = 0x03;
	OCR1AL = 0x1F;
	TIMSK1 = (1 << OCIE1A);

	CLOCK_DDR |= (1 << CLOCK_PIN); // Set the clock pin as an output
	DATA_PORT |= (1 << DATA_PIN);  // Pull-up resistor for input the data line
	sei();


	// Initially buffer doesn't need to be cleared (it's empty...)
	BufferReadyToClear = 0;

	// Reset the keyboard before scanning, we might be in a wierd state
	scan_resetKeyboard();
}


// Main Detection Loop
// Since this function is non-interruptable, we can do checks here on what stage of the
//  output clock we are at (0 or 1)
// We are looking for a start of packet
// If detected, all subsequent bits are then logged into a variable
// Once the end of the packet has been detected (always the same length), decode the pressed keys
inline uint8_t Scan_loop()
{
	// Only use as a valid signal
	// Check if there was a position change
	if ( positionCounter != statePositionCounter )
	{
		// At least 80% of the samples must be valid
		if ( stateSamples * 100 / stateSamplesTotal >= 80 )
		{
			// Reset the scan counter, all the keys have been iterated over
			// Ideally this should reset at 128, however
			//  due to noise in the cabling, this often moves around
			// The minimum this can possibly set to is 124 as there
			//  are keys to service at 123 (0x78)
			// Usually, unless there is lots of interference,
			//  this should limit most of the noise.
			if ( positionCounter >= 124 )
			{
				positionCounter = 0;
			}
			// Key Press Detected
			//  - Skip 0x00 to 0x0B (11) for better jitter immunity (as there are no keys mapped to those scancodes)
			else if ( positionCounter > 0x0B )
			{
				char tmp[15];
				hexToStr( positionCounter, tmp );
				dPrintStrsNL( "Key: ", tmp );

				// Make sure there aren't any duplicate keys
				uint8_t c;
				for ( c = 0; c < KeyIndex_BufferUsed; c++ )
					if ( KeyIndex_Buffer[c] == positionCounter )
						break;

				// No duplicate keys, add it to the buffer
				if ( c == KeyIndex_BufferUsed )
					Macro_bufferAdd( positionCounter );
			}
		}
		// Remove the key from the buffer
		else if ( positionCounter < 124 && positionCounter > 0x0B )
		{
			// Check for the released key, and shift the other keys lower on the buffer
			uint8_t c;
			for ( c = 0; c < KeyIndex_BufferUsed; c++ )
			{
				// Key to release found
				if ( KeyIndex_Buffer[c] == positionCounter )
				{
					// Shift keys from c position
					for ( uint8_t k = c; k < KeyIndex_BufferUsed - 1; k++ )
						KeyIndex_Buffer[k] = KeyIndex_Buffer[k + 1];

					// Decrement Buffer
					KeyIndex_BufferUsed--;

					break;
				}
			}
		}


		// Clear the state counters
		stateSamples = 0;
		stateSamplesTotal = 0;
		statePositionCounter = positionCounter;
	}

	// Pull in a data sample for this read instance
	if ( DATA_OUT & (1 <<DATA_PIN) )
		stateSamples++;
	stateSamplesTotal++;

	// Check if the clock de-synchronized
	// And reset
	if ( positionCounter > 128 )
	{
		char tmp[15];
		hexToStr( positionCounter, tmp );
		erro_dPrint( "De-synchronization detected at: ", tmp );
		errorLED( 1 );

		positionCounter = 0;
		KeyIndex_BufferUsed = 0;

		// Clear the state counters
		stateSamples = 0;
		stateSamplesTotal = 0;

		// A keyboard reset requires interrupts to be enabled
		sei();
		scan_resetKeyboard();
		cli();
	}

	// Regardless of what happens, always return 0
	return 0;
}

// Send data
uint8_t Scan_sendData( uint8_t dataPayload )
{
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
// NOTE: Does nothing with the HP150
void Scan_lockKeyboard( void )
{
}

// NOTE: Does nothing with the HP150
void Scan_unlockKeyboard( void )
{
}

// Reset Keyboard
void Scan_resetKeyboard( void )
{
	info_print("Attempting to synchronize the keyboard, do not press any keys...");
	errorLED( 1 );

	// Do a proper keyboard reset (flushes the ripple counters)
	RESET_PORT |=  (1 << RESET_PIN);
	_delay_us(10);
	RESET_PORT &= ~(1 << RESET_PIN);

	// Delay main keyboard scanning, until the bit counter is synchronized
	uint8_t synchronized = 0;
	while ( !synchronized )
	{
		// Only use as a valid signal
		// Check if there was a position change
		if ( positionCounter != statePositionCounter )
		{
			// At least 80% of the samples must be valid
			if ( stateSamples * 100 / stateSamplesTotal >= 80 )
			{
				// Read the current data value
				if ( DATA_OUT & (1 << DATA_PIN) )
				{
					// Check if synchronized
					// There are 128 positions to scan for with the HP150 keyboard protocol
					if ( positionCounter == 128 )
						synchronized = 1;

					positionCounter = 0;
				}
			}

			// Clear the state counters
			stateSamples = 0;
			stateSamplesTotal = 0;
			statePositionCounter = positionCounter;
		}
	}

	info_print("Keyboard Synchronized!");
}

