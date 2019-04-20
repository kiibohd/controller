/* Copyright (C) 2014-2019 by Jacob Alexander
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
#include <spi.h>

// Interconnect module if compiled in
#if defined(ConnectEnabled_define)
#include <connect_scan.h>
#endif

// KLL Include
#include <kll.h>

// Local Includes
#include "led_scan.h"



// ----- Defines -----

// ISSI Addresses
// IS31FL3743B (1 chip per CS)
#if ISSI_Chip_31FL3743B_define == 1
#define LED_BufferLength       198
#define LED_ScalingLength      198

#define ISSI_ConfigPage        0x52
#define ISSI_ConfigPageLength  0x2F
#define ISSI_LEDScalePage      0x51
#define ISSI_LEDPwmPage        0x50
#define ISSI_PageLength        0xC6

#else
#error "ISSI Driver Chip not defined in Scan scancode_map.kll..."
#endif

#define LED_TotalChannels     (LED_BufferLength * ISSI_Chips_define)



// ----- Macros -----

#define LED_ChannelMapDefine(ch) ch

#define LED_MaskDefine(ch) \
	{ \
		{ ISSILedMask##ch##_define }, \
	}



// ----- Structs -----

typedef struct LED_Buffer {
	uint8_t buffer[LED_BufferLength];
} LED_Buffer;

typedef struct LED_EnableBuffer {
	uint8_t buffer[LED_EnableBufferLength];
} LED_EnableBuffer;



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

// Output buffer used for SPI
volatile SPI_Packet LED_spi_buffer[ISSI_Chips_define * LED_BufferLength];
volatile SPI_Transaction LED_spi_transaction;

extern LED_Buffer LED_pageBuffer[ISSI_Chips_define];

uint8_t LED_displayFPS;     // Display fps to cli
uint8_t LED_enable;         // Enable/disable ISSI chips
uint8_t LED_enable_current; // Enable/disable ISSI chips (based on USB current availability)
uint8_t LED_pause;          // Pause ISSI updates
uint8_t LED_brightness;     // Global brightness for LEDs

uint32_t LED_framerate;     // Configured led framerate, given in ms per frame

Time LED_timePrev; // Last frame processed


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

// Latency measurement resource
static uint8_t ledLatencyResource;



// ----- Functions -----

// Write register on all ISSI chips
// Prepare pages first, then attempt write register with a minimal delay between chips
// page - ISSI page
// reg - Register address
// val - Value to write
void LED_syncReg(uint8_t page, uint8_t reg, uint8_t val)
{
	// Setup packet sequence for writing
	SPI_Packet buffer[3] = {
		{
			.lastxfer = 0,
			.pcs = cs,
			.data = page,
		},
		{
			.lastxfer = 0,
			.pcs = cs,
			.data = reg,
		},
		{
			.lastxfer = 1,
			.pcs = cs,
			.data = val,
		},
	};

	// Build each of the transactions
	volatile SPI_Transaction transaction[ISSI_Chips_define];
	for (uint8_t ch = 0; ch < ISSI_Chips_define; ch++)
	{
		// Setup transaction
		transaction[ch] = {
			.status = SPI_Transaction_Status_None,
			.rx_buffer = {
				.ul_addr = NULL,
				.ul_size = 0,
			},
			.tx_buffer = {
				.ul_addr = (uint32_t)buffer,
				.ul_size = 3,
			},
		};

		// Queue transaction
		spi_add_transaction(&transaction[ch]);
	}

	// Wait for final transaction to complete
	while (transaction[ISSI_Chips_define - 1].status != SPI_Transaction_Status_Finished)
		delay_us(ISSI_SendDelay);
}

// Write address
// cs - SPI Channel
// page - ISSI page
// reg - Register address
// val - Value to write
void LED_writeReg(uint8_t cs, uint8_t page, uint8_t reg, uint8_t val)
{
	/*
	info_msg("SPI Write cs(");
	printHex( cs );
	print(")page(");
	printHex( page );
	print(")reg(");
	printHex( reg );
	print(")val(");
	printHex( val );
	print(")" NL);
	*/

	// Setup packet sequence for writing
	SPI_Packet buffer[3] = {
		{
			.lastxfer = 0,
			.pcs = cs,
			.data = page,
		},
		{
			.lastxfer = 0,
			.pcs = cs,
			.data = reg,
		},
		{
			.lastxfer = 1,
			.pcs = cs,
			.data = val,
		},
	};

	// Setup transaction
	volatile SPI_Transaction transaction = {
		.status = SPI_Transaction_Status_None,
		.rx_buffer = {
			.ul_addr = NULL,
			.ul_size = 0,
		},
		.tx_buffer = {
			.ul_addr = (uint32_t)buffer,
			.ul_size = 3,
		},
	};

	// Queue transaction
	spi_add_transaction( &transaction );

	// Wait for transaction to complete
	while (transaction.status != SPI_Transaction_Status_Finished)
		delay_us( ISSI_SendDelay );
}

// Read address
// cs - SPI Channel
// page - ISSI page
// reg - Register address
uint8_t LED_readReg(uint8_t cs, uint8_t page, uint8_t reg)
{
	/*
	info_msg("SPI Read cs(");
	printHex( cs );
	print(")page(");
	printHex( page );
	print(")reg(");
	printHex( reg );
	print(")" NL );
	*/

	// Setup packet sequence for writing
	SPI_Packet txbuffer[2] = {
		{
			.lastxfer = 0,
			.pcs = cs,
			.data = page | 0x80, // Read bit
		},
		{
			.lastxfer = 0,
			.pcs = cs,
			.data = reg,
		},
	};
	SPI_Packet rxbuffer;

	// Setup transaction
	volatile SPI_Transaction transaction = {
		.status = SPI_Transaction_Status_None,
		.rx_buffer = {
			.ul_addr = (uint32_t)rxbuffer,
			.ul_size = 1,
		},
		.tx_buffer = {
			.ul_addr = (uint32_t)txbuffer,
			.ul_size = 2,
		},
	};

	// Queue transaction
	spi_add_transaction( &transaction );

	// Wait for transaction to complete
	while ( transaction.status != SPI_Transaction_Status_Finished )
		delay_us( ISSI_SendDelay );

	return (uint8_t)rxbuffer.data;
}

void LED_reset()
{
	// Force PixelMap to stop during reset
	Pixel_FrameState = FrameState_Sending;

	// Disable FPS by default
	LED_displayFPS = 0;

	// Enable Hardware shutdown (pull low)
	GPIO_Ctrl(hardware_shutdown_pin, GPIO_Type_DriveSetup, GPIO_Config_Pullup);
	GPIO_Ctrl(hardware_shutdown_pin, GPIO_Type_DriveLow, GPIO_Config_Pullup);
	delay_us(50);

	// Disable Hardware shutdown of ISSI chips (pull high)
	if ( LED_enable && LED_enable_current )
	{
		GPIO_Ctrl(hardware_shutdown_pin, GPIO_Type_DriveHigh, GPIO_Config_Pullup);
	}

	// Clear LED Pages
	// Call reset to clear all registers
	LED_syncReg(ISSI_ConfigPage, 0x2F, 0xAE);

	// Setup enable mask/scaling per channel
	for (uint8_t cs = 0; cs < ISSI_Chips_define; cs++)
	{
		for (uint16_t pkt = 0; pkt < LED_BufferLength; pkt++)
		{
			SPI_Packet *pos = &LED_spi_buffer[cs * pkt];
			pos->data = LED_ledEnableMask[cs].buffer[pkt];
		}
	}

	// Set the final pkt as the last packet
	LED_spi_buffer[ISSI_Chips_define * LED_BufferLength - 1].lastxfer = 1;

	// Setup spi transaction
	LED_spi_transaction = {
		.status = SPI_Transaction_Status_None,
		.rx_buffer = {
			.ul_addr = NULL,
			.ul_size = 0,
		},
		.tx_buffer = {
			.ul_addr = (uint32_t)&LED_spi_buffer,
			.ul_size = ISSI_Chips_define * LED_BufferLength,
		},
	};

	// Send scaling setup via spi
	// XXX (HaaTa): No need to wait as the next register set will wait for us
	spi_add_transaction(&LED_spi_transaction);

	// Reset global brightness
	LED_brightness = settings.brightness;

	// Set global brightness control
	LED_syncReg(ISSI_ConfigPage, 0x01, LED_brightness);

	// Enable pul-up and pull-down anti-ghosting resistors
	LED_syncReg(ISSI_ConfigPage, 0x02, 0x33);

	// Set tempeature roll-off
	LED_syncReg(ISSI_ConfigPage, 0x24, 0x00);

	// Setup ISSI sync and spread spectrum function
	for (uint8_t ch = 0; ch < ISSI_Chips_define; ch++)
	{
		// Enable master sync for the last chip and disable software shutdown
		// XXX (HaaTa); The last chip is used as it is the last chip all of the frame data is sent to
		// This is imporant as it may take more time to send the packet than the ISSI chip can handle
		// between frames.
		if (ch == ISSI_Chips_define - 1)
		{
			LED_writeReg(ch, ISSI_ConfigPage, 0x25, 0xC0);
		}
		// Slave sync for the rest and disable software shutdown
		else
		{
			LED_writeReg(ch, ISSI_ConfigPage, 0x25, 0x80);
		}
	}

	// Disable software shutdown
	LED_syncReg(ISSI_ConfigPage, 0x00, 0x01);

	// Force PixelMap to be ready for the next frame
	LED_spi_transaction.status = SPI_Transaction_Status_None;
	Pixel_FrameState = FrameState_Update;

	// Un-pause ISSI processing
	LED_pause = 0;
}

// Detect short or open circuit in Matrix
// - Uses LED Mask to determine which LED channels are working/valid
// - Detect shorts, used for manufacturing test (bad channel)
// - Detect opens, used for manufacturing test (bad solder/channel)
//   * LED Mask will define which LEDs are expected to be working
void LED_shortOpenDetect()
{
	// Pause ISSI processing
	LED_pause = 1;

	// Set Global Current Control (needed for accurate reading)
	LED_syncReg(ISSI_ConfigPage, 0x01, 0x0F);

	// -- Case 1: Open detection (bad soldering/no led) --

	// Disable pull resistors
	LED_syncReg(ISSI_ConfigPage, 0x02, 0x00);

	// Set OSD to open detection
	LED_syncReg(ISSI_ConfigPage, 0x00, 0x03);

	// Must wait for 750 us before reading (2 cycles)
	// Waiting 1 ms to be safe
	delay_us(1000);

	// TODO Validation
	info_print("Open Detection");
	for (uint8_t cs = 0; cs < ISSI_Chips_define; cs++)
	{
		for (uint8_t reg = 0x03; reg <= 0x23; reg++)
		{
			uint8_t val = LED_readReg(cs, ISSI_ConfigPage, reg);
			printHex_op( val, 2 );
			print(" ");
		}
		print(NL);
	}

	// -- Case 2: Short detection (bad soldering/bad led) --

	// Set pull down resistors
	LED_syncReg(ISSI_ConfigPage, 0x02, 0x30);

	// Set OSD to short detection
	LED_syncReg(ISSI_ConfigPage, 0x00, 0x05);

	// Must wait for 750 us before reading (2 cycles)
	// Waiting 1 ms to be safe
	delay_us(1000);

	// TODO Validation
	info_print("Short Detection");
	for (uint8_t cs = 0; cs < ISSI_Chips_define; cs++)
	{
		for (uint8_t reg = 0x03; reg <= 0x23; reg++)
		{
			uint8_t val = LED_readReg(cs, ISSI_ConfigPage, reg);
			printHex_op( val, 2 );
			print(" ");
		}
		print(NL);
	}

	// We have to adjust various settings in order to get the correct reading
	// Reset ISSI configuration
	LED_reset();
}

// Setup
inline void LED_setup()
{
	// Register Scan CLI dictionary
	CLI_registerDictionary(ledCLIDict, ledCLIDictName);
#if Storage_Enable_define == 1
	Storage_registerModule(&LedStorage);
#endif

	// Zero out FPS time
	LED_timePrev = Time_now();

	// Initialize framerate
	LED_framerate = ISSI_FrameRate_ms_define;

	// Global brightness setting
	LED_brightness = ISSI_Global_Brightness_define;

	// Initialize SPI
	spi_setup();

	// Setup LED_spi_buffer to include needed settings on SAM4S
	for (uint8_t cs = 0; cs < ISSI_Chips_define; cs++)
	{
		for (uint16_t pkt = 0; pkt < LED_BufferLength; pkt++)
		{
			SPI_Packet *pos = &LED_spi_buffer[cs * pkt];
			pos->lastxfer = 0;
			pos->pcs = cs;
			pos->data = 0;
		}
	}

	// LED default setting
	LED_enable = ISSI_Enable_define;
	LED_enable_current = ISSI_Enable_define; // Needs a default setting, almost always unset immediately

	// Enable Hardware shutdown (pull low)
	GPIO_Ctrl(hardware_shutdown_pin, GPIO_Type_DriveSetup, GPIO_Config_Pullup);
	GPIO_Ctrl(hardware_shutdown_pin, GPIO_Type_DriveLow, GPIO_Config_Pullup);

	// Call reset to clear all registers
	LED_syncReg(ISSI_ConfigPage, 0x2F, 0xAE);

	// Disable Hardware shutdown of ISSI chips (pull high)
	if (LED_enable && LED_enable_current)
	{
		GPIO_Ctrl(hardware_shutdown_pin, GPIO_Type_DriveHigh, GPIO_Config_Pullup);
	}

	// Reset LED sequencing
	LED_reset();

	// Allocate latency resource
	ledLatencyResource = Latency_add_resource("ISSILedSPI", LatencyOption_Ticks);
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

	// Check status of SPI transaction
	// Only allow frame updating once the transaction has finished
	if (LED_spi_transaction.status == SPI_Transaction_Status_Finished)
	{
		// SPI transaction finished
		LED_spi_transaction.status = SPI_Transaction_Status_None;
		Pixel_FrameState = FrameState_Update;
	}

	// Check if an LED_pause is set
	// Some ISSI operations need a clear buffer, but still have the chip running
	if ( LED_pause )
	{
		goto led_finish_scan;
	}

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
		dbug_msg("1frame/");
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

	// Update frame start time
	LED_timePrev = Time_now();

	// Do a sparse copy from the LED Buffer to the SPI Buffer
	for (uint8_t cs = 0; cs < ISSI_Chips_define; cs++)
	{
		for (uint16_t pkt = 0; pkt < LED_BufferLength; pkt++)
		{
			SPI_Packet *pos = &LED_spi_buffer[cs * pkt];
			pos->data = LED_pageBuffer[cs].buffer[pkt];
		}
	}

	// Setup spi transaction
	LED_spi_transaction = {
		.status = SPI_Transaction_Status_None,
		.rx_buffer = {
			.ul_addr = NULL,
			.ul_size = 0,
		},
		.tx_buffer = {
			.ul_addr = (uint32_t)&LED_spi_buffer,
			.ul_size = ISSI_Chips_define * LED_BufferLength,
		},
	};

	// Send scaling setup via spi
	// Purposefully not waiting, this will send in the background without interrupts or CPU interference
	// An interrupt is only used to move onto the next transaction if any are queued
	spi_add_transaction(&LED_spi_transaction);


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

	// Update brightness
	LED_syncReg(ISSI_ConfigPage, 0x01, LED_brightness);
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

	LED_shortOpenDetect();
}

void cliFunc_ledReset( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

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
		info_msg("FPS Toggle");
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
	info_msg("Setting framerate to: ");
	printInt32( LED_framerate );
	print("ms");
}

void cliFunc_ledToggle( char* args )
{
	print( NL ); // No \r\n by default after the command is entered
	info_msg("LEDs Toggle");
	LED_enable = !LED_enable;
}

void LED_setBrightness(uint8_t brightness) {
	LED_brightness = brightness;

	// Update brightness
	LED_syncReg(ISSI_ConfigPage, 0x01, LED_brightness);
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

	info_msg("LED Brightness Set");
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
