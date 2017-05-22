/* Copyright (C) 2015-2017 by Jacob Alexander
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

// KLL Include
#include <kll.h>



// ----- Defines -----



// ----- Enums -----

typedef enum FrameState {
	FrameState_Ready,   // Buffers have been updated and are ready to send
	FrameState_Sending, // Buffers are currently being sent, do not change
	FrameState_Update,  // Buffers need to be updated to latest frame
	FrameState_Pause,   // Pause frame state
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
	PixelChange_Set = 0,          // =
	PixelChange_Add,              // +
	PixelChange_Subtract,         // -
	PixelChange_NoRoll_Add,       // +:
	PixelChange_NoRoll_Subtract,  // -:
	PixelChange_LeftShift,        // <<
	PixelChange_RightShift,       // >>
} PixelChange;

// Frame Function
// - Frame transition function
// - e.g. Interpolation between animation frames
typedef enum PixelFrameFunction {
	PixelFrameFunction_Off = 0,
	PixelFrameFunction_Interpolation,
	PixelFrameFunction_InterpolationKLL,
} PixelFrameFunction;

// Pixel Function
// - Pixel fill function, allows for simpler KLL definitions
// - e.g. Interpolation between pixels within a frame
typedef enum PixelPixelFunction {
	PixelPixelFunction_Off = 0,
	PixelPixelFunction_PointInterpolation,
	PixelPixelFunction_PointInterpolationKLL,
} PixelPixelFunction;

// Pixel Mod Type
// - Determines how to address the pixel(s)
typedef enum PixelAddressType {
	PixelAddressType_End = 0,            // Signals end of frame

	PixelAddressType_Index,              // Direct lookup to the array map
	PixelAddressType_Rect,               // Row vs. Column lookup
	PixelAddressType_ColumnFill,         // Column fill
	PixelAddressType_RowFill,            // Row fill
	PixelAddressType_ScanCode,           // Scan code lookup (uses layer stack)
	                                     //  USB Codes are translated to type
	PixelAddressType_RelativeIndex,      // Relative index lookup
	PixelAddressType_RelativeRect,       // Relative row vs. column lookup
	PixelAddressType_RelativeColumnFill, // Relative column fill
	PixelAddressType_RelativeRowFill,    // Relative row fill
} PixelAddressType;

// Animation Replace Type
typedef enum AnimationReplaceType {
	AnimationReplaceType_None  = 0, // Don't replace (add new animation to stack if not full)
	AnimationReplaceType_Basic = 1, // Replace only if the same trigger initiated
	AnimationReplaceType_All   = 2, // Replace no matter what trigger initiated
} AnimationReplaceType;

// Animation Play State
typedef enum AnimationPlayState {
	AnimationPlayState_Start = 0, // Start animation (default)
	AnimationPlayState_Pause = 1, // Pause animation
	AnimationPlayState_Stop  = 2, // Stop animation (removes animation state)
} AnimationPlayState;



// ----- Structs -----

// Element of array of buffers pointers
typedef struct PixelBuf {
	uint8_t  size;   // Number of elements
	uint8_t  width;  // Width of each element
	uint16_t offset; // Subtraction offset from absolute channel
	void    *data;   // Pointer to start of buffer
} PixelBuf;
#define PixelBufElem(len,width,offset,ptr) { len, width, offset, (void*)ptr }

// Convience macros for Pixel Evaluations at different bit widths
#define PixelBuf8(pixbuf, ch)  ( ((uint8_t*) (pixbuf->data))[ ch - pixbuf->offset ] )
#define PixelBuf16(pixbuf, ch) ( ((uint16_t*)(pixbuf->data))[ ch - pixbuf->offset ] )
#define PixelBuf32(pixbuf, ch) ( ((uint32_t*)(pixbuf->data))[ ch - pixbuf->offset ] )


// Individual Pixel element
#define Pixel_MaxChannelPerPixel 3 // TODO Generate
typedef struct PixelElement {
	uint8_t  width;      // Number of bits in a channel
	uint8_t  channels;   // Number of channels
	                     // Hardware indices for each channel
	uint16_t indices[Pixel_MaxChannelPerPixel];
} PixelElement;
#define Pixel_Blank() { 0, 0, {} }

// Rectangle lookup for column, row and row vs. col
typedef struct PixelRect {
	int16_t col;
	int16_t row;
} PixelRect;

// Pixel Mod Element
// - Animation frame element
typedef struct PixelModElement {
	PixelAddressType type;        // Address type
	union {
		PixelRect rect;       // Rectangle lookup for column, row and row vs. col
		int32_t  index;       // Index lookup for direct and scancode lookups
	};
	uint8_t     data[0];          // Data size depends on PixelElement definition
	                              // ( PixelElement.width / 8 + sizeof(PixelChange) ) * PixelElement.channels
} __attribute((packed)) PixelModElement;

// Pixel Mod Data Element
// - Each element of uint8_t data[0]
typedef struct PixelModDataElement {
	PixelChange change;
	uint8_t     data[0];
} __attribute((packed)) PixelModDataElement;

// Animation stack element
typedef struct AnimationStackElement {
	TriggerMacro        *trigger;  // TriggerMacro that added element, set to 0 if unused
	                               // If set to 1 in default settings, animation is enabled at start time
	uint16_t             index;    // Animation id
	uint16_t             pos;      // Current fundamental frame (XXX Make 32bit?)
	uint8_t              loops;    // # of loops to run animation, 0 indicates infinite
	uint8_t              divmask;  // # of process loops used for frame transition/hold (2,4,8,16,etc.)
	                               //  Must be a contiguous mask
	                               //  e.g.  0x00, 0x01, 0x03, 0x0F, etc.
	                               //  *NOT* 0x02, 0x08, 0x24, etc.
	uint8_t              divshift; // Matches divmask
	                               //  Number of bits to shift until LSFB (least significant frame bit)
	                               //  e.g.  0x03 => 2; 0x0F => 4
	PixelFrameFunction   ffunc;    // Frame tweening function
	PixelPixelFunction   pfunc;    // Pixel tweening function
	// TODO ffunc and pfunc args
	AnimationReplaceType replace;  // Replace type for stack element
	AnimationPlayState   state;    // Animation state
} AnimationStackElement;

// Animation stack
#define Pixel_AnimationStackSize 20
typedef struct AnimationStack {
	uint16_t size;
	AnimationStackElement *stack[Pixel_AnimationStackSize];
} AnimationStack;



// ----- Variables -----

extern FrameState Pixel_FrameState;

extern const AnimationStackElement Pixel_AnimationSettings[];

extern       PixelBuf     Pixel_Buffers[];
extern const PixelElement Pixel_Mapping[];
extern const uint16_t     Pixel_DisplayMapping[];
extern const uint8_t    **Pixel_Animations[];
extern const uint16_t     Pixel_ScanCodeToDisplay[];
extern const uint16_t     Pixel_ScanCodeToPixel[];



// ----- Functions -----

void Pixel_process();
void Pixel_setup();

