/* Copyright (C) 2014-2020 by Jacob Alexander
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

// ----- Includes -----

// Compiler Includes
#include <Lib/ScanLib.h>

// Project Includes
#include <Lib/gpio.h>
#include <Lib/storage.h>
#include <cli.h>
#include <kll_defs.h>
#include <latency.h>
#include <led.h>
#include <print.h>
#include <pixel.h>

// Interconnect module if compiled in
#if defined(ConnectEnabled_define)
#include <connect_scan.h>
#endif

// KLL Include
#include <kll.h>

// Local Includes
#include "i2c.h"
#include "led_scan.h"



// ----- Defines -----

// ISSI Addresses
// IS31FL3731 (max 4 channels per bus)
#if ISSI_Chip_31FL3731_define == 1
#define LED_BufferLength       144
#define LED_EnableBufferLength  18

#define ISSI_ConfigPage        0x0B
#define ISSI_ConfigPageLength  0x0C
#define ISSI_LEDControlPage    0x00
#define ISSI_LEDPwmPage        0x00
#define ISSI_LEDPwmRegStart    0x00
#define ISSI_PageLength        0xB4
#define ISSI_SendDelay          200
#define ISSI_LEDPages            8
#define ISSI_LEDBlink           18
#define ISSI_LEDCtrlLength      18

#define ISSI_Ch1 0xE8
#define ISSI_Ch2 0xEA
#define ISSI_Ch3 0xEC
#define ISSI_Ch4 0xEE

// IS31FL3732 (max 16 channels per bus)
#elif ISSI_Chip_31FL3732_define == 1
#define LED_BufferLength       144
#define LED_EnableBufferLength  18

#define ISSI_ConfigPage        0x0B
#define ISSI_ConfigPageLength  0x0C
#define ISSI_LEDControlPage    0x00
#define ISSI_LEDPwmPage        0x00
#define ISSI_LEDPwmRegStart    0x00
#define ISSI_PageLength        0xB4
#define ISSI_SendDelay          200
#define ISSI_LEDPages            8
#define ISSI_LEDBlink           18
#define ISSI_LEDCtrlLength      18

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

// IS31FL3733 (max 16 channels per bus)
#elif ISSI_Chip_31FL3733_define == 1
#define LED_BufferLength       192
#define LED_EnableBufferLength  24

#define ISSI_ConfigPage        0x03
#define ISSI_ConfigPageLength  0x10
#define ISSI_LEDControlPage    0x00
#define ISSI_LEDPwmPage        0x01
#define ISSI_LEDPwmRegStart    0x00
#define ISSI_PageLength        0xBF
#define ISSI_SendDelay          70
#define ISSI_LEDPages            3
#define ISSI_LEDBlink            0
#define ISSI_LEDCtrlLength       0

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

// IS31FL3736 (max 16 channels per bus)
#elif ISSI_Chip_31FL3736_define == 1
#define LED_BufferLength       192
#define LED_EnableBufferLength  24

#define ISSI_ConfigPage        0x03
#define ISSI_ConfigPageLength  0x10
#define ISSI_LEDControlPage    0x00
#define ISSI_LEDPwmPage        0x01
#define ISSI_LEDPwmRegStart    0x00
#define ISSI_PageLength        0xBF
#define ISSI_SendDelay          70
#define ISSI_LEDPages            3
#define ISSI_LEDBlink            0
#define ISSI_LEDCtrlLength       0

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
#error "ISSI Driver Chip not defined in Scan scancode_map.kll..."
#endif

#define LED_TotalChannels     (LED_BufferLength * ISSI_Chips_define)



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



// ----- Structs -----

typedef struct LED_Buffer {
	uint16_t i2c_addr;
	uint16_t reg_addr;
	uint16_t ledctrl[ISSI_LEDCtrlLength];
	uint16_t unused[ISSI_LEDBlink];
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
void cliFunc_ledCheck ( char* args );
void cliFunc_ledFPS   ( char* args );
void cliFunc_ledReset ( char* args );
void cliFunc_ledSet   ( char* args );
void cliFunc_ledToggle( char* args );

void LED_loadConfig();
void LED_saveConfig();
void LED_printConfig();


// ----- Variables -----

// Scan Module command dictionary
CLIDict_Entry( ledCheck,    "Run LED diagnostics. Not all ISSI chips support this.");
CLIDict_Entry( ledFPS,      "Show/set FPS of LED driver, r - Reset framerate" );
CLIDict_Entry( ledReset,    "Reset ISSI chips." );
CLIDict_Entry( ledSet,      "Set ISSI overall brightness." );
CLIDict_Entry( ledToggle,   "Toggle ISSI hardware shutdown." );

CLIDict_Def( ledCLIDict, "ISSI LED Module Commands" ) = {
	CLIDict_Item( ledCheck ),
	CLIDict_Item( ledFPS ),
	CLIDict_Item( ledReset ),
	CLIDict_Item( ledSet ),
	CLIDict_Item( ledToggle ),
	{ 0, 0, 0 } // Null entry for dictionary end
};

// Storage Module
typedef struct {
	uint8_t brightness;
	uint8_t framerate;
} LedConfig;

static LedConfig settings = {
	.brightness = ISSI_Global_Brightness_define,
	.framerate = ISSI_FrameRate_ms_define,
};

#if Storage_Enable_define == 1
static LedConfig defaults = {
	.brightness = ISSI_Global_Brightness_define,
	.framerate = ISSI_FrameRate_ms_define,
};

static StorageModule LedStorage = {
	.name = "LED Scan",
	.settings = &settings,
	.defaults = &defaults,
	.size = sizeof(LedConfig),
	.onLoad = LED_loadConfig,
	.onSave = LED_saveConfig,
	.display = LED_printConfig
};
#endif

#if ISSI_Chip_31FL3731_define == 1
// Emulated brightness buffer
volatile LED_Buffer LED_pageBuffer_brightness[ISSI_Chips_define];
#endif
extern LED_Buffer LED_pageBuffer[ISSI_Chips_define];

uint8_t LED_displayFPS;     // Display fps to cli
uint8_t LED_enable;         // Enable/disable ISSI chips
uint8_t LED_enable_current; // Enable/disable ISSI chips (based on USB current availability)
uint8_t LED_pause;          // Pause ISSI updates
uint8_t LED_brightness;     // Global brightness for LEDs

uint32_t LED_framerate;     // Configured led framerate, given in ms per frame

Time LED_timePrev; // Last frame processed


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


#if ISSI_Chips_define >= 5
#error "Invalid number of ISSI Chips"
#endif

// GPIO Pins
static const GPIO_Pin hardware_shutdown_pin = ISSI_HardwareShutdownPin_define;
#if ISSI_Chip_31FL3733_define == 1 || ISSI_Chip_31FL3736_define == 1
static const GPIO_Pin iirst_pin = ISSI_IIRSTPin_define;
#endif

// Latency measurement resource
static uint8_t ledLatencyResource;



// ----- Functions -----

// Enables given page
// IS31FL3733 requires unlocking the 0xFD register
void LED_setupPage( uint8_t bus, uint8_t addr, uint8_t page )
{
#if ISSI_Chip_31FL3733_define == 1 || ISSI_Chip_31FL3736_define == 1
	// See http://www.issi.com/WW/pdf/31FL3733.pdf Table 3 Page 12
	// See http://www.issi.com/WW/pdf/31FL3736.pdf Table 3 Page 13
	uint16_t pageEnable[] = { addr, 0xFE, 0xC5 };
	while ( i2c_send( bus, pageEnable, sizeof( pageEnable ) / 2 ) == -1 )
		delay_us( ISSI_SendDelay );
#endif

	// Setup page
	uint16_t pageSetup[] = { addr, 0xFD, page };
	while ( i2c_send( bus, pageSetup, sizeof( pageSetup ) / 2 ) == -1 )
		delay_us( ISSI_SendDelay );

	// Delay until written
	while ( i2c_busy( bus ) )
		delay_us( ISSI_SendDelay );
}

// Zero out given ISSI page
void LED_zeroPages( uint8_t bus, uint8_t addr, uint8_t startPage, uint8_t numPages, uint8_t startReg, uint8_t endReg )
{
	// Clear Page
	// Max length of a page + chip id + reg start
	uint16_t clearPage[2 + ISSI_PageLength] = { 0 };
	clearPage[0] = addr;
	clearPage[1] = startReg;

	// Iterate through given pages, zero'ing out the given register regions
	for ( uint8_t page = startPage; page < startPage + numPages; page++ )
	{
		// Page Setup
		LED_setupPage( bus, addr, page );

		// Zero out page
		while ( i2c_send( bus, clearPage, 2 + endReg - startReg ) == -1 )
			delay_us( ISSI_SendDelay );
	}

	// Wait until finished zero'ing
	while ( i2c_busy( bus ) )
		delay_us( ISSI_SendDelay );
}

// Write ISSI page
void LED_sendPage( uint8_t bus, uint8_t addr, uint16_t *buffer, uint32_t len, uint8_t page )
{
	/*
	info_print("I2C Send Page: bus(");
	printHex( bus );
	print(")addr(");
	printHex( addr );
	print(")len(");
	printHex( len );
	print(")page(");
	printHex( page );
	print(")data[](");
	for ( uint8_t c = 0; c < len; c++ )
	{
		printHex( buffer[c] );
		print(" ");
	}
	print("..)" NL);
	*/

	// Page Setup
	LED_setupPage( bus, addr, page );

	// Write page to I2C Tx Buffer
	while ( i2c_send( bus, buffer, len ) == -1 )
		delay_us( ISSI_SendDelay );
}

// Write register on all ISSI chips
// Prepare pages first, then attempt write register with a minimal delay between chips
void LED_syncReg( uint8_t reg, uint8_t val, uint8_t page )
{
	// Setup each of the pages
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		LED_setupPage(
			LED_ChannelMapping[ ch ].bus,
			LED_ChannelMapping[ ch ].addr,
			page
		);
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
			delay_us(10);
	}

	// Delay until written
	while ( i2c_any_busy() )
		delay_us( ISSI_SendDelay );
}

// Write address
void LED_writeReg( uint8_t bus, uint8_t addr, uint8_t reg, uint8_t val, uint8_t page )
{
	/*
	info_print("I2C Write bus(");
	printHex( bus );
	print(")addr(");
	printHex( addr );
	print(")reg(");
	printHex( reg );
	print(")val(");
	printHex( val );
	print(")page(");
	printHex( page );
	print(")" NL);
	*/

	// Reg Write Setup
	uint16_t writeData[] = { addr, reg, val };

	// Setup page
	LED_setupPage( bus, addr, page );

	// Write register
	while ( i2c_send( bus, writeData, sizeof( writeData ) / 2 ) == -1 )
		delay_us( ISSI_SendDelay );

	// Delay until written
	while ( i2c_busy( bus ) )
		delay_us( ISSI_SendDelay );
}

// Read address
uint8_t LED_readReg( uint8_t bus, uint8_t addr, uint8_t reg, uint8_t page )
{
	/*
	info_print("I2C Read Bus: ");
	printHex( bus );
	print(" Addr: ");
	printHex( addr );
	print(" Reg: ");
	printHex( reg );
	print(" Page: ");
	printHex( page );
	print( NL );
	*/

#if ISSI_Chip_31FL3731_define == 1 || ISSI_Chip_31FL3732_define == 1
	// Software shutdown must be enabled to read registers
	LED_writeReg( bus, addr, 0x0A, 0x00, ISSI_ConfigPage );
#endif

	// Setup page
	LED_setupPage( bus, addr, page );

	// Register Read Command
	uint16_t regReadCmd[] = { addr, reg, I2C_RESTART, addr | 0x1, I2C_READ };
	uint8_t recv_data;

	// Request single register byte
	while ( i2c_read( bus, regReadCmd, sizeof( regReadCmd ) / 2, &recv_data ) == -1 )
		delay_us( ISSI_SendDelay );

#if ISSI_Chip_31FL3731_define == 1 || ISSI_Chip_31FL3732_define == 1
	// Disable software shutdown
	LED_writeReg( bus, addr, 0x0A, 0x01, ISSI_ConfigPage );
#endif

	return recv_data;
}

void LED_reset()
{
	// Initialize I2C in fast mode
	i2c_setup(1);

	// Force PixelMap to stop during reset
	Pixel_FrameState = FrameState_Sending;

	// Disable FPS by default
	LED_displayFPS = 0;

	// Hardware shutdown (pull low)
	GPIO_Ctrl( hardware_shutdown_pin, GPIO_Type_DriveSetup, GPIO_Config_Pullup );
	GPIO_Ctrl( hardware_shutdown_pin, GPIO_Type_DriveLow, GPIO_Config_Pullup );
	delay_us(200);

#if ISSI_Chip_31FL3733_define == 1 || ISSI_Chip_31FL3736_define == 1
	// Reset I2C bus
	GPIO_Ctrl( iirst_pin, GPIO_Type_DriveSetup, GPIO_Config_Pulldown );
	GPIO_Ctrl( iirst_pin, GPIO_Type_DriveLow, GPIO_Config_Pulldown );
	delay_us(200);
	GPIO_Ctrl( iirst_pin, GPIO_Type_DriveHigh, GPIO_Config_Pulldown );
	delay_us(200);
	GPIO_Ctrl( iirst_pin, GPIO_Type_DriveLow, GPIO_Config_Pulldown );
	delay_us(500);
#endif

#if ISSI_Chip_31FL3733_define == 1 || ISSI_Chip_31FL3736_define == 1
	// Clear LED Pages
	// Software shutdown
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		uint8_t addr = LED_ChannelMapping[ ch ].addr;
		uint8_t bus = LED_ChannelMapping[ ch ].bus;

		// POR (Power-on-Reset)
		LED_readReg( bus, addr, 0x11, ISSI_ConfigPage );
		delay_us(200);

		// Software shutdown
		LED_writeReg( bus, addr, 0x00, 0x00, ISSI_ConfigPage );
	}
#endif

	// Clear LED Pages
	// Enable LEDs based upon mask
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		uint8_t addr = LED_ChannelMapping[ ch ].addr;
		uint8_t bus = LED_ChannelMapping[ ch ].bus;

#if ISSI_Chip_31FL3733_define == 1 || ISSI_Chip_31FL3736_define == 1
		// Set the enable mask
		LED_sendPage(
			bus,
			addr,
			(uint16_t*)&LED_ledEnableMask[ ch ],
			sizeof( LED_EnableBuffer ) / 2,
			0
		);
#else
		// Clear LED control pages
		LED_zeroPages( bus, addr, 0x00, ISSI_LEDPages, 0x00, ISSI_PageLength ); // LED Registers

		// Copy enable mask to send buffer
		for ( uint8_t reg = 0; reg < LED_EnableBufferLength; reg++ )
		{
			LED_pageBuffer[ ch ].ledctrl[ reg ] =  LED_ledEnableMask[ ch ].buffer[ reg ];
		}
#endif
	}

	// Reset global brightness
	LED_brightness = settings.brightness;

#if ISSI_Chip_31FL3733_define == 1 || ISSI_Chip_31FL3736_define == 1
	// Enable pull-up and pull-down anti-ghosting resistors
	// Set global brightness control
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		uint8_t addr = LED_ChannelMapping[ ch ].addr;
		uint8_t bus = LED_ChannelMapping[ ch ].bus;

		LED_writeReg( bus, addr, 0x01, LED_brightness, ISSI_ConfigPage );
		LED_writeReg( bus, addr, 0x0F, 0x07, ISSI_ConfigPage ); // Pull-up
		LED_writeReg( bus, addr, 0x10, 0x07, ISSI_ConfigPage ); // Pull-down
	}
#elif ISSI_Chip_31FL3732_define == 1
	// Set global brightness control
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		uint8_t addr = LED_ChannelMapping[ ch ].addr;
		uint8_t bus = LED_ChannelMapping[ ch ].bus;

		LED_writeReg( bus, addr, 0x04, LED_brightness, ISSI_ConfigPage );
	}
#endif

	// Setup ISSI frame and sync modes; then disable software shutdown
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		uint8_t addr = LED_ChannelMapping[ ch ].addr;
		uint8_t bus = LED_ChannelMapping[ ch ].bus;

#if ISSI_Chip_31FL3733_define == 1 || ISSI_Chip_31FL3736_define == 1
		// Disable software shutdown
		LED_writeReg( bus, addr, 0x00, 0x01, ISSI_ConfigPage );
		delay_us(200);

		// Enable master sync for the last chip and disable software shutdown
		if ( ch == 0 )
		{
			LED_writeReg( bus, addr, 0x00, 0x41, ISSI_ConfigPage );
		}
		// Slave sync for the rest and disable software shutdown
		else
		{
			LED_writeReg( bus, addr, 0x00, 0x81, ISSI_ConfigPage );
		}

#elif ISSI_Chip_31FL3732_define == 1
		// Enable master sync for the first chip
		if ( ch == 0 )
		{
			LED_writeReg( bus, addr, 0x00, 0x40, ISSI_ConfigPage );
		}
		// Slave sync for the rest
		else
		{
			LED_writeReg( bus, addr, 0x00, 0x80, ISSI_ConfigPage );
		}

		// Disable Software shutdown of ISSI chip
		LED_writeReg( bus, addr, 0x0A, 0x01, ISSI_ConfigPage );
#else
		// Set MODE to Picture Frame
		LED_writeReg( bus, addr, 0x00, 0x00, ISSI_ConfigPage );

		// Disable Software shutdown of ISSI chip
		LED_writeReg( bus, addr, 0x0A, 0x01, ISSI_ConfigPage );
#endif
	}

	// Disable Hardware shutdown of ISSI chips (pull high)
	if ( LED_enable && LED_enable_current )
	{
		GPIO_Ctrl( hardware_shutdown_pin, GPIO_Type_DriveHigh, GPIO_Config_Pullup );
	}

	// Force PixelMap to be ready for the next frame
	Pixel_FrameState = FrameState_Update;

	// Un-pause ISSI processing
	LED_pause = 0;
}

// Detect short or open circuit in Matrix
// Only works with IS31FL3733
void LED_shortOpenDetect()
{
#if ISSI_Chip_31FL3733_define == 1 || ISSI_Chip_31FL3736_define == 1
	// Pause ISSI processing
	LED_pause = 1;

	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		uint8_t addr = LED_ChannelMapping[ ch ].addr;
		uint8_t bus = LED_ChannelMapping[ ch ].bus;

		// Set Global Current Control (needed for accurate reading)
		LED_writeReg( bus, addr, 0x01, 0x01, ISSI_ConfigPage );

		// Enable master sync for the first chip and disable software shutdown
		// Also enable OSD (Open/Short Detect)
		if ( ch == 0 )
		{
			LED_writeReg( bus, addr, 0x00, 0x45, ISSI_ConfigPage );
		}
		// Slave sync for the rest and disable software shutdown
		// Also enable OSD (Open/Short Detect)
		else
		{
			LED_writeReg( bus, addr, 0x00, 0x85, ISSI_ConfigPage );
		}

		// Wait for 3.3 ms before reading the value
		// Needs at least 3.264 ms to query the information
		delay_us(3300);

		// Read registers
		info_print("Bus: ");
		printHex( bus );
		print(" Addr: ");
		printHex( addr );
		print(" - 0x18 -> 0x2F + 0x30 -> 0x47");
		print(NL);

		// Validate open detection
		// TODO
		for ( uint8_t reg = 0x18; reg < 0x30; reg++ )
		{
			uint8_t val = LED_readReg( bus, addr, reg, ISSI_LEDControlPage );
			printHex_op( val, 2 );
			print(" ");
		}
		print(NL);

		// Validate short detection
		// TODO
		for ( uint8_t reg = 0x30; reg < 0x48; reg++ )
		{
			uint8_t val = LED_readReg( bus, addr, reg, ISSI_LEDControlPage );
			printHex_op( val, 2 );
			print(" ");
		}
		print(NL);
	}

	// We have to adjust various settings in order to get the correct reading
	// Reset ISSI configuration
	LED_reset();
#endif
}

// Setup
inline void LED_setup()
{
	// Register Scan CLI dictionary
	CLI_registerDictionary( ledCLIDict, ledCLIDictName );
#if Storage_Enable_define == 1
	Storage_registerModule(&LedStorage);
#endif

	// Zero out FPS time
	LED_timePrev = Time_now();

	// Initialize framerate
	LED_framerate = ISSI_FrameRate_ms_define;

	// Global brightness setting
	LED_brightness = ISSI_Global_Brightness_define;

	// Initialize I2C error counters
	i2c_initial();

	// Setup LED_pageBuffer addresses and brightness section
	LED_pageBuffer[0].i2c_addr = LED_MapCh1_Addr_define;
	LED_pageBuffer[0].reg_addr = ISSI_LEDPwmRegStart;
#if ISSI_Chips_define >= 2
	LED_pageBuffer[1].i2c_addr = LED_MapCh2_Addr_define;
	LED_pageBuffer[1].reg_addr = ISSI_LEDPwmRegStart;
#endif
#if ISSI_Chips_define >= 3
	LED_pageBuffer[2].i2c_addr = LED_MapCh3_Addr_define;
	LED_pageBuffer[2].reg_addr = ISSI_LEDPwmRegStart;
#endif
#if ISSI_Chips_define >= 4
	LED_pageBuffer[3].i2c_addr = LED_MapCh4_Addr_define;
	LED_pageBuffer[3].reg_addr = ISSI_LEDPwmRegStart;
#endif

	// Brightness emulation
#if ISSI_Chip_31FL3731_define
	// Setup LED_pageBuffer addresses and brightness section
	LED_pageBuffer_brightness[0].i2c_addr = LED_MapCh1_Addr_define;
	LED_pageBuffer_brightness[0].reg_addr = ISSI_LEDPwmRegStart;
#if ISSI_Chips_define >= 2
	LED_pageBuffer_brightness[1].i2c_addr = LED_MapCh2_Addr_define;
	LED_pageBuffer_brightness[1].reg_addr = ISSI_LEDPwmRegStart;
#endif
#if ISSI_Chips_define >= 3
	LED_pageBuffer_brightness[2].i2c_addr = LED_MapCh3_Addr_define;
	LED_pageBuffer_brightness[2].reg_addr = ISSI_LEDPwmRegStart;
#endif
#if ISSI_Chips_define >= 4
	LED_pageBuffer_brightness[3].i2c_addr = LED_MapCh4_Addr_define;
	LED_pageBuffer_brightness[3].reg_addr = ISSI_LEDPwmRegStart;
#endif
#endif

	// LED default setting
	LED_enable = ISSI_Enable_define;
	LED_enable_current = ISSI_Enable_define; // Needs a default setting, almost always unset immediately

	// Reset LED sequencing
	LED_reset();

	// Allocate latency resource
	ledLatencyResource = Latency_add_resource("ISSILed", LatencyOption_Ticks);
}


// LED Linked Send
// Call-back for i2c write when updating led display
// TODO Optimize linked send for multiple i2c buses
uint8_t LED_chipSend;
void LED_linkedSend()
{
	// Initialize I2C in slow mode
	i2c_setup(0);

	// Check if we've updated all the ISSI chips for this frame
	if ( LED_chipSend >= ISSI_Chips_define )
	{
		// Now ready to update the frame buffer
		Pixel_FrameState = FrameState_Update;

		// Initialize I2C in fast mode
		i2c_setup(1);

		// Finished sending the buffer, exit linked send
		return;
	}

	// Update ISSI Frame State
	Pixel_FrameState = FrameState_Sending;

	// Lookup bus number
	uint8_t bus = LED_ChannelMapping[ LED_chipSend ].bus;

	/*
	// Debug
	dbug_print("Linked Send: chip(");
	printHex( LED_chipSend );
	print(")addr(");
	printHex( LED_pageBuffer[ LED_chipSend ].i2c_addr );
	print(")reg(");
	printHex( LED_pageBuffer[ LED_chipSend ].reg_addr );
	print(")len(");
	printHex( sizeof( LED_Buffer ) / 2 );
	print(")data[](");
	//for ( uint8_t c = 0; c < 9; c++ )
	for ( uint8_t c = 0; c < sizeof( LED_Buffer ) / 2 - 2; c++ )
	{
		printHex( LED_pageBuffer[ LED_chipSend ].buffer[c] );
		print(" ");
	}
	print("..)" NL);
	*/

	// Artificial delay to assist i2c bus
	const uint32_t delay_tm = ISSI_SendDelay;
	//delay_us( delay_tm );

	// Send, and recursively call this function when finished
	while ( i2c_send_sequence(
		bus,
#if ISSI_Chip_31FL3731_define == 1
		/* Brightness emulation */
		(uint16_t*)&LED_pageBuffer_brightness[ LED_chipSend ],
#else
		(uint16_t*)&LED_pageBuffer[ LED_chipSend ],
#endif
		sizeof( LED_Buffer ) / 2,
		0,
		LED_linkedSend,
		0
	) == -1 )
		delay_us( delay_tm );

	// Increment chip position
	LED_chipSend++;
}


// LED State processing loop
unsigned int LED_currentEvent = 0;
inline void LED_scan()
{
	// Latency measurement start
	Latency_start_time( ledLatencyResource );

	// Check for current change event
	if ( LED_currentEvent )
	{
		// Turn LEDs off in low power mode
		if ( LED_currentEvent < 150 )
		{
			LED_enable_current = 0;

			// Pause animations and clear display
			Pixel_setAnimationControl( AnimationControl_WipePause );
		}
		else
		{
			LED_enable_current = 1;

			// Start animations
			Pixel_setAnimationControl( AnimationControl_Forward );
		}

		LED_currentEvent = 0;
	}

	// Check if an LED_pause is set
	// Some ISSI operations need a clear buffer, but still have the chip running
	if ( LED_pause )
		goto led_finish_scan;

	// Check enable state
	if ( LED_enable && LED_enable_current )
	{
		// Disable Hardware shutdown of ISSI chips (pull high)
		GPIO_Ctrl( hardware_shutdown_pin, GPIO_Type_DriveHigh, GPIO_Config_Pullup );
	}
	// Only write pages to I2C if chip is enabled (i.e. Hardware shutdown is disabled)
	else
	{
		// Enable hardware shutdown
		GPIO_Ctrl( hardware_shutdown_pin, GPIO_Type_DriveLow, GPIO_Config_Pullup );
		goto led_finish_scan;
	}

	// Check if any I2C buses have errored
	// Reset the buses and restart the Frame State
	if ( i2c_error() )
	{
		i2c_reset();
		Pixel_FrameState = FrameState_Update;
	}

	// Only start if we haven't already
	// And if we've finished updating the buffers
	if ( Pixel_FrameState == FrameState_Sending )
		goto led_finish_scan;

	// Only send frame to ISSI chip if buffers are ready
	if ( Pixel_FrameState != FrameState_Ready )
		goto led_finish_scan;

	// Adjust frame rate (i.e. delay and do something else for a bit)
	Time duration = Time_duration( LED_timePrev );
	if ( duration.ms < LED_framerate )
		goto led_finish_scan;

	// FPS Display
	if ( LED_displayFPS )
	{
		// Show frame calculation
		dbug_print("1frame/");
		printInt32( Time_ms( duration ) );
		print("ms + ");
		printInt32( duration.ticks );
		print(" ticks");

		// Check if we're not meeting frame rate
		if ( duration.ms > LED_framerate )
		{
			print(" - Could not meet framerate: ");
			printInt32( LED_framerate );
		}

		print( NL );
	}

	// Emulated brightness control
	// Lower brightness by LED_brightness
#if ISSI_Chip_31FL3731_define == 1
	for ( uint8_t chip = 0; chip < ISSI_Chips_define; chip++ )
	{
		for ( uint8_t ch = 0; ch < LED_EnableBufferLength; ch++ )
		{
			LED_pageBuffer_brightness[ chip ].ledctrl[ ch ] = LED_pageBuffer[ chip ].ledctrl[ ch ];
		}

		for ( uint8_t ch = 0; ch < LED_BufferLength; ch++ )
		{
			// Don't modify is 0
			if ( LED_pageBuffer[ chip ].buffer[ ch ] == 0 || LED_brightness == 0 )
			{
				LED_pageBuffer_brightness[ chip ].buffer[ ch ] = 0;
				continue;
			}

			// XXX (HaaTa) Yes, this is a bit slow, but it's pretty accurate
			LED_pageBuffer_brightness[ chip ].buffer[ ch ] =
				(LED_pageBuffer[ chip ].buffer[ ch ] * LED_brightness) / 0xFF;
		}
	}
#endif

	// Update frame start time
	LED_timePrev = Time_now();

	// Set the page of all the ISSI chips
	// This way we can easily link the buffers to send the brightnesses in the background
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		uint8_t bus = LED_ChannelMapping[ ch ].bus;
		// Page Setup
		LED_setupPage(
			bus,
			LED_ChannelMapping[ ch ].addr,
			ISSI_LEDPwmPage
		);
	}

	// Send current set of buffers
	// Uses interrupts to send to all the ISSI chips
	// Pixel_FrameState will be updated when complete
	LED_chipSend = 0; // Start with chip 0
	LED_linkedSend();

led_finish_scan:
	// Latency measurement end
	Latency_end_time( ledLatencyResource );
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
typedef enum LedControl {
	// Set all LEDs - with argument
	LedControl_brightness_decrease_all = 0,
	LedControl_brightness_increase_all = 1,
	LedControl_brightness_set_all      = 2,
	LedControl_brightness_default      = 10,
	// Set all LEDs - no argument
	LedControl_off                     = 3,
	LedControl_on                      = 4,
	LedControl_toggle                  = 5,
	// FPS Control - with argument
	LedControl_set_fps                 = 6,
	LedControl_increase_fps            = 7,
	LedControl_decrease_fps            = 8,
	LedControl_default_fps             = 9,
} LedControl;

void LED_control( LedControl control, uint8_t arg )
{
	switch ( control )
	{
	case LedControl_brightness_decrease_all:
		LED_enable = 1;
		// Only decrease to zero
		if ( LED_brightness - arg < 0 )
		{
			LED_brightness = 0;
		}
		else
		{
			LED_brightness -= arg;
		}
		break;

	case LedControl_brightness_increase_all:
		LED_enable = 1;
		// Only increase to max
		if ( LED_brightness + arg > 0xFF )
		{
			LED_brightness = 0xFF;
		}
		else
		{
			LED_brightness += arg;
		}
		break;

	case LedControl_brightness_set_all:
		LED_enable = 1;
		LED_brightness = arg;
		break;

	case LedControl_off:
		LED_enable = 0;
		return;

	case LedControl_on:
		LED_enable = 1;
		return;

	case LedControl_toggle:
		LED_enable = !LED_enable;
		return;

	case LedControl_set_fps:
		LED_framerate = (uint32_t)arg;
		return;

	case LedControl_increase_fps:
		if ( LED_framerate > 0 )
		{
			// Smaller timeout, higher FPS
			LED_framerate -= arg;
		}
		return;

	case LedControl_decrease_fps:
		if ( LED_framerate < 0xFF )
		{
			// Higher timeout, lower FPS
			LED_framerate += arg;
		}
		return;

	case LedControl_default_fps:
		LED_framerate = ISSI_FrameRate_ms_define;
		return;

	case LedControl_brightness_default:
		LED_brightness = ISSI_Global_Brightness_define;
		return;
	}

#if ISSI_Chip_31FL3733_define == 1 || ISSI_Chip_31FL3732_define == 1 || ISSI_Chip_31FL3736_define == 1
	// Update brightness
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		uint8_t addr = LED_ChannelMapping[ ch ].addr;
		uint8_t bus = LED_ChannelMapping[ ch ].bus;

#if ISSI_Chip_31FL3733_define == 1 || ISSI_Chip_31FL3736_define == 1
		LED_writeReg( bus, addr, 0x01, LED_brightness, ISSI_ConfigPage );
#elif ISSI_Chip_31FL3732_define == 1
		LED_writeReg( bus, addr, 0x04, LED_brightness, ISSI_ConfigPage );
#endif
	}
#elif ISSI_Chip_31FL3731_define == 1
	// XXX (HaaTa) - Brightness is emulated
#endif
}

void LED_SetPixels( uint8_t shouldSetOn )
{
	if ( shouldSetOn )
		LED_control( LedControl_on, 0 );
	else
		LED_control( LedControl_off, 0 );
}

void LED_control_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Initial:
		// Only use capability on press
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("LED_control_capability(mode,amount)");
		return;
	default:
		return;
	}

	// Set the input structure
	LedControl control = (LedControl)args[0];
	uint8_t arg = (uint8_t)args[1];

	// Interconnect broadcasting
#if defined(ConnectEnabled_define)
	// By default send to the *next* node, which will determine where to go next
	extern uint8_t Connect_id; // connect_scan.c
	uint8_t addr = Connect_id + 1;

	// Send interconnect remote capability packet
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
#endif

	// Modify led state of this node
	LED_control( control, arg );
}



// ----- CLI Command Functions -----

void cliFunc_ledCheck( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	// TODO check for shorts and n/c points
	LED_shortOpenDetect();
}

void cliFunc_ledReset( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	// Reset I2C bus
#if ISSI_Chip_31FL3733_define == 1 || ISSI_Chip_31FL3736_define == 1
	GPIO_Ctrl( iirst_pin, GPIO_Type_DriveSetup, GPIO_Config_Pullup );
	GPIO_Ctrl( iirst_pin, GPIO_Type_DriveHigh, GPIO_Config_Pullup );
	delay_us(50);
	GPIO_Ctrl( iirst_pin, GPIO_Type_DriveLow, GPIO_Config_Pullup );
#endif
	i2c_reset();

	// Clear buffers
	for ( uint8_t buf = 0; buf < ISSI_Chips_define; buf++ )
	{
		memset( (void*)LED_pageBuffer[ buf ].buffer, 0, LED_BufferLength * 2 );
	}

	// Reset LEDs
	LED_reset();
}

void cliFunc_ledFPS( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Process speed argument if given
	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Just toggling FPS display
	if ( *arg1Ptr == '\0' )
	{
		info_print("FPS Toggle");
		LED_displayFPS = !LED_displayFPS;
		return;
	}

	// Check if f argument was given
	switch ( *arg1Ptr )
	{
	case 'r': // Reset framerate
	case 'R':
		LED_framerate = ISSI_FrameRate_ms_define;
		break;

	default: // Convert to a number
		LED_framerate = numToInt( arg1Ptr );
		break;
	}

	// Show result
	info_print("Setting framerate to: ");
	printInt32( LED_framerate );
	print("ms");
}

void cliFunc_ledToggle( char* args )
{
	print( NL ); // No \r\n by default after the command is entered
	info_print("LEDs Toggle");
	LED_enable = !LED_enable;
}

void LED_setBrightness(uint8_t brightness) {
	LED_brightness = brightness;

#if ISSI_Chip_31FL3733_define == 1 || ISSI_Chip_31FL3732_define == 1 || ISSI_Chip_31FL3736_define == 1
	// Update brightness
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		uint8_t addr = LED_ChannelMapping[ ch ].addr;
		uint8_t bus = LED_ChannelMapping[ ch ].bus;

#if ISSI_Chip_31FL3733_define == 1 || ISSI_Chip_31FL3736_define == 1
		LED_writeReg( bus, addr, 0x01, LED_brightness, ISSI_ConfigPage );
#elif ISSI_Chip_31FL3732_define == 1
		LED_writeReg( bus, addr, 0x04, LED_brightness, ISSI_ConfigPage );
#endif
	}
#elif ISSI_Chip_31FL3731_define == 1
	// XXX (HaaTa) - Brightness is emulated
#endif
}

void cliFunc_ledSet( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Process speed argument if given
	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Reset brightness
	if ( *arg1Ptr == '\0' )
	{
		LED_setBrightness( settings.brightness );
	}
	else
	{
		LED_setBrightness( numToInt(arg1Ptr) );
	}

	info_print("LED Brightness Set");
}

#if Storage_Enable_define == 1
void LED_loadConfig() {
	LED_setBrightness(settings.brightness);
	LED_framerate = settings.framerate;
}

void LED_saveConfig() {
	settings.brightness = LED_brightness;
	settings.framerate = LED_framerate;
}

void LED_printConfig() {
	print(" \033[35mBrightness\033[0m       ");
	printInt8(settings.brightness);
	print( NL );
	print(" \033[35mFramerate (ms/f)\033[0m ");
	printInt8(settings.framerate);
	print( NL );
}
#endif
