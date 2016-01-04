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
} PixelTest;



// ----- Variables -----

// Macro Module command dictionary
CLIDict_Entry( chanTest,     "Channel test. No arg - next pixel. # - pixel, r - roll-through. a - all, s - stop" );
CLIDict_Entry( pixelList,    "Prints out pixel:channel mappings." );
CLIDict_Entry( pixelTest,    "Pixel test. No arg - next pixel. # - pixel, r - roll-through. a - all, s - stop" );

CLIDict_Def( pixelCLIDict, "Pixel Module Commands" ) = {
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




// -------------------------------
// TODO This part is generated
// -------------------------------

// TODO Generate list of buffers and pointers from kll
#define LED_BufferLength       144
typedef struct LED_Buffer {
	uint16_t i2c_addr;
	uint16_t reg_addr;
	uint16_t buffer[LED_BufferLength];
} LED_Buffer;
extern LED_Buffer LED_pageBuffer[ ISSI_Chips_define ];


// Buffer list
#define Pixel_BuffersLen 3
#define Pixel_TotalChannels 432
PixelBuf Pixel_Buffers[] = {
	PixelBufElem( LED_BufferLength, 16, 0, LED_pageBuffer[0].buffer ),
	PixelBufElem( LED_BufferLength, 16, 144, LED_pageBuffer[1].buffer ),
	PixelBufElem( LED_BufferLength, 16, 288, LED_pageBuffer[2].buffer ),
};


// Pixel Mapping
uint8_t Pixel_Mapping[] = {
	Pixel_RGBChannel(0,1,2),
	// TODO
};

// Frame of led changes
//  const uint8_t <animation>_frame<num>[] = { PixelMod, ... }
#define Pixel_ModRGB(pixel,type,r,g,b) pixel, PixelChange_##type, 1, r, g, b
const uint8_t testani_frame0[] = {
	Pixel_ModRGB(0, Set, 30, 70, 120),
};
const uint8_t testani_frame1[] = {
	Pixel_ModRGB(0, Set, 0, 0, 0),
};
const uint8_t testani_frame2[] = {
	Pixel_ModRGB(0, Set, 60, 90, 140),
};

// Index of frames for animations
//  uint8_t *<animation>_frames[] = { <animation>_frame<num>, ... }
const uint8_t *testani_frames[] = {
	testani_frame0,
	testani_frame1,
	testani_frame2,
};

// Index of animations
//  uint8_t *Pixel_Animations[] = { <animation>_frames, ... }
const uint8_t **Pixel_Animations[] = {
	testani_frames,
};

// -------------------------------
// TODO GENERATED END
// -------------------------------



// ----- Capabilities -----



// ----- Functions -----

PixelBuf Pixel_bufferMap( uint16_t channel )
{
	// TODO Generate
	if      ( channel < 144 ) return Pixel_Buffers[0];
	else if ( channel < 288 ) return Pixel_Buffers[1];
	else if ( channel < 432 ) return Pixel_Buffers[2];

	// Invalid channel, return first channel and display error
	erro_msg("Invalid channel: ");
	printHex( channel );
	print( NL );
	return Pixel_Buffers[0];
}

void Pixel_channelToggle( uint16_t channel )
{
	// Determine which buffer we are in
	PixelBuf pixbuf = Pixel_bufferMap( channel );

	// Toggle channel accordingly
	switch ( pixbuf.width )
	{
	// Invalid width, default to 8
	default:
		warn_msg("Unknown width, using 8: ");
		printInt8( pixbuf.width );
		print(" Ch: ");
		printHex( channel );
		print( NL );
		// Falls through on purpose

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
		for ( ch = Pixel_testPos; ch < Pixel_testPos + 50 && ch < Pixel_TotalChannels; ch++ )
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
	Pixel_testMode = PixelTest_Off;
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
	}
}

void cliFunc_pixelTest( char* args )
{
	print( NL );
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

