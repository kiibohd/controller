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

#pragma once

// ----- Includes -----

// Compiler Includes
#include <stdint.h>



// ----- Defines -----

// TODO Query from driver interface
#define HIDIO_MAX_PACKET_SIZE 64



// ----- Enumerations -----

typedef enum HIDIO_Packet_Type {
	HIDIO_Packet_Type__Data      = 0,
	HIDIO_Packet_Type__ACK       = 1,
	HIDIO_Packet_Type__NAK       = 2,
	HIDIO_Packet_Type__Sync      = 3,
	HIDIO_Packet_Type__Continued = 4,
} HIDIO_Packet_Type;

typedef enum HIDIO_Return {
	HIDIO_Return__Ok,
	HIDIO_Return__InBuffer_Fail,  // Problem with the incoming buffer
	HIDIO_Return__OutBuffer_Fail, // Problem generating the outgoing buffer
	HIDIO_Return__Delay,          // Delay processing (used when processing will be long, and within an interrupt)
	HIDIO_Return__Unknown,        // Unknown Id
} HIDIO_Return;



// ----- Structs -----

typedef struct HIDIO_Packet {
	HIDIO_Packet_Type type:3;
	uint8_t           cont:1;      // 0 - Only packet, 1 continued packet following
	uint8_t           id_width:1;  // 0 - 16bits, 1 - 32bits
	uint8_t           reserved:1;  // Reserved
	uint8_t           upper_len:2; // Upper 2 bits of length field (generally unused)
	uint8_t           len;         // Lower 8 bits of length field
	uint8_t           data[0];     // Start of data payload (may start with Id)
} __attribute((packed)) HIDIO_Packet;

typedef struct HIDIO_Packet16 {
	HIDIO_Packet_Type type:3;
	uint8_t           cont:1;      // 0 - Only packet, 1 continued packet following
	uint8_t           id_width:1;  // 0 - 16bits, 1 - 32bits
	uint8_t           reserved:1;  // Reserved
	uint8_t           upper_len:2; // Upper 2 bits of length field (generally unused)
	uint8_t           len;         // Lower 8 bits of length field
	uint16_t          id;          // Id field (check id_width to see which struct to use)
	uint8_t           data[0];     // Start of data payload
} __attribute((packed)) HIDIO_Packet16;

typedef struct HIDIO_Packet32 {
	HIDIO_Packet_Type type:3;
	uint8_t           cont:1;      // 0 - Only packet, 1 continued packet following
	uint8_t           id_width:1;  // 0 - 16bits, 1 - 32bits
	uint8_t           reserved:1;  // Reserved
	uint8_t           upper_len:2; // Upper 2 bits of length field (generally unused)
	uint8_t           len;         // Lower 8 bits of length field
	uint32_t          id;          // Id field (check id_width to see which struct to use)
	uint8_t           data[0];     // Start of data payload
} __attribute((packed)) HIDIO_Packet32;

typedef struct HIDIO_Buffer {
	uint16_t  head;
	uint16_t  tail;
	uint16_t  cur_buf_head; // On continued packets, we need to continously update the size field (HIDIO_Buffer_Entry)
	uint8_t   waiting; // Set to 1, if we need to update cur_buf_head; or if waiting for an ACK packet
	uint16_t  len;
	uint16_t  packets_ready; // Number of packets that are ready to be consumed
	uint8_t  *data; // HIDIO_Buffer_Entry
} HIDIO_Buffer;

typedef struct HIDIO_Buffer_Entry {
	uint32_t          id;
	uint16_t          size;
	uint8_t           done; // Set to 0 if not complete, 1 if complete
	HIDIO_Packet_Type type;
	uint8_t           data[0];
} __attribute((packed)) HIDIO_Buffer_Entry;



// ----- Variables -----

// ----- Functions -----

void HIDIO_setup();
void HIDIO_process();
void HIDIO_packet_interrupt( uint8_t* buf );

