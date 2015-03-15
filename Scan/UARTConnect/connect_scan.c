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
#include <macro.h>

// Local Includes
#include "connect_scan.h"



// ----- Macros -----

// Macro for adding to each uart Tx ring buffer
#define uart_addTxBuffer( uartNum ) \
case uartNum: \
	while ( uart##uartNum##_buffer_items + count > uart_buffer_size ) \
	{ \
		warn_msg("Too much data to send on UART0, waiting..."); \
		delay( 1 ); \
	} \
	for ( uint8_t c = 0; c < count; c++ ) \
	{ \
		printHex( buffer[ c ] ); \
		print( " +" #uartNum NL ); \
		uart##uartNum##_buffer[ uart##uartNum##_buffer_tail++ ] = buffer[ c ]; \
		uart##uartNum##_buffer_items++; \
		if ( uart##uartNum##_buffer_tail >= uart_buffer_size ) \
			uart##uartNum##_buffer_tail = 0; \
		if ( uart##uartNum##_buffer_head == uart##uartNum##_buffer_tail ) \
			uart##uartNum##_buffer_head++; \
		if ( uart##uartNum##_buffer_head >= uart_buffer_size ) \
			uart##uartNum##_buffer_head = 0; \
	} \
	break

// Macro for popping from Tx ring buffer
#define uart_fillTxFifo( uartNum ) \
{ \
	uint8_t fifoSize = ( ( UART##uartNum##_PFIFO & UART_PFIFO_TXFIFOSIZE ) >> 2 ); \
	if ( fifoSize == 0 ) \
		fifoSize = 1; \
	while ( UART##uartNum##_TCFIFO < fifoSize ) \
	{ \
		if ( uart##uartNum##_buffer_items == 0 ) \
			break; \
		UART##uartNum##_D = uart##uartNum##_buffer[ uart##uartNum##_buffer_head++ ]; \
		uart##uartNum##_buffer_items--; \
		if ( uart##uartNum##_buffer_head >= uart_buffer_size ) \
			uart##uartNum##_buffer_head = 0; \
	} \
}

// Macro for processing UART Rx
#define uart_processRx( uartNum ) \
{ \
	if ( !( UART##uartNum##_S1 & UART_S1_RDRF ) ) \
		return; \
	uint8_t available = UART##uartNum##_RCFIFO; \
	if ( available == 0 ) \
	{ \
		available = UART##uartNum##_D; \
		UART##uartNum##_CFIFO = UART_CFIFO_RXFLUSH; \
		return; \
	} \
	while ( available-- > 0 ) \
	{ \
		uint8_t byteRead = UART##uartNum##_D; \
		printHex( byteRead ); \
		print( "(" ); \
		printInt8( available ); \
		print( ") <-" ); \
		switch ( uart##uartNum##_rx_status ) \
		{ \
		case UARTStatus_Wait: \
			print(" SYN "); \
			uart##uartNum##_rx_status = byteRead == 0x16 ? UARTStatus_SYN : UARTStatus_Wait; \
			break; \
		case UARTStatus_SYN: \
			print(" SOH "); \
			uart##uartNum##_rx_status = byteRead == 0x01 ? UARTStatus_SOH : UARTStatus_Wait; \
			break; \
		case UARTStatus_SOH: \
		{ \
			print(" CMD "); \
			uint8_t byte = byteRead; \
			if ( byte <= Animation ) \
			{ \
				uart##uartNum##_rx_status = UARTStatus_Command; \
				uart##uartNum##_rx_command = byte; \
				uart##uartNum##_rx_bytes_waiting = 0xFFFF; \
			} \
			else \
			{ \
				uart##uartNum##_rx_status = UARTStatus_Wait; \
			} \
			switch ( uart##uartNum##_rx_command ) \
			{ \
			case IdRequest: \
				Connect_receive_IdRequest( 0, (uint16_t*)&uart##uartNum##_rx_bytes_waiting, uartNum ); \
				uart##uartNum##_rx_status = UARTStatus_Wait; \
				break; \
			default: \
				print("###"); \
				break; \
			} \
			break; \
		} \
		case UARTStatus_Command: \
		{ \
			print(" CMD "); \
			uint8_t (*rcvFunc)(uint8_t, uint16_t(*), uint8_t) = (uint8_t(*)(uint8_t, uint16_t(*), uint8_t))(Connect_receiveFunctions[ uart##uartNum##_rx_command ]); \
			if ( rcvFunc( byteRead, (uint16_t*)&uart##uartNum##_rx_bytes_waiting, uartNum ) ) \
				uart##uartNum##_rx_status = UARTStatus_Wait; \
			break; \
		} \
		default: \
			erro_msg("Invalid UARTStatus..."); \
			uart##uartNum##_rx_status = UARTStatus_Wait; \
			available++; \
			continue; \
		} \
		print( NL ); \
	} \
}

// Macros for locking/unlock Tx buffers
#define uart_lockTx( uartNum ) \
{ \
	while ( uart##uartNum##_tx_status == UARTStatus_Wait ); \
	uart##uartNum##_tx_status = UARTStatus_Wait; \
}

#define uart_unlockTx( uartNum ) \
{ \
	uart##uartNum##_tx_status = UARTStatus_Ready; \
}



// ----- Function Declarations -----

// CLI Functions
void cliFunc_connectCmd ( char *args );
void cliFunc_connectIdl ( char *args );
void cliFunc_connectMst ( char *args );
void cliFunc_connectRst ( char *args );
void cliFunc_connectSts ( char *args );



// ----- Variables -----

// Connect Module command dictionary
CLIDict_Entry( connectCmd,  "Sends a command via UART Connect, first arg is which uart, next arg is the command, rest are the arguments." );
CLIDict_Entry( connectIdl,  "Sends N number of Idle commands, 2 is the default value, and should be sufficient in most cases." );
CLIDict_Entry( connectMst,  "Sets the device as master. Use argument of s to set as slave." );
CLIDict_Entry( connectRst,  "Resets both Rx and Tx connect buffers and state variables." );
CLIDict_Entry( connectSts,  "UARTConnect status." );
CLIDict_Def( uartConnectCLIDict, "UARTConnect Module Commands" ) = {
	CLIDict_Item( connectCmd ),
	CLIDict_Item( connectIdl ),
	CLIDict_Item( connectMst ),
	CLIDict_Item( connectRst ),
	CLIDict_Item( connectSts ),
	{ 0, 0, 0 } // Null entry for dictionary end
};


// -- Connect Device Id Variables --
uint8_t Connect_id = 255; // Invalid, unset
uint8_t Connect_master = 0;


// -- Rx Status Variables --

volatile UARTStatus uart0_rx_status;
volatile UARTStatus uart1_rx_status;
volatile uint16_t uart0_rx_bytes_waiting;
volatile uint16_t uart1_rx_bytes_waiting;
volatile Command uart0_rx_command;
volatile Command uart1_rx_command;


// -- Tx Status Variables --

volatile UARTStatus uart0_tx_status;
volatile UARTStatus uart1_tx_status;


// -- Ring Buffer Variables --

#define uart_buffer_size UARTConnectBufSize_define
volatile uint8_t uart0_buffer_head;
volatile uint8_t uart0_buffer_tail;
volatile uint8_t uart0_buffer_items;
volatile uint8_t uart0_buffer[uart_buffer_size];
volatile uint8_t uart1_buffer_head;
volatile uint8_t uart1_buffer_tail;
volatile uint8_t uart1_buffer_items;
volatile uint8_t uart1_buffer[uart_buffer_size];

volatile uint8_t uarts_configured = 0;


// -- Ring Buffer Convenience Functions --

void Connect_addBytes( uint8_t *buffer, uint8_t count, uint8_t uart )
{
	// Too big to fit into buffer
	if ( count > uart_buffer_size )
	{
		erro_msg("Too big of a command to fit into the buffer...");
		return;
	}

	// Choose the uart
	switch ( uart )
	{
	uart_addTxBuffer( 0 );
	uart_addTxBuffer( 1 );
	default:
		erro_msg("Invalid UART to send from...");
		break;
	}
}


// -- Connect send functions --

// patternLen defines how many bytes should the incrementing pattern have
void Connect_send_CableCheck( uint8_t patternLen )
{
	// Wait until the Tx buffers are ready, then lock them
	uart_lockTx( 0 );
	uart_lockTx( 1 );

	// Prepare header
	uint8_t header[] = { 0x16, 0x01, CableCheck, patternLen };

	// Send header
	Connect_addBytes( header, sizeof( header ), 1 ); // Master
	Connect_addBytes( header, sizeof( header ), 0 ); // Slave

	// Send 0xD2 (11010010) for each argument
	uint8_t value = 0xD2;
	for ( uint8_t c = 0; c < patternLen; c++ )
	{
		Connect_addBytes( &value, 1, 1 ); // Master
		Connect_addBytes( &value, 1, 0 ); // Slave
	}

	// Release Tx buffers
	uart_unlockTx( 0 );
	uart_unlockTx( 1 );
}

void Connect_send_IdRequest()
{
	// Lock master bound Tx
	uart_lockTx( 1 );

	// Prepare header
	uint8_t header[] = { 0x16, 0x01, IdRequest };

	// Send header
	Connect_addBytes( header, sizeof( header ), 1 ); // Master

	// Unlock Tx
	uart_unlockTx( 1 );
}

// id is the value the next slave should enumerate as
void Connect_send_IdEnumeration( uint8_t id )
{
	// Lock slave bound Tx
	uart_lockTx( 0 );

	// Prepare header
	uint8_t header[] = { 0x16, 0x01, IdEnumeration, id };

	// Send header
	Connect_addBytes( header, sizeof( header ), 0 ); // Slave

	// Unlock Tx
	uart_unlockTx( 0 );
}

// id is the currently assigned id to the slave
void Connect_send_IdReport( uint8_t id )
{
	// Lock master bound Tx
	uart_lockTx( 1 );

	// Prepare header
	uint8_t header[] = { 0x16, 0x01, IdReport, id };

	// Send header
	Connect_addBytes( header, sizeof( header ), 1 ); // Master

	// Unlock Tx
	uart_unlockTx( 1 );
}

// id is the currently assigned id to the slave
// scanCodeStateList is an array of [scancode, state]'s (8 bit values)
// numScanCodes is the number of scan codes to parse from array
void Connect_send_ScanCode( uint8_t id, TriggerGuide *scanCodeStateList, uint8_t numScanCodes )
{
	// Lock master bound Tx
	uart_lockTx( 1 );

	// Prepare header
	uint8_t header[] = { 0x16, 0x01, ScanCode, id, numScanCodes };

	// Send header
	Connect_addBytes( header, sizeof( header ), 1 ); // Master

	// Send each of the scan codes
	Connect_addBytes( (uint8_t*)scanCodeStateList, numScanCodes * TriggerGuideSize, 1 ); // Master

	// Unlock Tx
	uart_unlockTx( 1 );
}

// id is the currently assigned id to the slave
// paramList is an array of [param, value]'s (8 bit values)
// numParams is the number of params to parse from the array
void Connect_send_Animation( uint8_t id, uint8_t *paramList, uint8_t numParams )
{
	// Lock slave bound Tx
	uart_lockTx( 0 );

	// Prepare header
	uint8_t header[] = { 0x16, 0x01, Animation, id, numParams };

	// Send header
	Connect_addBytes( header, sizeof( header ), 0 ); // Slave

	// Send each of the scan codes
	Connect_addBytes( paramList, numParams, 0 ); // Slave

	// Unlock Tx
	uart_unlockTx( 0 );
}

void Connect_send_Idle( uint8_t num )
{
	// Wait until the Tx buffers are ready, then lock them
	uart_lockTx( 0 );
	uart_lockTx( 1 );

	// Send n number of idles to reset link status (if in a bad state)
	uint8_t value = 0x16;
	for ( uint8_t c = 0; c < num; c++ )
	{
		Connect_addBytes( &value, 1, 1 ); // Master
		Connect_addBytes( &value, 1, 0 ); // Slave
	}

	// Release Tx buffers
	uart_unlockTx( 0 );
	uart_unlockTx( 1 );
}


// -- Connect receive functions --

// - Cable Check variables -
uint32_t Connect_cableFaultsMaster = 0;
uint32_t Connect_cableFaultsSlave = 0;
uint8_t  Connect_cableOkMaster = 0;
uint8_t  Connect_cableOkSlave = 0;

uint8_t Connect_receive_CableCheck( uint8_t byte, uint16_t *pending_bytes, uint8_t to_master )
{
	// Check if this is the first byte
	if ( *pending_bytes == 0xFFFF )
	{
		dbug_msg("PENDING SET -> ");
		printHex( byte );
		print(" ");
		*pending_bytes = byte;
		printHex( *pending_bytes );
		print( NL );
	}
	// Verify byte
	else
	{
		(*pending_bytes)--;

		// The argument bytes are always 0xD2 (11010010)
		if ( byte != 0xD2 )
		{
			warn_print("Cable Fault!");

			// Check which side of the chain
			if ( to_master )
			{
				Connect_cableFaultsMaster++;
				Connect_cableOkMaster = 0;
				print(" Master ");
			}
			else
			{
				Connect_cableFaultsSlave++;
				Connect_cableOkSlave = 0;
				print(" Slave ");
			}
			printHex( byte );
			print( NL );

			// Signal that the command should wait for a SYN again
			return 1;
		}
	}

	// If cable check was successful, set cable ok
	if ( *pending_bytes == 0 )
	{
		if ( to_master )
		{
			Connect_cableOkMaster = 1;
		}
		else
		{
			Connect_cableOkSlave = 1;
		}
	}
	dbug_msg("CABLECHECK RECEIVE - ");
	printHex( byte );
	print(" ");
	printHex( *pending_bytes );
	print(NL);

	// Check whether the cable check has finished
	return *pending_bytes == 0 ? 1 : 0;
}

uint8_t Connect_receive_IdRequest( uint8_t byte, uint16_t *pending_bytes, uint8_t to_master )
{
	dbug_print("IdRequest");
	// Check the directionality
	if ( !to_master )
	{
		erro_print("Invalid IdRequest direction...");
	}

	// Check if master, begin IdEnumeration
	if ( Connect_master )
	{
		// The first device is always id 1
		// Id 0 is reserved for the master
		Connect_send_IdEnumeration( 1 );
	}
	// Propagate IdRequest
	else
	{
		Connect_send_IdRequest();
	}

	return 1;
}

uint8_t Connect_receive_IdEnumeration( uint8_t id, uint16_t *pending_bytes, uint8_t to_master )
{
	dbug_print("IdEnumeration");
	// Check the directionality
	if ( to_master )
	{
		erro_print("Invalid IdEnumeration direction...");
	}

	// Set the device id
	Connect_id = id;

	// Send reponse back to master
	Connect_send_IdReport( id );

	// Propogate next Id if the connection is ok
	if ( Connect_cableOkSlave )
	{
		Connect_send_IdEnumeration( id + 1 );
	}

	return 1;
}

uint8_t Connect_receive_IdReport( uint8_t id, uint16_t *pending_bytes, uint8_t to_master )
{
	dbug_print("IdReport");
	// Check the directionality
	if ( !to_master )
	{
		erro_print("Invalid IdRequest direction...");
	}

	// Track Id response if master
	if ( Connect_master )
	{
		// TODO, setup id's
		info_msg("Id Reported: ");
		printHex( id );
		print( NL );
		return 1;
	}
	// Propagate id if yet another slave
	else
	{
		Connect_send_IdReport( id );
	}

	return 1;
}

// - Scan Code Variables -
TriggerGuide Connect_receive_ScanCodeBuffer;
uint8_t Connect_receive_ScanCodeBufferPos;
uint8_t Connect_receive_ScanCodeDeviceId;

uint8_t Connect_receive_ScanCode( uint8_t byte, uint16_t *pending_bytes, uint8_t to_master )
{
	dbug_print("ScanCode");
	// Check the directionality
	if ( !to_master )
	{
		erro_print("Invalid ScanCode direction...");
	}

	// Master node, trigger scan codes
	if ( Connect_master ) switch ( (*pending_bytes)-- )
	{
	case 0xFFFF: // Device Id
		Connect_receive_ScanCodeDeviceId = byte;
		break;

	case 0xFFFE: // Number of TriggerGuides in bytes (byte * 3)
		*pending_bytes = byte * 3;
		Connect_receive_ScanCodeBufferPos = 0;
		break;

	default:
		// Set the specific TriggerGuide entry
		((uint8_t*)&Connect_receive_ScanCodeBuffer)[ Connect_receive_ScanCodeBufferPos++ ] = byte;

		// Reset the BufferPos if higher than 3
		// And send the TriggerGuide to the Macro Module
		if ( Connect_receive_ScanCodeBufferPos > 3 )
		{
			Connect_receive_ScanCodeBufferPos = 0;
			Macro_triggerState( &Connect_receive_ScanCodeBuffer, 1 );
		}

		break;
	}
	// Propagate ScanCode packet
	else switch ( (*pending_bytes)-- )
	{
	case 0xFFFF: // Device Id
	{
		Connect_receive_ScanCodeDeviceId = byte;

		// Lock the master Tx buffer
		uart_lockTx( 1 );

		// Send header + Id byte
		uint8_t header[] = { 0x16, 0x01, ScanCode, byte };
		Connect_addBytes( header, sizeof( header ), 1 ); // Master
		break;
	}
	case 0xFFFE: // Number of TriggerGuides in bytes (byte * 3)
		*pending_bytes = byte * 3;
		Connect_receive_ScanCodeBufferPos = 0;

		// Pass through byte
		Connect_addBytes( &byte, 1, 1 ); // Master
		break;

	default:
		// Pass through byte
		Connect_addBytes( &byte, 1, 1 ); // Master

		// Unlock Tx Buffer after sending last byte
		if ( *pending_bytes == 0 )
			uart_unlockTx( 1 );
		break;
	}

	// Check whether the scan codes have finished sending
	return *pending_bytes == 0 ? 1 : 0;
}

uint8_t Connect_receive_Animation( uint8_t byte, uint16_t *pending_bytes, uint8_t to_master )
{
	dbug_print("Animation");
	return 1;
}


// Baud Rate
// NOTE: If finer baud adjustment is needed see UARTx_C4 -> BRFA in the datasheet
uint16_t Connect_baud = UARTConnectBaud_define; // Max setting of 8191
uint16_t Connect_baudFine = UARTConnectBaudFine_define;

// Connect receive function lookup
void *Connect_receiveFunctions[] = {
	Connect_receive_CableCheck,
	Connect_receive_IdRequest,
	Connect_receive_IdEnumeration,
	Connect_receive_IdReport,
	Connect_receive_ScanCode,
	Connect_receive_Animation,
};



// ----- Interrupt Functions -----

// Master / UART0 ISR
void uart0_status_isr()
{
	// Process Rx buffer
	uart_processRx( 0 );
}

// Slave / UART1 ISR
void uart1_status_isr()
{
	// Process Rx buffer
	uart_processRx( 1 );
}



// ----- Functions -----

// Resets the state of the UART buffers and state variables
void Connect_reset()
{
	// Rx Status Variables
	uart0_rx_status = UARTStatus_Wait;
	uart1_rx_status = UARTStatus_Wait;
	uart0_rx_bytes_waiting = 0;
	uart1_rx_bytes_waiting = 0;

	// Tx Status Variables
	uart0_tx_status = UARTStatus_Ready;
	uart1_tx_status = UARTStatus_Ready;

	// Ring Buffer Variables
	uart0_buffer_head = 0;
	uart0_buffer_tail = 0;
	uart0_buffer_items = 0;
	uart1_buffer_head = 0;
	uart1_buffer_tail = 0;
	uart1_buffer_items = 0;
}


// Setup connection to other side
// - Only supports a single slave and master
// - If USB has been initiallized at this point, this side is the master
// - If both sides assert master, flash error leds
void Connect_setup( uint8_t master )
{
	// Indication that UARTs are not ready
	uarts_configured = 0;

	// Register Connect CLI dictionary
	CLI_registerDictionary( uartConnectCLIDict, uartConnectCLIDictName );

	Connect_master = master;

	// Master / UART0 setup
	// Slave  / UART1 setup
	// Setup the the UART interface for keyboard data input
	SIM_SCGC4 |= SIM_SCGC4_UART0; // Disable clock gating
	SIM_SCGC4 |= SIM_SCGC4_UART1; // Disable clock gating

	// Pin Setup for UART0 / UART1
	// XXX TODO Set to actual (Teensy 3.1s don't have the correct pins available)
	PORTB_PCR16 = PORT_PCR_PE | PORT_PCR_PS | PORT_PCR_PFE | PORT_PCR_MUX(3); // RX Pin
	PORTB_PCR17 = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3); // TX Pin
	PORTC_PCR3  = PORT_PCR_PE | PORT_PCR_PS | PORT_PCR_PFE | PORT_PCR_MUX(3); // RX Pin
	PORTC_PCR4  = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3); // TX Pin
	//PORTA_PCR1 = PORT_PCR_PE | PORT_PCR_PS | PORT_PCR_PFE | PORT_PCR_MUX(2); // RX Pin
	//PORTA_PCR2 = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(2); // TX Pin
	//PORTE_PCR0 = PORT_PCR_PE | PORT_PCR_PS | PORT_PCR_PFE | PORT_PCR_MUX(3); // RX Pin
	//PORTE_PCR1 = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3); // TX Pin

	// Baud Rate setting
	UART0_BDH = (uint8_t)(Connect_baud >> 8);
	UART0_BDL = (uint8_t)Connect_baud;
	UART0_C4  = Connect_baudFine;
	UART1_BDH = (uint8_t)(Connect_baud >> 8);
	UART1_BDL = (uint8_t)Connect_baud;
	UART1_C4  = Connect_baudFine;

	// 8 bit, Even Parity, Idle Character bit after stop
	// NOTE: For 8 bit with Parity you must enable 9 bit transmission (pg. 1065)
	//       You only need to use UART0_D for 8 bit reading/writing though
	// UART_C1_M UART_C1_PE UART_C1_PT UART_C1_ILT
	UART0_C1 = UART_C1_M | UART_C1_PE | UART_C1_ILT;
	UART1_C1 = UART_C1_M | UART_C1_PE | UART_C1_ILT;

	// Number of bytes in FIFO before TX Interrupt
	// TODO Set 0
	UART0_TWFIFO = 1;
	UART1_TWFIFO = 1;

	// Number of bytes in FIFO before RX Interrupt
	UART0_RWFIFO = 1;
	UART1_RWFIFO = 1;

	// Enable TX and RX FIFOs
	UART0_PFIFO = UART_PFIFO_TXFE | UART_PFIFO_RXFE;
	UART1_PFIFO = UART_PFIFO_TXFE | UART_PFIFO_RXFE;

	// Reciever Inversion Disabled, LSBF
	// UART_S2_RXINV UART_S2_MSBF
	UART0_S2 |= 0x00;
	UART1_S2 |= 0x00;

	// Transmit Inversion Disabled
	// UART_C3_TXINV
	UART0_C3 |= 0x00;
	UART1_C3 |= 0x00;

	// TX Enabled, RX Enabled, RX Interrupt Enabled
	// UART_C2_TE UART_C2_RE UART_C2_RIE
	UART0_C2 = UART_C2_TE | UART_C2_RE | UART_C2_RIE;
	UART1_C2 = UART_C2_TE | UART_C2_RE | UART_C2_RIE;

	// Add interrupts to the vector table
	NVIC_ENABLE_IRQ( IRQ_UART0_STATUS );
	NVIC_ENABLE_IRQ( IRQ_UART1_STATUS );

	// UARTs are now ready to go
	uarts_configured = 1;

	// Reset the state of the UART variables
	Connect_reset();
}


// Scan for updates in the master/slave
// - Interrupts will deal with most input functions
// - Used to send queries
// - SyncEvent is sent immediately once the current command is sent
// - SyncEvent is also blocking until sent
void Connect_scan()
{
	// Check if Tx Buffers are empty and the Tx Ring buffers have data to send
	// This happens if there was previously nothing to send
	if ( uart0_buffer_items > 0 && UART0_TCFIFO == 0 )
		uart_fillTxFifo( 0 );
	if ( uart1_buffer_items > 0 && UART1_TCFIFO == 0 )
		uart_fillTxFifo( 1 );
}



// ----- CLI Command Functions -----

void cliFunc_connectCmd( char* args )
{
	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	print( NL );

	switch ( numToInt( &arg1Ptr[0] ) )
	{
	case CableCheck:
		Connect_send_CableCheck( 2 );
		break;

	case IdRequest:
		Connect_send_IdRequest();
		break;

	case IdEnumeration:
		Connect_send_IdEnumeration( 5 );
		break;

	case IdReport:
		Connect_send_IdReport( 8 );
		break;

	case ScanCode:
	{
		TriggerGuide scanCodes[] = { { 0x00, 0x01, 0x05 }, { 0x00, 0x03, 0x16 } };
		Connect_send_ScanCode( 10, scanCodes, 2 );
		break;
	}
	case Animation:
	default:
		break;
	}
}

void cliFunc_connectIdl( char* args )
{
	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	print( NL );
	info_msg("Sending Sync Idles...");

	uint8_t count = numToInt( &arg1Ptr[0] );
	// Default to 2 idles
	if ( count == 0 )
		count = 2;

	Connect_send_Idle( count );
}

void cliFunc_connectMst( char* args )
{
	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	print( NL );

	switch ( arg1Ptr[0] )
	{
	case 's':
	case 'S':
		info_msg("Setting device as slave.");
		Connect_master = 0;
		Connect_id = 0xFF;
		break;

	case 'm':
	case 'M':
	default:
		info_msg("Setting device as master.");
		Connect_master = 1;
		Connect_id = 0;
		break;
	}
}

void cliFunc_connectRst( char* args )
{
	print( NL );
	info_msg("Resetting UARTConnect state...");
	Connect_reset();

	// TODO - Argument for re-sync
}

void cliFunc_connectSts( char* args )
{
	print( NL );
	info_msg("UARTConnect Status");
	print( NL "Device Type:\t" );
	print( Connect_master ? "Master" : "Slave" );
	print( NL "Device Id:\t" );
	printHex( Connect_id );
	print( NL "Master <=" NL "\tStatus:\t");
	printHex( Connect_cableOkMaster );
	print( NL "\tFaults:\t");
	printHex( Connect_cableFaultsMaster );
	print( NL "\tRx:\t");
	printHex( uart1_rx_status );
	print( NL "\tTx:\t");
	printHex( uart1_tx_status );
	print( NL "Slave <=" NL "\tStatus:\t");
	printHex( Connect_cableOkSlave );
	print( NL "\tFaults:\t");
	printHex( Connect_cableFaultsSlave );
	print( NL "\tRx:\t");
	printHex( uart0_rx_status );
	print( NL "\tTx:\t");
	printHex( uart0_tx_status );
}

