/* Copyright (C) 2011-2013 by Joseph Makuch (jmakuch+f@gmail.com)
 * Additions by Jacob Alexander (2013-2014) (haata@kiibohd.com)
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
#include <led.h>
#include <macro.h>
#include <print.h>

// Local Includes
#include "scan_loop.h"



// ----- Defines -----

// TODO dfj defines...needs commenting and maybe some cleaning...
#define MAX_PRESS_DELTA_MV 450 // As measured from the Teensy ADC pin
#define THRESHOLD_MV (MAX_PRESS_DELTA_MV >> 1)
//(2560 / (0x3ff/2)) ~= 5
#define MV_PER_ADC 5
#define THRESHOLD (THRESHOLD_MV / MV_PER_ADC)

#define STROBE_SETTLE 1

#define ADHSM 7

// Right justification of ADLAR
#define ADLAR_BITS 0

// full muxmask
#define FULL_MUX_MASK ((1 << MUX0) | (1 << MUX1) | (1 << MUX2) | (1 << MUX3) | (1 << MUX4))

// F0-f7 pins only muxmask.
#define MUX_MASK ((1 << MUX0) | (1 << MUX1) | (1 << MUX2))

// Strobe Masks
#define D_MASK (0xff)
#define E_MASK (0x03)
#define C_MASK (0xff)

// set ADC clock prescale
#define PRESCALE_MASK ((1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2))
#define PRESCALE_SHIFT (ADPS0)
#define PRESCALE 3

// Max number of strobes supported by the hardware
// Strobe lines are detected at startup, extra strobes cause anomalies like phantom keypresses
#define MAX_STROBES 18

// Number of consecutive samples required to pass debounce
#define DEBOUNCE_THRESHOLD 5

// Scans to remain idle after all keys were release before starting averaging
// XXX So this makes the initial keypresses fast,
//      but it's still possible to lose a keypress if you press at the wrong time -HaaTa
#define KEY_IDLE_SCANS 30000

// Total number of muxes/sense lines available
#define MUXES_COUNT 8
#define MUXES_COUNT_XSHIFT 3

// Number of warm-up loops before starting to scan keys
#define WARMUP_LOOPS ( 1024 )
#define WARMUP_STOP (WARMUP_LOOPS - 1)

#define SAMPLE_CONTROL 3

#define KEY_COUNT ((MAX_STROBES) * (MUXES_COUNT))

#define RECOVERY_CONTROL 1
#define RECOVERY_SOURCE  0
#define RECOVERY_SINK    2

#define ON  1
#define OFF 0

// mix in 1/4 of the current average to the running average. -> (@mux_mix = 2)
#define MUX_MIX 2

#define IDLE_COUNT_SHIFT 8

// av = (av << shift) - av + sample; av >>= shift
// e.g. 1 -> (av + sample) / 2 simple average of new and old
//      2 -> (3 * av + sample) / 4 i.e. 3:1 mix of old to new.
//      3 -> (7 * av + sample) / 8 i.e. 7:1 mix of old to new.
#define KEYS_AVERAGES_MIX_SHIFT 3



// ----- Macros -----

// Select mux
#define SET_FULL_MUX(X) ((ADMUX) = (((ADMUX) & ~(FULL_MUX_MASK)) | ((X) & (FULL_MUX_MASK))))



// ----- Function Declarations -----

// CLI Functions
void cliFunc_avgDebug   ( char* args );
void cliFunc_echo       ( char* args );
void cliFunc_keyDebug   ( char* args );
void cliFunc_pressDebug ( char* args );
void cliFunc_problemKeys( char* args );
void cliFunc_senseDebug ( char* args );

// Debug Functions
void dumpSenseTable();

// High-level Capsense Functions
void setup_ADC();
void capsense_scan();

// Capsense Sense Functions
void testColumn  ( uint8_t strobe );
void sampleColumn( uint8_t column );

// Low-level Capsense Functions
void strobe_w( uint8_t strobe_num );
void recovery( uint8_t on );



// ----- Variables -----

// Scan Module command dictionary
CLIDict_Entry( echo,        "Example command, echos the arguments." );
CLIDict_Entry( avgDebug,    "Enables/Disables averaging results." NL "\t\tDisplays each average, starting from Key 0x00, ignoring 0 valued averages." );
CLIDict_Entry( keyDebug,    "Enables/Disables long debug for each keypress." NL "\t\tkeycode - [strobe:mux] : sense val : threshold+delta=total : margin" );
CLIDict_Entry( pressDebug,  "Enables/Disables short debug for each keypress." );
CLIDict_Entry( problemKeys, "Display current list of problem keys," );
CLIDict_Entry( senseDebug,  "Prints out the current sense table N times." NL "\t\tsense:max sense:delta" );

CLIDict_Def( scanCLIDict, "DPH Module Commands" ) = {
	CLIDict_Item( echo ),
	CLIDict_Item( avgDebug ),
	CLIDict_Item( keyDebug ),
	CLIDict_Item( pressDebug ),
	CLIDict_Item( problemKeys ),
	CLIDict_Item( senseDebug ),
	{ 0, 0, 0 } // Null entry for dictionary end
};


// CLI Control Variables
uint8_t enableAvgDebug   = 0;
uint8_t enableKeyDebug   = 0;
uint8_t enablePressDebug = 0;
uint8_t senseDebugCount  = 3; // In order to get boot-time oddities


// Variables used to calculate the starting sense value (averaging)
uint32_t full_avg = 0;
uint32_t high_avg = 0;
uint32_t  low_avg = 0;

uint8_t  high_count = 0;
uint8_t   low_count = 0;


uint16_t samples[MAX_STROBES][MUXES_COUNT];   // Overall table of cap sense ADC values
uint16_t sampleMax[MAX_STROBES][MUXES_COUNT]; // Records the max seen ADC value

uint8_t  key_activity = 0; // Increments for each detected key per each full scan of the keyboard, it is reset before each full scan
uint16_t key_idle     = 0; // Defines how scans after all keys were released before starting averaging again
uint8_t  key_release  = 0; // Indicates if going from key press state to release state (some keys pressed to no keys pressed)

uint16_t threshold = THRESHOLD;

uint16_t keys_averages_acc[KEY_COUNT];
uint16_t keys_averages    [KEY_COUNT];
uint8_t  keys_debounce    [KEY_COUNT]; // Contains debounce statistics
uint8_t  keys_problem     [KEY_COUNT]; // Marks keys that should be ignored (determined by averaging at startup)

// TODO: change this to 'booting', then count down.
uint16_t boot_count = 0;

uint8_t total_strobes = MAX_STROBES;
uint8_t strobe_map[MAX_STROBES];



// ----- Functions -----

// Initial setup for cap sense controller
inline void Scan_setup()
{
	// Register Scan CLI dictionary
	CLI_registerDictionary( scanCLIDict, scanCLIDictName );

	// Scan for active strobes
	// NOTE1: On IBM PCBs, each strobe line that is *NOT* used is connected to GND.
	//       This means, the strobe GPIO can be set to Tri-State pull-up to detect which strobe lines are not used.
	// NOTE2: This will *NOT* detect floating strobes.
	// NOTE3: Rev 0.4, the strobe numbers are reversed, so D0 is actually strobe 0 and C7 is strobe 17
	info_msg("Detecting Strobes...");

	DDRC  = 0;
	PORTC = C_MASK;
	DDRD  = 0;
	PORTD = D_MASK;
	DDRE  = 0;
	PORTE = E_MASK;

	// Initially there are 0 strobes
	total_strobes = 0;

	// Iterate over each the strobes
	for ( uint8_t strobe = 0; strobe < MAX_STROBES; strobe++ )
	{
		uint8_t detected = 0;

		// If PIN is high, then strobe is *NOT* connected to GND and may be a strobe
		switch ( strobe )
		{

		// Strobe Mappings
		//              Rev  Rev
		//              0.2  0.4
#ifndef REV0_4_DEBUG // XXX These pins should be reworked, and connect to GND on Rev 0.4
		case 0:  // D0   0   n/c
		case 1:  // D1   1   n/c
#endif
		case 2:  // D2   2   15
		case 3:  // D3   3   14
		case 4:  // D4   4   13
		case 5:  // D5   5   12
		case 6:  // D6   6   11
		case 7:  // D7   7   10
			detected = PIND & (1 << strobe);
			break;

		case 8:  // E0   8    9
		case 9:  // E1   9    8
			detected = PINE & (1 << (strobe - 8));
			break;

		case 10: // C0  10    7
		case 11: // C1  11    6
		case 12: // C2  12    5
		case 13: // C3  13    4
		case 14: // C4  14    3
		case 15: // C5  15    2
#ifndef REV0_2_DEBUG // XXX If not using the 18 pin connector on Rev 0.2, rework these pins to GND
		case 16: // C6  16    1
		case 17: // C7  17    0
#endif
			detected = PINC & (1 << (strobe - 10));
			break;

		default:
			break;
		}

		// Potential strobe line detected
		if ( detected )
		{
			strobe_map[total_strobes] = strobe;
			total_strobes++;
		}
	}

	printInt8( total_strobes );
	print( " strobes found." NL );

	// Setup Pins for Strobing
	DDRC  = C_MASK;
	PORTC = 0;
	DDRD  = D_MASK;
	PORTD = 0;
	DDRE  = E_MASK;
	PORTE = 0 ;

	// Initialize ADC
	setup_ADC();

	// Reset debounce table
	for ( int i = 0; i < KEY_COUNT; ++i )
	{
		keys_debounce[i] = 0;
	}

	// Warm things up a bit before we start collecting data, taking real samples.
	for ( uint8_t i = 0; i < total_strobes; ++i )
	{
		sampleColumn( strobe_map[i] );
	}
}


// Main Detection Loop
// This is where the important stuff happens
inline uint8_t Scan_loop()
{
	capsense_scan();

	// Return non-zero if macro and USB processing should be delayed
	// Macro processing will always run if returning 0
	// USB processing only happens once the USB send timer expires, if it has not, Scan_loop will be called
	//  after the macro processing has been completed
	return 0;
}


// Signal from macro module that keys have been processed
// NOTE: Only really required for implementing "tricks" in converters for odd protocols
void Scan_finishedWithMacro( uint8_t sentKeys )
{
	return;
}


// Signal from output module that keys have been processed/sent
// NOTE: Only really required for implementing "tricks" in converters for odd protocols
void Scan_finishedWithOutput( uint8_t sentKeys )
{
	return;
}


inline void capsense_scan()
{
	// Accumulated average used for the next scan
	uint32_t cur_full_avg = 0;
	uint32_t cur_high_avg = 0;

	// Reset average counters
	low_avg = 0;
	low_count = 0;

	high_count = 0;

	// Reset key activity, if there is no key activity, averages will accumulate for sense deltas, otherwise they will be reset
	key_activity = 0;

	// Scan each of the mapped strobes in the matrix
	for ( uint8_t strober = 0; strober < total_strobes; ++strober )
	{
		uint8_t map_strobe = strobe_map[strober];

		// Sample the ADCs for the given column/strobe
		sampleColumn( map_strobe );

		// Only process sense data if warmup is finished
		if ( boot_count >= WARMUP_LOOPS )
		{
			testColumn( map_strobe );
		}

		uint8_t strobe_line = map_strobe << MUXES_COUNT_XSHIFT;
		for ( int mux = 0; mux < MUXES_COUNT; ++mux )
		{
			// discard sketchy low bit, and meaningless high bits.
			uint8_t sample = samples[map_strobe][mux] >> 1;
			keys_averages_acc[strobe_line + mux] += sample;
		}

		// Accumulate 3 total averages (used for determining starting average during warmup)
		//     full_avg - Average of all sampled lines on the previous scan set
		// cur_full_avg - Average of all sampled lines for this scan set
		//     high_avg - Average of all sampled lines above full_avg on the previous scan set
		// cur_high_avg - Average of all sampled lines above full_avg
		//      low_avg - Average of all sampled lines below or equal to full_avg
		if ( boot_count < WARMUP_LOOPS )
		{
			for ( uint8_t mux = 0; mux < MUXES_COUNT; ++mux )
			{
				uint8_t sample = samples[map_strobe][mux] >> 1;

				// Sample is high, add it to high avg
				if ( sample > full_avg )
				{
					high_count++;
					cur_high_avg += sample;
				}
				// Sample is low, add it to low avg
				else
				{
					low_count++;
					low_avg += sample;
				}

				// If sample is higher than previous high_avg, then mark as "problem key"
				// XXX Giving a bit more margin to pass (high_avg vs. high_avg + high_avg - full_avg) -HaaTa
				keys_problem[strobe_line + mux] = sample > high_avg + (high_avg - full_avg) ? sample : 0;

				// Prepare for next average
				cur_full_avg += sample;
			}
		}
	} // for strober

	// Update total sense average (only during warm-up)
	if ( boot_count < WARMUP_LOOPS )
	{
		full_avg = cur_full_avg / (total_strobes * MUXES_COUNT);
		high_avg = cur_high_avg / high_count;
		low_avg /= low_count;

		// Update the base average value using the low_avg (best chance of not ignoring a keypress)
		for ( int i = 0; i < KEY_COUNT; ++i )
		{
			keys_averages[i] = low_avg;
			keys_averages_acc[i] = low_avg;
		}
	}

	// Warm up voltage references
	if ( boot_count < WARMUP_LOOPS )
	{
		boot_count++;

		switch ( boot_count )
		{
		// First loop
		case 1:
			// Show msg at first iteration only
			info_msg("Warming up the voltage references");
			break;
		// Middle iterations
		case 300:
		case 600:
		case 900:
		case 1200:
			print(".");
			break;
		// Last loop
		case WARMUP_STOP:
			print( NL );
			info_msg("Warmup finished using ");
			printInt16( WARMUP_LOOPS );
			print(" iterations" NL );

			// Display the final calculated averages of all the sensed strobes
			info_msg("Full average (");
			printInt8( total_strobes * MUXES_COUNT );
			print("): ");
			printHex( full_avg );

			print("  High average (");
			printInt8( high_count );
			print("): ");
			printHex( high_avg );

			print("  Low average (");
			printInt8( low_count );
			print("): ");
			printHex( low_avg );

			print("  Rejection threshold: ");
			printHex( high_avg + (high_avg - full_avg) );
			print( NL );

			// Display problem keys, and the sense value at the time
			for ( uint8_t key = 0; key < KEY_COUNT; key++ )
			{
				if ( keys_problem[key] )
				{
					warn_msg("Problem key detected: ");
					printHex( key );
					print(" (");
					printHex( keys_problem[key] );
					print(")" NL );
				}
			}

			info_print("If problem keys were detected, and were being held down, they will be reset as soon as let go.");
			info_print("Some keys have unusually high sense values, on the first press they should be re-enabled.");
			break;
		}
	}
	else
	{
		// No keypress, accumulate averages
		if( !key_activity )
		{
			// Only start averaging once the idle counter has counted down to 0
			if ( key_idle == 0 )
			{
				// Average Debugging
				if ( enableAvgDebug )
				{
					print("\033[1mAvg\033[0m: ");
				}

				// aggregate
				for ( uint8_t i = 0; i < KEY_COUNT; ++i )
				{
					uint16_t acc = keys_averages_acc[i];
					//uint16_t acc = keys_averages_acc[i] >> IDLE_COUNT_SHIFT; // XXX This fixes things... -HaaTa
					uint32_t av = keys_averages[i];

					av = (av << KEYS_AVERAGES_MIX_SHIFT) - av + acc;
					av >>= KEYS_AVERAGES_MIX_SHIFT;

					keys_averages[i] = av;
					keys_averages_acc[i] = 0;

					// Average Debugging
					if ( enableAvgDebug && av > 0 )
					{
						printHex( av );
						print(" ");
					}
				}

				// Average Debugging
				if ( enableAvgDebug )
				{
					print( NL );
				}

				// No key presses detected, set key_release indicator
				key_release = 1;
			}
			// Otherwise decrement the idle counter
			else
			{
				key_idle--;
			}
		}
		// Keypresses, reset accumulators
		else if ( key_release )
		{
			for ( uint8_t c = 0; c < KEY_COUNT; ++c ) { keys_averages_acc[c] = 0; }

			key_release = 0;
		}

		// If the debugging sense table is non-zero, display
		if ( senseDebugCount > 0 )
		{
			senseDebugCount--;
			print( NL );
			dumpSenseTable();
		}
	}
}


void setup_ADC()
{
	// disable adc digital pins.
	DIDR1 |= (1 << AIN0D) | (1<<AIN1D); // set disable on pins 1,0.
	DDRF = 0x0;
	PORTF = 0x0;
	uint8_t mux = 0 & 0x1f; // 0 == first. // 0x1e = 1.1V ref.

	// 0 = external aref 1,1 = 2.56V internal ref
	uint8_t aref = ((1 << REFS1) | (1 << REFS0)) & ((1 << REFS1) | (1 << REFS0));
	uint8_t adate = (1 << ADATE) & (1 << ADATE); // trigger enable
	uint8_t trig = 0 & ((1 << ADTS0) | (1 << ADTS1) | (1 << ADTS2)); // 0 = free running
	// ps2, ps1 := /64 ( 2^6 ) ps2 := /16 (2^4), ps1 := 4, ps0 :=2, PS1,PS0 := 8 (2^8)
	uint8_t prescale = ( ((PRESCALE) << PRESCALE_SHIFT) & PRESCALE_MASK ); // 001 == 2^1 == 2
	uint8_t hispeed = (1 << ADHSM);
	uint8_t en_mux = (1 << ACME);

	ADCSRA = (1 << ADEN) | prescale; // ADC enable

	// select ref.
	//ADMUX |= ((1 << REFS1) | (1 << REFS0)); // 2.56 V internal.
	//ADMUX |= ((1 << REFS0) ); // Vcc with external cap.
	//ADMUX &= ~((1 << REFS1) | (1 << REFS0)); // 0,0 : aref.
	ADMUX = aref | mux | ADLAR_BITS;

	// set free-running
	ADCSRA |= adate; // trigger enable
	ADCSRB  = en_mux | hispeed | trig | (ADCSRB & ~((1 << ADTS0) | (1 << ADTS1) | (1 << ADTS2))); // trigger select free running

	ADCSRA |= (1 << ADEN); // ADC enable
	ADCSRA |= (1 << ADSC); // start conversions q
}


void recovery( uint8_t on )
{
	DDRB  |=  (1 << RECOVERY_CONTROL);
	PORTB &= ~(1 << RECOVERY_SINK);   // SINK always zero
	DDRB  &= ~(1 << RECOVERY_SOURCE); // SOURCE high imp

	if ( on )
	{
		// set strobes to sink to gnd.
		DDRC |= C_MASK;
		DDRD |= D_MASK;
		DDRE |= E_MASK;

		PORTC &= ~C_MASK;
		PORTD &= ~D_MASK;
		PORTE &= ~E_MASK;

		DDRB  |= (1 << RECOVERY_SINK);   // SINK pull
		PORTB |= (1 << RECOVERY_CONTROL);
		PORTB |= (1 << RECOVERY_SOURCE); // SOURCE high
		DDRB  |= (1 << RECOVERY_SOURCE);
	}
	else
	{
		PORTB &= ~(1 << RECOVERY_CONTROL);
		DDRB  &= ~(1 << RECOVERY_SOURCE);
		PORTB &= ~(1 << RECOVERY_SOURCE); // SOURCE low
		DDRB  &= ~(1 << RECOVERY_SINK);   // SINK high-imp
	}
}


void hold_sample( uint8_t on )
{
	if ( !on )
	{
		PORTB |= (1 << SAMPLE_CONTROL);
		DDRB  |= (1 << SAMPLE_CONTROL);
	}
	else
	{
		DDRB  |=  (1 << SAMPLE_CONTROL);
		PORTB &= ~(1 << SAMPLE_CONTROL);
	}
}


void strobe_w( uint8_t strobe_num )
{
	PORTC &= ~(C_MASK);
	PORTD &= ~(D_MASK);
	PORTE &= ~(E_MASK);

	// Strobe table
	// Not all strobes are used depending on which are detected
	switch ( strobe_num )
	{

	case 0:  PORTD |= (1 << 0); break;
	case 1:  PORTD |= (1 << 1); break;
	case 2:  PORTD |= (1 << 2); break;
	case 3:  PORTD |= (1 << 3); break;
	case 4:  PORTD |= (1 << 4); break;
	case 5:  PORTD |= (1 << 5); break;
	case 6:  PORTD |= (1 << 6); break;
	case 7:  PORTD |= (1 << 7); break;

	case 8:  PORTE |= (1 << 0); break;
	case 9:  PORTE |= (1 << 1); break;

	case 10: PORTC |= (1 << 0); break;
	case 11: PORTC |= (1 << 1); break;
	case 12: PORTC |= (1 << 2); break;
	case 13: PORTC |= (1 << 3); break;
	case 14: PORTC |= (1 << 4); break;
	case 15: PORTC |= (1 << 5); break;
	case 16: PORTC |= (1 << 6); break;
	case 17: PORTC |= (1 << 7); break;

	default:
		break;
	}
}


inline uint16_t getADC(void)
{
	ADCSRA |= (1 << ADIF); // clear int flag by writing 1.

	//wait for last read to complete.
	while ( !( ADCSRA & (1 << ADIF) ) );

	return ADC; // return sample
}


void sampleColumn( uint8_t column )
{
	// ensure all probe lines are driven low, and chill for recovery delay.
	ADCSRA |= (1 << ADEN) | (1 << ADSC); // enable and start conversions

	PORTC &= ~C_MASK;
	PORTD &= ~D_MASK;
	PORTE &= ~E_MASK;

	PORTF = 0;
	DDRF  = 0;

	recovery( OFF );
	strobe_w( column );

	hold_sample( OFF );
	SET_FULL_MUX( 0 );

	// Allow strobes to settle
	for ( uint8_t i = 0; i < STROBE_SETTLE; ++i ) { getADC(); }

	hold_sample( ON );

	uint8_t mux = 0;
	SET_FULL_MUX( mux );
	getADC(); // throw away; unknown mux.
	do {
		SET_FULL_MUX( mux + 1 ); // our *next* sample will use this

		// retrieve current read.
		uint16_t readVal = getADC();
		samples[column][mux] = readVal;

		// Update max sense sample table
		if ( readVal > sampleMax[column][mux] )
		{
			sampleMax[column][mux] = readVal;
		}

		mux++;

	} while ( mux < 8 );

	hold_sample( OFF );
	recovery( ON );

	// turn off adc.
	ADCSRA &= ~(1 << ADEN);

	// pull all columns' strobe-lines low.
	DDRC |= C_MASK;
	DDRD |= D_MASK;
	DDRE |= E_MASK;

	PORTC &= ~C_MASK;
	PORTD &= ~D_MASK;
	PORTE &= ~E_MASK;
}


void testColumn( uint8_t strobe )
{
	uint16_t db_delta = 0;
	uint8_t  db_sample = 0;
	uint16_t db_threshold = 0;

	uint8_t column = 0;
	uint8_t bit = 1;

	for ( uint8_t mux = 0; mux < MUXES_COUNT; ++mux )
	{
		uint16_t delta = keys_averages[(strobe << MUXES_COUNT_XSHIFT) + mux];

		uint8_t key = (strobe << MUXES_COUNT_XSHIFT) + mux;

		// Check if this is a bad key (e.g. test point, or non-existent key)
		if ( keys_problem[key] )
		{
			// If the sample value of the problem key goes above initally recorded result + threshold
			//  re-enable the key
			if ( (db_sample = samples[strobe][mux] >> 1) > keys_problem[key] + threshold )
			//if ( (db_sample = samples[strobe][mux] >> 1) < high_avg )
			{
				info_msg("Re-enabling problem key: ");
				printHex( key );
				print( NL );

				keys_problem[key] = 0;
			}

			// Do not waste any more cycles processing, regardless, a keypress cannot be detected
			continue;
		}

		// Keypress detected
		//  db_sample (uint8_t), discard meaningless high bit, and garbage low bit
		if ( (db_sample = samples[strobe][mux] >> 1) > (db_threshold = threshold) + (db_delta = delta) )
		{
			column |= bit;
			key_activity++; // No longer idle, stop averaging ADC data
			key_idle = KEY_IDLE_SCANS; // Reset idle count-down

			// Only register keypresses once the warmup is complete, or not enough debounce info
			if ( keys_debounce[key] <= DEBOUNCE_THRESHOLD )
			{
				// Add to the Macro processing buffer if debounce criteria met
				// Automatically handles converting to a USB code and sending off to the PC
				if ( keys_debounce[key] == DEBOUNCE_THRESHOLD )
				{
					// Debug message, pressDebug CLI
					if ( enablePressDebug )
					{
						print("0x");
						printHex_op( key, 2 );
						print(" ");
					}

					// Initial Keypress
					Macro_keyState( key, 0x01 );
				}

				keys_debounce[key]++;

			}
			else if ( keys_debounce[key] >= DEBOUNCE_THRESHOLD )
			{
				// Held Key
				Macro_keyState( key, 0x02 );
			}

			// Long form key debugging
			if ( enableKeyDebug )
			{
				// Debug message
				// <key> [<strobe>:<mux>] : <sense val> : <delta + threshold> : <margin>
				dbug_msg("");
				printHex_op( key, 1 );
				print(" [");
				printInt8( strobe );
				print(":");
				printInt8( mux );
				print("] : ");
				printHex( db_sample ); // Sense
				print(" : ");
				printHex( db_threshold );
				print("+");
				printHex( db_delta );
				print("=");
				printHex( db_threshold + db_delta ); // Sense compare
				print(" : ");
				printHex( db_sample - ( db_threshold + db_delta ) ); // Margin
				print( NL );
			}
		}
		// Clear debounce entry if no keypress detected
		else
		{
			// Release Key
			if ( keys_debounce[key] >= DEBOUNCE_THRESHOLD )
			{
				Macro_keyState( key, 0x03 );
			}

			// Clear debounce entry
			keys_debounce[key] = 0;
		}

		bit <<= 1;
	}
}


void dumpSenseTable()
{
	// Initial table alignment, with base threshold used for every key
	print("\033[1m");
	printHex( threshold );
	print("\033[0m       ");

	// Print out headers first
	for ( uint8_t mux = 0; mux < MUXES_COUNT; ++mux )
	{
		print("  Mux \033[1m");
		printInt8( mux );
		print("\033[0m  ");
	}

	print( NL );

	// Display the full strobe/sense table
	for ( uint8_t strober = 0; strober < total_strobes; ++strober )
	{
		uint8_t strobe = strobe_map[strober];

		// Display the strobe
		print("Strobe \033[1m");
		printHex( strobe );
		print("\033[0m ");

		// For each mux, display sense:threshold:delta
		for ( uint8_t mux = 0; mux < MUXES_COUNT; ++mux )
		{
			uint8_t delta = keys_averages[(strobe << MUXES_COUNT_XSHIFT) + mux];
			uint8_t sample = samples[strobe][mux] >> 1;
			uint8_t max = sampleMax[strobe][mux] >> 1;

			// Indicate if the key is being pressed by displaying green
			if ( sample > delta + threshold )
			{
				print("\033[1;32m");
			}

			printHex_op( sample, 2 );
			print(":");
			printHex_op( max, 2 );
			print(":");
			printHex_op( delta, 2 );
			print("\033[0m ");
		}

		// New line for each strobe
		print( NL );
	}
}


// ----- CLI Command Functions -----

// XXX Just an example command showing how to parse arguments (more complex than generally needed)
void cliFunc_echo( char* args )
{
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Parse args until a \0 is found
	while ( 1 )
	{
		print( NL ); // No \r\n by default after the command is entered

		curArgs = arg2Ptr; // Use the previous 2nd arg pointer to separate the next arg from the list
		CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

		// Stop processing args if no more are found
		if ( *arg1Ptr == '\0' )
			break;

		// Print out the arg
		dPrint( arg1Ptr );
	}
}

void cliFunc_avgDebug( char* args )
{
	print( NL );

	// Args ignored, just toggling
	if ( enableAvgDebug )
	{
		info_print("Cap Sense averaging debug disabled.");
		enableAvgDebug = 0;
	}
	else
	{
		info_print("Cap Sense averaging debug enabled.");
		enableAvgDebug = 1;
	}
}

void cliFunc_keyDebug( char* args )
{
	print( NL );

	// Args ignored, just toggling
	if ( enableKeyDebug )
	{
		info_print("Cap Sense key long debug disabled - pre debounce.");
		enableKeyDebug = 0;
	}
	else
	{
		info_print("Cap Sense key long debug enabled - pre debounce.");
		enableKeyDebug = 1;
	}
}

void cliFunc_pressDebug( char* args )
{
	print( NL );

	// Args ignored, just toggling
	if ( enablePressDebug )
	{
		info_print("Cap Sense key debug disabled - post debounce.");
		enablePressDebug = 0;
	}
	else
	{
		info_print("Cap Sense key debug enabled - post debounce.");
		enablePressDebug = 1;
	}
}

void cliFunc_problemKeys( char* args )
{
	print( NL );

	uint8_t count = 0;

	// Args ignored, just displaying
	// Display problem keys, and the sense value at the time
	for ( uint8_t key = 0; key < KEY_COUNT; key++ )
	{
		if ( keys_problem[key] )
		{
			if ( count++ == 0 )
			{
				warn_msg("Problem keys: ");
			}
			printHex( key );
			print(" (");
			printHex( keys_problem[key] );
			print(")   "  );
		}
	}
}

void cliFunc_senseDebug( char* args )
{
	// Parse code from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	// Default to a single print
	senseDebugCount = 1;

	// If there was an argument, use that instead
	if ( *arg1Ptr != '\0' )
	{
		senseDebugCount = numToInt( arg1Ptr );
	}
}

