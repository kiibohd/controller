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
#include <led.h>
#include <print.h>

// KLL Include
#include <kll.h>

// Local Includes
#include "pixel.h"



// ----- Function Declarations -----

void cliFunc_aniAdd     ( char* args );
void cliFunc_aniDel     ( char* args );
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
	PixelTest_Pixel_All,  // Enable all positions
	PixelTest_Pixel_Roll, // Iterate over all positions
	PixelTest_Scan_All,
	PixelTest_Scan_Roll,
	PixelTest_XY_All,
	PixelTest_XY_Roll,
} PixelTest;



// ----- Variables -----

// Macro Module command dictionary
CLIDict_Entry( aniAdd,       "Add the given animation id to the stack" );
CLIDict_Entry( aniDel,       "Remove the given stack index animation" );
CLIDict_Entry( chanTest,     "Channel test. No arg - next pixel. # - pixel, r - roll-through. a - all, s - stop" );
CLIDict_Entry( pixelList,    "Prints out pixel:channel mappings." );
CLIDict_Entry( pixelSCTest,  "Scancode pixel test. No arg - next pixel. # - pixel, r - roll-through. a - all, s - stop" );
CLIDict_Entry( pixelTest,    "Pixel test. No arg - next pixel. # - pixel, r - roll-through. a - all, s - stop" );
CLIDict_Entry( pixelXYTest,  "XY pixel test. No arg - next pixel. # - pixel, r - roll-through. a - all, s - stop" );
CLIDict_Entry( rectDisp,     "Show the current output of the MCU pixel buffer." );

CLIDict_Def( pixelCLIDict, "Pixel Module Commands" ) = {
	CLIDict_Item( aniAdd ),
	CLIDict_Item( aniDel ),
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
uint16_t  Pixel_testPos;

// Frame State
//  Indicates to pixel and output modules current state of the buffer
FrameState Pixel_FrameState;

// Animation Stack
AnimationStack Pixel_AnimationStack;

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



// ----- Function Declarations -----

uint8_t Pixel_animationProcess( AnimationStackElement *elem );
uint8_t Pixel_addAnimation( AnimationStackElement *element );

PixelBuf *Pixel_bufferMap( uint16_t channel );



// ----- Capabilities -----

void Pixel_Animation_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Pixel_Animation_capability()");
		return;
	}

	// Only use capability on press
	// TODO Analog
	if ( state != 0x01 )
		return;

	AnimationStackElement element;
	element.index = 6;
	element.loops = 1;
	element.pfunc = 1;
	//element.divmask = 0x0F;
	//element.divshift = 4;
	Pixel_addAnimation( &element );
}

void Pixel_Pixel_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Pixel_Pixel_capability()");
		return;
	}

	// Only use capability on press
	// TODO Analog
	if ( state != 0x01 )
		return;
}



// ----- Functions -----

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

// Allocates animaton memory slot
// Initiates animation to process on the next cycle
// Returns 1 on success, 0 on failure to allocate
uint8_t Pixel_addAnimation( AnimationStackElement *element )
{
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
	if      ( channel < 144 ) return &Pixel_Buffers[0];
	else if ( channel < 288 ) return &Pixel_Buffers[1];
	else if ( channel < 432 ) return &Pixel_Buffers[2];
	else if ( channel < 576 ) return &Pixel_Buffers[3];

	// Invalid channel, return first channel and display error
	erro_msg("Invalid channel: ");
	printHex( channel );
	print( NL );
	return &Pixel_Buffers[0];
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
				uint16_t prev = PixelBuf16( pixbuf, ch_pos );
				PixelBuf16( pixbuf, ch_pos ) += (uint16_t)mod_value;
				if ( prev > PixelBuf16( pixbuf, ch_pos ) )
					PixelBuf16( pixbuf, ch_pos ) = 0xFFFF;
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
				PixelBuf16( pixbuf, ch_pos ) += (uint16_t)mod_value;
				if ( prev < PixelBuf16( pixbuf, ch_pos ) )
					PixelBuf16( pixbuf, ch_pos ) = 0;
				break;
			}
			case 32: // 32  bit mapping
			{
				uint32_t prev = PixelBuf32( pixbuf, ch_pos );
				PixelBuf32( pixbuf, ch_pos ) += (uint32_t)mod_value;
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
uint8_t Pixel_fillPixelLookup( PixelModElement *mod, PixelElement **elem, uint8_t prev )
{
	// Used to determine next pixel in column or row (fill)
	uint8_t cur = prev;
	uint8_t index = 0;

	// Lookup fill algorith
	switch ( mod->type )
	{
	case PixelAddressType_Index:
		// Lookup pixel by absolute index
		*elem = (PixelElement*)&Pixel_Mapping[mod->index];
		return 0;

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
			return 0;
		}

		// Lookup pixel in rectangle display organization
		*elem = (PixelElement*)&Pixel_Mapping[
			Pixel_DisplayMapping[
				mod->rect.row * Pixel_DisplayMapping_Cols_KLL + mod->rect.col
			] - 1
		];
		return 0;

	case PixelAddressType_ColumnFill:
		// Make sure column exists
		if ( mod->rect.col >= Pixel_DisplayMapping_Cols_KLL )
		{
			erro_msg("Invalid column index: ");
			printInt16( mod->rect.col );
			print( NL );
			return 0;
		}

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

		// Lookup pixel, pixels are 1 indexed, hence the -1
		*elem = (PixelElement*)&Pixel_Mapping[ index - 1 ];
		return cur;

	case PixelAddressType_RowFill:
		// Make sure row,column exists
		if ( mod->rect.row >= Pixel_DisplayMapping_Rows_KLL )
		{
			erro_msg("Invalid row index: ");
			printInt16( mod->rect.row );
			print( NL );
			return 0;
		}

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

		// Lookup pixel, pixels are 1 indexed, hence the -1
		*elem = (PixelElement*)&Pixel_Mapping[ index - 1 ];
		return cur;

	case PixelAddressType_ScanCode:
		// Make sure ScanCode exists
		//if ( mod->index >= MaxScanCode ) // TODO Add MaxScanCode to kll_defs.h
		if ( mod->index >= 0xFF )
		{
			erro_msg("Invalid ScanCode: ");
			printInt16( mod->index );
			print( NL );
			return 0;
		}

		// Lookup ScanCode - Indices are 1-indexed in both arrays (hence the -1)
		uint16_t pixel = Pixel_ScanCodeToPixel[ mod->index - 1 ];
		*elem = (PixelElement*)&Pixel_Mapping[ pixel - 1 ];
		break;

	case PixelAddressType_RelativeIndex:
	case PixelAddressType_RelativeRect:
	case PixelAddressType_RelativeColumnFill:
	case PixelAddressType_RelativeRowFill:
		// TODO
		break;

	// Skip
	default:
		break;
	}

	return 0;
}



// -- Pixel Tweening --

// Standard Pixel Pixel Function (standard lookup, no additonal processing)
void Pixel_pixelTweenStandard( const uint8_t *frame, AnimationStackElement *stack_elem )
{
	// Iterate over all of the Pixel Modifier elements of the Animation Frame
	uint16_t pos = 0;
	PixelModElement *mod = (PixelModElement*)&frame[pos];
	while ( mod->type != PixelAddressType_End )
	{
		// Lookup type of pixel, choose fill algorith and query all sub-pixels
		uint8_t next = 0;
		PixelElement *elem = 0;
		do {
			// Lookup pixel, and check if there are any more pixels left
			next = Pixel_fillPixelLookup( mod, &elem, next );

			// Apply operation to pixel
			Pixel_pixelEvaluation( mod, elem );
		} while ( next );

		// Calculate next position (this is dynamic, cannot be pre-calculated)
		pos += ( ( elem->width / 8 + sizeof( PixelChange ) ) * elem->channels ) + sizeof( PixelModElement );

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
		uint32_t start = mod->index;
		uint32_t end = mod->index;

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

		default:
			break;
		}

		// Lookup prev and mod PixelElements
		PixelElement *prev_elem = 0;
		if ( prev != 0 )
		{
			Pixel_fillPixelLookup( prev, &prev_elem, 0 );
		}
		PixelElement *mod_elem = 0;
		Pixel_fillPixelLookup( mod, &mod_elem, 0 );

		// Make sure mod_elem is pointing to something, if not, this could be a blank
		// In which case continue to the next definition
		if ( mod_elem == 0 )
		{
			goto next;
		}

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
		for ( uint32_t cur = 0; cur < end - start + 1; cur++ )
		//for ( uint32_t cur = 0; cur < end - start + 1; cur++ )
		{
			// Determine where the tween pixel is
			switch ( mod->type )
			{
			case PixelAddressType_ColumnFill:
				interp_mod->rect.col = start + cur;
				break;

			case PixelAddressType_RowFill:
				interp_mod->rect.row = start + cur;
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

			default:
				break;
			}

			// Calculate interpolation pixel value
			// Uses prev to current PixelMods as the base
			// TODO Non-8bit
			if ( prev != 0 ) for ( uint8_t ch = 0; ch < mod_elem->channels; ch++ )
			{
				uint8_t pos = ch * 2 + 1; // TODO Only works with 8 bit channels
				interp_mod->data[pos] = Pixel_8bitInterpolation( prev->data[pos], mod->data[pos], slice * cur );
			}

			// Lookup type of pixel, choose fill algorith and query all sub-pixels
			uint8_t next = 0;
			do {
				// Lookup pixel, and check if there are any more pixels left
				next = Pixel_fillPixelLookup( interp_mod, &elem, next );

				// Apply operation to pixel
				Pixel_pixelEvaluation( interp_mod, elem );
			} while ( next );
		}

		prev = mod;
next:
		// Calculate next position (this is dynamic, cannot be pre-calculated)
		pos += ( ( elem->width / 8 + sizeof( PixelChange ) ) * elem->channels ) + sizeof( PixelModElement );

		// Lookup next mod element
		mod = (PixelModElement*)&frame[pos];
	}
}



// -- Frame Tweening --

// Standard Pixel Frame Function (no additional processing)
void Pixel_frameTweenStandard( uint16_t frame, uint8_t subframe, const uint8_t *data, AnimationStackElement *elem )
{
	// Do nothing during sub-frames, skip
	if ( subframe != 0 )
	{
		return;
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

	// Increment frame position
	elem->pos++;
}

// Pixel Frame Interpolation Tweening
// Do averaging between key frames
void Pixel_frameTweenInterpolation( uint16_t frame, uint8_t subframe, const uint8_t *data, AnimationStackElement *elem )
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

	// Calculate sub-frame index
	uint8_t subframe = elem->pos & elem->divmask;

	// Calculate frame index
	uint16_t frame = elem->pos >> elem->divshift;

	// Lookup animation frame to make sure we have something to do
	// TODO Make sure animation index exists -HaaTa
	const uint8_t *data = Pixel_Animations[elem->index][frame];

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
			return 0;
		}
	}

	// Lookup Frame Tweening Function
	switch ( elem->ffunc )
	{
	case PixelFrameFunction_Interpolation:
		Pixel_frameTweenInterpolation( frame, subframe, data, elem );
		break;

	// Generic, no additonal processing necessary
	case PixelFrameFunction_Off:
	case PixelFrameFunction_InterpolationKLL:
		Pixel_frameTweenStandard( frame, subframe, data, elem );
		break;
	}

	return 1;
}



// -- Pixel Control --

// Toggle the given channel
void Pixel_channelToggle( uint16_t channel )
{
	// Determine which buffer we are in
	PixelBuf *pixbuf = Pixel_bufferMap( channel );

	// Toggle channel accordingly
	switch ( pixbuf->width )
	{
	// Invalid width, default to 8
	default:
		warn_msg("Unknown width, using 8: ");
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

// Toggle given pixel element
void Pixel_pixelToggle( PixelElement *elem )
{
	// Toggle each of the channels of the pixel
	for ( uint8_t ch = 0; ch < elem->channels; ch++ )
	{
		Pixel_channelToggle( elem->indices[ch] );
	}
}

// Pixel Procesing Loop
inline void Pixel_process()
{
	// Only update frame when ready
	if ( Pixel_FrameState != FrameState_Update )
		return;

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
		for ( uint16_t ch = Pixel_testPos; ch < Pixel_TotalChannels_KLL; ch++ )
		{
			// Toggle channel
			Pixel_channelToggle( ch );
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
			return;
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
			return;
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

pixel_process_done:
	// Frame is now ready to send
	Pixel_FrameState = FrameState_Ready;
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
	Pixel_AnimationStack.size = 0;
	Pixel_clearAnimations();
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
	}

	// Check for specific position
	if ( *arg1Ptr != '\0' )
	{
		Pixel_testPos = numToInt( arg1Ptr );
	}
	else
	{
		info_msg("Pixel: ");
		printInt16( Pixel_testPos + 1 );
	}

	// Toggle channel
	Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ Pixel_testPos ] );

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
		return;

	case 'r':
	case 'R':
		info_msg("Channel roll test");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_Chan_Roll;
		return;

	case 's':
	case 'S':
		info_msg("Stopping channel test");
		Pixel_testMode = PixelTest_Off;
		return;
	}

	// Check for specific position
	if ( *arg1Ptr != '\0' )
	{
		Pixel_testPos = numToInt( arg1Ptr );
	}
	else
	{
		info_msg("Channel: ");
		printInt16( Pixel_testPos );
	}

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
		return;

	case 'r':
	case 'R':
		info_msg("Scancode pixel roll test");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_Scan_Roll;
		return;

	case 's':
	case 'S':
		info_msg("Stopping scancode pixel test");
		Pixel_testMode = PixelTest_Off;
		return;
	}

	// Check for specific position
	if ( *arg1Ptr != '\0' )
	{
		Pixel_testPos = numToInt( arg1Ptr );
	}
	else
	{
		info_msg("Scancode: ");
		printInt16( Pixel_testPos + 1 );
		print(" Pixel: ");
		printInt16( Pixel_ScanCodeToPixel[ Pixel_testPos ] );
	}

	// Lookup pixel
	uint16_t pixel = Pixel_ScanCodeToPixel[ Pixel_testPos ];

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
	Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ pixel - 1 ] );
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
	else
	{
		info_msg("Position (x,y): ");
		printInt16( Pixel_testPos % Pixel_DisplayMapping_Cols_KLL );
		print(",");
		printInt16( Pixel_testPos / Pixel_DisplayMapping_Cols_KLL );
		print(":");
		printInt16( Pixel_testPos );
		print(" Pixel: ");
		printInt16( Pixel_DisplayMapping[ Pixel_testPos ] );
	}

	// Lookup pixel
	uint16_t pixel = Pixel_DisplayMapping[ Pixel_testPos ];

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
	Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ pixel - 1 ] );
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

