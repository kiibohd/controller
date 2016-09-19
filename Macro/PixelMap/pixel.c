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



// ----- Enums -----

typedef enum PixelTest {
	PixelTest_Off = 0,    // Disabled
	PixelTest_Chan_All,   // Enable all positions
	PixelTest_Chan_Roll,  // Iterate over all positions
	PixelTest_Pixel_All,  // Enable all positions
	PixelTest_Pixel_Roll, // Iterate over all positions
	PixelTest_Pixel_Test,
} PixelTest;



// ----- Variables -----

// Macro Module command dictionary
CLIDict_Entry( aniAdd,       "Add the given animation id to the stack" );
CLIDict_Entry( aniDel,       "Remove the given stack index animation" );
CLIDict_Entry( chanTest,     "Channel test. No arg - next pixel. # - pixel, r - roll-through. a - all, s - stop" );
CLIDict_Entry( pixelList,    "Prints out pixel:channel mappings." );
CLIDict_Entry( pixelTest,    "Pixel test. No arg - next pixel. # - pixel, r - roll-through. a - all, s - stop, t - test" );

CLIDict_Def( pixelCLIDict, "Pixel Module Commands" ) = {
	CLIDict_Item( aniAdd ),
	CLIDict_Item( aniDel ),
	CLIDict_Item( chanTest ),
	CLIDict_Item( pixelList ),
	CLIDict_Item( pixelTest ),
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



// ----- Capabilities -----

void Rainbow_Toggle_capability( uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Rainbow_Toggle_capability()");
		return;
	}

	// Only use capability on press
	// TODO Analog
	if ( state != 0x01 )
		return;

	Pixel_testMode = Pixel_testMode != PixelTest_Off
		? PixelTest_Pixel_Test
		: PixelTest_Off;

	if ( Pixel_testMode != PixelTest_Off )
	{
		print("OFF");
		print(NL);
		Pixel_testMode = PixelTest_Off;

		Pixel_testPos = 0;
		// TODO use better function
		// TODO FIXME
		/*
		extern void cliFunc_ledReset( char* args );
		cliFunc_ledReset( 0 );
		*/
	}
	else
	{
		print("ON");
		print(NL);
		Pixel_testMode = PixelTest_Pixel_Test;
	}
}



// ----- Functions -----

PixelBuf *Pixel_bufferMap( uint16_t channel )
{
	// TODO Generate
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

// Process each pixel in the frame
// TODO Handle non-8bit channel widths
void Pixel_pixelProcess( const uint8_t *frame, uint16_t size )
{
	// Map each pixel modification then apply change accordingly
	for ( uint16_t pos = 0; pos < size; pos += sizeof( PixelMod ) )
	{
		// Map pixel
		PixelMod *mod = (PixelMod*)&frame[pos];
		PixelElement *elem = (PixelElement*)&Pixel_Mapping[mod->pixel];
		PixelBuf *pixbuf;

		// Lookup number of channels in pixel
		uint8_t channels = elem->channels;

		// Apply operation to each channel of the pixel
		for ( uint8_t ch = 0; ch < channels; ch++ )
		{
			uint16_t ch_pos = elem->indices[ch];

			// Operation
			switch ( mod->change )
			{
			case PixelChange_Set: // =
				// Determine which buffer we are in
				pixbuf = Pixel_bufferMap( ch_pos );

				// TODO Dynamically? determine which PixelBuf function to use
				PixelBuf16( pixbuf, ch_pos ) = mod->data[ch];
				break;

			default:
				warn_print("Unimplemented pixel modifier");
				break;
			}
		}

		// Skip extra channels
		// TODO account for non-8bit widths
		pos += channels;
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

void Pixel_columnFill()
{
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
	case PixelTest_Pixel_Test:
	{
		/* Fill demo */
		/*
		Pixel_fill( Pixel_testPos );

		// Increment pixel
		Pixel_testPos++;
		if ( Pixel_testPos >= Pixel_DisplayMapping_Cols )
		//if ( Pixel_testPos >= Pixel_DisplayMapping_Rows )
			Pixel_testPos = 0;
		*/

		/* CES Rainbow demo
		// Start from the top of the Animation Stack
		// XXX Temp - Play rainbow
		Pixel_pixelProcess( rainbow_frames[rainbow_pos], rainbow_framesizes[rainbow_pos] );
		rainbow_pos++;

		if ( rainbow_pos >= sizeof( rainbow_framesizes ) / 2 )
		{
			rainbow_pos = 0;
			goto pixel_process_done;
		}
		*/
	}

	default:
		break;
	}


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
	Pixel_testMode = PixelTest_Pixel_Test;
	//Pixel_testMode = PixelTest_Pixel_Roll;

	// Clear animation stack
	Pixel_AnimationStack.size = 0;
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
			printHex32( (uint32_t)(Pixel_Buffers[ buf ].data) );
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

	case 't':
	case 'T':
		info_msg("Starting pixel test");
		Pixel_testMode = PixelTest_Pixel_Test;
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
		rainbow_pos = 0;
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
	uint16_t index = Pixel_AnimationStack.size;
	Pixel_AnimationStack.stack[index].index = 1;
	Pixel_AnimationStack.stack[index].pos = 1;
	Pixel_AnimationStack.stack[index].loops = 1;
	Pixel_AnimationStack.stack[index].divider = 0;
	Pixel_AnimationStack.stack[index].modifier = AnimationModifier_None;
	Pixel_AnimationStack.size++;
}

void cliFunc_aniDel( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	// TODO
	Pixel_AnimationStack.size--;
}

