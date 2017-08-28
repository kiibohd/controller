/* Copyright (C) 2017 by Jacob Alexander
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

// Project Includes
#include <delay.h>

// Local Includes
#include "../mchck.h"
#include "../debug.h"
#include "../usb-internal.h"



// ----- Defines -----

#define USBPortSwapDelay_ms 1000



// ----- Variables -----

uint32_t last_ms;
uint8_t  attempt;



// ----- Functions -----

// Called early-on during ResetHandler
inline void Device_reset()
{
}

// Called during bootloader initialization
inline void Device_setup()
{
	// PTA4 - USB Swap
	// Start, disabled
	GPIOA_PDDR |= (1<<4);
	PORTA_PCR4 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOA_PCOR |= (1<<4);

	// Setup parameters for USB port swap
	last_ms = systick_millis_count;
	attempt = 0;
}

// Called during each loop of the main bootloader sequence
inline void Device_process()
{
	// For keyboards with dual usb ports, doesn't do anything on keyboards without them
	// If a USB connection is not detected in 2 seconds, switch to the other port to see if it's plugged in there

	// Only check for swapping after delay
	uint32_t wait_ms = systick_millis_count - last_ms;
	if ( wait_ms < USBPortSwapDelay_ms + attempt / 2 * USBPortSwapDelay_ms )
	{
		return;
	}

	last_ms = systick_millis_count;

	// USB not initialized, attempt to swap
	if ( usb.state != USBD_STATE_ADDRESS )
	{
		print("USB not initializing, port swapping");
		GPIOA_PTOR |= (1<<4);
		attempt++;
	}
}

