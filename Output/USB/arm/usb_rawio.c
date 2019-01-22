/* Copyright (C) 2016-2019 by Jacob Alexander
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

#include <Lib/mcu_compat.h>
#include <kll_defs.h>

#if enableRawIO_define == 1

#if defined(_kinetis_)

// ----- Includes -----

// Compiler Includes
#include <string.h> // For memcpy

// Project Includes
#include <Lib/OutputLib.h>
#include <print.h>

// Local Includes
#include "usb_dev.h"
#include "usb_rawio.h"



// ----- Defines -----

// Maximum number of transmit packets to queue so we don't starve other endpoints for memory
#define TX_PACKET_LIMIT 5



// ----- Functions -----

// Check for packets available from host
uint32_t usb_rawio_available()
{
	// Error if USB isn't configured
	if ( !usb_configuration )
		return 0;

	// Query number of bytes available from the endpoint
	return usb_rx_byte_count( RAWIO_RX_ENDPOINT );
}

// Retrieve packets from host
// Always returns RAWIO_RX_SIZE
int32_t usb_rawio_rx( void *buf, uint32_t timeout )
{
	usb_packet_t *rx_packet;
	Time start = Time_now();

	// Read
	while ( 1 )
	{
		// Error if USB isn't configured
		if ( !usb_configuration )
			return -1;

		// Retrieve packet
		rx_packet = usb_rx( RAWIO_RX_ENDPOINT );
		if ( rx_packet )
			break;

		// Check for timeout
		if ( Time_duration_ms( start ) > timeout || !timeout )
		{
			warn_msg("RAWIO Rx - Timeout, dropping packet.");
			return 0;
		}

		yield();
	}

	// Transfer packet from USB buffer to given buffer
	memcpy( buf, rx_packet->buf, RAWIO_RX_SIZE );
	usb_free( rx_packet );

	// Data sent in full packet chunks
	return RAWIO_RX_SIZE;
}

// Send packet to host
// XXX Only transfers RAWIO_TX_SIZE on each call (likely 64 bytes)
// Always returns RAWIO_TX_SIZE
int32_t usb_rawio_tx( const void *buf, uint32_t timeout )
{
	usb_packet_t *tx_packet;
	Time start = Time_now();

	while ( 1 )
	{
		// Error if USB isn't configured
		if ( !usb_configuration )
			return -1;

		// Make sure we haven't exceeded the outgoing packet limit
		if ( usb_tx_packet_count( RAWIO_TX_ENDPOINT ) < TX_PACKET_LIMIT )
		{
			// Allocate a packet buffer
			tx_packet = usb_malloc();
			if ( tx_packet )
				break;
		}

		// Check for timeout
		if ( Time_duration_ms( start ) > timeout || !timeout )
		{
			warn_msg("RAWIO Tx - Timeout, dropping packet.");
			return 0;
		}

		yield();
	}

	// Copy input buffer to usb packet buffer and assign length
	memcpy( tx_packet->buf, buf, RAWIO_TX_SIZE );
	tx_packet->len = RAWIO_TX_SIZE;

	// Send USB packet
	usb_tx( RAWIO_TX_ENDPOINT, tx_packet );

	return RAWIO_TX_SIZE;
}

#elif defined(_sam_)

// ----- Includes -----

// Compiler Includes
#include <string.h> // For memcpy

// ASF Includes
#include <udi_hid_generic.h>

// Project Includes
#include <Lib/OutputLib.h>
#include <print.h>

// Local Includes
#include "usb_rawio.h"



// ----- Defines -----

// Maximum number of receive packets to queue
#define RX_PACKET_LIMIT 5



// ----- Structs -----

typedef struct USBRxPacketBuffer {
	uint8_t data[RAWIO_RX_SIZE * RX_PACKET_LIMIT];
	uint16_t head;
	uint16_t tail;
} USBRxPacketBuffer;



// ----- Variables -----

USBRxPacketBuffer rxbuf;



// ----- Functions -----

static uint32_t usb_rx_packet_buffer_size()
{
	// Wrap-around (tail less than head)
	if ( rxbuf.tail < rxbuf.head )
	{
		return RAWIO_RX_SIZE * RX_PACKET_LIMIT - rxbuf.head + rxbuf.tail + RAWIO_RX_SIZE;
	}

	return rxbuf.tail - rxbuf.head;
}

static bool usb_rx_packet_buffer_valid_head()
{
	// Buffer is empty, cannot increment head
	if (usb_rx_packet_buffer_size() == 0)
	{
		return false;
	}

	// Make sure we don't pass tail pointer
	if (rxbuf.head == rxbuf.tail)
	{
		return false;
	}

	uint16_t new_head = rxbuf.head + RAWIO_RX_SIZE;

	// Wrap-around case
	if (new_head == RAWIO_RX_SIZE * RX_PACKET_LIMIT)
	{
		// Make sure we don't pass tail pointer
		if (rxbuf.tail == 0)
		{
			return false;
		}
	}

	return true;
}

// Copies a packet into the buffer (fixed size)
static bool usb_rx_packet_buffer_add( uint8_t *data )
{
	// Buffer is full, cannot increment tail
	if (usb_rx_packet_buffer_size() >= RAWIO_RX_SIZE * RX_PACKET_LIMIT)
	{
		return false;
	}

	uint16_t new_tail = rxbuf.tail + RAWIO_RX_SIZE;

	// Make sure we don't collide with head pointer
	if (new_tail == rxbuf.head)
	{
		return false;
	}

	// Wrap-around case
	if (new_tail >= RAWIO_RX_SIZE * RX_PACKET_LIMIT)
	{
		// Make sure we don't collide with head pointer
		if (rxbuf.head == 0)
		{
			return false;
		}

		new_tail = 0;
	}

	// Don't set tail until we are sure we've allocated the space
	rxbuf.tail = new_tail;

	// Copy the packet into the buffer
	memcpy(&rxbuf.data[rxbuf.tail], data, RAWIO_RX_SIZE);

	return true;
}

// Peak at the head packet
static uint8_t* usb_rx_packet_buffer_head()
{
	return &rxbuf.data[rxbuf.head];
}

// Pop the head packet
static bool usb_rx_packet_buffer_pop()
{
	// Check if a valid head pointer first
	if (!usb_rx_packet_buffer_valid_head())
	{
		return false;
	}

	uint16_t new_head = rxbuf.head + RAWIO_RX_SIZE;

	// Wrap-around case
	if (new_head == RAWIO_RX_SIZE * RX_PACKET_LIMIT)
	{
		new_head = 0;
	}

	// Don't set head until we are sure we're ready to commit
	rxbuf.head = new_head;

	return true;
}

// RawIO enabled, shouldn't be much to do here
bool usb_hid_raw_enable_callback()
{
	return true;
}

// RawIO disabled, shouldn't be much to do here
bool usb_hid_raw_disable_callback()
{
	return true;
}

// Callback when an Rx packet arrives
// Only puts packets into the buffer, no processing
void usb_hid_raw_report_out( uint8_t *report )
{
	// Queue up buffer
	usb_rx_packet_buffer_add( report );
}

// Check for packets available from host
uint32_t usb_rawio_available()
{
	return usb_rx_packet_buffer_size();
}

// Retrieve packets from host
// Always returns RAWIO_RX_SIZE
int32_t usb_rawio_rx( void *buf, uint32_t timeout )
{
	uint8_t *rx_packet;
	Time start = Time_now();

	// Read
	while ( 1 )
	{
		// Error if USB isn't configured
		if ( !usb_configuration )
		{
			return -1;
		}

		// Retrieve packet
		rx_packet = usb_rx_packet_buffer_head();

		// Make sure it's a valid packet
		if ( usb_rx_packet_buffer_size() > 0 )
		{
			break;
		}

		// Check for timeout
		if ( Time_duration_ms( start ) > timeout || !timeout )
		{
			warn_msg("RAWIO Rx - Timeout, dropping packet.");
			return 0;
		}

		yield();
	}

	// Transfer packet from USB buffer to given buffer
	memcpy( buf, rx_packet, RAWIO_RX_SIZE );
	usb_rx_packet_buffer_pop();

	// Data sent in full packet chunks
	return RAWIO_RX_SIZE;
}

// Send packet to host
// XXX Only transfers RAWIO_TX_SIZE on each call (likely 64 bytes)
// Always returns RAWIO_TX_SIZE
int32_t usb_rawio_tx( const void *buf, uint32_t timeout )
{
	uint8_t *tx_packet = (uint8_t*)buf;
	Time start = Time_now();

	while ( 1 )
	{
		// Error if USB isn't configured
		if ( !usb_configuration )
		{
			return -1;
		}

		// Atempt to send, if it fails, try again
		if ( udi_hid_generic_send_report_in( tx_packet ) )
		{
			break;
		}

		// Check for timeout
		if ( Time_duration_ms( start ) > timeout || !timeout )
		{
			warn_msg("RAWIO Tx - Timeout, dropping packet.");
			return 0;
		}

		yield();
	}

	return RAWIO_TX_SIZE;
}

#endif // _sam_

#endif // enableRawIO_define

