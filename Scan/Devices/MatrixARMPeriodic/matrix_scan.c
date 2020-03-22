/* Copyright (C) 2014-2020 by Jacob Alexander
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
#include <Lib/ScanLib.h>

// Project Includes
#include <cli.h>
#include <kll_defs.h>
#include <kll.h>
#include <latency.h>
#include <led.h>
#include <print.h>
#include <macro.h>
#include <Lib/delay.h>
#include <Lib/gpio.h>
#include <Lib/periodic.h>
#include <Lib/time.h>

#if enableRawIO_define == 1
#include <hidio_com.h>
#endif

// Local Includes
#include "matrix_scan.h"



// ----- Defines -----

#define TickStore_MaxTicks 255



// ----- Matrix Definition -----
// Scan Type
const GPIO_Config Matrix_type = ScanCodeMatrixType_define;

// Matrix
const GPIO_Pin Matrix_cols[] = { ScanCodeStrobeList_define };
const GPIO_Pin Matrix_rows[] = { ScanCodeSenseList_define };

// Convenience Macros
#define Matrix_colsNum sizeof( Matrix_cols ) / sizeof( GPIO_Pin )
#define Matrix_rowsNum sizeof( Matrix_rows ) / sizeof( GPIO_Pin )
#define Matrix_maxKeys sizeof( Matrix_scanArray ) / sizeof( KeyState )



// ----- Function Declarations -----

// CLI Functions
void cliFunc_debounce( char* args );
void cliFunc_matrixDebug( char* args );
void cliFunc_matrixInfo( char* args );
void cliFunc_matrixState( char* args );
void cliFunc_strobeDelay( char* args );



// ----- Variables -----

// Scan Module command dictionary
CLIDict_Entry( debounce,     "Set the debounce timer (ms). Useful for bouncy switches." );
CLIDict_Entry( matrixDebug,  "Enables matrix debug mode, prints out each scan code." NL "\t\tIf argument \033[35mT\033[0m is given, prints out each scan code state transition." );
CLIDict_Entry( matrixInfo,   "Print info about the configured matrix." );
CLIDict_Entry( matrixState,  "Prints out the current scan table N times." NL "\t\t \033[1mO\033[0m - Off, \033[1;33mP\033[0m - Press, \033[1;32mH\033[0m - Hold, \033[1;35mR\033[0m - Release, \033[1;31mI\033[0m - Invalid" );
CLIDict_Entry( strobeDelay,  "Set the strobe delay (us). Useful for bad pullup resistors." );

CLIDict_Def( matrixCLIDict, "Matrix Module Commands" ) = {
	CLIDict_Item( debounce ),
	CLIDict_Item( matrixDebug ),
	CLIDict_Item( matrixInfo ),
	CLIDict_Item( matrixState ),
	CLIDict_Item( strobeDelay ),
	{ 0, 0, 0 } // Null entry for dictionary end
};

// Debounce Array
static volatile KeyState Matrix_scanArray[ Matrix_colsNum * Matrix_rowsNum ];


#if ScanCodeRemapping_define == 1
// ScanCode Remapping Array
static const uint16_t matrixScanCodeRemappingMatrix[] = { ScanCodeRemappingMatrix_define };
#endif


// Matrix debug flag - If set to 1, for each keypress the scan code is displayed in hex
//                     If set to 2, for each key state change, the scan code is displayed along with the state
//                     If set to 3, for each scan, update a state table
static volatile uint8_t matrixDebugMode;

// Matrix State Table Debug Counter - If non-zero display state table after every matrix scan
static volatile uint16_t matrixDebugStateCounter;

// Matrix Current Strobe
static volatile uint8_t matrixCurrentStrobe;

// System Timer used for delaying debounce decisions
extern volatile uint32_t systick_millis_count;

// Latency tracking
static volatile uint8_t matrixLatencyResource;

// Debounce expiry time
static volatile uint8_t debounceExpiryTime;

// Strobe delay setting
static volatile uint8_t strobeDelayTime;

// Activity/Inactivity Tick Stores
static TickStore activity_tickstore;
static TickStore inactivity_tickstore;
static Time activity_tick_duration;

// Activity counter - Keeps track of state changes (used to determine Activity/Inactivity)
// Press count is for inactivity reset
// Release count is for activity reset
static volatile uint16_t matrixStateActiveCount;
static volatile uint16_t matrixStatePressCount;
static volatile uint16_t matrixStateReleaseCount;



// ----- Functions -----

// Setup GPIO pins for matrix scanning
void Matrix_setup()
{
	// Register Matrix CLI dictionary
	CLI_registerDictionary( matrixCLIDict, matrixCLIDictName );

#if defined(_sam_)
	// 31.5.8 Reading the I/O line levels requires the clock of the PIO Controller to be enabled
	PMC->PMC_PCER0 = (1 << ID_PIOA) | (1 << ID_PIOB);
#endif

	// Setup Strobe Pins
	for ( uint8_t c = 0; c < Matrix_colsNum; c++ )
	{
		GPIO_ConfigPin pin = {
			.port = Matrix_cols[c].port,
			.pin = Matrix_cols[c].pin,
#if defined(_sam_)
			.peripheral = GPIO_Peripheral_Output0,
#endif
		};
		PIO_Setup(pin); // Enables GPIO controlled by system mux
		GPIO_Ctrl(Matrix_cols[c], GPIO_Type_DriveSetup, Matrix_type);
	}

	// Setup Sense Pins
	for ( uint8_t c = 0; c < Matrix_rowsNum; c++ )
	{
		GPIO_ConfigPin pin = {
			.port = Matrix_rows[c].port,
			.pin = Matrix_rows[c].pin,
#if defined(_sam_)
			.peripheral = GPIO_Peripheral_Input,
#endif
		};
		PIO_Setup(pin); // Enables GPIO controlled by system mux
		GPIO_Ctrl(Matrix_rows[c], GPIO_Type_ReadSetup, Matrix_type);
	}

	// Clear out Debounce Array
	for ( uint8_t item = 0; item < Matrix_maxKeys; item++ )
	{
		Matrix_scanArray[ item ].prevState        = KeyState_Off;
		Matrix_scanArray[ item ].curState         = KeyState_Off;
		Matrix_scanArray[ item ].activeCount      = 0;
		Matrix_scanArray[ item ].inactiveCount    = DebounceDivThreshold; // Start at 'off' steady state
		Matrix_scanArray[ item ].prevDecisionTime = 0;
	}

	// Reset strobe position
	matrixCurrentStrobe = 0;

	// Debug mode
	matrixDebugMode = 0;

	// Debug counter reset
	matrixDebugStateCounter = 0;

	// Debounce expiry time
	debounceExpiryTime = MinDebounceTime_define;

	// Strobe delay setting
	strobeDelayTime = StrobeDelay_define;

	// Setup tick duration (multiples of 1 second)
	activity_tick_duration = Time_init();
	activity_tick_duration.ms = 1000 * ActivityTimerMultiplier_define;

	// Setup Activity and Inactivity tick resources
	Time_tick_start( &activity_tickstore, activity_tick_duration, TickStore_MaxTicks );
	Time_tick_start( &inactivity_tickstore, activity_tick_duration, TickStore_MaxTicks );

	// Clear matrixStateActiveCount for activity check
	matrixStateActiveCount = 0;
	matrixStatePressCount = 0;
	matrixStateReleaseCount = 0;

	// Setup latency module
	matrixLatencyResource = Latency_add_resource("MatrixARMPeri", LatencyOption_Ticks);
}


// Show KeyPosition state
void Matrix_keyPositionDebug( KeyPosition pos )
{
	// Depending on the state, use a different flag + color
	switch ( pos )
	{
	case KeyState_Off:
		print("\033[1mO\033[0m");
		break;

	case KeyState_Press:
		print("\033[1;33mP\033[0m");
		break;

	case KeyState_Hold:
		print("\033[1;32mH\033[0m");
		break;

	case KeyState_Release:
		print("\033[1;35mR\033[0m");
		break;

	case KeyState_Invalid:
	default:
		print("\033[1;31mI\033[0m");
		break;
	}
}


// Number of strobe columns
inline uint8_t Matrix_totalColumns()
{
	return Matrix_colsNum;
}


// Single strobe matrix scan
// Only goes through a single strobe
// This module keeps track of the next strobe to scan
uint8_t Matrix_single_scan()
{
	// Start latency measurement
	Latency_start_time( matrixLatencyResource );


	// Read systick for event scheduling
	uint32_t currentTime = systick_millis_count;

	// Current strobe
	uint8_t strobe = matrixCurrentStrobe;

	// XXX (HaaTa)
	// Before strobing drain each sense line
	// This helps with faulty pull-up resistors (particularily with SAM4S)
	for ( uint8_t sense = 0; sense < Matrix_rowsNum; sense++ )
	{
		GPIO_Ctrl( Matrix_rows[ sense ], GPIO_Type_DriveSetup, Matrix_type );
#if ScanCodeMatrixInvert_define == 2 // GPIO_Config_Pulldown
		GPIO_Ctrl( Matrix_rows[ sense ], GPIO_Type_DriveLow, Matrix_type );
#elif ScanCodeMatrixInvert_define == 1 // GPIO_Config_Pullup
		GPIO_Ctrl( Matrix_rows[ sense ], GPIO_Type_DriveHigh, Matrix_type );
#endif
		GPIO_Ctrl( Matrix_rows[ sense ], GPIO_Type_ReadSetup, Matrix_type );
	}

	// Strobe Pin
	GPIO_Ctrl( Matrix_cols[ strobe ], GPIO_Type_DriveHigh, Matrix_type );

	// Used to allow the strobe signal to propagate, generally not required
	if ( strobeDelayTime > 0 )
	{
		delay_us( strobeDelayTime );
	}

	// Scan each of the sense pins
	for ( uint8_t sense = 0; sense < Matrix_rowsNum; sense++ )
	{
		// Key position
		uint16_t key = Matrix_colsNum * sense + strobe;
#if ScanCodeRemapping_define == 1
		uint16_t key_disp = matrixScanCodeRemappingMatrix[key];
#else
		uint16_t key_disp = key + 1; // 1-indexed for reporting purposes
#endif

		// Check bounds, before attempting to scan
		// 1-indexed as ScanCode 0 is not used
		if ( key_disp > MaxScanCode_KLL || key_disp == 0 )
		{
			continue;
		}
		volatile KeyState *state = &Matrix_scanArray[ key ];

		// Signal Detected
		// Increment count and right shift opposing count
		// This means there is a maximum of scan 13 cycles on a perfect off to on transition
		//  (coming from a steady state 0xFFFF off scans)
		// Somewhat longer with switch bounciness
		// The advantage of this is that the count is ongoing and never needs to be reset
		// State still needs to be kept track of to deal with what to send to the Macro module
		// Compared against the default state value (ScanCodeMatrixInvert_define), usually 0
		if ( GPIO_Ctrl( Matrix_rows[ sense ], GPIO_Type_Read, Matrix_type ) != ScanCodeMatrixInvert_define )
		{
			// Only update if not going to wrap around
			if ( state->activeCount < DebounceDivThreshold ) state->activeCount += 1;
			state->inactiveCount >>= 1;
		}
		// Signal Not Detected
		else
		{
			// Only update if not going to wrap around
			if ( state->inactiveCount < DebounceDivThreshold ) state->inactiveCount += 1;
			state->activeCount >>= 1;
		}

		// Check for state change
		// But only if:
		// 1) Enough time has passed since last state change
		// 2) Either active or inactive count is over the debounce threshold

		// Update previous state
		state->prevState = state->curState;

		// Determine time since last decision
		uint32_t lastTransition = currentTime - state->prevDecisionTime;

		// Attempt state transition
		switch ( state->prevState )
		{
		case KeyState_Press:
		case KeyState_Hold:
			if ( state->activeCount > state->inactiveCount )
			{
				state->curState = KeyState_Hold;
			}
			else
			{
				// If not enough time has passed since Hold
				// Keep previous state
				if ( lastTransition < debounceExpiryTime )
				{
					state->curState = state->prevState;
					Macro_keyState( key_disp, state->curState );
					continue;
				}

				state->curState = KeyState_Release;
			}
			break;

		case KeyState_Release:
		case KeyState_Off:
			if ( state->activeCount > state->inactiveCount )
			{
				// If not enough time has passed since Hold
				// Keep previous state
				if ( lastTransition < debounceExpiryTime )
				{
					state->curState = state->prevState;
					Macro_keyState( key_disp, state->curState );
					continue;
				}

				state->curState = KeyState_Press;
			}
			else
			{
				state->curState = KeyState_Off;
			}
			break;

		case KeyState_Invalid:
		default:
			erro_msg("Matrix scan bug!! Report me! - ");
			printHex( state->prevState );
			print(" Col: ");
			printHex( strobe );
			print(" Row: ");
			printHex( sense );
			print(" Key: ");
			printHex( key_disp );
			print( NL );
			break;
		}

		// Update decision time
		state->prevDecisionTime = currentTime;

		// Send keystate to macro module
		Macro_keyState( key_disp, state->curState );

		// Check for activity and inactivity
		if ( state->curState != KeyState_Off )
		{
			matrixStateActiveCount++;
		}
		switch ( state->curState )
		{
		case KeyState_Press:
			matrixStatePressCount++;
			break;

		case KeyState_Release:
			matrixStateReleaseCount++;
			break;

		default:
			break;
		}

		// Matrix Debug, only if there is a state change
		if ( matrixDebugMode && state->curState != state->prevState )
		{
			// Basic debug output
			if ( matrixDebugMode == 1 && state->curState == KeyState_Press )
			{
				printInt16( key_disp );
				print(":");
				printHex( key_disp );
				print(" ");
#if enableRawIO_define == 1
				HIDIO_print_flush();
#endif
			}
			// State transition debug output
			else if ( matrixDebugMode == 2 )
			{
				printInt16( key_disp );
				Matrix_keyPositionDebug( state->curState );
				print(" ");
#if enableRawIO_define == 1
				HIDIO_print_flush();
#endif
			}
			else if ( matrixDebugMode == 3 )
			{
				print("\033[1m");
				printInt16( key_disp );
				print("\033[0m");
				print(":");
				Matrix_keyPositionDebug( Matrix_scanArray[ key ].prevState );
				Matrix_keyPositionDebug( Matrix_scanArray[ key ].curState );
				print(" 0x");
				printHex_op( state->activeCount, 2 );
				print(" 0x");
				printHex_op( state->inactiveCount, 2 );
				print(" ");
				printInt32( lastTransition );
				print( NL );
			}

		}
	}

	// Unstrobe Pin
	GPIO_Ctrl( Matrix_cols[ strobe ], GPIO_Type_DriveLow, Matrix_type );

	// Measure ending latency
	Latency_end_time( matrixLatencyResource );


	// State Table Output Debug
	if ( matrixDebugStateCounter > 0 )
	{
		// Decrement counter
		matrixDebugStateCounter--;

		// Display the state info for each key
		print("<key>:<previous state><current state> <active count> <inactive count>");
		for ( uint8_t key = 0; key < Matrix_maxKeys; key++ )
		{
			// Every 4 keys, put a newline
			if ( key % 5 == 0 )
				print( NL );

			print("\033[1m");
			printInt16( key + 1 );
			print("\033[0m");
			print(":");
			Matrix_keyPositionDebug( Matrix_scanArray[ key ].prevState );
			Matrix_keyPositionDebug( Matrix_scanArray[ key ].curState );
			print(" 0x");
			printHex_op( Matrix_scanArray[ key ].activeCount, 2 );
			print(" 0x");
			printHex_op( Matrix_scanArray[ key ].inactiveCount, 2 );
			print(" ");
		}

		print( NL );
	}


	// Increment strobe, and allow matrix processing
	if ( ++matrixCurrentStrobe >= Matrix_colsNum )
	{
		matrixCurrentStrobe = 0;

		// Reset activity counter on a press
		// TODO (HaaTa) Extra 0 id signals may occur (active and inactive)
		if ( matrixStatePressCount > 0 && matrixStateReleaseCount == 0 )
		{
			Time_tick_reset( &inactivity_tickstore );
		}

		// Reset inactivity counter on a release
		if ( matrixStateReleaseCount > 0 && matrixStatePressCount == 0 )
		{
			Time_tick_reset( &activity_tickstore );
		}

		// Do Activity/Inactivity checking now that a full set of strobes has completed
		if ( matrixStateActiveCount > 0 )
		{
			if ( activity_tickstore.fresh_store )
			{
				Time_tick_reset( &inactivity_tickstore );
			}

			// Activity detected
			Macro_tick_update( &activity_tickstore, TriggerType_Active1 );
		}
		else
		{
			if ( inactivity_tickstore.fresh_store )
			{
				Time_tick_reset( &activity_tickstore );
			}

			// Inactivity detected
			Macro_tick_update( &inactivity_tickstore, TriggerType_Inactive1 );
		}

		// Finally reset the state change count
		matrixStateActiveCount = 0;
		matrixStatePressCount = 0;
		matrixStateReleaseCount = 0;

		return 1;
	}

	// No matrix processing yet
	return 0;
}


// Start periodic timer
void Matrix_start()
{
	// Set number of cycles to wait between scans
	Periodic_init( PeriodicCycles_define );
}


// Called by parent scan module whenever the available current changes
// current - mA
void Matrix_currentChange( unsigned int current )
{
	// TODO - Any potential power savings?
}



// ----- CLI Command Functions -----

void cliFunc_debounce( char* args )
{
	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	if ( arg1Ptr[0] != '\0' )
	{
		debounceExpiryTime = (uint8_t)numToInt( arg1Ptr );
	}

	print( NL );
	info_msg("Debounce Timer: ");
	printInt8( debounceExpiryTime );
	print("ms");
}

void cliFunc_matrixInfo( char* args )
{
	print( NL );
	info_msg("Columns:  ");
	printInt8( Matrix_colsNum );

	print( NL );
	info_msg("Rows:     ");
	printInt8( Matrix_rowsNum );

	print( NL );
	info_msg("Max Keys: ");
	printInt8( Matrix_maxKeys );
}

void cliFunc_matrixDebug( char* args )
{
	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	// Set the matrix debug flag depending on the argument
	// If no argument, set to scan code only
	// If set to T, set to state transition
	switch ( arg1Ptr[0] )
	{
	// T as argument
	case 'T':
	case 't':
	case '2':
		matrixDebugMode = matrixDebugMode != 2 ? 2 : 0;
		break;

	case '3':
		matrixDebugMode = matrixDebugMode != 3 ? 3 : 0;
		break;

	// No argument
	case '1':
	case '\0':
		matrixDebugMode = matrixDebugMode != 1 ? 1 : 0;
		break;

	// Invalid argument
	default:
		return;
	}

	print( NL );
	info_msg("Matrix Debug Mode: ");
	printInt8( matrixDebugMode );
}

void cliFunc_matrixState( char* args )
{
	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	// Default to 1 if no argument is given
	matrixDebugStateCounter = 1;

	if ( arg1Ptr[0] != '\0' )
	{
		matrixDebugStateCounter = (uint16_t)numToInt( arg1Ptr );
	}
}

void cliFunc_strobeDelay( char* args )
{
	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	if ( arg1Ptr[0] != '\0' )
	{
		strobeDelayTime = (uint8_t)numToInt( arg1Ptr );
	}

	print( NL );
	info_msg("Strobe Delay: ");
	printInt8( strobeDelayTime );
	print("us");
}

