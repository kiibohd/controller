/* Copyright (C) 2015-2019 by Jacob Alexander
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
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

// Frame Options
typedef enum PixelFrameOption {
	PixelFrameOption_None         = 0, // No options set
	PixelFrameOption_FrameStretch = 1, // During frame delay frames, re-run animation frame
} PixelFrameOption;

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
	AnimationReplaceType_None        = 0, // Don't replace (add new animation to stack if not full)
	AnimationReplaceType_Basic       = 1, // Replace only if the same trigger initiated
	AnimationReplaceType_All         = 2, // Replace no matter what trigger initiated
	AnimationReplaceType_State       = 3, // Using same trigger, start on Activate/Press, stop on Deactivate/Release
	AnimationReplaceType_Clear       = 4, // Clear all other animations before adding
	AnimationReplaceType_ClearActive = 5, // Clear all other animations before adding, except paused (replace those)
} AnimationReplaceType;

// Animation Play State
typedef enum AnimationPlayState {
	AnimationPlayState_Start     = 0,    // Start animation
	AnimationPlayState_Pause     = 1,    // Pause animation (default set by KLL Compiler)
	AnimationPlayState_Stop      = 2,    // Stop animation (removes animation state)
	AnimationPlayState_Single    = 3,    // Play a single frame of the animation
	AnimationPlayState_AutoStart = 0x80, // Set if animation is autostarting
	                                     // Multiple conditions must be met to autostart an animation
					     // This flag must be set
					     // Only default animations can be autostarted
					     // Exception would be NVM saved animations
} AnimationPlayState;

typedef enum AnimationControl {
	AnimationControl_Forward    = 0, // Default
	AnimationControl_ForwardOne = 1,
	AnimationControl_Pause      = 2, // Pauses current animations
	AnimationControl_Stop       = 3, // Clears all animations, then sets forward
	AnimationControl_Reset      = 4, // Clears all animations, starts initial animations (sets forward)
	AnimationControl_WipePause  = 5, // Pauses animations, clears the display
	AnimationControl_Clear      = 6, // Clears the display, animations continue
} AnimationControl;

typedef enum PixelPeriodIndex {
	PixelPeriodIndex_Off_to_On = 0, // Start, fading from off to on
	PixelPeriodIndex_On        = 1, // Hold on
	PixelPeriodIndex_On_to_Off = 2, // Fading from on to off
	PixelPeriodIndex_Off       = 3, // Hold off
} PixelPeriodIndex;



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
} __attribute__((packed)) PixelModElement;

// Pixel Mod Data Element
// - Each element of uint8_t data[0]
typedef struct PixelModDataElement {
	PixelChange change;
	uint8_t     data[0];
} __attribute__((packed)) PixelModDataElement;

// Animation stack element
typedef struct AnimationStackElement {
	TriggerMacro        *trigger;     // TriggerMacro that added element, set to 0 if unused
	                                  // If set to 1 in default settings, animation is enabled at start time
	uint16_t             index;       // Animation id
	uint16_t             pos;         // Current fundamental frame (XXX Make 32bit?)
	uint8_t              subpos;      // If framedelay is set, current delay position
	                                  // Counts down up to framedelay.
	uint8_t              loops;       // # of loops to run animation, 0 indicates infinite
	uint8_t              framedelay;  // # of frames to delay the animation per frame of the animation
	                                  // 0 - Full speed
	                                  // 1 - Half speed
	                                  // 2 - 1/3 speed
	                                  // etc.
	PixelFrameOption     frameoption; // Frame processing options
	PixelFrameFunction   ffunc;       // Frame tweening function
	PixelPixelFunction   pfunc;       // Pixel tweening function
	// TODO ffunc and pfunc args
	AnimationReplaceType replace;     // Replace type for stack element
	AnimationPlayState   state;       // Animation state
} AnimationStackElement;

// Animation stack
#define Pixel_AnimationStackSize Pixel_AnimationStackSize_define
typedef struct AnimationStack {
	uint16_t size;
	AnimationStackElement *stack[Pixel_AnimationStackSize];
} AnimationStack;

// start/end are defined in the number of bits used for the variable
// (1 << 0) == 1
// (1 << 4) == 16
// Maximum value is 24 (limit of algorithm)
// (1 << 24) == 16777216 (0x1000000)
// Maximum functional value is 15 (to save on EEPROM storage)
// Actual time delay will depend on the current FPS setting for the LED driver
typedef struct PixelPeriodConfig {
	uint8_t start:4; // (1 << start) - 1
	uint8_t end:4;   // (1 << end)
} __attribute__((packed)) PixelPeriodConfig;

typedef struct PixelFadeProfile {
	// 0: Off -> On period
	// 1: On hold time
	// 2: On -> Off period
	// 3: Off hold time
	PixelPeriodConfig conf[4];
	uint32_t pos;                 // Current position with the current PixelPeriodConfig
	PixelPeriodIndex period_conf; // Which PixelPeriodConfig is being processed
	uint8_t brightness;
} PixelFadeProfile;

typedef struct PixelLEDGroupEntry {
	const uint16_t size;
	const uint16_t *pixels;
} PixelLEDGroupEntry;



// ----- Variables -----

extern FrameState Pixel_FrameState;

extern const AnimationStackElement Pixel_AnimationSettings[];

extern const PixelLEDGroupEntry Pixel_LED_DefaultFadeGroups[];
extern const PixelPeriodConfig  Pixel_LED_FadePeriods[];
extern const uint8_t            Pixel_LED_FadePeriod_Defaults[4][4];
extern const uint8_t            Pixel_LED_FadeBrightness[4];

extern       PixelBuf     Pixel_Buffers[];
extern       PixelBuf     LED_Buffers[];
extern const PixelElement Pixel_Mapping[];
extern const uint16_t     Pixel_DisplayMapping[];
extern const uint8_t    **Pixel_Animations[];
extern const uint16_t     Pixel_ScanCodeToDisplay[];
extern const uint16_t     Pixel_ScanCodeToPixel[];

extern const Layer        LayerIndex[];



// ----- Functions -----

void Pixel_process();
void Pixel_setup();

void Pixel_setAnimationControl( AnimationControl control );

void LED_SetPixels( uint8_t shouldSetOn );

