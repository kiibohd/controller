/* Entropy - A entropy (random number) generator for the Arduino
 * Forked from https://github.com/pmjdebruijn/Arduino-Entropy-Library (2017)
 *
 *   The latest version of this library will always be stored in the following
 *   google code repository:
 *     http://code.google.com/p/avr-hardware-random-number-generation/source/browse/#git%2FEntropy
 *   with more information available on the libraries wiki page
 *     http://code.google.com/p/avr-hardware-random-number-generation/wiki/WikiAVRentropy
 *
 * Copyright 2014 by Walter Anderson
 * Modifications 2017-2018 by Jacob Alexander
 *
 * Entropy is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Entropy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Entropy.  If not, see <http://www.gnu.org/licenses/>.
 */

// ----- Target Includes -----

#include <kll_defs.h>
#include "mcu_compat.h"



#if defined(_host_)


// ----- Includes -----

#include <time.h>
#include <stdlib.h>

#include "entropy.h"



// ----- Functions -----

void rand_initialize()
{
}

void rand_disable()
{
}

uint8_t rand_available()
{
	return 1;
}

// Pseudo-random value using clock
// XXX (HaaTa) Host shouldn't need random numbers.
//             If they are needed, this should be changed to something better.
uint32_t rand_value32()
{
	srand( time( NULL ) );
	return rand();
}


#elif defined(_kinetis_)


// ----- Includes -----

#include "atomic.h"
#include "entropy.h"

#include "kinetis.h"



// ----- Defines -----

#define WDT_MAX_8INT  0xFF
#define WDT_MAX_16INT 0xFFFF
#define WDT_MAX_32INT 0xFFFFFFFF
#define WDT_POOL_SIZE 8

#define gWDT_buffer_SIZE 32



// ----- Variables -----

         uint8_t  gWDT_buffer[gWDT_buffer_SIZE];
         uint8_t  gWDT_buffer_position;
         uint8_t  gWDT_loop_counter;
volatile uint8_t  gWDT_pool_start;
volatile uint8_t  gWDT_pool_end;
volatile uint8_t  gWDT_pool_count;
volatile uint32_t gWDT_entropy_pool[WDT_POOL_SIZE];



// ----- Functions -----

// This function initializes the global variables needed to implement the circular entropy pool and
// the buffer that holds the raw Timer 1 values that are used to create the entropy pool.  It then
// Initializes the Low Power Timer (LPTMR) to perform an interrupt every 2048 clock cycles, (about
// 16 ms) which is as fast as it can be set.
void rand_initialize()
{
	gWDT_buffer_position = 0;
	gWDT_pool_start = 0;
	gWDT_pool_end = 0;
	gWDT_pool_count = 0;

	SIM_SCGC5 |= SIM_SCGC5_LPTIMER;
	LPTMR0_CSR = 0b10000100;
	LPTMR0_PSR = 0b00000101; // PCS=01 : 1 kHz clock
	LPTMR0_CMR = 0x0006;     // smaller number = faster random numbers...
	LPTMR0_CSR = 0b01000101;
	NVIC_ENABLE_IRQ( IRQ_LPTMR );
}


// Disables interrupt, thus stopping CPU usage generating entropy
void rand_disable()
{
	NVIC_DISABLE_IRQ( IRQ_LPTMR );
}


// This function returns a uniformly distributed random integer in the range
// of [0,0xFFFFFFFF] as long as some entropy exists in the pool and a 0
// otherwise.  To ensure a proper random return the available() function
// should be called first to ensure that entropy exists.
//
// The pool is implemented as an 8 value circular buffer
uint32_t rand_value32()
{
	uint32_t retVal = 0;
	uint8_t waiting;
	while ( gWDT_pool_count < 1 )
	{
		waiting += 1;
	}

	ATOMIC_BLOCK( ATOMIC_RESTORESTATE )
	{
		retVal = gWDT_entropy_pool[gWDT_pool_start];
		gWDT_pool_start = (gWDT_pool_start + 1) % WDT_POOL_SIZE;
		--gWDT_pool_count;
	}

	return retVal;
}


// This function returns a unsigned char (8-bit) with the number of unsigned long values
// in the entropy pool
uint8_t rand_available()
{
	return gWDT_pool_count;
}


// This interrupt service routine is called every time the LPTMR interrupt is triggered.
// With the default configuration that is approximately once every 16ms, producing
// approximately two 32-bit integer values every second.
//
// The pool is implemented as an 8 value circular buffer
static void isr_hardware_neutral( uint8_t val )
{
	gWDT_buffer[gWDT_buffer_position] = val;
	gWDT_buffer_position++; // every time the WDT interrupt is triggered

	if ( gWDT_buffer_position >= gWDT_buffer_SIZE )
	{
		gWDT_pool_end = (gWDT_pool_start + gWDT_pool_count) % WDT_POOL_SIZE;

		// The following code is an implementation of Jenkin's one at a time hash
		// This hash function has had preliminary testing to verify that it
		// produces reasonably uniform random results when using WDT jitter
		// on a variety of Arduino platforms
		for ( gWDT_loop_counter = 0; gWDT_loop_counter < gWDT_buffer_SIZE; ++gWDT_loop_counter )
		{
			gWDT_entropy_pool[gWDT_pool_end] += gWDT_buffer[gWDT_loop_counter];
			gWDT_entropy_pool[gWDT_pool_end] += (gWDT_entropy_pool[gWDT_pool_end] << 10);
			gWDT_entropy_pool[gWDT_pool_end] ^= (gWDT_entropy_pool[gWDT_pool_end] >> 6);
		}

		gWDT_entropy_pool[gWDT_pool_end] += (gWDT_entropy_pool[gWDT_pool_end] << 3);
		gWDT_entropy_pool[gWDT_pool_end] ^= (gWDT_entropy_pool[gWDT_pool_end] >> 11);
		gWDT_entropy_pool[gWDT_pool_end] += (gWDT_entropy_pool[gWDT_pool_end] << 15);
		gWDT_entropy_pool[gWDT_pool_end] = gWDT_entropy_pool[gWDT_pool_end];

		// Start collecting the next 32 bytes of Timer 1 counts
		gWDT_buffer_position = 0;

		// The entropy pool is full
		if (gWDT_pool_count == WDT_POOL_SIZE)
		{
			gWDT_pool_start = (gWDT_pool_start + 1) % WDT_POOL_SIZE;
		}
		// Add another unsigned long (32 bits) to the entropy pool
		else
		{
			++gWDT_pool_count;
		}
	}
}



// ----- Interrupts -----

void lptmr_isr()
{
	LPTMR0_CSR = 0b10000100;
	LPTMR0_CSR = 0b01000101;
	isr_hardware_neutral(SYST_CVR);
}


#elif defined(_sam_)


// ----- Includes -----

#include <stdlib.h>
#include "atomic.h"
#include "entropy.h"

#include "sam.h"



// ----- Defines -----

#define WDT_MAX_8INT  0xFF
#define WDT_MAX_16INT 0xFFFF
#define WDT_MAX_32INT 0xFFFFFFFF
#define WDT_POOL_SIZE 8

#define gWDT_buffer_SIZE 32



// ----- Variables -----

         uint8_t  gWDT_buffer[gWDT_buffer_SIZE];
         uint8_t  gWDT_buffer_position;
         uint8_t  gWDT_loop_counter;
volatile uint8_t  gWDT_pool_start;
volatile uint8_t  gWDT_pool_end;
volatile uint8_t  gWDT_pool_count;
volatile uint32_t gWDT_entropy_pool[WDT_POOL_SIZE];



// ----- Functions -----

// This function initializes the global variables needed to implement the circular entropy pool and
// the buffer that holds the raw Timer 1 values that are used to create the entropy pool.  It then
// Initializes tc0 (channel 1), to perform an interrupt every 2048 clock cycles.
// NOTE: Atmel is dumb and uses the range TC{0..5} to refer to TC{0,1}->TC_CHANNEL{0..2}
void rand_initialize()
{
	gWDT_buffer_position = 0;
	gWDT_pool_start = 0;
	gWDT_pool_end = 0;
	gWDT_pool_count = 0;

	// Enable clock for timer TC1 (Channel 4)
	PMC->PMC_PCER0 |= (1 << ID_TC2);

	// Setup Timer Counter to MCK/128, compare resets counter
	TC1->TC_CHANNEL[1].TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK4 | TC_CMR_CPCTRG;

	// Timer Count-down value
	// Number of cycles to count from CPU clock before calling interrupt
	TC1->TC_CHANNEL[1].TC_RC = TC_RC_RC(937); // Approx. ~1 kHz @ 120 MHz MCK

	// Enable Timer, Enable interrupt
	TC1->TC_CHANNEL[1].TC_IER = TC_IER_CPCS;
	TC1->TC_CHANNEL[1].TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG;

	// Enable TC1 interrupt
	NVIC_EnableIRQ( TC4_IRQn );

	// Set TC1 interrupt to a low priority
	NVIC_SetPriority( TC4_IRQn, Entropy_Priority_define );
}

// Disables interrupt, thus stopping CPU usage generating entropy
void rand_disable()
{
	TC1->TC_CHANNEL[1].TC_CCR = TC_CCR_CLKDIS;
	TC1->TC_CHANNEL[1].TC_IDR = 0xFF;
	NVIC_DisableIRQ( TC4_IRQn );
}

// This function returns a unsigned char (8-bit) with the number of unsigned long values
// in the entropy pool
uint8_t rand_available()
{
	return gWDT_pool_count;
}

// Pseudo-random value using clock
uint32_t rand_value32()
{
	uint32_t retVal = 0;
	uint8_t waiting;
	while ( gWDT_pool_count < 1 )
	{
		waiting += 1;
	}

	ATOMIC_BLOCK( ATOMIC_RESTORESTATE )
	{
		retVal = gWDT_entropy_pool[gWDT_pool_start];
		gWDT_pool_start = (gWDT_pool_start + 1) % WDT_POOL_SIZE;
		--gWDT_pool_count;
	}

	return retVal;
}

// This interrupt service routine is called every time the TC1 interrupt is triggered.
// With the default configuration that is approximately once every 16ms, producing
// approximately two 32-bit integer values every second.
//
// The pool is implemented as an 8 value circular buffer
static void isr_hardware_neutral( uint8_t val )
{
	gWDT_buffer[gWDT_buffer_position] = val;
	gWDT_buffer_position++; // every time the WDT interrupt is triggered

	if ( gWDT_buffer_position >= gWDT_buffer_SIZE )
	{
		gWDT_pool_end = (gWDT_pool_start + gWDT_pool_count) % WDT_POOL_SIZE;

		// The following code is an implementation of Jenkin's one at a time hash
		// This hash function has had preliminary testing to verify that it
		// produces reasonably uniform random results when using WDT jitter
		// on a variety of Arduino platforms
		for ( gWDT_loop_counter = 0; gWDT_loop_counter < gWDT_buffer_SIZE; ++gWDT_loop_counter )
		{
			gWDT_entropy_pool[gWDT_pool_end] += gWDT_buffer[gWDT_loop_counter];
			gWDT_entropy_pool[gWDT_pool_end] += (gWDT_entropy_pool[gWDT_pool_end] << 10);
			gWDT_entropy_pool[gWDT_pool_end] ^= (gWDT_entropy_pool[gWDT_pool_end] >> 6);
		}

		gWDT_entropy_pool[gWDT_pool_end] += (gWDT_entropy_pool[gWDT_pool_end] << 3);
		gWDT_entropy_pool[gWDT_pool_end] ^= (gWDT_entropy_pool[gWDT_pool_end] >> 11);
		gWDT_entropy_pool[gWDT_pool_end] += (gWDT_entropy_pool[gWDT_pool_end] << 15);
		gWDT_entropy_pool[gWDT_pool_end] = gWDT_entropy_pool[gWDT_pool_end];

		// Start collecting the next 32 bytes of Timer 1 counts
		gWDT_buffer_position = 0;

		// The entropy pool is full
		if (gWDT_pool_count == WDT_POOL_SIZE)
		{
			gWDT_pool_start = (gWDT_pool_start + 1) % WDT_POOL_SIZE;
		}
		// Add another unsigned long (32 bits) to the entropy pool
		else
		{
			++gWDT_pool_count;
		}
	}
}

void TC4_Handler()
{
	uint32_t status = TC1->TC_CHANNEL[1].TC_SR;
	if ( status & TC_SR_CPCS )
	{
		// Use the current state of systick for seeding
		isr_hardware_neutral(SysTick->VAL & SysTick_VAL_CURRENT_Msk);
	}
}



#elif defined(_nrf_)


// ----- Includes -----

#include <stdlib.h>
#include "entropy.h"



// ----- Functions -----

void rand_initialize()
{
	// TODO (HaaTa)
}

void rand_disable()
{
	// TODO (HaaTa)
}

uint8_t rand_available()
{
	// TODO (HaaTa)
	return 1;
}

// Pseudo-random value using clock
uint32_t rand_value32()
{
	// TODO (HaaTa)
	return 0;
}


#else
#error "Unknown build target for Lib/entropy"
#endif

