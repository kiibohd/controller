/* Copyright (C) 2014-2015 by Jacob Alexander
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
#if 1
#define ISSI_Ch1 0xE8
#define ISSI_Ch2 0xEA
#define ISSI_Ch3 0xEC
#define ISSI_Ch4 0xEE

// IS31FL3732 (max 16 channels per bus)
#else
#define ISSI_Ch1 0xB0
#define ISSI_Ch2 0xB2
#define ISSI_Ch3 0xB4
#define ISSI_Ch4 0xB6
#endif



// ----- Macros -----

#define LED_MaskDefine(ch) \
	{ \
		ISSI_Ch##ch, /* I2C address */ \
		0x00, /* Starting register address */ \
		{ ISSILedMask##ch##_define }, \
	}

#define LED_BrightnessDefine(ch) \
	{ \
		ISSI_Ch##ch, /* I2C address */ \
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



// ----- Function Declarations -----

// CLI Functions
void cliFunc_i2cSend  ( char* args );
void cliFunc_ledCtrl  ( char* args );
void cliFunc_ledNFrame( char* args );
void cliFunc_ledStart ( char* args );
void cliFunc_ledTest  ( char* args );
void cliFunc_ledWPage ( char* args );
void cliFunc_ledZero  ( char* args );



// ----- Variables -----

// Scan Module command dictionary
CLIDict_Entry( i2cSend,     "Send I2C sequence of bytes. Use |'s to split sequences with a stop." );
CLIDict_Entry( ledCtrl,     "Basic LED control. Args: <mode> <amount> [<index>]" );
CLIDict_Entry( ledNFrame,   "Increment led frame." );
CLIDict_Entry( ledStart,    "Disable software shutdown." );
CLIDict_Entry( ledTest,     "Test out the led pages." );
CLIDict_Entry( ledWPage,    "Write to given register page starting at address. i.e. 0xE8 0x2 0x24 0xF0 0x12" );
CLIDict_Entry( ledZero,     "Zero out LED register pages (non-configuration)." );

CLIDict_Def( ledCLIDict, "ISSI LED Module Commands" ) = {
	CLIDict_Item( i2cSend ),
	CLIDict_Item( ledCtrl ),
	CLIDict_Item( ledNFrame ),
	CLIDict_Item( ledStart ),
	CLIDict_Item( ledTest ),
	CLIDict_Item( ledWPage ),
	CLIDict_Item( ledZero ),
	{ 0, 0, 0 } // Null entry for dictionary end
};


LED_Buffer LED_pageBuffer[ ISSI_Chips_define ];

         uint8_t LED_FrameBuffersReady; // Starts at maximum, reset on interrupt from ISSI
volatile uint8_t LED_FrameBufferReset;  // INTB interrupt received, reset available buffer count when ready
         uint8_t LED_FrameBufferPage;   // Current page of the buffer
         uint8_t LED_FrameBufferStart;  // Whether or not a start signal can be sent

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

void LED_zeroPages( uint8_t addr, uint8_t startPage, uint8_t numPages, uint8_t startReg, uint8_t endReg )
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
		while ( i2c_send( pageSetup, sizeof( pageSetup ) / 2 ) == -1 )
			delay(1);

		// Zero out page
		while ( i2c_send( clearPage, 2 + endReg - startReg ) == -1 )
			delay(1);
	}

	// Wait until finished zero'ing
	while ( i2c_busy() )
		delay(1);
}

void LED_sendPage( uint8_t addr, uint16_t *buffer, uint32_t len, uint8_t page )
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
	while ( i2c_send( pageSetup, sizeof( pageSetup ) / 2 ) == -1 )
		delay(1);

	// Write page to I2C Tx Buffer
	while ( i2c_send( buffer, len ) == -1 )
		delay(1);
}

// Write address
void LED_writeReg( uint8_t addr, uint8_t reg, uint8_t val, uint8_t page )
{
	// Page Setup
	uint16_t pageSetup[] = { addr, 0xFD, page };

	// Reg Write Setup
	uint16_t writeData[] = { addr, reg, val };

	// Setup page
	while ( i2c_send( pageSetup, sizeof( pageSetup ) / 2 ) == -1 )
		delay(1);

	// Write register
	while ( i2c_send( writeData, sizeof( writeData ) / 2 ) == -1 )
		delay(1);

	// Delay until written
	while ( i2c_busy() )
		delay(1);
}

// Read address
// TODO Not working?
uint8_t LED_readReg( uint8_t addr, uint8_t reg, uint8_t page )
{
	// Software shutdown must be enabled to read registers
	LED_writeReg( addr, 0x0A, 0x00, 0x0B );

	// Page Setup
	uint16_t pageSetup[] = { addr, 0xFD, page };

	// Setup page
	while ( i2c_send( pageSetup, sizeof( pageSetup ) / 2 ) == -1 )
		delay(1);

	// Register Setup
	uint16_t regSetup[] = { addr, reg };

	// Configure register
	while ( i2c_send( regSetup, sizeof( regSetup ) / 2 ) == -1 )
		delay(1);

	// Register Read Command
	uint16_t regReadCmd[] = { addr | 0x1, I2C_READ };
	uint8_t recv_data;

	// Request single register byte
	while ( i2c_read( regReadCmd, sizeof( regReadCmd ) / 2, &recv_data ) == -1 )
		delay(1);

	// Disable software shutdown
	LED_writeReg( addr, 0x0A, 0x01, 0x0B );

	return recv_data;
}

// Setup
inline void LED_setup()
{
	// Register Scan CLI dictionary
	CLI_registerDictionary( ledCLIDict, ledCLIDictName );

	// Initialize I2C
	i2c_setup();

	// Setup LED_pageBuffer addresses and brightness section
	LED_pageBuffer[0].i2c_addr = ISSI_Ch1;
	LED_pageBuffer[0].reg_addr = 0x24;
#if ISSI_Chips_define >= 2
	LED_pageBuffer[1].i2c_addr = ISSI_Ch2;
	LED_pageBuffer[1].reg_addr = 0x24;
#endif
#if ISSI_Chips_define >= 3
	LED_pageBuffer[2].i2c_addr = ISSI_Ch3;
	LED_pageBuffer[2].reg_addr = 0x24;
#endif
#if ISSI_Chips_define >= 4
	LED_pageBuffer[3].i2c_addr = ISSI_Ch4;
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
		uint8_t addr = LED_pageBuffer[ ch ].i2c_addr;
		LED_zeroPages( addr, 0x0B, 1, 0x00, 0x0C ); // Control Registers
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

	// Reset frame buffer used count
	LED_FrameBuffersReady = LED_FrameBuffersMax;

	// Starting page for the buffers
	LED_FrameBufferPage = 4;

	// Initially do not allow autoplay to restart
	LED_FrameBufferStart = 0;

	// Clear LED Pages
	// Enable LEDs based upon mask
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		uint8_t addr = LED_pageBuffer[ ch ].i2c_addr;
		LED_zeroPages( addr, 0x00, 8, 0x00, 0xB4 ); // LED Registers

		// For each page
		for ( uint8_t pg = 0; pg < LED_FrameBuffersMax * 2; pg++ )
		{
			LED_sendPage(
				addr,
				(uint16_t*)&LED_ledEnableMask[ ch ],
				sizeof( LED_EnableBuffer ) / 2,
				pg
			);
		}
	}

	// Setup ISSI auto frame play, but do not start yet
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		uint8_t addr = LED_pageBuffer[ ch ].i2c_addr;
		// CNS 1 loop, FNS 4 frames - 0x14
		LED_writeReg( addr, 0x02, 0x14, 0x0B );

		// Default refresh speed - TxA
		// T is typically 11ms
		// A is 1 to 64 (where 0 is 64)
		LED_writeReg( addr, 0x03, ISSI_AnimationSpeed_define, 0x0B );

		// Set MODE to Auto Frame Play
		LED_writeReg( addr, 0x00, 0x08, 0x0B );
	}

	// Disable Software shutdown of ISSI chip
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		uint8_t addr = LED_pageBuffer[ ch ].i2c_addr;
		LED_writeReg( addr, 0x0A, 0x01, 0x0B );
	}
}


// LED Linked Send
// Call-back for i2c write when updating led display
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

	// Debug
	/*
	dbug_msg("Linked Send: chip(");
	printHex( LED_chipSend );
	print(") frame(");
	printHex( LED_FrameBufferPage );
	print(") addr(");
	printHex( LED_pageBuffer[0].i2c_addr );
	print(") reg(");
	printHex( LED_pageBuffer[0].reg_addr );
	print(") len(");
	printHex( sizeof( LED_Buffer ) / 2 );
	print(") data[]" NL "(");
	for ( uint8_t c = 0; c < 9; c++ )
	//for ( uint8_t c = 0; c < sizeof( LED_Buffer ) / 2 - 2; c++ )
	{
		printHex( LED_pageBuffer[0].buffer[c] );
		print(" ");
	}
	print(")" NL);
	*/

	// Send, and recursively call this function when finished
	while ( i2c_send_sequence(
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
inline void LED_scan()
{
	// Check to see if frame buffers are ready to replenish
	if ( LED_FrameBufferReset )
	{
		LED_FrameBufferReset = 0;
		LED_FrameBufferStart = 1;

		// Debug Status
		dbug_msg("4frames/");
		printInt32( systick_millis_count - LED_timePrev );
		LED_timePrev = systick_millis_count;
		print("ms" NL);
	}

	// Make sure there are buffers available
	if ( LED_FrameBuffersReady == 0 )
	{
		// Only start if we haven't already
		// And if we've finished updating the buffers
		if ( !LED_FrameBufferStart || Pixel_FrameState == FrameState_Sending )
			return;

		// Buffers are now full, start signal can be sent
		for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
		{
			uint8_t addr = LED_pageBuffer[ ch ].i2c_addr;

			// Start Auto Frame Play on either frame 1 or 5
			uint8_t frame = LED_FrameBufferPage == 0 ? 4 : 0;
			LED_writeReg( addr, 0x00, 0x08 | frame, 0x0B );
		}

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
		uint8_t addr = LED_pageBuffer[ ch ].i2c_addr;
		uint16_t pageSetup[] = { addr, 0xFD, LED_FrameBufferPage };

		// Send each update
		while ( i2c_send( pageSetup, sizeof( pageSetup ) / 2 ) == -1 )
			delay(1);
	}

	// Send current set of buffers
	// Uses interrupts to send to all the ISSI chips
	// Pixel_FrameState will be updated when complete
	LED_chipSend = 0; // Start with chip 0
	LED_linkedSend();
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
		LED_sendPage( LED_pageBuffer[ ch ].i2c_addr, (uint16_t*)&LED_pageBuffer[ ch ], sizeof( LED_Buffer ) / 2, 0 );
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
}

void cliFunc_ledWPage( char* args )
{
	/*
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
	*/
}

void cliFunc_ledStart( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		LED_zeroPages( LED_ledEnableMask[ ch ].i2c_addr, 0x0B, 1, 0x00, 0x0C ); // Control Registers
		//LED_zeroPages( 0x00, 8, 0x00, 0xB4 ); // LED Registers
		LED_writeReg( LED_ledEnableMask[ ch ].i2c_addr, 0x0A, 0x01, 0x0B );
		LED_sendPage( LED_ledEnableMask[ ch ].i2c_addr, (uint16_t*)&LED_ledEnableMask[ ch ], sizeof( LED_EnableBuffer ) / 2, 0 );
	}
}

void cliFunc_ledTest( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;
	uint8_t speed = ISSI_AnimationSpeed_define;

	// Process speed argument if given
	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Stop processing args if no more are found
	if ( *arg1Ptr != '\0' )
		speed = numToInt( arg1Ptr );


	// TODO REMOVEME
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		uint8_t addr = LED_pageBuffer[ ch ].i2c_addr;
		// CNS 1 loop, FNS 4 frames - 0x14
		//LED_writeReg( addr, 0x02, 0x14, 0x0B );

		// Default refresh speed - TxA
		// T is typically 11ms
		// A is 1 to 64 (where 0 is 64)
		LED_writeReg( addr, 0x03, speed, 0x0B );

		// Set MODE to Auto Frame Play
		// Set FS to 5, this is to train the IRQ on the ISSI for the processing loop
		//  The first 4 frames are blank, we fill the last 4
		//  Clear the interrupt, and the movie display starts at frame 5
		//LED_writeReg( addr, 0x00, 0x0C, 0x0B );
	}
	return;

	// Zero out Frame Registers
	// This needs to be done before disabling the hardware shutdown (or the leds will do undefined things)
	info_print("LED - Zeroing out all pages");
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		uint8_t addr = LED_pageBuffer[ ch ].i2c_addr;
		LED_zeroPages( addr, 0x0B, 1, 0x00, 0x0C ); // Control Registers
		LED_zeroPages( addr, 0x00, 8, 0x00, 0xB4 ); // LED Registers
	}

	// Clear LED Pages
	// Enable LEDs based upon mask
	info_print("LED - Setting LED enable mask");
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	//for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		uint8_t addr = LED_pageBuffer[ ch ].i2c_addr;

		// For each page
		for ( uint8_t pg = 0; pg < LED_FrameBuffersMax * 2; pg++ )
		{
			LED_sendPage(
				addr,
				(uint16_t*)&LED_ledEnableMask[ ch ],
				sizeof( LED_EnableBuffer ) / 2,
				pg
			);
		}
	}

	// Setup ISSI auto frame play, but do not start yet
	/*
	info_print("LED - Enabling 8 frame 3 loop auto play");
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		uint8_t addr = LED_pageBuffer[ ch ].i2c_addr;
		// CNS 3 loops, FNS all frames - 0x30
		LED_writeReg( addr, 0x02, 0x30, 0x0B );

		// Default refresh speed - TxA
		// T is typically 11ms
		// A is 1 to 64 (where 0 is 64)
		LED_writeReg( addr, 0x03, speed, 0x0B );

		// Set MODE to Auto Frame Play
		// Set FS to frame 1
		//LED_writeReg( addr, 0x00, 0x08, 0x0B );
	}
	*/

	// Load frame data
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		uint8_t addr = LED_pageBuffer[ ch ].i2c_addr;

		// Load each page with a different led enabled
		uint16_t data[] = {
			addr, 0xFD, 0x00,
		};
		while( i2c_send( data, sizeof( data ) / 2 ) == -1 )
			delay( 1 );
		data[1] = 0x24;
		data[2] = 0xFF;
		while( i2c_send( data, sizeof( data ) / 2 ) == -1 )
			delay( 1 );
	}

	// Disable Software shutdown of ISSI chip
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		uint8_t addr = LED_pageBuffer[ ch ].i2c_addr;
		LED_writeReg( addr, 0x0A, 0x01, 0x0B );
	}

/*
	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		LED_sendPage( LED_defaultBrightness[ ch ].i2c_addr, (uint16_t*)&LED_defaultBrightness[ ch ], sizeof( LED_Buffer ), 0 );
	}
*/
}

void cliFunc_ledZero( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	for ( uint8_t ch = 0; ch < ISSI_Chips_define; ch++ )
	{
		LED_zeroPages( LED_defaultBrightness[ ch ].i2c_addr, 0x0B, 1, 0x00, 0x0C ); // Control Registers
		LED_zeroPages( LED_defaultBrightness[ ch ].i2c_addr, 0x00, 8, 0x24, 0xB4 ); // Only PWMs
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
		uint8_t addr = LED_pageBuffer[ ch ].i2c_addr;
		LED_sendPage(
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

