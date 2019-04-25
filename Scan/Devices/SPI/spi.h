/*
 * Copyright (C) 2019 Jacob Alexander
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
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

#pragma once

// ----- Includes ----

// ASF Includes
#include <sam/drivers/pdc/pdc.h>

// Compiler Includes
#include <Lib/ScanLib.h>



// ----- Defines -----

// ----- Enumerations -----

typedef enum SPI_Transaction_Status {
	SPI_Transaction_Status_None     = 0, // Not yet queued
	SPI_Transaction_Status_Queued   = 1, // Awaiting SPI bus
	SPI_Transaction_Status_Running  = 2, // Transaction in progress
	SPI_Transaction_Status_Finished = 3, // Transaction complete, buffers may be freed
	                                     // Waits for both RXBUFF (Rx Full) and TXBUFE (Tx Empty)
} SPI_Transaction_Status;

typedef enum SPI_Size {
	SPI_Size_8_BIT  = 0,
	SPI_Size_9_BIT  = 1,
	SPI_Size_10_BIT = 2,
	SPI_Size_11_BIT = 3,
	SPI_Size_12_BIT = 4,
	SPI_Size_13_BIT = 5,
	SPI_Size_14_BIT = 6,
	SPI_Size_15_BIT = 7,
	SPI_Size_16_BIT = 8,
} SPI_Size;



// ----- Structs -----

// TODO Validate structure
typedef struct SPI_Packet {
	uint8_t lastxfer:1;
	uint8_t unused1:7;
	uint8_t pcs:4;
	uint8_t unused2:4;
	uint16_t data;
} __attribute__((packed)) SPI_Packet;

typedef struct SPI_Transaction {
	SPI_Transaction_Status status;
	pdc_packet_t rx_buffer;
	pdc_packet_t tx_buffer;
} SPI_Transaction;

typedef struct SPI_Channel {
	uint8_t cpol:1;
	uint8_t ncpha:1;
	uint8_t csnaat:1;
	uint8_t csaat:1;
	SPI_Size size:4;
	uint8_t scbr;
	uint8_t dlybs;
	uint8_t dlybct;
} __attribute__((packed)) SPI_Channel;



// ----- Functions -----

void spi_setup();

// Sets up channel settings
// All settings must be specified
// See datasheet (33.8.2 SAM4S SPI Mode Register) for settings when using external decoder
void spi_cs_setup(uint8_t cs, SPI_Channel settings);

// Add transaction
// Only stores a pointer to the transaction information
// It is up to the caller to handle the memory of the:
//   - Transaction information
//   - Rx Buffer
//   - Tx Buffer
// If less data needs to be read than written, set read packet
// to the minimum number of reads required.
// (Initial reads may need to be ignored)
// Also applies to writes vs. reads
//
// Returns 1 if queued correctly, 0 if unable to queue
uint8_t spi_add_transaction(volatile SPI_Transaction *transaction);

