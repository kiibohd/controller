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
#include <macro.h>

// Local Includes
#include "connect_scan.h"



// ----- Macros -----

#define UART_Master 1
#define UART_Slave  0
#define uart_lock_m( uartNum )         uart##uartNum##_lock
#define uart_buffer_items_m( uartNum ) uart##uartNum##_buffer_items
#define uart_buffer_m( uartNum )       uart##uartNum##_buffer
#define uart_buffer_head_m( uartNum )  uart##uartNum##_buffer_head
#define uart_buffer_tail_m( uartNum )  uart##uartNum##_buffer_tail
#define uart_tx_status_m( uartNum )    uart##uartNum##_tx_status

// Macro for adding to each uart Tx ring buffer
#define uart_addTxBuffer( uartNum ) \
case uartNum: \
	/* Delay UART copy until there's some space left */ \
	while ( uart_buffer_items_m( uartNum ) + count > uart_buffer_size ) \
	{ \
		warn_msg("Too much data to send on UART0, waiting..."); \
		delay( 1 ); \
	} \
	/* Append data to ring buffer */ \
	for ( uint8_t c = 0; c < count; c++ ) \
	{ \
		if ( Connect_debug ) \
		{ \
			printHex( buffer[ c ] ); \
			print( " +" #uartNum NL ); \
		} \
		uart_buffer_m( uartNum )[ uart_buffer_tail_m( uartNum )++ ] = buffer[ c ]; \
		uart_buffer_items_m( uartNum )++; \
		if ( uart_buffer_tail_m( uartNum ) >= uart_buffer_size ) \
			uart_buffer_tail_m( uartNum ) = 0; \
		if ( uart_buffer_head_m( uartNum ) == uart_buffer_tail_m( uartNum ) ) \
			uart_buffer_head_m( uartNum )++; \
		if ( uart_buffer_head_m( uartNum ) >= uart_buffer_size ) \
			uart_buffer_head_m( uartNum ) = 0; \
	} \
	break

// Macro for popping from Tx ring buffer
#define uart_fillTxFifo( uartNum ) \
{ \
	uint8_t fifoSize = ( ( UART##uartNum##_PFIFO & UART_PFIFO_TXFIFOSIZE ) >> 2 ); \
	if ( fifoSize == 0 ) \
		fifoSize = 1; \
	if ( Connect_debug ) \
	{ \
		print( "TxFIFO " #uartNum " - " ); \
		printHex( fifoSize ); \
		print("/"); \
		printHex( UART##uartNum##_TCFIFO ); \
		print("/"); \
		printHex( uart##uartNum##_buffer_items ); \
		print( NL ); \
	} \
	/* XXX Doesn't work well */ \
	/* while ( UART##uartNum##_TCFIFO < fifoSize ) */ \
	/* More reliable, albeit slower */ \
	fifoSize -= UART##uartNum##_TCFIFO; \
	while ( fifoSize-- != 0 ) \
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
	/* Process each byte in the UART buffer */ \
	while ( available-- > 0 ) \
	{ \
		/* First check if there was noise or Parity issues with current byte */ \
		uint8_t err_status = UART##uartNum##_ED; \
		/* Read byte from Rx FIFO */ \
		uint8_t byteRead = UART##uartNum##_D; \
		if ( Connect_debug ) \
		{ \
			printHex( byteRead ); \
			print("("); \
			printInt8( available ); \
			print(") <-"); \
		} \
		/* Check error status */ \
		if ( err_status & 0x80 ) \
		{ \
			print(" NOISY "); \
		} \
		if ( err_status & 0x40 ) \
		{ \
			print(" PARITY ERR "); \
		} \
		/* Ignore current byte if there was an error */ \
		if ( err_status ) \
		{ \
			uart##uartNum##_rx_status = UARTStatus_Wait; \
			if ( Connect_debug ) \
			{ \
				print( NL ); \
			} \
			continue; \
		} \
		switch ( uart##uartNum##_rx_status ) \
		{ \
		case UARTStatus_Wait: \
			if ( Connect_debug ) \
			{ \
				print(" Wait "); \
			} \
			uart##uartNum##_rx_status = byteRead == 0x16 ? UARTStatus_SYN : UARTStatus_Wait; \
			break; \
		case UARTStatus_SYN: \
			if ( Connect_debug ) \
			{ \
				print(" SYN "); \
			} \
			uart##uartNum##_rx_status = byteRead == 0x01 ? UARTStatus_SOH : UARTStatus_Wait; \
			break; \
		case UARTStatus_SOH: \
		{ \
			if ( Connect_debug ) \
			{ \
				print(" SOH "); \
			} \
			/* Check if this is actually a reserved CMD 0x16 */ \
			if ( byteRead == Command_SYN ) \
			{ \
				uart##uartNum##_rx_status = UARTStatus_SYN; \
				break; \
			} \
			/* Otherwise process the command */ \
			uint8_t byte = byteRead; \
			if ( byte < Command_TOP ) \
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
				if ( Connect_debug ) \
				{ \
					print(" ### "); \
					printHex( uart##uartNum##_rx_command ); \
				} \
				break; \
			} \
			break; \
		} \
		case UARTStatus_Command: \
		{ \
			if ( Connect_debug ) \
			{ \
				print(" CMD "); \
			} \
			/* Call specific UARTConnect command receive function */ \
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
		if ( Connect_debug ) \
		{ \
			print( NL ); \
		} \
	} \
}

// Macros for locking/unlock Tx buffers
#define uart_lockTx( uartNum ) \
{ \
	/* First, secure place in line for the resource */ \
	while ( uart_lock_m( uartNum ) ); \
	uart_lock_m( uartNum ) = 1; \
	/* Next, wait unit the UART is ready */ \
	while ( uart_tx_status_m( uartNum ) != UARTStatus_Ready ); \
	uart_tx_status_m( uartNum ) = UARTStatus_Wait; \
}

#define uart_lockBothTx( uartNum1, uartNum2 ) \
{ \
	/* First, secure place in line for the resource */ \
	while ( uart_lock_m( uartNum1 ) || uart_lock_m( uartNum2 ) ); \
	uart_lock_m( uartNum1 ) = 1; \
	uart_lock_m( uartNum2 ) = 1; \
	/* Next, wait unit the UARTs are ready */ \
	while ( uart_tx_status_m( uartNum1 ) != UARTStatus_Ready || uart_tx_status_m( uartNum2 ) != UARTStatus_Ready ); \
	uart_tx_status_m( uartNum1 ) = UARTStatus_Wait; \
	uart_tx_status_m( uartNum2 ) = UARTStatus_Wait; \
}

#define uart_unlockTx( uartNum ) \
{ \
	/* Ready the UART */ \
	uart_tx_status_m( uartNum ) = UARTStatus_Ready; \
	/* Unlock the resource */ \
	uart_lock_m( uartNum ) = 0; \
}



// ----- Function Declarations -----

// CLI Functions
void cliFunc_connectCmd ( char *args );
void cliFunc_connectDbg ( char *args );
void cliFunc_connectIdl ( char *args );
void cliFunc_connectLst ( char *args );
void cliFunc_connectMst ( char *args );
void cliFunc_connectRst ( char *args );
void cliFunc_connectSts ( char *args );



// ----- Variables -----

// Connect Module command dictionary
CLIDict_Entry( connectCmd,  "Sends a command via UART Connect, first arg is which uart, next arg is the command, rest are the arguments." );
CLIDict_Entry( connectDbg,  "Toggle UARTConnect debug mode." );
CLIDict_Entry( connectIdl,  "Sends N number of Idle commands, 2 is the default value, and should be sufficient in most cases." );
CLIDict_Entry( connectLst,  "Lists available UARTConnect commands and index id" );
CLIDict_Entry( connectMst,  "Sets the device as master. Use argument of s to set as slave." );
CLIDict_Entry( connectRst,  "Resets both Rx and Tx connect buffers and state variables." );
CLIDict_Entry( connectSts,  "UARTConnect status." );
CLIDict_Def( uartConnectCLIDict, "UARTConnect Module Commands" ) = {
	CLIDict_Item( connectCmd ),
	CLIDict_Item( connectDbg ),
	CLIDict_Item( connectIdl ),
	CLIDict_Item( connectLst ),
	CLIDict_Item( connectMst ),
	CLIDict_Item( connectRst ),
	CLIDict_Item( connectSts ),
	{ 0, 0, 0 } // Null entry for dictionary end
};


// -- Connect Device Id Variables --
uint8_t Connect_id = 255; // Invalid, unset
uint8_t Connect_master = 0;
uint8_t Connect_maxId = 0;


// -- Control Variables --
uint32_t Connect_lastCheck = 0; // Cable Check scheduler
uint8_t Connect_debug = 0;      // Set 1 for debug
uint8_t Connect_override = 0;   // Prevents master from automatically being set


// -- Rx Status Variables --

volatile UARTStatus uart0_rx_status;
volatile UARTStatus uart1_rx_status;
volatile uint16_t uart0_rx_bytes_waiting;
volatile uint16_t uart1_rx_bytes_waiting;
volatile Command uart0_rx_command;
volatile Command uart1_rx_command;
volatile uint8_t uart0_lock;
volatile uint8_t uart1_lock;


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
	uart_addTxBuffer( UART_Master );
	uart_addTxBuffer( UART_Slave );
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
	uart_lockBothTx( UART_Master, UART_Slave );

	// Prepare header
	uint8_t header[] = { 0x16, 0x01, CableCheck, patternLen };

	// Send header
	Connect_addBytes( header, sizeof( header ), UART_Master );
	Connect_addBytes( header, sizeof( header ), UART_Slave );

	// Send 0xD2 (11010010) for each argument
	uint8_t value = 0xD2;
	for ( uint8_t c = 0; c < patternLen; c++ )
	{
		Connect_addBytes( &value, 1, UART_Master );
		Connect_addBytes( &value, 1, UART_Slave );
	}

	// Release Tx buffers
	uart_unlockTx( UART_Master );
	uart_unlockTx( UART_Slave );
}

void Connect_send_IdRequest()
{
	// Lock master bound Tx
	uart_lockTx( UART_Master );

	// Prepare header
	uint8_t header[] = { 0x16, 0x01, IdRequest };

	// Send header
	Connect_addBytes( header, sizeof( header ), UART_Master );

	// Unlock Tx
	uart_unlockTx( UART_Master );
}

// id is the value the next slave should enumerate as
void Connect_send_IdEnumeration( uint8_t id )
{
	// Lock slave bound Tx
	uart_lockTx( UART_Slave );

	// Prepare header
	uint8_t header[] = { 0x16, 0x01, IdEnumeration, id };

	// Send header
	Connect_addBytes( header, sizeof( header ), UART_Slave );

	// Unlock Tx
	uart_unlockTx( UART_Slave );
}

// id is the currently assigned id to the slave
void Connect_send_IdReport( uint8_t id )
{
	// Lock master bound Tx
	uart_lockTx( UART_Master );

	// Prepare header
	uint8_t header[] = { 0x16, 0x01, IdReport, id };

	// Send header
	Connect_addBytes( header, sizeof( header ), UART_Master );

	// Unlock Tx
	uart_unlockTx( UART_Master );
}

// id is the currently assigned id to the slave
// scanCodeStateList is an array of [scancode, state]'s (8 bit values)
// numScanCodes is the number of scan codes to parse from array
void Connect_send_ScanCode( uint8_t id, TriggerGuide *scanCodeStateList, uint8_t numScanCodes )
{
	// Lock master bound Tx
	uart_lockTx( UART_Master );

	// Prepare header
	uint8_t header[] = { 0x16, 0x01, ScanCode, id, numScanCodes };

	// Send header
	Connect_addBytes( header, sizeof( header ), UART_Master );

	// Send each of the scan codes
	Connect_addBytes( (uint8_t*)scanCodeStateList, numScanCodes * TriggerGuideSize, UART_Master );

	// Unlock Tx
	uart_unlockTx( UART_Master );
}

// id is the currently assigned id to the slave
// paramList is an array of [param, value]'s (8 bit values)
// numParams is the number of params to parse from the array
void Connect_send_Animation( uint8_t id, uint8_t *paramList, uint8_t numParams )
{
	// Lock slave bound Tx
	uart_lockTx( UART_Slave );

	// Prepare header
	uint8_t header[] = { 0x16, 0x01, Animation, id, numParams };

	// Send header
	Connect_addBytes( header, sizeof( header ), UART_Slave );

	// Send each of the scan codes
	Connect_addBytes( paramList, numParams, UART_Slave );

	// Unlock Tx
	uart_unlockTx( UART_Slave );
}

void Connect_send_Idle( uint8_t num )
{
	// Wait until the Tx buffers are ready, then lock them
	uart_lockBothTx( UART_Slave, UART_Master );

	// Send n number of idles to reset link status (if in a bad state)
	uint8_t value = 0x16;
	for ( uint8_t c = 0; c < num; c++ )
	{
		Connect_addBytes( &value, 1, UART_Master );
		Connect_addBytes( &value, 1, UART_Slave );
	}

	// Release Tx buffers
	uart_unlockTx( UART_Master );
	uart_unlockTx( UART_Slave );
}


// -- Connect receive functions --

// - Cable Check variables -
uint32_t Connect_cableFaultsMaster = 0;
uint32_t Connect_cableFaultsSlave  = 0;
uint32_t Connect_cableChecksMaster = 0;
uint32_t Connect_cableChecksSlave  = 0;
uint8_t  Connect_cableOkMaster = 0;
uint8_t  Connect_cableOkSlave  = 0;

uint8_t Connect_receive_CableCheck( uint8_t byte, uint16_t *pending_bytes, uint8_t uart_num )
{
	// Check if this is the first byte
	if ( *pending_bytes == 0xFFFF )
	{
		*pending_bytes = byte;

		if ( Connect_debug )
		{
			dbug_msg("PENDING SET -> ");
			printHex( byte );
			print(" ");
			printHex( *pending_bytes );
			print( NL );
		}
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
			if ( uart_num == UART_Slave )
			{
				Connect_cableFaultsSlave++;
				Connect_cableOkSlave = 0;
				print(" Slave ");
			}
			else
			{
				Connect_cableFaultsMaster++;
				Connect_cableOkMaster = 0;
				print(" Master ");
			}
			printHex( byte );
			print( NL );

			// Signal that the command should wait for a SYN again
			return 1;
		}
		else
		{
			// Check which side of the chain
			if ( uart_num == UART_Slave )
			{
				Connect_cableChecksSlave++;
			}
			else
			{
				Connect_cableChecksMaster++;
			}
		}
	}

	// If cable check was successful, set cable ok
	if ( *pending_bytes == 0 )
	{
		if ( uart_num == UART_Slave )
		{
			Connect_cableOkSlave = 1;
		}
		else
		{
			Connect_cableOkMaster = 1;
		}
	}

	if ( Connect_debug )
	{
		dbug_msg("CABLECHECK RECEIVE - ");
		printHex( byte );
		print(" ");
		printHex( *pending_bytes );
		print( NL );
	}

	// Check whether the cable check has finished
	return *pending_bytes == 0 ? 1 : 0;
}

uint8_t Connect_receive_IdRequest( uint8_t byte, uint16_t *pending_bytes, uint8_t uart_num )
{
	dbug_print("IdRequest");
	// Check the directionality
	if ( uart_num == UART_Master )
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

uint8_t Connect_receive_IdEnumeration( uint8_t id, uint16_t *pending_bytes, uint8_t uart_num )
{
	dbug_print("IdEnumeration");
	// Check the directionality
	if ( uart_num == UART_Slave )
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

uint8_t Connect_receive_IdReport( uint8_t id, uint16_t *pending_bytes, uint8_t uart_num )
{
	dbug_print("IdReport");
	// Check the directionality
	if ( uart_num == UART_Master )
	{
		erro_print("Invalid IdRequest direction...");
	}

	// Track Id response if master
	if ( Connect_master )
	{
		info_msg("Id Reported: ");
		printHex( id );
		print( NL );

		// Check if this is the highest ID
		if ( id > Connect_maxId )
			Connect_maxId = id;
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

uint8_t Connect_receive_ScanCode( uint8_t byte, uint16_t *pending_bytes, uint8_t uart_num )
{
	// Check the directionality
	if ( uart_num == UART_Master )
	{
		erro_print("Invalid ScanCode direction...");
	}

	// Master node, trigger scan codes
	if ( Connect_master ) switch ( (*pending_bytes)-- )
	{
	// Byte count always starts at 0xFFFF
	case 0xFFFF: // Device Id
		Connect_receive_ScanCodeDeviceId = byte;
		break;

	case 0xFFFE: // Number of TriggerGuides in bytes (byte * 3)
		*pending_bytes = byte * sizeof( TriggerGuide );
		Connect_receive_ScanCodeBufferPos = 0;
		break;

	default:
		// Set the specific TriggerGuide entry
		((uint8_t*)&Connect_receive_ScanCodeBuffer)[ Connect_receive_ScanCodeBufferPos++ ] = byte;

		// Reset the BufferPos if higher than sizeof TriggerGuide
		// And send the TriggerGuide to the Macro Module
		if ( Connect_receive_ScanCodeBufferPos >= sizeof( TriggerGuide ) )
		{
			Connect_receive_ScanCodeBufferPos = 0;

			// Adjust ScanCode offset
			if ( Connect_receive_ScanCodeDeviceId > 0 )
			{
				// Check if this node is too large
				if ( Connect_receive_ScanCodeDeviceId >= InterconnectNodeMax )
				{
					warn_msg("Not enough interconnect layout nodes configured: ");
					printHex( Connect_receive_ScanCodeDeviceId );
					print( NL );
					break;
				}

				// This variable is in generatedKeymaps.h
				extern uint8_t InterconnectOffsetList[];
				Connect_receive_ScanCodeBuffer.scanCode = Connect_receive_ScanCodeBuffer.scanCode + InterconnectOffsetList[ Connect_receive_ScanCodeDeviceId - 1 ];
			}

			// ScanCode receive debug
			if ( Connect_debug )
			{
				dbug_msg("");
				printHex( Connect_receive_ScanCodeBuffer.type );
				print(" ");
				printHex( Connect_receive_ScanCodeBuffer.state );
				print(" ");
				printHex( Connect_receive_ScanCodeBuffer.scanCode );
				print( NL );
			}

			// Send ScanCode to macro module
			Macro_interconnectAdd( &Connect_receive_ScanCodeBuffer );
		}

		break;
	}
	// Propagate ScanCode packet
	else switch ( (*pending_bytes)-- )
	{
	// Byte count always starts at 0xFFFF
	case 0xFFFF: // Device Id
	{
		Connect_receive_ScanCodeDeviceId = byte;

		// Lock the master Tx buffer
		uart_lockTx( UART_Master );

		// Send header + Id byte
		uint8_t header[] = { 0x16, 0x01, ScanCode, byte };
		Connect_addBytes( header, sizeof( header ), UART_Master );
		break;
	}
	case 0xFFFE: // Number of TriggerGuides in bytes
		*pending_bytes = byte * sizeof( TriggerGuide );
		Connect_receive_ScanCodeBufferPos = 0;

		// Pass through byte
		Connect_addBytes( &byte, 1, UART_Master );
		break;

	default:
		// Pass through byte
		Connect_addBytes( &byte, 1, UART_Master );

		// Unlock Tx Buffer after sending last byte
		if ( *pending_bytes == 0 )
			uart_unlockTx( UART_Master );
		break;
	}

	// Check whether the scan codes have finished sending
	return *pending_bytes == 0 ? 1 : 0;
}

uint8_t Connect_receive_Animation( uint8_t byte, uint16_t *pending_bytes, uint8_t uart_num )
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
	uart0_lock = 0;
	uart1_lock = 0;

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

	// Check if master
	Connect_master = master;
	if ( Connect_master )
		Connect_id = 0; // 0x00 is always the master Id

	// Master / UART0 setup
	// Slave  / UART1 setup
	// Setup the the UART interface for keyboard data input
	SIM_SCGC4 |= SIM_SCGC4_UART0; // Disable clock gating
	SIM_SCGC4 |= SIM_SCGC4_UART1; // Disable clock gating

	// Pin Setup for UART0 / UART1
	PORTA_PCR1 = PORT_PCR_PE | PORT_PCR_PS | PORT_PCR_PFE | PORT_PCR_MUX(2); // RX Pin
	PORTA_PCR2 = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(2); // TX Pin
	PORTE_PCR0 = PORT_PCR_PE | PORT_PCR_PS | PORT_PCR_PFE | PORT_PCR_MUX(3); // RX Pin
	PORTE_PCR1 = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3); // TX Pin

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
	// Check if initially configured as a slave and usb comes up
	// Then reconfigure as a master
	if ( !Connect_master && Output_Available && !Connect_override )
	{
		Connect_setup( Output_Available );
	}

	// Limit how often we do cable checks
	uint32_t time_compare = 0x7FF; // Must be all 1's, 0x3FF is valid, 0x4FF is not
	uint32_t current_time = systick_millis_count;
	if ( Connect_lastCheck != current_time
		&& ( current_time & time_compare ) == time_compare
	)
	{
		// Make sure we don't double check if the clock speed is too high
		Connect_lastCheck = current_time;

		// Send a cable check command of 2 bytes
		Connect_send_CableCheck( UARTConnectCableCheckLength_define );

		// If this is a slave, and we don't have an id yeth
		// Don't bother sending if there are cable issues
		if ( !Connect_master && Connect_id == 0xFF && Connect_cableOkMaster )
		{
			Connect_send_IdRequest();
		}
	}

	// Only process commands if uarts have been configured
	if ( uarts_configured )
	{
		// Check if Tx Buffers are empty and the Tx Ring buffers have data to send
		// This happens if there was previously nothing to send
		if ( uart0_buffer_items > 0 && UART0_TCFIFO == 0 )
			uart_fillTxFifo( 0 );
		if ( uart1_buffer_items > 0 && UART1_TCFIFO == 0 )
			uart_fillTxFifo( 1 );
	}
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
		Connect_send_CableCheck( UARTConnectCableCheckLength_define );
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
		break;

	case RemoteCapability:
		// TODO
		break;

	case RemoteOutput:
		// TODO
		break;

	case RemoteInput:
		// TODO
		break;

	default:
		break;
	}
}

void cliFunc_connectDbg( char* args )
{
	print( NL );
	info_msg("Connect Debug Mode Toggle");
	Connect_debug = !Connect_debug;
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

void cliFunc_connectLst( char* args )
{
	const char *Command_strs[] = {
		"CableCheck",
		"IdRequest",
		"IdEnumeration",
		"IdReport",
		"ScanCode",
		"Animation",
		"RemoteCapability",
		"RemoteOutput",
		"RemoteInput",
	};

	print( NL );
	info_msg("List of UARTConnect commands");
	for ( uint8_t cmd = 0; cmd < Command_TOP; cmd++ )
	{
		print( NL );
		printInt8( cmd );
		print(" - ");
		dPrint( (char*)Command_strs[ cmd ] );
	}
}

void cliFunc_connectMst( char* args )
{
	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	print( NL );

	// Set override
	Connect_override = 1;

	switch ( arg1Ptr[0] )
	{
	// Disable override
	case 'd':
	case 'D':
		Connect_override = 0;
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

	// Reset node id
	Connect_id = 0xFF;
}

void cliFunc_connectSts( char* args )
{
	print( NL );
	info_msg("UARTConnect Status");
	print( NL "Device Type:\t" );
	print( Connect_master ? "Master" : "Slave" );
	print( NL "Device Id:\t" );
	printHex( Connect_id );
	print( NL "Max Id:\t" );
	printHex( Connect_maxId );
	print( NL "Master <=" NL "\tStatus:\t");
	printHex( Connect_cableOkMaster );
	print( NL "\tFaults:\t");
	printHex32( Connect_cableFaultsMaster );
	print("/");
	printHex32( Connect_cableChecksMaster );
	print( NL "\tRx:\t");
	printHex( uart1_rx_status );
	print( NL "\tTx:\t");
	printHex( uart1_tx_status );
	print( NL "Slave <=" NL "\tStatus:\t");
	printHex( Connect_cableOkSlave );
	print( NL "\tFaults:\t");
	printHex32( Connect_cableFaultsSlave );
	print("/");
	printHex32( Connect_cableChecksSlave );
	print( NL "\tRx:\t");
	printHex( uart0_rx_status );
	print( NL "\tTx:\t");
	printHex( uart0_tx_status );
}

