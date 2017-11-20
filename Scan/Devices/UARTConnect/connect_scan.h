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

#pragma once

// ----- Includes -----

// Project Includes
#include <kll.h>



// ----- Enums -----

// Functions
typedef enum Command {
	CableCheck,       // Comm check

	IdRequest,        // Slave initialization (request id from master)
	IdEnumeration,    // Slave initialization (begin enumeration from master)
	IdReport,         // Slave initialization complete, report id to master

	ScanCode,         // ScanCode event status change
	Animation,        // Master trigger animation event (same command is sent back to master when ready)

	RemoteCapability, // Activate a capability on the given node
	RemoteOutput,     // Remote debug output from a given node
	RemoteInput,      // Remote command to send to a given node's debug cli

	Command_TOP,      // Enum bounds
	Command_SYN = 0x16, // Reserved for error handling
} Command;

// UART Rx/Tx Status
typedef enum UARTStatus {
	UARTStatus_Wait    = 0, // Waiting  Rx: for SYN  Tx: for current command copy to finish
	UARTStatus_SYN     = 1, // Rx: SYN Received, waiting for SOH
	UARTStatus_SOH     = 2, // Rx: SOH Received, waiting for Command
	UARTStatus_Command = 3, // Rx: Command Received, waiting for data
	UARTStatus_Ready   = 4, // Tx: Ready to send commands
} UARTStatus;



// ----- Structs -----

// UART Connect Commands

// Cable Check Command
// Called on each UART every few seconds to make sure there is a connection
// Also used to make sure there aren't any serious problems with the cable with data corruption
// This command must pass before sending any other commands on the particular UART
// Each argument is always 0xD2 (11010010)
typedef struct CableCheckCommand {
	Command command;
	uint8_t numArgs;
	uint8_t firstArg[0];
} CableCheckCommand;

// Id Request Command
// Issued by the slave device (non-master) whenever it is powered up
// Do not issue any commands until given an Id
// (Except for Cable Check and IdRequestCommand)
typedef struct IdRequestCommand {
	Command command;
} IdRequestCommand;

// Id Enumeration Command
// Issued by the master whenever an Id Request is received
typedef struct IdEnumerationCommand {
	Command command;
	uint8_t id;
} IdEnumerationCommand;

// Id Report Command
// Issued by each slave to the master when assigned an Id
typedef struct IdReportCommand {
	Command command;
	uint8_t id;
} IdReportCommand;

// Scan Code Command
// Sent from the slave to the master whenever there is a scan code state change
typedef struct ScanCodeCommand {
	Command command;
	uint8_t id;
	uint8_t numScanCodes;
	TriggerGuide firstScanCode[0];
} ScanCodeCommand;

// Animation Command
// Initiated by the master whenever an animation id should modify it's state
// Then after the leaf slave node receives the command, send it back to the master
// On the way back, each device can begin the animation adjustment
//
// The master->leaf command should indicate to each device that it should finish sending the
// current slave->master data and wait for the leaf->master command
// This allows for a tighter synchronization of animation events
typedef struct AnimationCommand {
	Command command;
	uint8_t animationId;
	uint8_t numParams;
	uint8_t firstParam[0];
} AnimationCommand;

// Remote Capability Command
// Initiated by the master to trigger a capability on a given node
// RemoteOutput is enabled while capability is activated
// Set id to 255 if command should be sent in all directions
typedef struct RemoteCapabilityCommand {
	Command command;
	uint8_t id;
	uint8_t capabilityIndex;
	uint8_t state;
	uint8_t stateType;
	uint8_t numArgs; // # of bytes, args may be larger than 1 byte
	uint8_t firstArg[0];
} RemoteCapabilityCommand;

// Remote Output Command
// Sends debug output to the master node
// Uses print command redirection to generate each command message
typedef struct RemoteOutputCommand {
	Command command;
	uint8_t id;
	uint8_t length;
	uint8_t firstChar[0];
} RemoteOutputCommand;

// Remote Input Command
// Sends debug input to given node (usually from master)
// Uses debug cli to execute command and sends all output using Remote Output Command
typedef struct RemoteInputCommand {
	Command command;
	uint8_t id;
	uint8_t length;
	uint8_t firstChar[0];
} RemoteInputCommand;



// ----- Variables -----

extern uint8_t Connect_id;
extern uint8_t Connect_master; // Set if master



// ----- Functions -----

void Connect_setup( uint8_t master, uint8_t first );
void Connect_scan();
void Connect_reset();

void Connect_send_ScanCode( uint8_t id, TriggerEvent *scanCodeStateList, uint8_t numScanCodes );
void Connect_send_RemoteCapability( uint8_t id, uint8_t capabilityIndex, uint8_t state, uint8_t stateType, uint8_t numArgs, uint8_t *args );

void Connect_currentChange( unsigned int current );

