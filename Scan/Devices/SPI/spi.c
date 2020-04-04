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

// Project Includes
#include <Lib/gpio.h>
#include <print.h>
#include <kll_defs.h>

// ASF Includes
#include <sam/drivers/pmc/pmc.h>
#include <common/services/clock/sysclk.h>
#include <sam/drivers/spi/spi.h>

// Local Includes
#include "spi.h"



// ----- Defines -----

// Size of transaction buffer (stores pointers)
#define TransactionBuffer_Size 10



// ----- Structs -----

typedef struct TransactionBuffer {
	volatile SPI_Transaction *buf[TransactionBuffer_Size];
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
static volatile SPI_Transaction *TransactionBuffer_head()
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
		pdc_disable_transfer(pdc, PERIPH_PTCR_RXTDIS | PERIPH_PTCR_TXTDIS);
		NVIC_DisableIRQ(SPI_IRQn);

		return 0;
	}

	// Read the Rx buffer to clear the buffer just in case
	spi_get(SPI);

	// Get next transaction
	volatile SPI_Transaction *transaction = TransactionBuffer_head();

	// Load PDC
	pdc_rx_init(pdc, (pdc_packet_t*)&(transaction->rx_buffer), NULL);
	pdc_tx_init(pdc, (pdc_packet_t*)&(transaction->tx_buffer), NULL);
	transaction->status = SPI_Transaction_Status_Running;

	// Start PDC
	spi_enable_interrupt(SPI, SPI_IER_ENDRX | SPI_IER_ENDTX);
	NVIC_EnableIRQ(SPI_IRQn);
	pdc_enable_transfer(pdc, PERIPH_PTCR_RXTEN | PERIPH_PTCR_TXTEN);

	return 1;
}


// Increments TransactionBuffer
// Returns 1 if successful, 0 if buffer full and cannot increment
uint8_t spi_add_transaction(volatile SPI_Transaction *transaction)
{
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
	Transaction_buffer.buf[tail] = transaction;
	transaction->status = SPI_Transaction_Status_Queued;

	// Set tail
	Transaction_buffer.tail = new_tail;

	// Enable PDC if buffer previously empty, starting the transaction
	if (empty)
	{
		Pdc *pdc = spi_get_pdc_base(SPI);
		queue_next_transaction(pdc);
	}

	return 1;
}


// SPI Interrupt Handler
void SPI_Handler()
{
	uint32_t status = spi_read_status(SPI);
	Pdc *pdc = spi_get_pdc_base(SPI);

	// Check for status
	if (status & SPI_SR_ENDRX || status & SPI_SR_ENDTX)
	{
		// Disable Rx interrupt, re-enable when needed
		if (status & SPI_SR_ENDRX)
		{
			spi_disable_interrupt(SPI, SPI_IER_ENDRX);
		}

		// Disable Tx interrupt, re-enable when needed
		if (status & SPI_SR_ENDTX)
		{
			spi_disable_interrupt(SPI, SPI_IER_ENDTX);
		}

		// Only something to do if both Rx and Tx are finished
		if (pdc_read_rx_counter(pdc) == 0 && pdc_read_tx_counter(pdc) == 0)
		{
			// First set transaction as complete
			TransactionBuffer_head()->status = SPI_Transaction_Status_Finished;

			// Read the Rx buffer in case an extra byte was received
			spi_get(SPI);

			// Next pop head transaction
			// No need for a callback as the caller has everything they need and polls it periodically
			TransactionBuffer_increment_head();

			// Attempt to queue next transaction
			// If none available, pdc is disabled
			queue_next_transaction(pdc);
		}
	}
}


// SPI Setup
void spi_setup()
{
	// Reset transaction buffer
	Transaction_buffer.head = 0;
	Transaction_buffer.tail = 0;

	// Get pointer to SPI master PDC register base
	Pdc *pdc = spi_get_pdc_base(SPI);

	// Enable pins for SPI
	GPIO_ConfigPin miso = periph_io(A, 12, A);
	GPIO_ConfigPin mosi = periph_io(A, 13, A);
	GPIO_ConfigPin spck = periph_io(A, 14, A);
	PIO_Setup(miso);
	PIO_Setup(mosi);
	PIO_Setup(spck);

	// Reset SPI settings
	spi_disable(SPI);
	spi_enable_clock(SPI);
	spi_reset(SPI);
	spi_set_writeprotect(SPI, 0); // Disable write-protect
	spi_set_master_mode(SPI);
	spi_set_lastxfer(SPI);
	spi_set_variable_peripheral_select(SPI);
	spi_disable_mode_fault_detect(SPI);
	spi_disable_loopback(SPI);
	spi_disable_peripheral_select_decode(SPI);

	// Enable SPI
	spi_enable(SPI);

	// Make sure PDC is disabled until we're ready to use it
	pdc_disable_transfer(pdc, PERIPH_PTCR_RXTDIS | PERIPH_PTCR_TXTDIS);

	// Set SPI interrupt priority
	NVIC_SetPriority(SPI_IRQn, SPI_Priority_define);
}

// Write all channel settings in one shot using a packet structure
void spi_cs_setup(uint8_t cs, SPI_Channel settings)
{
	SPI->SPI_CSR[cs] = *((uint32_t*)&settings);
}

