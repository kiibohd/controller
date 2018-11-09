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
// WhiteFox
//

// ----- Includes -----

// Local Includes
#include "../device.h"
#include "../debug.h"



// ----- Defines -----

// ----- Variables -----
static uint8_t prev_btn_state = 1;

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
	PIOB->PIO_SODR = (1 << 1);

	// Rows (sense)
	Pin_Input(PIOA, 7);
}

// Called during each loop of the main bootloader sequence
void Device_process()
{
	uint8_t cur_btn_state;

	// stray capacitance hack
	Pin_Output(PIOA, 7);
	Pin_Input(PIOA, 7);

	// Check for S1 being pressed
	cur_btn_state = (PIOA->PIO_PDSR & (1<<7)) != 0;

	// Rising edge = press
	if ( cur_btn_state && !prev_btn_state )
	{
		print( "Reset key pressed." NL );
		SOFTWARE_RESET();
	}

	prev_btn_state = cur_btn_state;
}

