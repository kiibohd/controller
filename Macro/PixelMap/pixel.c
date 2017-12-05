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

// ----- Includes -----

// Compiler Includes
#include <Lib/MacroLib.h>

// Project Includes
#include <cli.h>
#include <kll_defs.h>
#include <latency.h>
#include <led.h>
#include <print.h>
#include <output_com.h>

// Local Includes
#include "pixel.h"



// ----- Function Declarations -----

void cliFunc_aniAdd     ( char* args );
void cliFunc_aniDel     ( char* args );
void cliFunc_aniStack   ( char* args );
void cliFunc_chanTest   ( char* args );
void cliFunc_pixelList  ( char* args );
void cliFunc_pixelSCTest( char* args );
void cliFunc_pixelTest  ( char* args );
void cliFunc_pixelXYTest( char* args );
void cliFunc_rectDisp   ( char* args );



// ----- Enums -----

typedef enum PixelTest {
	PixelTest_Off = 0,    // Disabled
	PixelTest_Chan_All,   // Enable all positions
	PixelTest_Chan_Roll,  // Iterate over all positions
	PixelTest_Chan_Full,  // Turn on all pixels
	PixelTest_Chan_Off,   // Turn off all pixels
	PixelTest_Pixel_All,  // Enable all positions
	PixelTest_Pixel_Roll, // Iterate over all positions
	PixelTest_Pixel_Full, // Turn on all pixels
	PixelTest_Pixel_Off,  // Turn off all pixels
	PixelTest_Scan_All,
	PixelTest_Scan_Roll,
	PixelTest_XY_All,
	PixelTest_XY_Roll,
} PixelTest;



// ----- Variables -----

// Macro Module command dictionary
CLIDict_Entry( aniAdd,       "Add the given animation id to the stack" );
CLIDict_Entry( aniDel,       "Remove the given stack index animation" );
CLIDict_Entry( aniStack,     "Displays the animation stack contents" );
CLIDict_Entry( chanTest,     "Channel test. No arg - next pixel. # - pixel, r - roll-through. a - all, s - stop" );
CLIDict_Entry( pixelList,    "Prints out pixel:channel mappings." );
CLIDict_Entry( pixelSCTest,  "Scancode pixel test. No arg - next pixel. # - pixel, r - roll-through. a - all, s - stop" );
CLIDict_Entry( pixelTest,    "Pixel test. No arg - next pixel. # - pixel, r - roll-through. a - all, s - stop, f - full" );
CLIDict_Entry( pixelXYTest,  "XY pixel test. No arg - next pixel. # - pixel, r - roll-through. a - all, s - stop" );
CLIDict_Entry( rectDisp,     "Show the current output of the MCU pixel buffer." );

CLIDict_Def( pixelCLIDict, "Pixel Module Commands" ) = {
	CLIDict_Item( aniAdd ),
	CLIDict_Item( aniDel ),
	CLIDict_Item( aniStack ),
	CLIDict_Item( chanTest ),
	CLIDict_Item( pixelList ),
	CLIDict_Item( pixelSCTest ),
	CLIDict_Item( pixelTest ),
	CLIDict_Item( pixelXYTest ),
	CLIDict_Item( rectDisp ),
	{ 0, 0, 0 } // Null entry for dictionary end
};

// Debug states
PixelTest Pixel_testMode;
uint16_t  Pixel_testPos = 0;

// Frame State
//  Indicates to pixel and output modules current state of the buffer
FrameState Pixel_FrameState;

// Animation Stack
AnimationStack Pixel_AnimationStack;

// Animation Control
AnimationControl Pixel_animationControl;

// Memory Stor for Animation Elements
// Animation elements may be called multiple times, thus memory must be allocated per instance
AnimationStackElement Pixel_AnimationElement_Stor[Pixel_AnimationStackSize];

#if defined(_host_)
uint16_t Pixel_AnimationStack_HostSize = Pixel_AnimationStackSize;
uint8_t  Pixel_Buffers_HostLen = Pixel_BuffersLen_KLL;
uint8_t  Pixel_MaxChannelPerPixel_Host = Pixel_MaxChannelPerPixel;
uint16_t Pixel_Mapping_HostLen = 128; // TODO Define
uint8_t  Pixel_AnimationStackElement_HostSize = sizeof( AnimationStackElement );
#endif

// Latency Measurement Resource
static uint8_t pixelLatencyResource;



// ----- Function Declarations -----

uint8_t Pixel_animationProcess( AnimationStackElement *elem );
uint8_t Pixel_addAnimation( AnimationStackElement *element );
uint8_t Pixel_determineLastTriggerScanCode( TriggerMacro *trigger );

void Pixel_pixelSet( PixelElement *elem, uint32_t value );

PixelBuf *Pixel_bufferMap( uint16_t channel );

AnimationStackElement *Pixel_lookupAnimation( uint16_t index, uint16_t prev );



// ----- Capabilities -----

void Pixel_AnimationIndex_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Pixel_AnimationIndex_capability(settingindex)");
		return;
	}

	// Only use capability on press
	// TODO Analog
	if ( state != 0x01 )
		return;

	// Lookup animation settings
	uint16_t index = *(uint16_t*)(&args[0]);

	// Check if a valid setting
	if ( index >= Pixel_AnimationSettingsNum_KLL )
	{
		warn_msg("Invalid AnimationSetting index: ");
		printInt16( index );
		print( NL );
		return;
	}

	AnimationStackElement element = Pixel_AnimationSettings[ index ];
	element.trigger = trigger;

	Pixel_addAnimation( &element );
}

void Pixel_Animation_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Pixel_Animation_capability(index,loops,pfunc,framedelay,frameoption,replace)");
		return;
	}

	// Only use capability on press
	// TODO Analog
	if ( state != 0x01 )
		return;

	AnimationStackElement element;
	element.trigger = trigger;
	element.pos = 0; // TODO (HaaTa) Start at specific frame
	element.subpos = 0;
	element.index = *(uint16_t*)(&args[0]);
	element.loops = *(uint8_t*)(&args[2]);
	element.pfunc = *(uint8_t*)(&args[3]);
	element.framedelay = *(uint8_t*)(&args[4]);
	element.frameoption = *(uint8_t*)(&args[5]);
	element.replace = *(uint8_t*)(&args[6]);

	Pixel_addAnimation( &element );
}

void Pixel_Pixel_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Pixel_Pixel_capability(pixel,chan,value)");
		return;
	}

	// Only use capability on press
	// TODO Analog
	if ( state != 0x01 )
		return;

	/*
	PixelChange change = *(PixelChange*)(&args[0]);
	uint16_t channel = *(uint16_t*)(&args[1]);
	uint32_t value = *(uint32_t*)(&args[3]);
	*/

	// TODO (HaaTa) Apply the channel modification
}

void Pixel_AnimationControl_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Pixel_AnimationControl_capability(func)");
		return;
	}

	// Only use capability on press
	// TODO Analog
	if ( state != 0x01 )
		return;

	uint8_t arg  = *(uint8_t*)(&args[0]);

	// Decide how to handle function
	switch ( arg )
	{
	case 0: // Pause/Resume
		// Determine how to handle Pause/Resume
		switch ( Pixel_animationControl )
		{
		case AnimationControl_Forward:
		case AnimationControl_ForwardOne:
			Pixel_animationControl = AnimationControl_Pause;
			break;
		case AnimationControl_Pause:
		default:
			Pixel_animationControl = AnimationControl_Forward;
			break;
		}
		break;
	case 1: // Forward one frame
		Pixel_animationControl = AnimationControl_ForwardOne;
		break;
	case 2: // Forward
		Pixel_animationControl = AnimationControl_Forward;
		break;
	case 3: // Stop (clears all animations)
		Pixel_animationControl = AnimationControl_Stop;
		break;
	case 4: // Reset (restarts animations)
		Pixel_animationControl = AnimationControl_Reset;
		break;
	case 5: // Pauses animations and clears display
		Pixel_animationControl = AnimationControl_WipePause;
		break;
	case 6: // Pauses animation
		Pixel_animationControl = AnimationControl_Pause;
		break;
	}
}



// ----- Functions -----

// -- Debug Functions --

// Debug info for PixelElement
void Pixel_showPixelElement( PixelElement *elem )
{
	print("W:");
	printInt8( elem->width );
	print(" C:");
	printInt8( elem->channels );
	print(" I:");
	printInt16( elem->indices[0] );
	for ( uint8_t c = 1; c < elem->channels; c++ )
	{
		print(",");
		printInt16( elem->indices[c] );

	}
}


// -- Utility Functions --

// TODO Support non-8bit channels
uint8_t Pixel_8bitInterpolation( uint8_t start, uint8_t end, uint8_t dist )
{
	return (start * (256 - dist) + end * dist) >> 8;
}

void Pixel_pixelInterpolate( PixelElement *elem, uint8_t position, uint8_t intensity )
{
	// Toggle each of the channels of the pixel

	for ( uint8_t ch = 0; ch < elem->channels; ch++ )
	{
		uint16_t ch_pos = elem->indices[ch];
		PixelBuf *pixbuf = Pixel_bufferMap( ch_pos );
		PixelBuf16( pixbuf, ch_pos ) = Pixel_8bitInterpolation( 0, intensity, position * (ch + 1) );
	}
}



// -- Animation Stack --

// Locates animation memory slot using default settings for the animation
// Initiates animation to process on the next cycle
// Returns 1 on success, 0 on failure to allocate
uint8_t Pixel_addDefaultAnimation( uint32_t index )
{
	if ( index >= Pixel_AnimationSettingsNum_KLL )
	{
		warn_msg("Invalid AnimationSetting index: ");
		printInt32( index );
		print( NL );
		return 0;
	}

	return Pixel_addAnimation( (AnimationStackElement*)&Pixel_AnimationSettings[ index ] );
}

// Allocates animaton memory slot
// Initiates animation to process on the next cycle
// Returns 1 on success, 0 on failure to allocate
uint8_t Pixel_addAnimation( AnimationStackElement *element )
{
	if ( element->replace )
	{
		AnimationStackElement *found = Pixel_lookupAnimation( element->index, 0 );

		// If found, modify stack element
		if ( found != NULL && ( found->trigger == element->trigger || element->replace == AnimationReplaceType_All ) )
		{
			found->pos = element->pos;
			found->subpos = element->subpos;
			found->loops = element->loops;
			found->pfunc = element->pfunc;
			found->ffunc = element->ffunc;
			found->framedelay = element->framedelay;
			found->frameoption = element->frameoption;
			found->replace = element->replace;
			found->state = element->state;
			return 0;
		}
	}

	// Make sure there is room left on the stack
	if ( Pixel_AnimationStack.size >= Pixel_AnimationStackSize )
	{
		warn_print("Animation stack is full...");
		return 0;
	}

	// Add to animation stack
	// Processing is done from bottom to top of the stack
	// First find a free memory slot, this is not ordered, so we have to search for it
	uint16_t pos = 0;
	for ( ; pos < Pixel_AnimationStackSize; pos++ )
	{
		// Check if memory is unused
		if ( Pixel_AnimationElement_Stor[pos].index == 0xFFFF )
		{
			break;
		}
	}

	// No memory left
	// XXX This shouldn't happen
	if ( pos >= Pixel_AnimationStackSize )
	{
		erro_print("Animation Stack memory leak...this is a bug!");
		return 0;
	}

	// Set stack location
	Pixel_AnimationStack.stack[Pixel_AnimationStack.size++] = &Pixel_AnimationElement_Stor[pos];

	// Copy animation settings
	memcpy( &Pixel_AnimationElement_Stor[pos], element, sizeof(AnimationStackElement) );

	return 1;
}

// Removes the first index of an animation
// Will be popped from the stack on the next animation processing loop
uint8_t Pixel_delAnimation( uint16_t index, uint8_t finish )
{
	// Find animation by index, look for the *first* one
	uint16_t pos = 0;
	for ( ; pos < Pixel_AnimationStackSize; pos++ )
	{
		// Check if memory is unused
		if ( Pixel_AnimationElement_Stor[pos].index == index )
		{
			// Let animation finish it's last frame
			if ( finish )
			{
				Pixel_AnimationElement_Stor[pos].loops = 1;
			}
			else
			{
				Pixel_AnimationElement_Stor[pos].index = 0xFFFF;
			}
			return 1;
		}
	}

	return 0;
}

// Cleans/resets animation stack. Removes all running animations.
// NOTE: Does not clear the output buffer
void Pixel_clearAnimations()
{
	// Set stack size to 0
	Pixel_AnimationStack.size = 0;

	// Set indices to max value to indicate un-allocated
	for ( uint16_t pos = 0; pos < Pixel_AnimationStackSize; pos++ )
	{
		Pixel_AnimationElement_Stor[pos].index = 0xFFFF;
	}
}

// Clears all pixels
void Pixel_clearPixels()
{
	// Update all positions
	for ( uint16_t px = 0; px < Pixel_TotalPixels_KLL; px++ )
	{
		// Unset pixel
		Pixel_pixelSet( (PixelElement*)&Pixel_Mapping[ px ], 0 );
	}
}

// Animation ID lookup
// - Does lookup from bottom to top
// - Set previous element to look for the next one
// - Set prev to 0 to start from the beginning
// - Returns NULL/0 if not found
AnimationStackElement *Pixel_lookupAnimation( uint16_t index, uint16_t prev )
{
	uint16_t pos = prev;

	// Look for next instance of index
	for ( ; pos < Pixel_AnimationStack.size; pos++ )
	{
		// Check if index matches
		if ( Pixel_AnimationStack.stack[pos]->index == index )
		{
			return Pixel_AnimationStack.stack[pos];
		}
	}

	// Could not find, return NULL
	return NULL;
}

// Iterate over each element in the Animation Stack
// If the animation is complete, do not re-add to the stack
// - The stack is re-built each time.
// - Ignores any stack element indices set to 0xFFFF/-1 (max)
void Pixel_stackProcess()
{
	uint16_t pos = 0;
	uint16_t size = Pixel_AnimationStack.size;

	// We reset the stack size, and rebuild the stack on the fly
	Pixel_AnimationStack.size = 0;

	// Process each element of the stack
	for ( ; pos < size; pos++ )
	{
		// Lookup animation stack element
		AnimationStackElement *elem = Pixel_AnimationStack.stack[pos];

		// Ignore animation if index is 0xFFFF (max)
		if ( elem->index == 0xFFFF )
		{
			continue;
		}

		// Process animation element
		if ( Pixel_animationProcess( elem ) )
		{
			// Re-add animation to stack
			Pixel_AnimationStack.stack[Pixel_AnimationStack.size++] = elem;
		}
	}
}



// -- Pixel Control --

// PixelBuf lookup
// - Determines which buffer a channel resides in
PixelBuf *Pixel_bufferMap( uint16_t channel )
{
	// TODO Generate based on keyboard
#if ISSI_Chip_31FL3731_define == 1 || ISSI_Chip_31FL3732_define == 1
	if      ( channel < 144 ) return &Pixel_Buffers[0];
	else if ( channel < 288 ) return &Pixel_Buffers[1];
	else if ( channel < 432 ) return &Pixel_Buffers[2];
	else if ( channel < 576 ) return &Pixel_Buffers[3];
#elif ISSI_Chip_31FL3733_define == 1
	if      ( channel < 192 ) return &Pixel_Buffers[0];
	else if ( channel < 384 ) return &Pixel_Buffers[1];
	else if ( channel < 576 ) return &Pixel_Buffers[2];
#else
	if      ( channel < 192 ) return &Pixel_Buffers[0];
	else if ( channel < 384 ) return &Pixel_Buffers[1];
	else if ( channel < 576 ) return &Pixel_Buffers[2];
#endif

	// Invalid channel, return first channel and display error
	erro_msg("Invalid channel: ");
	printHex( channel );
	print( NL );
	return 0;
}

#define PixelChange_Expansion(pixbuf, ch_pos, mod_value, op) \
	/* Lookup buffer to data width mapping */ \
	switch ( pixbuf->width ) \
	{ \
	case 8:  /*  8 bit mapping */ \
		PixelBuf8( pixbuf, ch_pos ) op (uint8_t)mod_value; break; \
	case 16: /* 16 bit mapping */ \
		PixelBuf16( pixbuf, ch_pos ) op (uint16_t)mod_value; break; \
	case 32: /* 32 bit mapping */ \
		PixelBuf32( pixbuf, ch_pos ) op (uint32_t)mod_value; break; \
	default: \
		warn_print("Invalid width mapping for "#op ); \
		break; \
	}

// Pixel Evaluation
// - Iterates over each of the Pixel channels and applies modifications
void Pixel_pixelEvaluation( PixelModElement *mod, PixelElement *elem )
{
	// Ignore if no element
	if ( elem == 0 )
	{
		return;
	}

	// Lookup number of channels in pixel
	uint8_t channels = elem->channels;

	// Data position iterator
	uint8_t position_iter = 0;

	// Apply operation to each channel of the pixel
	for ( uint8_t ch = 0; ch < channels; ch++ )
	{
		// Lookup channel position
		uint16_t ch_pos = elem->indices[ch];

		// Determine which buffer we are in
		PixelBuf *pixbuf = Pixel_bufferMap( ch_pos );

		// Invalid channel, stop
		if ( pixbuf == 0 )
		{
			break;
		}

		// Change Type (first 8 bits of each channel of data, see pixel.h for layout)
		PixelChange change = (PixelChange)mod->data[ position_iter++ ];

		// Modification Value
		uint32_t mod_value = 0;

		// Lookup modification value
		switch ( elem->width )
		{
		case 8:
			mod_value = mod->data[ position_iter++ ];
			break;

		case 16:
			mod_value = mod->data[ position_iter + 1 ] |
				( mod->data[ position_iter + 2 ] << 8 );
			position_iter += 2;
			break;

		case 32:
			mod_value = mod->data[ position_iter + 1 ] |
				( mod->data[ position_iter + 2 ] << 8 ) |
				( mod->data[ position_iter + 3 ] << 16 ) |
				( mod->data[ position_iter + 4 ] << 24 );
			position_iter += 4;
			break;

		default:
			warn_print("Invalid PixelElement width mapping");
			break;
		}

		// Operation
		switch ( change )
		{
		case PixelChange_Set:             // =
			PixelChange_Expansion( pixbuf, ch_pos, mod_value, = );
			break;

		case PixelChange_Add:             // +
			PixelChange_Expansion( pixbuf, ch_pos, mod_value, += );
			break;

		case PixelChange_Subtract:        // -
			PixelChange_Expansion( pixbuf, ch_pos, mod_value, -= );
			break;

		case PixelChange_LeftShift:       // <<
			PixelChange_Expansion( pixbuf, ch_pos, mod_value, <<= );
			break;

		case PixelChange_RightShift:      // >>
			PixelChange_Expansion( pixbuf, ch_pos, mod_value, >>= );
			break;

		case PixelChange_NoRoll_Add:      // +:
			// Lookup buffer to data width mapping
			switch ( pixbuf->width )
			{
			case 8:  //  8  bit mapping
			{
				uint8_t prev = PixelBuf8( pixbuf, ch_pos );
				PixelBuf8( pixbuf, ch_pos ) += (uint8_t)mod_value;
				if ( prev > PixelBuf8( pixbuf, ch_pos ) )
					PixelBuf8( pixbuf, ch_pos ) = 0xFF;
				break;
			}
			case 16: // 16  bit mapping
			{
				// TODO Fix for 16 on 8 bit (i.e. early K-Type)
				//uint16_t prev = PixelBuf16( pixbuf, ch_pos );
				PixelBuf16( pixbuf, ch_pos ) += (uint16_t)mod_value;
				/*
				if ( prev > PixelBuf16( pixbuf, ch_pos ) )
					PixelBuf16( pixbuf, ch_pos ) = 0xFFFF;
				*/
				if ( 0xFF < PixelBuf16( pixbuf, ch_pos ) )
					PixelBuf16( pixbuf, ch_pos ) = 0xFF;
				break;
			}
			case 32: // 32  bit mapping
			{
				uint32_t prev = PixelBuf32( pixbuf, ch_pos );
				PixelBuf32( pixbuf, ch_pos ) += (uint32_t)mod_value;
				if ( prev > PixelBuf32( pixbuf, ch_pos ) )
					PixelBuf32( pixbuf, ch_pos ) = 0xFFFFFFFF;
				break;
			}

			default:
				warn_print("Invalid width mapping on set");
				break;
			}
			break;

		case PixelChange_NoRoll_Subtract: // -:
			// Lookup buffer to data width mapping
			switch ( pixbuf->width )
			{
			case 8:  //  8  bit mapping
			{
				uint8_t prev = PixelBuf8( pixbuf, ch_pos );
				PixelBuf8( pixbuf, ch_pos ) -= (uint8_t)mod_value;
				if ( prev < PixelBuf8( pixbuf, ch_pos ) )
					PixelBuf8( pixbuf, ch_pos ) = 0;
				break;
			}
			case 16: // 16  bit mapping
			{
				uint16_t prev = PixelBuf16( pixbuf, ch_pos );
				PixelBuf16( pixbuf, ch_pos ) -= (uint16_t)mod_value;
				if ( prev < PixelBuf16( pixbuf, ch_pos ) )
					PixelBuf16( pixbuf, ch_pos ) = 0;
				break;
			}
			case 32: // 32  bit mapping
			{
				uint32_t prev = PixelBuf32( pixbuf, ch_pos );
				PixelBuf32( pixbuf, ch_pos ) -= (uint32_t)mod_value;
				if ( prev < PixelBuf32( pixbuf, ch_pos ) )
					PixelBuf32( pixbuf, ch_pos ) = 0;
				break;
			}

			default:
				warn_print("Invalid width mapping on set");
				break;
			}
			break;

		default:
			warn_print("Unimplemented pixel modifier");
			break;
		}
	}
}



// -- Fill Algorithms --

// Fill Algorithm Pixel Lookup
// - **elem stores a pointer to the PixelElement which can be used to lookup the channel buffer location
// - Determines which pixel element to work on next
// - If type:index has more than one pixel, non-0 is returned
// - The return value signifies the next value to set the prev argument
// - Once the function returns 0, all pixels have been processed
uint16_t Pixel_fillPixelLookup(
	PixelModElement *mod,
	PixelElement **elem,
	uint16_t prev,
	AnimationStackElement *stack_elem,
	uint16_t *valid
)
{
	// Used to determine next pixel in column or row (fill)
	uint16_t cur = prev;
	uint16_t index = 0;

	// Assume invalid (i.e. do not evaluate pixel unless we are sure it's valid)
	*valid = 0;

	// Default to nothing found
	*elem = 0;

	// Lookup fill algorith
	switch ( mod->type )
	{
	case PixelAddressType_Index:
		// Lookup pixel by absolute index
		*elem = (PixelElement*)&Pixel_Mapping[mod->index] - 1;
		if ( mod->index <= Pixel_TotalPixels_KLL )
		{
			*valid = 1;
		}
		break;

	case PixelAddressType_Rect:
		// Make sure row,column exists
		if ( mod->rect.col >= Pixel_DisplayMapping_Cols_KLL
			&& mod->rect.row >= Pixel_DisplayMapping_Rows_KLL )
		{
			erro_msg("Invalid row,column index: ");
			printInt16( mod->rect.row );
			print(",");
			printInt16( mod->rect.col );
			print( NL );
			break;
		}

		// Lookup pixel in rectangle display organization
		*elem = (PixelElement*)&Pixel_Mapping[
			Pixel_DisplayMapping[
				mod->rect.row * Pixel_DisplayMapping_Cols_KLL + mod->rect.col
			] - 1
		];
		*valid = 1;
		break;

	case PixelAddressType_ColumnFill:
		// Lookup pixel until either, non-0 index or we reached the last row
		do {
			// Pixel index
			index = Pixel_DisplayMapping[ cur * Pixel_DisplayMapping_Cols_KLL + mod->rect.col ];

			// Check if we've processed all rows
			if ( cur >= Pixel_DisplayMapping_Rows_KLL )
			{
				return 0;
			}
			cur++;
		} while ( index == 0 );

		// Validate index is actually a valid evaluation
		if ( index <= Pixel_TotalPixels_KLL )
		{
			*valid = 1;
		}

		// Lookup pixel, pixels are 1 indexed, hence the -1
		*elem = (PixelElement*)&Pixel_Mapping[ index - 1 ];
		return cur;

	case PixelAddressType_RowFill:
		// Lookup pixel until either, non-0 index or we reached the last column
		do {
			// Pixel index
			index = Pixel_DisplayMapping[ mod->rect.row * Pixel_DisplayMapping_Cols_KLL + cur ];

			// Check if we've processed all rows
			if ( cur >= Pixel_DisplayMapping_Cols_KLL )
			{
				return 0;
			}
			cur++;
		} while ( index == 0 );

		// Validate index is actually a valid evaluation
		if ( index <= Pixel_TotalPixels_KLL )
		{
			*valid = 1;
		}

		// Lookup pixel, pixels are 1 indexed, hence the -1
		*elem = (PixelElement*)&Pixel_Mapping[ index - 1 ];
		return cur;

	case PixelAddressType_ScanCode:
		// Make sure ScanCode exists
		if ( mod->index > MaxScanCode_KLL )
		{
			erro_msg("Invalid ScanCode: ");
			printInt16( mod->index );
			print( NL );
			break;
		}
		*valid = 1;

		// Lookup ScanCode - Indices are 1-indexed in both arrays (hence the -1)
		uint16_t pixel = Pixel_ScanCodeToPixel[ mod->index - 1 ];
		*elem = (PixelElement*)&Pixel_Mapping[ pixel - 1 ];
		break;

	case PixelAddressType_RelativeIndex:
		// TODO
		break;

	case PixelAddressType_RelativeRect:
	{
		// Determine scancode to be relative from
		uint8_t scan_code = Pixel_determineLastTriggerScanCode( stack_elem->trigger );

		// Lookup display position of scancode
		uint16_t position = Pixel_ScanCodeToDisplay[ scan_code - 1 ];

		// Calculate rectangle offset
		position += (int16_t)mod->rect.row * Pixel_DisplayMapping_Cols_KLL + (int16_t)mod->rect.col;

		// Make sure position exists
		if ( position >= Pixel_DisplayMapping_Cols_KLL * Pixel_DisplayMapping_Rows_KLL )
		{
			break;
		}

		// Lookup pixel, pixels are 1 indexed, hence the -1
		index = Pixel_DisplayMapping[ position ];

		// Validate index is actually a valid evaluation
		if ( index <= Pixel_TotalPixels_KLL && index != 0 )
		{
			*valid = 1;
			*elem = (PixelElement*)&Pixel_Mapping[ index - 1 ];
		}
		break;
	}
	case PixelAddressType_RelativeColumnFill:
	{
		// Determine scancode to be relative from
		uint8_t scan_code = Pixel_determineLastTriggerScanCode( stack_elem->trigger );

		// Lookup display position of scancode
		uint16_t position = Pixel_ScanCodeToDisplay[ scan_code - 1 ];

		// Calculate rectangle offset
		position += (int16_t)mod->rect.col;

		// Make sure column exists
		if ( position >= Pixel_DisplayMapping_Cols_KLL * Pixel_DisplayMapping_Rows_KLL )
		{
			erro_msg("Invalid position index (relcol): ");
			printInt16( position );
			print( NL );
			break;
		}

		// Determine first row in column
		position %= Pixel_DisplayMapping_Cols_KLL;

		// Lookup pixel until either, non-0 index or we reached the last row
		do {
			// Current position
			uint16_t curpos = cur++ * Pixel_DisplayMapping_Cols_KLL + position;

			// Check if we've gone too far (and finished all rows)
			if ( curpos >= Pixel_DisplayMapping_Cols_KLL * Pixel_DisplayMapping_Rows_KLL )
			{
				return 0;
			}

			// Pixel index
			index = Pixel_DisplayMapping[ curpos ];
		} while ( index == 0 );

		// Validate index is actually a valid evaluation
		if ( index <= Pixel_TotalPixels_KLL )
		{
			*valid = 1;
		}

		// Lookup pixel, pixels are 1 indexed, hence the -1
		*elem = (PixelElement*)&Pixel_Mapping[ index - 1 ];
		return cur;
	}
	case PixelAddressType_RelativeRowFill:
	{
		// Determine scancode to be relative from
		uint8_t scan_code = Pixel_determineLastTriggerScanCode( stack_elem->trigger );

		// Lookup display position of scancode
		uint16_t position = Pixel_ScanCodeToDisplay[ scan_code - 1 ];

		// Calculate rectangle offset
		position += (int16_t)mod->rect.row * Pixel_DisplayMapping_Rows_KLL;

		// Make sure column exists
		if ( position >= Pixel_DisplayMapping_Cols_KLL * Pixel_DisplayMapping_Rows_KLL )
		{
			erro_msg("Invalid position index (relrow): ");
			printInt16( position );
			print( NL );
			break;
		}

		// Determine which row we are in
		position /= Pixel_DisplayMapping_Cols_KLL;

		// Lookup pixel until either, non-0 index or we reached the last row
		do {
			// Current position
			uint16_t curpos = cur++ + Pixel_DisplayMapping_Cols_KLL * position;

			// Check if we've gone too far (and finished all rows)
			if ( cur >= Pixel_DisplayMapping_Cols_KLL )
			{
				return 0;
			}

			// Pixel index
			index = Pixel_DisplayMapping[ curpos ];
		} while ( index == 0 );

		// Validate index is actually a valid evaluation
		if ( index <= Pixel_TotalPixels_KLL )
		{
			*valid = 1;
		}

		// Lookup pixel, pixels are 1 indexed, hence the -1
		*elem = (PixelElement*)&Pixel_Mapping[ index - 1 ];
		return cur;
	}
	// Skip
	default:
		break;
	}

	return 0;
}



// -- Pixel Tweening --

uint16_t Pixel_pixelTweenNextPos( PixelElement *elem, PixelElement *prev )
{
	// No elem found
	// XXX (HaaTa) This is actually a hard problem for relative animations
	//             We may not find any element initially, so we don't know anothing to increment the position
	//             The best solution may be to just find a relatively nearby pixel and use that info...
	//             Or waste enough more flash...

	uint16_t ret = 0;

#if Pixel_HardCode_ChanWidth_define != 0 && Pixel_HardCode_Channels_define != 0
	// More efficient tweening, as we know the number of channels and width at compile time in all cases.
	ret = (
		( Pixel_HardCode_ChanWidth_define / 8 + sizeof( PixelChange ) )
		* Pixel_HardCode_Channels_define
	) + sizeof( PixelModElement );

#else
	// Determine tweening using nearby pixel definitions
	// First try the next element
	if ( elem != 0 )
	{
		ret = ( ( elem->width / 8 + sizeof( PixelChange ) ) * elem->channels ) + sizeof( PixelModElement );
		return ret;
	}

	// Next try the previous element
	if ( prev != 0 )
	{
		ret = ( ( prev->width / 8 + sizeof( PixelChange ) ) * prev->channels ) + sizeof( PixelModElement );
		return ret;
	}

	// BAD BAD BAD
	// TODO - This is BAD, will break in most cases, except for K-Type like keyboards.
	erro_print("Pixel Tween Bug!");
	ret = ( ( 8 / 8 + sizeof( PixelChange ) ) * 3 ) + sizeof( PixelModElement );
#endif

	return ret;
}

// Standard Pixel Pixel Function (standard lookup, no additonal processing)
void Pixel_pixelTweenStandard( const uint8_t *frame, AnimationStackElement *stack_elem )
{
	// Iterate over all of the Pixel Modifier elements of the Animation Frame
	uint16_t pos = 0;
	PixelModElement *mod = (PixelModElement*)&frame[pos];
	while ( mod->type != PixelAddressType_End )
	{
		// Lookup type of pixel, choose fill algorith and query all sub-pixels
		uint16_t next = 0;
		uint16_t valid = 0;
		PixelElement *prev_pixel_elem = 0;
		PixelElement *elem = 0;
		do {
			// Last element
			prev_pixel_elem = elem;

			// Lookup pixel, and check if there are any more pixels left
			next = Pixel_fillPixelLookup( mod, &elem, next, stack_elem, &valid );

			// Apply operation to pixel
			Pixel_pixelEvaluation( mod, elem );
		} while ( next );

		// Determine next position
		pos += Pixel_pixelTweenNextPos( elem, prev_pixel_elem );

		// Lookup next mod element
		mod = (PixelModElement*)&frame[pos];
	}
}

// Basic interpolation Pixel Pixel Function
// TODO - Only works with Colummn and Row fill currently
void Pixel_pixelTweenInterpolation( const uint8_t *frame, AnimationStackElement *stack_elem )
{
	// Iterate over all of the Pixel Modifier elements of the Animation Frame
	uint16_t pos = 0;
	PixelModElement *prev = 0;
	PixelModElement *mod = (PixelModElement*)&frame[pos];
	while ( mod->type != PixelAddressType_End )
	{
		// By default, no interpolation
		int32_t start = mod->index;
		int32_t end = mod->index;

		// Calculate interpolation position
		// TODO Depends on addressing type
		// TODO Work with dis-similar PixelModElement address types
		switch ( mod->type )
		{
		case PixelAddressType_ColumnFill:
			// If this is the first pixel, just set start at the same spot
			start = prev != 0 ? prev->rect.col : mod->rect.col;
			end = mod->rect.col;
			break;

		case PixelAddressType_RowFill:
			// If this is the first pixel, just set start at the same spot
			start = prev != 0 ? prev->rect.row : mod->rect.row;
			end = mod->rect.row;
			break;

		case PixelAddressType_Rect:
			// TODO - This is not correct (just a quick and dirty hack)
			if ( prev != 0 )
			{
				// TODO - Diagonal interpolation?
				if ( prev->rect.col != mod->rect.col )
				{
					start = prev->rect.col;
				}
				if ( prev->rect.row != mod->rect.row )
				{
					start = prev->rect.row;
				}
			}
			else
			{
				start = mod->rect.col;
			}
			//end = ( mod->rect.col + mod->rect.row ) / 2;
			end = mod->rect.col;
			break;

		case PixelAddressType_ScanCode:
			// If this is the first pixel, just set start at the same spot
			start = prev != 0 ? prev->index : mod->index;
			end = mod->index;
			break;

		case PixelAddressType_Index:
			// If this is the first pixel, just set start at the same spot
			start = prev != 0 ? prev->index : mod->index;
			end = mod->index;
			break;

		default:
			break;
		}

		// Lookup prev and mod PixelElements
		PixelElement *prev_elem = 0;
		uint16_t valid = 0;
		if ( prev != 0 )
		{
			Pixel_fillPixelLookup( prev, &prev_elem, 0, stack_elem, &valid );
		}
		PixelElement *mod_elem = 0;
		Pixel_fillPixelLookup( mod, &mod_elem, 0, stack_elem, &valid );

		// Make sure mod_elem is pointing to something, if not, this could be a blank
		// In which case continue to the next definition
		if ( mod_elem == 0 )
		{
			goto next;
		}
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
		PixelElement *prev_pixel_elem = 0;
#ifndef __clang__
#pragma GCC diagnostic pop
#endif
		PixelElement *elem = 0;

		// Prepare tweened PixelModElement
		// TODO allow for larger than 24-bit pixels (auto-generate?)
		uint8_t interp_data[ sizeof( PixelModElement ) + sizeof( PixelModDataElement ) * 3 + 4 * 3 ];
		PixelModElement *interp_mod = (PixelModElement*)&interp_data;
		memcpy( interp_mod, mod, sizeof( interp_data ) );

		// Calculate slice mulitplier size
		// TODO Non-8bit
		// XXX Division...
		uint16_t slice = prev != 0 ? 256 / (end - start + 1) : 0;

		// Iterate over tween-pixels
		for ( int32_t cur = 0; cur < end - start + 1; cur++ )
		{
			// Determine where the tween pixel is
			switch ( mod->type )
			{
			case PixelAddressType_ColumnFill:
				interp_mod->rect.col = start + cur;
				// Ignore invalid columns
				if ( interp_mod->rect.col >= Pixel_DisplayMapping_Cols_KLL || interp_mod->rect.col < 0 )
				{
					continue;
				}
				break;

			case PixelAddressType_RowFill:
				interp_mod->rect.row = start + cur;
				// Ignore invalid rows
				if ( interp_mod->rect.row >= Pixel_DisplayMapping_Rows_KLL || interp_mod->rect.row < 0 )
				{
					continue;
				}
				break;

			case PixelAddressType_Rect:
				// TODO - This is not correct (just a quick and dirty hack)
				interp_mod->rect.col = 0;
				interp_mod->rect.row = 0;
				if ( prev != 0 )
				{
					interp_mod->rect.col = prev->rect.col + cur;
					interp_mod->rect.row = 0;
					//interp_mod->rect.row = prev->rect.row + cur;
				}
				break;

			case PixelAddressType_ScanCode:
				interp_mod->index = start + cur;
				// TODO Ignore unused ScanCodes
				break;

			case PixelAddressType_Index:
				interp_mod->index = start + cur;
				// TODO Ignore unused Indices
				break;

			default:
				break;
			}

			// Calculate interpolation pixel value
			// Uses prev to current PixelMods as the base
			// TODO Non-8bit
			if ( prev != 0 )
			{
				int32_t distance = slice * cur;
				for ( uint8_t ch = 0; ch < mod_elem->channels; ch++ )
				{
					uint8_t pos = ch * 2 + 1; // TODO Only works with 8 bit channels
					interp_mod->data[pos] = Pixel_8bitInterpolation(
						prev->data[pos],
						mod->data[pos],
						distance
					);
				}
			}

			// Lookup type of pixel, choose fill algorith and query all sub-pixels
			uint16_t next = 0;
			uint16_t valid = 0;
			do {
				// Previous element
				prev_pixel_elem = elem;

				// Lookup pixel, and check if there are any more pixels left
				next = Pixel_fillPixelLookup( interp_mod, &elem, next, stack_elem, &valid );

				// Apply operation to pixel if at a valid location
				if ( valid )
				{
					Pixel_pixelEvaluation( interp_mod, elem );
				}
			} while ( next );
		}

next:
		// This may have been a valid frame in an invalid position
		// Store it so we can still use it for interpolation purposes
		prev = mod;

		// Determine next position
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
		pos += Pixel_pixelTweenNextPos( elem, prev_pixel_elem );
#ifndef __clang__
#pragma GCC diagnostic pop
#endif

		// Lookup next mod element
		mod = (PixelModElement*)&frame[pos];
	}
}



// -- Frame Tweening --

// Standard Pixel Frame Function (no additional processing)
void Pixel_frameTweenStandard( const uint8_t *data, AnimationStackElement *elem )
{
	// Do nothing during sub-frames, skip
	if ( elem->subpos != 0 )
	{
		// But only if frame strech isn't set
		if ( !( elem->frameoption & PixelFrameOption_FrameStretch ) )
		{
			return;
		}
	}

	// Lookup Pixel Tweening Function
	switch ( elem->pfunc )
	{
	case PixelPixelFunction_PointInterpolation:
		Pixel_pixelTweenInterpolation( data, elem );
		break;

	// Generic, no addition processing necessary
	case PixelPixelFunction_Off:
	case PixelPixelFunction_PointInterpolationKLL:
		Pixel_pixelTweenStandard( data, elem );
		break;
	}
}

// Pixel Frame Interpolation Tweening
// Do averaging between key frames
void Pixel_frameTweenInterpolation( const uint8_t *data, AnimationStackElement *elem )
{
	// TODO
}



// -- Animation Control --

// Process the animation stack element
// - Returns 1 if the animation should be re-added to the stack
// - Returns 0 if the animation is finished (clean-up memory)
uint8_t Pixel_animationProcess( AnimationStackElement *elem )
{
	// Skip if index has been invalidate (unset)
	if ( elem->index == 0xFFFF )
	{
		return 0;
	}

	// Check the play state
	switch ( elem->state )
	{
	// Pause animation (paused animations will take up animation stack memory)
	case AnimationPlayState_Pause:
		return 1;

	// Stopping animation (frees animation from stack memory)
	case AnimationPlayState_Stop:
		// Indicate animation slot is free
		elem->index = 0xFFFF;
		return 0;

	// Do nothing
	case AnimationPlayState_Start:
	default:
		break;
	}

	// Lookup animation frame to make sure we have something to do
	// TODO Make sure animation index exists -HaaTa
	const uint8_t *data = Pixel_Animations[elem->index][elem->pos];

	// If there is no frame data, that means we either stop, or restart
	if ( data == 0 )
	{
		// Check if we still have more loops, one signifies stop, 0 is infinite
		if ( elem->loops == 0 || elem->loops-- > 1 )
		{
			elem->pos = 0;
			return Pixel_animationProcess( elem );
		}
		// Stop animation
		else
		{
			// Indicate animation slot is free
			elem->index = 0xFFFF;
			return 0;
		}
	}

	// Lookup Frame Tweening Function
	switch ( elem->ffunc )
	{
	case PixelFrameFunction_Interpolation:
		Pixel_frameTweenInterpolation( data, elem );
		break;

	// Generic, no additonal processing necessary
	case PixelFrameFunction_Off:
	case PixelFrameFunction_InterpolationKLL:
		Pixel_frameTweenStandard( data, elem );
		break;
	}

	// Increment positions
	// framedelay case
	if ( elem->framedelay > 0 )
	{
		// Roll-over subpos for framedelay
		if ( elem->subpos == elem->framedelay )
		{
			elem->subpos = 0;
			elem->pos++;
		}
		// Increment subposition
		else
		{
			elem->subpos++;
		}
	}
	// Full-speed
	else
	{
		elem->pos++;
	}

	return 1;
}



// -- Pixel Control --

// Debug function, used by cli only XXX
void Pixel_channelSet( uint16_t channel, uint32_t value )
{
	// Determine which buffer we are in
	PixelBuf *pixbuf = Pixel_bufferMap( channel );

	// Toggle channel accordingly
	switch ( pixbuf->width )
	{
	// Invalid width, default to 8
	default:
		warn_msg("ChanSet Unknown width: ");
		printInt8( pixbuf->width );
		print(" Ch: ");
		printHex( channel );
		print( NL );
		// Falls through on purpose

	// 0bit width - ignore/blank
	case 0:
		break;

	// 8bit width
	case 8:
		PixelBuf8( pixbuf, channel ) = (uint8_t)value;
		break;

	// 16bit width
	case 16:
		PixelBuf16( pixbuf, channel ) = (uint16_t)value;
		break;
	}
}

// Toggle the given channel
// Debug function, used by cli only XXX
void Pixel_channelToggle( uint16_t channel )
{
	// Determine which buffer we are in
	PixelBuf *pixbuf = Pixel_bufferMap( channel );

	// Toggle channel accordingly
	switch ( pixbuf->width )
	{
	// Invalid width, default to 8
	default:
		warn_msg("ChanToggle Unknown width: ");
		printInt8( pixbuf->width );
		print(" Ch: ");
		printHex( channel );
		print( NL );
		// Falls through on purpose

	// 0bit width - ignore/blank
	case 0:
		break;

	// 8bit width
	case 8:
		PixelBuf8( pixbuf, channel ) ^= 128;
		break;

	// 16bit width
	case 16:
		PixelBuf16( pixbuf, channel ) ^= 128;
		break;
	}
}

// Set each of the channels to a specific value
void Pixel_pixelSet( PixelElement *elem, uint32_t value )
{
	// Set each of the channels of the pixel
	for ( uint8_t ch = 0; ch < elem->channels; ch++ )
	{
		Pixel_channelSet( elem->indices[ch], value );
	}
}

// Toggle given pixel element
// Debug function, used by cli only XXX
void Pixel_pixelToggle( PixelElement *elem )
{
	// Toggle each of the channels of the pixel
	for ( uint8_t ch = 0; ch < elem->channels; ch++ )
	{
		Pixel_channelToggle( elem->indices[ch] );
	}
}



// -- General --

// Looks up the final scancode in a trigger macro
uint8_t Pixel_determineLastTriggerScanCode( TriggerMacro *trigger )
{
	// Ignore (set to zero) if unset
	if ( trigger == 0 )
	{
		return 0;
	}

	// Iterate over each TriggerGuide element
	uint8_t curScanCode = 0;
	for ( var_uint_t pos = 0; ; pos += trigger->guide[ pos ] * TriggerGuideSize + 1 )
	{
		// Length of this combo
		uint8_t comboLength = trigger->guide[ pos ] * TriggerGuideSize;

		// Determine scancode, use first TriggerGuide scanCode
		TriggerGuide *guide = (TriggerGuide*)&trigger->guide[ pos + 1 ];
		curScanCode = guide->scanCode;

		// If this combo has zero length, we are at the end
		if ( trigger->guide[ pos + comboLength + 1 ] == 0 )
		{
			return curScanCode;
		}
	}
}

// Updates animations for USB Lock LEDs
// TODO - Should be generated by KLL
void Pixel_updateUSBLEDs()
{
#if !defined(_host_)
	if ( !USBKeys_LEDs_Changed )
		return;

	/*
	AnimationStackElement element;
	element.trigger = 0;
	element.pos = 0;
	element.subpos = 0;
	element.loops = 0;
	element.pfunc = 0;
	element.ffunc = 0;
	element.framedelay = 1;
	element.frameoption = 0;
	element.replace = AnimationReplaceType_Basic;

	// NumLock
	if ( USBKeys_LEDs & 0x01 )
	{
		print("NumLock On");
		// TODO
	}

	// CapsLock
	// TODO Set index properly for animation
	const uint16_t caps_index = 9;
	if ( USBKeys_LEDs & 0x02 )
	{
		element.index = caps_index;
		Pixel_addAnimation( &element );
	}
	else
	{
		Pixel_delAnimation( caps_index, 1 );
	}

	// ScrollLock
	// TODO Set index properly for animation
	const uint16_t scroll_index = 12;
	if ( USBKeys_LEDs & 0x04 )
	{
		element.index = scroll_index;
		Pixel_addAnimation( &element );
	}
	else
	{
		Pixel_delAnimation( scroll_index, 1 );
	}
	*/

	USBKeys_LEDs_Changed = 0;
#endif
}

// External Animation Control
void Pixel_setAnimationControl( AnimationControl control )
{
	Pixel_animationControl = control;
}

// Starting Animation setup
void Pixel_initializeStartAnimations()
{
	// Iterate over starting animations
	for ( uint32_t index = 0; index < Pixel_AnimationSettingsNum_KLL; index++ )
	{
		// Check if a starting animation
		if ( Pixel_AnimationSettings[ index ].state == AnimationPlayState_Start )
		{
			// Default animations are noted by the TriggerMacro *trigger pointer being set to 1
			if ( (uintptr_t)(Pixel_AnimationSettings[ index ].trigger) == 1 )
			{
				// Start animation
				if ( Pixel_addDefaultAnimation( index ) == 0 )
				{
					warn_msg("Failed to start starting animation index: ");
					printInt32( index );
					print( NL );
				}
			}
		}
	}
}

// Pixel Procesing Loop
inline void Pixel_process()
{
	// Start latency measurement
	Latency_start_time( pixelLatencyResource );

	// Update USB LED Status
	Pixel_updateUSBLEDs();

	// Only update frame when ready
	switch( Pixel_FrameState )
	{
	case FrameState_Update:
	case FrameState_Pause:
		break;
	default:
		goto pixel_process_final;
	}

	// Pause animation if set
	switch ( Pixel_animationControl )
	{
	case AnimationControl_Forward:    // Ok
	case AnimationControl_ForwardOne: // Ok + 1, then stop
		Pixel_FrameState = FrameState_Update;
		break;
	case AnimationControl_Stop:       // Clear animations, then proceed forward
		Pixel_FrameState = FrameState_Update;
		Pixel_clearAnimations();
		Pixel_clearPixels();
		Pixel_animationControl = AnimationControl_Forward;
		break;
	case AnimationControl_Reset:      // Clear animations, then restart initial
		Pixel_FrameState = FrameState_Update;
		Pixel_clearAnimations();
		Pixel_clearPixels();
		Pixel_animationControl = AnimationControl_Forward;
		Pixel_initializeStartAnimations();
		break;
	case AnimationControl_WipePause:  // Pauses animations, clears the display
		Pixel_FrameState = FrameState_Pause;
		Pixel_clearPixels();
		break;
	default: // Pause
		Pixel_FrameState = FrameState_Pause;
		goto pixel_process_final;
	}

	// First check if we are in a test mode
	switch ( Pixel_testMode )
	{
	// Toggle current position, then increment
	case PixelTest_Chan_Roll:
		// Toggle channel
		Pixel_channelToggle( Pixel_testPos );

		// Increment channel
		Pixel_testPos++;
		if ( Pixel_testPos >= Pixel_TotalChannels_KLL )
			Pixel_testPos = 0;

		goto pixel_process_done;

	// Blink all channels
	case PixelTest_Chan_All:
		// Update all positions
		for ( uint16_t ch = 0; ch < Pixel_TotalChannels_KLL; ch++ )
		{
			// Toggle channel
			Pixel_channelToggle( ch );
		}

		goto pixel_process_done;

	// Enable all channels
	case PixelTest_Chan_Full:
		// Update all positions
		for ( uint16_t ch = 0; ch < Pixel_TotalChannels_KLL; ch++ )
		{
			// Toggle channel
			Pixel_channelSet( ch, 255 );
		}

		goto pixel_process_done;

	// Turn off all channels
	case PixelTest_Chan_Off:
		// Update all positions
		for ( uint16_t ch = 0; ch < Pixel_TotalChannels_KLL; ch++ )
		{
			// Toggle channel
			Pixel_channelSet( ch, 0 );
		}

		goto pixel_process_done;

	// Toggle current position, then increment
	case PixelTest_Pixel_Roll:
		// Toggle pixel
		Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ Pixel_testPos ] );

		// Increment pixel
		Pixel_testPos++;
		if ( Pixel_testPos >= Pixel_TotalPixels_KLL )
			Pixel_testPos = 0;

		goto pixel_process_done;

	// Toggle all positions
	case PixelTest_Pixel_All:
		// Update all positions
		for ( uint16_t px = Pixel_testPos; px < Pixel_TotalPixels_KLL; px++ )
		{
			// Toggle pixel
			Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ px ] );
		}

		goto pixel_process_done;

	// Enable all positions
	case PixelTest_Pixel_Full:
		// Update all positions
		for ( uint16_t px = 0; px < Pixel_TotalPixels_KLL; px++ )
		{
			// Toggle pixel
			// XXX (only works for 8 bit atm)
			Pixel_pixelSet( (PixelElement*)&Pixel_Mapping[ px ], 255 );
			Pixel_testMode = PixelTest_Off;
		}

		goto pixel_process_done;

	// Disable all positions
	case PixelTest_Pixel_Off:
		// Update all positions
		for ( uint16_t px = 0; px < Pixel_TotalPixels_KLL; px++ )
		{
			// Unset pixel
			Pixel_pixelSet( (PixelElement*)&Pixel_Mapping[ px ], 0 );
			Pixel_testMode = PixelTest_Off;
		}

		goto pixel_process_done;

	// Toggle current position, then increment
	case PixelTest_Scan_Roll:
	{
		// Lookup pixel
		uint16_t pixel = Pixel_ScanCodeToPixel[ Pixel_testPos ];

		// Increment pixel
		Pixel_testPos++;
		if ( Pixel_testPos >= MaxScanCode_KLL )
			Pixel_testPos = 0;

		// Ignore if pixel set to 0
		if ( pixel == 0 )
		{
			goto pixel_process_final;
		}

		// Toggle channel
		Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ pixel - 1 ] );

		goto pixel_process_done;
	}
	// Toggle all positions
	case PixelTest_Scan_All:
		for ( uint16_t px = Pixel_testPos; px < MaxScanCode_KLL; px++ )
		{
			// Lookup pixel
			uint16_t pixel = Pixel_ScanCodeToPixel[ px ];

			// Ignore if pixel set to 0
			if ( pixel == 0 )
			{
				continue;
			}

			// Toggle pixel
			Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ pixel - 1 ] );
		}

		goto pixel_process_done;

	// Toggle current position, then increment
	case PixelTest_XY_Roll:
	{
		// Lookup pixel
		uint16_t pixel = Pixel_DisplayMapping[ Pixel_testPos ];

		// Increment pixel
		Pixel_testPos++;
		if ( Pixel_testPos >= Pixel_DisplayMapping_Cols_KLL * Pixel_DisplayMapping_Rows_KLL )
			Pixel_testPos = 0;

		// Ignore if pixel set to 0
		if ( pixel == 0 )
		{
			goto pixel_process_final;
		}

		// Toggle channel
		Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ pixel - 1 ] );

		goto pixel_process_done;
	}
	// Toggle all positions
	case PixelTest_XY_All:
		for ( uint16_t px = Pixel_testPos; px < Pixel_DisplayMapping_Cols_KLL * Pixel_DisplayMapping_Rows_KLL; px++ )
		{
			// Lookup pixel
			uint16_t pixel = Pixel_DisplayMapping[ px ];

			// Ignore if pixel set to 0
			if ( pixel == 0 )
			{
				continue;
			}

			// Toggle pixel
			Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ pixel - 1 ] );
		}

		goto pixel_process_done;

	// Otherwise ignore
	default:
		break;
	}

	// Process Animation Stack
	Pixel_stackProcess();

	// Pause if necessary
	switch( Pixel_animationControl )
	{
	case AnimationControl_ForwardOne:
		Pixel_animationControl = AnimationControl_Pause;
		break;
	default:
		break;
	}

pixel_process_done:
	// Frame is now ready to send
	Pixel_FrameState = FrameState_Ready;

pixel_process_final:
	// End latency measurement
	Latency_end_time( pixelLatencyResource );
}


inline void Pixel_setup()
{
	// Register Pixel CLI dictionary
	CLI_registerDictionary( pixelCLIDict, pixelCLIDictName );

	// Set frame state to update
	Pixel_FrameState = FrameState_Update;

	// Disable test modes by default, start at position 0
	Pixel_testMode = Pixel_Test_Mode_define;

	// Clear animation stack
	Pixel_clearAnimations();

	// Set default animation control
	Pixel_animationControl = AnimationControl_Forward;

	// Add initial animations
	Pixel_initializeStartAnimations();

	// Allocate latency resource
	pixelLatencyResource = Latency_add_resource("PixelMap", LatencyOption_Ticks);
}


// ----- CLI Command Functions -----

void cliFunc_pixelList( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Process argument if given
	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Check for special args
	switch ( *arg1Ptr )
	{
	case 'b':
	case 'B':
		info_msg("Buffer List");

		// List all buffers
		for ( uint8_t buf = 0; buf < Pixel_BuffersLen_KLL; buf++ )
		{
			print( NL "\t" );
			printInt8( buf );
			print(":");
			printHex32( (uint32_t)(uintptr_t)(Pixel_Buffers[ buf ].data) );
			print(":width(");
			printInt8( Pixel_Buffers[ buf ].width );
			print("):size(");
			printInt8( Pixel_Buffers[ buf ].size );
			print(")");
		}
		break;

	default:
		info_msg("Pixel List - <num>[<ch1>,...]<width>:...");

		// List all pixels
		for ( uint16_t pixel = 0; pixel < Pixel_TotalPixels_KLL; pixel++ )
		{
			// NL occaisionally
			if ( pixel % 5 == 0 )
				print( NL );

			PixelElement *elem = (PixelElement*)&Pixel_Mapping[ pixel ];

			printHex_op( pixel, 2 );
			print(":");
			printInt8( elem->width );
			print("[");

			// Display each of the channels
			printHex_op( elem->indices[0], 2 );
			for ( uint8_t ch = 1; ch < elem->channels; ch++ )
			{
				print(",");
				printHex_op( elem->indices[ch], 2 );
			}

			print("]");
			print("  ");
		}

		break;
	}
}

void cliFunc_pixelTest( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Process argument if given
	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Check for special args
	switch ( *arg1Ptr )
	{
	case 'a':
	case 'A':
		info_msg("All pixel test");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_Pixel_All;
		return;

	case 'r':
	case 'R':
		info_msg("Pixel roll test");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_Pixel_Roll;
		return;

	case 's':
	case 'S':
		info_msg("Stopping pixel test");
		Pixel_testMode = PixelTest_Off;
		return;

	case 'f':
	case 'F':
		info_msg("Enable all pixels");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_Pixel_Full;
		return;

	case 'o':
	case 'O':
		info_msg("Disable all pixels");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_Pixel_Off;
		return;
	}

	// Check for specific position
	if ( *arg1Ptr != '\0' )
	{
		Pixel_testPos = numToInt( arg1Ptr );
	}

	// Debug info
	print( NL );
	info_msg("Pixel: ");
	printInt16( Pixel_testPos + 1 );
	print(" ");

	// Lookup pixel element
	PixelElement *elem = (PixelElement*)&Pixel_Mapping[ Pixel_testPos ];
	Pixel_showPixelElement( elem );
	print( NL );

	// Toggle channel
	Pixel_pixelToggle( elem );

	// Increment channel
	Pixel_testPos++;
	if ( Pixel_testPos >= Pixel_TotalPixels_KLL )
		Pixel_testPos = 0;
}

void cliFunc_chanTest( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Process argument if given
	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Check for special args
	switch ( *arg1Ptr )
	{
	case 'a':
	case 'A':
		info_msg("All channel test");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_Chan_All;
		break;

	case 'r':
	case 'R':
		info_msg("Channel roll test");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_Chan_Roll;
		break;

	case 's':
	case 'S':
		info_msg("Stopping channel test");
		Pixel_testMode = PixelTest_Off;
		break;

	case 'f':
	case 'F':
		info_msg("Enable all pixels");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_Chan_Full;
		break;

	case 'o':
	case 'O':
		info_msg("Disable all pixels");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_Chan_Off;
		break;
	}

	// Check for specific position
	if ( *arg1Ptr != '\0' )
	{
		Pixel_testPos = numToInt( arg1Ptr );
	}

	// Debug info
	print( NL );
	info_msg("Channel: ");
	printInt16( Pixel_testPos );
	print( NL );

	// Toggle pixel
	Pixel_channelToggle( Pixel_testPos );

	// Increment pixel
	Pixel_testPos++;
	if ( Pixel_testPos >= Pixel_TotalChannels_KLL )
		Pixel_testPos = 0;
}

void cliFunc_pixelSCTest( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Process argument if given
	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Check for special args
	switch ( *arg1Ptr )
	{
	case 'a':
	case 'A':
		info_msg("All scancode pixel test");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_Scan_All;
		break;

	case 'r':
	case 'R':
		info_msg("Scancode pixel roll test");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_Scan_Roll;
		break;

	case 's':
	case 'S':
		info_msg("Stopping scancode pixel test");
		Pixel_testMode = PixelTest_Off;
		break;
	}

	// Check for specific position
	if ( *arg1Ptr != '\0' )
	{
		Pixel_testPos = numToInt( arg1Ptr );
	}

	// Lookup pixel
	uint16_t pixel = Pixel_ScanCodeToPixel[ Pixel_testPos ];

	// Debug info
	print( NL );
	info_msg("ScanCode: ");
	printInt16( Pixel_testPos + 1 );
	print(" Pixel: ");
	printInt16( pixel );
	print(" ");

	// Lookup pixel element
	PixelElement *elem = (PixelElement*)&Pixel_Mapping[ pixel - 1 ];
	Pixel_showPixelElement( elem );
	print( NL );

	// Increment pixel
	Pixel_testPos++;
	if ( Pixel_testPos >= MaxScanCode_KLL )
		Pixel_testPos = 0;

	// Ignore if pixel set to 0
	if ( pixel == 0 )
	{
		return;
	}

	// Toggle pixel
	Pixel_pixelToggle( elem );
}

void cliFunc_pixelXYTest( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Process argument if given
	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Check for special args
	switch ( *arg1Ptr )
	{
	case 'a':
	case 'A':
		info_msg("All x,y pixel test");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_XY_All;
		return;

	case 'r':
	case 'R':
		info_msg("x,y pixel roll test");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_XY_Roll;
		return;

	case 's':
	case 'S':
		info_msg("Stopping x,y pixel test");
		Pixel_testMode = PixelTest_Off;
		return;

	case 'h':
	case 'H':
		// Make sure we aren't too far already
		if ( Pixel_testPos >= Pixel_DisplayMapping_Rows_KLL )
		{
			Pixel_testPos = 0;
		}

		info_msg("Horizontal: ");
		printInt16( Pixel_testPos );

		// Iterate over the row
		for ( uint16_t pos = Pixel_DisplayMapping_Cols_KLL * Pixel_testPos;
			pos < Pixel_DisplayMapping_Cols_KLL * ( Pixel_testPos + 1);
			pos++
		)
		{
			uint16_t pixel = Pixel_DisplayMapping[ pos ];
			print(" ");
			printInt16( pixel );

			// Toggle pixel
			Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ pixel - 1 ] );
		}
		Pixel_testPos++;

		return;

	case 'v':
	case 'V':
		// Make sure we aren't too far already
		if ( Pixel_testPos >= Pixel_DisplayMapping_Cols_KLL )
		{
			Pixel_testPos = 0;
		}

		info_msg("Vertical: ");
		printInt16( Pixel_testPos );

		// Iterate over the column
		for ( uint16_t pos = 0; pos < Pixel_DisplayMapping_Rows_KLL; pos++ )
		{
			uint16_t pos_calc = pos * Pixel_DisplayMapping_Cols_KLL + Pixel_testPos;
			print(" ");
			printInt16( pos_calc );
			uint16_t pixel = Pixel_DisplayMapping[ pos_calc ];
			print(":");
			printInt16( pixel );

			// Toggle pixel
			Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ pixel - 1 ] );
		}
		Pixel_testPos++;

		return;
	}

	// Check for specific position
	if ( *arg1Ptr != '\0' )
	{
		Pixel_testPos = numToInt( arg1Ptr );
	}

	// Lookup pixel
	uint16_t pixel = Pixel_DisplayMapping[ Pixel_testPos ];

	// Debug info
	print( NL );
	info_msg("Position (x,y): ");
	printInt16( Pixel_testPos % Pixel_DisplayMapping_Cols_KLL );
	print(",");
	printInt16( Pixel_testPos / Pixel_DisplayMapping_Cols_KLL );
	print(":");
	printInt16( Pixel_testPos );
	print(" Pixel: ");
	printInt16( pixel );
	print(" ");

	// Lookup pixel element
	PixelElement *elem = (PixelElement*)&Pixel_Mapping[ pixel - 1 ];
	Pixel_showPixelElement( elem );
	print( NL );

	// Increment pixel
	Pixel_testPos++;
	if ( Pixel_testPos >= Pixel_DisplayMapping_Cols_KLL * Pixel_DisplayMapping_Rows_KLL )
		Pixel_testPos = 0;

	// Ignore if pixel set to 0
	if ( pixel == 0 )
	{
		return;
	}

	// Toggle pixel
	Pixel_pixelToggle( elem );
}
void cliFunc_aniAdd( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	// TODO
	/*
	uint16_t index = Pixel_AnimationStack.size;
	Pixel_AnimationStack.stack[index].index = 1;
	Pixel_AnimationStack.stack[index].pos = 1;
	Pixel_AnimationStack.stack[index].loops = 1;
	Pixel_AnimationStack.stack[index].divider = 0;
	Pixel_AnimationStack.stack[index].modifier = AnimationModifier_None;
	Pixel_AnimationStack.size++;
	*/
}

void cliFunc_aniDel( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	// TODO
	Pixel_AnimationStack.size--;
}

void cliFunc_aniStack( char* args )
{
	print(NL);
	info_msg("Stack Size: ");
	printInt16( Pixel_AnimationStack.size );
	for ( uint8_t pos = 0; pos < Pixel_AnimationStack.size; pos++ )
	{
		print(NL);
		AnimationStackElement *elem = Pixel_AnimationStack.stack[pos];
		print(" index(");
		printInt16( elem->index );
		print(") pos(");
		printInt16( elem->pos );
		print(") loops(");
		printInt8( elem->loops );
		print(") framedelay(");
		printInt8( elem->framedelay );
		print(") frameoption(");
		printInt8( elem->frameoption );
		print(") ffunc(");
		printInt8( elem->ffunc );
		print(") pfunc(");
		printInt8( elem->pfunc );
		print(")");
	}
}

void Pixel_dispBuffer()
{
	uint8_t row = 0;
	uint8_t col = 0;
	for ( uint16_t px = 0; px < Pixel_DisplayMapping_Cols_KLL * Pixel_DisplayMapping_Rows_KLL; px++ )
	{
		// Display a + if it's a blank pixel
		if ( Pixel_DisplayMapping[px] == 0 )
		{
			print("+");
		}
		// Lookup pixel
		else
		{
			// Determine number of channels
			// TODO Adjust output if single channel

			PixelElement *elem = (PixelElement*)&Pixel_Mapping[ Pixel_DisplayMapping[px] - 1 ];

			// Lookup channel data
			// TODO account for different channel size mappings
			print("\033[48;2");
			for ( uint8_t ch = 0; ch < elem->channels; ch++ )
			{
				print(";");
				uint16_t ch_pos = elem->indices[ch];
				PixelBuf *pixbuf = Pixel_bufferMap( ch_pos );
				printInt8( PixelBuf16( pixbuf, ch_pos ) );
			}
			print("m");
			print(" ");
			print("\033[0m");
		}

		// Determine what to increment next
		if ( col >= Pixel_DisplayMapping_Cols_KLL - 1 )
		{
			col = 0;
			row++;
			print(" ");
			print(NL);
		}
		else
		{
			col++;
		}
	}
}

void cliFunc_rectDisp( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	// TODO move to own function, use this func to control startup/args
	Pixel_dispBuffer();
}

