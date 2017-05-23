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
#include <macro.h>

// Local Includes
#include "connect_scan.h"



// ----- Defines -----

#define UART_Num_Interfaces 2
#define UART_Master 1
#define UART_Slave  0
#define UART_Buffer_Size UARTConnectBufSize_define



// ----- Macros -----

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
		printHex( uart_tx_buf[ uartNum ].items ); \
		print( NL ); \
	} \
	/* XXX Doesn't work well */ \
	/* while ( UART##uartNum##_TCFIFO < fifoSize ) */ \
	/* More reliable, albeit slower */ \
	fifoSize -= UART##uartNum##_TCFIFO; \
	while ( fifoSize-- != 0 ) \
	{ \
		if ( uart_tx_buf[ uartNum ].items == 0 ) \
			break; \
		UART##uartNum##_D = uart_tx_buf[ uartNum ].buffer[ uart_tx_buf[ uartNum ].head++ ]; \
		uart_tx_buf[ uartNum ].items--; \
		if ( uart_tx_buf[ uartNum ].head >= UART_Buffer_Size ) \
			uart_tx_buf[ uartNum ].head = 0; \
	} \
}

// Macros for locking/unlock Tx buffers
#define uart_lockTx( uartNum ) \
{ \
	/* First, secure place in line for the resource */ \
	while ( uart_tx_status[ uartNum ].lock ); \
	uart_tx_status[ uartNum ].lock = 1; \
	/* Next, wait unit the UART is ready */ \
	while ( uart_tx_status[ uartNum ].status != UARTStatus_Ready ); \
	uart_tx_status[ uartNum ].status = UARTStatus_Wait; \
}

#define uart_lockBothTx( uartNum1, uartNum2 ) \
{ \
	/* First, secure place in line for the resource */ \
	while ( uart_tx_status[ uartNum1 ].lock || uart_tx_status[ uartNum2 ].lock ); \
	uart_tx_status[ uartNum1 ].lock = 1; \
	uart_tx_status[ uartNum2 ].lock = 1; \
	/* Next, wait unit the UARTs are ready */ \
	while ( uart_tx_status[ uartNum1 ].status != UARTStatus_Ready || uart_tx_status[ uartNum2 ].status != UARTStatus_Ready ); \
	uart_tx_status[ uartNum1 ].status = UARTStatus_Wait; \
	uart_tx_status[ uartNum2 ].status = UARTStatus_Wait; \
}

#define uart_unlockTx( uartNum ) \
{ \
	/* Ready the UART */ \
	uart_tx_status[ uartNum ].status = UARTStatus_Ready; \
	/* Unlock the resource */ \
	uart_tx_status[ uartNum ].lock = 0; \
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



// ----- Structs -----

typedef struct UARTRingBuf {
	uint8_t head;
	uint8_t tail;
	uint8_t items;
	uint8_t buffer[UART_Buffer_Size];
} UARTRingBuf;

typedef struct UARTDMABuf {
	uint8_t  buffer[UART_Buffer_Size];
	uint16_t last_read;
} UARTDMABuf;

typedef struct UARTStatusRx {
	UARTStatus status;
	Command    command;
	uint16_t   bytes_waiting;
} UARTStatusRx;

typedef struct UARTStatusTx {
	UARTStatus status;
	uint8_t    lock;
} UARTStatusTx;



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

volatile uint8_t uarts_configured = 0;


// -- Rx Variables --

volatile UARTDMABuf   uart_rx_buf[UART_Num_Interfaces];
volatile UARTStatusRx uart_rx_status[UART_Num_Interfaces];


// -- Tx Variables --

UARTRingBuf  uart_tx_buf   [UART_Num_Interfaces];
UARTStatusTx uart_tx_status[UART_Num_Interfaces];


// -- Ring Buffer Convenience Functions --

void Connect_addBytes( uint8_t *buffer, uint8_t count, uint8_t uart )
{
	// Too big to fit into buffer
	if ( count > UART_Buffer_Size )
	{
		erro_msg("Too big of a command to fit into the buffer...");
		return;
	}

	// Invalid UART
	if ( uart >= UART_Num_Interfaces )
	{
		erro_print("Invalid UART to send from...");
		return;
	}

	// Delay UART copy until there's some space left
	while ( uart_tx_buf[ uart ].items + count > UART_Buffer_Size )
	{
		warn_msg("Too much data to send on UART");
		printInt8( uart );
		print( ", waiting..." NL );
		delay( 1 );
		// FIXME Buffer will not drain here....
	}

	// Append data to ring buffer
	for ( uint8_t c = 0; c < count; c++ )
	{
		if ( Connect_debug )
		{
			printHex( buffer[ c ] );
			print(" +");
			printInt8( uart );
			print( NL );
		}

		uart_tx_buf[ uart ].buffer[ uart_tx_buf[ uart ].tail++ ] = buffer[ c ];
		uart_tx_buf[ uart ].items++;
		if ( uart_tx_buf[ uart ].tail >= UART_Buffer_Size )
			uart_tx_buf[ uart ].tail = 0;
		if ( uart_tx_buf[ uart ].head == uart_tx_buf[ uart ].tail )
			uart_tx_buf[ uart ].head++;
		if ( uart_tx_buf[ uart ].head >= UART_Buffer_Size )
			uart_tx_buf[ uart ].head = 0;
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

// Send a remote capability command using capability index
// This may not be what's expected (especially if the firmware is not the same on each node)
// To broadcast to all slave nodes, set id to 255 instead of a specific id
void Connect_send_RemoteCapability( uint8_t id, uint8_t capabilityIndex, uint8_t state, uint8_t stateType, uint8_t numArgs, uint8_t *args )
{
	// Prepare header
	uint8_t header[] = { 0x16, 0x01, RemoteCapability, id, capabilityIndex, state, stateType, numArgs };

	// Ignore current id
	if ( id == Connect_id )
		return;

	// Send towards slave node
	if ( id > Connect_id )
	{
		// Lock slave bound Tx
		uart_lockTx( UART_Slave );

		// Send header
		Connect_addBytes( header, sizeof( header ), UART_Slave );

		// Send arguments
		Connect_addBytes( args, numArgs, UART_Slave );

		// Unlock Tx
		uart_unlockTx( UART_Slave );
	}

	// Send towards master node
	if ( id < Connect_id || id == 255 )
	{
		// Lock slave bound Tx
		uart_lockTx( UART_Master );

		// Send header
		Connect_addBytes( header, sizeof( header ), UART_Master );

		// Send arguments
		Connect_addBytes( args, numArgs, UART_Master );

		// Unlock Tx
		uart_unlockTx( UART_Master );
	}
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
				// Lower current requirement during errors
				// USB minimum
				// Only if this is not the master node
				if ( Connect_id != 0 )
				{
					Output_update_external_current( 100 );
				}

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
				// If we already have an Id, then set max current again
				if ( Connect_id != 255 && Connect_id != 0 )
				{
					// TODO reset to original negotiated current
					Output_update_external_current( 500 );
				}
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

	// Node now enumerated, set external power to USB Max
	// Only set if this is not the master node
	// TODO Determine power slice for each node as part of protocol
	if ( Connect_id != 0 )
	{
		Output_update_external_current( 500 );
	}

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
			Macro_pressReleaseAdd( &Connect_receive_ScanCodeBuffer );
		}

		break;
	}
	// Propagate ScanCode packet
	// XXX It would be safer to buffer the scancodes first, before transmitting the packet -Jacob
	//     The current method is the more efficient/aggressive, but could cause issues if there were errors during transmission
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

// - Remote Capability Variables -
#define Connect_receive_RemoteCapabilityMaxArgs 25 // XXX Calculate the max using kll
RemoteCapabilityCommand Connect_receive_RemoteCapabilityBuffer;
uint8_t Connect_receive_RemoteCapabilityArgs[Connect_receive_RemoteCapabilityMaxArgs];

uint8_t Connect_receive_RemoteCapability( uint8_t byte, uint16_t *pending_bytes, uint8_t uart_num )
{
	// Check which byte in the packet we are at
	switch ( (*pending_bytes)-- )
	{
	// Byte count always starts at 0xFFFF
	case 0xFFFF: // Device Id
		Connect_receive_RemoteCapabilityBuffer.id = byte;
		break;

	case 0xFFFE: // Capability Index
		Connect_receive_RemoteCapabilityBuffer.capabilityIndex = byte;
		break;

	case 0xFFFD: // State
		Connect_receive_RemoteCapabilityBuffer.state = byte;
		break;

	case 0xFFFC: // StateType
		Connect_receive_RemoteCapabilityBuffer.stateType = byte;
		break;

	case 0xFFFB: // Number of args
		Connect_receive_RemoteCapabilityBuffer.numArgs = byte;
		*pending_bytes = byte;
		break;

	default:     // Args (# defined by previous byte)
		Connect_receive_RemoteCapabilityArgs[
			Connect_receive_RemoteCapabilityBuffer.numArgs - *pending_bytes + 1
		] = byte;

		// If entire packet has been fully received
		if ( *pending_bytes == 0 )
		{
			// Determine if this is the node to run the capability on
			// Conditions: Matches or broadcast (0xFF)
			if ( Connect_receive_RemoteCapabilityBuffer.id == 0xFF
				|| Connect_receive_RemoteCapabilityBuffer.id == Connect_id )
			{
				extern const Capability CapabilitiesList[]; // See generatedKeymap.h
				void (*capability)(TriggerMacro*, uint8_t, uint8_t, uint8_t*) = \
					(void(*)(TriggerMacro*, uint8_t, uint8_t, uint8_t*))(
						CapabilitiesList[
							Connect_receive_RemoteCapabilityBuffer.capabilityIndex
						].func
				);
				// TODO (HaaTa) - Send some sort of TriggerMacro information as a hint for the capability
				capability(
					0,
					Connect_receive_RemoteCapabilityBuffer.state,
					Connect_receive_RemoteCapabilityBuffer.stateType,
					&Connect_receive_RemoteCapabilityArgs[2]
				);
			}

			// If this is not the correct node, keep sending it in the same direction (doesn't matter if more nodes exist)
			// or if this is a broadcast
			if ( Connect_receive_RemoteCapabilityBuffer.id == 0xFF
				|| Connect_receive_RemoteCapabilityBuffer.id != Connect_id )
			{
				// Prepare outgoing packet
				Connect_receive_RemoteCapabilityBuffer.command = RemoteCapability;

				// Send to the other UART (not the one receiving the packet from
				uint8_t uart_direction = uart_num == UART_Master ? UART_Slave : UART_Master;

				// Lock Tx UART
				switch ( uart_direction )
				{
				case UART_Master: uart_lockTx( UART_Master ); break;
				case UART_Slave:  uart_lockTx( UART_Slave );  break;
				}

				// Send header
				uint8_t header[] = { 0x16, 0x01 };
				Connect_addBytes( header, sizeof( header ), uart_direction );

				// Send Remote Capability and arguments
				Connect_addBytes( (uint8_t*)&Connect_receive_RemoteCapabilityBuffer, sizeof( RemoteCapabilityCommand ), uart_direction );
				Connect_addBytes( Connect_receive_RemoteCapabilityArgs, Connect_receive_RemoteCapabilityBuffer.numArgs, uart_direction );

				// Unlock Tx UART
				switch ( uart_direction )
				{
				case UART_Master: uart_unlockTx( UART_Master ); break;
				case UART_Slave:  uart_unlockTx( UART_Slave );  break;
				}
			}
		}
		break;
	}

	// Check whether the scan codes have finished sending
	return *pending_bytes == 0 ? 1 : 0;
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
	Connect_receive_RemoteCapability,
};



// ----- Functions -----

// Resets the state of the UART buffers and state variables
void Connect_reset()
{
	// Reset Rx
	memset( (void*)uart_rx_status, 0, sizeof( UARTStatusRx ) * UART_Num_Interfaces );

	// Reset Tx
	memset( (void*)uart_tx_buf,    0, sizeof( UARTRingBuf )  * UART_Num_Interfaces );
	memset( (void*)uart_tx_status, 0, sizeof( UARTStatusTx ) * UART_Num_Interfaces );

	// Set Rx/Tx buffers as ready
	for ( uint8_t inter = 0; inter < UART_Num_Interfaces; inter++ )
	{
		uart_tx_status[ inter ].status = UARTStatus_Ready;
		uart_rx_buf[ inter ].last_read = UART_Buffer_Size;
	}
}


// Setup connection to other side
// - Only supports a single slave and master
// - If USB has been initiallized at this point, this side is the master
// - If both sides assert master, flash error leds
void Connect_setup( uint8_t master, uint8_t first )
{
	// Indication that UARTs are not ready
	uarts_configured = 0;

	// Register Connect CLI dictionary
	if ( first )
		CLI_registerDictionary( uartConnectCLIDict, uartConnectCLIDictName );

	// Check if master
	Connect_master = master;
	if ( Connect_master )
		Connect_id = 0; // 0x00 is always the master Id

	// UART0 setup
	// UART1 setup
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

	// Only using Tx Fifos
	UART0_PFIFO = UART_PFIFO_TXFE;
	UART1_PFIFO = UART_PFIFO_TXFE;

	// Setup DMA clocks
	SIM_SCGC6 |= SIM_SCGC6_DMAMUX;
	SIM_SCGC7 |= SIM_SCGC7_DMA;

	// Start with channels disabled first
	DMAMUX0_CHCFG0 = 0;
	DMAMUX0_CHCFG1 = 0;

	// Configure DMA channels
	//DMA_DSR_BCR0 |= DMA_DSR_BCR_DONE_MASK; // TODO What's this?
	DMA_TCD0_CSR = 0;
	DMA_TCD1_CSR = 0;

	// Default control register
	DMA_CR = 0;

	// DMA Priority
	DMA_DCHPRI0 = 0; // Ch 0, priority 0
	DMA_DCHPRI1 = 1; // ch 1, priority 1

	// Clear error interrupts
	DMA_EEI = 0;

	// Setup TCD
	DMA_TCD0_SADDR = (uint32_t*)&UART0_D;
	DMA_TCD1_SADDR = (uint32_t*)&UART1_D;
	DMA_TCD0_SOFF = 0;
	DMA_TCD1_SOFF = 0;

	// No modulo, 8-bit transfer size
	DMA_TCD0_ATTR = DMA_TCD_ATTR_SMOD(0) | DMA_TCD_ATTR_SSIZE(0) | DMA_TCD_ATTR_DMOD(0) | DMA_TCD_ATTR_DSIZE(0);
	DMA_TCD1_ATTR = DMA_TCD_ATTR_SMOD(0) | DMA_TCD_ATTR_SSIZE(0) | DMA_TCD_ATTR_DMOD(0) | DMA_TCD_ATTR_DSIZE(0);

	// One byte transferred at a time
	DMA_TCD0_NBYTES_MLNO = 1;
	DMA_TCD1_NBYTES_MLNO = 1;

	// Source address does not change
	DMA_TCD0_SLAST = 0;
	DMA_TCD1_SLAST = 0;

	// Destination buffer
	DMA_TCD0_DADDR = (uint32_t*)uart_rx_buf[0].buffer;
	DMA_TCD1_DADDR = (uint32_t*)uart_rx_buf[1].buffer;

	// Incoming byte, increment by 1 in the rx buffer
	DMA_TCD0_DOFF = 1;
	DMA_TCD1_DOFF = 1;

	// Single major loop, must be the same value
	DMA_TCD0_CITER_ELINKNO = UART_Buffer_Size;
	DMA_TCD1_CITER_ELINKNO = UART_Buffer_Size;
	DMA_TCD0_BITER_ELINKNO = UART_Buffer_Size;
	DMA_TCD1_BITER_ELINKNO = UART_Buffer_Size;

	// Reset buffer when full
	DMA_TCD0_DLASTSGA = -( UART_Buffer_Size );
	DMA_TCD1_DLASTSGA = -( UART_Buffer_Size );

	// Enable DMA channels
	DMA_ERQ |= DMA_ERQ_ERQ0 | DMA_ERQ_ERQ1;

	// Setup DMA channel routing
	DMAMUX0_CHCFG0 = DMAMUX_ENABLE | DMAMUX_SOURCE_UART0_RX;
	DMAMUX0_CHCFG1 = DMAMUX_ENABLE | DMAMUX_SOURCE_UART1_RX;

	// Enable DMA requests (requires Rx interrupts)
	UART0_C5 = UART_C5_RDMAS;
	UART1_C5 = UART_C5_RDMAS;

	// TX Enabled, RX Enabled, RX Interrupt Enabled
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


#define DMA_BUF_POS( x, pos ) \
	case x: \
		pos = DMA_TCD##x##_CITER_ELINKNO; \
		break
void Connect_rx_process( uint8_t uartNum )
{
	// Determine current position to read until
	uint16_t bufpos = 0;
	switch ( uartNum )
	{
	DMA_BUF_POS( 0, bufpos );
	DMA_BUF_POS( 1, bufpos );
	}

	// Process each of the new bytes
	// Even if we receive more bytes during processing, wait until the next check so we don't starve other tasks
	while ( bufpos != uart_rx_buf[ uartNum ].last_read )
	{
		// If the last_read byte is at the buffer edge, roll back to beginning
		if ( uart_rx_buf[ uartNum ].last_read == 0 )
		{
			uart_rx_buf[ uartNum ].last_read = UART_Buffer_Size;

			// Check to see if we're at the boundary
			if ( bufpos == UART_Buffer_Size )
				break;
		}

		// Read the byte out of Rx DMA buffer
		uint8_t byte = uart_rx_buf[ uartNum ].buffer[ UART_Buffer_Size - uart_rx_buf[ uartNum ].last_read-- ];

		if ( Connect_debug )
		{
			printHex( byte );
			print(" ");
		}

		// Process UART byte
		switch ( uart_rx_status[ uartNum ].status )
		{
		// Every packet must start with a SYN / 0x16
		case UARTStatus_Wait:
			if ( Connect_debug )
			{
				print(" Wait ");
			}
			uart_rx_status[ uartNum ].status = byte == 0x16 ? UARTStatus_SYN : UARTStatus_Wait;
			break;

		// After a SYN, there must be a SOH / 0x01
		case UARTStatus_SYN:
			if ( Connect_debug )
			{
				print(" SYN ");
			}
			uart_rx_status[ uartNum ].status = byte == 0x01 ? UARTStatus_SOH : UARTStatus_Wait;
			break;

		// After a SOH the packet structure may diverge a bit
		// This is the packet type field (refer to the Command enum)
		// For very small packets (e.g. IdRequest) this is all that's required to take action
		case UARTStatus_SOH:
		{
			if ( Connect_debug )
			{
				print(" SOH ");
			}

			// Check if this is actually a reserved CMD 0x16 (Error condition)
			if ( byte == Command_SYN )
			{
				uart_rx_status[ uartNum ].status = UARTStatus_SYN;
				break;
			}

			// Otherwise process the command
			if ( byte < Command_TOP )
			{
				uart_rx_status[ uartNum ].status = UARTStatus_Command;
				uart_rx_status[ uartNum ].command = byte;
				uart_rx_status[ uartNum ].bytes_waiting = 0xFFFF;
			}
			// Invalid packet type, ignore
			else
			{
				uart_rx_status[ uartNum ].status = UARTStatus_Wait;
			}

			// Check if this is a very short packet
			switch ( uart_rx_status[ uartNum ].command )
			{
			case IdRequest:
				Connect_receive_IdRequest( 0, (uint16_t*)&uart_rx_status[ uartNum ].bytes_waiting, uartNum );
				uart_rx_status[ uartNum ].status = UARTStatus_Wait;
				break;

			default:
				if ( Connect_debug )
				{
					print(" ### ");
					printHex( uart_rx_status[ uartNum ].command );
				}
				break;
			}
			break;
		}

		// After the packet type has been deciphered do Command specific processing
		// Until the Command has received all the bytes it requires the UART buffer stays in this state
		case UARTStatus_Command:
		{
			if ( Connect_debug )
			{
				print(" CMD ");
			}
			/* Call specific UARTConnect command receive function */
			uint8_t (*rcvFunc)(uint8_t, uint16_t(*), uint8_t) = (uint8_t(*)(uint8_t, uint16_t(*), uint8_t))(Connect_receiveFunctions[ uart_rx_status[ uartNum ].command ]);
			if ( rcvFunc( byte, (uint16_t*)&uart_rx_status[ uartNum ].bytes_waiting, uartNum ) )
				uart_rx_status[ uartNum ].status = UARTStatus_Wait;
			break;
		}

		// Unknown status, should never get here
		default:
			erro_msg("Invalid UARTStatus...");
			uart_rx_status[ uartNum ].status = UARTStatus_Wait;
			continue;
		}

		if ( Connect_debug )
		{
			print( NL );
		}
	}
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
		Connect_setup( Output_Available, 0 );
	}

	// Limit how often we do cable checks
	//uint32_t time_compare = 0x007; // XXX Used for debugging cables -HaaTa
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
		if ( uart_tx_buf[ 0 ].items > 0 && UART0_TCFIFO == 0 )
			uart_fillTxFifo( 0 );
		if ( uart_tx_buf[ 1 ].items > 0 && UART1_TCFIFO == 0 )
			uart_fillTxFifo( 1 );

		// Process Rx Buffers
		Connect_rx_process( 0 );
		Connect_rx_process( 1 );
	}
}


// Called by parent Scan module whenever the available current changes
void Connect_currentChange( unsigned int current )
{
	// TODO - Any potential power saving here?
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
	printHex( uart_rx_status[UART_Master].status );
	print( NL "\tTx:\t");
	printHex( uart_tx_status[UART_Master].status );
	print( NL "Slave <=" NL "\tStatus:\t");
	printHex( Connect_cableOkSlave );
	print( NL "\tFaults:\t");
	printHex32( Connect_cableFaultsSlave );
	print("/");
	printHex32( Connect_cableChecksSlave );
	print( NL "\tRx:\t");
	printHex( uart_rx_status[UART_Slave].status );
	print( NL "\tTx:\t");
	printHex( uart_tx_status[UART_Slave].status );
}

