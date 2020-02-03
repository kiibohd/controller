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
#include <matrix_scan.h>
#include <Lib/delay.h>
#include <Lib/gpio.h>
#include <Lib/periodic.h>
#include <Lib/time.h>

// ASF Includes
#include <sam/drivers/adc/adc.h>
#include <sam/drivers/pmc/pmc.h>
#include <common/services/clock/sysclk.h>

// Local Includes
#include "joystick_scan.h"



// ----- Sense Definition -----

// Channel/Sense list for sequence
// Temperature sensor is always the first element (i.e. always ignore when doing size calculations)
const enum adc_channel_num_t Joystick_channels[] = {
	ADC_TEMPERATURE_SENSOR,
	Joystick_x_channel_define,
	Joystick_y_channel_define,
};



// ----- Defines -----

#define ADCEntry_Size sizeof(Joystick_channels)
#define ADCBuffer_Size 50 // XXX (HaaTa): This is just a buffer it should not be much larger than ADCMaxReadings to make sure that samples don't time drift too much, or too small such that the ADC slows down.
#define ADCMaxValue 4095

#define FunctionIndices 8



// ----- Enumerations -----

typedef enum JoystickMode {
	JoystickMode_Analog = 0,
	JoystickMode_Mouse = 1,
	JoystickMode_Function = 2,
	JoystickMode_Joystick = 3,
} JoystickMode;



// ----- Structs -----

typedef struct ADCReading {
	uint16_t data:12;
	uint8_t  chan:4;
} __attribute__((packed)) ADCReading;

typedef struct ADCTimedEntry {
	Time       time;                // Time that entry started recording (may not be immediate as ADC takes some time to initialize)
	ADCReading data[ADCEntry_Size]; // Array of data entries, matches pre-defined ADC reading sequence order
	uint8_t    done;                // Set to 1 when buffer is complete
} __attribute__((packed)) ADCTimedEntry;

typedef struct ADCBuffer {
	ADCTimedEntry buf[ADCBuffer_Size];
	uint16_t      head;
	uint16_t      tail;
} ADCBuffer;

typedef struct SenseHistory {
	uint16_t min;            // Minimum value seen (generally not used, mostly informational)
	uint16_t max;            // Maximum value seen (generally not used, mostly informational)
	uint16_t prev;           // Previous sample (if 0xFFFF, consider invalid as values only go to 12-bit)
	uint8_t prev_samples;    // Number of samples used to get previous result
	uint8_t scratch_samples; // Number of samples in scratch
	uint32_t scratch;        // Used to gather an average, once this value is good it is set to prev.
} SenseHistory;

// See USB/output_usb.c Output_usbMouse_capability() for usage
typedef struct MouseData {
	uint16_t button;
	int16_t x;
	int16_t y;
} __attribute__ ((packed)) MouseData;

typedef struct FunctionState {
	KeyPosition prev_state;
	KeyPosition cur_state;
	uint32_t prev_decision_time;
} FunctionState;



// ----- Joystick Definition -----

// Convenience Macros
#define Joystick_channelsNum sizeof(Joystick_channels) - 1



// ----- Function Declarations -----

// CLI Functions
void cliFunc_joyDebug(char* args);
void cliFunc_joyEntries(char *args);
void cliFunc_joyInfo(char* args);
void cliFunc_temp(char* args);

#if enableMouse_define == 1
void Output_usbMouse_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args );
#endif



// ----- Variables -----

// Scan Module command dictionary
CLIDict_Entry(joyDebug,  "Enables joystick debug mode, prints out X and Y values.");
CLIDict_Entry(joyEntries, "Displays unprocesed ADC entries");
CLIDict_Entry(joyInfo,   "Print info about the configured joy.");
CLIDict_Entry(temp,         "Displays current MCU temperature.");

CLIDict_Def(joystickCLIDict, "Joystick Module Commands") = {
	CLIDict_Item(joyDebug),
	CLIDict_Item(joyEntries),
	CLIDict_Item(joyInfo),
	CLIDict_Item(temp),
	{ 0, 0, 0 } // Null entry for dictionary end
};

// ADC Buffer
static volatile ADCBuffer Joystick_buffer;

// ADC Status Variable
// Set to 1 if ADC interrupt is enabled, 0 otherwise
static volatile uint8_t Joystick_adc_enabled;

// Temperature reading
static volatile SenseHistory Joystick_temp;

// X/Y readings
static volatile SenseHistory Joystick_x;
static volatile SenseHistory Joystick_y;

// X/Y Scan Code mappings
static const uint16_t Joystick_x_index = Joystick_x_index_define;
static const uint16_t Joystick_y_index = Joystick_y_index_define;

// Function Scan Code mappings
static const uint8_t Joystick_func_index_start = Joystick_func_index_define;

// Function State
static FunctionState Joystick_func_state[FunctionIndices];


// Joystick debug flag - If set to 1, for each keypress the scan code is displayed in hex
//                     If set to 2, for each key state change, the scan code is displayed along with the state
//                     If set to 3, for each scan, update a state table
static volatile uint8_t joystickDebugMode;

// Latency tracking
static volatile uint8_t joystickLatencyResource;

// Counters
static volatile uint32_t total_adc_count;
static volatile uint32_t total_macro_proccessing_count;

// Mode
static volatile JoystickMode Joystick_mode;



// ----- Functions -----

// Calculates the current size of the ADC buffer
static uint16_t ADCBuffer_current_size()
{
	// Wrap-around (tail less than head)
	if ( Joystick_buffer.tail < Joystick_buffer.head )
	{
		return ADCBuffer_Size - Joystick_buffer.head + Joystick_buffer.tail + 1;
	}

	return Joystick_buffer.tail - Joystick_buffer.head;
}

// Retrieves the ADCTimedEntry from the head of the ADC buffer
static volatile ADCTimedEntry *ADCBuffer_head()
{
	return &Joystick_buffer.buf[Joystick_buffer.head];
}

static uint8_t ADCBuffer_valid_head()
{
	// Buffer is empty, cannot increment head
	if (ADCBuffer_current_size() == 0)
	{
		return 0;
	}

	// Make sure we don't pass tail pointer
	if (Joystick_buffer.head == Joystick_buffer.tail)
	{
		return 0;
	}

	uint16_t new_head = Joystick_buffer.head + 1;

	// Wrap-around case
	if (new_head == ADCBuffer_Size)
	{
		// Make sure we don't pass tail pointer
		if (Joystick_buffer.tail == 0)
		{
			return 0;
		}
	}

	return 1;
}

// Increments ADCBuffer head pointer
// Returns 1 if successful, 0 if buffer empty and cannot increment
static uint8_t ADCBuffer_increment_head()
{
	/*
	// Check if a valid head pointer first
	if (!ADCBuffer_valid_head())
	{
		return 0;
	}
	*/

	uint16_t new_head = Joystick_buffer.head + 1;

	// Wrap-around case
	if (new_head == ADCBuffer_Size)
	{
		new_head = 0;
	}

	// Don't set head until we are sure we're ready to commit
	Joystick_buffer.head = new_head;

	return 1;
}

// Increments ADCBuffer tail pointer
// Returns 1 if successful, 0 if buffer full and cannot increment
static uint8_t ADCBuffer_increment_tail()
{
	// Buffer is full, cannot increment tail
	if (ADCBuffer_current_size() >= ADCBuffer_Size)
	{
		return 0;
	}

	uint16_t new_tail = Joystick_buffer.tail + 1;

	// Make sure we don't collide with head pointer
	if (new_tail == Joystick_buffer.head)
	{
		return 0;
	}

	// Wrap-around case
	if (new_tail >= ADCBuffer_Size)
	{
		// Make sure we don't collide with head pointer
		if (Joystick_buffer.head == 0)
		{
			return 0;
		}

		new_tail = 0;
	}

	// Make sure to tag the entry as not done to make sure threaded processing doesn't try to queue
	// immediately
	Joystick_buffer.buf[new_tail].done = 0;

	// Don't set tail until we are sure we've allocated the space
	Joystick_buffer.tail = new_tail;

	return 1;
}


// Setup PDC (Peripheral DMA) pointers for next buffer entry
// We only use the first DMA buffer as it's more important to timestamp data correctly than to get more data more quickly
static void Joystick_buffer_entry_setup(volatile ADCTimedEntry *buffer)
{
	// Set the buffer length (when this reaches 0, ENDRX is triggered)
	// (RXBUFF is triggered when both RCR and RNCR reach 0)
	ADC->ADC_RCR = Joystick_channelsNum + 1; // Includes temp sensor

	// Tag buffer entry
	buffer->time = Time_now();
	buffer->done = 0;

	// Set buffer pointer
	ADC->ADC_RPR = (uint32_t)&(buffer->data);

	// Enable Recieiver Transfer
	ADC->ADC_PTCR = ADC_PTCR_RXTEN;
}

// Setup Joystick ADC channels
volatile uint8_t adc_ready_next;
static volatile Time adc_time;
void Joystick_setup()
{
	// Register Joystick CLI dictionary
	CLI_registerDictionary(joystickCLIDict, joystickCLIDictName);

	// Debug mode
	joystickDebugMode = 0;

	// Default joystick mode
	Joystick_mode = JoystickMode_default_define;

	// ADC disabled initially
	Joystick_adc_enabled = 0;

	// Setup ADC latency timer
	adc_time = Time_now();

	// Reset counters
	total_adc_count = 0;
	total_macro_proccessing_count = 0;

	// Clear ADC Buffer
	Joystick_buffer.head = 0;
	Joystick_buffer.tail = 0;

	// Setup ADC
	pmc_enable_periph_clk(ID_ADC);

	/*
	 * Formula: ADCClock = MCK / ((PRESCAL+1) * 2)
	 *  MCK = 120MHz, PRESCAL = 2, then:
	 *  ADCClock = 120 / ((2+1) * 2) = 20MHz;
	 *  sam4s max ADCClock = 22 MHz
	 *
	 * AVR127: Understanding ADC Parameters (6.1)
	 * After ADC first enabled, or after a wake from sleep
	 *
	 * Formula:
	 *     Startup  Time = startup value / ADCClock
	 *     Startup time = 64 / 20MHz = 3.2 us (4)
	 *     Startup time = 80 / 20MHz = 4 us (5)
	 *     Startup time = 96 / 20MHz = 4.8 us (6)
	 *     Startup time = 112 / 20MHz = 5.6 us (7)
	 *     Startup time = 512 / 20MHz = 25.6 us (8)
	 *     Startup time = 576 / 20MHz = 28.8 us (9)
	 *     Startup time = 640 / 20MHz = 32 us (10)
	 *     Startup time = 704 / 20MHz = 35.2 us (11)
	 *     Startup time = 768 / 20MHz = 38.4 us (12)
	 *     Startup time = 832 / 20MHz = 41.6 us (13)
	 *     Startup time = 896 / 20MHz = 44.8 us (14)
	 *     Startup time = 960 / 20MHz = 48 us (15)
	 *     sam4s Min Startup Time = 4 us (max 12 us)
	 */
	adc_init(ADC, sysclk_get_cpu_hz(), 20000000, ADC_STARTUP_TIME_5);

	/* Set ADC timing.
	 * Formula:
	 *
	 *     Ttrack minimum = 0.054 * Zsource + 205
	 *     Ttrack minimum = 0.054 * 1.5k + 205 = 286 ns
	 *     20MHz -> 50 ns * 15 cycles = 750 ns
	 *     750 ns > 286 ns -> Tracktim can be set to 0
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
	 *     ...
	 *     Tracking Time = (15 + 1) / 20MHz = 800 ns
	 *
	 *     Analog Settling Time
	 *     (TODO May need to tune this)
	 *     Settling Time = 3 / 20MHz = 150 ns (0)
	 *     Settling Time = 5 / 20MHz = 250 ns (1)
	 *     Settling Time = 9 / 20MHz = 450 ns (2)
	 *     Settling Time = 17 / 20MHz = 850 ns (3)
	 */
	const uint8_t tracking_time = 0;
	const uint8_t transfer_period = 2; // Recommended to be set to 2 by datasheet (42.7.2)
	adc_configure_timing(ADC, tracking_time, ADC_SETTLING_TIME_1, transfer_period);

	// Enable channel number tag
	adc_enable_tag(ADC);

	// Set user defined channel sequence
	adc_configure_sequence(ADC, Joystick_channels, sizeof(Joystick_channels));

	// Enable sequencer
	adc_start_sequencer(ADC);

	// Enable channels
	for (uint8_t i = 0; i < sizeof(Joystick_channels); i++)
	{
		adc_enable_channel(ADC, (enum adc_channel_num_t)i);
	}

	// Enable temperature sensor
	adc_enable_ts(ADC);

	// Set gain and offset
	// Not setting gain and offset for temp sensor
	for (uint8_t i = 1; i < sizeof(Joystick_channels); i++)
	{
		// Gain
		adc_set_channel_input_gain(ADC, Joystick_channels[i], ADCGain_define);

		// Offset
		if (ADCOffset_define)
		{
			adc_enable_channel_input_offset(ADC, Joystick_channels[i]);
		}
		else
		{
			adc_disable_channel_input_offset(ADC, Joystick_channels[i]);
		}
	}

	// Allow different gain/offset values for each channel
	adc_enable_anch(ADC);

	// Set Auto Calibration Mode
	adc_set_calibmode(ADC);
	while ((adc_get_status(ADC) & ADC_ISR_EOCAL) != ADC_ISR_EOCAL);

	// Setup first PDC buffer entry
	Joystick_buffer_entry_setup(&Joystick_buffer.buf[0]);

	// Enable PDC channel interrupt.
	adc_enable_interrupt(ADC, ADC_IER_ENDRX); // ADC_IER_RXBUFF may also work

	// Enable ADC interrupt.
	NVIC_EnableIRQ(ADC_IRQn);
	NVIC_SetPriority(ADC_IRQn, ADC_Priority_define);
	Joystick_adc_enabled = 1;

	// Configure trigger mode and starting convention
	adc_configure_trigger(ADC, ADC_TRIG_SW, 0); // Disable hardware trigger

	// Reset SenseHistory
	Joystick_temp.min = 0xFFFF;
	Joystick_temp.max = 0;
	Joystick_x.min = 0xFFFF;
	Joystick_x.max = 0;
	Joystick_y.min = 0xFFFF;
	Joystick_y.max = 0;

	// Start ADC
	adc_start(ADC);

	// Setup latency module
	joystickLatencyResource = Latency_add_resource("Joystick", LatencyOption_Ticks);
}


// Process all ADC samples
void Joystick_process_adc_samples()
{
	// Read out all sense data
	// This buffer can possibly increase in size while processing it.
	while (ADCBuffer_current_size() > 0)
	{
		// Make sure this is a valid head pointer
		if (!ADCBuffer_valid_head())
		{
			break;
		}

		// Grab first entry
		volatile ADCTimedEntry *entry = ADCBuffer_head();

		// Process each of the sense readings
		for (uint8_t sense = 0; sense < ADCEntry_Size; sense++)
		{
			// Get channel reading
			volatile ADCReading *reading = &entry->data[sense];

			// Record of sense data
			volatile SenseHistory *hist;

			// Determine channel
			switch (reading->chan)
			{
			case ADC_TEMPERATURE_SENSOR:
				hist = &Joystick_temp;
				break;
			case Joystick_x_channel_define: // X
				hist = &Joystick_x;
				break;
			case Joystick_y_channel_define: // Y
				hist = &Joystick_y;
				break;
			default: // Invalid
				continue;
			}

			// Check min/max
			if (hist->min > reading->data)
			{
				hist->min = reading->data;
			}
			if (hist->max < reading->data)
			{
				hist->max = reading->data;
			}

			// Accumulate sample
			// Ignore samples beyond limit
			if (hist->scratch_samples < ADCMaxReadings_define)
			{
				hist->scratch += reading->data;
				hist->scratch_samples++;
			}
		}

		// Processing was successful, pop head
		ADCBuffer_increment_head();
	}

	// If ADC buffer has entries free, re-enable
	if (!Joystick_adc_enabled && ADCBuffer_current_size() < ADCBuffer_Size)
	{
		Joystick_adc_enabled = 1;
		NVIC_EnableIRQ(ADC_IRQn);
		adc_start(ADC);
	}
}



// Calculates averages and generates events
void Joystick_periodic()
{
	// Start latency measurement
	Latency_start_time(joystickLatencyResource);

	// Process ADC samples
	Joystick_process_adc_samples();

	// Calculate average values for each of the ADC fields
	uint32_t x, y;
	if (Joystick_x.scratch_samples == ADCMaxReadings_define)
	{
		x = Joystick_x.scratch / ADCMaxReadings_define;

		// Update previous value
		Joystick_x.prev = x;
		Joystick_x.prev_samples = ADCMaxReadings_define;
	}
	// Fewer samples (slower calculation)
	else
	{
		x = Joystick_x.scratch / Joystick_x.scratch_samples;

		// Update previous value
		Joystick_x.prev = x;
		Joystick_x.prev_samples = Joystick_x.scratch_samples;
	}
	if (Joystick_y.scratch_samples == ADCMaxReadings_define)
	{
		y = Joystick_y.scratch / ADCMaxReadings_define;

		// Update previous value
		Joystick_y.prev = y;
		Joystick_y.prev_samples = ADCMaxReadings_define;
	}
	else
	{
		y = Joystick_y.scratch / Joystick_y.scratch_samples;

		// Update previous value
		Joystick_y.prev = y;
		Joystick_y.prev_samples = Joystick_y.scratch_samples;
	}

	// Temperature
	if (Joystick_temp.scratch_samples == ADCMaxReadings_define)
	{
		Joystick_temp.prev = Joystick_temp.scratch / ADCMaxReadings_define;
		Joystick_temp.prev_samples = ADCMaxReadings_define;
	}
	else
	{
		Joystick_temp.prev = Joystick_temp.scratch / Joystick_temp.scratch_samples;
		Joystick_temp.prev_samples = Joystick_temp.scratch_samples;
	}

	// Reset scratch (scratch_samples must be reset last!)
	Joystick_x.scratch = 0;
	Joystick_y.scratch = 0;

	Joystick_x.scratch_samples = 0;
	Joystick_y.scratch_samples = 0;

	// Debug mode
	if (joystickDebugMode)
	{
		print("x");
		printInt16(x);
		print(":y");
		printInt16(y);
		print(NL);
	}

	// Generate KLL events
	switch (Joystick_mode)
	{
	case JoystickMode_Analog:
		Macro_analogState(Joystick_x_index, x);
		Macro_analogState(Joystick_y_index, y);
		break;

	case JoystickMode_Mouse:
	{
#if enableMouse_define == 1
		// 4095 / 2 (positive and negative) / 6 (regions) = 341.25
		const uint8_t regions = 6;
		const uint16_t center = ADCMaxValue / 2;
		const uint16_t divider = center / regions;
		const uint16_t multiplier = 1;

		MouseData mouse;

		// Compute which region of the joystick reading is in
		// https://www.imgpresents.com/joy2mse/guide/html/jtmoperation.html#:~:text=By%20moving%20the%20joystick%2C%20you,to%20act%20as%20a%20mouse!
		mouse.x = (x - center) / divider;
		mouse.y = (y - center) / divider;

		// Apply multplier to increase movement speed
		mouse.x *= multiplier;
		mouse.y *= multiplier;

		// TODO (HaaTa) - Add mouse support to KLL
		Output_usbMouse_capability(NULL, ScheduleType_P, TriggerType_Switch1, (uint8_t*)&mouse);
#endif
		break;
	}
	case JoystickMode_Function:
	{
		const uint16_t center = ADCMaxValue / 2;
		const uint16_t threshold = center / 2;
		uint8_t detected_index = 8; // Default to center (no function)

		// Stabilize / round-off x:y values to smooth readings
		// Round of the bottom 4 bits
		uint16_t x_stablized = x & 0xFF00;
		uint16_t y_stablized = y & 0xFF00;

		// Compute which function index the joystick is currently pointing to
		// 0 - North
		// 1 - North-East
		// 2 - East
		// 3 - South-East
		// 4 - South
		// 5 - South-West
		// 6 - West
		// 7 - North-West
		// 8 - Center (no function)
		int16_t x_relative = x_stablized - center;
		int16_t y_relative = y_stablized - center;

		// Compute absolute value
		uint16_t x_abs = x_relative < 0 ? -x_relative : x_relative;
		uint16_t y_abs = y_relative < 0 ? -y_relative : y_relative;

		// Determine time since last decision
		uint32_t current_time = systick_millis_count;

		// Determine if joystick is being used
		// North / South
		if (y_abs > threshold)
		{
			// Determine if x direction is used
			// North-East, South-East, South-West, North-West
			if (x_abs > threshold)
			{
				// North-East
				if (y_relative > 0 && x_relative > 0)
				{
					detected_index = 1;
				}
				// North-West
				else if (y_relative > 0)
				{
					detected_index = 7;
				}
				// South-East
				else if (x_relative > 0)
				{
					detected_index = 3;
				}
				// South-West
				else
				{
					detected_index = 5;
				}
			}
			// North / South (only)
			else
			{
				// North
				if (y_relative > 0)
				{
					detected_index = 0;
				}
				// South
				else
				{
					detected_index = 4;
				}
			}
		}
		// East / West (only)
		else if (x_abs > threshold)
		{
			// East
			if (x_relative > 0)
			{
				detected_index = 2;
			}
			// West
			else
			{
				detected_index = 6;
			}
		}

		// Evaluate each of the possible function indicies
		for (uint8_t index = 0; index < FunctionIndices; index++)
		{
			FunctionState *func = &Joystick_func_state[index];
			uint32_t last_transition = current_time - func->prev_decision_time;
			// Delay between decisions
			if (last_transition < 500)
				continue;

			// Transition state current -> previous
			func->prev_state = func->cur_state;

			switch (func->prev_state)
			{
			case KeyState_Press:
			case KeyState_Hold:
				if (detected_index == index)
				{
					func->cur_state = KeyState_Hold;
				}
				else
				{
					func->cur_state = KeyState_Release;
				}
				break;

			case KeyState_Release:
			case KeyState_Off:
			default:
				if (detected_index == index)
				{
					func->cur_state = KeyState_Press;
				}
				else
				{
					func->cur_state = KeyState_Off;
				}
				break;
			}

			if (func->cur_state == KeyState_Press)
			{
				print("KEY: ");
				printInt8(index);
				print("  ");
				printInt16(x);
				print(":");
				printInt16(y);
				print("   ");
				printInt16(x_stablized);
				print(":");
				printInt16(y_stablized);
				printNL();
			}
			else if (func->cur_state == KeyState_Release)
			{
				print("RELEASE: ");
				printInt8(index);
				print("  ");
				printInt16(x);
				print(":");
				printInt16(y);
				print("   ");
				printInt16(x_stablized);
				print(":");
				printInt16(y_stablized);
				printNL();
			}

			// Send KLL keystate event
			//Macro_keyState(Joystick_func_index_start + index, func->cur_state);
		}
		break;
	}
	case JoystickMode_Joystick:
		// TODO - Use HID-IO
		break;
	}

	// Measure ending latency
	Latency_end_time(joystickLatencyResource);
}


// ADC Interrupt Handler
void ADC_Handler()
{
	// RCR depleted (buffer entry filled)
	if ((adc_get_status(ADC) & ADC_ISR_ENDRX) == ADC_ISR_ENDRX)
	{
		// Mark buffer entry as complete
		Joystick_buffer.buf[Joystick_buffer.tail].done = 1;

		// Make sure the overall buffer hasn't filled up
		// If the buffer is full, don't start the next ADC capture
		if (ADCBuffer_increment_tail())
		{
			// ADC Counter stat
			total_adc_count++;

			// Increment tail counter and setup next buffer entry
			Joystick_buffer_entry_setup(&Joystick_buffer.buf[Joystick_buffer.tail]);

			// Trigger ADC
			adc_start(ADC);
			return;
		}
		/*
		else
		{
			warn_print("ADC Joystick buffer is full... ");
			printInt16(ADCBuffer_current_size());
			print(" Head: ");
			printInt16(Joystick_buffer.head);
			print(" Tail: ");
			printInt16(Joystick_buffer.tail);
			print(NL);
		}
		*/

		// Pause ADC transfer until buffer has room
		Joystick_adc_enabled = 0;
		NVIC_DisableIRQ(ADC_IRQn);
	}
}


// Joystick post-processing
void Joystick_process()
{
	// N/A
}


// Called by parent scan module whenever the available current changes
// current - mA
void Joystick_currentChange(unsigned int current)
{
	// TODO - Any potential power savings?
}


// Called when macro processing has finished
void Joystick_finishedWithMacro()
{
	// Increment counter
	total_macro_proccessing_count++;
}



// ----- CLI Command Functions -----

void cliFunc_joyInfo(char* args)
{
	print(NL);
	info_print("Channels: ");
	printInt8(Joystick_channelsNum);
	print(NL);
	info_print("ADC Buffer Size: ");
	printInt16(ADCBuffer_Size);
	print(" (");
	printInt16(ADCBuffer_current_size());
	print(" Entries)");
	print(" Head: ");
	printInt16(Joystick_buffer.head);
	print(" Tail: ");
	printInt16(Joystick_buffer.tail);
	print(NL);
	info_print("ADC Enabled: ");
	printInt8(Joystick_adc_enabled);
	print(NL);
	info_print("Total ADC Count: ");
	printInt32(total_adc_count);
	print(NL);
	info_print("Total Macro Processing Count: ");
	printInt32(total_macro_proccessing_count);
	print(NL);
	info_print("Joystick X Stats: min:");
	printInt16(Joystick_x.min);
	print(" max:");
	printInt16(Joystick_x.max);
	print(" prev:");
	printInt16(Joystick_x.prev);
	print(" prev_samples:");
	printInt8(Joystick_x.prev_samples);
	print(" scratch:");
	printInt32(Joystick_x.scratch);
	print(" scratch_samples:");
	printInt8(Joystick_x.scratch_samples);
	print(NL);
	info_print("Joystick Y Stats: min:");
	printInt16(Joystick_y.min);
	print(" max:");
	printInt16(Joystick_y.max);
	print(" prev:");
	printInt16(Joystick_y.prev);
	print(" prev_samples:");
	printInt8(Joystick_y.prev_samples);
	print(" scratch:");
	printInt32(Joystick_y.scratch);
	print(" scratch_samples:");
	printInt8(Joystick_y.scratch_samples);
}

void cliFunc_joyDebug(char* args)
{
	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation(args, &arg1Ptr, &arg2Ptr);

	// Set the joystick debug flag depending on the argument
	// If no argument, set to scan code only
	switch (arg1Ptr[0])
	{
	// No argument
	case '1':
	case '\0':
		joystickDebugMode = joystickDebugMode != 1 ? 1 : 0;
		break;

	// Invalid argument
	default:
		return;
	}

	print(NL);
	info_print("Joystick Debug Mode: ");
	printInt8(joystickDebugMode);
}

void cliFunc_temp(char* args)
{
	// Read all the values first (as they may change during the calculations)
	uint32_t orig_min = Joystick_temp.min;
	uint32_t orig_max = Joystick_temp.max;
	uint32_t orig_cur = Joystick_temp.prev;

	// Convert to voltage (mV)
	uint32_t min_adj = orig_min * ADCVRef_define / ADCMaxValue;
	uint32_t max_adj = orig_max * ADCVRef_define / ADCMaxValue;
	uint32_t cur_adj = orig_cur * ADCVRef_define / ADCMaxValue;

	// Offset for Typical voltage at 27 C (mV) 1.44 V -> 1440 mV
	const uint16_t volt_offset = 1440;
	int32_t min = min_adj - volt_offset;
	int32_t max = max_adj - volt_offset;
	int32_t cur = cur_adj - volt_offset;

	// Multiplier for decimal point
	const uint32_t mul = 10;

	// Convert to C (4.7 mV/C) -> 47 (multiply by 10 first)
	const uint32_t celsius_conv = 4.7 * mul;
	min = min * mul / celsius_conv;
	max = max * mul / celsius_conv;
	cur = cur * mul / celsius_conv;

	// Offset to 27 C (x10 to give one decimal place)
	const uint32_t temp_offset = 27 * mul;
	min += temp_offset;
	max += temp_offset;
	cur += temp_offset;

	// Print results
	print(NL);
	info_print("Temperature");
	print(NL "      Min: ");

	printDecimal32(min, mul);
	print(" C (");
	printInt16(orig_min);
	print("->");
	printInt16(min_adj);
	print(")" NL "      Max: ");
	printDecimal32(max, mul);
	print(" C (");
	printInt16(orig_max);
	print("->");
	printInt16(max_adj);
	print(")" NL "  Current: ");
	printDecimal32(cur, mul);
	print(" C (");
	printInt16(orig_cur);
	print("->");
	printInt16(cur_adj);
	print(")");
}

void cliFunc_joyEntries(char *args)
{
	// Check if there are no entries to display
	if (!ADCBuffer_valid_head())
	{
		return;
	}

	// Iterate over adc buffer entries
	uint16_t count = 0;
	for (uint16_t pos = Joystick_buffer.head; pos != Joystick_buffer.tail;)
	{
		print(NL);
		volatile ADCTimedEntry *entry = &Joystick_buffer.buf[pos];
		printInt16(count);
		print(":");
		printTime(entry->time);
		print(" ");
		printInt8(entry->done);

		// Increment, handling wrap-around
		pos++;
		if (pos > ADCBuffer_Size)
		{
			pos = 0;
		}
		count++;
	}
}

