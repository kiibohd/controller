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
// K-Type
//

// ----- Includes -----

// Project Includes
#include <Lib/gpio.h>
#include <delay.h>

// Local Includes
#include "../device.h"
#include "../debug.h"
#include "../usb-internal.h"



// ----- Defines -----

#define USBPortSwapDelay_ms 1000



// ----- Variables -----

uint32_t last_ms;
uint8_t  attempt;

// USB swap pin
const GPIO_Pin usb_swap_pin = gpio(A,4);

// Esc key strobe
const GPIO_Pin strobe_pin = gpio(B,2);
const GPIO_Pin sense_pin = gpio(D,5);



// ----- Functions -----

// Called early-on during ResetHandler
void Device_reset()
{
}

// Called during bootloader initialization
void Device_setup()
{
	// PTA4 - USB Swap
	// Start, disabled
	GPIO_Ctrl( usb_swap_pin, GPIO_Type_DriveSetup, GPIO_Config_None );
	GPIO_Ctrl( usb_swap_pin, GPIO_Type_DriveLow, GPIO_Config_None );

	// Setup parameters for USB port swap
	last_ms = systick_millis_count;
	attempt = 0;

	// Setup scanning for S1
	// Col 1 (strobe)
	GPIO_Ctrl( strobe_pin, GPIO_Type_DriveSetup, GPIO_Config_None );
	GPIO_Ctrl( strobe_pin, GPIO_Type_DriveHigh, GPIO_Config_None );

	// Row 1 (sense)
	GPIO_Ctrl( sense_pin, GPIO_Type_ReadSetup, GPIO_Config_Pullup );
}

// Called during each loop of the main bootloader sequence
void Device_process()
{
	// For keyboards with dual usb ports, doesn't do anything on keyboards without them
	// If a USB connection is not detected in 2 seconds, switch to the other port to see if it's plugged in there
	// USB not initialized, attempt to swap
	if ( usb.state != USBD_STATE_ADDRESS )
	{
		// Only check for swapping after delay
		uint32_t wait_ms = systick_millis_count - last_ms;
		if ( wait_ms < USBPortSwapDelay_ms + attempt / 2 * USBPortSwapDelay_ms )
		{
			return;
		}

		last_ms = systick_millis_count;

		print("USB not initializing, port swapping");
		GPIO_Ctrl( usb_swap_pin, GPIO_Type_DriveToggle, GPIO_Config_None );
		attempt++;
	}

	// Check for S1 being pressed
	if ( GPIO_Ctrl( sense_pin, GPIO_Type_Read, GPIO_Config_Pullup ) )
	{
		print( "Reset key pressed." NL );
		SOFTWARE_RESET();
	}
}

