/* Copyright (C) 2014-2017 by Jacob Alexander
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// ----- Includes -----

// Compiler Includes
#include <Lib/OutputLib.h>

// Project Includes
#include <cli.h>
#include <led.h>
#include <print.h>
#include <scan_loop.h>

// USB Includes
#if defined(_avr_at_)
#elif defined(_kinetis_)
#include "arm/uart_serial.h"
#endif

// KLL Include
#include <kll.h>

// Local Includes
#include "output_com.h"



// ----- Function Declarations -----

// ----- Variables -----

// USBKeys Keyboard Buffer
USBKeys USBKeys_primary; // Primary send buffer
USBKeys USBKeys_idle;    // Idle timeout send buffer

// The number of keys sent to the usb in the array
	uint8_t  USBKeys_Sent    = 0;

// 1=num lock, 2=caps lock, 4=scroll lock, 8=compose, 16=kana
volatile uint8_t  USBKeys_LEDs = 0;
volatile uint8_t  USBKeys_LEDs_Changed;

// Protocol setting from the host.
// 0 - Boot Mode
// 1 - NKRO Mode (Default, unless set by a BIOS or boot interface)
volatile uint8_t  USBKeys_Protocol = 0;

// Indicate if USB should send update
// OS only needs update if there has been a change in state
USBKeyChangeState USBKeys_Changed = USBKeyChangeState_None;

// the idle configuration, how often we send the report to the
// host (ms * 4) even when it hasn't changed
	uint8_t  USBKeys_Idle_Config = 125;

// count until idle timeout
	uint8_t  USBKeys_Idle_Count = 0;

// Indicates whether the Output module is fully functional
// 0 - Not fully functional, 1 - Fully functional
// 0 is often used to show that a USB cable is not plugged in (but has power)
volatile uint8_t  Output_Available = 0;



// ----- Capabilities -----

// Ignored capabilities
void Output_kbdProtocolBoot_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args ) {}
void Output_kbdProtocolNKRO_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args ) {}
void Output_toggleKbdProtocol_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args ) {}
void Output_consCtrlSend_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args ) {}
void Output_noneSend_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args ) {}
void Output_sysCtrlSend_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args ) {}
void Output_usbCodeSend_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args ) {}
void Output_usbMouse_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args ) {}

void Output_flashMode_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Output_flashMode(usbCode)");
		return;
	}

	// Start flash mode
	Output_firmwareReload();
}



// ----- Functions -----

// UART Module Setup
inline void Output_setup()
{
	// Setup UART
	uart_serial_setup();
}


// UART Data Poll
inline void Output_poll()
{
}


// UART Data Periodic
inline void Output_periodic()
{
}


// UART Data Ready
uint8_t Output_ready()
{
	return 1;
}


// Sets the device into firmware reload mode
inline void Output_firmwareReload()
{
	uart_device_reload();
}


// UART Input buffer available
inline unsigned int Output_availablechar()
{
	return uart_serial_available();
}


// UART Get Character from input buffer
inline int Output_getchar()
{
	// XXX Make sure to check output_availablechar() first! Information is lost with the cast (error codes) (AVR)
	return (int)uart_serial_getchar();
}


// UART Send Character to output buffer
inline int Output_putchar( char c )
{
	return uart_serial_putchar( c );
}


// UART Send String to output buffer, null terminated
inline int Output_putstr( char* str )
{
#if defined(_avr_at_) // AVR
	uint16_t count = 0;
#elif defined(_kinetis_) // ARM
	uint32_t count = 0;
#endif
	// Count characters until NULL character, then send the amount counted
	while ( str[count] != '\0' )
		count++;

	return uart_serial_write( str, count );
}


// Soft Chip Reset
inline void Output_softReset()
{
#if defined(_avr_at_) // AVR
#elif defined(_kinetis_) // ARM
	SOFTWARE_RESET();
#endif
}


// ----- CLI Command Functions -----

