/* Copyright (C) 2011-2018 by Jacob Alexander
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
#include <Lib/MainLib.h>

// Project Includes
#include <macro.h>
#include <scan_loop.h>
#include <output_com.h>

#include <cli.h>
#include <latency.h>
#include <led.h>
#include <print.h>

#include <Lib/periodic.h>
#include <Lib/sysview.h>
#include <Lib/storage.h>



// ----- Enumerations -----

typedef enum PeriodicStage {
	PeriodicStage_Scan,
	PeriodicStage_Macro,
	PeriodicStage_Output,
	PeriodicStage_Screensaver,
} PeriodicStage;



// ----- Variables -----

// Periodic Stage Tracker
static volatile PeriodicStage stage_tracker;

//Number of times a complete periodic function rotation need to be run to try to check screensaver timing
static volatile uint8_t screensaver_div;



// ----- Functions -----

// Run periodically at a consistent time rate
// Used to process events that need to be run at regular intervals
// And have negative effect being delayed or stretched too much
//
// Returns 1 if full rotation has completed
// Returns 0 otherwise
int main_periodic()
{
	// Scan module periodic routines
	switch ( stage_tracker )
	{
	case PeriodicStage_Scan:
		// Returns non-zero if ready to process macros
		SEGGER_SYSVIEW_OnTaskStartExec(TASK_SCAN_PERIODIC);
		if ( Scan_periodic() )
		{
			stage_tracker = PeriodicStage_Macro;
		}
		SEGGER_SYSVIEW_OnTaskTerminate(TASK_SCAN_PERIODIC);
		break;

	case PeriodicStage_Macro:
		// Run Macros over Key Indices and convert to USB Keys
		SEGGER_SYSVIEW_OnTaskStartExec(TASK_MACRO_PERIODIC);
		Macro_periodic();
		stage_tracker = PeriodicStage_Output;
		SEGGER_SYSVIEW_OnTaskTerminate(TASK_MACRO_PERIODIC);
		break;

	case PeriodicStage_Output:
		// Send periodic USB results
		SEGGER_SYSVIEW_OnTaskStartExec(TASK_OUTPUT_PERIODIC);
		Output_periodic();
		stage_tracker = PeriodicStage_Screensaver;
		SEGGER_SYSVIEW_OnTaskTerminate(TASK_OUTPUT_PERIODIC);
		break;

	case PeriodicStage_Screensaver:
		if ( !screensaver_div-- ) {
			screensaver_div = 100;
			Screensaver_periodic();
		}
		stage_tracker = PeriodicStage_Scan;

		// Full rotation
		return 1;
	}

	return 0;
}

// ----- MCU-only Functions -----
#if !defined(_host_)

int main()
{
#ifdef SEGGER_SYSVIEW_H
	SEGGER_SYSVIEW_Conf();
	SEGGER_SYSVIEW_OnTaskCreate(TASK_SCAN_PERIODIC);
	SEGGER_SYSVIEW_OnTaskCreate(TASK_MACRO_PERIODIC);
	SEGGER_SYSVIEW_OnTaskCreate(TASK_OUTPUT_PERIODIC);
	SEGGER_SYSVIEW_OnTaskCreate(TASK_CLI_PROCESS);
	SEGGER_SYSVIEW_OnTaskCreate(TASK_SCAN_POLL);
	SEGGER_SYSVIEW_OnTaskCreate(TASK_MACRO_POLL);
	SEGGER_SYSVIEW_OnTaskCreate(TASK_OUTPUT_POLL);
#endif

	// Setup Latency Measurements
	Latency_init();

	// Enable CLI
	CLI_init();

	// Setup periodic timer function
	Periodic_function( &main_periodic );

#if Storage_Enable_define == 1
	Storage_init();
#endif

	// Setup Modules
	Output_setup();
	Macro_setup();
	Scan_setup();

#if Storage_Enable_define == 1
	storage_load_settings();
#endif

	// Start scanning on first periodic loop
	stage_tracker = PeriodicStage_Scan;

	// Fill defautl screensaver div value
	screensaver_div = 100;

#if DEBUG_RESETS
	// Blink to indicate a reset happened
	errorLED(0);
	delay_ms(500);
	errorLED(1);
	delay_ms(500);
	errorLED(0);
#endif

	// Main Detection Loop
	while ( 1 )
	{
		// Run constantly
		// Used to process things such as the cli and output module (i.e. USB).
		// Should not be used to run things that require consistent timing.
		// While counter-intuitive, things such as LED/Display modules should be run as poll
		// as they need to run as quickly as possible, in case there needs to be frame drops

		// Process CLI
		SEGGER_SYSVIEW_OnTaskStartExec(TASK_CLI_PROCESS);
		CLI_process();
		SEGGER_SYSVIEW_OnTaskTerminate(TASK_CLI_PROCESS);

		//SEGGER_SYSVIEW_OnIdle();

		// Scan module poll routines
		SEGGER_SYSVIEW_OnTaskStartExec(TASK_SCAN_POLL);
		Scan_poll();
		SEGGER_SYSVIEW_OnTaskTerminate(TASK_SCAN_POLL);

		//SEGGER_SYSVIEW_OnIdle();

		// Macro module poll routines
		SEGGER_SYSVIEW_OnTaskStartExec(TASK_MACRO_POLL);
		Macro_poll();
		SEGGER_SYSVIEW_OnTaskTerminate(TASK_MACRO_POLL);

		// Output module poll routines
		SEGGER_SYSVIEW_OnTaskStartExec(TASK_OUTPUT_POLL);
		Output_poll();
		SEGGER_SYSVIEW_OnTaskTerminate(TASK_OUTPUT_POLL);

		SEGGER_SYSVIEW_OnIdle();

#if defined(_sam_)
		// Not locked up... Reset the watchdog timer
		WDT->WDT_CR = WDT_CR_KEY_PASSWD | WDT_CR_WDRSTT;
#elif defined(_kinetis_)
		// Not locked up... Reset the watchdog timer
		if ( WDOG_STCTRLH_WDOGEN && WDOG_TMROUTL > 2 )
		{
			__disable_irq();
			WDOG_REFRESH = WDOG_REFRESH_SEQ1;
			WDOG_REFRESH = WDOG_REFRESH_SEQ2;
			__enable_irq();
		}
#endif
	}
}

#endif

// ----- Host-only Functions -----
#if defined(_host_)

int Host_init()
{
	// Enable CLI
	CLI_init();

	// Setup Modules
	Output_setup();
	Macro_setup();
	Scan_setup();

	// Start scanning on first periodic loop
	stage_tracker = PeriodicStage_Scan;

	return 1;
}

int Host_cli_process()
{
	// Process CLI
	return CLI_process();
}

int Host_periodic()
{
	return main_periodic();
}

int Host_poll()
{
	// Run constantly
	// Used to process things such as the cli and output module (i.e. USB).
	// Should not be used to run things that require consistent timing.
	// While counter-intuitive, things such as LED/Display modules should be run as poll
	// as they need to run as quickly as possible, in case there needs to be frame drops

	// Process CLI
	CLI_process();

	// Scan module poll routines
	Scan_poll();

	// Macro module poll routines
	Macro_poll();

	// Output module poll routines
	Output_poll();

	return 1;
}

int Host_process()
{
	// Run periodic loop for a full rotation
	while ( !Host_periodic() );

	// Then a single poll loop
	Host_poll();

	return 1;
}

// Register callback for system calls
int Host_register_callback( void* func )
{
	Output_Host_Callback = func;
	return 1;
}

// Change the value of systick (milliseconds)
volatile uint32_t systick_millis_count;
int Host_set_systick( uint32_t systick_ms )
{
	systick_millis_count = systick_ms;
	return 1;
}

// Change the nanosecs since last systick
volatile uint32_t ns_since_systick_count;
int Host_set_nanosecs_since_systick( uint32_t systick_ns )
{
	ns_since_systick_count = systick_ns;
	return 1;
}

// Test function to validate library
int Host_callback_test()
{
	// Print twice to validate callback
	print("Test 1 ");
	print("Test 2 ");
	print( NL );
	print("Systick Millisecond: ");
	printHex32( systick_millis_count );
	print( NL );
	return 9;
}

#endif

