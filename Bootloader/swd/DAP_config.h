/**
 * @file    DAP_config.h
 * @brief
 *
 * DAPLink Interface Firmware
 * Copyright (c) 2009-2016, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 * Modifications by Jacob Alexander 2020
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __DAP_CONFIG_H__
#define __DAP_CONFIG_H__

//**************************************************************************************************
/**
\defgroup DAP_Config_Debug_gr CMSIS-DAP Debug Unit Information
\ingroup DAP_ConfigIO_gr
@{
Provides definitions about:
 - Definition of Cortex-M processor parameters used in CMSIS-DAP Debug Unit.
 - Debug Unit communication packet size.
 - Debug Access Port communication mode (JTAG or SWD).
 - Optional information about a connected Target Device (for Evaluation Boards).
*/

#include "IO_Config.h"
#include <sam4s.h>
#include <Lib/gpio.h>
#include <debug.h>

/// Processor Clock of the Cortex-M MCU used in the Debug Unit.
/// This value is used to calculate the SWD/JTAG clock speed.
#define CPU_CLOCK               120000000        ///< Specifies the CPU Clock in Hz

/// Number of processor cycles for I/O Port write operations.
/// This value is used to calculate the SWD/JTAG clock speed that is generated with I/O
/// Port write operations in the Debug Unit by a Cortex-M MCU. Most Cortex-M processors
/// requrie 2 processor cycles for a I/O Port Write operation.  If the Debug Unit uses
/// a Cortex-M0+ processor with high-speed peripheral I/O only 1 processor cycle might be
/// requrired.
#define IO_PORT_WRITE_CYCLES    2               ///< I/O Cycles: 2=default, 1=Cortex-M0+ fast I/0

/// Default communication speed on the Debug Access Port for SWD and JTAG mode.
/// Used to initialize the default SWD/JTAG clock frequency.
/// The command \ref DAP_SWJ_Clock can be used to overwrite this default setting.
/// XXX (HaaTa): 20 MHz is about the fastest nRF52810 will work at
#define DAP_DEFAULT_SWJ_CLOCK   20000000         ///< Default SWD/JTAG clock frequency in Hz.

/// Clock frequency of the Test Domain Timer. Timer value is returned with \ref TIMESTAMP_GET.
#define TIMESTAMP_CLOCK         1000000U      ///< Timestamp clock in Hz (0 = timestamps not supported).

///@}


//**************************************************************************************************
/**
\defgroup DAP_Config_PortIO_gr CMSIS-DAP Hardware I/O Pin Access
\ingroup DAP_ConfigIO_gr
@{

Standard I/O Pins of the CMSIS-DAP Hardware Debug Port support standard JTAG mode
and Serial Wire Debug (SWD) mode. In SWD mode only 2 pins are required to implement the debug
interface of a device. The following I/O Pins are provided:

SWD I/O Pin          | CMSIS-DAP Hardware pin mode
-------------------- | ---------------------------------------------
SWCLK: Clock         | Output Push/Pull
SWDIO: Data I/O      | Output Push/Pull; Input (for receiving data)
nRESET: Device Reset | Output Open Drain with pull-up resistor


DAP Hardware I/O Pin Access Functions
-------------------------------------
The various I/O Pins are accessed by functions that implement the Read, Write, Set, or Clear to
these I/O Pins.

For the SWDIO I/O Pin there are additional functions that are called in SWD I/O mode only.
This functions are provided to achieve faster I/O that is possible with some advanced GPIO
peripherals that can independently write/read a single I/O pin without affecting any other pins
of the same I/O port. The following SWDIO I/O Pin functions are provided:
 - \ref PIN_SWDIO_OUT_ENABLE to enable the output mode from the DAP hardware.
 - \ref PIN_SWDIO_OUT_DISABLE to enable the input mode to the DAP hardware.
 - \ref PIN_SWDIO_IN to read from the SWDIO I/O pin with utmost possible speed.
 - \ref PIN_SWDIO_OUT to write to the SWDIO I/O pin with utmost possible speed.
*/


// Configure DAP I/O pins ------------------------------

/** Setup SWD I/O pins: SWCLK, SWDIO, and nRESET.
Configures the DAP Hardware I/O pins for Serial Wire Debug (SWD) mode:
 - SWCLK, SWDIO, nRESET to output mode and set to default high level.
*/
__STATIC_INLINE void PORT_SWD_SETUP(void)
{
    PMC->PMC_PCER0 = (1 << 10) | (1 << 11) | (1 << 12);  // Enable clock for all PIOs

    // Using nRST as nRESET means we cannot control it by usually gpio methods

    PIN_SWCLK_PORT->PIO_MDDR = PIN_SWCLK; // Disable multi drive
    PIN_SWCLK_PORT->PIO_PUER = PIN_SWCLK; // pull-up enable
    PIN_SWCLK_PORT->PIO_SODR = PIN_SWCLK; // HIGH
    PIN_SWCLK_PORT->PIO_OER  = PIN_SWCLK; // output
    PIN_SWCLK_PORT->PIO_PER  = PIN_SWCLK; // GPIO control

    PIN_SWDIO_PORT->PIO_MDDR = PIN_SWDIO; // Disable multi drive
    PIN_SWDIO_PORT->PIO_PUER = PIN_SWDIO; // pull-up enable
    PIN_SWDIO_PORT->PIO_SODR = PIN_SWDIO; // HIGH
    PIN_SWDIO_PORT->PIO_OER  = PIN_SWDIO; // output
    PIN_SWDIO_PORT->PIO_PER  = PIN_SWDIO; // GPIO control
}

/** Disable JTAG/SWD I/O Pins.
Disables the DAP Hardware I/O pins which configures:
 - SWCLK, SWDIO, nRESET to High-Z mode.
*/
__STATIC_INLINE void PORT_OFF(void)
{
    Reset_CleanupExternal();

    PIN_SWCLK_PORT->PIO_PUDR  = PIN_SWCLK; // pull-up disable
    PIN_SWCLK_PORT->PIO_PPDDR = PIN_SWCLK; // pull-down disable
    PIN_SWCLK_PORT->PIO_ODR   = PIN_SWCLK; // input
    PIN_SWCLK_PORT->PIO_PER   = PIN_SWCLK; // GPIO control

    PIN_SWDIO_PORT->PIO_PUDR  = PIN_SWDIO; // pull-up disable
    PIN_SWDIO_PORT->PIO_PPDDR = PIN_SWDIO; // pull-down disable
    PIN_SWDIO_PORT->PIO_ODR   = PIN_SWDIO; // input
    PIN_SWDIO_PORT->PIO_PER   = PIN_SWDIO; // GPIO control

}

// SWCLK/TCK I/O pin -------------------------------------

/** SWCLK/TCK I/O pin: Get Input.
\return Current status of the SWCLK/TCK DAP hardware I/O pin.
*/
__STATIC_FORCEINLINE uint32_t PIN_SWCLK_TCK_IN(void)
{
    return ((PIN_SWCLK_PORT->PIO_PDSR >> PIN_SWCLK_BIT) & 1);
}

/** SWCLK/TCK I/O pin: Set Output to High.
Set the SWCLK/TCK DAP hardware I/O pin to high level.
*/
__STATIC_FORCEINLINE void     PIN_SWCLK_TCK_SET(void)
{
    PIN_SWCLK_PORT->PIO_SODR = PIN_SWCLK;
}

/** SWCLK/TCK I/O pin: Set Output to Low.
Set the SWCLK/TCK DAP hardware I/O pin to low level.
*/
__STATIC_FORCEINLINE void     PIN_SWCLK_TCK_CLR(void)
{
    PIN_SWCLK_PORT->PIO_CODR = PIN_SWCLK;
}

// SWDIO/TMS Pin I/O --------------------------------------

/** SWDIO/TMS I/O pin: Get Input.
\return Current status of the SWDIO/TMS DAP hardware I/O pin.
*/
__STATIC_FORCEINLINE uint32_t PIN_SWDIO_TMS_IN(void)
{
    return ((PIN_SWDIO_PORT->PIO_PDSR >> PIN_SWDIO_BIT) & 1);
}

/** SWDIO/TMS I/O pin: Set Output to High.
Set the SWDIO/TMS DAP hardware I/O pin to high level.
*/
__STATIC_FORCEINLINE void     PIN_SWDIO_TMS_SET(void)
{
    PIN_SWDIO_PORT->PIO_SODR = PIN_SWDIO;
}

/** SWDIO/TMS I/O pin: Set Output to Low.
Set the SWDIO/TMS DAP hardware I/O pin to low level.
*/
__STATIC_FORCEINLINE void     PIN_SWDIO_TMS_CLR(void)
{
    PIN_SWDIO_PORT->PIO_CODR = PIN_SWDIO;
}

/** SWDIO I/O pin: Get Input (used in SWD mode only).
\return Current status of the SWDIO DAP hardware I/O pin.
*/
__STATIC_FORCEINLINE uint32_t PIN_SWDIO_IN(void)
{
    return ((PIN_SWDIO_PORT->PIO_PDSR >> PIN_SWDIO_BIT) & 1);
}

/** SWDIO I/O pin: Set Output (used in SWD mode only).
\param bit Output value for the SWDIO DAP hardware I/O pin.
*/
__STATIC_FORCEINLINE void     PIN_SWDIO_OUT(uint32_t bit)
{
    if (bit & 1) {
        PIN_SWDIO_PORT->PIO_SODR = PIN_SWDIO;

    } else {
        PIN_SWDIO_PORT->PIO_CODR = PIN_SWDIO;
    }
}

/** SWDIO I/O pin: Switch to Output mode (used in SWD mode only).
Configure the SWDIO DAP hardware I/O pin to output mode. This function is
called prior \ref PIN_SWDIO_OUT function calls.
*/
__STATIC_FORCEINLINE void     PIN_SWDIO_OUT_ENABLE(void)
{
    PIN_SWDIO_PORT->PIO_OER = PIN_SWDIO;
}

/** SWDIO I/O pin: Switch to Input mode (used in SWD mode only).
Configure the SWDIO DAP hardware I/O pin to input mode. This function is
called prior \ref PIN_SWDIO_IN function calls.
*/
__STATIC_FORCEINLINE void     PIN_SWDIO_OUT_DISABLE(void)
{
    PIN_SWDIO_PORT->PIO_ODR = PIN_SWDIO;
}


// nRESET Pin I/O------------------------------------------

/** nRESET I/O pin: Get Input.
\return Current status of the nRESET DAP hardware I/O pin.
*/
__STATIC_FORCEINLINE uint32_t PIN_nRESET_IN(void)
{
    //return ((PIN_nRESET_PORT->PIO_PDSR >> PIN_nRESET_BIT) & 1);
    return (RSTC->RSTC_SR & RSTC_SR_NRSTL) ? 1 : 0;
}

/** nRESET I/O pin: Set Output.
\param bit target device hardware reset pin status:
           - 0: issue a device hardware reset.
           - 1: release device hardware reset.
*/
__STATIC_FORCEINLINE void     PIN_nRESET_OUT(uint32_t bit)
{
    if (bit & 1) {
        Reset_AssertExternal(10, true);
        //PIN_nRESET_PORT->PIO_SODR = PIN_nRESET;

    } else {
        //PIN_nRESET_PORT->PIO_CODR = PIN_nRESET;
    }
}
///@}


//**************************************************************************************************
/**
\defgroup DAP_Config_Timestamp_gr CMSIS-DAP Timestamp
\ingroup DAP_ConfigIO_gr
@{
Access function for Test Domain Timer.

The value of the Test Domain Timer in the Debug Unit is returned by the function \ref TIMESTAMP_GET. By
default, the DWT timer is used.  The frequency of this timer is configured with \ref TIMESTAMP_CLOCK.

*/

/** Get timestamp of Test Domain Timer.
\return Current timestamp value.
*/
__STATIC_INLINE uint32_t TIMESTAMP_GET (void) {
  return (DWT->CYCCNT) / (CPU_CLOCK / TIMESTAMP_CLOCK);
}

///@}


#endif /* __DAP_CONFIG_H__ */
