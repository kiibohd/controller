/* Copyright (C) 2014-2020 by Jacob Alexander
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this file.  If not, see <http://www.gnu.org/licenses/>.
 */

// ----- Includes -----

// Compiler Includes
#include <Lib/ScanLib.h>

// Project Includes
#include <cli.h>
#include <kll_defs.h>
#include <kll.h>
#include <latency.h>
#include <led.h>
#include <print.h>
#include <macro.h>
#include <Lib/gpio.h>
#include <Lib/time.h>

// Local Includes
#include "dial_scan.h"



// ----- Defines -----

// ----- Enumerations -----

// ----- Structs -----

// ----- Function Declarations -----

// CLI Functions
void cliFunc_dialDebug(char* args);
void cliFunc_dialDivider(char* args);



// ----- Variables -----

// Scan Module command dictionary
CLIDict_Entry(dialDebug,   "Enables dial debug mode, prints out direction on each event.");
CLIDict_Entry(dialDivider, "Sets the quadrature dial divider.");

CLIDict_Def(dialCLIDict, "Dial Module Commands") = {
	CLIDict_Item(dialDebug),
	CLIDict_Item(dialDivider),
	{ 0, 0, 0 } // Null entry for dictionary end
};

// 2-bit Grey Code
// 00 - 0
// 01 - 1
// 11 - 2
// 10 - 3
// 00 - 0
static const uint8_t grey_code_lookup[] = {
	0, // 00 - 0
	1, // 01 - 1
	3, // 10 - 3
	2, // 11 - 2
};

// Quadrature pins
static const GPIO_Pin pin1 = Dial_Pin1_define;
static const GPIO_Pin pin2 = Dial_Pin2_define;

// Quadrature dial index
static const uint8_t Dial_dial1_index = 0;

// Quadrature tracking
static uint8_t previous_state;
static int32_t rotations_pending;

// Quadrature dial divider
// Determines how many rotations are required to fire a single event
static uint32_t dial_divider;

// Direction mirroring
// Flips the direction
static bool dial_mirror;

// Dial debug flag - If set to 1, for each event, print the direction
static volatile uint8_t dialDebugMode;

// Latency tracking
static volatile uint8_t dialLatencyResource;



// ----- Functions -----

// State tracking
// Read value of both side of quadrature
void Dial_stateTracking()
{
	uint8_t grey;

	// Read state of both pins as a grey code
	grey = GPIO_Ctrl(pin1, GPIO_Type_Read, GPIO_Config_Pulldown);
	grey |= GPIO_Ctrl(pin2, GPIO_Type_Read, GPIO_Config_Pulldown) << 1;

	// Lookup state using grey code
	uint8_t new_state = grey_code_lookup[grey];

	// Compare against previous state
	if (previous_state == 0xFF)
	{
		// Reset state and counter
		rotations_pending = 0;
	}
	// Reverse rollover, 0 -> 3
	else if (previous_state == 0 && new_state == 3)
	{
		rotations_pending--; // CCW
	}
	// Rollover, 3 -> 0
	else if (previous_state == 3 && new_state == 0)
	{
		rotations_pending++; // CW
	}
	// Increment
	else if (previous_state < new_state)
	{
		rotations_pending++; // CW
	}
	// Decrement
	else if (previous_state > new_state)
	{
		rotations_pending--; // CCW
	}

	// Debug
	if (dialDebugMode)
	{
		printSInt32(rotations_pending);
		print(" (");
		printHex(previous_state);
		print("->");
		printHex(new_state);
		print(")" NL);
	}

	// Any other state transition is a no-op and will update the current state
	// This may be possible if the dial spins too quickly (unlikely)
	// A proper moter quadrature decoder is necessary in this case (hardware based)

	// Prepare for next state
	previous_state = new_state;
}


// Setup Dial Quadrature decoding
void Dial_setup()
{
	// Register Dial CLI dictionary
	CLI_registerDictionary(dialCLIDict, dialCLIDictName);

	// Debug mode
	dialDebugMode = 0;

	// Setup event divider
	dial_divider = Dial_Divider_define;

	// Setup direction mirroring
	dial_mirror = DialMirror_define;

	// Setup GPIO inputs
	GPIO_Ctrl(pin1, GPIO_Type_ReadSetup, GPIO_Config_Pulldown);
	GPIO_Ctrl(pin2, GPIO_Type_ReadSetup, GPIO_Config_Pulldown);

	// Read current state and reset rotation counter
	previous_state = 0xFF;
	Dial_stateTracking();

	// Setup interrupt
	GPIO_IrqSetup(pin1, 1);
	GPIO_IrqSetup(pin2, 1);
	NVIC_EnableIRQ(PIOA_IRQn);

	// Setup latency module
	dialLatencyResource = Latency_add_resource("Dial", LatencyOption_Ticks);
}


// Calculates averages and generates events
void Dial_periodic()
{
	// Start latency measurement
	Latency_start_time(dialLatencyResource);

	uint8_t neg = 0;
	uint32_t abs_rotations = 0;

	// Determine absolute value and direction
	if (rotations_pending > 0)
	{
		abs_rotations = rotations_pending;
	}
	else if (rotations_pending < 0)
	{
		neg = 1;
		abs_rotations = -rotations_pending;
	}

	// Determine if enough rotation events have occurred for an event
	if (abs_rotations >= dial_divider)
	{
		// CCW event
		if (neg)
		{
			rotations_pending += dial_divider;
			Macro_dialState(Dial_dial1_index, dial_mirror ? ScheduleType_Inc : ScheduleType_Dec);
		}
		// CW
		else
		{
			rotations_pending -= dial_divider;
			Macro_dialState(Dial_dial1_index, dial_mirror ? ScheduleType_Dec : ScheduleType_Inc);
		}
	}


	// Measure ending latency
	Latency_end_time(dialLatencyResource);
}


// Dial post-processing
void Dial_process()
{
	// N/A
}


// Pin interrupt
void PIOA_Handler()
{
	// TODO (HaaTa) Utilize capabilities.kll and possibly ASF pio_handler
	//              This will be necessary to use other ports (B and C)
	// Reading PIOA_ISR will clear interrupt flags
	uint32_t status = REG_PIOA_ISR;

	// One of two pins received the edge
	// This algorithm interrupts on every edge of both pin 1 and pin 2
	// Most algorithms use only a single pin to simplify logic
	// Instead, we convert the value into grey code and then determine the movement
	// This has the advantage of having 2x as many detents.
	if (status & (1 << pin1.pin))
	{
		Dial_stateTracking();
	}
	// While it's possible to have 2 pending edge interrupts, the state transitions will likely not change in time
	// and will just generate a no-op anyways
	if (status & (1 << pin2.pin))
	{
		Dial_stateTracking();
	}
}


// Called by parent scan module whenever the available current changes
// current - mA
void Dial_currentChange(unsigned int current)
{
	// TODO - Any potential power savings?
}


// Called when macro processing has finished
void Dial_finishedWithMacro()
{
}



// ----- CLI Command Functions -----

void cliFunc_dialDebug(char* args)
{
	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation(args, &arg1Ptr, &arg2Ptr);

	// Set the dial debug flag depending on the argument
	// If no argument, set to scan code only
	switch (arg1Ptr[0])
	{
	// No argument
	case '1':
	case '\0':
		dialDebugMode = dialDebugMode != 1 ? 1 : 0;
		break;

	// Invalid argument
	default:
		return;
	}

	print(NL);
	info_msg("Dial Debug Mode: ");
	printInt8(dialDebugMode);
}

void cliFunc_dialDivider(char *args)
{
	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation(args, &arg1Ptr, &arg2Ptr);

	// Default to KLL define
	dial_divider = Dial_Divider_define;

	if (arg1Ptr[0] != '\0')
	{
		dial_divider = numToInt(arg1Ptr);
	}

	print(NL);
	info_msg("Dial Divider: ");
	printInt32(dial_divider);
}
