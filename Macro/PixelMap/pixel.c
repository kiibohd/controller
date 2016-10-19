/* Copyright (C) 2015-2016 by Jacob Alexander
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

// Local Includes
#include "pixel.h"



// ----- Function Declarations -----

void cliFunc_aniAdd    ( char* args );
void cliFunc_aniDel    ( char* args );
void cliFunc_chanTest  ( char* args );
void cliFunc_pixelList ( char* args );
void cliFunc_pixelTest ( char* args );
void cliFunc_rectDisp  ( char* args );



// ----- Enums -----

typedef enum PixelTest {
	PixelTest_Off = 0,    // Disabled
	PixelTest_Chan_All,   // Enable all positions
	PixelTest_Chan_Roll,  // Iterate over all positions
	PixelTest_Pixel_All,  // Enable all positions
	PixelTest_Pixel_Roll, // Iterate over all positions
} PixelTest;



// ----- Variables -----

// Macro Module command dictionary
CLIDict_Entry( aniAdd,       "Add the given animation id to the stack" );
CLIDict_Entry( aniDel,       "Remove the given stack index animation" );
CLIDict_Entry( chanTest,     "Channel test. No arg - next pixel. # - pixel, r - roll-through. a - all, s - stop" );
CLIDict_Entry( pixelList,    "Prints out pixel:channel mappings." );
CLIDict_Entry( pixelTest,    "Pixel test. No arg - next pixel. # - pixel, r - roll-through. a - all, s - stop" );
CLIDict_Entry( rectDisp,     "Show the current output of the MCU pixel buffer." );

CLIDict_Def( pixelCLIDict, "Pixel Module Commands" ) = {
	CLIDict_Item( aniAdd ),
	CLIDict_Item( aniDel ),
	CLIDict_Item( chanTest ),
	CLIDict_Item( pixelList ),
	CLIDict_Item( pixelTest ),
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
uint8_t  Pixel_Buffers_HostLen = Pixel_BuffersLen;
uint8_t  Pixel_MaxChannelPerPixel_Host = Pixel_MaxChannelPerPixel;
uint16_t Pixel_Mapping_HostLen = 128; // TODO Define
uint8_t  Pixel_AnimationStackElement_HostSize = sizeof( AnimationStackElement );
#endif



// ----- Function Declarations -----

uint8_t Pixel_animationProcess( AnimationStackElement *elem );



// ----- Capabilities -----

void Pixel_Animation_capability( uint8_t state, uint8_t stateType, uint8_t *args )
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

}

void Pixel_Pixel_capability( uint8_t state, uint8_t stateType, uint8_t *args )
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

#define PixelChange_Expansion(pixbuf, ch_pos, mod, ch, op) \
	/* Lookup buffer to data width mapping */ \
	switch ( elem->width + pixbuf->width ) \
	{ \
	case 8+8:   /*  8:8  bit mapping */ \
		PixelBuf8( pixbuf, ch_pos ) op PixelData8( mod, ch ); break; \
	case 16+8:  /* 16:8  bit mapping */ \
		PixelBuf16( pixbuf, ch_pos ) op PixelData8( mod, ch ); break; \
	case 32+8:  /* 32:8  bit mapping */ \
		PixelBuf32( pixbuf, ch_pos ) op PixelData8( mod, ch ); break; \
	case 16+16: /* 16:16 bit mapping */ \
		PixelBuf16( pixbuf, ch_pos ) op PixelData16( mod, ch ); break; \
	case 32+16: /* 32:16 bit mapping */ \
		PixelBuf32( pixbuf, ch_pos ) op PixelData16( mod, ch ); break; \
	case 32+32: /* 32:32 bit mapping */ \
		PixelBuf32( pixbuf, ch_pos ) op PixelData32( mod, ch ); break; \
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

	// Apply operation to each channel of the pixel
	for ( uint8_t ch = 0; ch < channels; ch++ )
	{
		// Lookup channel position
		uint16_t ch_pos = elem->indices[ch];

		// Determine which buffer we are in
		PixelBuf *pixbuf = Pixel_bufferMap( ch_pos );

		// Operation
		switch ( mod->change )
		{
		case PixelChange_Set:             // =
			PixelChange_Expansion( pixbuf, ch_pos, mod, ch, = );
			break;

		case PixelChange_Add:             // +
			PixelChange_Expansion( pixbuf, ch_pos, mod, ch, += );
			break;

		case PixelChange_Subtract:        // -
			PixelChange_Expansion( pixbuf, ch_pos, mod, ch, -= );
			break;

		case PixelChange_LeftShift:       // <<
			PixelChange_Expansion( pixbuf, ch_pos, mod, ch, <<= );
			break;

		case PixelChange_RightShift:      // >>
			PixelChange_Expansion( pixbuf, ch_pos, mod, ch, >>= );
			break;

		case PixelChange_NoRoll_Add:      // +:
			// Lookup buffer to data width mapping
			switch ( elem->width + pixbuf->width )
			{
			case 8+8:   //  8:8  bit mapping
			{
				uint8_t prev = PixelBuf8( pixbuf, ch_pos );
				PixelBuf8( pixbuf, ch_pos ) += PixelData8( mod, ch );
				if ( prev > PixelBuf8( pixbuf, ch_pos ) )
					PixelBuf8( pixbuf, ch_pos ) = 0xFF;
				break;
			}
			case 16+8:  // 16:8  bit mapping
			{
				uint16_t prev = PixelBuf16( pixbuf, ch_pos );
				PixelBuf16( pixbuf, ch_pos ) += PixelData8( mod, ch );
				if ( prev > PixelBuf16( pixbuf, ch_pos ) )
					PixelBuf16( pixbuf, ch_pos ) = 0xFFFF;
				break;
			}
			case 32+8:  // 32:8  bit mapping
			{
				uint32_t prev = PixelBuf32( pixbuf, ch_pos );
				PixelBuf32( pixbuf, ch_pos ) += PixelData8( mod, ch );
				if ( prev > PixelBuf32( pixbuf, ch_pos ) )
					PixelBuf32( pixbuf, ch_pos ) = 0xFFFFFFFF;
				break;
			}
			case 16+16: // 16:16 bit mapping
			{
				uint16_t prev = PixelBuf16( pixbuf, ch_pos );
				PixelBuf16( pixbuf, ch_pos ) += PixelData16( mod, ch );
				if ( prev > PixelBuf16( pixbuf, ch_pos ) )
					PixelBuf16( pixbuf, ch_pos ) = 0xFFFF;
				break;
			}
			case 32+16: // 32:16 bit mapping
			{
				uint32_t prev = PixelBuf32( pixbuf, ch_pos );
				PixelBuf32( pixbuf, ch_pos ) += PixelData16( mod, ch );
				if ( prev > PixelBuf32( pixbuf, ch_pos ) )
					PixelBuf32( pixbuf, ch_pos ) = 0xFFFFFFFF;
				break;
			}
			case 32+32: // 32:32 bit mapping
			{
				uint32_t prev = PixelBuf32( pixbuf, ch_pos );
				PixelBuf32( pixbuf, ch_pos ) += PixelData32( mod, ch );
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
			switch ( elem->width + pixbuf->width )
			{
			case 8+8:   //  8:8  bit mapping
			{
				uint8_t prev = PixelBuf8( pixbuf, ch_pos );
				PixelBuf8( pixbuf, ch_pos ) -= PixelData8( mod, ch );
				if ( prev < PixelBuf8( pixbuf, ch_pos ) )
					PixelBuf8( pixbuf, ch_pos ) = 0;
				break;
			}
			case 16+8:  // 16:8  bit mapping
			{
				uint16_t prev = PixelBuf16( pixbuf, ch_pos );
				PixelBuf16( pixbuf, ch_pos ) += PixelData8( mod, ch );
				if ( prev < PixelBuf16( pixbuf, ch_pos ) )
					PixelBuf16( pixbuf, ch_pos ) = 0;
				break;
			}
			case 32+8:  // 32:8  bit mapping
			{
				uint32_t prev = PixelBuf32( pixbuf, ch_pos );
				PixelBuf32( pixbuf, ch_pos ) += PixelData8( mod, ch );
				if ( prev < PixelBuf32( pixbuf, ch_pos ) )
					PixelBuf32( pixbuf, ch_pos ) = 0;
				break;
			}
			case 16+16: // 16:16 bit mapping
			{
				uint16_t prev = PixelBuf16( pixbuf, ch_pos );
				PixelBuf16( pixbuf, ch_pos ) += PixelData16( mod, ch );
				if ( prev < PixelBuf16( pixbuf, ch_pos ) )
					PixelBuf16( pixbuf, ch_pos ) = 0;
				break;
			}
			case 32+16: // 32:16 bit mapping
			{
				uint32_t prev = PixelBuf32( pixbuf, ch_pos );
				PixelBuf32( pixbuf, ch_pos ) += PixelData16( mod, ch );
				if ( prev < PixelBuf32( pixbuf, ch_pos ) )
					PixelBuf32( pixbuf, ch_pos ) = 0;
				break;
			}
			case 32+32: // 32:32 bit mapping
			{
				uint32_t prev = PixelBuf32( pixbuf, ch_pos );
				PixelBuf32( pixbuf, ch_pos ) += PixelData32( mod, ch );
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

	// Lookup fill algorith
	switch ( mod->type )
	{
	case PixelAddressType_Index:
		*elem = (PixelElement*)&Pixel_Mapping[mod->index];
		break;

	case PixelAddressType_Rect:
	case PixelAddressType_ColumnFill:
	case PixelAddressType_RowFill:
	case PixelAddressType_ScanCode:
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
	//PixelModElement *prev = 0;
	PixelModElement *mod = (PixelModElement*)&frame[pos];
	while ( mod->type != PixelAddressType_End )
	{
		// Lookup type of pixel, choose fill algorith and query all sub-pixels
		uint8_t next = 1;
		PixelElement *elem = 0;
		do {
			// Lookup pixel, and check if there are any more pixels left
			next = Pixel_fillPixelLookup( mod, &elem, next );

			// Apply operation to pixel
			Pixel_pixelEvaluation( mod, elem );
		} while ( next );

		// Calculate next position (this is dynamic, cannot be pre-calculated)
		pos += (elem->width / 8) * elem->channels + sizeof( PixelModElement );

		// Lookup next mod element
		//prev = mod;
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
		// TODO
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
		// TODO
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

// Toggle given pixel elementt
void Pixel_pixelToggle( PixelElement *elem )
{
	// Toggle each of the channels of the pixel
	for ( uint8_t ch = 0; ch < elem->channels; ch++ )
	{
		Pixel_channelToggle( elem->indices[ch] );
	}
}

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

// TODO Direction, starting position, value or function
uint8_t fill_counter = 0;
void Pixel_fill( uint16_t position )
{
	// Column fill
	fill_counter += position;
	for ( uint8_t row = 0; row < Pixel_DisplayMapping_Rows; row++ )
	{
		// Lookup Pixel
		uint8_t px = Pixel_DisplayMapping[ row * Pixel_DisplayMapping_Cols + position ];

		// Skip if 0, then subtract one to get the actual index
		if ( px-- == 0 )
		{
			continue;
		}

		// Toggle pixel
		//Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ px ] );

		// Interpolate Pixel
		Pixel_pixelInterpolate( (PixelElement*)&Pixel_Mapping[ px ], fill_counter, (row + 1) * 50 );
	}

	// Row fill
	/*
	for ( uint8_t col = 0; col < Pixel_DisplayMapping_Cols; col++ )
	{
		// Lookup Pixel
		uint8_t px = Pixel_DisplayMapping[ position * Pixel_DisplayMapping_Cols + col ];

		// Skip if 0, then subtract one to get the actual index
		if ( px-- == 0 )
		{
			continue;
		}

		// Toggle pixel
		Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ px ] );
	}
	*/
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
		if ( Pixel_testPos >= Pixel_TotalChannels )
			Pixel_testPos = 0;

		goto pixel_process_done;

	// Blink all channels
	case PixelTest_Chan_All:
	{
		uint16_t ch;

		// Only update 50 positions at a time
		for ( ch = Pixel_testPos; ch < Pixel_TotalChannels; ch++ )
		//for ( ch = Pixel_testPos; ch < Pixel_testPos + 50 && ch < Pixel_TotalChannels; ch++ )
		{
			// Toggle channel
			Pixel_channelToggle( ch );
		}

		Pixel_testPos = ch;

		// Only signal frame update after all pixels complete
		if ( Pixel_testPos >= Pixel_TotalChannels )
		{
			Pixel_testPos = 0;
			goto pixel_process_done;
		}

		return;
	}

	// Toggle current position, then increment
	case PixelTest_Pixel_Roll:
		// Toggle pixel
		Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ Pixel_testPos ] );

		// Increment pixel
		Pixel_testPos++;
		if ( Pixel_testPos >= Pixel_TotalPixels )
			Pixel_testPos = 0;

		goto pixel_process_done;


	case PixelTest_Pixel_All:
	{
		uint16_t px;

		// Only update 10 positions at a time
		for ( px = Pixel_testPos; px < Pixel_testPos + 50 && px < Pixel_TotalPixels; px++ )
		{
			// Toggle pixel
			Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ px ] );
		}

		Pixel_testPos = px;

		// Only signal frame update after all pixels complete
		if ( Pixel_testPos >= Pixel_TotalPixels )
		{
			Pixel_testPos = 0;
			goto pixel_process_done;
		}

		return;
	}

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

	// Process speed argument if given
	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Check for special args
	switch ( *arg1Ptr )
	{
	case 'b':
	case 'B':
		info_msg("Buffer List");

		// List all buffers
		for ( uint8_t buf = 0; buf < Pixel_BuffersLen; buf++ )
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
		for ( uint16_t pixel = 0; pixel < Pixel_TotalPixels; pixel++ )
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

	// Process speed argument if given
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
		printInt16( Pixel_testPos );
	}

	// Toggle channel
	Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ Pixel_testPos ] );

	// Increment channel
	Pixel_testPos++;
	if ( Pixel_testPos >= Pixel_TotalPixels )
		Pixel_testPos = 0;
}

void cliFunc_chanTest( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Process speed argument if given
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

	// Toggle channel
	Pixel_channelToggle( Pixel_testPos );

	// Increment channel
	Pixel_testPos++;
	if ( Pixel_testPos >= Pixel_TotalChannels )
		Pixel_testPos = 0;
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
	for ( uint16_t px = 0; px < Pixel_DisplayMapping_Cols * Pixel_DisplayMapping_Rows; px++ )
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
		if ( col >= Pixel_DisplayMapping_Cols - 1 )
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

