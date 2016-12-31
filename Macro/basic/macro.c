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

// Compiler Includes
#include <Lib/MacroLib.h>

// Project Includes
#include <led.h>
#include <print.h>
#include <scan_loop.h>
#include <usb_com.h>

// Keymaps
#include <keymap.h>
#include <usb_keys.h>

// Local Includes
#include "macro.h"



// ----- Variables -----

// Keeps track of the sequence used to reflash the teensy in software
static uint8_t Bootloader_ConditionSequence[] = {1,16,6,11};
       uint8_t Bootloader_ConditionState      = 0;
       uint8_t Bootloader_NextPositionReady   = 1;



// ----- Functions -----

void jumpToBootloader(void)
{
	cli();
	// disable watchdog, if enabled
	// disable all peripherals
	UDCON = 1;
	USBCON = (1<<FRZCLK);  // disable USB
	UCSR1B = 0;
	_delay_ms(5);

#if defined(__AVR_AT90USB162__)                // Teensy 1.0
	EIMSK = 0; PCICR = 0; SPCR = 0; ACSR = 0; EECR = 0;
	TIMSK0 = 0; TIMSK1 = 0; UCSR1B = 0;
	DDRB = 0; DDRC = 0; DDRD = 0;
	PORTB = 0; PORTC = 0; PORTD = 0;
	asm volatile("jmp 0x3E00");
#elif defined(__AVR_ATmega32U4__)              // Teensy 2.0
	EIMSK = 0; PCICR = 0; SPCR = 0; ACSR = 0; EECR = 0; ADCSRA = 0;
	TIMSK0 = 0; TIMSK1 = 0; TIMSK3 = 0; TIMSK4 = 0; UCSR1B = 0; TWCR = 0;
	DDRB = 0; DDRC = 0; DDRD = 0; DDRE = 0; DDRF = 0; TWCR = 0;
	PORTB = 0; PORTC = 0; PORTD = 0; PORTE = 0; PORTF = 0;
	asm volatile("jmp 0x7E00");
#elif defined(__AVR_AT90USB646__)              // Teensy++ 1.0
	EIMSK = 0; PCICR = 0; SPCR = 0; ACSR = 0; EECR = 0; ADCSRA = 0;
	TIMSK0 = 0; TIMSK1 = 0; TIMSK2 = 0; TIMSK3 = 0; UCSR1B = 0; TWCR = 0;
	DDRA = 0; DDRB = 0; DDRC = 0; DDRD = 0; DDRE = 0; DDRF = 0;
	PORTA = 0; PORTB = 0; PORTC = 0; PORTD = 0; PORTE = 0; PORTF = 0;
	asm volatile("jmp 0xFC00");
#elif defined(__AVR_AT90USB1286__)             // Teensy++ 2.0
	EIMSK = 0; PCICR = 0; SPCR = 0; ACSR = 0; EECR = 0; ADCSRA = 0;
	TIMSK0 = 0; TIMSK1 = 0; TIMSK2 = 0; TIMSK3 = 0; UCSR1B = 0; TWCR = 0;
	DDRA = 0; DDRB = 0; DDRC = 0; DDRD = 0; DDRE = 0; DDRF = 0;
	PORTA = 0; PORTB = 0; PORTC = 0; PORTD = 0; PORTE = 0; PORTF = 0;
	asm volatile("jmp 0x1FC00");
#endif
}

// Given a sampling array, and the current number of detected keypress
// Add as many keypresses from the sampling array to the USB key send array as possible.
inline void keyPressDetection( uint8_t *keys, uint8_t numberOfKeys, uint8_t *modifiers, uint8_t numberOfModifiers, uint8_t *map )
{
	uint8_t Bootloader_KeyDetected = 0;
	uint8_t processed_keys         = 0;

	// Parse the detection array starting from 1 (all keys are purposefully mapped from 1 -> total as per typical PCB labels)
	for ( uint8_t key = 0; key < numberOfKeys + 1; key++ )
	{
		if ( keys[key] & (1 << 7) )
		{
			processed_keys++;

			// Display the detected scancode
			char tmpStr[4];
			int8ToStr( key, tmpStr );
			dPrintStrs( tmpStr, " " );

			// Is this a bootloader sequence key?
			if ( !Bootloader_KeyDetected
			   && Bootloader_NextPositionReady
			   && key == Bootloader_ConditionSequence[Bootloader_ConditionState] )
			{
				Bootloader_KeyDetected = 1;
				Bootloader_NextPositionReady = 0;
				Bootloader_ConditionState++;
			}
			else if ( Bootloader_ConditionState > 0 && key == Bootloader_ConditionSequence[Bootloader_ConditionState - 1] )
			{
				Bootloader_KeyDetected = 1;
			}

			// Determine if the key is a modifier
			uint8_t modFound = 0;
			for ( uint8_t mod = 0; mod < numberOfModifiers; mod++ ) {
				// Modifier found
				if ( modifiers[mod] == key ) {
					USBKeys_Modifiers |= map[key];
					modFound = 1;
					break;
				}
			}

			// Modifier, already done this loop
			if ( modFound )
				continue;

			// Too many keys
			if ( USBKeys_Sent >= USBKeys_MaxSize )
			{
				info_print("USB Key limit reached");
				errorLED( 1 );
				break;
			}

			// Allow ignoring keys with 0's
			if ( map[key] != 0 )
				USBKeys_Array[USBKeys_Sent++] = map[key];
		}
	}

	// Boot loader sequence state handler
	switch ( processed_keys )
	{
	// The next bootloader key can now be pressed, if there were no keys processed
	case 0:
		Bootloader_NextPositionReady = 1;
		break;
	// If keys were detected, and it wasn't in the sequence (or there was multiple keys detected), start bootloader sequence over
	// This case purposely falls through
	case 1:
		if ( Bootloader_KeyDetected )
			break;
	default:
		Bootloader_ConditionState = 0;
		break;
	}

	// Add debug separator if keys sent via USB
	if ( USBKeys_Sent > 0 )
		print("\033[1;32m|\033[0m\n");
}

inline void process_macros(void)
{
	// Only process macros once (if some were found), until the next USB send
	if ( USBKeys_Sent != 0 )
		return;

	// Debounce Sampling Array to USB Data Array
	keyPressDetection( KeyIndex_Array, KeyIndex_Size, MODIFIER_MASK, sizeof(MODIFIER_MASK), KEYINDEX_MASK );

	// Check for bootloader condition
	if ( Bootloader_ConditionState == sizeof( Bootloader_ConditionSequence ) )
		jumpToBootloader();
}

