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
		extern void cliFunc_ledReset( char* args );
		cliFunc_ledReset( 0 );
	}
	else
	{
		print("ON");
		print(NL);
		Pixel_testMode = PixelTest_Pixel_Test;
	}
}



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
#define s2bs(n) (n & 0xFF), (n >> 8)
#define Pixel_ModRGB(pixel,type,color) s2bs(pixel), PixelChange_##type, 1, color
#define Pixel_ModRGB_(pixel,type,r,g,b) pixel, PixelChange_##type, 1, r, g, b
const uint8_t testani_frame0[] = {
	Pixel_ModRGB_(0, Set, 30, 70, 120),
};
const uint8_t testani_frame1[] = {
	Pixel_ModRGB_(0, Set, 0, 0, 0),
};
const uint8_t testani_frame2[] = {
	Pixel_ModRGB_(0, Set, 60, 90, 140),
};

// Temp convenience colours
#define RGB_HalfRed      127,0,0
#define RGB_Red          255,0,0
#define RGB_RedOrange    255,64,0
#define RGB_Orange       255,127,0
#define RGB_OrangeYellow 255,191,0
#define RGB_Yellow       255,255,0
#define RGB_YellowGreen  127,255,0
#define RGB_Green        0,255,0
#define RGB_GreenBlue    0,127,127
#define RGB_Blue         0,0,255
#define RGB_BlueIndigo   38,0,193
#define RGB_Indigo       75,0,130
#define RGB_IndigoViolet 101,0,193
#define RGB_Violet       127,0,255
#define RGB_HalfViolet   64,0,127

#define RGB_White        255,255,255
//#define RGB_Black        0,0,0 // TODO
#define RGB_Black        RGB_White

// Rainbow Animation - Hardcoded
const uint8_t rainbow_frame0[] = {
	// Set 1
	Pixel_ModRGB(84, Set, RGB_HalfRed),
};

const uint8_t rainbow_frame1[] = {
	// Set 1
	Pixel_ModRGB(84, Set, RGB_Red),

	// Set 2
	Pixel_ModRGB(16, Set, RGB_HalfRed),
	Pixel_ModRGB(35, Set, RGB_HalfRed),
	Pixel_ModRGB(53, Set, RGB_HalfRed),
	Pixel_ModRGB(68, Set, RGB_HalfRed),
	Pixel_ModRGB(85, Set, RGB_HalfRed),
};

const uint8_t rainbow_frame2[] = {
	// Set 1
	Pixel_ModRGB(84, Set, RGB_RedOrange),

	// Set 2
	Pixel_ModRGB(16, Set, RGB_Red),
	Pixel_ModRGB(35, Set, RGB_Red),
	Pixel_ModRGB(53, Set, RGB_Red),
	Pixel_ModRGB(68, Set, RGB_Red),
	Pixel_ModRGB(85, Set, RGB_Red),

	// Set 3
	Pixel_ModRGB(0, Set, RGB_HalfRed),
	Pixel_ModRGB(17, Set, RGB_HalfRed),
	Pixel_ModRGB(36, Set, RGB_HalfRed),
	Pixel_ModRGB(54, Set, RGB_HalfRed),
	Pixel_ModRGB(70, Set, RGB_HalfRed),
	Pixel_ModRGB(86, Set, RGB_HalfRed),
};

const uint8_t rainbow_frame3[] = {
	// Set 1
	Pixel_ModRGB(84, Set, RGB_Orange),

	// Set 2
	Pixel_ModRGB(16, Set, RGB_RedOrange),
	Pixel_ModRGB(35, Set, RGB_RedOrange),
	Pixel_ModRGB(53, Set, RGB_RedOrange),
	Pixel_ModRGB(68, Set, RGB_RedOrange),
	Pixel_ModRGB(85, Set, RGB_RedOrange),

	// Set 3
	Pixel_ModRGB(0, Set, RGB_Red),
	Pixel_ModRGB(17, Set, RGB_Red),
	Pixel_ModRGB(36, Set, RGB_Red),
	Pixel_ModRGB(54, Set, RGB_Red),
	Pixel_ModRGB(70, Set, RGB_Red),
	Pixel_ModRGB(86, Set, RGB_Red),

	// Set 4
	Pixel_ModRGB(18, Set, RGB_HalfRed),
	Pixel_ModRGB(37, Set, RGB_HalfRed),
	Pixel_ModRGB(55, Set, RGB_HalfRed),
	Pixel_ModRGB(71, Set, RGB_HalfRed),
};

const uint8_t rainbow_frame4[] = {
	// Set 1
	Pixel_ModRGB(84, Set, RGB_OrangeYellow),

	// Set 2
	Pixel_ModRGB(16, Set, RGB_Orange),
	Pixel_ModRGB(35, Set, RGB_Orange),
	Pixel_ModRGB(53, Set, RGB_Orange),
	Pixel_ModRGB(68, Set, RGB_Orange),
	Pixel_ModRGB(85, Set, RGB_Orange),

	// Set 3
	Pixel_ModRGB(0, Set, RGB_RedOrange),
	Pixel_ModRGB(17, Set, RGB_RedOrange),
	Pixel_ModRGB(36, Set, RGB_RedOrange),
	Pixel_ModRGB(54, Set, RGB_RedOrange),
	Pixel_ModRGB(70, Set, RGB_RedOrange),
	Pixel_ModRGB(86, Set, RGB_RedOrange),

	// Set 4
	Pixel_ModRGB(18, Set, RGB_Red),
	Pixel_ModRGB(37, Set, RGB_Red),
	Pixel_ModRGB(55, Set, RGB_Red),
	Pixel_ModRGB(71, Set, RGB_Red),

	// Set 5
	Pixel_ModRGB(1, Set, RGB_HalfRed),
	Pixel_ModRGB(19, Set, RGB_HalfRed),
	Pixel_ModRGB(38, Set, RGB_HalfRed),
	Pixel_ModRGB(56, Set, RGB_HalfRed),
	Pixel_ModRGB(72, Set, RGB_HalfRed),
};

const uint8_t rainbow_frame5[] = {
	// Set 1
	Pixel_ModRGB(84, Set, RGB_Yellow),

	// Set 2
	Pixel_ModRGB(16, Set, RGB_OrangeYellow),
	Pixel_ModRGB(35, Set, RGB_OrangeYellow),
	Pixel_ModRGB(53, Set, RGB_OrangeYellow),
	Pixel_ModRGB(68, Set, RGB_OrangeYellow),
	Pixel_ModRGB(85, Set, RGB_OrangeYellow),

	// Set 3
	Pixel_ModRGB(0, Set, RGB_Orange),
	Pixel_ModRGB(17, Set, RGB_Orange),
	Pixel_ModRGB(36, Set, RGB_Orange),
	Pixel_ModRGB(54, Set, RGB_Orange),
	Pixel_ModRGB(70, Set, RGB_Orange),
	Pixel_ModRGB(86, Set, RGB_Orange),

	// Set 4
	Pixel_ModRGB(18, Set, RGB_RedOrange),
	Pixel_ModRGB(37, Set, RGB_RedOrange),
	Pixel_ModRGB(55, Set, RGB_RedOrange),
	Pixel_ModRGB(71, Set, RGB_RedOrange),

	// Set 5
	Pixel_ModRGB(1, Set, RGB_Red),
	Pixel_ModRGB(19, Set, RGB_Red),
	Pixel_ModRGB(38, Set, RGB_Red),
	Pixel_ModRGB(56, Set, RGB_Red),
	Pixel_ModRGB(72, Set, RGB_Red),

	// Set 6
	Pixel_ModRGB(2, Set, RGB_HalfRed),
	Pixel_ModRGB(20, Set, RGB_HalfRed),
	Pixel_ModRGB(39, Set, RGB_HalfRed),
	Pixel_ModRGB(57, Set, RGB_HalfRed),
	Pixel_ModRGB(73, Set, RGB_HalfRed),
};

const uint8_t rainbow_frame6[] = {
	// Set 1
	Pixel_ModRGB(84, Set, RGB_YellowGreen),

	// Set 2
	Pixel_ModRGB(16, Set, RGB_Yellow),
	Pixel_ModRGB(35, Set, RGB_Yellow),
	Pixel_ModRGB(53, Set, RGB_Yellow),
	Pixel_ModRGB(68, Set, RGB_Yellow),
	Pixel_ModRGB(85, Set, RGB_Yellow),

	// Set 3
	Pixel_ModRGB(0, Set, RGB_OrangeYellow),
	Pixel_ModRGB(17, Set, RGB_OrangeYellow),
	Pixel_ModRGB(36, Set, RGB_OrangeYellow),
	Pixel_ModRGB(54, Set, RGB_OrangeYellow),
	Pixel_ModRGB(70, Set, RGB_OrangeYellow),
	Pixel_ModRGB(86, Set, RGB_OrangeYellow),

	// Set 4
	Pixel_ModRGB(18, Set, RGB_Orange),
	Pixel_ModRGB(37, Set, RGB_Orange),
	Pixel_ModRGB(55, Set, RGB_Orange),
	Pixel_ModRGB(71, Set, RGB_Orange),

	// Set 5
	Pixel_ModRGB(1, Set, RGB_RedOrange),
	Pixel_ModRGB(19, Set, RGB_RedOrange),
	Pixel_ModRGB(38, Set, RGB_RedOrange),
	Pixel_ModRGB(56, Set, RGB_RedOrange),
	Pixel_ModRGB(72, Set, RGB_RedOrange),

	// Set 6
	Pixel_ModRGB(2, Set, RGB_Red),
	Pixel_ModRGB(20, Set, RGB_Red),
	Pixel_ModRGB(39, Set, RGB_Red),
	Pixel_ModRGB(57, Set, RGB_Red),
	Pixel_ModRGB(73, Set, RGB_Red),

	// Set 7
	Pixel_ModRGB(3, Set, RGB_HalfRed),
	Pixel_ModRGB(21, Set, RGB_HalfRed),
	Pixel_ModRGB(40, Set, RGB_HalfRed),
	Pixel_ModRGB(58, Set, RGB_HalfRed),
	Pixel_ModRGB(74, Set, RGB_HalfRed),
	Pixel_ModRGB(87, Set, RGB_HalfRed),
};

const uint8_t rainbow_frame7[] = {
	// Set 1
	Pixel_ModRGB(84, Set, RGB_Green),

	// Set 2
	Pixel_ModRGB(16, Set, RGB_YellowGreen),
	Pixel_ModRGB(35, Set, RGB_YellowGreen),
	Pixel_ModRGB(53, Set, RGB_YellowGreen),
	Pixel_ModRGB(68, Set, RGB_YellowGreen),
	Pixel_ModRGB(85, Set, RGB_YellowGreen),

	// Set 3
	Pixel_ModRGB(0, Set, RGB_Yellow),
	Pixel_ModRGB(17, Set, RGB_Yellow),
	Pixel_ModRGB(36, Set, RGB_Yellow),
	Pixel_ModRGB(54, Set, RGB_Yellow),
	Pixel_ModRGB(70, Set, RGB_Yellow),
	Pixel_ModRGB(86, Set, RGB_Yellow),

	// Set 4
	Pixel_ModRGB(18, Set, RGB_OrangeYellow),
	Pixel_ModRGB(37, Set, RGB_OrangeYellow),
	Pixel_ModRGB(55, Set, RGB_OrangeYellow),
	Pixel_ModRGB(71, Set, RGB_OrangeYellow),

	// Set 5
	Pixel_ModRGB(1, Set, RGB_Orange),
	Pixel_ModRGB(19, Set, RGB_Orange),
	Pixel_ModRGB(38, Set, RGB_Orange),
	Pixel_ModRGB(56, Set, RGB_Orange),
	Pixel_ModRGB(72, Set, RGB_Orange),

	// Set 6
	Pixel_ModRGB(2, Set, RGB_RedOrange),
	Pixel_ModRGB(20, Set, RGB_RedOrange),
	Pixel_ModRGB(39, Set, RGB_RedOrange),
	Pixel_ModRGB(57, Set, RGB_RedOrange),
	Pixel_ModRGB(73, Set, RGB_RedOrange),

	// Set 7
	Pixel_ModRGB(3, Set, RGB_Red),
	Pixel_ModRGB(21, Set, RGB_Red),
	Pixel_ModRGB(40, Set, RGB_Red),
	Pixel_ModRGB(58, Set, RGB_Red),
	Pixel_ModRGB(74, Set, RGB_Red),
	Pixel_ModRGB(87, Set, RGB_Red),

	// Set 8
	Pixel_ModRGB(4, Set, RGB_HalfRed),
	Pixel_ModRGB(22, Set, RGB_HalfRed),
	Pixel_ModRGB(41, Set, RGB_HalfRed),
	Pixel_ModRGB(59, Set, RGB_HalfRed),
	Pixel_ModRGB(75, Set, RGB_HalfRed),
};

const uint8_t rainbow_frame8[] = {
	// Set 1
	Pixel_ModRGB(84, Set, RGB_GreenBlue),

	// Set 2
	Pixel_ModRGB(16, Set, RGB_Green),
	Pixel_ModRGB(35, Set, RGB_Green),
	Pixel_ModRGB(53, Set, RGB_Green),
	Pixel_ModRGB(68, Set, RGB_Green),
	Pixel_ModRGB(85, Set, RGB_Green),

	// Set 3
	Pixel_ModRGB(0, Set, RGB_YellowGreen),
	Pixel_ModRGB(17, Set, RGB_YellowGreen),
	Pixel_ModRGB(36, Set, RGB_YellowGreen),
	Pixel_ModRGB(54, Set, RGB_YellowGreen),
	Pixel_ModRGB(70, Set, RGB_YellowGreen),
	Pixel_ModRGB(86, Set, RGB_YellowGreen),

	// Set 4
	Pixel_ModRGB(18, Set, RGB_Yellow),
	Pixel_ModRGB(37, Set, RGB_Yellow),
	Pixel_ModRGB(55, Set, RGB_Yellow),
	Pixel_ModRGB(71, Set, RGB_Yellow),

	// Set 5
	Pixel_ModRGB(1, Set, RGB_OrangeYellow),
	Pixel_ModRGB(19, Set, RGB_OrangeYellow),
	Pixel_ModRGB(38, Set, RGB_OrangeYellow),
	Pixel_ModRGB(56, Set, RGB_OrangeYellow),
	Pixel_ModRGB(72, Set, RGB_OrangeYellow),

	// Set 6
	Pixel_ModRGB(2, Set, RGB_Orange),
	Pixel_ModRGB(20, Set, RGB_Orange),
	Pixel_ModRGB(39, Set, RGB_Orange),
	Pixel_ModRGB(57, Set, RGB_Orange),
	Pixel_ModRGB(73, Set, RGB_Orange),

	// Set 7
	Pixel_ModRGB(3, Set, RGB_RedOrange),
	Pixel_ModRGB(21, Set, RGB_RedOrange),
	Pixel_ModRGB(40, Set, RGB_RedOrange),
	Pixel_ModRGB(58, Set, RGB_RedOrange),
	Pixel_ModRGB(74, Set, RGB_RedOrange),
	Pixel_ModRGB(87, Set, RGB_RedOrange),

	// Set 8
	Pixel_ModRGB(4, Set, RGB_Red),
	Pixel_ModRGB(22, Set, RGB_Red),
	Pixel_ModRGB(41, Set, RGB_Red),
	Pixel_ModRGB(59, Set, RGB_Red),
	Pixel_ModRGB(75, Set, RGB_Red),

	// Set 9
	Pixel_ModRGB(5, Set, RGB_HalfRed),
	Pixel_ModRGB(23, Set, RGB_HalfRed),
	Pixel_ModRGB(42, Set, RGB_HalfRed),
	Pixel_ModRGB(60, Set, RGB_HalfRed),
	Pixel_ModRGB(76, Set, RGB_HalfRed),
};

const uint8_t rainbow_frame9[] = {
	// Set 1
	Pixel_ModRGB(84, Set, RGB_Blue),

	// Set 2
	Pixel_ModRGB(16, Set, RGB_GreenBlue),
	Pixel_ModRGB(35, Set, RGB_GreenBlue),
	Pixel_ModRGB(53, Set, RGB_GreenBlue),
	Pixel_ModRGB(68, Set, RGB_GreenBlue),
	Pixel_ModRGB(85, Set, RGB_GreenBlue),

	// Set 3
	Pixel_ModRGB(0, Set, RGB_Green),
	Pixel_ModRGB(17, Set, RGB_Green),
	Pixel_ModRGB(36, Set, RGB_Green),
	Pixel_ModRGB(54, Set, RGB_Green),
	Pixel_ModRGB(70, Set, RGB_Green),
	Pixel_ModRGB(86, Set, RGB_Green),

	// Set 4
	Pixel_ModRGB(18, Set, RGB_YellowGreen),
	Pixel_ModRGB(37, Set, RGB_YellowGreen),
	Pixel_ModRGB(55, Set, RGB_YellowGreen),
	Pixel_ModRGB(71, Set, RGB_YellowGreen),

	// Set 5
	Pixel_ModRGB(1, Set, RGB_Yellow),
	Pixel_ModRGB(19, Set, RGB_Yellow),
	Pixel_ModRGB(38, Set, RGB_Yellow),
	Pixel_ModRGB(56, Set, RGB_Yellow),
	Pixel_ModRGB(72, Set, RGB_Yellow),

	// Set 6
	Pixel_ModRGB(2, Set, RGB_OrangeYellow),
	Pixel_ModRGB(20, Set, RGB_OrangeYellow),
	Pixel_ModRGB(39, Set, RGB_OrangeYellow),
	Pixel_ModRGB(57, Set, RGB_OrangeYellow),
	Pixel_ModRGB(73, Set, RGB_OrangeYellow),

	// Set 7
	Pixel_ModRGB(3, Set, RGB_Orange),
	Pixel_ModRGB(21, Set, RGB_Orange),
	Pixel_ModRGB(40, Set, RGB_Orange),
	Pixel_ModRGB(58, Set, RGB_Orange),
	Pixel_ModRGB(74, Set, RGB_Orange),
	Pixel_ModRGB(87, Set, RGB_Orange),

	// Set 8
	Pixel_ModRGB(4, Set, RGB_RedOrange),
	Pixel_ModRGB(22, Set, RGB_RedOrange),
	Pixel_ModRGB(41, Set, RGB_RedOrange),
	Pixel_ModRGB(59, Set, RGB_RedOrange),
	Pixel_ModRGB(75, Set, RGB_RedOrange),

	// Set 9
	Pixel_ModRGB(5, Set, RGB_Red),
	Pixel_ModRGB(23, Set, RGB_Red),
	Pixel_ModRGB(42, Set, RGB_Red),
	Pixel_ModRGB(60, Set, RGB_Red),
	Pixel_ModRGB(76, Set, RGB_Red),

	// Set 10
	Pixel_ModRGB(6, Set, RGB_HalfRed),
	Pixel_ModRGB(24, Set, RGB_HalfRed),
	Pixel_ModRGB(43, Set, RGB_HalfRed),
	Pixel_ModRGB(61, Set, RGB_HalfRed),
	Pixel_ModRGB(77, Set, RGB_HalfRed),
};

const uint8_t rainbow_frame10[] = {
	// Set 1
	Pixel_ModRGB(84, Set, RGB_BlueIndigo),

	// Set 2
	Pixel_ModRGB(16, Set, RGB_Blue),
	Pixel_ModRGB(35, Set, RGB_Blue),
	Pixel_ModRGB(53, Set, RGB_Blue),
	Pixel_ModRGB(68, Set, RGB_Blue),
	Pixel_ModRGB(85, Set, RGB_Blue),

	// Set 3
	Pixel_ModRGB(0, Set, RGB_GreenBlue),
	Pixel_ModRGB(17, Set, RGB_GreenBlue),
	Pixel_ModRGB(36, Set, RGB_GreenBlue),
	Pixel_ModRGB(54, Set, RGB_GreenBlue),
	Pixel_ModRGB(70, Set, RGB_GreenBlue),
	Pixel_ModRGB(86, Set, RGB_GreenBlue),

	// Set 4
	Pixel_ModRGB(18, Set, RGB_Green),
	Pixel_ModRGB(37, Set, RGB_Green),
	Pixel_ModRGB(55, Set, RGB_Green),
	Pixel_ModRGB(71, Set, RGB_Green),

	// Set 5
	Pixel_ModRGB(1, Set, RGB_YellowGreen),
	Pixel_ModRGB(19, Set, RGB_YellowGreen),
	Pixel_ModRGB(38, Set, RGB_YellowGreen),
	Pixel_ModRGB(56, Set, RGB_YellowGreen),
	Pixel_ModRGB(72, Set, RGB_YellowGreen),

	// Set 6
	Pixel_ModRGB(2, Set, RGB_Yellow),
	Pixel_ModRGB(20, Set, RGB_Yellow),
	Pixel_ModRGB(39, Set, RGB_Yellow),
	Pixel_ModRGB(57, Set, RGB_Yellow),
	Pixel_ModRGB(73, Set, RGB_Yellow),

	// Set 7
	Pixel_ModRGB(3, Set, RGB_OrangeYellow),
	Pixel_ModRGB(21, Set, RGB_OrangeYellow),
	Pixel_ModRGB(40, Set, RGB_OrangeYellow),
	Pixel_ModRGB(58, Set, RGB_OrangeYellow),
	Pixel_ModRGB(74, Set, RGB_OrangeYellow),
	Pixel_ModRGB(87, Set, RGB_OrangeYellow),

	// Set 8
	Pixel_ModRGB(4, Set, RGB_Orange),
	Pixel_ModRGB(22, Set, RGB_Orange),
	Pixel_ModRGB(41, Set, RGB_Orange),
	Pixel_ModRGB(59, Set, RGB_Orange),
	Pixel_ModRGB(75, Set, RGB_Orange),

	// Set 9
	Pixel_ModRGB(5, Set, RGB_RedOrange),
	Pixel_ModRGB(23, Set, RGB_RedOrange),
	Pixel_ModRGB(42, Set, RGB_RedOrange),
	Pixel_ModRGB(60, Set, RGB_RedOrange),
	Pixel_ModRGB(76, Set, RGB_RedOrange),

	// Set 10
	Pixel_ModRGB(6, Set, RGB_Red),
	Pixel_ModRGB(24, Set, RGB_Red),
	Pixel_ModRGB(43, Set, RGB_Red),
	Pixel_ModRGB(61, Set, RGB_Red),
	Pixel_ModRGB(77, Set, RGB_Red),

	// Set 11
	Pixel_ModRGB(7, Set, RGB_HalfRed),
	Pixel_ModRGB(25, Set, RGB_HalfRed),
	Pixel_ModRGB(44, Set, RGB_HalfRed),
	Pixel_ModRGB(62, Set, RGB_HalfRed),
	Pixel_ModRGB(78, Set, RGB_HalfRed),
	Pixel_ModRGB(88, Set, RGB_HalfRed),
};

const uint8_t rainbow_frame11[] = {
	// Set 1
	Pixel_ModRGB(84, Set, RGB_Indigo),

	// Set 2
	Pixel_ModRGB(16, Set, RGB_BlueIndigo),
	Pixel_ModRGB(35, Set, RGB_BlueIndigo),
	Pixel_ModRGB(53, Set, RGB_BlueIndigo),
	Pixel_ModRGB(68, Set, RGB_BlueIndigo),
	Pixel_ModRGB(85, Set, RGB_BlueIndigo),

	// Set 3
	Pixel_ModRGB(0, Set, RGB_Blue),
	Pixel_ModRGB(17, Set, RGB_Blue),
	Pixel_ModRGB(36, Set, RGB_Blue),
	Pixel_ModRGB(54, Set, RGB_Blue),
	Pixel_ModRGB(70, Set, RGB_Blue),
	Pixel_ModRGB(86, Set, RGB_Blue),

	// Set 4
	Pixel_ModRGB(18, Set, RGB_GreenBlue),
	Pixel_ModRGB(37, Set, RGB_GreenBlue),
	Pixel_ModRGB(55, Set, RGB_GreenBlue),
	Pixel_ModRGB(71, Set, RGB_GreenBlue),

	// Set 5
	Pixel_ModRGB(1, Set, RGB_Green),
	Pixel_ModRGB(19, Set, RGB_Green),
	Pixel_ModRGB(38, Set, RGB_Green),
	Pixel_ModRGB(56, Set, RGB_Green),
	Pixel_ModRGB(72, Set, RGB_Green),

	// Set 6
	Pixel_ModRGB(2, Set, RGB_YellowGreen),
	Pixel_ModRGB(20, Set, RGB_YellowGreen),
	Pixel_ModRGB(39, Set, RGB_YellowGreen),
	Pixel_ModRGB(57, Set, RGB_YellowGreen),
	Pixel_ModRGB(73, Set, RGB_YellowGreen),

	// Set 7
	Pixel_ModRGB(3, Set, RGB_Yellow),
	Pixel_ModRGB(21, Set, RGB_Yellow),
	Pixel_ModRGB(40, Set, RGB_Yellow),
	Pixel_ModRGB(58, Set, RGB_Yellow),
	Pixel_ModRGB(74, Set, RGB_Yellow),
	Pixel_ModRGB(87, Set, RGB_Yellow),

	// Set 8
	Pixel_ModRGB(4, Set, RGB_OrangeYellow),
	Pixel_ModRGB(22, Set, RGB_OrangeYellow),
	Pixel_ModRGB(41, Set, RGB_OrangeYellow),
	Pixel_ModRGB(59, Set, RGB_OrangeYellow),
	Pixel_ModRGB(75, Set, RGB_OrangeYellow),

	// Set 9
	Pixel_ModRGB(5, Set, RGB_Orange),
	Pixel_ModRGB(23, Set, RGB_Orange),
	Pixel_ModRGB(42, Set, RGB_Orange),
	Pixel_ModRGB(60, Set, RGB_Orange),
	Pixel_ModRGB(76, Set, RGB_Orange),

	// Set 10
	Pixel_ModRGB(6, Set, RGB_RedOrange),
	Pixel_ModRGB(24, Set, RGB_RedOrange),
	Pixel_ModRGB(43, Set, RGB_RedOrange),
	Pixel_ModRGB(61, Set, RGB_RedOrange),
	Pixel_ModRGB(77, Set, RGB_RedOrange),

	// Set 11
	Pixel_ModRGB(7, Set, RGB_Red),
	Pixel_ModRGB(25, Set, RGB_Red),
	Pixel_ModRGB(44, Set, RGB_Red),
	Pixel_ModRGB(62, Set, RGB_Red),
	Pixel_ModRGB(78, Set, RGB_Red),
	Pixel_ModRGB(88, Set, RGB_Red),

	// Set 12
	Pixel_ModRGB(8, Set, RGB_HalfRed),
	Pixel_ModRGB(26, Set, RGB_HalfRed),
	Pixel_ModRGB(45, Set, RGB_HalfRed),
	Pixel_ModRGB(63, Set, RGB_HalfRed),
	Pixel_ModRGB(79, Set, RGB_HalfRed),
	Pixel_ModRGB(89, Set, RGB_HalfRed),
};

const uint8_t rainbow_frame12[] = {
	// Set 1
	Pixel_ModRGB(84, Set, RGB_IndigoViolet),

	// Set 2
	Pixel_ModRGB(16, Set, RGB_Indigo),
	Pixel_ModRGB(35, Set, RGB_Indigo),
	Pixel_ModRGB(53, Set, RGB_Indigo),
	Pixel_ModRGB(68, Set, RGB_Indigo),
	Pixel_ModRGB(85, Set, RGB_Indigo),

	// Set 3
	Pixel_ModRGB(0, Set, RGB_BlueIndigo),
	Pixel_ModRGB(17, Set, RGB_BlueIndigo),
	Pixel_ModRGB(36, Set, RGB_BlueIndigo),
	Pixel_ModRGB(54, Set, RGB_BlueIndigo),
	Pixel_ModRGB(70, Set, RGB_BlueIndigo),
	Pixel_ModRGB(86, Set, RGB_BlueIndigo),

	// Set 4
	Pixel_ModRGB(18, Set, RGB_Blue),
	Pixel_ModRGB(37, Set, RGB_Blue),
	Pixel_ModRGB(55, Set, RGB_Blue),
	Pixel_ModRGB(71, Set, RGB_Blue),

	// Set 5
	Pixel_ModRGB(1, Set, RGB_GreenBlue),
	Pixel_ModRGB(19, Set, RGB_GreenBlue),
	Pixel_ModRGB(38, Set, RGB_GreenBlue),
	Pixel_ModRGB(56, Set, RGB_GreenBlue),
	Pixel_ModRGB(72, Set, RGB_GreenBlue),

	// Set 6
	Pixel_ModRGB(2, Set, RGB_Green),
	Pixel_ModRGB(20, Set, RGB_Green),
	Pixel_ModRGB(39, Set, RGB_Green),
	Pixel_ModRGB(57, Set, RGB_Green),
	Pixel_ModRGB(73, Set, RGB_Green),

	// Set 7
	Pixel_ModRGB(3, Set, RGB_YellowGreen),
	Pixel_ModRGB(21, Set, RGB_YellowGreen),
	Pixel_ModRGB(40, Set, RGB_YellowGreen),
	Pixel_ModRGB(58, Set, RGB_YellowGreen),
	Pixel_ModRGB(74, Set, RGB_YellowGreen),
	Pixel_ModRGB(87, Set, RGB_YellowGreen),

	// Set 8
	Pixel_ModRGB(4, Set, RGB_Yellow),
	Pixel_ModRGB(22, Set, RGB_Yellow),
	Pixel_ModRGB(41, Set, RGB_Yellow),
	Pixel_ModRGB(59, Set, RGB_Yellow),
	Pixel_ModRGB(75, Set, RGB_Yellow),

	// Set 9
	Pixel_ModRGB(5, Set, RGB_OrangeYellow),
	Pixel_ModRGB(23, Set, RGB_OrangeYellow),
	Pixel_ModRGB(42, Set, RGB_OrangeYellow),
	Pixel_ModRGB(60, Set, RGB_OrangeYellow),
	Pixel_ModRGB(76, Set, RGB_OrangeYellow),

	// Set 10
	Pixel_ModRGB(6, Set, RGB_Orange),
	Pixel_ModRGB(24, Set, RGB_Orange),
	Pixel_ModRGB(43, Set, RGB_Orange),
	Pixel_ModRGB(61, Set, RGB_Orange),
	Pixel_ModRGB(77, Set, RGB_Orange),

	// Set 11
	Pixel_ModRGB(7, Set, RGB_RedOrange),
	Pixel_ModRGB(25, Set, RGB_RedOrange),
	Pixel_ModRGB(44, Set, RGB_RedOrange),
	Pixel_ModRGB(62, Set, RGB_RedOrange),
	Pixel_ModRGB(78, Set, RGB_RedOrange),
	Pixel_ModRGB(88, Set, RGB_RedOrange),

	// Set 12
	Pixel_ModRGB(8, Set, RGB_Red),
	Pixel_ModRGB(26, Set, RGB_Red),
	Pixel_ModRGB(45, Set, RGB_Red),
	Pixel_ModRGB(63, Set, RGB_Red),
	Pixel_ModRGB(79, Set, RGB_Red),
	Pixel_ModRGB(89, Set, RGB_Red),

	// Set 13
	Pixel_ModRGB(9, Set, RGB_HalfRed),
	Pixel_ModRGB(27, Set, RGB_HalfRed),
	Pixel_ModRGB(46, Set, RGB_HalfRed),
	Pixel_ModRGB(64, Set, RGB_HalfRed),
};

const uint8_t rainbow_frame13[] = {
	// Set 1
	Pixel_ModRGB(84, Set, RGB_Violet),

	// Set 2
	Pixel_ModRGB(16, Set, RGB_IndigoViolet),
	Pixel_ModRGB(35, Set, RGB_IndigoViolet),
	Pixel_ModRGB(53, Set, RGB_IndigoViolet),
	Pixel_ModRGB(68, Set, RGB_IndigoViolet),
	Pixel_ModRGB(85, Set, RGB_IndigoViolet),

	// Set 3
	Pixel_ModRGB(0, Set, RGB_Indigo),
	Pixel_ModRGB(17, Set, RGB_Indigo),
	Pixel_ModRGB(36, Set, RGB_Indigo),
	Pixel_ModRGB(54, Set, RGB_Indigo),
	Pixel_ModRGB(70, Set, RGB_Indigo),
	Pixel_ModRGB(86, Set, RGB_Indigo),

	// Set 4
	Pixel_ModRGB(18, Set, RGB_BlueIndigo),
	Pixel_ModRGB(37, Set, RGB_BlueIndigo),
	Pixel_ModRGB(55, Set, RGB_BlueIndigo),
	Pixel_ModRGB(71, Set, RGB_BlueIndigo),

	// Set 5
	Pixel_ModRGB(1, Set, RGB_Blue),
	Pixel_ModRGB(19, Set, RGB_Blue),
	Pixel_ModRGB(38, Set, RGB_Blue),
	Pixel_ModRGB(56, Set, RGB_Blue),
	Pixel_ModRGB(72, Set, RGB_Blue),

	// Set 6
	Pixel_ModRGB(2, Set, RGB_GreenBlue),
	Pixel_ModRGB(20, Set, RGB_GreenBlue),
	Pixel_ModRGB(39, Set, RGB_GreenBlue),
	Pixel_ModRGB(57, Set, RGB_GreenBlue),
	Pixel_ModRGB(73, Set, RGB_GreenBlue),

	// Set 7
	Pixel_ModRGB(3, Set, RGB_Green),
	Pixel_ModRGB(21, Set, RGB_Green),
	Pixel_ModRGB(40, Set, RGB_Green),
	Pixel_ModRGB(58, Set, RGB_Green),
	Pixel_ModRGB(74, Set, RGB_Green),
	Pixel_ModRGB(87, Set, RGB_Green),

	// Set 8
	Pixel_ModRGB(4, Set, RGB_YellowGreen),
	Pixel_ModRGB(22, Set, RGB_YellowGreen),
	Pixel_ModRGB(41, Set, RGB_YellowGreen),
	Pixel_ModRGB(59, Set, RGB_YellowGreen),
	Pixel_ModRGB(75, Set, RGB_YellowGreen),

	// Set 9
	Pixel_ModRGB(5, Set, RGB_Yellow),
	Pixel_ModRGB(23, Set, RGB_Yellow),
	Pixel_ModRGB(42, Set, RGB_Yellow),
	Pixel_ModRGB(60, Set, RGB_Yellow),
	Pixel_ModRGB(76, Set, RGB_Yellow),

	// Set 10
	Pixel_ModRGB(6, Set, RGB_OrangeYellow),
	Pixel_ModRGB(24, Set, RGB_OrangeYellow),
	Pixel_ModRGB(43, Set, RGB_OrangeYellow),
	Pixel_ModRGB(61, Set, RGB_OrangeYellow),
	Pixel_ModRGB(77, Set, RGB_OrangeYellow),

	// Set 11
	Pixel_ModRGB(7, Set, RGB_Orange),
	Pixel_ModRGB(25, Set, RGB_Orange),
	Pixel_ModRGB(44, Set, RGB_Orange),
	Pixel_ModRGB(62, Set, RGB_Orange),
	Pixel_ModRGB(78, Set, RGB_Orange),
	Pixel_ModRGB(88, Set, RGB_Orange),

	// Set 12
	Pixel_ModRGB(8, Set, RGB_RedOrange),
	Pixel_ModRGB(26, Set, RGB_RedOrange),
	Pixel_ModRGB(45, Set, RGB_RedOrange),
	Pixel_ModRGB(63, Set, RGB_RedOrange),
	Pixel_ModRGB(79, Set, RGB_RedOrange),
	Pixel_ModRGB(89, Set, RGB_RedOrange),

	// Set 13
	Pixel_ModRGB(9, Set, RGB_Red),
	Pixel_ModRGB(27, Set, RGB_Red),
	Pixel_ModRGB(46, Set, RGB_Red),
	Pixel_ModRGB(64, Set, RGB_Red),

	// Set 14
	Pixel_ModRGB(10, Set, RGB_HalfRed),
	Pixel_ModRGB(28, Set, RGB_HalfRed),
	Pixel_ModRGB(47, Set, RGB_HalfRed),
	Pixel_ModRGB(90, Set, RGB_HalfRed),
};

const uint8_t rainbow_frame14[] = {
	// Set 1
	Pixel_ModRGB(84, Set, RGB_HalfViolet),

	// Set 2
	Pixel_ModRGB(16, Set, RGB_Violet),
	Pixel_ModRGB(35, Set, RGB_Violet),
	Pixel_ModRGB(53, Set, RGB_Violet),
	Pixel_ModRGB(68, Set, RGB_Violet),
	Pixel_ModRGB(85, Set, RGB_Violet),

	// Set 3
	Pixel_ModRGB(0, Set, RGB_IndigoViolet),
	Pixel_ModRGB(17, Set, RGB_IndigoViolet),
	Pixel_ModRGB(36, Set, RGB_IndigoViolet),
	Pixel_ModRGB(54, Set, RGB_IndigoViolet),
	Pixel_ModRGB(70, Set, RGB_IndigoViolet),
	Pixel_ModRGB(86, Set, RGB_IndigoViolet),

	// Set 4
	Pixel_ModRGB(18, Set, RGB_Indigo),
	Pixel_ModRGB(37, Set, RGB_Indigo),
	Pixel_ModRGB(55, Set, RGB_Indigo),
	Pixel_ModRGB(71, Set, RGB_Indigo),

	// Set 5
	Pixel_ModRGB(1, Set, RGB_BlueIndigo),
	Pixel_ModRGB(19, Set, RGB_BlueIndigo),
	Pixel_ModRGB(38, Set, RGB_BlueIndigo),
	Pixel_ModRGB(56, Set, RGB_BlueIndigo),
	Pixel_ModRGB(72, Set, RGB_BlueIndigo),

	// Set 6
	Pixel_ModRGB(2, Set, RGB_Blue),
	Pixel_ModRGB(20, Set, RGB_Blue),
	Pixel_ModRGB(39, Set, RGB_Blue),
	Pixel_ModRGB(57, Set, RGB_Blue),
	Pixel_ModRGB(73, Set, RGB_Blue),

	// Set 7
	Pixel_ModRGB(3, Set, RGB_GreenBlue),
	Pixel_ModRGB(21, Set, RGB_GreenBlue),
	Pixel_ModRGB(40, Set, RGB_GreenBlue),
	Pixel_ModRGB(58, Set, RGB_GreenBlue),
	Pixel_ModRGB(74, Set, RGB_GreenBlue),
	Pixel_ModRGB(87, Set, RGB_GreenBlue),

	// Set 8
	Pixel_ModRGB(4, Set, RGB_Green),
	Pixel_ModRGB(22, Set, RGB_Green),
	Pixel_ModRGB(41, Set, RGB_Green),
	Pixel_ModRGB(59, Set, RGB_Green),
	Pixel_ModRGB(75, Set, RGB_Green),

	// Set 9
	Pixel_ModRGB(5, Set, RGB_YellowGreen),
	Pixel_ModRGB(23, Set, RGB_YellowGreen),
	Pixel_ModRGB(42, Set, RGB_YellowGreen),
	Pixel_ModRGB(60, Set, RGB_YellowGreen),
	Pixel_ModRGB(76, Set, RGB_YellowGreen),

	// Set 10
	Pixel_ModRGB(6, Set, RGB_Yellow),
	Pixel_ModRGB(24, Set, RGB_Yellow),
	Pixel_ModRGB(43, Set, RGB_Yellow),
	Pixel_ModRGB(61, Set, RGB_Yellow),
	Pixel_ModRGB(77, Set, RGB_Yellow),

	// Set 11
	Pixel_ModRGB(7, Set, RGB_OrangeYellow),
	Pixel_ModRGB(25, Set, RGB_OrangeYellow),
	Pixel_ModRGB(44, Set, RGB_OrangeYellow),
	Pixel_ModRGB(62, Set, RGB_OrangeYellow),
	Pixel_ModRGB(78, Set, RGB_OrangeYellow),
	Pixel_ModRGB(88, Set, RGB_OrangeYellow),

	// Set 12
	Pixel_ModRGB(8, Set, RGB_Orange),
	Pixel_ModRGB(26, Set, RGB_Orange),
	Pixel_ModRGB(45, Set, RGB_Orange),
	Pixel_ModRGB(63, Set, RGB_Orange),
	Pixel_ModRGB(79, Set, RGB_Orange),
	Pixel_ModRGB(89, Set, RGB_Orange),

	// Set 13
	Pixel_ModRGB(9, Set, RGB_RedOrange),
	Pixel_ModRGB(27, Set, RGB_RedOrange),
	Pixel_ModRGB(46, Set, RGB_RedOrange),
	Pixel_ModRGB(64, Set, RGB_RedOrange),

	// Set 14
	Pixel_ModRGB(10, Set, RGB_Red),
	Pixel_ModRGB(28, Set, RGB_Red),
	Pixel_ModRGB(47, Set, RGB_Red),
	Pixel_ModRGB(90, Set, RGB_Red),

	// Set 15
	Pixel_ModRGB(11, Set, RGB_HalfRed),
	Pixel_ModRGB(30, Set, RGB_HalfRed),
	Pixel_ModRGB(66, Set, RGB_HalfRed),
	Pixel_ModRGB(81, Set, RGB_HalfRed),
};

const uint8_t rainbow_frame15[] = {
	// Set 1
	Pixel_ModRGB(84, Set, RGB_Black),

	// Set 2
	Pixel_ModRGB(16, Set, RGB_HalfViolet),
	Pixel_ModRGB(35, Set, RGB_HalfViolet),
	Pixel_ModRGB(53, Set, RGB_HalfViolet),
	Pixel_ModRGB(68, Set, RGB_HalfViolet),
	Pixel_ModRGB(85, Set, RGB_HalfViolet),

	// Set 3
	Pixel_ModRGB(0, Set, RGB_Violet),
	Pixel_ModRGB(17, Set, RGB_Violet),
	Pixel_ModRGB(36, Set, RGB_Violet),
	Pixel_ModRGB(54, Set, RGB_Violet),
	Pixel_ModRGB(70, Set, RGB_Violet),
	Pixel_ModRGB(86, Set, RGB_Violet),

	// Set 4
	Pixel_ModRGB(18, Set, RGB_IndigoViolet),
	Pixel_ModRGB(37, Set, RGB_IndigoViolet),
	Pixel_ModRGB(55, Set, RGB_IndigoViolet),
	Pixel_ModRGB(71, Set, RGB_IndigoViolet),

	// Set 5
	Pixel_ModRGB(1, Set, RGB_Indigo),
	Pixel_ModRGB(19, Set, RGB_Indigo),
	Pixel_ModRGB(38, Set, RGB_Indigo),
	Pixel_ModRGB(56, Set, RGB_Indigo),
	Pixel_ModRGB(72, Set, RGB_Indigo),

	// Set 6
	Pixel_ModRGB(2, Set, RGB_BlueIndigo),
	Pixel_ModRGB(20, Set, RGB_BlueIndigo),
	Pixel_ModRGB(39, Set, RGB_BlueIndigo),
	Pixel_ModRGB(57, Set, RGB_BlueIndigo),
	Pixel_ModRGB(73, Set, RGB_BlueIndigo),

	// Set 7
	Pixel_ModRGB(3, Set, RGB_Blue),
	Pixel_ModRGB(21, Set, RGB_Blue),
	Pixel_ModRGB(40, Set, RGB_Blue),
	Pixel_ModRGB(58, Set, RGB_Blue),
	Pixel_ModRGB(74, Set, RGB_Blue),
	Pixel_ModRGB(87, Set, RGB_Blue),

	// Set 8
	Pixel_ModRGB(4, Set, RGB_GreenBlue),
	Pixel_ModRGB(22, Set, RGB_GreenBlue),
	Pixel_ModRGB(41, Set, RGB_GreenBlue),
	Pixel_ModRGB(59, Set, RGB_GreenBlue),
	Pixel_ModRGB(75, Set, RGB_GreenBlue),

	// Set 9
	Pixel_ModRGB(5, Set, RGB_Green),
	Pixel_ModRGB(23, Set, RGB_Green),
	Pixel_ModRGB(42, Set, RGB_Green),
	Pixel_ModRGB(60, Set, RGB_Green),
	Pixel_ModRGB(76, Set, RGB_Green),

	// Set 10
	Pixel_ModRGB(6, Set, RGB_YellowGreen),
	Pixel_ModRGB(24, Set, RGB_YellowGreen),
	Pixel_ModRGB(43, Set, RGB_YellowGreen),
	Pixel_ModRGB(61, Set, RGB_YellowGreen),
	Pixel_ModRGB(77, Set, RGB_YellowGreen),

	// Set 11
	Pixel_ModRGB(7, Set, RGB_Yellow),
	Pixel_ModRGB(25, Set, RGB_Yellow),
	Pixel_ModRGB(44, Set, RGB_Yellow),
	Pixel_ModRGB(62, Set, RGB_Yellow),
	Pixel_ModRGB(78, Set, RGB_Yellow),
	Pixel_ModRGB(88, Set, RGB_Yellow),

	// Set 12
	Pixel_ModRGB(8, Set, RGB_OrangeYellow),
	Pixel_ModRGB(26, Set, RGB_OrangeYellow),
	Pixel_ModRGB(45, Set, RGB_OrangeYellow),
	Pixel_ModRGB(63, Set, RGB_OrangeYellow),
	Pixel_ModRGB(79, Set, RGB_OrangeYellow),
	Pixel_ModRGB(89, Set, RGB_OrangeYellow),

	// Set 13
	Pixel_ModRGB(9, Set, RGB_Orange),
	Pixel_ModRGB(27, Set, RGB_Orange),
	Pixel_ModRGB(46, Set, RGB_Orange),
	Pixel_ModRGB(64, Set, RGB_Orange),

	// Set 14
	Pixel_ModRGB(10, Set, RGB_RedOrange),
	Pixel_ModRGB(28, Set, RGB_RedOrange),
	Pixel_ModRGB(47, Set, RGB_RedOrange),
	Pixel_ModRGB(90, Set, RGB_RedOrange),

	// Set 15
	Pixel_ModRGB(11, Set, RGB_Red),
	Pixel_ModRGB(30, Set, RGB_Red),
	Pixel_ModRGB(66, Set, RGB_Red),
	Pixel_ModRGB(81, Set, RGB_Red),

	// Set 16
	Pixel_ModRGB(12, Set, RGB_HalfRed),
	Pixel_ModRGB(48, Set, RGB_HalfRed),
	Pixel_ModRGB(91, Set, RGB_HalfRed),
};

const uint8_t rainbow_frame16[] = {
	// Set 2
	Pixel_ModRGB(16, Set, RGB_Black),
	Pixel_ModRGB(35, Set, RGB_Black),
	Pixel_ModRGB(53, Set, RGB_Black),
	Pixel_ModRGB(68, Set, RGB_Black),
	Pixel_ModRGB(85, Set, RGB_Black),

	// Set 3
	Pixel_ModRGB(0, Set, RGB_HalfViolet),
	Pixel_ModRGB(17, Set, RGB_HalfViolet),
	Pixel_ModRGB(36, Set, RGB_HalfViolet),
	Pixel_ModRGB(54, Set, RGB_HalfViolet),
	Pixel_ModRGB(70, Set, RGB_HalfViolet),
	Pixel_ModRGB(86, Set, RGB_HalfViolet),

	// Set 4
	Pixel_ModRGB(18, Set, RGB_Violet),
	Pixel_ModRGB(37, Set, RGB_Violet),
	Pixel_ModRGB(55, Set, RGB_Violet),
	Pixel_ModRGB(71, Set, RGB_Violet),

	// Set 5
	Pixel_ModRGB(1, Set, RGB_IndigoViolet),
	Pixel_ModRGB(19, Set, RGB_IndigoViolet),
	Pixel_ModRGB(38, Set, RGB_IndigoViolet),
	Pixel_ModRGB(56, Set, RGB_IndigoViolet),
	Pixel_ModRGB(72, Set, RGB_IndigoViolet),

	// Set 6
	Pixel_ModRGB(2, Set, RGB_Indigo),
	Pixel_ModRGB(20, Set, RGB_Indigo),
	Pixel_ModRGB(39, Set, RGB_Indigo),
	Pixel_ModRGB(57, Set, RGB_Indigo),
	Pixel_ModRGB(73, Set, RGB_Indigo),

	// Set 7
	Pixel_ModRGB(3, Set, RGB_BlueIndigo),
	Pixel_ModRGB(21, Set, RGB_BlueIndigo),
	Pixel_ModRGB(40, Set, RGB_BlueIndigo),
	Pixel_ModRGB(58, Set, RGB_BlueIndigo),
	Pixel_ModRGB(74, Set, RGB_BlueIndigo),
	Pixel_ModRGB(87, Set, RGB_BlueIndigo),

	// Set 8
	Pixel_ModRGB(4, Set, RGB_Blue),
	Pixel_ModRGB(22, Set, RGB_Blue),
	Pixel_ModRGB(41, Set, RGB_Blue),
	Pixel_ModRGB(59, Set, RGB_Blue),
	Pixel_ModRGB(75, Set, RGB_Blue),

	// Set 9
	Pixel_ModRGB(5, Set, RGB_GreenBlue),
	Pixel_ModRGB(23, Set, RGB_GreenBlue),
	Pixel_ModRGB(42, Set, RGB_GreenBlue),
	Pixel_ModRGB(60, Set, RGB_GreenBlue),
	Pixel_ModRGB(76, Set, RGB_GreenBlue),

	// Set 10
	Pixel_ModRGB(6, Set, RGB_Green),
	Pixel_ModRGB(24, Set, RGB_Green),
	Pixel_ModRGB(43, Set, RGB_Green),
	Pixel_ModRGB(61, Set, RGB_Green),
	Pixel_ModRGB(77, Set, RGB_Green),

	// Set 11
	Pixel_ModRGB(7, Set, RGB_YellowGreen),
	Pixel_ModRGB(25, Set, RGB_YellowGreen),
	Pixel_ModRGB(44, Set, RGB_YellowGreen),
	Pixel_ModRGB(62, Set, RGB_YellowGreen),
	Pixel_ModRGB(78, Set, RGB_YellowGreen),
	Pixel_ModRGB(88, Set, RGB_YellowGreen),

	// Set 12
	Pixel_ModRGB(8, Set, RGB_Yellow),
	Pixel_ModRGB(26, Set, RGB_Yellow),
	Pixel_ModRGB(45, Set, RGB_Yellow),
	Pixel_ModRGB(63, Set, RGB_Yellow),
	Pixel_ModRGB(79, Set, RGB_Yellow),
	Pixel_ModRGB(89, Set, RGB_Yellow),

	// Set 13
	Pixel_ModRGB(9, Set, RGB_OrangeYellow),
	Pixel_ModRGB(27, Set, RGB_OrangeYellow),
	Pixel_ModRGB(46, Set, RGB_OrangeYellow),
	Pixel_ModRGB(64, Set, RGB_OrangeYellow),

	// Set 14
	Pixel_ModRGB(10, Set, RGB_Orange),
	Pixel_ModRGB(28, Set, RGB_Orange),
	Pixel_ModRGB(47, Set, RGB_Orange),
	Pixel_ModRGB(90, Set, RGB_Orange),

	// Set 15
	Pixel_ModRGB(11, Set, RGB_RedOrange),
	Pixel_ModRGB(30, Set, RGB_RedOrange),
	Pixel_ModRGB(66, Set, RGB_RedOrange),
	Pixel_ModRGB(81, Set, RGB_RedOrange),

	// Set 16
	Pixel_ModRGB(12, Set, RGB_Red),
	Pixel_ModRGB(48, Set, RGB_Red),
	Pixel_ModRGB(91, Set, RGB_Red),

	// Set 17
	Pixel_ModRGB(13, Set, RGB_HalfRed),
	Pixel_ModRGB(32, Set, RGB_HalfRed),
	Pixel_ModRGB(50, Set, RGB_HalfRed),
	Pixel_ModRGB(92, Set, RGB_HalfRed),
};

const uint8_t rainbow_frame17[] = {
	// Set 3
	Pixel_ModRGB(0, Set, RGB_Black),
	Pixel_ModRGB(17, Set, RGB_Black),
	Pixel_ModRGB(36, Set, RGB_Black),
	Pixel_ModRGB(54, Set, RGB_Black),
	Pixel_ModRGB(70, Set, RGB_Black),
	Pixel_ModRGB(86, Set, RGB_Black),

	// Set 4
	Pixel_ModRGB(18, Set, RGB_HalfViolet),
	Pixel_ModRGB(37, Set, RGB_HalfViolet),
	Pixel_ModRGB(55, Set, RGB_HalfViolet),
	Pixel_ModRGB(71, Set, RGB_HalfViolet),

	// Set 5
	Pixel_ModRGB(1, Set, RGB_Violet),
	Pixel_ModRGB(19, Set, RGB_Violet),
	Pixel_ModRGB(38, Set, RGB_Violet),
	Pixel_ModRGB(56, Set, RGB_Violet),
	Pixel_ModRGB(72, Set, RGB_Violet),

	// Set 6
	Pixel_ModRGB(2, Set, RGB_IndigoViolet),
	Pixel_ModRGB(20, Set, RGB_IndigoViolet),
	Pixel_ModRGB(39, Set, RGB_IndigoViolet),
	Pixel_ModRGB(57, Set, RGB_IndigoViolet),
	Pixel_ModRGB(73, Set, RGB_IndigoViolet),

	// Set 7
	Pixel_ModRGB(3, Set, RGB_Indigo),
	Pixel_ModRGB(21, Set, RGB_Indigo),
	Pixel_ModRGB(40, Set, RGB_Indigo),
	Pixel_ModRGB(58, Set, RGB_Indigo),
	Pixel_ModRGB(74, Set, RGB_Indigo),
	Pixel_ModRGB(87, Set, RGB_Indigo),

	// Set 8
	Pixel_ModRGB(4, Set, RGB_BlueIndigo),
	Pixel_ModRGB(22, Set, RGB_BlueIndigo),
	Pixel_ModRGB(41, Set, RGB_BlueIndigo),
	Pixel_ModRGB(59, Set, RGB_BlueIndigo),
	Pixel_ModRGB(75, Set, RGB_BlueIndigo),

	// Set 9
	Pixel_ModRGB(5, Set, RGB_Blue),
	Pixel_ModRGB(23, Set, RGB_Blue),
	Pixel_ModRGB(42, Set, RGB_Blue),
	Pixel_ModRGB(60, Set, RGB_Blue),
	Pixel_ModRGB(76, Set, RGB_Blue),

	// Set 10
	Pixel_ModRGB(6, Set, RGB_GreenBlue),
	Pixel_ModRGB(24, Set, RGB_GreenBlue),
	Pixel_ModRGB(43, Set, RGB_GreenBlue),
	Pixel_ModRGB(61, Set, RGB_GreenBlue),
	Pixel_ModRGB(77, Set, RGB_GreenBlue),

	// Set 11
	Pixel_ModRGB(7, Set, RGB_Green),
	Pixel_ModRGB(25, Set, RGB_Green),
	Pixel_ModRGB(44, Set, RGB_Green),
	Pixel_ModRGB(62, Set, RGB_Green),
	Pixel_ModRGB(78, Set, RGB_Green),
	Pixel_ModRGB(88, Set, RGB_Green),

	// Set 12
	Pixel_ModRGB(8, Set, RGB_YellowGreen),
	Pixel_ModRGB(26, Set, RGB_YellowGreen),
	Pixel_ModRGB(45, Set, RGB_YellowGreen),
	Pixel_ModRGB(63, Set, RGB_YellowGreen),
	Pixel_ModRGB(79, Set, RGB_YellowGreen),
	Pixel_ModRGB(89, Set, RGB_YellowGreen),

	// Set 13
	Pixel_ModRGB(9, Set, RGB_Yellow),
	Pixel_ModRGB(27, Set, RGB_Yellow),
	Pixel_ModRGB(46, Set, RGB_Yellow),
	Pixel_ModRGB(64, Set, RGB_Yellow),

	// Set 14
	Pixel_ModRGB(10, Set, RGB_OrangeYellow),
	Pixel_ModRGB(28, Set, RGB_OrangeYellow),
	Pixel_ModRGB(47, Set, RGB_OrangeYellow),
	Pixel_ModRGB(90, Set, RGB_OrangeYellow),

	// Set 15
	Pixel_ModRGB(11, Set, RGB_Orange),
	Pixel_ModRGB(30, Set, RGB_Orange),
	Pixel_ModRGB(66, Set, RGB_Orange),
	Pixel_ModRGB(81, Set, RGB_Orange),

	// Set 16
	Pixel_ModRGB(12, Set, RGB_RedOrange),
	Pixel_ModRGB(48, Set, RGB_RedOrange),
	Pixel_ModRGB(91, Set, RGB_RedOrange),

	// Set 17
	Pixel_ModRGB(13, Set, RGB_Red),
	Pixel_ModRGB(32, Set, RGB_Red),
	Pixel_ModRGB(50, Set, RGB_Red),
	Pixel_ModRGB(92, Set, RGB_Red),

	// Set 18
	Pixel_ModRGB(14, Set, RGB_HalfRed),
	Pixel_ModRGB(33, Set, RGB_HalfRed),
	Pixel_ModRGB(51, Set, RGB_HalfRed),
	Pixel_ModRGB(83, Set, RGB_HalfRed),
	Pixel_ModRGB(93, Set, RGB_HalfRed),
};

const uint8_t rainbow_frame18[] = {
	// Set 4
	Pixel_ModRGB(18, Set, RGB_Black),
	Pixel_ModRGB(37, Set, RGB_Black),
	Pixel_ModRGB(55, Set, RGB_Black),
	Pixel_ModRGB(71, Set, RGB_Black),

	// Set 5
	Pixel_ModRGB(1, Set, RGB_HalfViolet),
	Pixel_ModRGB(19, Set, RGB_HalfViolet),
	Pixel_ModRGB(38, Set, RGB_HalfViolet),
	Pixel_ModRGB(56, Set, RGB_HalfViolet),
	Pixel_ModRGB(72, Set, RGB_HalfViolet),

	// Set 6
	Pixel_ModRGB(2, Set, RGB_Violet),
	Pixel_ModRGB(20, Set, RGB_Violet),
	Pixel_ModRGB(39, Set, RGB_Violet),
	Pixel_ModRGB(57, Set, RGB_Violet),
	Pixel_ModRGB(73, Set, RGB_Violet),

	// Set 7
	Pixel_ModRGB(3, Set, RGB_IndigoViolet),
	Pixel_ModRGB(21, Set, RGB_IndigoViolet),
	Pixel_ModRGB(40, Set, RGB_IndigoViolet),
	Pixel_ModRGB(58, Set, RGB_IndigoViolet),
	Pixel_ModRGB(74, Set, RGB_IndigoViolet),
	Pixel_ModRGB(87, Set, RGB_IndigoViolet),

	// Set 8
	Pixel_ModRGB(4, Set, RGB_Indigo),
	Pixel_ModRGB(22, Set, RGB_Indigo),
	Pixel_ModRGB(41, Set, RGB_Indigo),
	Pixel_ModRGB(59, Set, RGB_Indigo),
	Pixel_ModRGB(75, Set, RGB_Indigo),

	// Set 9
	Pixel_ModRGB(5, Set, RGB_BlueIndigo),
	Pixel_ModRGB(23, Set, RGB_BlueIndigo),
	Pixel_ModRGB(42, Set, RGB_BlueIndigo),
	Pixel_ModRGB(60, Set, RGB_BlueIndigo),
	Pixel_ModRGB(76, Set, RGB_BlueIndigo),

	// Set 10
	Pixel_ModRGB(6, Set, RGB_Blue),
	Pixel_ModRGB(24, Set, RGB_Blue),
	Pixel_ModRGB(43, Set, RGB_Blue),
	Pixel_ModRGB(61, Set, RGB_Blue),
	Pixel_ModRGB(77, Set, RGB_Blue),

	// Set 11
	Pixel_ModRGB(7, Set, RGB_GreenBlue),
	Pixel_ModRGB(25, Set, RGB_GreenBlue),
	Pixel_ModRGB(44, Set, RGB_GreenBlue),
	Pixel_ModRGB(62, Set, RGB_GreenBlue),
	Pixel_ModRGB(78, Set, RGB_GreenBlue),
	Pixel_ModRGB(88, Set, RGB_GreenBlue),

	// Set 12
	Pixel_ModRGB(8, Set, RGB_Green),
	Pixel_ModRGB(26, Set, RGB_Green),
	Pixel_ModRGB(45, Set, RGB_Green),
	Pixel_ModRGB(63, Set, RGB_Green),
	Pixel_ModRGB(79, Set, RGB_Green),
	Pixel_ModRGB(89, Set, RGB_Green),

	// Set 13
	Pixel_ModRGB(9, Set, RGB_YellowGreen),
	Pixel_ModRGB(27, Set, RGB_YellowGreen),
	Pixel_ModRGB(46, Set, RGB_YellowGreen),
	Pixel_ModRGB(64, Set, RGB_YellowGreen),

	// Set 14
	Pixel_ModRGB(10, Set, RGB_Yellow),
	Pixel_ModRGB(28, Set, RGB_Yellow),
	Pixel_ModRGB(47, Set, RGB_Yellow),
	Pixel_ModRGB(90, Set, RGB_Yellow),

	// Set 15
	Pixel_ModRGB(11, Set, RGB_OrangeYellow),
	Pixel_ModRGB(30, Set, RGB_OrangeYellow),
	Pixel_ModRGB(66, Set, RGB_OrangeYellow),
	Pixel_ModRGB(81, Set, RGB_OrangeYellow),

	// Set 16
	Pixel_ModRGB(12, Set, RGB_Orange),
	Pixel_ModRGB(48, Set, RGB_Orange),
	Pixel_ModRGB(91, Set, RGB_Orange),

	// Set 17
	Pixel_ModRGB(13, Set, RGB_RedOrange),
	Pixel_ModRGB(32, Set, RGB_RedOrange),
	Pixel_ModRGB(50, Set, RGB_RedOrange),
	Pixel_ModRGB(92, Set, RGB_RedOrange),

	// Set 18
	Pixel_ModRGB(14, Set, RGB_Red),
	Pixel_ModRGB(33, Set, RGB_Red),
	Pixel_ModRGB(51, Set, RGB_Red),
	Pixel_ModRGB(83, Set, RGB_Red),
	Pixel_ModRGB(93, Set, RGB_Red),

	// Set 19
	Pixel_ModRGB(15, Set, RGB_HalfRed),
	Pixel_ModRGB(34, Set, RGB_HalfRed),
	Pixel_ModRGB(52, Set, RGB_HalfRed),
	Pixel_ModRGB(94, Set, RGB_HalfRed),
};

const uint8_t rainbow_frame19[] = {
	// Set 5
	Pixel_ModRGB(1, Set, RGB_Black),
	Pixel_ModRGB(19, Set, RGB_Black),
	Pixel_ModRGB(38, Set, RGB_Black),
	Pixel_ModRGB(56, Set, RGB_Black),
	Pixel_ModRGB(72, Set, RGB_Black),

	// Set 6
	Pixel_ModRGB(2, Set, RGB_HalfViolet),
	Pixel_ModRGB(20, Set, RGB_HalfViolet),
	Pixel_ModRGB(39, Set, RGB_HalfViolet),
	Pixel_ModRGB(57, Set, RGB_HalfViolet),
	Pixel_ModRGB(73, Set, RGB_HalfViolet),

	// Set 7
	Pixel_ModRGB(3, Set, RGB_Violet),
	Pixel_ModRGB(21, Set, RGB_Violet),
	Pixel_ModRGB(40, Set, RGB_Violet),
	Pixel_ModRGB(58, Set, RGB_Violet),
	Pixel_ModRGB(74, Set, RGB_Violet),
	Pixel_ModRGB(87, Set, RGB_Violet),

	// Set 8
	Pixel_ModRGB(4, Set, RGB_IndigoViolet),
	Pixel_ModRGB(22, Set, RGB_IndigoViolet),
	Pixel_ModRGB(41, Set, RGB_IndigoViolet),
	Pixel_ModRGB(59, Set, RGB_IndigoViolet),
	Pixel_ModRGB(75, Set, RGB_IndigoViolet),

	// Set 9
	Pixel_ModRGB(5, Set, RGB_Indigo),
	Pixel_ModRGB(23, Set, RGB_Indigo),
	Pixel_ModRGB(42, Set, RGB_Indigo),
	Pixel_ModRGB(60, Set, RGB_Indigo),
	Pixel_ModRGB(76, Set, RGB_Indigo),

	// Set 10
	Pixel_ModRGB(6, Set, RGB_BlueIndigo),
	Pixel_ModRGB(24, Set, RGB_BlueIndigo),
	Pixel_ModRGB(43, Set, RGB_BlueIndigo),
	Pixel_ModRGB(61, Set, RGB_BlueIndigo),
	Pixel_ModRGB(77, Set, RGB_BlueIndigo),

	// Set 11
	Pixel_ModRGB(7, Set, RGB_Blue),
	Pixel_ModRGB(25, Set, RGB_Blue),
	Pixel_ModRGB(44, Set, RGB_Blue),
	Pixel_ModRGB(62, Set, RGB_Blue),
	Pixel_ModRGB(78, Set, RGB_Blue),
	Pixel_ModRGB(88, Set, RGB_Blue),

	// Set 12
	Pixel_ModRGB(8, Set, RGB_GreenBlue),
	Pixel_ModRGB(26, Set, RGB_GreenBlue),
	Pixel_ModRGB(45, Set, RGB_GreenBlue),
	Pixel_ModRGB(63, Set, RGB_GreenBlue),
	Pixel_ModRGB(79, Set, RGB_GreenBlue),
	Pixel_ModRGB(89, Set, RGB_GreenBlue),

	// Set 13
	Pixel_ModRGB(9, Set, RGB_Green),
	Pixel_ModRGB(27, Set, RGB_Green),
	Pixel_ModRGB(46, Set, RGB_Green),
	Pixel_ModRGB(64, Set, RGB_Green),

	// Set 14
	Pixel_ModRGB(10, Set, RGB_YellowGreen),
	Pixel_ModRGB(28, Set, RGB_YellowGreen),
	Pixel_ModRGB(47, Set, RGB_YellowGreen),
	Pixel_ModRGB(90, Set, RGB_YellowGreen),

	// Set 15
	Pixel_ModRGB(11, Set, RGB_Yellow),
	Pixel_ModRGB(30, Set, RGB_Yellow),
	Pixel_ModRGB(66, Set, RGB_Yellow),
	Pixel_ModRGB(81, Set, RGB_Yellow),

	// Set 16
	Pixel_ModRGB(12, Set, RGB_OrangeYellow),
	Pixel_ModRGB(48, Set, RGB_OrangeYellow),
	Pixel_ModRGB(91, Set, RGB_OrangeYellow),

	// Set 17
	Pixel_ModRGB(13, Set, RGB_Orange),
	Pixel_ModRGB(32, Set, RGB_Orange),
	Pixel_ModRGB(50, Set, RGB_Orange),
	Pixel_ModRGB(92, Set, RGB_Orange),

	// Set 18
	Pixel_ModRGB(14, Set, RGB_RedOrange),
	Pixel_ModRGB(33, Set, RGB_RedOrange),
	Pixel_ModRGB(51, Set, RGB_RedOrange),
	Pixel_ModRGB(83, Set, RGB_RedOrange),
	Pixel_ModRGB(93, Set, RGB_RedOrange),

	// Set 19
	Pixel_ModRGB(15, Set, RGB_Red),
	Pixel_ModRGB(34, Set, RGB_Red),
	Pixel_ModRGB(52, Set, RGB_Red),
	Pixel_ModRGB(94, Set, RGB_Red),
};

const uint8_t rainbow_frame20[] = {
	// Set 6
	Pixel_ModRGB(2, Set, RGB_Black),
	Pixel_ModRGB(20, Set, RGB_Black),
	Pixel_ModRGB(39, Set, RGB_Black),
	Pixel_ModRGB(57, Set, RGB_Black),
	Pixel_ModRGB(73, Set, RGB_Black),

	// Set 7
	Pixel_ModRGB(3, Set, RGB_HalfViolet),
	Pixel_ModRGB(21, Set, RGB_HalfViolet),
	Pixel_ModRGB(40, Set, RGB_HalfViolet),
	Pixel_ModRGB(58, Set, RGB_HalfViolet),
	Pixel_ModRGB(74, Set, RGB_HalfViolet),
	Pixel_ModRGB(87, Set, RGB_HalfViolet),

	// Set 8
	Pixel_ModRGB(4, Set, RGB_Violet),
	Pixel_ModRGB(22, Set, RGB_Violet),
	Pixel_ModRGB(41, Set, RGB_Violet),
	Pixel_ModRGB(59, Set, RGB_Violet),
	Pixel_ModRGB(75, Set, RGB_Violet),

	// Set 9
	Pixel_ModRGB(5, Set, RGB_IndigoViolet),
	Pixel_ModRGB(23, Set, RGB_IndigoViolet),
	Pixel_ModRGB(42, Set, RGB_IndigoViolet),
	Pixel_ModRGB(60, Set, RGB_IndigoViolet),
	Pixel_ModRGB(76, Set, RGB_IndigoViolet),

	// Set 10
	Pixel_ModRGB(6, Set, RGB_Indigo),
	Pixel_ModRGB(24, Set, RGB_Indigo),
	Pixel_ModRGB(43, Set, RGB_Indigo),
	Pixel_ModRGB(61, Set, RGB_Indigo),
	Pixel_ModRGB(77, Set, RGB_Indigo),

	// Set 11
	Pixel_ModRGB(7, Set, RGB_BlueIndigo),
	Pixel_ModRGB(25, Set, RGB_BlueIndigo),
	Pixel_ModRGB(44, Set, RGB_BlueIndigo),
	Pixel_ModRGB(62, Set, RGB_BlueIndigo),
	Pixel_ModRGB(78, Set, RGB_BlueIndigo),
	Pixel_ModRGB(88, Set, RGB_BlueIndigo),

	// Set 12
	Pixel_ModRGB(8, Set, RGB_Blue),
	Pixel_ModRGB(26, Set, RGB_Blue),
	Pixel_ModRGB(45, Set, RGB_Blue),
	Pixel_ModRGB(63, Set, RGB_Blue),
	Pixel_ModRGB(79, Set, RGB_Blue),
	Pixel_ModRGB(89, Set, RGB_Blue),

	// Set 13
	Pixel_ModRGB(9, Set, RGB_GreenBlue),
	Pixel_ModRGB(27, Set, RGB_GreenBlue),
	Pixel_ModRGB(46, Set, RGB_GreenBlue),
	Pixel_ModRGB(64, Set, RGB_GreenBlue),

	// Set 14
	Pixel_ModRGB(10, Set, RGB_Green),
	Pixel_ModRGB(28, Set, RGB_Green),
	Pixel_ModRGB(47, Set, RGB_Green),
	Pixel_ModRGB(90, Set, RGB_Green),

	// Set 15
	Pixel_ModRGB(11, Set, RGB_YellowGreen),
	Pixel_ModRGB(30, Set, RGB_YellowGreen),
	Pixel_ModRGB(66, Set, RGB_YellowGreen),
	Pixel_ModRGB(81, Set, RGB_YellowGreen),

	// Set 16
	Pixel_ModRGB(12, Set, RGB_Yellow),
	Pixel_ModRGB(48, Set, RGB_Yellow),
	Pixel_ModRGB(91, Set, RGB_Yellow),

	// Set 17
	Pixel_ModRGB(13, Set, RGB_OrangeYellow),
	Pixel_ModRGB(32, Set, RGB_OrangeYellow),
	Pixel_ModRGB(50, Set, RGB_OrangeYellow),
	Pixel_ModRGB(92, Set, RGB_OrangeYellow),

	// Set 18
	Pixel_ModRGB(14, Set, RGB_Orange),
	Pixel_ModRGB(33, Set, RGB_Orange),
	Pixel_ModRGB(51, Set, RGB_Orange),
	Pixel_ModRGB(83, Set, RGB_Orange),
	Pixel_ModRGB(93, Set, RGB_Orange),

	// Set 19
	Pixel_ModRGB(15, Set, RGB_RedOrange),
	Pixel_ModRGB(34, Set, RGB_RedOrange),
	Pixel_ModRGB(52, Set, RGB_RedOrange),
	Pixel_ModRGB(94, Set, RGB_RedOrange),
};

const uint8_t rainbow_frame21[] = {
	// Set 7
	Pixel_ModRGB(3, Set, RGB_Black),
	Pixel_ModRGB(21, Set, RGB_Black),
	Pixel_ModRGB(40, Set, RGB_Black),
	Pixel_ModRGB(58, Set, RGB_Black),
	Pixel_ModRGB(74, Set, RGB_Black),
	Pixel_ModRGB(87, Set, RGB_Black),

	// Set 8
	Pixel_ModRGB(4, Set, RGB_HalfViolet),
	Pixel_ModRGB(22, Set, RGB_HalfViolet),
	Pixel_ModRGB(41, Set, RGB_HalfViolet),
	Pixel_ModRGB(59, Set, RGB_HalfViolet),
	Pixel_ModRGB(75, Set, RGB_HalfViolet),

	// Set 9
	Pixel_ModRGB(5, Set, RGB_Violet),
	Pixel_ModRGB(23, Set, RGB_Violet),
	Pixel_ModRGB(42, Set, RGB_Violet),
	Pixel_ModRGB(60, Set, RGB_Violet),
	Pixel_ModRGB(76, Set, RGB_Violet),

	// Set 10
	Pixel_ModRGB(6, Set, RGB_IndigoViolet),
	Pixel_ModRGB(24, Set, RGB_IndigoViolet),
	Pixel_ModRGB(43, Set, RGB_IndigoViolet),
	Pixel_ModRGB(61, Set, RGB_IndigoViolet),
	Pixel_ModRGB(77, Set, RGB_IndigoViolet),

	// Set 11
	Pixel_ModRGB(7, Set, RGB_Indigo),
	Pixel_ModRGB(25, Set, RGB_Indigo),
	Pixel_ModRGB(44, Set, RGB_Indigo),
	Pixel_ModRGB(62, Set, RGB_Indigo),
	Pixel_ModRGB(78, Set, RGB_Indigo),
	Pixel_ModRGB(88, Set, RGB_Indigo),

	// Set 12
	Pixel_ModRGB(8, Set, RGB_BlueIndigo),
	Pixel_ModRGB(26, Set, RGB_BlueIndigo),
	Pixel_ModRGB(45, Set, RGB_BlueIndigo),
	Pixel_ModRGB(63, Set, RGB_BlueIndigo),
	Pixel_ModRGB(79, Set, RGB_BlueIndigo),
	Pixel_ModRGB(89, Set, RGB_BlueIndigo),

	// Set 13
	Pixel_ModRGB(9, Set, RGB_Blue),
	Pixel_ModRGB(27, Set, RGB_Blue),
	Pixel_ModRGB(46, Set, RGB_Blue),
	Pixel_ModRGB(64, Set, RGB_Blue),

	// Set 14
	Pixel_ModRGB(10, Set, RGB_GreenBlue),
	Pixel_ModRGB(28, Set, RGB_GreenBlue),
	Pixel_ModRGB(47, Set, RGB_GreenBlue),
	Pixel_ModRGB(90, Set, RGB_GreenBlue),

	// Set 15
	Pixel_ModRGB(11, Set, RGB_Green),
	Pixel_ModRGB(30, Set, RGB_Green),
	Pixel_ModRGB(66, Set, RGB_Green),
	Pixel_ModRGB(81, Set, RGB_Green),

	// Set 16
	Pixel_ModRGB(12, Set, RGB_YellowGreen),
	Pixel_ModRGB(48, Set, RGB_YellowGreen),
	Pixel_ModRGB(91, Set, RGB_YellowGreen),

	// Set 17
	Pixel_ModRGB(13, Set, RGB_Yellow),
	Pixel_ModRGB(32, Set, RGB_Yellow),
	Pixel_ModRGB(50, Set, RGB_Yellow),
	Pixel_ModRGB(92, Set, RGB_Yellow),

	// Set 18
	Pixel_ModRGB(14, Set, RGB_OrangeYellow),
	Pixel_ModRGB(33, Set, RGB_OrangeYellow),
	Pixel_ModRGB(51, Set, RGB_OrangeYellow),
	Pixel_ModRGB(83, Set, RGB_OrangeYellow),
	Pixel_ModRGB(93, Set, RGB_OrangeYellow),

	// Set 19
	Pixel_ModRGB(15, Set, RGB_Orange),
	Pixel_ModRGB(34, Set, RGB_Orange),
	Pixel_ModRGB(52, Set, RGB_Orange),
	Pixel_ModRGB(94, Set, RGB_Orange),
};

const uint8_t rainbow_frame22[] = {
	// Set 8
	Pixel_ModRGB(4, Set, RGB_Black),
	Pixel_ModRGB(22, Set, RGB_Black),
	Pixel_ModRGB(41, Set, RGB_Black),
	Pixel_ModRGB(59, Set, RGB_Black),
	Pixel_ModRGB(75, Set, RGB_Black),

	// Set 9
	Pixel_ModRGB(5, Set, RGB_HalfViolet),
	Pixel_ModRGB(23, Set, RGB_HalfViolet),
	Pixel_ModRGB(42, Set, RGB_HalfViolet),
	Pixel_ModRGB(60, Set, RGB_HalfViolet),
	Pixel_ModRGB(76, Set, RGB_HalfViolet),

	// Set 10
	Pixel_ModRGB(6, Set, RGB_Violet),
	Pixel_ModRGB(24, Set, RGB_Violet),
	Pixel_ModRGB(43, Set, RGB_Violet),
	Pixel_ModRGB(61, Set, RGB_Violet),
	Pixel_ModRGB(77, Set, RGB_Violet),

	// Set 11
	Pixel_ModRGB(7, Set, RGB_IndigoViolet),
	Pixel_ModRGB(25, Set, RGB_IndigoViolet),
	Pixel_ModRGB(44, Set, RGB_IndigoViolet),
	Pixel_ModRGB(62, Set, RGB_IndigoViolet),
	Pixel_ModRGB(78, Set, RGB_IndigoViolet),
	Pixel_ModRGB(88, Set, RGB_IndigoViolet),

	// Set 12
	Pixel_ModRGB(8, Set, RGB_Indigo),
	Pixel_ModRGB(26, Set, RGB_Indigo),
	Pixel_ModRGB(45, Set, RGB_Indigo),
	Pixel_ModRGB(63, Set, RGB_Indigo),
	Pixel_ModRGB(79, Set, RGB_Indigo),
	Pixel_ModRGB(89, Set, RGB_Indigo),

	// Set 13
	Pixel_ModRGB(9, Set, RGB_BlueIndigo),
	Pixel_ModRGB(27, Set, RGB_BlueIndigo),
	Pixel_ModRGB(46, Set, RGB_BlueIndigo),
	Pixel_ModRGB(64, Set, RGB_BlueIndigo),

	// Set 14
	Pixel_ModRGB(10, Set, RGB_Blue),
	Pixel_ModRGB(28, Set, RGB_Blue),
	Pixel_ModRGB(47, Set, RGB_Blue),
	Pixel_ModRGB(90, Set, RGB_Blue),

	// Set 15
	Pixel_ModRGB(11, Set, RGB_GreenBlue),
	Pixel_ModRGB(30, Set, RGB_GreenBlue),
	Pixel_ModRGB(66, Set, RGB_GreenBlue),
	Pixel_ModRGB(81, Set, RGB_GreenBlue),

	// Set 16
	Pixel_ModRGB(12, Set, RGB_Green),
	Pixel_ModRGB(48, Set, RGB_Green),
	Pixel_ModRGB(91, Set, RGB_Green),

	// Set 17
	Pixel_ModRGB(13, Set, RGB_YellowGreen),
	Pixel_ModRGB(32, Set, RGB_YellowGreen),
	Pixel_ModRGB(50, Set, RGB_YellowGreen),
	Pixel_ModRGB(92, Set, RGB_YellowGreen),

	// Set 18
	Pixel_ModRGB(14, Set, RGB_Yellow),
	Pixel_ModRGB(33, Set, RGB_Yellow),
	Pixel_ModRGB(51, Set, RGB_Yellow),
	Pixel_ModRGB(83, Set, RGB_Yellow),
	Pixel_ModRGB(93, Set, RGB_Yellow),

	// Set 19
	Pixel_ModRGB(15, Set, RGB_OrangeYellow),
	Pixel_ModRGB(34, Set, RGB_OrangeYellow),
	Pixel_ModRGB(52, Set, RGB_OrangeYellow),
	Pixel_ModRGB(94, Set, RGB_OrangeYellow),
};

const uint8_t rainbow_frame23[] = {
	// Set 9
	Pixel_ModRGB(5, Set, RGB_Black),
	Pixel_ModRGB(23, Set, RGB_Black),
	Pixel_ModRGB(42, Set, RGB_Black),
	Pixel_ModRGB(60, Set, RGB_Black),
	Pixel_ModRGB(76, Set, RGB_Black),

	// Set 10
	Pixel_ModRGB(6, Set, RGB_HalfViolet),
	Pixel_ModRGB(24, Set, RGB_HalfViolet),
	Pixel_ModRGB(43, Set, RGB_HalfViolet),
	Pixel_ModRGB(61, Set, RGB_HalfViolet),
	Pixel_ModRGB(77, Set, RGB_HalfViolet),

	// Set 11
	Pixel_ModRGB(7, Set, RGB_Violet),
	Pixel_ModRGB(25, Set, RGB_Violet),
	Pixel_ModRGB(44, Set, RGB_Violet),
	Pixel_ModRGB(62, Set, RGB_Violet),
	Pixel_ModRGB(78, Set, RGB_Violet),
	Pixel_ModRGB(88, Set, RGB_Violet),

	// Set 12
	Pixel_ModRGB(8, Set, RGB_IndigoViolet),
	Pixel_ModRGB(26, Set, RGB_IndigoViolet),
	Pixel_ModRGB(45, Set, RGB_IndigoViolet),
	Pixel_ModRGB(63, Set, RGB_IndigoViolet),
	Pixel_ModRGB(79, Set, RGB_IndigoViolet),
	Pixel_ModRGB(89, Set, RGB_IndigoViolet),

	// Set 13
	Pixel_ModRGB(9, Set, RGB_Indigo),
	Pixel_ModRGB(27, Set, RGB_Indigo),
	Pixel_ModRGB(46, Set, RGB_Indigo),
	Pixel_ModRGB(64, Set, RGB_Indigo),

	// Set 14
	Pixel_ModRGB(10, Set, RGB_BlueIndigo),
	Pixel_ModRGB(28, Set, RGB_BlueIndigo),
	Pixel_ModRGB(47, Set, RGB_BlueIndigo),
	Pixel_ModRGB(90, Set, RGB_BlueIndigo),

	// Set 15
	Pixel_ModRGB(11, Set, RGB_Blue),
	Pixel_ModRGB(30, Set, RGB_Blue),
	Pixel_ModRGB(66, Set, RGB_Blue),
	Pixel_ModRGB(81, Set, RGB_Blue),

	// Set 16
	Pixel_ModRGB(12, Set, RGB_GreenBlue),
	Pixel_ModRGB(48, Set, RGB_GreenBlue),
	Pixel_ModRGB(91, Set, RGB_GreenBlue),

	// Set 17
	Pixel_ModRGB(13, Set, RGB_Green),
	Pixel_ModRGB(32, Set, RGB_Green),
	Pixel_ModRGB(50, Set, RGB_Green),
	Pixel_ModRGB(92, Set, RGB_Green),

	// Set 18
	Pixel_ModRGB(14, Set, RGB_YellowGreen),
	Pixel_ModRGB(33, Set, RGB_YellowGreen),
	Pixel_ModRGB(51, Set, RGB_YellowGreen),
	Pixel_ModRGB(83, Set, RGB_YellowGreen),
	Pixel_ModRGB(93, Set, RGB_YellowGreen),

	// Set 19
	Pixel_ModRGB(15, Set, RGB_Yellow),
	Pixel_ModRGB(34, Set, RGB_Yellow),
	Pixel_ModRGB(52, Set, RGB_Yellow),
	Pixel_ModRGB(94, Set, RGB_Yellow),
};

const uint8_t rainbow_frame24[] = {
	// Set 10
	Pixel_ModRGB(6, Set, RGB_Black),
	Pixel_ModRGB(24, Set, RGB_Black),
	Pixel_ModRGB(43, Set, RGB_Black),
	Pixel_ModRGB(61, Set, RGB_Black),
	Pixel_ModRGB(77, Set, RGB_Black),

	// Set 11
	Pixel_ModRGB(7, Set, RGB_HalfViolet),
	Pixel_ModRGB(25, Set, RGB_HalfViolet),
	Pixel_ModRGB(44, Set, RGB_HalfViolet),
	Pixel_ModRGB(62, Set, RGB_HalfViolet),
	Pixel_ModRGB(78, Set, RGB_HalfViolet),
	Pixel_ModRGB(88, Set, RGB_HalfViolet),

	// Set 12
	Pixel_ModRGB(8, Set, RGB_Violet),
	Pixel_ModRGB(26, Set, RGB_Violet),
	Pixel_ModRGB(45, Set, RGB_Violet),
	Pixel_ModRGB(63, Set, RGB_Violet),
	Pixel_ModRGB(79, Set, RGB_Violet),
	Pixel_ModRGB(89, Set, RGB_Violet),

	// Set 13
	Pixel_ModRGB(9, Set, RGB_IndigoViolet),
	Pixel_ModRGB(27, Set, RGB_IndigoViolet),
	Pixel_ModRGB(46, Set, RGB_IndigoViolet),
	Pixel_ModRGB(64, Set, RGB_IndigoViolet),

	// Set 14
	Pixel_ModRGB(10, Set, RGB_Indigo),
	Pixel_ModRGB(28, Set, RGB_Indigo),
	Pixel_ModRGB(47, Set, RGB_Indigo),
	Pixel_ModRGB(90, Set, RGB_Indigo),

	// Set 15
	Pixel_ModRGB(11, Set, RGB_BlueIndigo),
	Pixel_ModRGB(30, Set, RGB_BlueIndigo),
	Pixel_ModRGB(66, Set, RGB_BlueIndigo),
	Pixel_ModRGB(81, Set, RGB_BlueIndigo),

	// Set 16
	Pixel_ModRGB(12, Set, RGB_Blue),
	Pixel_ModRGB(48, Set, RGB_Blue),
	Pixel_ModRGB(91, Set, RGB_Blue),

	// Set 17
	Pixel_ModRGB(13, Set, RGB_GreenBlue),
	Pixel_ModRGB(32, Set, RGB_GreenBlue),
	Pixel_ModRGB(50, Set, RGB_GreenBlue),
	Pixel_ModRGB(92, Set, RGB_GreenBlue),

	// Set 18
	Pixel_ModRGB(14, Set, RGB_Green),
	Pixel_ModRGB(33, Set, RGB_Green),
	Pixel_ModRGB(51, Set, RGB_Green),
	Pixel_ModRGB(83, Set, RGB_Green),
	Pixel_ModRGB(93, Set, RGB_Green),

	// Set 19
	Pixel_ModRGB(15, Set, RGB_YellowGreen),
	Pixel_ModRGB(34, Set, RGB_YellowGreen),
	Pixel_ModRGB(52, Set, RGB_YellowGreen),
	Pixel_ModRGB(94, Set, RGB_YellowGreen),
};

const uint8_t rainbow_frame25[] = {
	// Set 11
	Pixel_ModRGB(7, Set, RGB_Black),
	Pixel_ModRGB(25, Set, RGB_Black),
	Pixel_ModRGB(44, Set, RGB_Black),
	Pixel_ModRGB(62, Set, RGB_Black),
	Pixel_ModRGB(78, Set, RGB_Black),
	Pixel_ModRGB(88, Set, RGB_Black),

	// Set 12
	Pixel_ModRGB(8, Set, RGB_HalfViolet),
	Pixel_ModRGB(26, Set, RGB_HalfViolet),
	Pixel_ModRGB(45, Set, RGB_HalfViolet),
	Pixel_ModRGB(63, Set, RGB_HalfViolet),
	Pixel_ModRGB(79, Set, RGB_HalfViolet),
	Pixel_ModRGB(89, Set, RGB_HalfViolet),

	// Set 13
	Pixel_ModRGB(9, Set, RGB_Violet),
	Pixel_ModRGB(27, Set, RGB_Violet),
	Pixel_ModRGB(46, Set, RGB_Violet),
	Pixel_ModRGB(64, Set, RGB_Violet),

	// Set 14
	Pixel_ModRGB(10, Set, RGB_IndigoViolet),
	Pixel_ModRGB(28, Set, RGB_IndigoViolet),
	Pixel_ModRGB(47, Set, RGB_IndigoViolet),
	Pixel_ModRGB(90, Set, RGB_IndigoViolet),

	// Set 15
	Pixel_ModRGB(11, Set, RGB_Indigo),
	Pixel_ModRGB(30, Set, RGB_Indigo),
	Pixel_ModRGB(66, Set, RGB_Indigo),
	Pixel_ModRGB(81, Set, RGB_Indigo),

	// Set 16
	Pixel_ModRGB(12, Set, RGB_BlueIndigo),
	Pixel_ModRGB(48, Set, RGB_BlueIndigo),
	Pixel_ModRGB(91, Set, RGB_BlueIndigo),

	// Set 17
	Pixel_ModRGB(13, Set, RGB_Blue),
	Pixel_ModRGB(32, Set, RGB_Blue),
	Pixel_ModRGB(50, Set, RGB_Blue),
	Pixel_ModRGB(92, Set, RGB_Blue),

	// Set 18
	Pixel_ModRGB(14, Set, RGB_GreenBlue),
	Pixel_ModRGB(33, Set, RGB_GreenBlue),
	Pixel_ModRGB(51, Set, RGB_GreenBlue),
	Pixel_ModRGB(83, Set, RGB_GreenBlue),
	Pixel_ModRGB(93, Set, RGB_GreenBlue),

	// Set 19
	Pixel_ModRGB(15, Set, RGB_Green),
	Pixel_ModRGB(34, Set, RGB_Green),
	Pixel_ModRGB(52, Set, RGB_Green),
	Pixel_ModRGB(94, Set, RGB_Green),
};

const uint8_t rainbow_frame26[] = {
	// Set 12
	Pixel_ModRGB(8, Set, RGB_Black),
	Pixel_ModRGB(26, Set, RGB_Black),
	Pixel_ModRGB(45, Set, RGB_Black),
	Pixel_ModRGB(63, Set, RGB_Black),
	Pixel_ModRGB(79, Set, RGB_Black),
	Pixel_ModRGB(89, Set, RGB_Black),

	// Set 13
	Pixel_ModRGB(9, Set, RGB_HalfViolet),
	Pixel_ModRGB(27, Set, RGB_HalfViolet),
	Pixel_ModRGB(46, Set, RGB_HalfViolet),
	Pixel_ModRGB(64, Set, RGB_HalfViolet),

	// Set 14
	Pixel_ModRGB(10, Set, RGB_Violet),
	Pixel_ModRGB(28, Set, RGB_Violet),
	Pixel_ModRGB(47, Set, RGB_Violet),
	Pixel_ModRGB(90, Set, RGB_Violet),

	// Set 15
	Pixel_ModRGB(11, Set, RGB_IndigoViolet),
	Pixel_ModRGB(30, Set, RGB_IndigoViolet),
	Pixel_ModRGB(66, Set, RGB_IndigoViolet),
	Pixel_ModRGB(81, Set, RGB_IndigoViolet),

	// Set 16
	Pixel_ModRGB(12, Set, RGB_Indigo),
	Pixel_ModRGB(48, Set, RGB_Indigo),
	Pixel_ModRGB(91, Set, RGB_Indigo),

	// Set 17
	Pixel_ModRGB(13, Set, RGB_BlueIndigo),
	Pixel_ModRGB(32, Set, RGB_BlueIndigo),
	Pixel_ModRGB(50, Set, RGB_BlueIndigo),
	Pixel_ModRGB(92, Set, RGB_BlueIndigo),

	// Set 18
	Pixel_ModRGB(14, Set, RGB_Blue),
	Pixel_ModRGB(33, Set, RGB_Blue),
	Pixel_ModRGB(51, Set, RGB_Blue),
	Pixel_ModRGB(83, Set, RGB_Blue),
	Pixel_ModRGB(93, Set, RGB_Blue),

	// Set 19
	Pixel_ModRGB(15, Set, RGB_GreenBlue),
	Pixel_ModRGB(34, Set, RGB_GreenBlue),
	Pixel_ModRGB(52, Set, RGB_GreenBlue),
	Pixel_ModRGB(94, Set, RGB_GreenBlue),
};

const uint8_t rainbow_frame27[] = {
	// Set 13
	Pixel_ModRGB(9, Set, RGB_Black),
	Pixel_ModRGB(27, Set, RGB_Black),
	Pixel_ModRGB(46, Set, RGB_Black),
	Pixel_ModRGB(64, Set, RGB_Black),

	// Set 14
	Pixel_ModRGB(10, Set, RGB_HalfViolet),
	Pixel_ModRGB(28, Set, RGB_HalfViolet),
	Pixel_ModRGB(47, Set, RGB_HalfViolet),
	Pixel_ModRGB(90, Set, RGB_HalfViolet),

	// Set 15
	Pixel_ModRGB(11, Set, RGB_Violet),
	Pixel_ModRGB(30, Set, RGB_Violet),
	Pixel_ModRGB(66, Set, RGB_Violet),
	Pixel_ModRGB(81, Set, RGB_Violet),

	// Set 16
	Pixel_ModRGB(12, Set, RGB_IndigoViolet),
	Pixel_ModRGB(48, Set, RGB_IndigoViolet),
	Pixel_ModRGB(91, Set, RGB_IndigoViolet),

	// Set 17
	Pixel_ModRGB(13, Set, RGB_Indigo),
	Pixel_ModRGB(32, Set, RGB_Indigo),
	Pixel_ModRGB(50, Set, RGB_Indigo),
	Pixel_ModRGB(92, Set, RGB_Indigo),

	// Set 18
	Pixel_ModRGB(14, Set, RGB_BlueIndigo),
	Pixel_ModRGB(33, Set, RGB_BlueIndigo),
	Pixel_ModRGB(51, Set, RGB_BlueIndigo),
	Pixel_ModRGB(83, Set, RGB_BlueIndigo),
	Pixel_ModRGB(93, Set, RGB_BlueIndigo),

	// Set 19
	Pixel_ModRGB(15, Set, RGB_Blue),
	Pixel_ModRGB(34, Set, RGB_Blue),
	Pixel_ModRGB(52, Set, RGB_Blue),
	Pixel_ModRGB(94, Set, RGB_Blue),
};

const uint8_t rainbow_frame28[] = {
	// Set 14
	Pixel_ModRGB(10, Set, RGB_Black),
	Pixel_ModRGB(28, Set, RGB_Black),
	Pixel_ModRGB(47, Set, RGB_Black),
	Pixel_ModRGB(90, Set, RGB_Black),

	// Set 15
	Pixel_ModRGB(11, Set, RGB_HalfViolet),
	Pixel_ModRGB(30, Set, RGB_HalfViolet),
	Pixel_ModRGB(66, Set, RGB_HalfViolet),
	Pixel_ModRGB(81, Set, RGB_HalfViolet),

	// Set 16
	Pixel_ModRGB(12, Set, RGB_Violet),
	Pixel_ModRGB(48, Set, RGB_Violet),
	Pixel_ModRGB(91, Set, RGB_Violet),

	// Set 17
	Pixel_ModRGB(13, Set, RGB_IndigoViolet),
	Pixel_ModRGB(32, Set, RGB_IndigoViolet),
	Pixel_ModRGB(50, Set, RGB_IndigoViolet),
	Pixel_ModRGB(92, Set, RGB_IndigoViolet),

	// Set 18
	Pixel_ModRGB(14, Set, RGB_Indigo),
	Pixel_ModRGB(33, Set, RGB_Indigo),
	Pixel_ModRGB(51, Set, RGB_Indigo),
	Pixel_ModRGB(83, Set, RGB_Indigo),
	Pixel_ModRGB(93, Set, RGB_Indigo),

	// Set 19
	Pixel_ModRGB(15, Set, RGB_BlueIndigo),
	Pixel_ModRGB(34, Set, RGB_BlueIndigo),
	Pixel_ModRGB(52, Set, RGB_BlueIndigo),
	Pixel_ModRGB(94, Set, RGB_BlueIndigo),
};

const uint8_t rainbow_frame29[] = {
	// Set 15
	Pixel_ModRGB(11, Set, RGB_Black),
	Pixel_ModRGB(30, Set, RGB_Black),
	Pixel_ModRGB(66, Set, RGB_Black),
	Pixel_ModRGB(81, Set, RGB_Black),

	// Set 16
	Pixel_ModRGB(12, Set, RGB_HalfViolet),
	Pixel_ModRGB(48, Set, RGB_HalfViolet),
	Pixel_ModRGB(91, Set, RGB_HalfViolet),

	// Set 17
	Pixel_ModRGB(13, Set, RGB_Violet),
	Pixel_ModRGB(32, Set, RGB_Violet),
	Pixel_ModRGB(50, Set, RGB_Violet),
	Pixel_ModRGB(92, Set, RGB_Violet),

	// Set 18
	Pixel_ModRGB(14, Set, RGB_IndigoViolet),
	Pixel_ModRGB(33, Set, RGB_IndigoViolet),
	Pixel_ModRGB(51, Set, RGB_IndigoViolet),
	Pixel_ModRGB(83, Set, RGB_IndigoViolet),
	Pixel_ModRGB(93, Set, RGB_IndigoViolet),

	// Set 19
	Pixel_ModRGB(15, Set, RGB_Indigo),
	Pixel_ModRGB(34, Set, RGB_Indigo),
	Pixel_ModRGB(52, Set, RGB_Indigo),
	Pixel_ModRGB(94, Set, RGB_Indigo),
};

const uint8_t rainbow_frame30[] = {
	// Set 16
	Pixel_ModRGB(12, Set, RGB_Black),
	Pixel_ModRGB(48, Set, RGB_Black),
	Pixel_ModRGB(91, Set, RGB_Black),

	// Set 17
	Pixel_ModRGB(13, Set, RGB_HalfViolet),
	Pixel_ModRGB(32, Set, RGB_HalfViolet),
	Pixel_ModRGB(50, Set, RGB_HalfViolet),
	Pixel_ModRGB(92, Set, RGB_HalfViolet),

	// Set 18
	Pixel_ModRGB(14, Set, RGB_Violet),
	Pixel_ModRGB(33, Set, RGB_Violet),
	Pixel_ModRGB(51, Set, RGB_Violet),
	Pixel_ModRGB(83, Set, RGB_Violet),
	Pixel_ModRGB(93, Set, RGB_Violet),

	// Set 19
	Pixel_ModRGB(15, Set, RGB_IndigoViolet),
	Pixel_ModRGB(34, Set, RGB_IndigoViolet),
	Pixel_ModRGB(52, Set, RGB_IndigoViolet),
	Pixel_ModRGB(94, Set, RGB_IndigoViolet),
};

const uint8_t rainbow_frame31[] = {
	// Set 17
	Pixel_ModRGB(13, Set, RGB_Black),
	Pixel_ModRGB(32, Set, RGB_Black),
	Pixel_ModRGB(50, Set, RGB_Black),
	Pixel_ModRGB(92, Set, RGB_Black),

	// Set 18
	Pixel_ModRGB(14, Set, RGB_HalfViolet),
	Pixel_ModRGB(33, Set, RGB_HalfViolet),
	Pixel_ModRGB(51, Set, RGB_HalfViolet),
	Pixel_ModRGB(83, Set, RGB_HalfViolet),
	Pixel_ModRGB(93, Set, RGB_HalfViolet),

	// Set 19
	Pixel_ModRGB(15, Set, RGB_Violet),
	Pixel_ModRGB(34, Set, RGB_Violet),
	Pixel_ModRGB(52, Set, RGB_Violet),
	Pixel_ModRGB(94, Set, RGB_Violet),
};

const uint8_t rainbow_frame32[] = {
	// Set 18
	Pixel_ModRGB(14, Set, RGB_Black),
	Pixel_ModRGB(33, Set, RGB_Black),
	Pixel_ModRGB(51, Set, RGB_Black),
	Pixel_ModRGB(83, Set, RGB_Black),
	Pixel_ModRGB(93, Set, RGB_Black),

	// Set 19
	Pixel_ModRGB(15, Set, RGB_HalfViolet),
	Pixel_ModRGB(34, Set, RGB_HalfViolet),
	Pixel_ModRGB(52, Set, RGB_HalfViolet),
	Pixel_ModRGB(94, Set, RGB_HalfViolet),
};

const uint8_t rainbow_frame33[] = {
	// Set 19
	Pixel_ModRGB(15, Set, RGB_Black),
	Pixel_ModRGB(34, Set, RGB_Black),
	Pixel_ModRGB(52, Set, RGB_Black),
	Pixel_ModRGB(94, Set, RGB_Black),
};



// Index of frames for animations
//  uint8_t *<animation>_frames[] = { <animation>_frame<num>, ... }
const uint8_t *testani_frames[] = {
	testani_frame0,
	testani_frame1,
	testani_frame2,
};


// Rainbow frame index
const uint8_t *rainbow_frames[] = {
	rainbow_frame0,
	rainbow_frame1,
	rainbow_frame2,
	rainbow_frame3,
	rainbow_frame4,
	rainbow_frame5,
	rainbow_frame6,
	rainbow_frame7,
	rainbow_frame8,
	rainbow_frame9,
	rainbow_frame10,
	rainbow_frame11,
	rainbow_frame12,
	rainbow_frame13,
	rainbow_frame14,
	rainbow_frame15,
	rainbow_frame16,
	rainbow_frame17,
	rainbow_frame18,
	rainbow_frame19,
	rainbow_frame20,
	rainbow_frame21,
	rainbow_frame22,
	rainbow_frame23,
	rainbow_frame24,
	rainbow_frame25,
	rainbow_frame26,
	rainbow_frame27,
	rainbow_frame28,
	rainbow_frame29,
	rainbow_frame30,
	rainbow_frame31,
	rainbow_frame32,
	rainbow_frame33,
};

const uint16_t rainbow_framesizes[] = {
	sizeof( rainbow_frame0 ),
	sizeof( rainbow_frame1 ),
	sizeof( rainbow_frame2 ),
	sizeof( rainbow_frame3 ),
	sizeof( rainbow_frame4 ),
	sizeof( rainbow_frame5 ),
	sizeof( rainbow_frame6 ),
	sizeof( rainbow_frame7 ),
	sizeof( rainbow_frame8 ),
	sizeof( rainbow_frame9 ),
	sizeof( rainbow_frame10 ),
	sizeof( rainbow_frame11 ),
	sizeof( rainbow_frame12 ),
	sizeof( rainbow_frame13 ),
	sizeof( rainbow_frame14 ),
	sizeof( rainbow_frame15 ),
	sizeof( rainbow_frame16 ),
	sizeof( rainbow_frame17 ),
	sizeof( rainbow_frame18 ),
	sizeof( rainbow_frame19 ),
	sizeof( rainbow_frame20 ),
	sizeof( rainbow_frame21 ),
	sizeof( rainbow_frame22 ),
	sizeof( rainbow_frame23 ),
	sizeof( rainbow_frame24 ),
	sizeof( rainbow_frame25 ),
	sizeof( rainbow_frame26 ),
	sizeof( rainbow_frame27 ),
	sizeof( rainbow_frame28 ),
	sizeof( rainbow_frame29 ),
	sizeof( rainbow_frame30 ),
	sizeof( rainbow_frame31 ),
	sizeof( rainbow_frame32 ),
	sizeof( rainbow_frame33 ),
};

// XXX Temp
uint16_t rainbow_pos = 0;


// Index of animations
//  uint8_t *Pixel_Animations[] = { <animation>_frames, ... }
const uint8_t **Pixel_Animations[] = {
	testani_frames,
	rainbow_frames,
};

// -------------------------------
// TODO GENERATED END
// -------------------------------



// ----- Capabilities -----



// ----- Functions -----

PixelBuf *Pixel_bufferMap( uint16_t channel )
{
	// TODO Generate
	if      ( channel < 144 ) return &Pixel_Buffers[0];
	else if ( channel < 288 ) return &Pixel_Buffers[1];
	else if ( channel < 432 ) return &Pixel_Buffers[2];

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
		PixelElement *elem = &Pixel_Mapping[mod->pixel];
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
	case PixelTest_Pixel_Test:
		// Start from the top of the Animation Stack
		// XXX Temp - Play rainbow
		Pixel_pixelProcess( rainbow_frames[rainbow_pos], rainbow_framesizes[rainbow_pos] );
		rainbow_pos++;

		if ( rainbow_pos >= sizeof( rainbow_framesizes ) / 2 )
		{
			rainbow_pos = 0;
			goto pixel_process_done;
		}

	default:
		break;
	}

	/*
	// Start from the top of the Animation Stack
	// TODO

	// XXX Temp - Play rainbow
	Pixel_pixelProcess( rainbow_frames[rainbow_pos], rainbow_framesizes[rainbow_pos] );
	rainbow_pos++;
	Pixel_testMode = PixelTest_Off;

	if ( rainbow_pos >= sizeof( rainbow_framesizes ) / 2 )
	{
		rainbow_pos = 0;
	}
	*/


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
	//Pixel_testMode = PixelTest_Off;

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

