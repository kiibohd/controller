/* Copyright (C) 2017-2018 by Jacob Alexander
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file.  If not, see <http://www.gnu.org/licenses/>.
 */

//
// Gemini Dusk/Dawn
//

// ----- Includes -----

// Local Includes
#include "../device.h"
#include "../debug.h"



// ----- Defines -----

// ----- Variables -----

// ----- Functions -----

// Called early-on during ResetHandler
void Device_reset()
{
}

void Pin_Input(Pio *pio, uint8_t pin) {
	pio->PIO_PUDR = (1 << pin);
	pio->PIO_PPDER = (1 << pin);
	pio->PIO_IFER = (1 << pin);
	pio->PIO_ODR = (1 << pin);
	pio->PIO_PER = (1 << pin);
}

void Pin_Output(Pio *pio, uint8_t pin) {
	pio->PIO_OER = (1 << pin);
	pio->PIO_PER = (1 << pin);
	pio->PIO_CODR = (1 << pin);
}

// Called during bootloader initialization
void Device_setup()
{
	// Setup scanning for S1
	PMC->PMC_PCER0 = (1 << ID_PIOA) | (1 << ID_PIOB);

	// Cols (strobe)
	Pin_Output(PIOB, 1);

	// Rows (sense)
	Pin_Input(PIOA, 26);
}

// Called during each loop of the main bootloader sequence
void Device_process()
{
	// stray capacitance hack
	Pin_Output(PIOA, 26);
	Pin_Input(PIOA, 26);

	// Check for S1 being pressed
	PIOB->PIO_SODR = (1 << 1);

	if ( PIOA->PIO_PDSR & (1<<26) )
	{
		print( "Reset key pressed." NL );
		SOFTWARE_RESET();
	}

	PIOB->PIO_CODR = (1 << 1);
}

