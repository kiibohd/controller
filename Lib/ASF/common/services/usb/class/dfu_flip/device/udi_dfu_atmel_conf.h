/**
 * \file
 *
 * \brief Default Atmel DFU configuration for a USB Device
 * with a single interface Atmel DFU
 *
 * Copyright (c) 2011-2018 Microchip Technology Inc. and its subsidiaries.
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

#ifndef _UDI_DFU_ATMEL_CONF_H_
#define _UDI_DFU_ATMEL_CONF_H_

#include "conf_usb.h"
#include "flip_protocol.h"

/**
 * \addtogroup udi_dfu_atmel_group_single_desc
 * @{
 */

//! Control endpoint size
#if (FLIP_PROTOCOL_VERSION == FLIP_PROTOCOL_VERSION_2)
#define  USB_DEVICE_EP_CTRL_SIZE       64
#else
#define  USB_DEVICE_EP_CTRL_SIZE       32
#endif

//! Interface number
#define  UDI_DFU_ATMEL_IFACE_NUMBER    0

/**
 * \name UDD Configuration
 */
//@{
//! 0 endpoints used by DFU interface
//#define  USB_DEVICE_MAX_EP             0
//@}

//@}

#endif // _UDI_DFU_ATMEL_CONF_H_
