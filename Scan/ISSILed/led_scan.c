/* Copyright (C) 2014-2017 by Jacob Alexander
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

// Interconnect module if compiled in
#if defined(ConnectEnabled_define)
#include <connect_scan.h>
#endif

// KLL Include
#include <kll.h>

// Local Includes
#include "led_scan.h"



// ----- Defines -----

#define I2C_TxBufferLength 300
#define I2C_RxBufferLength 8

#define LED_BufferLength 144

// TODO Needs to be defined per keyboard
#define LED_TotalChannels 144



// ----- Structs -----

typedef struct I2C_Buffer {
	uint16_t  head;
	uint16_t  tail;
	uint8_t   sequencePos;
	uint16_t  size;
	uint8_t  *buffer;
} I2C_Buffer;

typedef struct LED_Buffer {
	uint8_t i2c_addr;
	uint8_t reg_addr;
	uint8_t buffer[LED_BufferLength];
} LED_Buffer;



// ----- Function Declarations -----

// CLI Functions
void cliFunc_i2cRecv ( char* args );
void cliFunc_i2cSend ( char* args );
void cliFunc_ledCtrl ( char* args );
void cliFunc_ledRPage( char* args );
void cliFunc_ledStart( char* args );
void cliFunc_ledTest ( char* args );
void cliFunc_ledWPage( char* args );
void cliFunc_ledZero ( char* args );

uint8_t I2C_TxBufferPop();
void I2C_BufferPush( uint8_t byte, I2C_Buffer *buffer );
uint16_t I2C_BufferLen( I2C_Buffer *buffer );
uint8_t I2C_Send( uint8_t *data, uint8_t sendLen, uint8_t recvLen );



// ----- Variables -----

// Scan Module command dictionary
CLIDict_Entry( i2cRecv,     "Send I2C sequence of bytes and expect a reply of 1 byte on the last sequence." NL "\t\tUse |'s to split sequences with a stop." );
CLIDict_Entry( i2cSend,     "Send I2C sequence of bytes. Use |'s to split sequences with a stop." );
CLIDict_Entry( ledCtrl,     "Basic LED control. Args: <mode> <amount> [<index>]" );
CLIDict_Entry( ledRPage,    "Read the given register page." );
CLIDict_Entry( ledStart,    "Disable software shutdown." );
CLIDict_Entry( ledTest,     "Test out the led pages." );
CLIDict_Entry( ledWPage,    "Write to given register page starting at address. i.e. 0x2 0x24 0xF0 0x12" );
CLIDict_Entry( ledZero,     "Zero out LED register pages (non-configuration)." );

CLIDict_Def( ledCLIDict, "ISSI LED Module Commands" ) = {
	CLIDict_Item( i2cRecv ),
	CLIDict_Item( i2cSend ),
	CLIDict_Item( ledCtrl ),
	CLIDict_Item( ledRPage ),
	CLIDict_Item( ledStart ),
	CLIDict_Item( ledTest ),
	CLIDict_Item( ledWPage ),
	CLIDict_Item( ledZero ),
	{ 0, 0, 0 } // Null entry for dictionary end
};



// Before sending the sequence, I2C_TxBuffer_CurLen is assigned and as each byte is sent, it is decremented
// Once I2C_TxBuffer_CurLen reaches zero, a STOP on the I2C bus is sent
volatile uint8_t I2C_TxBufferPtr[ I2C_TxBufferLength ];
volatile uint8_t I2C_RxBufferPtr[ I2C_TxBufferLength ];

volatile I2C_Buffer I2C_TxBuffer = { 0, 0, 0, I2C_TxBufferLength, (uint8_t*)I2C_TxBufferPtr };
volatile I2C_Buffer I2C_RxBuffer = { 0, 0, 0, I2C_RxBufferLength, (uint8_t*)I2C_RxBufferPtr };

LED_Buffer LED_pageBuffer;

// A bit mask determining which LEDs are enabled in the ISSI chip
const uint8_t LED_ledEnableMask1[] = {
	0xE8, // I2C address
	0x00, // Starting register address
	ISSILedMask1_define
};

// Default LED brightness
const uint8_t LED_defaultBrightness1[] = {
	0xE8, // I2C address
	0x24, // Starting register address
	ISSILedBrightness1_define
};



// ----- Interrupt Functions -----

void i2c0_isr()
{
	cli(); // Disable Interrupts

	uint8_t status = I2C0_S; // Read I2C Bus status

	// Master Mode Transmit
	if ( I2C0_C1 & I2C_C1_TX )
	{
		// Check current use of the I2C bus
		// Currently sending data
		if ( I2C_TxBuffer.sequencePos > 0 )
		{
			// Make sure slave sent an ACK
			if ( status & I2C_S_RXAK )
			{
				// NACK Detected, disable interrupt
				erro_print("I2C NAK detected...");
				I2C0_C1 = I2C_C1_IICEN;

				// Abort Tx Buffer
				I2C_TxBuffer.head = 0;
				I2C_TxBuffer.tail = 0;
				I2C_TxBuffer.sequencePos = 0;
			}
			else
			{
				// Transmit byte
				I2C0_D = I2C_TxBufferPop();
			}
		}
		// Receiving data
		else if ( I2C_RxBuffer.sequencePos > 0 )
		{
			// Master Receive, addr sent
			if ( status & I2C_S_ARBL )
			{
				// Arbitration Lost
				erro_print("Arbitration lost...");
				// TODO Abort Rx

				I2C0_C1 = I2C_C1_IICEN;
				I2C0_S = I2C_S_ARBL | I2C_S_IICIF; // Clear ARBL flag and interrupt
			}
			if ( status & I2C_S_RXAK )
			{
				// Slave Address NACK Detected, disable interrupt
				erro_print("Slave Address I2C NAK detected...");
				// TODO Abort Rx

				I2C0_C1 = I2C_C1_IICEN;
			}
			else
			{
				dbug_msg("Attempting to read byte - ");
				printHex( I2C_RxBuffer.sequencePos );
				print( NL );
				I2C0_C1 = I2C_RxBuffer.sequencePos == 1
					? I2C_C1_IICEN | I2C_C1_IICIE | I2C_C1_MST | I2C_C1_TXAK // Single byte read
					: I2C_C1_IICEN | I2C_C1_IICIE | I2C_C1_MST; // Multi-byte read
			}
		}
		else
		{
			/*
			dbug_msg("STOP - ");
			printHex( I2C_BufferLen( (I2C_Buffer*)&I2C_TxBuffer ) );
			print(NL);
			*/

			// Delay around STOP to make sure it actually happens...
			delayMicroseconds( 1 );
			I2C0_C1 = I2C_C1_IICEN; // Send STOP
			delayMicroseconds( 7 );

			// If there is another sequence, start sending
			if ( I2C_BufferLen( (I2C_Buffer*)&I2C_TxBuffer ) < I2C_TxBuffer.size )
			{
				// Clear status flags
				I2C0_S = I2C_S_IICIF | I2C_S_ARBL;

				// Wait...till the master dies
				while ( I2C0_S & I2C_S_BUSY );

				// Enable I2C interrupt
				I2C0_C1 = I2C_C1_IICEN | I2C_C1_IICIE | I2C_C1_MST | I2C_C1_TX;

				// Transmit byte
				I2C0_D = I2C_TxBufferPop();
			}
		}
	}
	// Master Mode Receive
	else
	{
		// XXX Do we need to handle 2nd last byte?
		//I2C0_C1 = I2C_C1_IICEN | I2C_C1_IICIE | I2C_C1_MST | I2C_C1_TXAK; // No STOP, Rx, NAK on recv

		// Last byte
		if ( I2C_TxBuffer.sequencePos <= 1 )
		{
			// Change to Tx mode
			I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_TX;

			// Grab last byte
			I2C_BufferPush( I2C0_D, (I2C_Buffer*)&I2C_RxBuffer );

			delayMicroseconds( 1 ); // Should be enough time before issuing the stop
			I2C0_C1 = I2C_C1_IICEN; // Send STOP
		}
		else
		{
			// Retrieve data
			I2C_BufferPush( I2C0_D, (I2C_Buffer*)&I2C_RxBuffer );
		}
	}

	I2C0_S = I2C_S_IICIF; // Clear interrupt

	sei(); // Re-enable Interrupts
}



// ----- Functions -----

inline void I2C_setup()
{
	// Enable I2C internal clock
	SIM_SCGC4 |= SIM_SCGC4_I2C0; // Bus 0

	// External pull-up resistor
	PORTB_PCR0 = PORT_PCR_ODE | PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(2);
	PORTB_PCR1 = PORT_PCR_ODE | PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(2);

	// SCL Frequency Divider
	// 400kHz -> 120 (0x85) @ 48 MHz F_BUS
	I2C0_F = 0x85;
	I2C0_FLT = 4;
	I2C0_C1 = I2C_C1_IICEN;
	I2C0_C2 = I2C_C2_HDRS; // High drive select

	// Enable I2C Interrupt
	NVIC_ENABLE_IRQ( IRQ_I2C0 );
}

void LED_zeroPages( uint8_t startPage, uint8_t numPages, uint8_t startReg, uint8_t endReg )
{
	// Page Setup
	uint8_t pageSetup[] = { 0xE8, 0xFD, 0x00 };

	// Max length of a page + chip id + reg start
	uint8_t fullPage[ 0xB4 + 2 ] = { 0 }; // Max size of page
	fullPage[0] = 0xE8;     // Set chip id
	fullPage[1] = startReg; // Set start reg

	// Iterate through given pages, zero'ing out the given register regions
	for ( uint8_t page = startPage; page < startPage + numPages; page++ )
	{
		// Set page
		pageSetup[2] = page;

		// Setup page
		while ( I2C_Send( pageSetup, sizeof( pageSetup ), 0 ) == 0 )
			delay(1);

		// Zero out page
		while ( I2C_Send( fullPage, endReg - startReg + 2, 0 ) == 0 )
			delay(1);
	}
}

void LED_sendPage( uint8_t *buffer, uint8_t len, uint8_t page )
{
	// Page Setup
	uint8_t pageSetup[] = { 0xE8, 0xFD, page };

	// Setup page
	while ( I2C_Send( pageSetup, sizeof( pageSetup ), 0 ) == 0 )
		delay(1);

	// Write page to I2C Tx Buffer
	while ( I2C_Send( buffer, len, 0 ) == 0 )
		delay(1);

}

void LED_writeReg( uint8_t reg, uint8_t val, uint8_t page )
{
	// Page Setup
	uint8_t pageSetup[] = { 0xE8, 0xFD, page };

	// Reg Write Setup
	uint8_t writeData[] = { 0xE8, reg, val };

	// Setup page
	while ( I2C_Send( pageSetup, sizeof( pageSetup ), 0 ) == 0 )
		delay(1);

	while ( I2C_Send( writeData, sizeof( writeData ), 0 ) == 0 )
		delay(1);
}

void LED_readPage( uint8_t len, uint8_t page )
{
	// Software shutdown must be enabled to read registers
	LED_writeReg( 0x0A, 0x00, 0x0B );

	// Page Setup
	uint8_t pageSetup[] = { 0xE8, 0xFD, page };

	// Setup page
	while ( I2C_Send( pageSetup, sizeof( pageSetup ), 0 ) == 0 )
		delay(1);

	// Register Setup
	uint8_t regSetup[] = { 0xE8, 0x00 };

	// Read each register in the page
	for ( uint8_t reg = 0; reg < len; reg++ )
	{
		// Update register to read
		regSetup[1] = reg;

		// Configure register
		while ( I2C_Send( regSetup, sizeof( regSetup ), 0 ) == 0 )
			delay(1);

		// Register Read Command
		uint8_t regReadCmd[] = { 0xE9 };

		// Request single register byte
		while ( I2C_Send( regReadCmd, sizeof( regReadCmd ), 1 ) == 0 )
			delay(1);
		dbug_print("NEXT");
	}

	// Disable software shutdown
	LED_writeReg( 0x0A, 0x01, 0x0B );
}

// Setup
inline void LED_setup()
{
	// Register Scan CLI dictionary
	CLI_registerDictionary( ledCLIDict, ledCLIDictName );

	// Initialize I2C
	I2C_setup();

	// Zero out Frame Registers
	// This needs to be done before disabling the hardware shutdown (or the leds will do undefined things)
	LED_zeroPages( 0x0B, 1, 0x00, 0x0C ); // Control Registers

	// Disable Hardware shutdown of ISSI chip (pull high)
	GPIOB_PDDR |= (1<<16);
	PORTB_PCR16 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOB_PSOR |= (1<<16);

	// Clear LED Pages
	LED_zeroPages( 0x00, 8, 0x00, 0xB4 ); // LED Registers

	// Enable LEDs based upon mask
	LED_sendPage( (uint8_t*)LED_ledEnableMask1, sizeof( LED_ledEnableMask1 ), 0 );

	// Set default brightness
	LED_sendPage( (uint8_t*)LED_defaultBrightness1, sizeof( LED_defaultBrightness1 ), 0 );

	// Do not disable software shutdown of ISSI chip unless current is high enough
	// Require at least 150 mA
	// May be enabled/disabled at a later time
	if ( Output_current_available() >= 150 )
	{
		// Disable Software shutdown of ISSI chip
		LED_writeReg( 0x0A, 0x01, 0x0B );
	}
}


inline uint8_t I2C_BufferCopy( uint8_t *data, uint8_t sendLen, uint8_t recvLen, I2C_Buffer *buffer )
{
	uint8_t reTurn = 0;

	// If sendLen is greater than buffer fail right away
	if ( sendLen > buffer->size )
		return 0;

	// Calculate new tail to determine if buffer has enough space
	// The first element specifies the expected number of bytes from the slave (+1)
	// The second element in the new buffer is the length of the buffer sequence (+1)
	uint16_t newTail = buffer->tail + sendLen + 2;
	if ( newTail >= buffer->size )
		newTail -= buffer->size;

	if ( I2C_BufferLen( buffer ) < sendLen + 2 )
		return 0;

/*
	print("|");
	printHex( sendLen + 2 );
	print("|");
	printHex( *tail );
	print("@");
	printHex( newTail );
	print("@");
*/

	// If buffer is clean, return 1, otherwise 2
	reTurn = buffer->head == buffer->tail ? 1 : 2;

	// Add to buffer, already know there is enough room (simplifies adding logic)
	uint8_t bufferHeaderPos = 0;
	for ( uint16_t c = 0; c < sendLen; c++ )
	{
		// Add data to buffer
		switch ( bufferHeaderPos )
		{
		case 0:
			buffer->buffer[ buffer->tail ] = recvLen;
			bufferHeaderPos++;
			c--;
			break;

		case 1:
			buffer->buffer[ buffer->tail ] = sendLen;
			bufferHeaderPos++;
			c--;
			break;

		default:
			buffer->buffer[ buffer->tail ] = data[ c ];
			break;
		}

		// Check for wrap-around case
		if ( buffer->tail + 1 >= buffer->size )
		{
			buffer->tail = 0;
		}
		// Normal case
		else
		{
			buffer->tail++;
		}
	}

	return reTurn;
}


inline uint16_t I2C_BufferLen( I2C_Buffer *buffer )
{
	// Tail >= Head
	if ( buffer->tail >= buffer->head )
		return buffer->head + buffer->size - buffer->tail;

	// Head > Tail
	return buffer->head - buffer->tail;
}


void I2C_BufferPush( uint8_t byte, I2C_Buffer *buffer )
{
	dbug_msg("DATA: ");
	printHex( byte );

	// Make sure buffer isn't full
	if ( buffer->tail + 1 == buffer->head || ( buffer->head > buffer->tail && buffer->tail + 1 - buffer->size == buffer->head ) )
	{
		warn_msg("I2C_BufferPush failed, buffer full: ");
		printHex( byte );
		print( NL );
		return;
	}

	// Check for wrap-around case
	if ( buffer->tail + 1 >= buffer->size )
	{
		buffer->tail = 0;
	}
	// Normal case
	else
	{
		buffer->tail++;
	}

	// Add byte to buffer
	buffer->buffer[ buffer->tail ] = byte;
}


uint8_t I2C_TxBufferPop()
{
	// Return 0xFF if no buffer left (do not rely on this)
	if ( I2C_BufferLen( (I2C_Buffer*)&I2C_TxBuffer ) >= I2C_TxBuffer.size )
	{
		erro_msg("No buffer to pop an entry from... ");
		printHex( I2C_TxBuffer.head );
		print(" ");
		printHex( I2C_TxBuffer.tail );
		print(" ");
		printHex( I2C_TxBuffer.sequencePos );
		print(NL);
		return 0xFF;
	}

	// If there is currently no sequence being sent, the first entry in the RingBuffer is the length
	if ( I2C_TxBuffer.sequencePos == 0 )
	{
		I2C_TxBuffer.sequencePos = 0xFF; // So this doesn't become an infinite loop
		I2C_RxBuffer.sequencePos = I2C_TxBufferPop();
		I2C_TxBuffer.sequencePos = I2C_TxBufferPop();
	}

	uint8_t data = I2C_TxBuffer.buffer[ I2C_TxBuffer.head ];

	// Prune head
	I2C_TxBuffer.head++;

	// Wrap-around case
	if ( I2C_TxBuffer.head >= I2C_TxBuffer.size )
		I2C_TxBuffer.head = 0;

	// Decrement buffer sequence (until next stop will be sent)
	I2C_TxBuffer.sequencePos--;

	/*
	dbug_msg("Popping: ");
	printHex( data );
	print(" ");
	printHex( I2C_TxBuffer.head );
	print(" ");
	printHex( I2C_TxBuffer.tail );
	print(" ");
	printHex( I2C_TxBuffer.sequencePos );
	print(NL);
	*/
	return data;
}


uint8_t I2C_Send( uint8_t *data, uint8_t sendLen, uint8_t recvLen )
{
	// Check head and tail pointers
	// If full, return 0
	// If empty, start up I2C Master Tx
	// If buffer is non-empty and non-full, just append to the buffer
	switch ( I2C_BufferCopy( data, sendLen, recvLen, (I2C_Buffer*)&I2C_TxBuffer ) )
	{
	// Not enough buffer space...
	case 0:
		/*
		erro_msg("Not enough Tx buffer space... ");
		printHex( I2C_TxBuffer.head );
		print(":");
		printHex( I2C_TxBuffer.tail );
		print("+");
		printHex( sendLen );
		print("|");
		printHex( I2C_TxBuffer.size );
		print( NL );
		*/
		return 0;

	// Empty buffer, initialize I2C
	case 1:
		// Clear status flags
		I2C0_S = I2C_S_IICIF | I2C_S_ARBL;

		// Check to see if we already have control of the bus
		if ( I2C0_C1 & I2C_C1_MST )
		{
			// Already the master (ah yeah), send a repeated start
			I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_RSTA | I2C_C1_TX;
		}
		// Otherwise, seize control
		else
		{
			// Wait...till the master dies
			while ( I2C0_S & I2C_S_BUSY );

			// Now we're the master (ah yisss), get ready to send stuffs
			I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_TX;
		}

		// Enable I2C interrupt
		I2C0_C1 = I2C_C1_IICEN | I2C_C1_IICIE | I2C_C1_MST | I2C_C1_TX;

		// Depending on what type of transfer, the first byte is configured for R or W
		I2C0_D = I2C_TxBufferPop();

		return 1;
	}

	// Dirty buffer, I2C already initialized
	return 2;
}



// LED State processing loop
unsigned int LED_currentEvent = 0;
inline uint8_t LED_scan()
{
	// Check for current change event
	if ( LED_currentEvent )
	{
		// TODO dim LEDs in low power mode instead of shutting off
		if ( LED_currentEvent < 150 )
		{
			// Enable Software shutdown of ISSI chip
			LED_writeReg( 0x0A, 0x00, 0x0B );
		}
		else
		{
			// Disable Software shutdown of ISSI chip
			LED_writeReg( 0x0A, 0x01, 0x0B );
		}

		LED_currentEvent = 0;
	}

	return 0;
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
	// Only send if we've completed all other transactions
	/*
	if ( I2C_TxBuffer.sequencePos > 0 )
		return;
	*/

	// Configure based upon the given mode
	// TODO Perhaps do gamma adjustment?
	switch ( control->mode )
	{
	case LedControlMode_brightness_decrease:
		// Don't worry about rolling over, the cycle is quick
		LED_pageBuffer.buffer[ control->index ] -= control->amount;
		break;

	case LedControlMode_brightness_increase:
		// Don't worry about rolling over, the cycle is quick
		LED_pageBuffer.buffer[ control->index ] += control->amount;
		break;

	case LedControlMode_brightness_set:
		LED_pageBuffer.buffer[ control->index ] = control->amount;
		break;

	case LedControlMode_brightness_decrease_all:
		for ( uint8_t channel = 0; channel < LED_TotalChannels; channel++ )
		{
			// Don't worry about rolling over, the cycle is quick
			LED_pageBuffer.buffer[ channel ] -= control->amount;
		}
		break;

	case LedControlMode_brightness_increase_all:
		for ( uint8_t channel = 0; channel < LED_TotalChannels; channel++ )
		{
			// Don't worry about rolling over, the cycle is quick
			LED_pageBuffer.buffer[ channel ] += control->amount;
		}
		break;

	case LedControlMode_brightness_set_all:
		for ( uint8_t channel = 0; channel < LED_TotalChannels; channel++ )
		{
			LED_pageBuffer.buffer[ channel ] = control->amount;
		}
		break;
	}

	// Sync LED buffer with ISSI chip buffer
	// TODO Support multiple frames
	LED_pageBuffer.i2c_addr = 0xE8; // Chip 1
	LED_pageBuffer.reg_addr = 0x24; // Brightness section
	LED_sendPage( (uint8_t*)&LED_pageBuffer, sizeof( LED_Buffer ), 0 );
}

uint8_t LED_control_timer = 0;
void LED_control_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
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
	uint8_t buffer[ i2cSend_BuffLenMax ];
	uint8_t bufferLen = 0;

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
			I2C_Send( buffer, bufferLen, 0 );
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

	I2C_Send( buffer, bufferLen, 0 );
}

void cliFunc_i2cRecv( char* args )
{
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Buffer used after interpretting the args, will be sent to I2C functions
	// NOTE: Limited to 8 bytes currently (can be increased if necessary
	#define i2cSend_BuffLenMax 8
	uint8_t buffer[ i2cSend_BuffLenMax ];
	uint8_t bufferLen = 0;

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
			I2C_Send( buffer, bufferLen, 0 );
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

	I2C_Send( buffer, bufferLen, 1 ); // Only 1 byte is ever read at a time with the ISSI chip
}

// TODO Currently not working correctly
void cliFunc_ledRPage( char* args )
{
	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	// Default to 0 if no argument is given
	uint8_t page = 0;

	if ( arg1Ptr[0] != '\0' )
	{
		page = (uint8_t)numToInt( arg1Ptr );
	}

	// No \r\n by default after the command is entered
	print( NL );

	LED_readPage( 0x1, page );
	//LED_readPage( 0xB4, page );
}

void cliFunc_ledWPage( char* args )
{
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// First process page and starting address
	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Stop processing args if no more are found
	if ( *arg1Ptr == '\0' )
		return;
	uint8_t page[] = { 0xE8, 0xFD, numToInt( arg1Ptr ) };

	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Stop processing args if no more are found
	if ( *arg1Ptr == '\0' )
		return;
	uint8_t data[] = { 0xE8, numToInt( arg1Ptr ), 0 };

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

void cliFunc_ledStart( char* args )
{
	print( NL ); // No \r\n by default after the command is entered
	LED_zeroPages( 0x0B, 1, 0x00, 0x0C ); // Control Registers
	//LED_zeroPages( 0x00, 8, 0x00, 0xB4 ); // LED Registers
	LED_writeReg( 0x0A, 0x01, 0x0B );
	LED_sendPage( (uint8_t*)LED_ledEnableMask1, sizeof( LED_ledEnableMask1 ), 0 );

}

void cliFunc_ledTest( char* args )
{
	print( NL ); // No \r\n by default after the command is entered
	LED_sendPage( (uint8_t*)LED_defaultBrightness1, sizeof( LED_defaultBrightness1 ), 0 );
}

void cliFunc_ledZero( char* args )
{
	print( NL ); // No \r\n by default after the command is entered
	LED_zeroPages( 0x00, 8, 0x24, 0xB4 ); // Only PWMs
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

