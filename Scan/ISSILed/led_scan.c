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
#include <led.h>
#include <print.h>

// Local Includes
#include "led_scan.h"



// ----- Defines -----

#define I2C_TxBufferLength 300
#define I2C_RxBufferLength 8

#define LED_BufferLength 144


// ----- Structs -----

typedef struct I2C_Buffer {
	uint16_t  head;
	uint16_t  tail;
	uint8_t   sequencePos;
	uint16_t  size;
	uint8_t  *buffer;
} I2C_Buffer;

typedef struct LED_Buffer {
	uint8_t buffer[LED_BufferLength];
} LED_Buffer;



// ----- Function Declarations -----

// CLI Functions
void cliFunc_echo( char* args );
void cliFunc_i2cRecv( char* args );
void cliFunc_i2cSend( char* args );
void cliFunc_ledTest( char* args );
void cliFunc_ledZero( char* args );

uint8_t I2C_TxBufferPop();
void I2C_BufferPush( uint8_t byte, I2C_Buffer *buffer );
uint16_t I2C_BufferLen( I2C_Buffer *buffer );
uint8_t I2C_Send( uint8_t *data, uint8_t sendLen, uint8_t recvLen );



// ----- Variables -----

// Scan Module command dictionary
CLIDict_Entry( i2cRecv,     "Send I2C sequence of bytes and expect a reply of 1 byte on the last sequence." NL "\t\tUse |'s to split sequences with a stop." );
CLIDict_Entry( i2cSend,     "Send I2C sequence of bytes. Use |'s to split sequences with a stop." );
CLIDict_Entry( ledTest,     "Test out the led pages." );
CLIDict_Entry( ledZero,     "Zero out LED register pages (non-configuration)." );

CLIDict_Def( ledCLIDict, "ISSI LED Module Commands" ) = {
	CLIDict_Item( i2cRecv ),
	CLIDict_Item( i2cSend ),
	CLIDict_Item( ledTest ),
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
// 0x00 -> 0x11
const uint8_t LED_ledEnableMask[] = {
0xE8, // I2C address
0x00, // Starting register address
0xFF, 0xFF, // C1-1 -> C1-16
0xFF, 0xFF, // C2-1 -> C2-16
0xFF, 0xFF, // C3-1 -> C3-16
0xFF, 0xFF, // C4-1 -> C4-16
0xFF, 0xFF, // C5-1 -> C5-16
0xFF, 0xFF, // C6-1 -> C6-16
0xFF, 0xFF, // C7-1 -> C7-16
0xFF, 0xFF, // C8-1 -> C8-16
0xFF, 0xFF, // C9-1 -> C9-16
};

// XXX Pre-fill example of buffers
const uint8_t examplePage[] = {
0xE8, // I2C address
0x24, // Starting register address
0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, // C1-1 -> C1-16
0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, // C2-1 -> C2-16
0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, // C3-1 -> C3-16
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, // C4-1 -> C4-16
0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, // C5-1 -> C5-16
0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, // C6-1 -> C6-16
0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, // C7-1 -> C7-16
0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, // C8-1 -> C8-16
0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, // C9-1 -> C9-16
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
				dbug_print("Attempting to read byte");
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
//},

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
	GPIOD_PDDR |= (1<<1);
	PORTD_PCR1 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOD_PSOR |= (1<<1);

	// Clear LED Pages
	LED_zeroPages( 0x00, 8, 0x00, 0xB4 ); // LED Registers

	// Enable LEDs based upon mask
	LED_sendPage( (uint8_t*)LED_ledEnableMask, sizeof( LED_ledEnableMask ), 0 );

	// Disable Software shutdown of ISSI chip
	LED_writeReg( 0x0A, 0x01, 0x0B );
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
inline uint8_t LED_scan()
{

	// I2C Busy
	// S & I2C_S_BUSY
	//I2C_S_BUSY

	return 0;
}



// ----- CLI Command Functions -----

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

void cliFunc_ledTest( char* args )
{
	print( NL ); // No \r\n by default after the command is entered
	LED_sendPage( (uint8_t*)examplePage, sizeof( examplePage ), 0 );
}

void cliFunc_ledZero( char* args )
{
	print( NL ); // No \r\n by default after the command is entered
	LED_zeroPages( 0x00, 8, 0x24, 0xB4 ); // Only PWMs
}

