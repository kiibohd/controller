/* Copyright (C) 2015 by Jacob Alexander
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



// ----- Enums -----

typedef enum FrameState {
	FrameState_Ready,   // Buffers have been updated and are ready to send
	FrameState_Sending, // Buffers are currently being sent, do not change
	FrameState_Update,  // Buffers need to be updated to latest frame
} FrameState;

// Pixel Change Storage
// - Store only the change of the pixel
// - Change is a value (size of the pixel)
// - Contiguous sets of pixel changes can be stored for maximized packing (with the same width)
// - Each value has a corresponding operator
//   * Add
//   * Subtract
//   * Left shift
//   * Right shift
//   * Set
//   * Add no-rollover
//   * Subtract no-rollover
typedef enum PixelChange {
	PixelChange_Set = 0,          // <no op>
	PixelChange_Add,              // +
	PixelChange_Subtract,         // -
	PixelChange_NoRoll_Add,       // +:
	PixelChange_NoRoll_Subtract,  // -:
	PixelChange_LeftShift,        // <<
	PixelChange_RightShift,       // >>
} PixelChange;



// ----- Structs -----

// Element of array of buffers pointers
typedef struct PixelBuf {
	uint8_t  size;   // Number of elements
	uint8_t  width;  // Width of each element
	uint16_t offset; // Subtraction offset from absolute channel
	void    *data;   // Pointer to start of buffer
} PixelBuf;
#define PixelBufElem(len,width,offset,ptr) { len, width, offset, (void*)ptr }
#define PixelBuf8(pixbuf, ch)  ( ((uint8_t*) (pixbuf.data))[ ch - pixbuf.offset ] )
#define PixelBuf16(pixbuf, ch) ( ((uint16_t*)(pixbuf.data))[ ch - pixbuf.offset ] )


// Individual Pixel element
typedef struct PixelElement {
	uint8_t  width;      // Number of bits in a channel
	uint8_t  channels;   // Number of channels
	uint16_t indices[0]; // Hardware indices for each channel
} PixelElement;
#define s2b(num) (num >> 8), (num & 0xFF)
#define Pixel_RGBChannel(r,g,b) 8, 3, s2b(r), s2b(g), s2b(b)
#define Pixel_8bitChannel(c)    8, 1, s2b(c)


typedef struct PixelMod {
	uint16_t    pixel;            // Pixel index
	PixelChange change;           // Change to apply to pixel
	uint8_t     contiguousPixels; // # of contiguous pixels to apply same changes too
	uint8_t     data[0];          // Data size depends on PixelElement definition
} PixelMod;



// ----- Variables -----

extern FrameState Pixel_FrameState;



// ----- Functions -----

void Pixel_process();
void Pixel_setup();

