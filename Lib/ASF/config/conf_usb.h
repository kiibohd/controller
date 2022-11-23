/**
 * \file
 *
 * \brief USB configuration file
 *
 * Copyright (c) 2009-2018 Microchip Technology Inc. and its subsidiaries.
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

#pragma once

#include <buildvars.h>
#include <kll_defs.h>
#include "compiler.h"

/**
 * USB Device Configuration
 * @{
 */

//! Device definition (mandatory)
#define  USB_DEVICE_VENDOR_ID             VENDOR_ID
#define  USB_DEVICE_PRODUCT_ID            PRODUCT_ID
#define  USB_DEVICE_ALT_PRODUCT_ID        ALT_PRODUCT_ID
#define  USB_DEVICE_VERSION               BCD_VERSION
#define  USB_DEVICE_ATTR                  (USB_CONFIG_ATTR_REMOTE_WAKEUP|USB_CONFIG_ATTR_BUS_POWERED)

#ifdef USB_Priority_define
#define UDD_USB_INT_LEVEL USB_Priority_define
#endif


/**
 * USB Device Callbacks definitions (Optional)
 * @{
 */
//#define  UDC_VBUS_EVENT(b_vbus_high)
//! Mandatory when USB_DEVICE_ATTR authorizes remote wakeup feature
#define  UDC_REMOTEWAKEUP_ENABLE()
#define  UDC_REMOTEWAKEUP_DISABLE()
//@}

//@}


/**
 * USB Interface Configuration
 * @{
 */


/**
 * Configuration of DFU interface
 * @{
 */
//! Interface callback definition
#define  UDI_DFU_ENABLE_EXT()             true
#define  UDI_DFU_DISABLE_EXT()

//! FLIP protocol version to use
#define  FLIP_PROTOCOL_VERSION   FLIP_PROTOCOL_VERSION_2
// Split erase is available since batchisp 1.2.5 to avoid USB protocol 2 error
#define  UDI_DFU_ATMEL_PROTOCOL_2_SPLIT_ERASE_CHIP
// Reduce the RAM used (1KB instead of 2KB), but the CODE increase of 80B
#define  UDI_DFU_SMALL_RAM
//@}
//@}



//! The includes of classes and other headers must be done at the end of this file to avoid compile error
// ===== Bootloader =====
#if defined(_bootloader_)
bool udi_dfu_atmel_setup(void);
extern bool main_extra_string();

#undef USB_DEVICE_MAX_EP
#define USB_DEVICE_SPECIFIC_REQUEST udi_dfu_atmel_setup
#define UDC_GET_EXTRA_STRING() main_extra_string()
#define USB_DEVICE_MAX_EP 1
#define  USB_DEVICE_POWER                 100 // Consumption on Vbus line (mA)
#include "udi_dfu_atmel_conf.h"

// ===== Firmware =====
#else

// - Last Include -
#include "Output/USB/arm/usb_dev.h"
#include "Output/USB/arm/usb_desc.h"
#define  UDC_SUSPEND_EVENT()              usb_set_sleep_state(1)
#define  UDC_RESUME_EVENT()               usb_set_sleep_state(0)
#define  UDC_SOF_EVENT()                  usb_sof_event()
#define USB_DEVICE_SPECIFIC_REQUEST my_udi_hid_setup
#define USB_CONFIGURATION_CHANGED usb_set_configuration
#define  USB_DEVICE_POWER                 500 // Consumption on Vbus line (mA)

#endif


#include "main.h"

