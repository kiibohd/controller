/* Copyright (C) 2017-2019 by Jacob Alexander
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

//
// Gemini Dusk/Dawn
//

// ----- Includes -----

// Project Includes
#include <Lib/gpio.h>
#include <delay.h>

#include <common/services/usb/udc/udc.h>
#include <sam/drivers/udp/udp_device.h>

// Local Includes
#include "../device.h"
#include "../debug.h"



// ----- Defines -----

#define USBPortSwapDelay_ms 1000



// ----- Variables -----

static uint32_t last_ms;
static uint8_t  attempt;

static uint8_t prev_btn_state = 1;

// USB swap pin
const GPIO_Pin usb_swap_pin = gpio(A,12);

// Esc key strobe
const GPIO_Pin strobe_pin = gpio(B,1);
const GPIO_Pin sense_pin = gpio(A,26);

// Debug led
const GPIO_Pin debug_led = gpio(B,0);



// ----- Functions -----

// Called early-on during ResetHandler
void Device_reset()
{
	// Make sure debug LED is off
	GPIO_Ctrl( debug_led, GPIO_Type_DriveSetup, GPIO_Config_None );
	GPIO_Ctrl( debug_led, GPIO_Type_DriveLow, GPIO_Config_None );
}

// Called during bootloader initialization
void Device_setup()
{
	// Enable Debug LED
	GPIO_Ctrl( debug_led, GPIO_Type_DriveSetup, GPIO_Config_None );
	GPIO_Ctrl( debug_led, GPIO_Type_DriveHigh, GPIO_Config_None );

	// Setup scanning for S1
	PMC->PMC_PCER0 = (1 << ID_PIOA) | (1 << ID_PIOB);

	// Cols (strobe)
	GPIO_Ctrl( strobe_pin, GPIO_Type_DriveSetup, GPIO_Config_Pulldown );
	GPIO_Ctrl( strobe_pin, GPIO_Type_DriveHigh, GPIO_Config_Pulldown );

	// Rows (sense)
	GPIO_Ctrl( sense_pin, GPIO_Type_ReadSetup, GPIO_Config_Pulldown );

	// PA12 - USB Swap
	// Start, disabled
	GPIO_Ctrl( usb_swap_pin, GPIO_Type_DriveSetup, GPIO_Config_None );
	GPIO_Ctrl( usb_swap_pin, GPIO_Type_DriveLow, GPIO_Config_None );

	// Setup parameters for USB port swap
	last_ms = 0;
	attempt = 0;
}

// Called during each loop of the main bootloader sequence
void Device_process()
{
	uint8_t cur_btn_state;

	// stray capacitance hack
	GPIO_Ctrl( sense_pin, GPIO_Type_DriveSetup, GPIO_Config_Pulldown );
	GPIO_Ctrl( sense_pin, GPIO_Type_DriveLow, GPIO_Config_Pulldown );
	GPIO_Ctrl( sense_pin, GPIO_Type_ReadSetup, GPIO_Config_Pulldown );

	// Check for S1 being pressed
	cur_btn_state = GPIO_Ctrl( sense_pin, GPIO_Type_Read, GPIO_Config_Pulldown ) != 0;

	// Rising edge = press
	if ( cur_btn_state && !prev_btn_state )
	{
		print( "Reset key pressed." NL );
		SOFTWARE_RESET();
	}

	prev_btn_state = cur_btn_state;

	// For keyboards with dual usb ports, doesn't do anything on keyboards without them
	// If a USB connection is not detected in 2 seconds, switch to the other port to see if it's plugged in there
	// USB not initialized, attempt to swap
	uint32_t wait_ms = systick_millis_count - last_ms;
	if ( wait_ms > USBPortSwapDelay_ms + attempt / 2 * USBPortSwapDelay_ms )
	{
		// Update timeout
		last_ms = systick_millis_count;

		// USB not initialized, attempt to swap
		if ( udd_get_configured_address() == 0 )
		{
			print("USB not initializing, port swapping");
			GPIO_Ctrl( usb_swap_pin, GPIO_Type_DriveToggle, GPIO_Config_None );

			// Re-initialize USB
			udc_stop();
			delay_ms(10);
			udc_stop();
			udc_start();

			attempt++;
		}
	}
}

