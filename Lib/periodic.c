/* Copyright (C) 2017-2018 by Jacob Alexander
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

#include "mcu_compat.h"

#if defined(_kinetis_)
#include "kinetis.h"

#elif defined(_sam_)
#include "sam.h"

#elif defined(_nrf_)
#include "nrf5.h"

#elif defined(_host_)
#include <stdint.h>

#endif



// ----- Variables -----

static void (*periodic_func)(void);



// ----- Functions -----

#if defined(_kinetis_k_)
// Must set function pointer first!!
void Periodic_init( uint32_t cycles )
{
	// Setup PIT (Programmable Interrupt Timer)
	SIM_SCGC6 |= SIM_SCGC6_PIT;;
	PIT_TCTRL0 = 0x00; // Make sure timer is disabled first
	PIT_MCR = 0x00; // Enable module, do not freeze timers in debug mode

	// Timer Count-down value
	// Number of cycles to count from CPU clock before calling interrupt
	PIT_LDVAL0 = cycles;

	// Enable Timer, Enable interrupt
	PIT_TCTRL0 = PIT_TCTRL_TIE | PIT_TCTRL_TEN;

	// Enable PIT Ch0 interrupt
	NVIC_ENABLE_IRQ( IRQ_PIT_CH0 );

	// Set PIT0 interrupt to a low priority
	NVIC_SET_PRIORITY( IRQ_PIT_CH0, 200 );
}

void Periodic_enable()
{
	// Used to re-enable IRQ
	NVIC_ENABLE_IRQ( IRQ_PIT_CH0 );
}

void Periodic_disable()
{
	// Used to disable IRQ
	NVIC_DISABLE_IRQ( IRQ_PIT_CH0 );
}

void Periodic_function( void *func )
{
	// Set function pointer
	periodic_func = func;
}

uint32_t Periodic_cycles()
{
	return PIT_LDVAL0;
}

void pit0_isr()
{
	// Call specified function
	(*periodic_func)();

	// Clear the interrupt
	PIT_TFLG0 = PIT_TFLG_TIF;
}


#elif defined(_sam_)
void Periodic_init( uint32_t cycles )
{
	// Setup TC (Timer Counter)
	// SAM TODO: which clock should be used?
	TC0->TC_BMR |= TC_BMR_TC2XC2S_Msk;
	TC0->TC_CHANNEL[0].TC_CMR = TC_CMR_TCCLKS_Msk;

	// Timer Count-down value
	// Number of cycles to count from CPU clock before calling interrupt
	TC0->TC_CHANNEL[0].TC_RA = TC_RA_RA(cycles);

	// Enable Timer, Enable interrupt
	TC0->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKEN;
	TC0->TC_CHANNEL[0].TC_IER = TC_IER_CPAS;

	// SAM TODO: Handle interrupts
}

void Periodic_function( void *func )
{
	// Set function pointer
	periodic_func = func;
}

void Periodic_enable()
{
	//SAM TODO
}

void Periodic_disable()
{
	//SAM TODO
}

uint32_t Periodic_cycles()
{
	//SAM TODO
	return 0;
}


#elif defined(_nrf_)
void Periodic_init( uint32_t cycles )
{
	// NRF5 TODO
}

void Periodic_function( void *func )
{
	// Set function pointer
	periodic_func = func;
}

void Periodic_enable()
{
	// NRF5 TODO
}

void Periodic_disable()
{
	// NRF5 TODO
}

uint32_t Periodic_cycles()
{
	// NRF5 TODO
	return 0;
}


#elif defined(_host_)
void Periodic_init( uint32_t cycles )
{
}

void Periodic_function( void *func )
{
	// Set function pointer
	periodic_func = func;
}

void Periodic_enable()
{
}

void Periodic_disable()
{
}

uint32_t Periodic_cycles()
{
	return 0;
}
#endif

