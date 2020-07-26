/* Copyright (C) 2020 by Jacob Alexander
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

#if !defined(_bootloader_)

// ----- Includes -----

// Project Includes
#include <output_com.h>
#include <print.h>
#include <Lib/gpio.h>

#if defined(_sam_)
// ASF Includes
#include <sam/drivers/pmc/pmc.h>
#include <sam/drivers/supc/supc.h>
#include <sam/drivers/wdt/wdt.h>
#endif

// KLL Includes
#include <kll_defs.h>

// Local Includes
#include "sleep.h"



// ----- Defines -----

#define MASK_STATUS0 0xFFFFFFFC
#define MASK_STATUS1 0xFFFFFFFF

#if !defined(ScanCodeStrobeList_define)
#define ScanCodeStrobeList_define
#endif
#if !defined(ScanCodeSenseList_define)
#define ScanCodeSenseList_define
#endif



// ----- Variables -----

const GPIO_Pin Sleep_pins[] = { WakeupPinList_define };
const GPIO_Pin Sleep_strobes[] = { ScanCodeStrobeList_define };
const GPIO_Pin Sleep_senses[] = { ScanCodeSenseList_define };

uint32_t wake_status;



// ----- Function Declarations -----

// ----- Weak Functions

void Scan_prepare_sleep() __attribute__ ((weak));



// ----- Functions -----

#if defined(_sam4s_)
// Lookup wkup index
// Returns -1 if the GPIO pin doesn't have wkup functionality
int8_t wkup_lookup(GPIO_Pin pin)
{
	switch (pin.port)
	{
	case GPIO_Port_A:
		switch (pin.pin)
		{
		case GPIO_Pin_0:
			return 0;
		case GPIO_Pin_1:
			return 1;
		case GPIO_Pin_2:
			return 2;
		case GPIO_Pin_4:
			return 3;
		case GPIO_Pin_5:
			return 4;
		case GPIO_Pin_8:
			return 5;
		case GPIO_Pin_9:
			return 6;
		case GPIO_Pin_11:
			return 7;
		case GPIO_Pin_14:
			return 8;
		case GPIO_Pin_15:
			return 14;
		case GPIO_Pin_16:
			return 15;
		case GPIO_Pin_19:
			return 9;
		case GPIO_Pin_20:
			return 10;
		case GPIO_Pin_30:
			return 11;
		default:
			break;
		}
		break;

	case GPIO_Port_B:
		switch (pin.pin)
		{
		case GPIO_Pin_2:
			return 12;
		case GPIO_Pin_5:
			return 13;
		default:
			break;
		}
		break;

	default:
		break;
	}

	// No wkup functionality
	return -1;
}
#endif


// To prepare for sleep mode, set all GPIO to floating
// The intention is to set all pins to floating, then configure the necessary ones to correctly enter deep sleep
void sleep_set_all_floats()
{
#if defined(_sam4s_)
	// Setup all pins as PIO
	PIOA->PIO_PER = 0xFFFFFFFF;
	PIOB->PIO_PER = 0xFFFFFFFF;
#if defined(_sam4s_c_)
	PIOB->PIO_PER = 0xFFFFFFFF;
#endif

	// Disable output on all PIOs
	PIOA->PIO_ODR = 0xFFFFFFFF;
	PIOB->PIO_ODR = 0xFFFFFFFF;
#if defined(_sam4s_c_)
	PIOB->PIO_ODR = 0xFFFFFFFF;
#endif

	// Disable all internal pull-ups
	PIOA->PIO_PUDR = 0xFFFFFFFF;
	PIOB->PIO_PUDR = 0xFFFFFFFF;
#if defined(_sam4s_c_)
	PIOB->PIO_PUDR = 0xFFFFFFFF;
#endif
#endif
}


void sleep_set_strobes(bool drive_high)
{
#if defined(_sam4s_)
	// Enable output on all strobes
	for (uint8_t strobe = 0; strobe < sizeof(Sleep_strobes) / sizeof(GPIO_Pin); strobe++)
	{
		GPIO_Ctrl(Sleep_strobes[strobe], GPIO_Type_DriveSetup, GPIO_Config_None);
		GPIO_Ctrl(Sleep_strobes[strobe], drive_high ? GPIO_Type_DriveHigh : GPIO_Type_DriveLow, GPIO_Config_None);
	}
#endif
}


void sleep_set_senses(bool pullup)
{
#if defined(_sam4s_)
	// Setup pull-down on sense pins
	for (uint8_t sense = 0; sense < sizeof(Sleep_senses) / sizeof(GPIO_Pin); sense++)
	{
		GPIO_Ctrl(Sleep_senses[sense], GPIO_Type_ReadSetup, pullup ? GPIO_Config_Pullup : GPIO_Config_Pulldown);
	}
#endif
}


void sleep_set_wake_pins()
{
#if defined(_sam4s_)
	// TODO (HaaTa): Current we only support Low to High pulse events
	uint32_t wkup_inputs = 0;
	uint32_t wkup_transitions = 0;
	for (uint8_t pin = 0; pin < sizeof(Sleep_pins) / sizeof(GPIO_Pin); pin++)
	{
		int8_t offset = wkup_lookup(Sleep_pins[pin]);
		if (offset < 0)
		{
			// Invalid gpio pin
			continue;
		}
		wkup_inputs |= (1 << offset);
		//wkup_transitions |= (1 << (offset + 16));
	}
	supc_set_wakeup_inputs(SUPC, wkup_inputs, wkup_transitions);
#endif
}


// Check if MCU is ready for deep sleep
// If ignore_usb is set, don't look to USB to see if we shouldn't sleep yet
bool deep_sleep_ready(bool ignore_usb)
{
	// Don't check USB status, we're ready for sleep
	if (ignore_usb)
	{
		return true;
	}

	// Check if Output Module is available
	if (Output_Available)
	{
		return false;
	}

	return true;
}


// Enter sleep mode
// Must be awoken using a configured GPIO/WKUP pin
void deep_sleep()
{
#if defined(_sam4s_)
	// Prepare for low power mode
	Output_prepare_sleep_mode();

	// Disable watchdog
	wdt_disable(WDT);

#if SPILink_Enabled_define == 1
	// Wait for SPILink sleep signal
	// BLE Module will pull it's CS high right before entering deep sleep
	// TODO
#endif

	// Do device sleep prep
	Scan_prepare_sleep();

	// Disable all peripheral clocks
	pmc_disable_all_periph_clk();
	pmc_disable_all_pck();
	pmc_disable_udpck();

	// Setup WKUP pins
	sleep_set_wake_pins();

	// Enable backup mode (deepest sleep state)
	// - Disable voltage regulator
	pmc_enable_backupmode();
#else
	printNL("deep_sleep - Unsupported");
#endif
}

#endif
