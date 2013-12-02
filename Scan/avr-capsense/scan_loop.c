/* Copyright (C) 2011-2013 by Joseph Makuch
 * Additions by Jacob Alexander (2013)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

// ----- Includes -----

// Compiler Includes
#include <Lib/ScanLib.h>

// Project Includes
#include <led.h>
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

#define TEST_KEY_STROBE (0x05)
#define TEST_KEY_MASK (1 << 0)

#define ADHSM 7

#define RIGHT_JUSTIFY 0
#define LEFT_JUSTIFY (0xff)

// set left or right justification here:
#define JUSTIFY_ADC RIGHT_JUSTIFY
#define ADLAR_MASK (1 << ADLAR)

#ifdef JUSTIFY_ADC
#define ADLAR_BITS ((ADLAR_MASK) & (JUSTIFY_ADC))
#else // defaults to right justification.
#define ADLAR_BITS 0
#endif

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

#define MUXES_COUNT 8
#define MUXES_COUNT_XSHIFT 3

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

#define IDLE_COUNT_MASK 0xff
#define IDLE_COUNT_SHIFT 8

// av = (av << shift) - av + sample; av >>= shift
// e.g. 1 -> (av + sample) / 2 simple average of new and old
//      2 -> (3 * av + sample) / 4 i.e. 3:1 mix of old to new.
//      3 -> (7 * av + sample) / 8 i.e. 7:1 mix of old to new.
#define KEYS_AVERAGES_MIX_SHIFT 3



// ----- Macros -----

// Make sure we haven't overflowed the buffer
#define bufferAdd(byte) \
		if ( KeyIndex_BufferUsed < KEYBOARD_BUFFER ) \
			KeyIndex_Buffer[KeyIndex_BufferUsed++] = byte

// Select mux
#define SET_FULL_MUX(X) ((ADMUX) = (((ADMUX) & ~(FULL_MUX_MASK)) | ((X) & (FULL_MUX_MASK))))



// ----- Variables -----

// Buffer used to inform the macro processing module which keys have been detected as pressed
volatile uint8_t KeyIndex_Buffer[KEYBOARD_BUFFER];
volatile uint8_t KeyIndex_BufferUsed;


// TODO dfj variables...needs cleaning up and commenting

// Variables used to calculate the starting sense value (averaging)
uint32_t full_avg = 0;
uint32_t high_avg = 0;
uint32_t  low_avg = 0;

uint8_t  high_count = 0;
uint8_t   low_count = 0;


uint8_t ze_strober = 0;

uint16_t samples[MUXES_COUNT];

uint8_t cur_keymap[MAX_STROBES];

uint8_t keymap_change;

uint16_t threshold = THRESHOLD;

uint8_t column = 0;

uint16_t keys_averages_acc[KEY_COUNT];
uint16_t keys_averages    [KEY_COUNT];
uint8_t  keys_debounce    [KEY_COUNT]; // Contains debounce statistics
uint8_t  keys_problem     [KEY_COUNT]; // Marks keys that should be ignored (determined by averaging at startup)

uint8_t full_samples[KEY_COUNT];

// TODO: change this to 'booting', then count down.
uint16_t boot_count = 0;

uint16_t idle_count = 0;
uint8_t idle = 1;

uint8_t error = 0;
uint16_t error_data = 0;

uint8_t total_strobes = MAX_STROBES;
uint8_t strobe_map[MAX_STROBES];

uint8_t dump_count = 0;



// ----- Function Declarations -----

void dump( void );

void recovery( uint8_t on );

int sampleColumn( uint8_t column );

void capsense_scan( void );

void setup_ADC( void );

void strobe_w( uint8_t strobe_num );

uint8_t testColumn( uint8_t strobe );



// ----- Functions -----

// Initial setup for cap sense controller
inline void scan_setup()
{
	// TODO dfj code...needs cleanup + commenting...
	setup_ADC();

	DDRC  = C_MASK;
	PORTC = 0;
	DDRD  = D_MASK;
	PORTD = 0;
	DDRE  = E_MASK;
	PORTE = 0 ;

	// Hardcoded strobes for debugging
	// Strobes start at 0 and go to 17 (18), not all Model Fs use all of the available strobes
	// The single row ribbon connector Model Fs only have a max of 16 strobes
#define KISHSAVER_STROBE
//#define TERMINAL_6110668_STROBE
//#define UNSAVER_STROBE
#ifdef KISHSAVER_STROBE
	total_strobes = 8;

	strobe_map[0] = 2; // Kishsaver doesn't use strobe 0 and 1
	strobe_map[1] = 3;
	strobe_map[2] = 4;
	strobe_map[3] = 5;
	strobe_map[4] = 6;
	strobe_map[5] = 7;
	strobe_map[6] = 8;
	strobe_map[7] = 9;
	// XXX - Disabling for now, not sure how to deal with test points yet (without spamming the debug)
	total_strobes = 9;
	strobe_map[8] = 15; // Test point strobe (3 test points, sense 1, 4, 5)
#elif defined(TERMINAL_6110668_STROBE)
	total_strobes = 16;

	strobe_map[0] = 0;
	strobe_map[1] = 1;
	strobe_map[2] = 2;
	strobe_map[3] = 3;
	strobe_map[4] = 4;
	strobe_map[5] = 5;
	strobe_map[6] = 6;
	strobe_map[7] = 7;
	strobe_map[8] = 8;
	strobe_map[9] = 9;
	strobe_map[10] = 10;
	strobe_map[11] = 11;
	strobe_map[12] = 12;
	strobe_map[13] = 13;
	strobe_map[14] = 14;
	strobe_map[15] = 15;
#elif defined(UNSAVER_STROBE)
	total_strobes = 14;

	strobe_map[0] = 0;
	strobe_map[1] = 1;
	strobe_map[2] = 2;
	strobe_map[3] = 3;
	strobe_map[4] = 4;
	strobe_map[5] = 5;
	strobe_map[6] = 6;
	strobe_map[7] = 7;
	strobe_map[8] = 8;
	strobe_map[9] = 9;
	strobe_map[10] = 10;
	strobe_map[11] = 11;
	strobe_map[12] = 12;
	strobe_map[13] = 13;
#else
	// Strobe detection
	// TODO
#endif

	// TODO all this code should probably be in scan_resetKeyboard
	for ( int i = 0; i < total_strobes; ++i)
	{
		cur_keymap[i] = 0;
	}

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


	// Reset the keyboard before scanning, we might be in a wierd state
	// Also sets the KeyIndex_BufferUsed to 0
	scan_resetKeyboard();
}


// Main Detection Loop
// This is where the important stuff happens
inline uint8_t scan_loop()
{
	capsense_scan();

	// Error case, should not occur in normal operation
	if ( error )
	{
		erro_msg("Problem detected... ");

		// Keymap scan debug
		for ( uint8_t i = 0; i < total_strobes; ++i )
		{
			printHex(cur_keymap[strobe_map[i]]);
			print(" ");
		}

		print(" : ");
		printHex(error);
		error = 0;
		print(" : ");
		printHex(error_data);
		error_data = 0;

		// Display keymaps and other debug information if warmup completede
		if ( boot_count >= WARMUP_LOOPS )
		{
			dump();
		}
	}


	// Return non-zero if macro and USB processing should be delayed
	// Macro processing will always run if returning 0
	// USB processing only happens once the USB send timer expires, if it has not, scan_loop will be called
	//  after the macro processing has been completed
	return 0;
}


// Reset Keyboard
void scan_resetKeyboard( void )
{
	// Empty buffer, now that keyboard has been reset
	KeyIndex_BufferUsed = 0;
}


// Send data to keyboard
// NOTE: Only used for converters, since the scan module shouldn't handle sending data in a controller
uint8_t scan_sendData( uint8_t dataPayload )
{
	return 0;
}


// Reset/Hold keyboard
// NOTE: Only used for converters, not needed for full controllers
void scan_lockKeyboard( void )
{
}

// NOTE: Only used for converters, not needed for full controllers
void scan_unlockKeyboard( void )
{
}


// Signal KeyIndex_Buffer that it has been properly read
// NOTE: Only really required for implementing "tricks" in converters for odd protocols
void scan_finishedWithBuffer( uint8_t sentKeys )
{
	// Convenient place to clear the KeyIndex_Buffer
	KeyIndex_BufferUsed = 0;
	return;
}


// Signal KeyIndex_Buffer that it has been properly read and sent out by the USB module
// NOTE: Only really required for implementing "tricks" in converters for odd protocols
void scan_finishedWithUSBBuffer( uint8_t sentKeys )
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

	// Scan each of the mapped strobes in the matrix
	for ( uint8_t strober = 0; strober < total_strobes; ++strober )
	{
		uint8_t map_strobe = strobe_map[strober];

		uint8_t tries = 1;
		while ( tries++ && sampleColumn( map_strobe ) ) { tries &= 0x7; } // don't waste this one just because the last one was poop.

		// Only process sense data if warmup is finished
		if ( boot_count >= WARMUP_LOOPS )
		{
			column = testColumn( map_strobe );

			idle |= column; // if column has any pressed keys, then we are not idle.

			// TODO Is this needed anymore? Really only helps debug -HaaTa
			if( column != cur_keymap[map_strobe] && ( boot_count >= WARMUP_LOOPS ) )
			{
				cur_keymap[map_strobe] = column;
				keymap_change = 1;
			}

			idle |= keymap_change; // if any keys have changed inc. released, then we are not idle.
		}

		if ( error == 0x50 )
		{
			error_data |= (((uint16_t)map_strobe) << 12);
		}

		uint8_t strobe_line = map_strobe << MUXES_COUNT_XSHIFT;
		for ( int i = 0; i < MUXES_COUNT; ++i )
		{
			// discard sketchy low bit, and meaningless high bits.
			uint8_t sample = samples[i] >> 1;
			full_samples[strobe_line + i] = sample;
			keys_averages_acc[strobe_line + i] += sample;
		}

		// Accumulate 3 total averages (used for determining starting average during warmup)
		//     full_avg - Average of all sampled lines on the previous scan set
		// cur_full_avg - Average of all sampled lines for this scan set
		//     high_avg - Average of all sampled lines above full_avg on the previous scan set
		// cur_high_avg - Average of all sampled lines above full_avg
		//      low_avg - Average of all sampled lines below or equal to full_avg
		if ( boot_count < WARMUP_LOOPS )
		{
			for ( uint8_t i = 0; i < MUXES_COUNT; ++i )
			{
				uint8_t sample = samples[i] >> 1;

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
				keys_problem[strobe_line + i] = sample > high_avg ? sample : 0;

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

#ifdef VERIFY_TEST_PAD
	// verify test key is not down.
	if ( ( cur_keymap[TEST_KEY_STROBE] & TEST_KEY_MASK ) )
	{
		error = 0x05;
		error_data = cur_keymap[TEST_KEY_STROBE] << 8;
		error_data += full_samples[TEST_KEY_STROBE * 8];
	}
#endif

	/** aggregate if booting, or if idle;
	 * else, if not booting, check for dirty USB.
	 * */

	idle_count++;
	idle_count &= IDLE_COUNT_MASK;

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
			print("\n");
			info_msg("Warmup finished using ");
			printInt16( WARMUP_LOOPS );
			print(" iterations\n");

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
			print("\n");

			// Display problem keys, and the sense value at the time
			for ( uint8_t key = 0; key < KEY_COUNT; key++ )
			{
				if ( keys_problem[key] )
				{
					warn_msg("Problem key detected: ");
					printHex( key );
					print(" (");
					printHex( keys_problem[key] );
					print(")\n");
				}
			}

			info_print("If problem keys were detected, and were being held down, they will be reset as soon as let go");
			break;
		}
	}
	else
	{
		// Reset accumulators and idle flag/counter
		if ( keymap_change )
		{
			for ( uint8_t c = 0; c < KEY_COUNT; ++c ) { keys_averages_acc[c] = 0; }
			idle_count = 0;
			idle = 0;

			keymap_change = 0;
		}

		if ( !idle_count )
		{
			if( idle )
			{
				// aggregate
				for ( uint8_t i = 0; i < KEY_COUNT; ++i )
				{
					uint16_t acc = keys_averages_acc[i] >> IDLE_COUNT_SHIFT;
					uint32_t av = keys_averages[i];

					av = (av << KEYS_AVERAGES_MIX_SHIFT) - av + acc;
					av >>= KEYS_AVERAGES_MIX_SHIFT;

					keys_averages[i] = av;
					keys_averages_acc[i] = 0;
				}
			}

			if ( boot_count >= WARMUP_LOOPS )
			{
				dump();
			}
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

		DDRB  |= (1 << RECOVERY_SINK);	 // SINK pull
		PORTB |= (1 << RECOVERY_CONTROL);
		PORTB |= (1 << RECOVERY_SOURCE); // SOURCE high
		DDRB  |= (1 << RECOVERY_SOURCE);
	}
	else
	{
		PORTB &= ~(1 << RECOVERY_CONTROL);
		DDRB  &= ~(1 << RECOVERY_SOURCE);
		PORTB &= ~(1 << RECOVERY_SOURCE); // SOURCE low
		DDRB  &= ~(1 << RECOVERY_SINK);	  // SINK high-imp
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


int sampleColumn_8x( uint8_t column, uint16_t * buffer )
{
	// ensure all probe lines are driven low, and chill for recovery delay.
	ADCSRA |= (1 << ADEN) | (1 << ADSC); // enable and start conversions

	PORTC &= ~C_MASK;
	PORTD &= ~D_MASK;
	PORTE &= ~E_MASK;

	PORTF = 0;
	DDRF  = 0;

	recovery(OFF);
	strobe_w(column);

	hold_sample(OFF);
	SET_FULL_MUX(0);

	for ( uint8_t i = 0; i < STROBE_SETTLE; ++i ) { getADC(); }

	hold_sample(ON);

	uint8_t mux = 0;
	SET_FULL_MUX(mux);
	getADC(); // throw away; unknown mux.
	do {
		SET_FULL_MUX(mux + 1); // our *next* sample will use this

		// retrieve current read.
		buffer[mux] = getADC();
		mux++;

	} while (mux < 8);

	hold_sample(OFF);
	recovery(ON);

	// turn off adc.
	ADCSRA &= ~(1 << ADEN);

	// pull all columns' strobe-lines low.
	DDRC |= C_MASK;
	DDRD |= D_MASK;
	DDRE |= E_MASK;

	PORTC &= ~C_MASK;
	PORTD &= ~D_MASK;
	PORTE &= ~E_MASK;

	return 0;
}


int sampleColumn( uint8_t column )
{
	int rval = 0;

	rval = sampleColumn_8x( column, samples );

	return rval;
}


uint8_t testColumn( uint8_t strobe )
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
			// If the sample value of the problem key goes below full_avg (overall initial average)
			//  re-enable the key
			if ( (db_sample = samples[mux] >> 1) < full_avg )
			{
				info_msg("Re-enabling problem key: ");
				printHex( key );
				print("\n");

				keys_problem[key] = 0;
			}
			// Otherwise, don't waste any more cycles processing the problem key
			else
			{
				continue;
			}
		}

		// Keypress detected
		//  db_sample (uint8_t), discard meaningless high bit, and garbage low bit
		if ( (db_sample = samples[mux] >> 1) > (db_threshold = threshold) + (db_delta = delta) )
		{
			column |= bit;

			// Only register keypresses once the warmup is complete, or not enough debounce info
			if ( keys_debounce[key] <= DEBOUNCE_THRESHOLD )
			{
				// Add to the Macro processing buffer if debounce criteria met
				// Automatically handles converting to a USB code and sending off to the PC
				if ( keys_debounce[key] == DEBOUNCE_THRESHOLD )
				{
//#define KEYSCAN_DEBOUNCE_DEBUG
#ifdef KEYSCAN_DEBOUNCE_DEBUG
					// Debug message
					print("0x");
					printHex_op( key, 2 );
					print(" ");
#endif

					// Only add the key to the buffer once
					// NOTE: Buffer can easily handle multiple adds, just more efficient
					//        and nicer debug messages :P
					//bufferAdd( key );
				}

				keys_debounce[key]++;

#define KEYSCAN_THRESHOLD_DEBUG
#ifdef KEYSCAN_THRESHOLD_DEBUG
				// Debug message
				// <key> [<strobe>:<mux>] : <sense val> : <delta + threshold> : <margin>
				dbug_msg("0x");
				printHex_op( key, 2 );
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
				print("\n");
#endif
			}
		}
		// Clear debounce entry if no keypress detected
		else
		{
			// If the key was previously pressed, remove from the buffer
			for ( uint8_t c = 0; c < KeyIndex_BufferUsed; c++ )
                        {
                                // Key to release found
                                if ( KeyIndex_Buffer[c] == key )
                                {
                                        // Shift keys from c position
                                        for ( uint8_t k = c; k < KeyIndex_BufferUsed - 1; k++ )
                                                KeyIndex_Buffer[k] = KeyIndex_Buffer[k + 1];

                                        // Decrement Buffer
                                        KeyIndex_BufferUsed--;

                                        break;
                                }
                        }


			// Clear debounce entry
			keys_debounce[key] = 0;
		}

		bit <<= 1;
	}
	return column;
}


void dump(void) {

#ifdef DEBUG_FULL_SAMPLES_AVERAGES
	// we don't want to debug-out during the measurements.
	if ( !dump_count )
	{
		// Averages currently set per key
		for ( int i = 0; i < KEY_COUNT; ++i )
		{
			if ( !(i & 0x0f) )
			{
				print("\n");
			}
			else if ( !(i & 0x07) )
			{
				print("  ");
			}

			print(" ");
			printHex( keys_averages[i] );
		}

		print("\n");

		// Previously read full ADC scans?
		for ( int i = 0; i< KEY_COUNT; ++i)
		{
			if ( !(i & 0x0f) )
			{
				print("\n");
			}
			else if ( !(i & 0x07) )
			{
				print("  ");
			}

			print(" ");
			printHex(full_samples[i]);
		}
	}
#endif

#ifdef DEBUG_STROBE_SAMPLES_AVERAGES
	// Per strobe information
	uint8_t cur_strober = ze_strober;
	print("\n");

	printHex(cur_strober);

	// Previously read ADC scans on current strobe
	print(" :");
	for ( uint8_t i = 0; i < MUXES_COUNT; ++i )
	{
		print(" ");
		printHex(full_samples[(cur_strober << MUXES_COUNT_XSHIFT) + i]);
	}

	// Averages current set on current strobe
	print(" :");

	for ( uint8_t i = 0; i < MUXES_COUNT; ++i )
	{
		print(" ");
		printHex(keys_averages[(cur_strober << MUXES_COUNT_XSHIFT) + i]);
	}

#endif

#ifdef DEBUG_USB_KEYMAP
	print("\n      ");

	// Current keymap values
	for ( uint8_t i = 0; i < total_strobes; ++i )
	{
		printHex(cur_keymap[i]);
		print(" ");
	}
#endif

	ze_strober++;
	ze_strober &= 0xf;

	dump_count++;
	dump_count &= 0x0f;
}

