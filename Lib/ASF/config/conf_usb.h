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
#define  USB_DEVICE_VERSION               BCD_VERSION
#define  USB_DEVICE_POWER                 100 // Consumption on Vbus line (mA)
#define  USB_DEVICE_ATTR                  (USB_CONFIG_ATTR_REMOTE_WAKEUP|USB_CONFIG_ATTR_SELF_POWERED)
//	(USB_CONFIG_ATTR_REMOTE_WAKEUP|USB_CONFIG_ATTR_BUS_POWERED)
//	(USB_CONFIG_ATTR_BUS_POWERED)

#ifdef USB_Priority_define
#define UDD_USB_INT_LEVEL USB_Priority_define
#endif


/**
 * USB Device Callbacks definitions (Optional)
 * @{
 */
//#define  UDC_VBUS_EVENT(b_vbus_high)
//#define  UDC_SOF_EVENT()                  main_sof_action()
//! Mandatory when USB_DEVICE_ATTR authorizes remote wakeup feature
#define  UDC_REMOTEWAKEUP_ENABLE()
#define  UDC_REMOTEWAKEUP_DISABLE()
//! When a extra string descriptor must be supported
//! other than manufacturer, product and serial string
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


/**
 * Configuration of CDC interface
 * @{
 */

//! Number of communication port used (1 to 3)
#define  UDI_CDC_PORT_NB 1

//! Interface callback definition
#define  UDI_CDC_TX_EMPTY_NOTIFY(port)
#define  UDI_CDC_SET_CODING_EXT(port,cfg)
#define  UDI_CDC_SET_DTR_EXT(port,set)
#define  UDI_CDC_SET_RTS_EXT(port,set)

#define UDI_CDC_ENABLE_EXT(port) usb_cdc_enable_callback()
extern bool usb_cdc_enable_callback();
#define UDI_CDC_DISABLE_EXT(port) usb_cdc_disable_callback()
extern void usb_cdc_disable_callback();
#define  UDI_CDC_RX_NOTIFY(port) usb_cdc_rx_notify_callback(port)
extern void usb_cdc_rx_notify_callback(uint8_t port);
// #define  UDI_CDC_TX_EMPTY_NOTIFY(port) my_callback_tx_empty_notify(port)
// extern void my_callback_tx_empty_notify(uint8_t port);
// #define  UDI_CDC_SET_CODING_EXT(port,cfg) my_callback_config(port,cfg)
// extern void my_callback_config(uint8_t port, usb_cdc_line_coding_t * cfg);
// #define  UDI_CDC_SET_DTR_EXT(port,set) my_callback_cdc_set_dtr(port,set)
// extern void my_callback_cdc_set_dtr(uint8_t port, bool b_enable);
// #define  UDI_CDC_SET_RTS_EXT(port,set) my_callback_cdc_set_rts(port,set)
// extern void my_callback_cdc_set_rts(uint8_t port, bool b_enable);

//! Define it when the transfer CDC Device to Host is a low rate (<512000 bauds)
//! to reduce CDC buffers size
#define  UDI_CDC_LOW_RATE

//! Default configuration of communication port
#define  UDI_CDC_DEFAULT_RATE             115200
#define  UDI_CDC_DEFAULT_STOPBITS         CDC_STOP_BITS_1
#define  UDI_CDC_DEFAULT_PARITY           CDC_PAR_NONE
#define  UDI_CDC_DEFAULT_DATABITS         8
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
#include "udi_dfu_atmel_conf.h"

// ===== Firmware =====
#else
#include "udi_cdc_conf.h"

// - Last Include -
#include "Output/USB/arm/usb_dev.h"
#include "Output/USB/arm/usb_desc.h"
#define  UDC_SUSPEND_EVENT()              usb_set_sleep_state(1)
#define  UDC_RESUME_EVENT()               usb_set_sleep_state(0)
#define USB_DEVICE_SPECIFIC_REQUEST my_udi_hid_setup
#define USB_CONFIGURATION_CHANGED usb_set_configuration

#endif


#include "main.h"

