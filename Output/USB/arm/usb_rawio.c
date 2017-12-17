/* Copyright (C) 2016-2017 by Jacob Alexander
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

#include <kll_defs.h>
#if enableRawIO_define == 1

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

#endif

