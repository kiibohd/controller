/*
 * Copyright (C) 2019 Jacob Alexander
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files ( the "Software" ), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// ----- Includes ----

// Compiler Includes
#include <Lib/ScanLib.h>

// ASF Includes
#include <common/services/clock/sysclk.h>
#include <sam/drivers/dacc/dacc.h>
#include <sam/drivers/tc/tc.h>

// Project Includes
#include <Lib/gpio.h>
#include <print.h>
#include <kll_defs.h>

// Local Includes
#include "dac.h"



// ----- Defines -----

// Size of transaction buffer (stores pointers)
#define TransactionBuffer_Size 10



// ----- Structs -----

typedef struct TransactionBuffer {
	DAC_Sample *buf[TransactionBuffer_Size];
	uint8_t head;
	uint8_t tail;
} TransactionBuffer;



// ----- Variables -----

static volatile TransactionBuffer Transaction_buffer;



// ----- Functions -----

// Calculates the current size of the Transaction buffer
static uint8_t TransactionBuffer_current_size()
{
	// Wrap-around (tail less than head)
	if ( Transaction_buffer.tail < Transaction_buffer.head )
	{
		return TransactionBuffer_Size - Transaction_buffer.head + Transaction_buffer.tail + 1;
	}

	return Transaction_buffer.tail - Transaction_buffer.head;
}


// Retrieves the head of the Transaction buffer
static volatile DAC_Sample *TransactionBuffer_head()
{
	return Transaction_buffer.buf[Transaction_buffer.head];
}


// Check if valid head
static uint8_t TransactionBuffer_valid_head()
{
	// Buffer is empty, cannot increment head
	if (TransactionBuffer_current_size() == 0)
	{
		return 0;
	}

	// Make sure we don't pass tail pointer
	if (Transaction_buffer.head == Transaction_buffer.tail)
	{
		return 0;
	}

	uint16_t new_head = Transaction_buffer.head + 1;

	// Wrap-around case
	if (new_head == TransactionBuffer_Size)
	{
		// Make sure we don't pass tail pointer
		if (Transaction_buffer.tail == TransactionBuffer_Size)
		{
			return 0;
		}
	}

	return 1;
}


// Increments TransactionBuffer head pointer
// Returns 1 if successful, 0 if buffer empty and cannot increment
static uint8_t TransactionBuffer_increment_head()
{
	// Check if a valid head pointer first
	if (!TransactionBuffer_valid_head())
	{
		return 0;
	}

	uint16_t new_head = Transaction_buffer.head + 1;

	// Wrap-around case
	if (new_head == TransactionBuffer_Size)
	{
		new_head = 0;
	}

	// Don't set head until we are sure we're ready to commit
	Transaction_buffer.head = new_head;

	return 1;
}


// Queues next transmission
// Returns 1 if successfully queued, 0 if there was nothing to queue
static uint8_t queue_next_transaction(Pdc *pdc)
{
	// Make sure there are transactions
	if (TransactionBuffer_current_size() == 0)
	{
		// PDC isn't needed, disable
		pdc_disable_transfer(pdc, PERIPH_PTCR_TXTDIS);
		NVIC_DisableIRQ(DACC_IRQn);

		return 0;
	}

	// Get next transaction
	volatile DAC_Sample *sample = TransactionBuffer_head();

	// Load PDC
	pdc_tx_init(pdc, (pdc_packet_t*)&sample->tx_buffer, NULL);
	sample->status = DAC_Sample_Status_Started;

	// Start PDC
	dacc_enable_interrupt(DACC, DACC_ISR_ENDTX);
	NVIC_EnableIRQ(DACC_IRQn);
	pdc_enable_transfer(pdc, PERIPH_PTCR_TXTEN);

	return 1;
}


// Sets the current DAC sample
// Will disable the pdc of a currently running sample (even if it's the same incoming sample)
// If any samples are queued, they are also cleared
// Returns
//  0: Could not set sample
//  1: Sample was set
uint8_t dac_set_sample(volatile DAC_Sample *sample)
{
	Pdc *pdc = dacc_get_pdc_base(DACC);

	// First disable the pdc and interrupt
	pdc_disable_transfer(pdc, PERIPH_PTCR_TXTDIS);
	NVIC_DisableIRQ(DACC_IRQn);

	// Next mark the currently running samples as errored (if one was running)
	while (TransactionBuffer_current_size() > 0)
	{
		TransactionBuffer_head()->status = DAC_Sample_Status_Stopped;
		TransactionBuffer_increment_head();
	}

	// Now that the samples have been cleared, add the sample
	return dac_next_sample(sample);
}


// Appends the next sample
// If no sample is running, start sample.
// If a sample is already running queue a single sample
// If 1 sample is already queue, do not add
// Returns
//  0: Could not add sample
//  1: Sample queued
//  2: Already queued
uint8_t dac_next_sample(volatile DAC_Sample *sample)
{
	// Check if sample is already in the queue
	uint16_t pos = 0;
	while (pos < TransactionBuffer_current_size())
	{
		uint16_t pos_adj = pos > TransactionBuffer_Size ? pos - TransactionBuffer_current_size() : pos;
		if (sample == Transaction_buffer.buf[pos_adj])
		{
			return 2;
		}
		pos++;
	}

	// Buffer is full, cannot increment tail
	if (TransactionBuffer_current_size() >= TransactionBuffer_Size)
	{
		return 0;
	}

	uint16_t tail = Transaction_buffer.tail;
	uint16_t new_tail = tail + 1;

	// Make sure we don't collide with head pointer
	if (new_tail == Transaction_buffer.head)
	{
		return 0;
	}

	// Wrap-around case
	if (new_tail >= TransactionBuffer_Size)
	{
		// Make sure we don't collide with head pointer
		if (Transaction_buffer.head == 0)
		{
			return 0;
		}

		new_tail = 0;
	}

	// Check if buffer is currently empty
	uint8_t empty = TransactionBuffer_current_size() == 0 ? 1 : 0;

	// Set transaction
	Transaction_buffer.buf[tail] = (DAC_Sample*)sample;
	sample->status = DAC_Sample_Status_Queued;

	// Set tail
	Transaction_buffer.tail = new_tail;

	// Enable PDC if buffer previously empty, starting the transaction
	if (empty)
	{
		Pdc *pdc = dacc_get_pdc_base(DACC);
		queue_next_transaction(pdc);
	}

	return 1;
}

void dac_setup()
{
	// Setup timer for dac
	// Using Timer Module 0, Channel 3 (TC2)
	PMC->PMC_PCER0 |= (1 << ID_TC2);

	// DAC Sample Frequency
	// Must be no faster than 0.5 us (2 MHz) - Settling Time
	// Must be no slower than 24 us (42 kHz) - 8-bit refresh time
	// Common Audio Forms:
	// 44.1  kHz (CD)
	// 48    kHz (DVD-Audio)
	// 88.2  kHz (2x 44.1 kHz)
	// 96    kHz (2x 48 kHz)
	// 176.4 kHz (4x 44.1 kHz)
	// 192   kHz (4x 48 kHz)
	//
	// TODO (HaaTa): Base the time based on the input sample
	uint32_t sample_freq = 44100;
	uint32_t sys_clk = sysclk_get_cpu_hz();
	uint32_t tc_divisor = 0;
	uint32_t tc_field_divisor = 0;
	tc_find_mck_divisor(
		sample_freq,
		sys_clk,
		&tc_divisor,       // Out
		&tc_field_divisor, // Out
		sys_clk
	);
	uint32_t rc = (sys_clk / tc_divisor / sample_freq) - 1;

	// Setup Timer Counter to MCK/32
	TC0->TC_CHANNEL[2].TC_CMR = (
		tc_field_divisor |  // Calculated timer flags
		TC_CMR_WAVE |       // Waveform generation mode
		TC_CMR_WAVSEL_UP_RC // Up count, clear on match RC
	);

	// Timer Count-down value
	// Number of cycles to count from CPU clock before calling interrupt
	TC0->TC_CHANNEL[2].TC_RC = TC_RA_RA(rc);

	// Enable Timer
	TC0->TC_CHANNEL[2].TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG;

	// Allow DACC to control PB13
	GPIO_ConfigPin dac1 = periph_io(B, 13, D);
	PIO_Setup(dac1);

	// Enable DACC Clock
	sysclk_enable_peripheral_clock(ID_DACC);

	// Reset DACC
	dacc_reset(DACC);

	// Half word transfer mode
	dacc_set_transfer_mode(DACC, 0);

	// Set DACC Timing
	// Startup Time (peripheral clock periods)
	// XXX (HaaTa): SAM4S DAC only supports up to a 50 MHz peripheral clock
	//              But, going to ignore that and just live with it at 60 MHz (120 MHz / 2)
	//
	// From sleep to normal, the maximum startup time is 40 us (worst case)
	// Min: 20
	// Typ: 30
	// Max: 40
	//
	// 1 / 60 MHz * 10^6 * 1152 (18) = 19.200 us
	// 1 / 60 MHz * 10^6 * 1216 (19) = 20.267 us
	// 1 / 60 MHz * 10^6 * 1280 (20) = 21.333 us
	//
	// 1 / 60 MHz * 10^6 * 1792 (28) = 29.867 us
	// 1 / 60 MHz * 10^6 * 1856 (29) = 30.933 us
	// 1 / 60 MHz * 10^6 * 1920 (30) = 32.000 us
	//
	// 1 / 60 MHz * 10^6 * 2368 (37) = 39.467 us
	// 1 / 60 MHz * 10^6 * 2432 (38) = 40.533 us <---
	// 1 / 60 MHz * 10^6 * 2496 (39) = 41.600 us
	//
	// 0  - 0 periods
	// 1  - 8 periods
	// 2  - 16 periods
	// 3  - 24 periods
	// 4  - 64 periods
	// 5  - 80 periods
	// 6  - 96 periods
	// 7  - 112 periods
	// 8  - 512 periods
	// 9  - 576 periods
	// 10 - 640 periods
	// 11 - 704 periods
	// 12 - 768 periods
	// 13 - 832 periods
	// 14 - 896 periods
	// 15 - 960 periods
	// 16 - 1024 periods
	// 17 - 1088 periods
	// 18 - 1152 periods
	// 19 - 1216 periods
	// 20 - 1280 periods
	// 21 - 1344 periods
	// 22 - 1408 periods
	// 23 - 1472 periods
	// 24 - 1536 periods
	// 25 - 1600 periods
	// 26 - 1664 periods
	// 27 - 1728 periods
	// 28 - 1792 periods
	// 29 - 1856 periods
	// 30 - 1920 periods
	// 31 - 1984 periods
	// 32 - 2048 periods
	// 33 - 2112 periods
	// 34 - 2176 periods
	// 35 - 2240 periods
	// 36 - 2304 periods
	// 37 - 2368 periods
	// 38 - 2432 periods
	// 39 - 2496 periods
	// 40 - 2560 periods
	// 41 - 2624 periods
	// 42 - 2688 periods
	// 43 - 2752 periods
	// 44 - 2816 periods
	// 45 - 2880 periods
	// 46 - 2944 periods
	// 47 - 3008 periods
	// 48 - 3072 periods
	// 49 - 3136 periods
	// 50 - 3200 periods
	// 51 - 3264 periods
	// 52 - 3328 periods
	// 53 - 3392 periods
	// 54 - 3456 periods
	// 55 - 3520 periods
	// 56 - 3584 periods
	// 57 - 3648 periods
	// 58 - 3712 periods
	// 59 - 3776 periods
	// 60 - 3840 periods
	// 61 - 3904 periods
	// 62 - 3968 periods
	// 63 - 4032 periods
	dacc_set_timing(
		DACC,
		1,  // Must be set to 1 (refresh not available on SAM4S)
		0,  // Normal (0) / Max Speed (1)
		38  // Startup time
	);   // Normal Mode, 1024 clock start up

	// Free running mode
	dacc_disable_trigger(DACC);

	// Set trigger
	// Only works with timer channels 0..2 (1->3)
	dacc_set_trigger(
		DACC,
		3 // TRGSEL3 - Timer Channel 3 (TC0,2)
	);

	// Set analog current
	dacc_set_analog_control(
		DACC,
		(
			DACC_ACR_IBCTLCH0(0x03) |   // 10.7V/us slew
			DACC_ACR_IBCTLDACCORE(0x03) // 0.89 mA bias current control
		)
	);

	// Select channel 0
	dacc_set_channel_selection(DACC, 0);

	// Enable DACC on Channel 0
	dacc_enable_channel(DACC, 0);

	// Make sure pdc is disable until we're ready to use it
	Pdc *pdc = dacc_get_pdc_base(DACC);
	pdc_disable_transfer(pdc, PERIPH_PTCR_TXTDIS);

	// Set DACC interrupts
	NVIC_SetPriority(DACC_IRQn, DAC_Priority_define);
}

// DAC Interrupt Handler
void DACC_Handler()
{
	uint32_t status = dacc_get_interrupt_status(DACC);
	Pdc *pdc = dacc_get_pdc_base(DACC);

	// Check end of tx for dac buffer
	if (status & DACC_ISR_ENDTX)
	{
		// Disable interrupt, re-enable when needed
		dacc_disable_interrupt(DACC, DACC_IER_ENDTX);

		// Check if the buffer has finished sending
		if (pdc_read_tx_counter(pdc) == 0)
		{
			// First set transaction as complete
			TransactionBuffer_head()->status = DAC_Sample_Status_Finished;

			// Next pop head transaction
			// No need for a callback as the caller has everything they need and polls it periodically
			TransactionBuffer_increment_head();

			// Attempt to queue next transaction
			// If none available, pdc is disabled
			queue_next_transaction(pdc);
		}
	}
}

