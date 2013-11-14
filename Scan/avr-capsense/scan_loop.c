/* Copyright (C) 2011-2013 by Joseph Makuch
 * Additions by Jacob Alexander (2013)
 *
 * dfj, put whatever license here you want -HaaTa
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

// TODO dfj defines...needs cleaning up and commenting...
#define THRESHOLD 0x0a
#define BUMP_THRESHOLD 0x50
//((THRESHOLD) * 3)
#define BUMP_REST_US 1200

#define HYST 1
#define HYST_T 0x10

#define TEST_KEY_STROBE (0x05)
#define TEST_KEY_MASK (1 << 0)

#define ADHSM 7

/** Whether to use all of D and C, vs using E0, E1 instead of D6, D7,
 * or alternately all of D, and E0,E1 and C0,..5 */
//#define ALL_D_C
//#define SHORT_D
#define SHORT_C

// rough offset voltage: one diode drop, about 50mV = 0x3ff * 50/3560 = 20
//#define OFFSET_VOLTAGE 0x14
#define OFFSET_VOLTAGE 0x28


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

#define MUX_1_1 0x1e
#define MUX_GND 0x1f


	// set ADC clock prescale
#define PRESCALE_MASK ((1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2))
#define PRESCALE_SHIFT (ADPS0)
#define PRESCALE 3


#ifdef EXTENDED_STROBE

#define STROBE_LINES 18

#else

#define STROBE_LINES 16

#endif

#define STROBE_LINES_XSHIFT 4
#define STROBE_LINES_MASK 0x0f
#define MUXES_COUNT 8
#define MUXES_COUNT_XSHIFT 3
#define MUXES_MASK 0x7

#define WARMUP_LOOPS ( 1024 )

#define RECOVERY_US 6

#define SAMPLES 10


#define SAMPLE_OFFSET ((SAMPLES) - MUXES_COUNT)
//#define SAMPLE_OFFSET 9
#define STROBE_OFFSET 0


#define KEY_COUNT ((STROBE_LINES) * (MUXES_COUNT))

#define LX2FX


#define RECOVERY_CONTROL 1

#define RECOVERY_SOURCE 0
#define RECOVERY_SINK 2
#define RECOVERY_MASK 0x03


// mix in 1/4 of the current average to the running average. -> (@mux_mix = 2)
#define MUX_MIX 2


#define IDLE_COUNT_MASK 0xff
#define MAX_ICS 8

#define IDLE_COUNT_SHIFT 4
#define KEYS_AVERAGES_MIX 2


#ifdef ALL_D_C

#define D_MASK (0xff)
#define D_SHIFT 0

#define E_MASK (0x00)
#define E_SHIFT 0

#define C_MASK (0xff)
#define C_SHIFT 8

#else
#if defined(SHORT_D)

#define D_MASK (0x3f)
#define D_SHIFT 0

#define E_MASK (0x03)
#define E_SHIFT 6

#define C_MASK (0xff)
#define C_SHIFT 8

#else
#if defined(SHORT_C)

#define D_MASK (0xff)
#define D_SHIFT 0

#define E_MASK (0x03)
#define E_SHIFT 6

#define C_MASK (0xff)
#define C_SHIFT 8
#endif
#endif
#endif





// ----- Macros -----

// Make sure we haven't overflowed the buffer
#define bufferAdd(byte) \
		if ( KeyIndex_BufferUsed < KEYBOARD_BUFFER ) \
			KeyIndex_Buffer[KeyIndex_BufferUsed++] = byte


// TODO dfj macros...needs cleaning up and commenting...
#define STROBE_CASE(SC_CASE, SC_REG_A) case (SC_CASE): PORT##SC_REG_A = \
	(( (PORT##SC_REG_A) & ~(1 << (SC_CASE - SC_REG_A##_SHIFT)) ) | (1 << (SC_CASE - SC_REG_A##_SHIFT)))

#define SET_MUX(X) ((ADMUX) = (((ADMUX) & ~(MUX_MASK)) | ((X) & (MUX_MASK))))
#define SET_FULL_MUX(X) ((ADMUX) = (((ADMUX) & ~(FULL_MUX_MASK)) | ((X) & (FULL_MUX_MASK))))





// ----- Variables -----

// Buffer used to inform the macro processing module which keys have been detected as pressed
volatile uint8_t KeyIndex_Buffer[KEYBOARD_BUFFER];
volatile uint8_t KeyIndex_BufferUsed;


// TODO dfj variables...needs cleaning up and commenting
         uint8_t  blink = 0;
volatile uint8_t  idle_count = 1;
volatile uint16_t full_av = 0;

/**/ uint8_t ze_strober = 0;

int16_t samples [SAMPLES];

//int16_t gsamples [SAMPLES];

/**/ int16_t adc_mux_averages[MUXES_COUNT];
/**/ int16_t adc_strobe_averages[STROBE_LINES];


/**/ uint8_t cur_keymap[STROBE_LINES];
// /**/ int8_t last_keymap[STROBE_LINES];
/**/ uint8_t usb_keymap[STROBE_LINES];
uint8_t dirty;
uint8_t unstable;
uint8_t usb_dirty;

int16_t threshold = THRESHOLD;
uint16_t tests = 0;

uint8_t col_a=0;
uint8_t col_b=0;
uint8_t col_c=0;

uint8_t column=0;


int16_t keys_averages_acc[KEY_COUNT];
uint16_t keys_averages[KEY_COUNT];

uint8_t full_samples[KEY_COUNT];

/* viable starting biases for near 0.830V offset. and adc PRESCALE 3
0017 0016 001B 001A 0016 0016 000F 000E 001B 001E 001E 0018 0017 0015 000E 001D
001B 001A 0016 0016 000F 000E 001C 001B 001E 0018 0017 0015 000E 001D 0024 001F
0016 0016 000F 000E 001C 001B 001E 001E 0017 0015 000E 001D 0024 001F 0020 001F
000F 000E 001C 001B 001E 001E 0018 0017 000E 001D 0024 001F 0020 001F 0020 0017
001C 001B 001E 001E 0018 0017 0015 000E 0024 001F 0020 001F 0020 0017 0010 001D
001E 001E 0018 0017 0015 000E 001D 0024 0020 001F 0020 0017 0010 001D 0024 0021
0018 0017 0015 000E 001D 0024 001F 0020 0020 0017 0010 001D 0024 0021 0021 0021
0015 000E 001D 0024 001F 0020 001F 0020 0010 001D 0024 0021 0021 0021 0021 0018
*/

/*** starting bias relative to fixed offset estimate of 820mV (0x50)
 *   77 69 65 5B 50 4E 4C 45   66 53 4D 49 45 3F 3E 35
 *   68 54 4F 49 45 40 3F 34   74 66 5F 56 4E 4D 4C 3F
 *   6D 5D 53 4C 49 46 45 38   6D 5A 53 4E 49 48 45 3E
 *   6F 5D 56 4E 4B 48 48 3A   6D 5C 54 4E 48 48 45 37
 *   75 68 5F 57 4F 4D 4C 3F   60 4E 48 41 3C 3C 39 2F
 *   65 53 4E 49 41 3F 3E 34   65 54 4E 49 43 3F 3E 34
 *   60 51 4A 45 3F 3E 3C 30   57 4C 45 3E 3B 37 37 2E
 *   64 4E 48 44 3C 3B 39 2F   5D 4F 48 45 3E 3C 3B 30
 */

/*volatile*/ uint16_t count = 0;

/*volatile*/ uint8_t error = 0;
uint16_t error_data = 0;


int16_t mux_averages[MUXES_COUNT];
int16_t strobe_averages[STROBE_LINES];

uint8_t dump_count = 0;



// ----- Function Declarations -----

void dump    ( void );
void dumpkeys( void );

void recovery( uint8_t on );

int sampleColumn  ( uint8_t column );
//int sampleColumn_i( uint8_t column, uint8_t muxes, int16_t * buffer); // XXX Not currently used
int sampleColumn_k( uint8_t column, int16_t *buffer );

void setup_ADC( void );

void strobe_w( uint8_t strobe_num );

uint8_t testColumn( uint8_t strobe );



// ----- Functions -----

// Initial setup for cap sense controller
inline void scan_setup()
{
	// TODO dfj code...needs cleanup + commenting...
	setup_ADC();

	// Configure timer 0 to generate a timer overflow interrupt every
	// 256*1024 clock cycles, or approx 61 Hz when using 16 MHz clock
	// This demonstrates how to use interrupts to implement a simple
	// inactivity timeout.
	//TCCR0A = 0x00;
	//TCCR0B = 0x05;
	//TIMSK0 = (1<<TOIE0);

	DDRC = C_MASK;
	PORTC = 0;
	DDRD = D_MASK;
	PORTD = 0;
	DDRE = E_MASK;
	PORTE = 0 ;

	//DDRC |= (1 << 6);
	//PORTC &= ~(1<< 6);

	//uint16_t strobe = 1;


	// TODO all this code should probably be in scan_resetKeyboard
	for (int i=0; i< STROBE_LINES; ++i) {
		cur_keymap[i] = 0;
		//last_keymap[i] = 0;
		usb_keymap[i] = 0;
	}

	for(int i=0; i < MUXES_COUNT; ++i) {
		adc_mux_averages[i] = 0x20; // experimentally determined.
	}
	for(int i=0; i < STROBE_LINES; ++i) {
		adc_strobe_averages[i] = 0x20; // yup.
	}

	for(int i=0; i< KEY_COUNT; ++i) {
		keys_averages[i] = 0x40;
		keys_averages_acc[i] = (0x400);
	}

	/** warm things up a bit before we start collecting data, taking real samples. */
	for(uint8_t i = 0; i< STROBE_LINES; ++i) {
		sampleColumn(i);
	}


	// Reset the keyboard before scanning, we might be in a wierd state
	// Also sets the KeyIndex_BufferUsed to 0
	scan_resetKeyboard();
}


// Main Detection Loop
// This is where the important stuff happens
inline uint8_t scan_loop()
{
	// TODO dfj code...needs commenting + cleanup...
	uint8_t strober = 0;
	uint32_t full_av_acc = 0;

	for (strober = 0; strober < STROBE_LINES; ++strober) {

		uint8_t tries;
		tries = 1;
		while (tries++ && sampleColumn(strober)) { tries &= 0x7; } // don't waste this one just because the last one was poop.
		column = testColumn(strober);

		if( column != cur_keymap[strober] && (count >= WARMUP_LOOPS) ) {
			tests++;

			tries = 1;
			while (tries++ && sampleColumn(strober)) { tries &= 0x7; } // don't waste this one just because the last one was poop.
			col_a = testColumn(strober);

			tries = 1;
			while (tries++ && sampleColumn(strober)) { tries &= 0x7; } // don't waste this one just because the last one was poop.
			col_b = testColumn(strober);

			tries = 1;
			while (tries++ && sampleColumn(strober)) { tries &= 0x7; } // don't waste this one just because the last one was poop.
			col_c = testColumn(strober);

			if( (col_a == col_b) && (col_b == col_c) && (cur_keymap[strober] != col_a) ) {
				cur_keymap[strober] = col_a;
				usb_dirty = 1;
			}
		}

		if(error == 0x50) {
			error_data |= (((uint16_t)strober) << 12);
		}

		for(int i=0; i<MUXES_COUNT; ++i) {
			full_samples[(strober << MUXES_COUNT_XSHIFT) + i] = samples[SAMPLE_OFFSET + i];
		}

		strobe_averages[strober] = 0;
		for (uint8_t i = SAMPLE_OFFSET; i < (SAMPLE_OFFSET + MUXES_COUNT); ++i) {
			//samples[i] -= samples[i-SAMPLE_OFFSET]; // av; // + full_av); // -something.
			//samples[i] -= OFFSET_VOLTAGE; // moved to sampleColumn.

			full_av_acc += (samples[i]);
			mux_averages[i - SAMPLE_OFFSET] += samples[i];
			strobe_averages[strober] += samples[i];
			//samples[i] -= (full_av - HYST_T);

			//++count;
		}
		adc_strobe_averages[strober] += strobe_averages[strober] >> 3;
		adc_strobe_averages[strober] >>= 1;

		/** test if we went negative. */
		if ((adc_strobe_averages[strober] & 0xFF00) && (count
				>= WARMUP_LOOPS)) {
			//count = 0; // TODO : constrain properly.
			error = 0xf; error_data = adc_strobe_averages[strober];
		}

		uint8_t strobe_line = strober << MUXES_COUNT_XSHIFT;
		for (int i = 0; i < MUXES_COUNT; ++i) {
			keys_averages_acc[strobe_line + i]
					+= samples[SAMPLE_OFFSET + i];
		}

	} // for strober

	if (count < WARMUP_LOOPS) {
		error = 0x0C;
		error_data = count;
		count++;
	}

	// verify test key is not down.
	if((cur_keymap[TEST_KEY_STROBE] & TEST_KEY_MASK) ) {
		//count=0;
		error = 0x05;
		error_data = cur_keymap[TEST_KEY_STROBE] << 8;
		error_data += full_samples[TEST_KEY_STROBE * 8];
		//threshold++;
	}

	// calc mux averages.
	if (count < WARMUP_LOOPS) {
		full_av += (full_av_acc >> (7));
		full_av >>= 1;
		//full_av = full_av_acc / count;
		full_av_acc = 0;
		for (int i=0; i < MUXES_COUNT; ++i) {

			adc_mux_averages[i] = (adc_mux_averages[i] << MUX_MIX) - adc_mux_averages[i];
			adc_mux_averages[i] += (mux_averages[i] >> 4);
			adc_mux_averages[i] >>= MUX_MIX;

			mux_averages[i] = 0;
		}

	}

	idle_count++;
	idle_count &= IDLE_COUNT_MASK;

	if (/*usb_dirty &&*/ (count >= WARMUP_LOOPS) ) {
		for (int i=0; i<STROBE_LINES; ++i) {
			usb_keymap[i] = cur_keymap[i];
		}

		dumpkeys();
		usb_dirty=0;
		_delay_ms(2);
	}

	if (count < WARMUP_LOOPS) {
		for (uint8_t i = 0; i < KEY_COUNT; ++i) {
			uint16_t acc = keys_averages_acc[i];
			uint32_t av = keys_averages[i];

			av = av + av + av + acc;
			av >>= 2;

			keys_averages[i] = av;
			keys_averages_acc[i] = 0;
		}
	}


	if(!idle_count) {

		for (int i=0; i< KEY_COUNT; ++i) {
			keys_averages_acc[i] = 0;
		}

		sampleColumn(0x0); // to resync us if we dumped a mess 'o text.
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


void
_delay_loop(uint8_t __count)
{
	__asm__ volatile (
		"1: dec %0" "\n\t"
		"brne 1b"
		: "=r" (__count)
		: "0" (__count)
	);
}


void setup_ADC (void) {
	// disable adc digital pins.
	DIDR1 |= (1 << AIN0D) | (1<<AIN1D); // set disable on pins 1,0.
	//DIDR0 = 0xff; // disable all. (port F, usually). - testing w/o disable.
	DDRF = 0x0;
	PORTF = 0x0;
	uint8_t mux = 0 & 0x1f; // 0 == first. // 0x1e = 1.1V ref.

	// 0 = external aref 1,1 = 2.56V internal ref
	uint8_t aref = ((1 << REFS1) | (1 << REFS0)) & ((1 << REFS1) | (1 << REFS0));
//	uint8_t adlar = 0xff & (1 << ADLAR); // 1 := left justify bits, 0 := right
	uint8_t adate = (1 << ADATE) & (1 << ADATE); // trigger enable
	uint8_t trig = 0 & ((1 << ADTS0) | (1 << ADTS1) | (1 << ADTS2)); // 0 = free running
	// ps2, ps1 := /64 ( 2^6 ) ps2 := /16 (2^4), ps1 := 4, ps0 :=2, PS1,PS0 := 8 (2^8)
	uint8_t prescale = ( ((PRESCALE) << PRESCALE_SHIFT) & PRESCALE_MASK ); // 001 == 2^1 == 2
	uint8_t hispeed = (1 << ADHSM);
	uint8_t en_mux = (1 << ACME);

	//ADCSRA = (ADCSRA & ~PRESCALES) | ((1 << ADPS1) | (1 << ADPS2)); // 2, 1 := /64 ( 2^6 )
	//ADCSRA = (ADCSRA & ~PRESCALES) | ((1 << ADPS0) | (1 << ADPS2)); // 2, 0 := /32 ( 2^5 )
	//ADCSRA = (ADCSRA & ~PRESCALES) | ((1 << ADPS2)); // 2 := /16 ( 2^4 )

	ADCSRA = (1 << ADEN) | prescale; // ADC enable

	// select ref.
	//ADMUX |= ((1 << REFS1) | (1 << REFS0)); // 2.56 V internal.
	//ADMUX |= ((1 << REFS0) ); // Vcc with external cap.
	//ADMUX &= ~((1 << REFS1) | (1 << REFS0)); // 0,0 : aref.
	ADMUX = aref | mux | ADLAR_BITS;

	// enable MUX
	// ADCSRB |= (1 << ACME); 	// enable
	// ADCSRB &= ~(1 << ADEN); // ?

	// select first mux.
	//ADMUX = (ADMUX & ~MUXES); // start at 000 = ADC0

	// clear adlar to left justify data
	//ADMUX = ~();

	// set adlar to right justify data
	//ADMUX |= (1 << ADLAR);


	// set free-running
	ADCSRA |= adate; // trigger enable
	ADCSRB  = en_mux | hispeed | trig | (ADCSRB & ~((1 << ADTS0) | (1 << ADTS1) | (1 << ADTS2))); // trigger select free running

//	ADCSRA |= (1 << ADATE); // tiggger enable

	ADCSRA |= (1 << ADEN); // ADC enable
	ADCSRA |= (1 << ADSC); // start conversions q

}


void recovery(uint8_t on) {
	DDRB |= (1 << RECOVERY_CONTROL);

	PORTB &= ~(1 << RECOVERY_SINK);    // SINK always zero
	DDRB &= ~(1 << RECOVERY_SOURCE);  // SOURCE high imp

	if(on) {
		DDRB |= (1 << RECOVERY_SINK);	// SINK pull


		PORTB |= (1 << RECOVERY_CONTROL);

		PORTB |= (1 << RECOVERY_SOURCE); // SOURCE high
		DDRB |= (1 << RECOVERY_SOURCE);
	} else {
		_delay_loop(10);
		PORTB &= ~(1 << RECOVERY_CONTROL);

		DDRB &= ~(1 << RECOVERY_SOURCE);
		PORTB &= ~(1 << RECOVERY_SOURCE); // SOURCE low
		DDRB &= ~(1 << RECOVERY_SINK);	// SINK high-imp

		//DDRB &= ~(1 << RECOVERY_SINK);
	}
}


void strobe_w(uint8_t strobe_num) {

	PORTC &= ~(C_MASK);
	PORTD &= ~(D_MASK);
	PORTE &= ~(E_MASK);

#ifdef SHORT_C
	strobe_num = 15 - strobe_num;
#endif
	/*
	printHex( strobe_num );
	print(" ");
	strobe_num = 9 - strobe_num;
	printHex( strobe_num );
	print("\n");
	*/

	switch(strobe_num) {

	// XXX Kishsaver strobe (note that D0, D1 are not used)
	case 2: PORTD |= (1 << 2); break;
	case 3: PORTD |= (1 << 3); break;
	case 4: PORTD |= (1 << 4); break;
	case 5: PORTD |= (1 << 5); break;

	// TODO REMOVEME
	case 6: PORTD |= (1 << 6); break;
	case 7: PORTD |= (1 << 7); break;
	case 8: PORTE |= (1 << 0); break;
	case 9: PORTE |= (1 << 1); break;
	case 15: PORTC |= (1 << 5); break;
/*
#ifdef ALL_D

	case 6: PORTD |= (1 << 6); break;
	case 7: PORTD |= (1 << 7); break;

	case 8:  PORTC |= (1 << 0); break;
	case 9:  PORTC |= (1 << 1); break;
	case 10: PORTC |= (1 << 2); break;
	case 11: PORTC |= (1 << 3); break;
	case 12: PORTC |= (1 << 4); break;
	case 13: PORTC |= (1 << 5); break;
	case 14: PORTC |= (1 << 6); break;
	case 15: PORTC |= (1 << 7); break;

	case 16: PORTE |= (1 << 0); break;
	case 17: PORTE |= (1 << 1); break;

#else
#ifdef SHORT_D

	case 6: PORTE |= (1 << 0); break;
	case 7: PORTE |= (1 << 1); break;

	case 8:  PORTC |= (1 << 0); break;
	case 9:  PORTC |= (1 << 1); break;
	case 10: PORTC |= (1 << 2); break;
	case 11: PORTC |= (1 << 3); break;
	case 12: PORTC |= (1 << 4); break;
	case 13: PORTC |= (1 << 5); break;
	case 14: PORTC |= (1 << 6); break;
	case 15: PORTC |= (1 << 7); break;

#else
#ifdef SHORT_C

	case 6: PORTD |= (1 << 6); break;
	case 7: PORTD |= (1 << 7); break;

	case 8: PORTE |= (1 << 0); break;
	case 9: PORTE |= (1 << 1); break;

	case 10:  PORTC |= (1 << 0); break;
	case 11:  PORTC |= (1 << 1); break;
	case 12: PORTC |= (1 << 2); break;
	case 13: PORTC |= (1 << 3); break;
	case 14: PORTC |= (1 << 4); break;
	case 15: PORTC |= (1 << 5); break;

	case 16: PORTC |= (1 << 6); break;
	case 17: PORTC |= (1 << 7); break;

#endif
#endif
#endif
*/

	default:
		break;
	}

}

#if 0
int sampleColumn_i(uint8_t column, uint8_t muxes, int16_t * buffer) {

	// ensure all probe lines are driven low, and chill for recovery delay.
	PORTC &= ~C_MASK;
	PORTD &= ~D_MASK;
	PORTE &= ~E_MASK;
	recovery(1);
	_delay_us(RECOVERY_US);
	recovery(0);

	//uint8_t index = 0;

	for (uint8_t i=0; i<8; ++i) {
		if(muxes & (1 << i)) {
			buffer[index++] = i;
		}
	}

	SET_FULL_MUX(MUX_1_1); // crap sample will use this.
	ADCSRA |= (1 << ADEN) | (1 << ADSC); // enable and start conversions
	ADCSRA |= (1 << ADIF); // clear int flag by writing 1.

	//uint16_t sample;

	while (! (ADCSRA & (1 << ADIF))); // wait until ready.
	sample = ADC; // 1st sample, icky.
	//ADC; // 1st sample, icky. XXX Not sure if the compiler throws this away, but less compiler warnings -HaaTa

	strobe_w(column);
	//recovery(0);

	/**
	 * we are running in continuous mode, so we must setup the next
	 * read _before_ the current read completes.
	 *
	 * setup 0,
	 * read garbage,
	 * do not store
	 *
	 * setup 1,
	 * read 0,
	 * store 0,
	 *
	 * ...
	 *
	 * setup junk,
	 * read n
	 * store n
	 *
	 * */


	ADCSRA |= (1 << ADIF); // clear int flag by writing 1.
	//wait for last read to complete.
	while (! (ADCSRA & (1 << ADIF)));
	sample = ADC; // throw away strobe'd value.
	//ADC; // throw away strobe'd value.

#if 0
	for (uint8_t i=0; i <= index; ++i) {

		// setup i'th read.
		SET_FULL_MUX(buffer[i]); // _next_ read will use this.
		// wait for i-1'th read to complete:
		ADCSRA |= (1 << ADIF); // clear int flag by writing 1.
		while (! (ADCSRA & (1 << ADIF)));

		// retrieve last (i-1'th) read.
		if (i) {
			buffer[i-1] = ADC - OFFSET_VOLTAGE;
		} /*else {
			buffer[0] = ADC - OFFSET_VOLTAGE;
		}*/

		//index++;
	}
#else
	for (uint8_t i=0; i < index; ++i) {

		// setup i'th read.
		SET_FULL_MUX(buffer[i]); // _next_ read will use this.

		ADCSRA |= (1 << ADIF); // clear int flag by writing 1.
		while (! (ADCSRA & (1 << ADIF)));
		sample = ADC; // throw away warmup value.
		//ADC; // throw away warmup value.



		/*
		ADCSRA |= (1 << ADIF); // clear int flag by writing 1.
		while (! (ADCSRA & (1 << ADIF)));
		//sample = ADC; // throw away warmup value.
		ADC; // throw away warmup value.
*/

		ADCSRA |= (1 << ADIF); // clear int flag by writing 1.
		while (! (ADCSRA & (1 << ADIF)));

		// retrieve current read.
		buffer[i] = ADC - OFFSET_VOLTAGE;


	}
#endif


	// turn off adc.
	ADCSRA &= ~(1 << ADEN);

	// pull all columns' probe-lines low.
	PORTC &= ~C_MASK;
	PORTD &= ~D_MASK;
	PORTE &= ~E_MASK;

	// test for humps. :/
	/*uint16_t delta = full_av;
	if(buffer[0] > BUMP_THRESHOLD + delta) {
		// ze horror.
		return 1;
	} else {
		return 0; //all good.
	}*/
	return 0;

}
#endif


int sampleColumn_k(uint8_t column, int16_t * buffer) {
	// ensure all probe lines are driven low, and chill for recovery delay.
	uint16_t sample;

	ADCSRA |= (1 << ADEN) | (1 << ADSC); // enable and start conversions
	ADCSRA |= (1 << ADIF); // clear int flag by writing 1.

	// sync up with adc clock:
	while (! (ADCSRA & (1 << ADIF))); // wait until ready.
	//ADC; // throw it away. // XXX Not sure if the compiler throws this away, but less compiler warnings -HaaTa
	sample = ADC; // throw it away.

	for(uint8_t mux=0; mux < 8; ++mux) {

		PORTC &= ~C_MASK;
		PORTD &= ~D_MASK;
		PORTE &= ~E_MASK;

		SET_FULL_MUX(mux); // our sample will use this

		for(uint8_t i=0; i < 2; ++i) {
			ADCSRA |= (1 << ADIF); // clear int flag by writing 1.
			//wait for last read to complete.
			while (! (ADCSRA & (1 << ADIF)));
			sample = ADC; // throw away strobe'd value.
			//ADC; // throw away strobe'd value.
		}

		recovery(0);
		strobe_w(column);

		ADCSRA |= (1 << ADIF); // clear int flag by writing 1.
		//wait for last read to complete.
		while (! (ADCSRA & (1 << ADIF)));
		sample = ADC; // throw away strobe'd value.
		//ADC; // throw away strobe'd value.

		ADCSRA |= (1 << ADIF); // clear int flag by writing 1.
		while (! (ADCSRA & (1 << ADIF)));

		// retrieve current read.
		buffer[mux] = ADC - OFFSET_VOLTAGE;
		recovery(1);

	}

	// turn off adc.
	ADCSRA &= ~(1 << ADEN);

	// pull all columns' probe-lines low.
	PORTC &= ~C_MASK;
	PORTD &= ~D_MASK;
	PORTE &= ~E_MASK;
//		recovery(1);


	return 0;
}


int sampleColumn(uint8_t column) {
	int rval = 0;

	/*
	sampleColumn_i(column, 0x0f, samples+SAMPLE_OFFSET);
	sampleColumn_i(column, 0xf0, samples+SAMPLE_OFFSET + 4 );
*/

	rval = sampleColumn_k(column, samples+SAMPLE_OFFSET);

	//for(uint8_t i=0; i<1; ++i) { // TODO REMOVEME
	for(uint8_t i=0; i<8; ++i) {
		if(samples[SAMPLE_OFFSET + i] - adc_mux_averages[i] > BUMP_THRESHOLD) {
			// was a hump

			_delay_us(BUMP_REST_US);
			rval++;
			error = 0x50;
			error_data = samples[SAMPLE_OFFSET +i]; //  | ((uint16_t)i << 8);
			return rval;
		}
	}

	return rval;
}


uint8_t testColumn(uint8_t strobe) {
    uint8_t column = 0;
    uint8_t bit = 1;
    for (uint8_t i=0; i < MUXES_COUNT; ++i) {
		uint16_t delta = keys_averages[(strobe << MUXES_COUNT_XSHIFT) + i];
		if ((int16_t)samples[SAMPLE_OFFSET + i] > threshold + delta) {
			column |= bit;
		}
		bit <<= 1;
	}
    return column;
}


void dumpkeys(void) {
	//print(" \n");
	if(error) {
		if (count >= WARMUP_LOOPS && error) {
			dump();
		}

		// Key scan debug
		/*
		for (uint8_t i=0; i < STROBE_LINES; ++i) {
				printHex(usb_keymap[i]);
				print(" ");
		}

		print(" : ");
		printHex(error);
		error = 0;
		print(" : ");
		printHex(error_data);
		error_data = 0;
		print(" : " NL);
		*/
	}

	// XXX Will be cleaned up eventually, but this will do for now :P -HaaTa
	for (uint8_t i=0; i < STROBE_LINES; ++i) {
		for(uint8_t j=0; j<MUXES_COUNT; ++j) {
			if ( usb_keymap[i] & (1 << j) ) {
				uint8_t key = (i << MUXES_COUNT_XSHIFT) + j;

				// Add to the Macro processing buffer
				// Automatically handles converting to a USB code and sending off to the PC
				//bufferAdd( key );

				if(usb_dirty)
				{
				/*
					printHex( key );
					print(" ");
				*/
				}
			}
		}
	}
	//if(usb_dirty) print("\n");
	usb_keyboard_send();
}


void dump(void) {

	if(!dump_count) {  // we don't want to debug-out during the measurements.

		// Averages currently set per key
		for(int i =0; i< KEY_COUNT; ++i) {
			if(!(i & 0x0f)) {
				print("\n");
			} else if (!(i & 0x07)) {
				print("  ");
			}
			print(" ");
			//printHex (keys_averages[(i >> MUXES_COUNT_XSHIFT) + (i & STROBE_LINES_MASK) ]);
			printHex (keys_averages[i]);
		}

		print("\n");

		// Previously read full ADC scans?
		for(int i =0; i< KEY_COUNT; ++i) {
			if(!(i & 0x0f)) {
				print("\n");
			} else if (!(i & 0x07)) {
				print("  ");
			}
			print(" ");
			//printHex (keys_averages[(i >> MUXES_COUNT_XSHIFT) + (i & STROBE_LINES_MASK) ]);
			//printHex (keys_averages_acc[i]);
			printHex(full_samples[i]);
		}
	}

	// Per strobe information
//	uint8_t cur_strober = 0xe;
	uint8_t cur_strober = ze_strober;
	print("\n");

	printHex(cur_strober);
	//print(":         ");
#if 1
	// Previously read ADC scans on current strobe
	print(" :");
	for (uint8_t i=0; i < MUXES_COUNT; ++i) {
		print(" ");
		printHex(full_samples[(cur_strober << MUXES_COUNT_XSHIFT) + i]);
	}

	// Averages current set on current strobe
	print(" :");
//	printHex(threshold);
	for (uint8_t i=0; i < MUXES_COUNT; ++i) {
		print(" ");
		printHex(keys_averages[(cur_strober << MUXES_COUNT_XSHIFT) + i]);
	}

#endif
	/*
	for (uint8_t i=0; i< SAMPLES; ++i) {
		print(" ");
		printHex(samples[i]);
		//printHex(ADC);
	}*/
	//print(" : ");
	//dPrint((was_active)?" ":"*");

	//printHex(keymap[TEST_KEY_STROBE] & TEST_KEY_MASK);
	/*print(" "); */
	//printHex(keymap[TEST_KEY_STROBE]);


	//print("\n");
	//print(":");
	//printHex(full_av);
	//printHex(count);
	//print(" : ");
	print("\n      ");

	// Current keymap values
	for (uint8_t i=0; i < STROBE_LINES; ++i) {
		printHex(cur_keymap[i]);
		print(" ");

		//print(" ");
	}


	//print(": ");
	//printHex(adc_strobe_averages[ze_strober]);
	//print(" ");


	/* Already printing this above...
	for (uint8_t i=0; i < MUXES_COUNT; ++i) {
		print(" ");
		//printHex(adc_mux_averages[i] + adc_strobe_averages[ze_strober] - full_av);
		//printHex((adc_mux_averages[i] + adc_strobe_averages[ze_strober]) >> 1);
		//printHex((adc_mux_averages[i] * 3  + adc_strobe_averages[ze_strober]) >> 2);
		//printHex(adc_mux_averages[i] + threshold);
		//printHex(gsamples[i + SAMPLE_OFFSET] - (adc_mux_averages[i] + threshold) + 0x100);
		//printHex(keys_averages[(ze_strober << MUXES_COUNT_XSHIFT) + i] + (uint8_t)threshold);
		printHex(keys_averages[(ze_strober << MUXES_COUNT_XSHIFT) + i]);
	}
	*/

	/* Being printed in dumpkeys()
	if(error) {
		print(" ");
		printHex(error);
		print(" ");
		printHex(error_data);
		error = 0;
		error_data = 0;
	}
	//print("\n");
	*/

	ze_strober++;
	ze_strober &= 0xf;


	dump_count++;
	dump_count &= 0x0f;



	//ze_strobe = (1 << (ze_strober ) );



	//printHex(ADCSRA);
	//print(" ");
	//print("\n");
}

