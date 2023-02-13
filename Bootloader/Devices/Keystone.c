/* Copyright (C) 2017-2023 by Jacob Alexander
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
// Keystone
//
// XXX
// XXX - To enable UART debugging, you must comment out P(A,10)
// XXX   below, otherwise the UART will not work.
// XXX   This is due to the ADC strobes using the same pins and will
// XXX   interfere with ADC sense used to detect the Esc key.
// XXX
//

// ----- Includes -----

// Project Includes
#include <Lib/gpio.h>
#include <delay.h>

// ASF Includes
#include <common/services/clock/sysclk.h>
#include <sam/drivers/adc/adc.h>

// Local Includes
#include "../device.h"
#include "../debug.h"



// ----- Defines -----

// This value is approximately the tactile point of a Silo tactile
// switch.
#define ADC_PRESS_THRESHOLD 0x400

// ----- Variables -----

// Esc key strobe
const GPIO_Pin strobe_pin = gpio(B,0);
const GPIO_Pin sense_pin = gpio(A,17); // Row 1

// Debug led
const GPIO_Pin debug_led = gpio(A,15);

// Cfg Pins
const GPIO_Pin cfg1_pin = gpio(C,25);
const GPIO_Pin cfg2_pin = gpio(C,26);

// Model Check Pin
const GPIO_Pin model_check_strobe = gpio(A,28);

// Disable strobes
const GPIO_Pin Matrix_cols[] = {
	gpio(B,0),
	gpio(B,1),
	gpio(B,2),
	gpio(B,3),
	gpio(B,4),
	gpio(B,5),
	gpio(B,14),
	gpio(A,0),
	gpio(A,1),
	gpio(A,2),
	gpio(A,5),
	gpio(A,6),
	gpio(A,7),
	gpio(A,8),
	gpio(A,9),
	gpio(A,10), // XXX (HaaTa) You must disable this for UART debug
	gpio(A,23),
	gpio(A,24),
	gpio(A,25),
	gpio(A,26),
	gpio(A,27),
	gpio(A,28),
};

// Used to compare with the previous adc state
uint32_t prev_adc_val = 0xFFFF;

// Convenience Macros
#define Matrix_colsNum sizeof(Matrix_cols) / sizeof(GPIO_Pin)



// ----- Functions -----


// ADC Setup
void adc_setup()
{
	// Setup ADC
	pmc_enable_periph_clk(ID_ADC);

	// Disable write protection
	adc_set_writeprotect(ADC, 0);

	/*
	 * Formula: ADCClock = MCK / ((PRESCAL+1) * 2)
	 *  MCK = 120MHz, PRESCAL = 2, then:
	 *  ADCClock = 120 / ((2+1) * 2) = 20MHz;
	 *  sam4s max ADCClock = 22 MHz
	 *
	 * Formula:
	 *     Startup  Time = startup value / ADCClock
	 *     Startup time = 64 / 20MHz = 3.2 us (4)
	 *     Startup time = 80 / 20MHz = 4 us (5)
	 *     Startup time = 96 / 20MHz = 4.8 us (6)
	 *     Startup time = 112 / 20MHz = 5.6 us (7)
	 *     Startup time = 512 / 20MHz = 25.6 us (8)
	 *     sam4s Min Startup Time = 4 us (max 12 us)
	 */
	adc_init(ADC, sysclk_get_cpu_hz(), 20000000, ADC_STARTUP_TIME_5);

	/* Set ADC timing.
	 * Formula:
	 *
	 *     Ttrack minimum = 0.054 * Zsource + 205
	 *     Ttrack minimum = 0.054 * 1.5k + 205 = 286 ns
	 *     Ttrack minimum = 0.054 * 10k + 205 = 745 ns
	 *     Ttrack minimum = 0.054 * 20k + 205 = 1285 ns
	 *     20MHz -> 50 ns * 15 cycles = 750 ns
	 *     750 ns > 286 ns -> Tracktim can be set to 0
	 *     750 ns > 745 ns -> Tracktim can be set to 0
	 *     750 ns < 1285 ns -> Tracktim can be set to 10 => 750 ns + 550 ns (10) = 1300 ns
	 *     See sam4s datasheet Figure 44-21 and Table 44-41 for details
	 *
	 *     Transfer Time = (TRANSFER * 2 + 3) / ADCClock
	 *     Tracking Time = (TRACKTIM + 1) / ADCClock
	 *     Settling Time = settling value / ADCClock
	 *
	 *     Hold Time
	 *     Transfer Time = (0 * 2 + 3) / 20MHz = 150 ns
	 *     Transfer Time = (1 * 2 + 3) / 20MHz = 250 ns
	 *     Transfer Time = (2 * 2 + 3) / 20MHz = 350 ns
	 *     Transfer Time = (3 * 2 + 3) / 20MHz = 450 ns
	 *
	 *     Track Time
	 *     Tracking Time = (0 + 1) / 20MHz = 50 ns
	 *     Tracking Time = (1 + 1) / 20MHz = 100 ns
	 *     Tracking Time = (2 + 1) / 20MHz = 150 ns
	 *     Tracking Time = (3 + 1) / 20MHz = 200 ns
	 *     Tracking Time = (4 + 1) / 20MHz = 250 ns
	 *     Tracking Time = (5 + 1) / 20MHz = 300 ns
	 *     Tracking Time = (6 + 1) / 20MHz = 350 ns
	 *     Tracking Time = (7 + 1) / 20MHz = 400 ns
	 *     Tracking Time = (8 + 1) / 20MHz = 450 ns
	 *     Tracking Time = (9 + 1) / 20MHz = 500 ns
	 *     Tracking Time = (10 + 1) / 20MHz = 550 ns
	 *     Tracking Time = (11 + 1) / 20MHz = 600 ns
	 *     Tracking Time = (12 + 1) / 20MHz = 650 ns
	 *     Tracking Time = (13 + 1) / 20MHz = 700 ns
	 *     Tracking Time = (14 + 1) / 20MHz = 750 ns
	 *     Tracking Time = (15 + 1) / 20MHz = 800 ns
	 *
	 *     Analog Settling Time
	 *     (TODO May need to tune this)
	 *     Settling Time = 3 / 20MHz = 150 ns (0)
	 *     Settling Time = 5 / 20MHz = 250 ns (1)
	 *     Settling Time = 9 / 20MHz = 450 ns (2)
	 *     Settling Time = 17 / 20MHz = 850 ns (3)
	 */
	const uint8_t tracking_time = 15;
	const uint8_t transfer_period = 2; // Recommended to be set to 2 by datasheet (42.7.2)
	adc_configure_timing(ADC, tracking_time, ADC_SETTLING_TIME_3, transfer_period);

	// Enable ADC channel 0
	adc_enable_channel(ADC, (enum adc_channel_num_t)0);

	// Set gain to 4x
	adc_set_channel_input_gain(ADC, (enum adc_channel_num_t)0, 3);

	// Enable offset
	adc_enable_channel_input_offset(ADC, (enum adc_channel_num_t)0);

	// Set Auto Calibration Mode
	adc_set_calibmode(ADC);
	while ((adc_get_status(ADC) & ADC_ISR_EOCAL) != ADC_ISR_EOCAL);

	// Configure free run mode /w SW trigger
	adc_configure_trigger(ADC, ADC_TRIG_SW, 1);

	// Enable ADC (SW trigger)
	adc_start(ADC);
}

// Read the ADC and determine if the key has been pressed
// Uses a threshold
// This is a very simple "threshold break" event detection
// All other events are ignored.
bool adc_key_pressed()
{
	// Get the most recent ADC value and mask to 12 bits
	// Mask the bottom 8 bits to get more consistent results
	uint32_t adc_value = adc_get_latest_value(ADC) & 0xF00;
	// If the previous ADC value was higher than the threshold, ignore
	// the current state and store for later.
	if (prev_adc_val > ADC_PRESS_THRESHOLD)
	{
		prev_adc_val = adc_value;
		return false;
	}
	prev_adc_val = adc_value;

	// XXX (HaaTa) - There is a bug here, if you hold down the key
	//               too long, the bootloader resets, and on release
	//               there will be another reset. Likely this is due
	//               to the initial ADC state. More careful initialization
	//               can fix this, but it really doesn't matter (reset
	//               is very fast).
	return adc_value >= ADC_PRESS_THRESHOLD;
}

// Called early-on during ResetHandler
void Device_reset()
{
	// Make sure debug LED is off
	GPIO_Ctrl( debug_led, GPIO_Type_DriveSetup, GPIO_Config_None );
	GPIO_Ctrl( debug_led, GPIO_Type_DriveHigh, GPIO_Config_None );
}

// Called during bootloader initialization
void Device_setup(bool *alt_device)
{
	// Enable Debug LED
	GPIO_Ctrl( debug_led, GPIO_Type_DriveSetup, GPIO_Config_None );
	GPIO_Ctrl( debug_led, GPIO_Type_DriveLow, GPIO_Config_None );

	// Disable all strobes (to prevent voltage regulator from overheating on prototypes)
	for (uint8_t c = 0; c < Matrix_colsNum; c++)
	{
		GPIO_Ctrl(Matrix_cols[c], GPIO_Type_DriveSetup, GPIO_Config_Pulldown);
		GPIO_Ctrl(Matrix_cols[c], GPIO_Type_DriveHigh, GPIO_Config_Pulldown);
	}

	// Detect which configuration this pcb is in
	GPIO_Ctrl( cfg1_pin, GPIO_Type_ReadSetup, GPIO_Config_Pulldown );
	GPIO_Ctrl( cfg2_pin, GPIO_Type_ReadSetup, GPIO_Config_Pulldown );
	uint8_t cfg1 = GPIO_Ctrl( cfg1_pin, GPIO_Type_Read, GPIO_Config_Pulldown ) != 0;
	uint8_t cfg2 = GPIO_Ctrl( cfg2_pin, GPIO_Type_Read, GPIO_Config_Pulldown ) != 0;

	// Compute layout
	// Default to ANSI
	if ( !cfg1 && cfg2 )
	{
		// TODO ISO
	}
	else if ( cfg1 && !cfg2 )
	{
		// TODO JIS
	}
	else
	{
		// TODO ANSI
	}

	// Setup scanning for S1
	PMC->PMC_PCER0 = (1 << ID_PIOA) | (1 << ID_PIOB);

	// Check Fullsize vs TKL using fullsize-only strobes
	// Strobes are wired to a 1k pullup, so if the pin is high, it's fullsize
	GPIO_Ctrl( model_check_strobe, GPIO_Type_ReadSetup, GPIO_Config_Pulldown );
	delay_ms(10);
	if (GPIO_Ctrl( model_check_strobe, GPIO_Type_Read, GPIO_Config_None ) == 1)
	{
		// Fullsize
		print("Fullsize detected" NL);
		*alt_device = false;
	}
	else
	{
		// TKL
		print("TKL detected" NL);
		*alt_device = true;
	}

	// Cols (strobe)
	GPIO_Ctrl( strobe_pin, GPIO_Type_DriveSetup, GPIO_Config_None );
	GPIO_Ctrl( strobe_pin, GPIO_Type_DriveLow, GPIO_Config_None );

	// Row (sense) - ADC
	adc_setup();
}

// Called during each loop of the main bootloader sequence
void Device_process()
{
	// Check ADC to see if the key has been pressed
	if ( adc_key_pressed() )
	{
		print( "Reset key pressed." NL );
		SOFTWARE_RESET();
	}
}

