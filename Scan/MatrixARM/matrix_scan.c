/* Copyright (C) 2014 by Jacob Alexander
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
#include <cli.h>
#include <led.h>
#include <print.h>
#include <macro.h>

// Local Includes
#include "matrix_scan.h"

// Matrix Configuration
#include <matrix.h>



// ----- Variables -----

// Debounce Array
KeyState Matrix_scanArray[ Matrix_colsNum * Matrix_rowsNum ];



// ----- Functions -----

// Pin action (Strobe, Sense, Strobe Setup, Sense Setup)
// NOTE: This function is highly dependent upon the organization of the register map
//       Only guaranteed to work with Freescale MK20 series uCs
uint8_t Matrix_pin( GPIO_Pin gpio, Type type )
{
	// Register width is defined as size of a pointer
	uint8_t port_offset = (uint8_t)gpio.port * sizeof(unsigned int*);

	// Assumes 6 registers between GPIO Port registers
	volatile unsigned int GPIO_PDDR = *(&GPIOA_PDDR + port_offset * 6);
	volatile unsigned int GPIO_PSOR = *(&GPIOA_PSOR + port_offset * 6);
	volatile unsigned int GPIO_PCOR = *(&GPIOA_PCOR + port_offset * 6);
	volatile unsigned int GPIO_PDIR = *(&GPIOA_PDIR + port_offset * 6);

	// Assumes 35 registers between PORT pin registers
	volatile unsigned int PORT_PCR = *(&PORTA_PCR0 + port_offset * 35);

	// Operation depends on Type
	switch ( type )
	{
	case Type_StrobeOn:
		GPIO_PSOR |= (1 << gpio.pin);
		break;

	case Type_StrobeOff:
		GPIO_PCOR |= (1 << gpio.pin);
		break;

	case Type_StrobeSetup:
		// Set as output pin
		GPIO_PDDR |= (1 << gpio.pin);

		// Configure pin with slow slew, high drive strength and GPIO mux
		PORT_PCR = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);

		// Enabling open-drain if specified
		switch ( Matrix_type )
		{
		case Config_Opendrain:
			PORT_PCR |= PORT_PCR_ODE;
			break;

		// Do nothing otherwise
		default:
			break;
		}
		break;

	case Type_Sense:
		return GPIO_PDIR & (1 << gpio.pin) ? 1 : 0;

	case Type_SenseSetup:
		// Set as input pin
		GPIO_PDDR &= ~(1 << gpio.pin);

		// Configure pin with passive filter and GPIO mux
		PORT_PCR = PORT_PCR_PFE | PORT_PCR_MUX(1);

		// Pull resistor config
		switch ( Matrix_type )
		{
		case Config_Pullup:
			PORT_PCR |= PORT_PCR_PE | PORT_PCR_PS;
			break;

		case Config_Pulldown:
			PORT_PCR |= PORT_PCR_PE;
			break;

		// Do nothing otherwise
		default:
			break;
		}
		break;
	}

	return 0;
}

// Setup GPIO pins for matrix scanning
void Matrix_setup()
{
	// Setup Strobe Pins
	for ( uint8_t pin = 0; pin < Matrix_colsNum; pin++ )
	{
		Matrix_pin( Matrix_cols[ pin ], Type_StrobeSetup );
	}

	// Setup Sense Pins
	for ( uint8_t pin = 0; pin < Matrix_rowsNum; pin++ )
	{
		Matrix_pin( Matrix_rows[ pin ], Type_SenseSetup );
	}

	// Clear out Debounce Array
	for ( uint8_t item = 0; item < Matrix_maxKeys; item++ )
	{
		Matrix_scanArray[ item ].prevState     = KeyState_Off;
		Matrix_scanArray[ item ].curState      = KeyState_Off;
		Matrix_scanArray[ item ].activeCount   = 0;
		Matrix_scanArray[ item ].inactiveCount = 0;
	}
}

// Scan the matrix for keypresses
// NOTE: firstScan should be set on the first scan after a USB send (to reset all the counters)
void Matrix_scan( uint16_t scanNum, uint8_t firstScan )
{
	// For each strobe, scan each of the sense pins
	for ( uint8_t strobe = 0; strobe < Matrix_colsNum; strobe++ )
	{
		// Strobe Pin
		Matrix_pin( Matrix_cols[ strobe ], Type_StrobeOn );

		// Scan each of the sense pins
		for ( uint8_t sense = 0; sense < Matrix_rowsNum; sense++ )
		{
			// Key position
			uint8_t key = Matrix_rowsNum * strobe + sense;
			KeyState *state = &Matrix_scanArray[ key ];

			// If first scan, reset state
			if ( firstScan )
			{
				// Set previous state, and reset current state
				state->prevState = state->curState;
				state->curState  = KeyState_Invalid;
			}

			// Signal Detected
			if ( Matrix_pin( Matrix_rows[ sense ], Type_Sense ) )
			{
				// Only update if not going to wrap around
				state->activeCount   += state->activeCount   < 255 ? 1 : 0;
				state->inactiveCount -= state->inactiveCount > 0   ? 1 : 0;
			}
			// Signal Not Detected
			else
			{
				// Only update if not going to wrap around
				state->inactiveCount += state->inactiveCount < 255 ? 1 : 0;
				state->activeCount   -= state->activeCount   > 0   ? 1 : 0;
			}

			// Check for state change if it hasn't been set
			// Only check if the minimum number of scans has been met
			//   the current state is invalid
			//   and either active or inactive count is over the debounce threshold
			if ( scanNum > DEBOUNCE_THRESHOLD
			  && state->curState != KeyState_Invalid
			  && ( state->activeCount > DEBOUNCE_THRESHOLD || state->inactiveCount > DEBOUNCE_THRESHOLD ) )
			{
				switch ( state->prevState )
				{
				case KeyState_Press:
				case KeyState_Hold:
					if ( state->activeCount > DEBOUNCE_THRESHOLD )
					{
						state->curState = KeyState_Hold;
					}
					else
					{
						state->curState = KeyState_Release;
					}
					break;

				case KeyState_Release:
				case KeyState_Off:
					if ( state->activeCount > DEBOUNCE_THRESHOLD )
					{
						state->curState = KeyState_Press;
					}
					else if ( state->inactiveCount > DEBOUNCE_THRESHOLD )
					{
						state->curState = KeyState_Off;
					}
					break;

				case KeyState_Invalid:
					erro_print("Matrix scan bug!! Report me!");
					break;
				}

				// Send keystate to macro module
				Macro_keyState( key, state->curState );
			}
		}

		// Unstrobe Pin
		Matrix_pin( Matrix_cols[ strobe ], Type_StrobeOff );
	}
}

