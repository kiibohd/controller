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

// - Pinout Defines -

// Scan Bit Pins (from keyboard)
// - Reads in the ASCII scancode
// - Shift and ShiftLock are handled internally
#define READSCAN_PORT PORTC
#define READSCAN_DDR   DDRC
#define READSCAN_PIN   PINC


// Interrupt Pins (from keyboard)
// - CODEINT  (Code key signal  interrupt/press)
//   - Normally high, low when "Code" key is pressed; separate from all other key presses
// - PRESSINT (Press signal     interrupt/press)
//   - Normally high, low when any key (or multiple except "Code") is pressed, returns to high once all keys are released
//   - Signal is changed BEFORE the Scan Bits are updated
// - PULSEINT (Key action pulse interrupt/press)
//   - Normally high, low pulses of 147us on key presses (depending on the combination of mode control pins)
//   - Pulse is guarranteed to sent after the Scan Bits are updated
#define CODEINT_PORT PORTE
#define CODEINT_DDR   DDRE
#define CODEINT_PIN   PINE
#define CODEINT_POS      7

#define PRESSINT_PORT PORTE
#define PRESSINT_DDR   DDRE
#define PRESSINT_POS      6

#define PULSEINT_PORT PORTD
#define PULSEINT_DDR   DDRD
#define PULSEINT_POS      3


// LED Pins (to keyboard)
// - 1 disable LED
// - 1  enable LED
#define LED1_PORT PORTF // [Pin 19]
#define LED1_DDR   DDRF
#define LED1_POS      6

#define LED2_PORT PORTF // [Pin 20]
#define LED2_DDR   DDRF
#define LED2_POS      7


// Mode Control Pins (to keyboard)
// - Repeat [Pin 14]
//   - 1 Single   pulse mode (PULSEINT)
//   - 0 Repeated pulse mode (PULSEINT) (1 pulse, pause, then constant pulses)
// - Multi  [Pin 15]
//   - 1 1KRO mode (typewriter compatibility mode)
//   - 0 NKRO mode (new pulse on each keypress - PULSEINT)
// - Signal [Pin 18]
//   - 1 disables pulse interrupt (PULSEINT)
//   - 0 enables  pulse interrupt (PULSEINT)
#define REPEAT_PORT PORTF
#define REPEAT_DDR   DDRF
#define REPEAT_POS      3

#define MULTI_PORT PORTF
#define MULTI_DDR   DDRF
#define MULTI_POS      4

#define SIGNAL_PORT PORTF
#define SIGNAL_DDR   DDRF
#define SIGNAL_POS      5


// Manually Scanned Keys
// Keys that the controller screws up, requiring a separate wire to be brought to the controller
// Note: Safer to route these through a NOT gate to boost the signal strength
//       Values below are AFTER NOT gate
// - Shift (both shift keys are on the same scan line)
//   - 0 Released
//   - 1 Pressed
// - Shift Lock
//   - 0 Released
//   - 1 Pressed
#define MANUAL_SCAN_KEYS 2

#define SHIFT_KEY      0
#define SHIFT_PORT PORTF
#define SHIFT_DDR   DDRF
#define SHIFT_PIN   PINF
#define SHIFT_POS      0

#define SHIFTLOCK_KEY      1
#define SHIFTLOCK_PORT PORTF
#define SHIFTLOCK_DDR   DDRF
#define SHIFTLOCK_PIN   PINF
#define SHIFTLOCK_POS      1


// ----- Macros -----



// ----- Variables -----

// Buffer used to inform the macro processing module which keys have been detected as pressed
volatile uint8_t KeyIndex_Buffer[KEYBOARD_BUFFER];
volatile uint8_t KeyIndex_BufferUsed;
volatile uint8_t KeyIndex_Add_InputSignal; // Used to pass the (click/input value) to the keyboard for the clicker

volatile uint8_t KeyScan_Table[MANUAL_SCAN_KEYS]; // Used for tracking key status of manually scanned keys
volatile uint8_t KeyScan_Prev [MANUAL_SCAN_KEYS]; // Keeps track of key state changes
volatile uint8_t KeyScan_Count;



// ----- Functions -----

// Pre-declarations
void processKeyValue( uint8_t keyValue );



// Setup
inline void Scan_setup()
{
	// Setup the external interrupts for
	// - General keypresses     (INT6/E6) ->         rising edge (to detect key       release)
	// - "Code" key             (INT7/E7) -> falling/rising edge (to detect key press/release)
	// - General keypress pulse (INT3/D3) -> falling        edge (to detect key press        )
	EICRA = 0x80;
	EICRB = 0x70;
	EIMSK = 0xC8;


	// Setup Interrupt Pins
	CODEINT_PORT  |=  (1 << CODEINT_POS );
	CODEINT_DDR   &= ~(1 << CODEINT_POS );

	PRESSINT_PORT |=  (1 << PRESSINT_POS);
	PRESSINT_DDR  &= ~(1 << PRESSINT_POS);

	PULSEINT_PORT |=  (1 << PULSEINT_POS);
	PULSEINT_DDR  &= ~(1 << PULSEINT_POS);


	// Setup LED Pins (default off)
	LED1_PORT |= (1 << LED1_POS);
	LED1_DDR  |= (1 << LED1_POS);

	LED2_PORT |= (1 << LED2_POS);
	LED2_DDR  |= (1 << LED2_POS);


	// Setup READSCAN pins to read out scancode
	READSCAN_PORT = 0xFF;
	READSCAN_DDR  = 0x00;


	// Setup Mode Control Pins
	// Note: These can be changed at any time, but there is no real reason too for a USB converter
	REPEAT_PORT |=  (1 << REPEAT_POS); // Setting high for single press mode
	REPEAT_DDR  |=  (1 << REPEAT_POS);

	MULTI_PORT  &= ~(1 << MULTI_POS ); // Setting low for multi press mode (NKRO)
	MULTI_DDR   |=  (1 << MULTI_POS );

	SIGNAL_PORT &= ~(1 << SIGNAL_POS); // Setting low to enable PULSEINT
	SIGNAL_DDR  |=  (1 << SIGNAL_POS);


	// Setup Troublesome Key Pins
	SHIFT_PORT     &= ~(1 << SHIFT_POS    );
	SHIFT_DDR      &= ~(1 << SHIFT_POS    );

	SHIFTLOCK_PORT &= ~(1 << SHIFTLOCK_POS);
	SHIFTLOCK_DDR  &= ~(1 << SHIFTLOCK_POS);


	// Reset the keyboard before scanning, we might be in a wierd state
	scan_resetKeyboard();
}

// Main Detection Loop
// Not needed for the Sony OA-S3400 as signals are interrupt based, thus this is a busy loop
// XXX Function is used for scanning troublesome keys, technically this is not needed for a pure converter
//     I just want proper use of the shift and shift lock keys, without having to do major rework to attach to the entire matrix
inline uint8_t Scan_loop()
{
	// Loop through known keys
	for ( uint8_t key = 0; key < MANUAL_SCAN_KEYS; key++ ) switch ( key )
	{
	case SHIFT_KEY:
		if ( SHIFT_PIN & (1 << SHIFT_POS) )
		{
			KeyScan_Table[SHIFT_KEY]++;
		}
		break;
	case SHIFTLOCK_KEY:
		if ( SHIFTLOCK_PIN & (1 << SHIFTLOCK_POS) )
		{
			KeyScan_Table[SHIFTLOCK_KEY]++;
		}
		break;
	default:
		erro_print("Invalid key scan index");
		break;
	}

	// Increment vote instance
	KeyScan_Count++;

	// Loop function again if not enough votes have been tallied
	if ( KeyScan_Count < 255 )
		return 1;

	// Clear vote data
	KeyScan_Count = 0;

	// Loop through known keys
	for ( uint8_t key = 0; key < MANUAL_SCAN_KEYS; key++ )
	{
		// Key scanned as pressed (might have been held from a previous vote)
		if ( KeyScan_Table[key] > 127 )
		{
			// Keypress detected
			if ( !KeyScan_Prev[key] )
			{
				processKeyValue( 0x90 + key ); // Arbitrary key mapping starts at 0x90
				KeyScan_Prev[key] = 1;
			}
		}
		// Key scanned as released
		else
		{
			// Keypress detected
			if ( KeyScan_Prev[key] )
			{
				processKeyValue( 0xA0 + key ); // Arbitrary key mapping release starts at 0xA0
				KeyScan_Prev[key] = 0;
			}
		}

		// Clear votes
		KeyScan_Table[key] = 0;
	}

	// End loop, process macros and USB data
	return 0;
}

void processKeyValue( uint8_t keyValue )
{
	// - Convert Shifted Value to non-shifted ASCII code -

	// Alphabetic
	if ( keyValue >= 0x61 && keyValue <= 0x7A )
	{
		keyValue -= 0x20;
	}
	// Other keys with ASCII shift codes
	else
	{
		switch ( keyValue )
		{
		case 0x21: // 1
		case 0x23: // 3
		case 0x24: // 4
		case 0x25: // 5
			keyValue += 0x10;
			break;
		case 0x26: // 7
		case 0x28: // 9
			keyValue += 0x11;
			break;
		case 0x81: // 1/2
		case 0x3A: // ;
			keyValue += 0x01;
			break;
		case 0x29: // 0
			keyValue = 0x30;
			break;
		case 0x40: // 2
			keyValue = 0x32;
			break;
		case 0x80: // 6
			keyValue = 0x36;
			break;
		case 0x2A: // 8
			keyValue = 0x38;
			break;
		case 0x5F: // -
			keyValue = 0x2D;
			break;
		case 0x2B: // +
			keyValue = 0x3D;
			break;
		case 0x22: // "
			keyValue = 0x27;
			break;
		case 0x3F: // ?
			keyValue = 0x2F;
			break;
		}
	}

	// TODO Move to Macro Section
	switch ( keyValue )
	{
	case 0xD3: // F11
		scan_sendData( 0x01 );
		break;
	case 0xD4: // F12
		scan_sendData( 0x02 );
		break;
	}

	// Scan code is now finalized, and ready to add to buffer
	// Note: Scan codes come from 3 different interrupts and a manual key scan into this function

	// Debug
	char tmpStr[6];
	hexToStr( keyValue, tmpStr );
	dPrintStrs( tmpStr, " " );

	// Detect release condition
	uint8_t release = 0;
	switch ( keyValue )
	{
	case 0xA0:
	case 0xA1:
	case 0xA2:
		keyValue -= 0x10;
	case 0xB0:
		release = 1;
		break;
	}

	// Key Release
	if ( release )
	{
		// Check for the released key, and shift the other keys lower on the buffer
		uint8_t c;
		for ( c = 0; c < KeyIndex_BufferUsed; c++ )
		{
			// General key buffer clear
			if ( keyValue == 0xB0 )
			{
				switch ( KeyIndex_Buffer[c] )
				{
				// Ignore these keys on general key release (have their own release codes)
				case 0x90:
				case 0x91:
				case 0x92:
					break;

				// Remove key from buffer
				default:
					// Shift keys from c position
					for ( uint8_t k = c; k < KeyIndex_BufferUsed - 1; k++ )
						KeyIndex_Buffer[k] = KeyIndex_Buffer[k + 1];

					// Decrement Buffer
					KeyIndex_BufferUsed--;

					// Start at this position again for the next loop
					c--;

					break;
				}
			}
			// Key to release found
			else if ( KeyIndex_Buffer[c] == keyValue )
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

// Key Press Detected Interrupt
ISR(INT3_vect)
{
	cli(); // Disable Interrupts

	uint8_t keyValue = 0x00;

	// Bits are flipped coming in from the keyboard
	keyValue = ~READSCAN_PIN;

	// Process the scancode
	processKeyValue( keyValue );

	sei(); // Re-enable Interrupts
}

// Key Release Detected Interrupt
ISR(INT6_vect)
{
	cli(); // Disable Interrupts

	// Send release code for general keys, 0xB0
	processKeyValue( 0xB0 );

	sei(); // Re-enable Interrupts
}

// Code Key Interrupt
ISR(INT7_vect)
{
	cli(); // Disable Interrupts

	// Code Key Released (send scancode)
	if ( CODEINT_PIN & (1 << CODEINT_POS) )
	{
		processKeyValue( 0xA2 );
	}
	// Code Key Pressed (send scancode)
	else
	{
		processKeyValue( 0x92 );
	}

	sei(); // Re-enable Interrupts
}


// Send data to keyboard
// Sony OA-S3400 has no serial/parallel dataport to send data too...
// Using this function for LED enable/disable
uint8_t Scan_sendData( uint8_t dataPayload )
{
	switch ( dataPayload )
	{
	case 0x01:
		LED1_PORT ^= (1 << LED1_POS);
		break;
	case 0x02:
		LED2_PORT ^= (1 << LED2_POS);
		break;
	default:
		erro_print("Invalid data send attempt");
		break;
	}
	return 0;
}

// Signal KeyIndex_Buffer that it has been properly read
// Not needed as a signal is sent to remove key-presses
void Scan_finishedWithBuffer( uint8_t sentKeys )
{
	return;
}

// Reset/Hold keyboard
// Sony OA-S3400 has no locking signals
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

	// Clear the KeyScan table and count
	for ( uint8_t key = 0; key < MANUAL_SCAN_KEYS; key++ )
	{
		KeyScan_Table[key] = 0;
		KeyScan_Prev [key] = 0;
	}
	KeyScan_Count = 0;
}

// USB module is finished with buffer
// Not needed as a signal is sent to remove key-presses
void Scan_finishedWithUSBBuffer( uint8_t sentKeys )
{
	return;
}

