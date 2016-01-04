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
#define Pixel_TotalPixels 95 // TODO Generate
PixelElement Pixel_Mapping[] = {
	// Function Row (1-16)
	Pixel_RGBChannel(0,33,49), // 1
	Pixel_RGBChannel(1,17,50), // 2
	Pixel_RGBChannel(2,18,34), // 3
	Pixel_RGBChannel(3,19,35), // 4
	Pixel_RGBChannel(4,20,36), // 5
	Pixel_RGBChannel(5,21,37), // 6
	Pixel_RGBChannel(6,22,38), // 7
	Pixel_RGBChannel(7,23,39), // 8
	Pixel_RGBChannel(128,112,96), // 9
	Pixel_RGBChannel(129,113,97), // 10
	Pixel_RGBChannel(130,114,98), // 11
	Pixel_RGBChannel(131,115,99), // 12
	Pixel_RGBChannel(132,116,100), // 13
	Pixel_RGBChannel(133,117,101), // 14
	Pixel_RGBChannel(134,118,85), // 15
	Pixel_RGBChannel(135,102,86), // 16

	// Number Row (17-35)
	Pixel_RGBChannel(8,41,57), // 17
	Pixel_RGBChannel(9,25,58), // 18
	Pixel_RGBChannel(10,26,42), // 19
	Pixel_RGBChannel(11,27,43), // 20
	Pixel_RGBChannel(12,28,44), // 21
	Pixel_RGBChannel(13,29,45), // 22
	Pixel_RGBChannel(14,30,46), // 23
	Pixel_RGBChannel(15,31,47), // 24
	Pixel_RGBChannel(136,120,104), // 25
	Pixel_RGBChannel(137,121,105), // 26
	Pixel_RGBChannel(138,122,106), // 27
	Pixel_RGBChannel(139,123,107), // 28
	Pixel_RGBChannel(140,124,108), // 29
	Pixel_RGBChannel(141,125,109), // 30
	Pixel_RGBChannel(142,126,93), // 31
	Pixel_RGBChannel(143,110,94), // 32
	Pixel_RGBChannel(144,177,193), // 33
	Pixel_RGBChannel(145,161,194), // 34
	Pixel_RGBChannel(146,162,178), // 35

	// Top Alpha Row (36-53)
	Pixel_RGBChannel(147,163,179), // 36
	Pixel_RGBChannel(148,164,180), // 37
	Pixel_RGBChannel(149,165,181), // 38
	Pixel_RGBChannel(150,166,182), // 39
	Pixel_RGBChannel(151,167,183), // 40
	Pixel_RGBChannel(272,256,240), // 41
	Pixel_RGBChannel(273,257,241), // 42
	Pixel_RGBChannel(274,258,242), // 43
	Pixel_RGBChannel(275,259,243), // 44
	Pixel_RGBChannel(276,260,244), // 45
	Pixel_RGBChannel(277,261,245), // 46
	Pixel_RGBChannel(278,262,229), // 47
	Pixel_RGBChannel(279,246,230), // 48
	Pixel_RGBChannel(152,185,201), // 49
	Pixel_RGBChannel(153,169,202), // 50
	Pixel_RGBChannel(154,170,186), // 51
	Pixel_RGBChannel(155,171,187), // 52
	Pixel_RGBChannel(156,172,188), // 53

	// Mid Alpha Row (54-67)
	Pixel_RGBChannel(157,173,189), // 54
	Pixel_RGBChannel(158,174,190), // 55
	Pixel_RGBChannel(159,175,191), // 56
	Pixel_RGBChannel(280,264,248), // 57
	Pixel_RGBChannel(281,265,249), // 58
	Pixel_RGBChannel(282,266,250), // 59
	Pixel_RGBChannel(283,267,251), // 60
	Pixel_RGBChannel(284,268,252), // 61
	Pixel_RGBChannel(285,269,253), // 62
	Pixel_RGBChannel(286,270,237), // 63
	Pixel_RGBChannel(287,254,238), // 64
	Pixel_RGBChannel(288,321,337), // 65
	Pixel_RGBChannel(289,305,338), // 66
	Pixel_RGBChannel(290,306,322), // 67

	// Low Alpha Row (68-84)
	Pixel_RGBChannel(291,307,323), // 68
	Pixel_RGBChannel(292,308,324), // 69
	Pixel_RGBChannel(293,309,325), // 70
	Pixel_RGBChannel(294,310,326), // 71
	Pixel_RGBChannel(295,311,327), // 72
	Pixel_RGBChannel(416,400,384), // 73
	Pixel_RGBChannel(417,401,385), // 74
	Pixel_RGBChannel(418,402,386), // 75
	Pixel_RGBChannel(419,403,387), // 76
	Pixel_RGBChannel(420,404,388), // 77
	Pixel_RGBChannel(421,405,389), // 78
	Pixel_RGBChannel(422,406,373), // 79
	Pixel_RGBChannel(423,390,374), // 80
	Pixel_RGBChannel(296,329,345), // 81
	Pixel_RGBChannel(297,313,346), // 82
	Pixel_RGBChannel(298,314,330), // 83
	Pixel_RGBChannel(299,315,331), // 84

	// Mod Row (85-95)
	Pixel_RGBChannel(300,316,332), // 85
	Pixel_RGBChannel(301,317,333), // 86
	Pixel_RGBChannel(302,318,334), // 87
	Pixel_RGBChannel(303,319,335), // 88
	Pixel_RGBChannel(424,408,392), // 89
	Pixel_RGBChannel(425,409,393), // 90
	Pixel_RGBChannel(426,410,394), // 91
	Pixel_RGBChannel(427,411,395), // 92
	Pixel_RGBChannel(428,412,396), // 93
	Pixel_RGBChannel(429,413,397), // 94
	Pixel_RGBChannel(430,414,381), // 95

	// Underlighting - TODO
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

// Toggle the given channel
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

// Toggle given pixel elementt
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

	// Toggle current position, then increment
	case PixelTest_Pixel_Roll:
		// Toggle channel
		Pixel_pixelToggle( &Pixel_Mapping[ Pixel_testPos ] );

		// Increment channel
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
			// Toggle channel
			Pixel_pixelToggle( &Pixel_Mapping[ px ] );
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
	Pixel_pixelToggle( &Pixel_Mapping[ Pixel_testPos ] );

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

