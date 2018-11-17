/* Copyright (C) 2014-2017 by Jacob Alexander
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


/*
 * NOTE! This has been depricated in favor of the MatrixARMPeriodic implementation.
 */


// ----- Includes -----

// Compiler Includes
#include <Lib/ScanLib.h>

// Project Includes
#include <cli.h>
#include <kll_defs.h>
#include <latency.h>
#include <led.h>
#include <print.h>
#include <macro.h>
#include <Lib/delay.h>

// Local Includes
#include "matrix_scan.h"

// Matrix Configuration
#include <matrix.h>



// ----- Defines -----

#if ( DebounceThrottleDiv_define > 0 )
nat_ptr_t Matrix_divCounter = 0;
#endif

#if StrobeDelay_define > 0 && !defined( STROBE_DELAY )
#define STROBE_DELAY StrobeDelay_define
#endif



// ----- Function Declarations -----

// CLI Functions
void cliFunc_matrixDebug( char* args );
void cliFunc_matrixInfo( char* args );
void cliFunc_matrixState( char* args );



// ----- Variables -----

// Scan Module command dictionary
CLIDict_Entry( matrixDebug,  "Enables matrix debug mode, prints out each scan code." NL "\t\tIf argument \033[35mT\033[0m is given, prints out each scan code state transition." );
CLIDict_Entry( matrixInfo,   "Print info about the configured matrix." );
CLIDict_Entry( matrixState,  "Prints out the current scan table N times." NL "\t\t \033[1mO\033[0m - Off, \033[1;33mP\033[0m - Press, \033[1;32mH\033[0m - Hold, \033[1;35mR\033[0m - Release, \033[1;31mI\033[0m - Invalid" );

CLIDict_Def( matrixCLIDict, "Matrix Module Commands" ) = {
	CLIDict_Item( matrixDebug ),
	CLIDict_Item( matrixInfo ),
	CLIDict_Item( matrixState ),
	{ 0, 0, 0 } // Null entry for dictionary end
};

// Debounce Array
KeyState Matrix_scanArray[ Matrix_colsNum * Matrix_rowsNum ];

// Ghost Arrays
#ifdef GHOSTING_MATRIX
KeyGhost Matrix_ghostArray[ Matrix_colsNum * Matrix_rowsNum ];

uint8_t col_use[Matrix_colsNum], row_use[Matrix_rowsNum];  // used count
uint8_t col_ghost[Matrix_colsNum], row_ghost[Matrix_rowsNum];  // marked as having ghost if 1
uint8_t col_ghost_old[Matrix_colsNum], row_ghost_old[Matrix_rowsNum];  // old ghost state
#endif


// Matrix debug flag - If set to 1, for each keypress the scan code is displayed in hex
//                     If set to 2, for each key state change, the scan code is displayed along with the state
uint8_t matrixDebugMode = 0;

// Matrix State Table Debug Counter - If non-zero display state table after every matrix scan
uint16_t matrixDebugStateCounter = 0;

// Matrix Scan Counters
uint16_t matrixMaxScans  = 0;
uint16_t matrixCurScans  = 0;
uint16_t matrixPrevScans = 0;

// System Timer used for delaying debounce decisions
extern volatile uint32_t systick_millis_count;

// Latency tracking
uint8_t matrixLatencyResource;



// ----- Functions -----

// Pin action (Strobe, Sense, Strobe Setup, Sense Setup)
// NOTE: This function is highly dependent upon the organization of the register map
//       Only guaranteed to work with Freescale MK20 series uCs
uint8_t Matrix_pin( GPIO_Pin gpio, Type type )
{
	// Register width is defined as size of a pointer
	unsigned int gpio_offset = gpio.port * 0x40   / sizeof(unsigned int*);
	unsigned int port_offset = gpio.port * 0x1000 / sizeof(unsigned int*) + gpio.pin;

	// Assumes 0x40 between GPIO Port registers and 0x1000 between PORT pin registers
	// See Lib/mk20dx.h
	volatile unsigned int *GPIO_PDDR = (unsigned int*)(&GPIOA_PDDR) + gpio_offset;
	#ifndef GHOSTING_MATRIX
	volatile unsigned int *GPIO_PSOR = (unsigned int*)(&GPIOA_PSOR) + gpio_offset;
	#endif
	volatile unsigned int *GPIO_PCOR = (unsigned int*)(&GPIOA_PCOR) + gpio_offset;
	volatile unsigned int *GPIO_PDIR = (unsigned int*)(&GPIOA_PDIR) + gpio_offset;
	volatile unsigned int *PORT_PCR  = (unsigned int*)(&PORTA_PCR0) + port_offset;

	// Operation depends on Type
	switch ( type )
	{
	case Type_StrobeOn:
		#ifdef GHOSTING_MATRIX
		*GPIO_PCOR |= (1 << gpio.pin);
		*GPIO_PDDR |= (1 << gpio.pin);  // output, low
		#else
		*GPIO_PSOR |= (1 << gpio.pin);
		#endif
		break;

	case Type_StrobeOff:
		#ifdef GHOSTING_MATRIX
		// Ghosting martix needs to put not used (off) strobes in high impedance state
		*GPIO_PDDR &= ~(1 << gpio.pin);  // input, high Z state
		#endif
		*GPIO_PCOR |= (1 << gpio.pin);
		break;

	case Type_StrobeSetup:
		#ifdef GHOSTING_MATRIX
		*GPIO_PDDR &= ~(1 << gpio.pin);  // input, high Z state
		*GPIO_PCOR |= (1 << gpio.pin);
		#else
		// Set as output pin
		*GPIO_PDDR |= (1 << gpio.pin);
		#endif

		// Configure pin with slow slew, high drive strength and GPIO mux
		*PORT_PCR = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);

		// Enabling open-drain if specified
		switch ( Matrix_type )
		{
		case Config_Opendrain:
			*PORT_PCR |= PORT_PCR_ODE;
			break;

		// Do nothing otherwise
		default:
			break;
		}
		break;

	case Type_Sense:
		#ifdef GHOSTING_MATRIX  // inverted
		return *GPIO_PDIR & (1 << gpio.pin) ? 0 : 1;
		#else
		return *GPIO_PDIR & (1 << gpio.pin) ? 1 : 0;
		#endif

	case Type_SenseSetup:
		// Set as input pin
		*GPIO_PDDR &= ~(1 << gpio.pin);

		// Configure pin with passive filter and GPIO mux
		*PORT_PCR = PORT_PCR_PFE | PORT_PCR_MUX(1);

		// Pull resistor config
		switch ( Matrix_type )
		{
		case Config_Pullup:
			*PORT_PCR |= PORT_PCR_PE | PORT_PCR_PS;
			break;

		case Config_Pulldown:
			*PORT_PCR |= PORT_PCR_PE;
			break;

		// Do nothing otherwise
		default:
			break;
		}
		break;
	}

	return 0;
}

// Setup GPIO pins for matrix scanning
void Matrix_setup()
{
	// Register Matrix CLI dictionary
	CLI_registerDictionary( matrixCLIDict, matrixCLIDictName );

	// Setup Strobe Pins
	for ( uint8_t pin = 0; pin < Matrix_colsNum; pin++ )
	{
		Matrix_pin( Matrix_cols[ pin ], Type_StrobeSetup );
		#ifdef GHOSTING_MATRIX
		col_use[pin] = 0;
		col_ghost[pin] = 0;
		col_ghost_old[pin] = 0;
		#endif
	}

	// Setup Sense Pins
	for ( uint8_t pin = 0; pin < Matrix_rowsNum; pin++ )
	{
		Matrix_pin( Matrix_rows[ pin ], Type_SenseSetup );
		#ifdef GHOSTING_MATRIX
		row_use[pin] = 0;
		row_ghost[pin] = 0;
		row_ghost_old[pin] = 0;
		#endif
	}

	// Clear out Debounce Array
	for ( uint8_t item = 0; item < Matrix_maxKeys; item++ )
	{
		Matrix_scanArray[ item ].prevState        = KeyState_Off;
		Matrix_scanArray[ item ].curState         = KeyState_Off;
		Matrix_scanArray[ item ].activeCount      = 0;
		Matrix_scanArray[ item ].inactiveCount    = DebounceDivThreshold_define; // Start at 'off' steady state
		Matrix_scanArray[ item ].prevDecisionTime = 0;
		#ifdef GHOSTING_MATRIX
		Matrix_ghostArray[ item ].prev            = KeyState_Off;
		Matrix_ghostArray[ item ].cur             = KeyState_Off;
		Matrix_ghostArray[ item ].saved           = KeyState_Off;
		#endif
	}

	// Clear scan stats counters
	matrixMaxScans  = 0;
	matrixPrevScans = 0;

	// Setup latency module
	matrixLatencyResource = Latency_add_resource("MatrixARM", LatencyOption_Ticks);
}

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


inline uint8_t Matrix_totalColumns()
{
	return Matrix_colsNum;
}


// Scan the matrix for keypresses
// NOTE: scanNum should be reset to 0 after a USB send (to reset all the counters)
void Matrix_scan( uint16_t scanNum, uint8_t *position, uint8_t count )
{
#if ( DebounceThrottleDiv_define > 0 )
	// Scan-rate throttling
	// By scanning using a divider, the scan rate slowed down
	// DebounceThrottleDiv_define == 1 means -> /2 or half scan rate
	// This helps with bouncy switches on fast uCs
	if ( !( Matrix_divCounter++ & (1 << ( DebounceThrottleDiv_define - 1 )) ) )
		return;
#endif

	// Increment stats counters
	if ( scanNum > matrixMaxScans ) matrixMaxScans = scanNum;
	if ( scanNum == 0 )
	{
		matrixPrevScans = matrixCurScans;
		matrixCurScans = 0;
	}
	else
	{
		matrixCurScans++;
	}

	// Read systick for event scheduling
	uint8_t currentTime = (uint8_t)systick_millis_count;

	// For each strobe, scan each of the sense pins
	uint8_t strobe_section = *position + count;

	// Start of each full section
	if ( *position == 0 )
	{
		Latency_start_time( matrixLatencyResource );
	}

	for ( uint8_t strobe = *position; strobe < Matrix_colsNum && strobe < strobe_section; strobe++ )
	{
		#ifdef STROBE_DELAY
		uint32_t start = micros();
		while ((micros() - start) < STROBE_DELAY);
		#endif

		// Strobe Pin
		Matrix_pin( Matrix_cols[ strobe ], Type_StrobeOn );

		#ifdef STROBE_DELAY
		start = micros();
		while ((micros() - start) < STROBE_DELAY);
		#endif

		// Scan each of the sense pins
		for ( uint8_t sense = 0; sense < Matrix_rowsNum; sense++ )
		{
			// Key position
			uint8_t key = Matrix_colsNum * sense + strobe;
			uint8_t key_disp = key + 1; // 1-indexed for reporting purposes
			KeyState *state = &Matrix_scanArray[ key ];

			// If first scan, reset state
			if ( scanNum == 0 )
			{
				// Set previous state, and reset current state
				state->prevState = state->curState;
				state->curState  = KeyState_Invalid;
			}

			// Signal Detected
			// Increment count and right shift opposing count
			// This means there is a maximum of scan 13 cycles on a perfect off to on transition
			//  (coming from a steady state 0xFFFF off scans)
			// Somewhat longer with switch bounciness
			// The advantage of this is that the count is ongoing and never needs to be reset
			// State still needs to be kept track of to deal with what to send to the Macro module
			if ( Matrix_pin( Matrix_rows[ sense ], Type_Sense ) )
			{
				// Only update if not going to wrap around
				if ( state->activeCount < DebounceDivThreshold_define ) state->activeCount += 1;
				state->inactiveCount >>= 1;
			}
			// Signal Not Detected
			else
			{
				// Only update if not going to wrap around
				if ( state->inactiveCount < DebounceDivThreshold_define ) state->inactiveCount += 1;
				state->activeCount >>= 1;
			}

			// Check for state change if it hasn't been set
			// But only if enough time has passed since last state change
			// Only check if the minimum number of scans has been met
			//   the current state is invalid
			//   and either active or inactive count is over the debounce threshold
			if ( state->curState == KeyState_Invalid )
			{
				// Determine time since last decision
				uint8_t lastTransition = currentTime - state->prevDecisionTime;

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
						if ( lastTransition < MinDebounceTime_define )
						{
							//warn_print("FAST Release stopped");
							state->curState = state->prevState;
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
						if ( lastTransition < MinDebounceTime_define )
						{
							//warn_print("FAST Press stopped");
							state->curState = state->prevState;
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
				#ifndef GHOSTING_MATRIX
				Macro_keyState( key_disp, state->curState );
				#endif

				// Matrix Debug, only if there is a state change
				if ( matrixDebugMode && state->curState != state->prevState )
				{
					// Basic debug output
					if ( matrixDebugMode == 1 && state->curState == KeyState_Press )
					{
						printInt8( key_disp );
						print(":");
						printHex( key_disp );
						print(" ");
					}
					// State transition debug output
					else if ( matrixDebugMode == 2 )
					{
						printHex( key_disp );
						Matrix_keyPositionDebug( state->curState );
						print(" ");
					}
				}
			}
		}

		// Unstrobe Pin
		Matrix_pin( Matrix_cols[ strobe ], Type_StrobeOff );

		// Update position
		*position = strobe;
	}
	(*position)++;


	// Matrix ghosting check and elimination
	// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#ifdef GHOSTING_MATRIX
	// strobe = column, sense = row

	// Count (rows) use for columns
	for ( uint8_t col = 0; col < Matrix_colsNum; col++ )
	{
		uint8_t used = 0;
		for ( uint8_t row = 0; row < Matrix_rowsNum; row++ )
		{
			uint8_t key = Matrix_colsNum * row + col;
			KeyState *state = &Matrix_scanArray[ key ];
			if ( keyOn(state->curState) )
				used++;
		}
		col_use[col] = used;
		col_ghost_old[col] = col_ghost[col];
		col_ghost[col] = 0;  // clear
	}

	// Count (columns) use for rows
	for ( uint8_t row = 0; row < Matrix_rowsNum; row++ )
	{
		uint8_t used = 0;
		for ( uint8_t col = 0; col < Matrix_colsNum; col++ )
		{
			uint8_t key = Matrix_colsNum * row + col;
			KeyState *state = &Matrix_scanArray[ key ];
			if ( keyOn(state->curState) )
				used++;
		}
		row_use[row] = used;
		row_ghost_old[row] = row_ghost[row];
		row_ghost[row] = 0;  // clear
	}

	// Check if matrix has ghost
	// Happens when key is pressed and some other key is pressed in same row and another in same column
	for ( uint8_t col = 0; col < Matrix_colsNum; col++ )
	{
		for ( uint8_t row = 0; row < Matrix_rowsNum; row++ )
		{
			uint8_t key = Matrix_colsNum * row + col;
			KeyState *state = &Matrix_scanArray[ key ];
			if ( keyOn(state->curState) && col_use[col] >= 2 && row_use[row] >= 2 )
			{
				// mark col and row as having ghost
				col_ghost[col] = 1;
				row_ghost[row] = 1;
			}
		}
	}

	// Send keys
	for ( uint8_t col = 0; col < Matrix_colsNum; col++ )
	{
		for ( uint8_t row = 0; row < Matrix_rowsNum; row++ )
		{
			uint8_t key = Matrix_colsNum * row + col;
			uint8_t key_disp = key + 1;
			KeyState *state = &Matrix_scanArray[ key ];
			KeyGhost *st = &Matrix_ghostArray[ key ];

			// col or row is ghosting (crossed)
			uint8_t ghost = (col_ghost[col] > 0 || row_ghost[row] > 0) ? 1 : 0;
			uint8_t ghost_old = (col_ghost_old[col] > 0 || row_ghost_old[row] > 0) ? 1 : 0;
			ghost = ghost || ghost_old ? 1 : 0;

			st->prev = st->cur;  // previous
			// save state if no ghost or outside ghosted area
			if ( ghost == 0 )
				st->saved = state->curState;  // save state if no ghost
			// final
			// use saved state if ghosting, or current if not
			st->cur = ghost > 0 ? st->saved : state->curState;

			//  Send keystate to macro module
			KeyPosition k = !st->cur
				? (!st->prev ? KeyState_Off : KeyState_Release)
				: ( st->prev ? KeyState_Hold : KeyState_Press);
			Macro_keyState( key_disp, k );
		}
	}
#endif

	// Measure ending latency if final strobe
	if ( *position + 1 == Matrix_colsNum )
	{
		Latency_end_time( matrixLatencyResource );
	}

	// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .


	// State Table Output Debug
	if ( matrixDebugStateCounter > 0 )
	{
		// Decrement counter
		matrixDebugStateCounter--;

		// Output stats on number of scans being done per USB send
		print( NL );
		info_msg("Max scans:      ");
		printHex( matrixMaxScans );
		print( NL );
		info_msg("Previous scans: ");
		printHex( matrixPrevScans );
		print( NL );

		// Output current scan number
		info_msg("Scan Number:    ");
		printHex( scanNum );
		print( NL );

		// Display the state info for each key
		print("<key>:<previous state><current state> <active count> <inactive count>");
		for ( uint8_t key = 0; key < Matrix_maxKeys; key++ )
		{
			// Every 4 keys, put a newline
			if ( key % 4 == 0 )
				print( NL );

			print("\033[1m0x");
			printHex_op( key + 1, 2 );
			print("\033[0m");
			print(":");
			Matrix_keyPositionDebug( Matrix_scanArray[ key ].prevState );
			Matrix_keyPositionDebug( Matrix_scanArray[ key ].curState );
			print(" 0x");
			printHex_op( Matrix_scanArray[ key ].activeCount, 4 );
			print(" 0x");
			printHex_op( Matrix_scanArray[ key ].inactiveCount, 4 );
			print(" ");
		}

		print( NL );
	}
}


// Called by parent scan module whenever the available current changes
// current - mA
void Matrix_currentChange( unsigned int current )
{
	// TODO - Any potential power savings?
}



// ----- CLI Command Functions -----

void cliFunc_matrixInfo( char* args )
{
	print( NL );
	info_msg("Columns:  ");
	printHex( Matrix_colsNum );

	print( NL );
	info_msg("Rows:     ");
	printHex( Matrix_rowsNum );

	print( NL );
	info_msg("Max Keys: ");
	printHex( Matrix_maxKeys );
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
		matrixDebugMode = matrixDebugMode != 2 ? 2 : 0;
		break;

	// No argument
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

