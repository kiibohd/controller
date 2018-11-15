/* Copyright (C) 2015-2018 by Jacob Alexander
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

// ----- Defines -----

// Compile-modes
// See capabilities.kll for details
#define None         0
#define USBSwap      1
#define USBInterSwap 2
#define USBHubSwap   3



// ----- Includes -----

// Compiler Includes
#include <Lib/ScanLib.h>

// Project Includes
#include <cli.h>
#include <kll.h>
#include <kll_defs.h>
#include <latency.h>
#include <led.h>
#include <print.h>
#include <Lib/gpio.h>

// USB Includes
#if defined(_avr_at_)
#include <avr/usb_keyboard_serial.h>
#elif defined(_kinetis_) || defined(_sam_)
#include <arm/usb_dev.h>
#endif

// Interconnect Includes
#if Port_SwapMode_define == USBInterSwap
#include <connect_scan.h>
#endif

// Local Includes
#include "port_scan.h"



// ----- Structs -----

// ----- Function Declarations -----

// CLI Functions
void cliFunc_portCross( char* args );
void cliFunc_portUART ( char* args );
void cliFunc_portUSB  ( char* args );



// ----- Variables -----

uint32_t Port_lastcheck_ms;

// GPIOs used for swapping
#if Port_SwapMode_define == USBSwap || Port_SwapMode_define == USBInterSwap
static const GPIO_Pin usb_swap_pin1 = Port_SwapUSBPin1_define;
#endif
#if Port_SwapMode_define == USBHubSwap
static const GPIO_Pin usb_swap_pin2 = Port_SwapUSBPin2_define;
static const GPIO_Pin usb_swap_pin3 = Port_SwapUSBPin3_define;
static const GPIO_Pin usb_swap_pin4 = Port_SwapUSBPin4_define;
#endif

#if Port_SwapMode_define == USBInterSwap
static const GPIO_Pin uart_swap_pin1 = Port_SwapInterpPin1_define;;
#endif

#if Port_SwapMode_define == USBInterSwap
static const GPIO_Pin uart_cross_pin1 = Port_CrossInterPin1_define;
#endif

// Scan Module command dictionary
CLIDict_Entry( portCross, "Cross interconnect pins." );
CLIDict_Entry( portUSB,   "Swap USB ports manually, forces usb and interconnect to re-negotiate if necessary." );
CLIDict_Entry( portUART,  "Swap interconnect ports." );

CLIDict_Def( portCLIDict, "Port Swap Module Commands" ) = {
	CLIDict_Item( portCross ),
	CLIDict_Item( portUART ),
	CLIDict_Item( portUSB ),
	{ 0, 0, 0 } // Null entry for dictionary end
};

// Latency measurement resource
static uint8_t portLatencyResource;



// ----- Functions -----

void Port_usb_swap()
{
#if Port_SwapMode_define == USBSwap || Port_SwapMode_define == USBInterSwap
	info_print("USB Port Swap");

	// USB Swap
	GPIO_Ctrl( usb_swap_pin1, GPIO_Type_DriveToggle, GPIO_Config_None );

	// Re-initialize usb
	// Call usb_configured() to check if usb is ready
	usb_init();
#else
	warn_print("Unsupported");
#endif
}

void Port_uart_swap()
{
#if Port_SwapMode_define == USBInterSwap
	info_print("Interconnect Line Swap");

	// UART Swap
	GPIO_Ctrl( uart_swap_pin1, GPIO_Type_DriveToggle, GPIO_Config_None );
#else
	warn_print("Unsupported");
#endif
}

void Port_cross()
{
#if Port_SwapMode_define == USBInterSwap
	info_print("Interconnect Line Cross");

	// UART Tx/Rx cross-over
	GPIO_Ctrl( uart_cross_pin1, GPIO_Type_DriveToggle, GPIO_Config_None );

	// Reset interconnects
	Connect_reset();
#else
	warn_print("Unsupported");
#endif
}

// Setup
inline void Port_setup()
{
	// Register Scan CLI dictionary
	CLI_registerDictionary( portCLIDict, portCLIDictName );

#if Port_SwapMode_define == USBSwap
	// USB Swap
	// Start, disabled
	GPIO_Ctrl( usb_swap_pin1, GPIO_Type_DriveSetup, GPIO_Config_None );
	GPIO_Ctrl( usb_swap_pin1, GPIO_Type_DriveLow, GPIO_Config_None );
#elif Port_SwapMode_define == USBInterSwap
	// USB Swap
	// Start, disabled
	GPIO_Ctrl( usb_swap_pin1, GPIO_Type_DriveSetup, GPIO_Config_None );
	GPIO_Ctrl( usb_swap_pin1, GPIO_Type_DriveLow, GPIO_Config_None );

	// UART Tx/Rx cross-over
	// Start, disabled
	GPIO_Ctrl( uart_cross_pin1, GPIO_Type_DriveSetup, GPIO_Config_None );
	GPIO_Ctrl( uart_cross_pin1, GPIO_Type_DriveLow, GPIO_Config_None );

	// UART Swap
	// Start, disabled
	GPIO_Ctrl( uart_swap_pin1, GPIO_Type_DriveSetup, GPIO_Config_None );
	GPIO_Ctrl( uart_swap_pin1, GPIO_Type_DriveLow, GPIO_Config_None );
#else
	warn_print("Unsupported");
#endif

	// Starting point for automatic port swapping
	Port_lastcheck_ms = systick_millis_count;

	// Allocate latency measurement resource
	portLatencyResource = Latency_add_resource("PortSwap", LatencyOption_Ticks);
}

// Port State processing loop
inline uint8_t Port_scan()
{
	// Latency measurement start
	Latency_start_time( portLatencyResource );

	// TODO Add in interconnect line cross

	#define USBPortSwapDelay_ms 1000
	// Wait 1000 ms before checking
	// Only check for swapping after delay
	uint32_t wait_ms = systick_millis_count - Port_lastcheck_ms;
	if ( wait_ms > USBPortSwapDelay_ms )
	{
		// Update timeout
		Port_lastcheck_ms = systick_millis_count;

		// USB not initialized, attempt to swap
		if ( !usb_configured() )
		{
			Port_usb_swap();
		}
	}

	// Latency measurement end
	Latency_end_time( portLatencyResource );

	return 0;
}



// ----- Capabilities -----

void Port_uart_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Last:
		// Only use capability on release
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Port_uart_capability()");
		return;
	default:
		return;
	}

	Port_uart_swap();
}

void Port_usb_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Last:
		// Only use capability on release
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Port_usb_capability()");
		return;
	default:
		return;
	}

	Port_usb_swap();
}

void Port_cross_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Last:
		// Only use capability on release
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Port_cross_capability()");
		return;
	default:
		return;
	}

	Port_cross();
}



// ----- CLI Command Functions -----

void cliFunc_portUART( char* args )
{
	print( NL );
	Port_uart_swap();
}

void cliFunc_portUSB( char* args )
{
	print( NL );
	Port_usb_swap();
}

void cliFunc_portCross( char* args )
{
	print( NL );
	Port_cross();
}

