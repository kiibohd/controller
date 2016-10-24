/* Copyright (C) 2014-2016 by Jacob Alexander
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
#include <Lib/ScanLib.h>

// Project Includes
#include <cli.h>
#include <kll_defs.h>
#include <led.h>
#include <print.h>
#include <pixel.h>

// Interconnect module if compiled in
#if defined(ConnectEnabled_define)
#include <connect_scan.h>
#endif

// Local Includes
#include "i2c.h"
#include "led_scan.h"



// ----- Defines -----

// TODO Make this a kll define
//      I2C transfers are more efficient if all 144 don't have to be updated
//      (and most implementations don't use all the channels)
//      Ergodox  tested @ 83fps /w 38
//      Whitefox tested @ 45fps /w 71
#define LED_BufferLength       144

#define LED_EnableBufferLength 18
#define LED_FrameBuffersMax     4
#define LED_TotalChannels     (LED_BufferLength * ISSI_Chips_define)

// ISSI Addresses
// IS31FL3731 (max 4 channels per bus)
#if ISSI_Chip_31FL3731_define == 1
#define ISSI_Ch1 0xE8
#define ISSI_Ch2 0xEA
#define ISSI_Ch3 0xEC
#define ISSI_Ch4 0xEE

// IS31FL3732 (max 16 channels per bus)
#elif ISSI_Chip_31FL3732_define == 1
#define ISSI_Ch1  0xA0
#define ISSI_Ch2  0xA2
#define ISSI_Ch3  0xA4
#define ISSI_Ch4  0xA6
#define ISSI_Ch5  0xA8
#define ISSI_Ch6  0xAA
#define ISSI_Ch7  0xAC
#define ISSI_Ch8  0xAE
#define ISSI_Ch9  0xB0
#define ISSI_Ch10 0xB2
#define ISSI_Ch11 0xB4
#define ISSI_Ch12 0xB6
#define ISSI_Ch13 0xB8
#define ISSI_Ch14 0xBA
#define ISSI_Ch15 0xBC
#define ISSI_Ch16 0xBE

#else
#error "ISSI Driver Chip not defined in Scan defaultMap.kll..."
#endif

// TODO Generate in KLL
#define LED_MapCh1_Bus_define  0x0
#define LED_MapCh1_Addr_define ISSI_Ch1
#define LED_MapCh2_Bus_define  0x0
#define LED_MapCh2_Addr_define ISSI_Ch2
#define LED_MapCh3_Bus_define  0x1
#define LED_MapCh3_Addr_define ISSI_Ch1
#define LED_MapCh4_Bus_define  0x1
#define LED_MapCh4_Addr_define ISSI_Ch2



// ----- Macros -----

#define LED_ChannelMapDefine(ch) \
	{ \
		LED_MapCh##ch##_Bus_define, /* I2C bus number */ \
		LED_MapCh##ch##_Addr_define, /* I2C address */ \
	}

#define LED_MaskDefine(ch) \
	{ \
		LED_MapCh##ch##_Addr_define, /* I2C address */ \
		0x00, /* Starting register address */ \
		{ ISSILedMask##ch##_define }, \
	}

#define LED_BrightnessDefine(ch) \
	{ \
		LED_MapCh##ch##_Addr_define, /* I2C address */ \
		0x24, /* Starting register address */ \
		{ ISSILedBrightness##ch##_define }, \
	}



// ----- Structs -----

typedef struct LED_Buffer {
	uint16_t i2c_addr;
	uint16_t reg_addr;
	uint16_t buffer[LED_BufferLength];
} LED_Buffer;

typedef struct LED_EnableBuffer {
	uint16_t i2c_addr;
	uint16_t reg_addr;
	uint16_t buffer[LED_EnableBufferLength];
} LED_EnableBuffer;

typedef struct LED_ChannelMap {
	uint8_t bus;
	uint8_t addr;
} LED_ChannelMap;



// ----- Function Declarations -----

// CLI Functions
void cliFunc_i2cSend  ( char* args );
void cliFunc_ledCtrl  ( char* args );
void cliFunc_ledReset ( char* args );
void cliFunc_ledSpeed ( char* args );



// ----- Variables -----

// Scan Module command dictionary
CLIDict_Entry( i2cSend,     "Send I2C sequence of bytes. Use |'s to split sequences with a stop." );
CLIDict_Entry( ledCtrl,     "Basic LED control. Args: <mode> <amount> [<index>]" );
CLIDict_Entry( ledReset,    "Reset ISSI chips." );
CLIDict_Entry( ledSpeed,    "ISSI frame rate 0-63, 1 is fastest. f - display fps" );

CLIDict_Def( ledCLIDict, "ISSI LED Module Commands" ) = {
	CLIDict_Item( i2cSend ),
	CLIDict_Item( ledCtrl ),
	CLIDict_Item( ledReset ),
	CLIDict_Item( ledSpeed ),
	{ 0, 0, 0 } // Null entry for dictionary end
};


volatile LED_Buffer LED_pageBuffer[ISSI_Chips_define];

         uint8_t LED_FrameBuffersReady; // Starts at maximum, reset on interrupt from ISSI
volatile uint8_t LED_FrameBufferReset;  // INTB interrupt received, reset available buffer count when ready
         uint8_t LED_FrameBufferPage;   // Current page of the buffer
         uint8_t LED_FrameBufferStart;  // Whether or not a start signal can be sent

         uint8_t LED_displayFPS;        // Display fps to cli

// TODO - Autogenerate for each keyboard
// ISSI Driver Channel to Bus:Address mapping
const LED_ChannelMap LED_ChannelMapping[ISSI_Chips_define] = {
	LED_ChannelMapDefine( 1 ),
#if ISSI_Chips_define >= 2
	LED_ChannelMapDefine( 2 ),
#endif
#if ISSI_Chips_define >= 3
	LED_ChannelMapDefine( 3 ),
#endif
#if ISSI_Chips_define >= 4
	LED_ChannelMapDefine( 4 ),
#endif
};

// Enable mask and default brightness for ISSI chip channel
const LED_EnableBuffer LED_ledEnableMask[ISSI_Chips_define] = {
	LED_MaskDefine( 1 ),
#if ISSI_Chips_define >= 2
	LED_MaskDefine( 2 ),
#endif
#if ISSI_Chips_define >= 3
	LED_MaskDefine( 3 ),
#endif
#if ISSI_Chips_define >= 4
	LED_MaskDefine( 4 ),
#endif
};

// Default LED brightness
const LED_Buffer LED_defaultBrightness[ISSI_Chips_define] = {
	LED_BrightnessDefine( 1 ),
#if ISSI_Chips_define >= 2
	LED_BrightnessDefine( 2 ),
#endif
#if ISSI_Chips_define >= 3
	LED_BrightnessDefine( 3 ),
#endif
#if ISSI_Chips_define >= 4
	LED_BrightnessDefine( 4 ),
#endif
};


#if ISSI_Chips_define >= 5
#error "Invalid number of ISSI Chips"
#endif



// ----- Interrupt Functions -----

void portb_isr()
{
	// Check for ISSI INTB IRQ
	if ( PORTB_ISFR & (1 << 17) )
	{
		// Set frame buffer replenish condition
		LED_FrameBufferReset = 1;

		// Clear IRQ
		PORTB_ISFR |= (1 << 17);
	}
}



// ----- Functions -----

void LED_zeroPages( uint8_t bus, uint8_t addr, uint8_t startPage, uint8_t numPages, uint8_t startReg, uint8_t endReg )
{
	// Clear Page
	// Max length of a page + chip id + reg start
	uint16_t clearPage[2 + 0xB4] = { 0 };
	clearPage[0] = addr;
	clearPage[1] = startReg;

	// Iterate through given pages, zero'ing out the given register regions
	for ( uint8_t page = startPage; page < startPage + numPages; page++ )
	{
		// Page Setup
		uint16_t pageSetup[] = { addr, 0xFD, page };

		// Setup page
		while ( i2c_send( bus, pageSetup, sizeof( pageSetup ) / 2 ) == -1 )
			delay(1);

		// Zero out page
		while ( i2c_send( bus, clearPage, 2 + endReg - startReg ) == -1 )
			delay(1);
	}

	// Wait until finished zero'ing
	while ( i2c_busy( bus ) )
		delay(1);
}

void LED_sendPage( uint8_t bus, uint8_t addr, uint16_t *buffer, uint32_t len, uint8_t page )
{
	/*
	info_msg("I2C Send Page Addr: ");
	printHex( addr );
	print(" Len: ");
	printHex( len );
	print(" Page: ");
	printHex( page );
	print( NL );
	*/

	// Page Setup
	uint16_t pageSetup[] = { addr, 0xFD, page };

	// Setup page
	while ( i2c_send( bus, pageSetup, sizeof( pageSetup ) / 2 ) == -1 )
		delay(1);

	// Write page to I2C Tx Buffer
	while ( i2c_send( bus, buffer, len ) == -1 )
		delay(1);
}

// Write register on all ISSI chips
// Prepare pages first, then attempt write register with a minimal delay between chips
void LED_syncReg( uint8_t reg, uint8_t val, uint8_t page )
{
	uint16_t pageSetup[] = { 0, 0xFD, page };

	// Setup each of the pages
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		pageSetup[0] = LED_ChannelMapping[ ch ].addr;
		uint8_t bus = LED_ChannelMapping[ ch ].bus;

		while ( i2c_send( bus, pageSetup, sizeof( pageSetup ) / 2 ) == -1 )
			delay(1);
	}

	// Reg Write Setup
	uint16_t writeData[] = { 0, reg, val };

	// Write to all the registers
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		writeData[0] = LED_ChannelMapping[ ch ].addr;
		uint8_t bus = LED_ChannelMapping[ ch ].bus;

		// Delay very little to help with synchronization
		while ( i2c_send( bus, writeData, sizeof( writeData ) / 2 ) == -1 )
			delayMicroseconds(10);
	}

	// Delay until written
	while ( i2c_any_busy() )
		delay(1);
}

// Write address
void LED_writeReg( uint8_t bus, uint8_t addr, uint8_t reg, uint8_t val, uint8_t page )
{
	// Page Setup
	uint16_t pageSetup[] = { addr, 0xFD, page };

	// Reg Write Setup
	uint16_t writeData[] = { addr, reg, val };

	// Setup page
	while ( i2c_send( bus, pageSetup, sizeof( pageSetup ) / 2 ) == -1 )
		delay(1);

	// Write register
	while ( i2c_send( bus, writeData, sizeof( writeData ) / 2 ) == -1 )
		delay(1);

	// Delay until written
	while ( i2c_busy( bus ) )
		delay(1);
}

// Read address
// TODO Not working?
uint8_t LED_readReg( uint8_t bus, uint8_t addr, uint8_t reg, uint8_t page )
{
	// Software shutdown must be enabled to read registers
	LED_writeReg( bus, addr, 0x0A, 0x00, 0x0B );

	// Page Setup
	uint16_t pageSetup[] = { addr, 0xFD, page };

	// Setup page
	while ( i2c_send( bus, pageSetup, sizeof( pageSetup ) / 2 ) == -1 )
		delay(1);

	// Register Setup
	uint16_t regSetup[] = { addr, reg };

	// Configure register
	while ( i2c_send( bus, regSetup, sizeof( regSetup ) / 2 ) == -1 )
		delay(1);

	// Register Read Command
	uint16_t regReadCmd[] = { addr | 0x1, I2C_READ };
	uint8_t recv_data;

	// Request single register byte
	while ( i2c_read( bus, regReadCmd, sizeof( regReadCmd ) / 2, &recv_data ) == -1 )
		delay(1);

	// Disable software shutdown
	LED_writeReg( bus, addr, 0x0A, 0x01, 0x0B );

	return recv_data;
}

void LED_reset()
{
	// Reset frame buffer used count
	LED_FrameBuffersReady = LED_FrameBuffersMax;

	// Starting page for the buffers
	LED_FrameBufferPage = 4;

	// Initially do not allow autoplay to restart
	LED_FrameBufferStart = 0;

	// Disable FPS by default
	LED_displayFPS = 0;

	// Clear LED Pages
	// Enable LEDs based upon mask
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		uint8_t addr = LED_ChannelMapping[ ch ].addr;
		uint8_t bus = LED_ChannelMapping[ ch ].bus;
		LED_zeroPages( bus, addr, 0x00, 8, 0x00, 0xB4 ); // LED Registers

		// For each page
		for ( uint8_t pg = 0; pg < LED_FrameBuffersMax * 2; pg++ )
		{
			LED_sendPage(
				bus,
				addr,
				(uint16_t*)&LED_ledEnableMask[ ch ],
				sizeof( LED_EnableBuffer ) / 2,
				pg
			);
		}
	}

	// Do not disable software shutdown of ISSI chip unless current is high enough
	// Require at least 150 mA
	// May be enabled/disabled at a later time
	// TODO
	if ( Output_current_available() >= 150 )
	{
		// Disable Software shutdown of ISSI chip
		// TODO
		//LED_writeReg( 0x0A, 0x01, 0x0B );
	}

	// Set global brightness control
#if ISSI_Chip_31FL3732_define == 1
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		uint8_t addr = LED_ChannelMapping[ ch ].addr;
		uint8_t bus = LED_ChannelMapping[ ch ].bus;

		// See, 31FL3732 datasheet for details on calculation
		// Depends on Rext
		// TODO Array set per chip
		if ( ch == 3 )
		{
			LED_writeReg( bus, addr, 0x04, 50, 0x0B );
		}
		else
		{
			LED_writeReg( bus, addr, 0x04, ISSI_Global_Brightness_define, 0x0B );
		}
	}
#endif

	// Setup ISSI auto frame play, but do not start yet
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		uint8_t addr = LED_ChannelMapping[ ch ].addr;
		uint8_t bus = LED_ChannelMapping[ ch ].bus;
		// CNS 1 loop, FNS 4 frames - 0x14
		LED_writeReg( bus, addr, 0x02, 0x14, 0x0B );

		// Default refresh speed - TxA
		// T is typically 11ms
		// A is 1 to 64 (where 0 is 64)
		LED_writeReg( bus, addr, 0x03, ISSI_AnimationSpeed_define, 0x0B );

		// Set MODE to Auto Frame Play
		LED_writeReg( bus, addr, 0x00, 0x08, 0x0B );
	}

	// Disable Software shutdown of ISSI chip
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		// Disable Software shutdown of ISSI chip
		for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
		{
			uint8_t addr = LED_ChannelMapping[ ch ].addr;
			uint8_t bus = LED_ChannelMapping[ ch ].bus;
			LED_writeReg( bus, addr, 0x0A, 0x01, 0x0B );
		}
	}
}

// Setup
inline void LED_setup()
{
	// Register Scan CLI dictionary
	CLI_registerDictionary( ledCLIDict, ledCLIDictName );

	// Initialize I2C
	i2c_setup();

	// Setup LED_pageBuffer addresses and brightness section
	LED_pageBuffer[0].i2c_addr = LED_MapCh1_Addr_define;
	LED_pageBuffer[0].reg_addr = 0x24;
#if ISSI_Chips_define >= 2
	LED_pageBuffer[1].i2c_addr = LED_MapCh2_Addr_define;
	LED_pageBuffer[1].reg_addr = 0x24;
#endif
#if ISSI_Chips_define >= 3
	LED_pageBuffer[2].i2c_addr = LED_MapCh3_Addr_define;
	LED_pageBuffer[2].reg_addr = 0x24;
#endif
#if ISSI_Chips_define >= 4
	LED_pageBuffer[3].i2c_addr = LED_MapCh4_Addr_define;
	LED_pageBuffer[3].reg_addr = 0x24;
#endif

	// Enable Hardware shutdown (pull low)
	GPIOB_PDDR |= (1<<16);
	PORTB_PCR16 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOB_PCOR |= (1<<16);

	// Zero out Frame Registers
	// This needs to be done before disabling the hardware shutdown (or the leds will do undefined things)
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		uint8_t addr = LED_ChannelMapping[ ch ].addr;
		uint8_t bus = LED_ChannelMapping[ ch ].bus;
		LED_zeroPages( bus, addr, 0x0B, 1, 0x00, 0x0C ); // Control Registers
	}

	// Disable Hardware shutdown of ISSI chip (pull high)
	GPIOB_PSOR |= (1<<16);

	// Prepare pin to read INTB interrupt of ISSI chip (Active Low)
	// Enable interrupt to detect falling edge
	// Uses external pullup resistor
	GPIOB_PDDR |= ~(1<<17);
	PORTB_PCR17 = PORT_PCR_IRQC(0xA) | PORT_PCR_PFE | PORT_PCR_MUX(1);
	LED_FrameBufferReset = 0; // Clear frame buffer reset condition for ISSI

	// Enable PORTB interrupt
	NVIC_ENABLE_IRQ( IRQ_PORTB );

	// Reset LED sequencing
	LED_reset();
}


// LED Linked Send
// Call-back for i2c write when updating led display
// TODO Optimize linked send for multiple i2c buses
uint8_t LED_chipSend;
void LED_linkedSend()
{
	// Check if we've updated all the ISSI chips for this frame
	if ( LED_chipSend >= ISSI_Chips_define )
	{
		// Increment the buffer page
		// And reset if necessary
		if ( ++LED_FrameBufferPage >= LED_FrameBuffersMax * 2 )
		{
			LED_FrameBufferPage = 0;
		}

		// Now ready to update the frame buffer
		Pixel_FrameState = FrameState_Update;
		return;
	}

	// Update ISSI Frame State
	Pixel_FrameState = FrameState_Sending;

	// Lookup bus number
	uint8_t bus = LED_ChannelMapping[ LED_chipSend ].bus;

	// Debug
	dbug_msg("Linked Send: chip(");
	printHex( LED_chipSend );
	print(") frame(");
	printHex( LED_FrameBufferPage );
	print(") addr(");
	printHex( LED_pageBuffer[ LED_chipSend ].i2c_addr );
	print(") reg(");
	printHex( LED_pageBuffer[ LED_chipSend ].reg_addr );
	print(") len(");
	printHex( sizeof( LED_Buffer ) / 2 );
	print(") data[]" "(");
	for ( uint8_t c = 0; c < 9; c++ )
	//for ( uint8_t c = 0; c < sizeof( LED_Buffer ) / 2 - 2; c++ )
	{
		printHex( LED_pageBuffer[ LED_chipSend ].buffer[c] );
		print(" ");
	}
	print(")" NL);

	// Send, and recursively call this function when finished
	while ( i2c_send_sequence(
		bus,
		(uint16_t*)&LED_pageBuffer[ LED_chipSend ],
		sizeof( LED_Buffer ) / 2,
		0,
		LED_linkedSend,
		0
	) == -1 )
		delay(1);

	// Increment chip position
	LED_chipSend++;
}


// LED State processing loop
uint32_t LED_timePrev = 0;
unsigned int LED_currentEvent = 0;
inline void LED_scan()
{
	// Check for current change event
	if ( LED_currentEvent )
	{
		// TODO dim LEDs in low power mode instead of shutting off
		if ( LED_currentEvent < 150 )
		{
			// Enable Software shutdown of ISSI chip
			for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
			{
				uint8_t addr = LED_ChannelMapping[ ch ].addr;
				uint8_t bus = LED_ChannelMapping[ ch ].bus;
				LED_writeReg( bus, addr, 0x0A, 0x00, 0x0B );
			}
		}
		else
		{
			// Disable Software shutdown of ISSI chip
			for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
			{
				uint8_t addr = LED_ChannelMapping[ ch ].addr;
				uint8_t bus = LED_ChannelMapping[ ch ].bus;
				LED_writeReg( bus, addr, 0x0A, 0x01, 0x0B );
			}
		}

		LED_currentEvent = 0;
	}

	// Check to see if frame buffers are ready to replenish
	if ( LED_FrameBufferReset )
	{
		LED_FrameBufferReset = 0;
		// Delay, in order to synchronize chips
		LED_FrameBufferStart = 1;

		// FPS Display
		if ( LED_displayFPS )
		{
			dbug_msg("4frames/");
			printInt32( systick_millis_count - LED_timePrev );
			LED_timePrev = systick_millis_count;
			print( "ms" NL );
		}
	}

	// Make sure there are buffers available
	if ( LED_FrameBuffersReady == 0 )
	{
		// Only start if we haven't already
		// And if we've finished updating the buffers
		if ( !LED_FrameBufferStart || Pixel_FrameState == FrameState_Sending )
			return;

		// Start Auto Frame Play on either frame 1 or 5
		uint8_t frame = LED_FrameBufferPage == 0 ? 4 : 0;
		LED_syncReg( 0x00, 0x08 | frame, 0x0B );

		LED_FrameBufferStart = 0;
		LED_FrameBuffersReady = LED_FrameBuffersMax;

		return;
	}
	/*
	else
	{
		dbug_msg(":/ - Start(");
		printHex( LED_FrameBufferStart );
		print(") BuffersReady(");
		printHex( LED_FrameBuffersReady );
		print(") State(");
		printHex( Pixel_FrameState );
		print(")"NL);
	}
	*/

	// Only send frame to ISSI chip if buffers are ready
	if ( Pixel_FrameState != FrameState_Ready )
		return;

	LED_FrameBuffersReady--;

	// Set the page of all the ISSI chips
	// This way we can easily link the buffers to send the brightnesses in the background
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		// Page Setup
		uint8_t addr = LED_ChannelMapping[ ch ].addr;
		uint8_t bus = LED_ChannelMapping[ ch ].bus;
		uint16_t pageSetup[] = { addr, 0xFD, LED_FrameBufferPage };

		// Send each update
		while ( i2c_send( bus, pageSetup, sizeof( pageSetup ) / 2 ) == -1 )
			delay(1);
	}

	// Check for current change event
	if ( LED_currentEvent )
	{
		// TODO dim LEDs in low power mode instead of shutting off
		if ( LED_currentEvent < 150 )
		{
			// Enable Software shutdown of ISSI chip
			// TODO
			//LED_writeReg( 0x0A, 0x00, 0x0B );
		}
		else
		{
			// Disable Software shutdown of ISSI chip
			// TODO
			//LED_writeReg( 0x0A, 0x01, 0x0B );
		}

		LED_currentEvent = 0;
	}

	// Send current set of buffers
	// Uses interrupts to send to all the ISSI chips
	// Pixel_FrameState will be updated when complete
	LED_chipSend = 0; // Start with chip 0
	LED_linkedSend();
}


// Called by parent Scan Module whenver the available current has changed
// current - mA
void LED_currentChange( unsigned int current )
{
	// Delay action till next LED scan loop (as this callback sometimes occurs during interrupt requests)
	LED_currentEvent = current;
}



// ----- Capabilities -----

// Basic LED Control Capability
typedef enum LedControlMode {
	// Single LED Modes
	LedControlMode_brightness_decrease,
	LedControlMode_brightness_increase,
	LedControlMode_brightness_set,
	// Set all LEDs (index argument not required)
	LedControlMode_brightness_decrease_all,
	LedControlMode_brightness_increase_all,
	LedControlMode_brightness_set_all,
} LedControlMode;

typedef struct LedControl {
	LedControlMode mode;   // XXX Make sure to adjust the .kll capability if this variable is larger than 8 bits
	uint8_t        amount;
	uint16_t       index;
} LedControl;

void LED_control( LedControl *control )
{
	// Configure based upon the given mode
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		// TODO Perhaps do gamma adjustment?
		switch ( control->mode )
		{
		case LedControlMode_brightness_decrease:
			// Don't worry about rolling over, the cycle is quick
			LED_pageBuffer[ ch ].buffer[ control->index ] -= control->amount;
			break;

		case LedControlMode_brightness_increase:
			// Don't worry about rolling over, the cycle is quick
			LED_pageBuffer[ ch ].buffer[ control->index ] += control->amount;
			break;

		case LedControlMode_brightness_set:
			LED_pageBuffer[ ch ].buffer[ control->index ] = control->amount;
			break;

		case LedControlMode_brightness_decrease_all:
			for ( uint8_t channel = 0; channel < LED_TotalChannels; channel++ )
			{
				// Don't worry about rolling over, the cycle is quick
				LED_pageBuffer[ ch ].buffer[ channel ] -= control->amount;
			}
			break;

		case LedControlMode_brightness_increase_all:
			for ( uint8_t channel = 0; channel < LED_TotalChannels; channel++ )
			{
				// Don't worry about rolling over, the cycle is quick
				LED_pageBuffer[ ch ].buffer[ channel ] += control->amount;
			}
			break;

		case LedControlMode_brightness_set_all:
			for ( uint8_t channel = 0; channel < LED_TotalChannels; channel++ )
			{
				LED_pageBuffer[ ch ].buffer[ channel ] = control->amount;
			}
			break;
		}
	}

	// Sync LED buffer with ISSI chip buffer
	// TODO Support multiple frames
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		LED_sendPage(
			LED_ChannelMapping[ ch ].bus,
			LED_ChannelMapping[ ch ].addr,
			(uint16_t*)&LED_pageBuffer[ ch ],
			sizeof( LED_Buffer ) / 2, 0
		);
	}
}

uint8_t LED_control_timer = 0;
void LED_control_capability( uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("LED_control_capability(mode,amount,index)");
		return;
	}

	// Only use capability on press
	// TODO Analog
	if ( stateType == 0x00 && state == 0x03 ) // Not on release
		return;

	// XXX
	// ISSI Chip locks up if we spam updates too quickly (might be an I2C bug on this side too -HaaTa)
	// Make sure we only send an update every 30 milliseconds at most
	// It may be possible to optimize speed even further, but will likely require serious time with a logic analyzer

	uint8_t currentTime = (uint8_t)systick_millis_count;
	int8_t compare = (int8_t)(currentTime - LED_control_timer) & 0x7F;
	if ( compare < 30 )
	{
		return;
	}
	LED_control_timer = currentTime;

	// Set the input structure
	LedControl *control = (LedControl*)args;

	// Interconnect broadcasting
#if defined(ConnectEnabled_define)
	uint8_t send_packet = 0;
	uint8_t ignore_node = 0;

	// By default send to the *next* node, which will determine where to go next
	extern uint8_t Connect_id; // connect_scan.c
	uint8_t addr = Connect_id + 1;

	switch ( control->mode )
	{
	// Calculate the led address to send
	// If greater than the Total hannels
	// Set address - Total channels
	// Otherwise, ignore
	case LedControlMode_brightness_decrease:
	case LedControlMode_brightness_increase:
	case LedControlMode_brightness_set:
		// Ignore if led is on this node
		if ( control->index < LED_TotalChannels )
			break;

		// Calculate new led index
		control->index -= LED_TotalChannels;

		ignore_node = 1;
		send_packet = 1;
		break;

	// Broadcast to all nodes
	// XXX Do not set broadcasting address
	//     Will send command twice
	case LedControlMode_brightness_decrease_all:
	case LedControlMode_brightness_increase_all:
	case LedControlMode_brightness_set_all:
		send_packet = 1;
		break;
	}

	// Only send interconnect remote capability packet if necessary
	if ( send_packet )
	{
		// generatedKeymap.h
		extern const Capability CapabilitiesList[];

		// Broadcast layerStackExact remote capability (0xFF is the broadcast id)
		Connect_send_RemoteCapability(
			addr,
			LED_control_capability_index,
			state,
			stateType,
			CapabilitiesList[ LED_control_capability_index ].argCount,
			args
		);
	}

	// If there is nothing to do on this node, ignore
	if ( ignore_node )
		return;
#endif

	// Modify led state of this node
	LED_control( control );
}



// ----- CLI Command Functions -----

// TODO Currently not working correctly
void cliFunc_i2cSend( char* args )
{
	/*
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Buffer used after interpretting the args, will be sent to I2C functions
	// NOTE: Limited to 8 bytes currently (can be increased if necessary
	#define i2cSend_BuffLenMax 8
	uint16_t buffer[ i2cSend_BuffLenMax ];
	uint8_t  bufferLen = 0;

	// No \r\n by default after the command is entered
	print( NL );
	info_msg("Sending: ");

	// Parse args until a \0 is found
	while ( bufferLen < i2cSend_BuffLenMax )
	{
		curArgs = arg2Ptr; // Use the previous 2nd arg pointer to separate the next arg from the list
		CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

		// Stop processing args if no more are found
		if ( *arg1Ptr == '\0' )
			break;

		// If | is found, end sequence and start new one
		if ( *arg1Ptr == '|' )
		{
			print("| ");
			i2c_send( buffer, bufferLen );
			bufferLen = 0;
			continue;
		}

		// Interpret the argument
		buffer[ bufferLen++ ] = (uint8_t)numToInt( arg1Ptr );

		// Print out the arg
		dPrint( arg1Ptr );
		print(" ");
	}

	print( NL );

	i2c_send( buffer, bufferLen );
	*/
}

/*
void cliFunc_ledWPage( char* args )
{
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// First specify the write address
	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Stop processing args if no more are found
	if ( *arg1Ptr == '\0' )
		return;
	uint8_t addr = numToInt( arg1Ptr );

	// Next process page and starting address
	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Stop processing args if no more are found
	if ( *arg1Ptr == '\0' )
		return;
	uint8_t page[] = { addr, 0xFD, numToInt( arg1Ptr ) };

	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Stop processing args if no more are found
	if ( *arg1Ptr == '\0' )
		return;
	uint8_t data[] = { addr, numToInt( arg1Ptr ), 0 };

	// Set the register page
	while ( I2C_Send( page, sizeof( page ), 0 ) == 0 )
		delay(1);

	// Process all args
	for ( ;; )
	{
		curArgs = arg2Ptr;
		CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

		// Stop processing args if no more are found
		if ( *arg1Ptr == '\0' )
			break;

		data[2] = numToInt( arg1Ptr );

		// Write register location and data to I2C
		while ( I2C_Send( data, sizeof( data ), 0 ) == 0 )
			delay(1);

		// Increment address
		data[1]++;
	}
}
*/

void cliFunc_ledReset( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		LED_zeroPages(
			LED_ChannelMapping[ ch ].bus,
			LED_ChannelMapping[ ch ].addr,
			0x0B, 1, 0x00, 0x0C
		); // Control Registers
	}

	// Clear buffers
	for ( uint8_t buf = 0; buf < ISSI_Chips_define; buf++ )
	{
		memset( (void*)LED_pageBuffer[ buf ].buffer, 0, LED_BufferLength * 2 );
	}

	LED_reset();

	// TODO Remove me
	/*
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		LED_sendPage(
			LED_ChannelMapping[ ch ].bus,
			LED_ChannelMapping[ ch ].addr,
			0x0B, 1, 0x00, 0x0C
		); // Control Registers
	}
	*/
}

void cliFunc_ledSpeed( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;
	uint8_t speed = ISSI_AnimationSpeed_define;

	// Process speed argument if given
	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Check if f argument was given
	switch ( *arg1Ptr )
	{
	case 'f':
	case 'F':
		info_msg("FPS Toggle");
		LED_displayFPS = !LED_displayFPS;
		return;
	}

	// Stop processing args if no more are found
	if ( *arg1Ptr != '\0' )
	{
		speed = numToInt( arg1Ptr );
	}
	// Default to default speed
	else
	{
		info_msg("Setting default speed: ");
		printInt8( speed );
	}


	// Set refresh speed per ISSI chip
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		uint8_t addr = LED_ChannelMapping[ ch ].addr;
		uint8_t bus = LED_ChannelMapping[ ch ].bus;

		// Default refresh speed - TxA
		// T is typically 11ms
		// A is 1 to 64 (where 0 is 64)
		LED_writeReg( bus, addr, 0x03, speed, 0x0B );
	}
}

void cliFunc_ledCtrl( char* args )
{
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;
	LedControl control;

	// First process mode
	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Stop processing args if no more are found
	if ( *arg1Ptr == '\0' )
		return;
	control.mode = numToInt( arg1Ptr );


	// Next process amount
	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Stop processing args if no more are found
	if ( *arg1Ptr == '\0' )
		return;
	control.amount = numToInt( arg1Ptr );


	// Finally process led index, if it exists
	// Default to 0
	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );
	control.index = *arg1Ptr == '\0' ? 0 : numToInt( arg1Ptr );

	// Process request
	LED_control( &control );
}

void cliFunc_ledNFrame( char* args )
{
	// TODO REMOVEME
	LED_FrameBufferStart = 1;
	return;
	/*
		LED_FrameBufferReset = 0;
		LED_FrameBuffersReady = LED_FrameBuffersMax;
		LED_FrameBufferStart = 1;
	*/
	//LED_FrameBuffersReady++;
	//LED_FrameBufferStart = 1;
	//uint8_t addr = LED_pageBuffer[ 0 ].i2c_addr;
	//LED_writeReg( addr, 0x00, 0x08, 0x0B );

	//LED_FrameBuffersReady--;

	// Iterate over available buffers
	// Each pass can only send one buffer (per chip)
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		// XXX It is more efficient to only send positions that are used
		// However, this may actually have more addressing overhead
		// For simplicity, just sending the full 144 positions per ISSI chip
		uint8_t addr = LED_ChannelMapping[ ch ].addr;
		uint8_t bus = LED_ChannelMapping[ ch ].bus;
		LED_sendPage(
			bus,
			addr,
			(uint16_t*)&LED_pageBuffer[ ch ],
			sizeof( LED_Buffer ) / 2,
			LED_FrameBufferPage
		);
	}

	// Increment the buffer page
	// And reset if necessary
	if ( ++LED_FrameBufferPage >= LED_FrameBuffersMax * 2 )
	{
		LED_FrameBufferPage = 0;
	}
}

