/**
 * \file
 *
 * \brief ISP configuration file template.
 *
 * Copyright (c) 2014-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#ifndef _CONF_ISP_H_
#define _CONF_ISP_H_

// Bootloader Versions
// Example: Version 0x00 gives 1.0.0 on batchisp log
// Example: Version 0x03 gives 1.0.3 on batchisp log
// Example: Version 0x25 gives 1.2.5 on batchisp log
//
// Note: a specific UC3 rule is defined:
// - 1.0.X for USB bootloaders that follow the AVR32784 application note
// specification
// - 1.1.X for USB bootloaders that follow the AVR32806 application note
// specification
//
#define BOOTLOADER_VERSION   0x00  // 1.0.0

// If all memories (flash,eeprom,...) do not exceed 64KB.
// then the ISP interface can be optimized to save CODE.
//#define ISP_SMALL_MEMORY_SIZE

// Definition of hardware condition to enter in ISP mode on AVR Xmega devices
#define ISP_PORT_DIR      PORTX_DIR
#define ISP_PORT_PINCTRL  PORTX_PIN5CTRL
#define ISP_PORT_IN       PORTX_IN
#define ISP_PORT_PIN      0 to 7

#endif // _CONF_ISP_H_
