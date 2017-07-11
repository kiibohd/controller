/* Copyright (C) 2017 by Jacob Alexander
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file.  If not, see <http://www.gnu.org/licenses/>.
 */

// ----- Includes -----

// Compiler Includes
#include <Lib/OutputLib.h>

// Project Includes
#include <cli.h>
#include <latency.h>
#include <led.h>
#include <output_com.h>
#include <print.h>

// KLL
#include <kll_defs.h>
#include <kll.h>

// Local Includes
#include "hidio_com.h"



// ----- Defines -----

#define HIDIO_Id_List_MaxSize 20
#define HIDIO_Max_ACK_Payload 70
#define HIDIO_Max_Payload 200
#define HIDIO_Max_Tx_Payload 200



// ----- Macros -----

// ----- Enumerations -----

typedef enum HIDIO_Info_1_Property {
	HIDIO_Info_1_Property__Major_Verion  = 0,
	HIDIO_Info_1_Property__Minor_Verion  = 1,
	HIDIO_Info_1_Property__Patch_Verion  = 2,
	HIDIO_Info_1_Property__OS_Type       = 3,
	HIDIO_Info_1_Property__OS_Version    = 4,
	HIDIO_Info_1_Property__Host_Software = 4,
} HIDIO_Info_1_Property;



// ----- Structs -----

typedef struct HIDIO_Id_Entry {
	uint32_t id;
	void *call_func;
	void *reply_func;
} HIDIO_Id_Entry;



// ----- Function Declarations -----

// ----- Variables -----

CLIDict_Def( hidioCLIDict, "HID-IO Module Commands" ) = {
	{ 0, 0, 0 } // Null entry for dictionary end
};

// Latency Resource
static uint8_t hidioLatencyResource;

// Id List
HIDIO_Id_Entry HIDIO_Id_List[ HIDIO_Id_List_MaxSize ];
uint32_t HIDIO_Id_List_Size;

// Packet information
uint16_t HIDIO_Packet_Size;

// Assembly Ring Buffer
uint8_t HIDIO_assembly_buf_data[HIDIO_Max_Payload + sizeof(HIDIO_Buffer_Entry)];
HIDIO_Buffer HIDIO_assembly_buf;

// ACK Send Buffer
uint8_t HIDIO_ack_send_data[HIDIO_Max_Tx_Payload + sizeof(HIDIO_Buffer_Entry)];
HIDIO_Buffer HIDIO_ack_send_buf;

// ACK Receive Buffer
uint8_t HIDIO_ack_buf_data[HIDIO_Max_ACK_Payload + sizeof(HIDIO_Buffer_Entry)];
HIDIO_Buffer_Entry *HIDIO_ack_buf;

// Packet Tx Buffer
uint8_t HIDIO_tx_buf_data[HIDIO_Max_Tx_Payload + sizeof(HIDIO_Packet)];
HIDIO_Buffer HIDIO_tx_buf;



// ----- Capabilities -----

// ----- Utility Functions -----

// Munch off the given length of the HIDIO packet buffer
// Does not modify head or tail pointers
// buf - Must pass an array of at least len in size
//       Sometimes an allocation is not necessary and the buffer memory is used "in-place"
//       If there is a wrap-around, the buffer is copied.
// buf_pos - Index position in the buffer
// len - Length of the desired buffer
//
// A pointer the data is returned, which may or may not be the provided buffer
uint8_t *HIDIO_buffer_munch( HIDIO_Buffer *buffer, uint8_t *buf, uint16_t buf_pos, uint16_t len )
{
	// Determine if buffer is contiguous for the length
	if ( buf_pos + len < buffer->len )
	{
		// We can just set the buffer directly
		return &(buffer->data[ buf_pos ]);
	}

	// If just a single byte, just return the first position (wrap-around)
	if ( len == 1 )
	{
		return &(buffer->data[0]);
	}

	// Copy into the buffer
	uint16_t cur_len = buffer->len - buf_pos;
	memcpy( buf, &(buffer->data[ buf_pos ]), cur_len );
	memcpy( &buf[ cur_len ], buffer->data, len - cur_len );

	return buf;
}

// Push byte to ring buffer
// XXX (HaaTa): Does not check if full, that needs to be validated ahead of time
void HIDIO_buffer_push_byte( HIDIO_Buffer *buffer, uint8_t byte )
{
	// Check if wrap-around case
	if ( buffer->tail == buffer->len )
	{
		buffer->tail = 0;
	}

	// Add byte at tail
	buffer->data[ buffer->tail++ ] = byte;
}

// Modify buffer in place
// XXX (HaaTa): Does not check tail bounds, responsibility of caller to verify bonuds are correct
void HIDIO_modify_buffer( HIDIO_Buffer *buffer, uint16_t start, uint8_t *data, uint16_t len )
{
	for ( uint16_t c = 0; c < len; c++ )
	{
		uint16_t pos = c + start;
		// Wrap-around case
		if ( pos >= buffer->len )
		{
			pos -= buffer->len;
		}

		buffer->data[pos] = data[c];
	}
}

// Pop bytes from ring buffer, just drops them, nothing returned
// XXX (HaaTa): Does not check if empty, that needs to be validated ahead of time
uint8_t HIDIO_buffer_pop_bytes( HIDIO_Buffer *buffer, uint16_t len )
{
	// Check if len is longer than total buffer
	if ( len > buffer->len )
	{
		erro_msg("Requested HIDIO buffer pop larger than entire buffer: ");
		printInt16( len );
		print(":");
		printInt16( buffer->len );
		print(NL);
		return 0;
	}

	// Adjust head position, check for wrap-around
	if ( len + buffer->head > buffer->len )
	{
		buffer->head = len + buffer->head - buffer->len;
	}
	else
	{
		buffer->head += len;
	}

	return 1;
}

// Determines max payload with given id width
uint16_t HIDIO_max_payload( uint8_t id_width )
{
	return HIDIO_Packet_Size - sizeof(HIDIO_Packet) - id_width;
}

// Determines available space in ring buffer
uint16_t HIDIO_buffer_free_bytes( HIDIO_Buffer *buffer )
{
	if ( buffer->head <= buffer->tail )
	{
		return buffer->len - (buffer->tail - buffer->head);
	}
	else
	{
		return buffer->head - buffer->tail;
	}
}

// Determines buffer position (index) after given length
// To munch the buffer, we need the starting buffer position.
// This function gives the position after a given length, handles wrap-around for you.
uint16_t HIDIO_buffer_position( HIDIO_Buffer *buffer, uint16_t cur_pos, uint16_t distance )
{
	if ( cur_pos + distance >= buffer->len )
	{
		return buffer->len - (cur_pos + distance);
	}
	else
	{
		return cur_pos + distance;
	}
}

// Get id from data stream
// May be either 16 bits or 32 bits wide
uint32_t HIDIO_buffer_id( HIDIO_Packet *packet )
{
	uint32_t id = 0;

	// 16 bit Id
	if ( packet->id_width == 0 )
	{
		HIDIO_Packet16 *pkt = (HIDIO_Packet16*)packet;
		id = pkt->id;
	}
	// 32 bit Id
	else
	{
		HIDIO_Packet32 *pkt = (HIDIO_Packet32*)packet;
		id = pkt->id;
	}

	return id;
}

// Get data payload start of packet
uint8_t *HIDIO_payload_start( HIDIO_Packet *packet )
{
	uint8_t *data = 0;

	// 16 bit Id
	if ( packet->id_width == 0 )
	{
		HIDIO_Packet16 *pkt = (HIDIO_Packet16*)packet;
		data = pkt->data;
	}
	// 32 bit Id
	else
	{
		HIDIO_Packet32 *pkt = (HIDIO_Packet32*)packet;
		data = pkt->data;
	}

	return data;
}

// Return width in bytes
uint8_t HIDIO_buffer_id_width( uint8_t id_width )
{
	// If 1, 32bit; 0, 16bit
	return id_width ? 4 : 2;
}

// Calculate Id width in bytes
uint8_t HIDIO_id_width( uint32_t id )
{
	// Calculate width of id
	uint8_t width = 0;
	if ( id <= 0xFFFF )
	{
		width = 2;
	}
	else
	{
		width = 4;
	}

	return width;
}

// Generate packet
// This function can be called multiple times to generate a full packet
// Continue to call unitil the value returned equals payload_len
// If 0 is returned, there has been an error, and the packet is aborted.
// To start a new packet, start at pos = 0
uint16_t HIDIO_buffer_generate_packet(
	HIDIO_Buffer *buf,
	uint16_t pos,
	uint16_t payload_len,
	uint8_t *data,
	uint16_t data_len,
	HIDIO_Packet_Type type,
	uint32_t id
)
{
	/*
	print("head: ");
	printInt16( buf->head );
	print(" tail: ");
	printInt16( buf->tail );
	print(" bytes_left: ");
	printInt16( HIDIO_buffer_free_bytes( buf ) );
	print(" request: ");
	printInt16( data_len );
	print(NL);
	*/

	// Determine payload max
	uint8_t width = HIDIO_id_width( id );
	uint16_t max_payload = HIDIO_max_payload( width );

	// Number of packets and current packet number
	// Ignore calculation for zero-length packet
	uint16_t packet_count = 1;
	uint16_t cur_packet = 0;
	if ( payload_len != 0 )
	{
		packet_count = payload_len / max_payload;
		packet_count += payload_len % max_payload != 0 ? 1 : 0;

		cur_packet = pos / max_payload;
		cur_packet += pos % max_payload != 0 ? 1 : 0;
	}

	// Determine if there is enough room in tx buffer
	uint16_t requested = payload_len - pos + sizeof(HIDIO_Packet) * ( packet_count - cur_packet );
	if ( requested > HIDIO_buffer_free_bytes( buf ) )
	{
		erro_msg("Not enough bytes in HIDIO buffer: ");
		printInt16( HIDIO_buffer_free_bytes( buf ) );
		print(" bytes left, ");
		printInt16( buf->len );
		print(" bytes total ");
		printInt16( requested );
		print(" bytes requested");
		print(NL);
		return payload_len;
	}

	// Generate a payload of given length, repeating the payload value
	// Also proceed if building a zero-length packet
	for ( ; pos < payload_len || payload_len == 0; pos++ )
	{
		// Check if we need to start a new packet
		uint16_t bytes_left = pos % max_payload;
		if ( bytes_left == 0 )
		{
			// Reset bytes left
			bytes_left = max_payload;

			// Start of a new packet, signal previous one is ready
			// Do not increment for the very first packet in the sequence
			if ( pos != 0 )
			{
				buf->packets_ready++;
			}

			// Increment current packet counter
			cur_packet++;

			// Determine length of this new packet
			// Start with total bytes left, plus id width
			uint16_t packet_len = bytes_left + width;

			// If larger than payload_len, reduce
			uint16_t cur_payload_len = payload_len - pos + width;
			if ( packet_len > cur_payload_len )
			{
				packet_len = cur_payload_len;
			}

			// Get info for packet header
			uint8_t p_type = cur_packet == 1 ? type : HIDIO_Packet_Type__Continued;
			uint8_t p_cont = cur_packet == packet_count ? 0 : 1;

			// Start new packet and copy into buffer
			// Use 16 bit Ids if possible (more efficient)
			if ( id <= 0xFFFF )
			{
				HIDIO_Packet16 packet = {
					.type = p_type,
					.cont = p_cont,
					.id = (uint16_t)id,
					.id_width = 0,
					.upper_len = (packet_len >> 8) & 0x3,
					.len = (packet_len & 0xFF),
				};

				// Copy packet header data to buffer
				for ( uint8_t byte = 0; byte < sizeof(HIDIO_Packet16); byte++ )
				{
					HIDIO_buffer_push_byte( buf, ((uint8_t*)&packet)[ byte ] );
				}

				// There's always enough room for header
				bytes_left -= sizeof(HIDIO_Packet16);
			}
			else
			{
				HIDIO_Packet32 packet = {
					.type = p_type,
					.cont = p_cont,
					.id = id,
					.id_width = 1,
					.upper_len = (packet_len >> 8) & 0x3,
					.len = (packet_len & 0xFF),
				};

				// Copy packet header data to buffer
				for ( uint8_t byte = 0; byte < sizeof(HIDIO_Packet32); byte++ )
				{
					HIDIO_buffer_push_byte( buf, ((uint8_t*)&packet)[ byte ] );
				}

				// There's always enough room for header
				bytes_left -= sizeof(HIDIO_Packet32);
			}
		}

		// Push payload
		// Monitor:
		// - bytes_left (payload left in packet)
		// - data_len (input data buffer length)
		// - Current overall payload position
		uint16_t byte = 0;
		for ( ; byte < data_len && bytes_left > 0; byte++, pos++, bytes_left-- )
		{
			HIDIO_buffer_push_byte( buf, data[ byte ] );
		}

		// If we are out of data in the data buffer, return the current position
		if ( byte >= data_len )
		{
			break;
		}
	}

	// Finished packet
	if ( pos == payload_len )
	{
		buf->packets_ready++;
	}

	// Current current (or final) position in the payload
	return pos;
}

void HIDIO_nopayload_ack( uint32_t id )
{
	// Generate a no payload ACK
	HIDIO_buffer_generate_packet(
		&HIDIO_ack_send_buf,
		0,
		0,
		0,
		0,
		HIDIO_Packet_Type__ACK,
		id
	);
}



// ----- Internal Id Functions -----

// Supported Ids Request
void HIDIO_supported_0_request()
{
	// TODO
}

// Supported Ids call
HIDIO_Return HIDIO_supported_0_call( uint16_t buf_pos, uint8_t irq )
{
	// TODO
	return HIDIO_Return__Ok;
}

// Supported Ids reply
HIDIO_Return HIDIO_supported_0_reply( HIDIO_Buffer_Entry *buf, uint8_t irq )
{
	// TODO
	return HIDIO_Return__Ok;
}

// Info Request
void HIDIO_info_1_request( uint8_t property )
{
	// TODO
}

// Info call
HIDIO_Return HIDIO_info_1_call( uint16_t buf_pos, uint8_t irq )
{
	// TODO
	return HIDIO_Return__Ok;
}

// Info reply
HIDIO_Return HIDIO_info_1_reply( HIDIO_Buffer_Entry *buf, uint8_t irq )
{
	// TODO
	return HIDIO_Return__Ok;
}

// Test Packet
void HIDIO_test_2_request( uint16_t payload_len, uint8_t payload_value )
{
	uint16_t pos = 0;

	// Push the same byte, payload_len times to the tx buffer
	while ( pos < payload_len )
	{
		pos = HIDIO_buffer_generate_packet(
			&HIDIO_tx_buf,
			pos,
			payload_len,
			&payload_value,
			1,
			HIDIO_Packet_Type__Data,
			2
		);
	}
}

// Test call
HIDIO_Return HIDIO_test_2_call( uint16_t buf_pos, uint8_t irq )
{
	// TODO (HaaTa) - Add option to process optionally inside irqs
	if ( irq )
	{
		return HIDIO_Return__Delay;
	}

	// Munch buffer entry header, not including data
	uint8_t tmpbuf[ sizeof(HIDIO_Buffer_Entry) ];
	uint8_t *buf = HIDIO_buffer_munch( &HIDIO_assembly_buf, (uint8_t*)&tmpbuf, buf_pos, sizeof(HIDIO_Buffer_Entry) );
	HIDIO_Buffer_Entry *entry = (HIDIO_Buffer_Entry*)buf;

	// Make sure entry is ready
	if ( !entry->done )
	{
		return HIDIO_Return__Delay;
	}

	// Get size and iterate through payload
	uint16_t transitions = 0;
	uint8_t last_byte = 0;
	for ( uint16_t pos = 0; pos < entry->size; pos++ )
	{
		uint8_t buf;
		uint16_t calc_buf_pos = HIDIO_buffer_position( &HIDIO_assembly_buf, buf_pos + sizeof(HIDIO_Buffer_Entry), pos );
		uint8_t *byte = HIDIO_buffer_munch( &HIDIO_assembly_buf, &buf, calc_buf_pos, 1 );

		// Count transitions, should only be 1, from 0 to value
		if ( *byte != last_byte )
		{
			transitions++;
			last_byte = *byte;
		}
	}

	// Check if there was any data corruption
	if ( transitions > 1 )
	{
		// This will automatically NAK for us
		return HIDIO_Return__InBuffer_Fail;
	}

	// TODO (HaaTa): Delay/segment transfer if ACK send buffer is not large enough

	// Prepare ACK
	uint16_t pos = 0;
	while ( pos < entry->size )
	{
		pos = HIDIO_buffer_generate_packet(
			&HIDIO_ack_send_buf,
			pos,
			entry->size,
			&last_byte,
			1,
			HIDIO_Packet_Type__ACK,
			2
		);
	}

	// Buffer is automatically released for us
	return HIDIO_Return__Ok;
}

// Test reply
HIDIO_Return HIDIO_test_2_reply( HIDIO_Buffer_Entry *buf, uint8_t irq )
{
	// TODO (HaaTa) - Add option to process optionally inside irqs
	if ( irq )
	{
		return HIDIO_Return__Delay;
	}

	// Make sure entry is ready
	if ( !buf->done )
	{
		return HIDIO_Return__Delay;
	}

	// Get size and iterate through payload, start after id
	uint16_t transitions = 0;
	uint8_t last_byte = 0;
	for ( uint16_t pos = 0; pos < buf->size; pos++ )
	{
		uint8_t byte = buf->data[ pos ];

		// Count transitions, should only be 1, from 0 to value
		if ( byte != last_byte )
		{
			transitions++;
			last_byte = byte;
		}
	}

	// Check if there was any data corruption
	if ( transitions > 1 )
	{
		// This will automatically NAK for us
		return HIDIO_Return__InBuffer_Fail;
	}

	// Buffer is automatically released for us
	return HIDIO_Return__Ok;
}

// Invalid Id Request
void HIDIO_invalid_65535_request()
{
	// Generate invalid Id 0-length data packet
	HIDIO_buffer_generate_packet(
		&HIDIO_ack_send_buf,
		0,
		0,
		0,
		0,
		HIDIO_Packet_Type__Data,
		65535
	);
}



// ----- Functions -----

// Register HID-IO callbacks
// If an Id is not registered, it is ignored and automatically NAK'd
void HIDIO_register_id( uint32_t id, void* incoming_call_func, void* incoming_reply_func )
{
	// Check if there is any room left in the list
	if ( HIDIO_Id_List_Size >= HIDIO_Id_List_MaxSize )
	{
		erro_msg("HIDIO_Id_List is full, cannot register Id: ");
		printInt32( id );
		print( NL );
		return;
	}

	// Add id to unsorted list (no reason to sort, as Ids may not be contiguous)
	HIDIO_Id_Entry *entry = &HIDIO_Id_List[ HIDIO_Id_List_Size++ ];
	entry->id = id;
	entry->call_func = incoming_call_func;
	entry->reply_func = incoming_reply_func;
}

// Initiate registered call function
// id - Function id
// buf_pos - Index in ring buffer
// irq - Set to 1 if called from an IRQ
HIDIO_Return HIDIO_call_id( uint32_t id, uint16_t buf_pos, uint8_t irq )
{
	HIDIO_Return retval = HIDIO_Return__Unknown;

	// Find id
	for ( uint16_t pos = 0; pos < HIDIO_Id_List_Size; pos++ )
	{
		// Match id
		if ( HIDIO_Id_List[ pos ].id == id )
		{
			// Map function pointer
			HIDIO_Return (*func)(uint16_t, uint8_t) = \
				(HIDIO_Return(*)(uint16_t, uint8_t))(HIDIO_Id_List[ pos ].call_func);

			// Call function
			retval = func( buf_pos, irq );
		}
	}

	// Enough space to store header
	uint8_t tmpdata[sizeof(HIDIO_Buffer_Entry)];
	uint16_t datasize;
	HIDIO_Buffer_Entry *entry;

	switch ( retval )
	{
	// HIDIO_Return__Ok, we can pop the most recent assembly_buf packet
	case HIDIO_Return__Ok:
		entry = (HIDIO_Buffer_Entry*)HIDIO_buffer_munch( &HIDIO_assembly_buf, tmpdata, HIDIO_assembly_buf.head, sizeof(tmpdata) );

		// Determine size of data
		datasize = sizeof(HIDIO_Buffer_Entry) + entry->size;

		// Pop bytes and decrement packet ready counter
		if ( HIDIO_buffer_pop_bytes( &HIDIO_assembly_buf, datasize ) )
		{
			HIDIO_assembly_buf.packets_ready--;
		}
		// Failed pop, generally popping more buffer than is available
		// (this is very bad, but recovering anyways)
		else
		{
			HIDIO_assembly_buf.packets_ready = 0;
			HIDIO_assembly_buf.head = 0;
			HIDIO_assembly_buf.tail = 0;
		}

		// Unset waiting for ACK packet
		HIDIO_assembly_buf.waiting = 0;
		break;

	// HIDIO_Return__InBuffer_Fail, Nak (or related), drop all continued packets if necessary
	case HIDIO_Return__InBuffer_Fail:
		// TODO (HaaTa)
		print("FAIL"NL);
		break;

	default:
		break;
	}

	return retval;
}

// Initiate registered reply function
// id - Function id
// buf - Pointer to the ack buffer
// irq - Set to 1 if called from an IRQ
HIDIO_Return HIDIO_reply_id( uint32_t id, uint8_t *buf, uint8_t irq )
{
	HIDIO_Return retval = HIDIO_Return__Unknown;

	// Find id
	for ( uint16_t pos = 0; pos < HIDIO_Id_List_Size; pos++ )
	{
		// Match id
		if ( HIDIO_Id_List[ pos ].id == id )
		{
			// Map function pointer
			HIDIO_Return (*func)(HIDIO_Buffer_Entry*, uint8_t) = \
				(HIDIO_Return(*)(HIDIO_Buffer_Entry*, uint8_t))(HIDIO_Id_List[ pos ].reply_func);

			// Call function
			retval = func( (HIDIO_Buffer_Entry*)buf, irq );
			break;
		}
	}

	// Enough space to store header
	uint8_t tmpdata[6];
	uint16_t datasize;
	HIDIO_Packet *packet;

	switch ( retval )
	{
	// HIDIO_Return__Ok, we can pop the most recent tx_buf packet
	case HIDIO_Return__Ok:
		packet = (HIDIO_Packet*)HIDIO_buffer_munch( &HIDIO_tx_buf, tmpdata, HIDIO_tx_buf.head, sizeof(tmpdata) );

		// Determine size of data
		datasize = (packet->upper_len << 8) | packet->len;

		// Pop bytes and decrement packet ready counter
		if ( HIDIO_buffer_pop_bytes( &HIDIO_tx_buf, datasize + 2 ) )
		{
			HIDIO_tx_buf.packets_ready--;
		}
		// Failed pop, generally popping more buffer than is available
		// (this is very bad, but recovering anyways)
		else
		{
			HIDIO_tx_buf.packets_ready = 0;
			HIDIO_tx_buf.head = 0;
			HIDIO_tx_buf.tail = 0;
		}

		// Unset waiting for ACK packet
		HIDIO_tx_buf.waiting = 0;
		break;

	// HIDIO_Return__InBuffer_Fail, Nak (or related), drop all continued packets if necessary
	case HIDIO_Return__InBuffer_Fail:
		// TODO (HaaTa)
		print("FAIL"NL);
		break;

	default:
		break;
	}

	return retval;
}

// Set packet size
void HIDIO_packet_size( uint16_t size )
{
	HIDIO_Packet_Size = size;
}

// Called on any incoming packets
void HIDIO_packet_interrupt( uint8_t *buf )
{
	print("YAY");
}

// HID-IO Module Setup
inline void HIDIO_setup()
{
	// Default packet size (i.e. lowest supported)
	HIDIO_Packet_Size = 8;

	// Setup Assembly Buffer
	HIDIO_assembly_buf.head = 0;
	HIDIO_assembly_buf.tail = 0;
	HIDIO_assembly_buf.cur_buf_head = 0;
	HIDIO_assembly_buf.len = sizeof(HIDIO_assembly_buf_data);
	HIDIO_assembly_buf.packets_ready = 0;
	HIDIO_assembly_buf.waiting = 0;
	HIDIO_assembly_buf.data = HIDIO_assembly_buf_data;

	// Setup ACK Buffer
	HIDIO_ack_buf = (HIDIO_Buffer_Entry*)HIDIO_ack_buf_data;
	HIDIO_ack_buf->id = 0;
	HIDIO_ack_buf->size = 0;
	HIDIO_ack_buf->done = 0;

	// Setup ACK Send Buffer
	HIDIO_ack_send_buf.head = 0;
	HIDIO_ack_send_buf.tail = 0;
	HIDIO_ack_send_buf.cur_buf_head = 0;
	HIDIO_ack_send_buf.len = sizeof(HIDIO_ack_send_data);
	HIDIO_ack_send_buf.packets_ready = 0;
	HIDIO_ack_send_buf.waiting = 0;
	HIDIO_ack_send_buf.data = HIDIO_ack_send_data;

	// Setup Tx Buffer
	HIDIO_tx_buf.head = 0;
	HIDIO_tx_buf.tail = 0;
	HIDIO_tx_buf.cur_buf_head = 0;
	HIDIO_tx_buf.len = sizeof(HIDIO_tx_buf_data);
	HIDIO_tx_buf.packets_ready = 0;
	HIDIO_tx_buf.waiting = 0;
	HIDIO_tx_buf.data = HIDIO_tx_buf_data;

	// Register Output CLI dictionary
	CLI_registerDictionary( hidioCLIDict, hidioCLIDictName );

	// Allocate latency resource
	hidioLatencyResource = Latency_add_resource("HID-IO", LatencyOption_Ticks);

	// Reset internal id list
	HIDIO_Id_List_Size = 0;

	// Register internal Ids
	HIDIO_register_id( 0, (void*)HIDIO_supported_0_call, (void*)HIDIO_supported_0_reply );
	HIDIO_register_id( 1, (void*)HIDIO_info_1_call, (void*)HIDIO_info_1_reply );
	HIDIO_register_id( 2, (void*)HIDIO_test_2_call, (void*)HIDIO_test_2_reply );
}

// HID-IO Process Packet
void HIDIO_process_incoming_packet( uint8_t *buf, uint8_t irq )
{
	// Map structure to packet data
	HIDIO_Packet *packet = (HIDIO_Packet*)buf;

	// Check header packet type to see if a valid packet
	if ( packet->type > HIDIO_Packet_Type__Continued )
		return;

	// Check if the length is valid
	uint16_t packet_len = (packet->upper_len << 8) | packet->len;
	if ( packet_len > HIDIO_Packet_Size )
		return;

	// Packet type
	HIDIO_Packet_Type type = packet->type;

	// Id Width
	uint8_t id_width_len = HIDIO_buffer_id_width( packet->id_width );

	// Payload length, excludes the Id length from the packet length
	uint16_t payload_len = packet_len - id_width_len;

	// Check if valid Id
	uint32_t id = 0;
	uint8_t *data = 0;
	switch ( type )
	{
	case HIDIO_Packet_Type__Sync:
		// TODO
		print("SYNC");
		break;

	// XXX Falls through on purpose
	// We first determine if we're reassembling in the data or ack buffers
	case HIDIO_Packet_Type__Continued:
		// Modify type so we know what to do with the payload
		type = HIDIO_ack_buf->done ? HIDIO_Packet_Type__Data : HIDIO_Packet_Type__ACK;

	// Most packet types
	default:
		id = HIDIO_buffer_id( packet );

		// Data start
		data = HIDIO_payload_start( packet );
		break;
	}

	// Process packet
	switch ( type )
	{
	case HIDIO_Packet_Type__Data:
		// Determine if we're appending to a continued packet
		if ( !HIDIO_assembly_buf.waiting )
		{
			// If this is a continued packet, we drop, because we weren't waiting for one
			if ( packet->type == HIDIO_Packet_Type__Continued )
			{
				warn_print("Dropping incoming Continued Data packet...");
				return;
			}

			// Check if there's enough space in the ring buffer
			// If there isn't, we drop the packet
			if ( HIDIO_buffer_free_bytes( &HIDIO_assembly_buf ) < sizeof(HIDIO_Buffer_Entry) + payload_len )
			{
				warn_print("Dropping incoming Data packet, not enough buffer space...");
				print("head: ");
				printInt16( HIDIO_assembly_buf.head );
				print(" tail: ");
				printInt16( HIDIO_assembly_buf.tail );
				print(" bytes_left: ");
				printInt16( HIDIO_buffer_free_bytes( &HIDIO_assembly_buf ) );
				print(" request: ");
				printInt16( sizeof(HIDIO_Buffer_Entry) + payload_len );
				print(NL);
				return;
			}

			// Generate HIDIO_Buffer_Entry
			HIDIO_Buffer_Entry entry;
			entry.id = id;
			entry.size = payload_len;
			entry.done = packet->cont ? 0 : 1;
			entry.type = packet->type;

			// Determine buffer position where we are starting
			HIDIO_assembly_buf.cur_buf_head = HIDIO_assembly_buf.tail;

			// Copy byte-by-byte into ring buffer
			// TODO (HaaTa): Make more efficient by checking conditions and using memcpy when appropriate
			// First the entry info
			for ( uint8_t c = 0; c < sizeof(HIDIO_Buffer_Entry); c++ )
			{
				HIDIO_buffer_push_byte( &HIDIO_assembly_buf, ((uint8_t*)&entry)[c] );
			}
		}
		else
		{
			// Lookup entry, and update accordingly
			uint8_t tmpbuf[sizeof(HIDIO_Buffer_Entry)];
			HIDIO_Buffer_Entry *entry = (HIDIO_Buffer_Entry*)HIDIO_buffer_munch(
				&HIDIO_assembly_buf,
				tmpbuf,
				HIDIO_assembly_buf.cur_buf_head,
				sizeof(tmpbuf)
			);

			// Update entry
			entry->size += payload_len;
			entry->done = packet->cont ? 0 : 1;

			// Check if we had to copy the entry into memory first
			// If so, we have to copy the data back
			if ( tmpbuf == (uint8_t*)entry )
			{
				HIDIO_modify_buffer(
					&HIDIO_assembly_buf,
					HIDIO_assembly_buf.cur_buf_head,
					tmpbuf,
					sizeof(tmpbuf)
				);
			}
		}

		// Determine if we are waiting for continued packets
		HIDIO_assembly_buf.waiting = packet->cont;

		// Then the payload data
		for ( uint16_t c = 0; c < payload_len; c++ )
		{
			HIDIO_buffer_push_byte( &HIDIO_assembly_buf, data[c] );
		}

		// If finished, send to appropriate registered callback
		HIDIO_assembly_buf.packets_ready++;
		if ( !HIDIO_assembly_buf.waiting )
		{
			HIDIO_Return retval = HIDIO_call_id( id, HIDIO_assembly_buf.cur_buf_head, irq );

			// Handle error cases
			switch ( retval )
			{
			// Invalid Id
			case HIDIO_Return__Unknown:
				// Release packet
				HIDIO_assembly_buf.packets_ready--;
				HIDIO_assembly_buf.head = HIDIO_assembly_buf.tail;

				// Generate invalid Id 0-length data packet
				HIDIO_buffer_generate_packet(
					&HIDIO_ack_send_buf,
					0,
					0,
					0,
					0,
					HIDIO_Packet_Type__NAK,
					id
				);
				break;

			// Do nothing usually
			default:
				break;
			}
		}
		// Otherwise do a simple ACK
		else
		{
			HIDIO_nopayload_ack( id );
		}
		break;

	case HIDIO_Packet_Type__ACK:
	case HIDIO_Packet_Type__NAK:
		// ACK/NAK packets have their own assembly buffer
		// Setup entry
		HIDIO_ack_buf->id = id;
		HIDIO_ack_buf->size = payload_len;
		HIDIO_ack_buf->done = packet->cont ? 0 : 1;
		HIDIO_ack_buf->type = packet->type;

		// Copy to buffer
		memcpy( &(HIDIO_ack_buf->data[0]), data, payload_len );

		// If finished, send to appropriate registered callback
		if ( HIDIO_ack_buf->done )
		{
			HIDIO_reply_id( id, (uint8_t*)HIDIO_ack_buf, irq );
		}
		break;
		break;

	default:
		// TODO (HaaTa)
		print("TODO!"NL);
	}

	// Debug
	/*
	print("(");
	printInt32( id );
	print(":");
	printInt16( packet_len );
	print(") ");
	for ( uint16_t pos = 0; pos < payload_len; pos++ )
	{
		printChar( (char)(data[ pos ]) );
	}
	print( NL );
	*/
}

// HID-IO Processing Loop
inline void HIDIO_process()
{
	// Start latency measurement
	Latency_start_time( hidioLatencyResource );

	// TODO (HaaTa): Handle timeouts and lost packets

	// Retrieve incoming packets
	while ( Output_rawio_availablechar() )
	{
		// XXX Double copy is done here with getbuffer
		uint8_t tmpdata[ HIDIO_MAX_PACKET_SIZE ];
		Output_rawio_getbuffer( (char*)&tmpdata );

		// Process Packet, regular process (no interrupt)
		HIDIO_process_incoming_packet( tmpdata, 0 );
	}

	// Send all ACK packets
	while ( HIDIO_ack_send_buf.packets_ready > 0 )
	{
		// Prepare 64 byte packet
		// TODO (HaaTa): Handle internal max size
		uint8_t tmpdata[64];
		uint8_t *buf = HIDIO_buffer_munch( &HIDIO_ack_send_buf, tmpdata, HIDIO_ack_send_buf.head, HIDIO_Packet_Size );
		HIDIO_Packet *packet = (HIDIO_Packet*)buf;

		// Send packet
		// TODO (HaaTa): Check error?
		Output_rawio_sendbuffer( (char*)buf );

		// Indicate waiting for ACK packet
		HIDIO_ack_send_buf.waiting = 1;

		// Determine size of data
		uint16_t datasize = (packet->upper_len << 8) | packet->len;

		// Pop bytes and decrement packet ready counter
		if ( HIDIO_buffer_pop_bytes( &HIDIO_ack_send_buf, datasize + 2 ) )
		{
			HIDIO_ack_send_buf.packets_ready--;
		}
		// Failed pop, generally popping more buffer than is available
		// (this is very bad, but recovering anyways)
		else
		{
			HIDIO_ack_send_buf.packets_ready = 0;
			HIDIO_ack_send_buf.head = 0;
			HIDIO_ack_send_buf.tail = 0;
		}
	}

	// Send outgoing packet, we can only send one at a time
	// and the next one can only be sent after an ACK is recieved
	if ( HIDIO_tx_buf.packets_ready > 0 && HIDIO_tx_buf.waiting == 0 )
	{
		// Prepare 64 byte packet
		// TODO (HaaTa): Handle internal max size
		uint8_t tmpdata[64];
		uint8_t *buf = HIDIO_buffer_munch( &HIDIO_tx_buf, tmpdata, HIDIO_tx_buf.head, HIDIO_Packet_Size );
		HIDIO_Packet *packet = (HIDIO_Packet*)buf;

		// Send packet
		// TODO (HaaTa): Check error?
		Output_rawio_sendbuffer( (char*)packet );

		// Indicate waiting for ACK packet
		// Once ACK has been received (or NAK) the packet will be released
		HIDIO_tx_buf.waiting = 1;
	}

	// End latency measurement
	Latency_end_time( hidioLatencyResource );
}



// ----- CLI Command Functions -----

