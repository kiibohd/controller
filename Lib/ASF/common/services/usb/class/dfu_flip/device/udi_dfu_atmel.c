/**
 * \file
 *
 * \brief USB Device Atmel Firmware Upgrade (DFU) interface definitions.
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

#include "conf_usb.h"
#include <common/services/usb/usb_protocol.h>
#include "../usb_protocol_dfu.h"
#include <common/services/usb/udc/udd.h>
#include <common/services/usb/udc/udc.h>
#include "udi_dfu_atmel.h"
#include "conf_isp.h"
#include "isp.h"
#include "flip_protocol.h"
#include "string.h"

#ifndef FLIP_PROTOCOL_VERSION
#  error FLIP_PROTOCOL_VERSION must be define in conf_usb.h
#endif
#if ((FLIP_PROTOCOL_VERSION != FLIP_PROTOCOL_VERSION_2) \
    &&  (FLIP_PROTOCOL_VERSION != FLIP_PROTOCOL_VERSION_1))
#  error Bootloader protocol not supported (FLIP_PROTOCOL_VERSION)
#endif

/**
 * \ingroup udi_dfu_atmel_group
 * \defgroup udi_dfu_atmel_group_udc Interface with USB Device Core (UDC)
 *
 * Structures and functions required by UDC.
 *
 * @{
 */
bool udi_dfu_atmel_enable(void);
void udi_dfu_atmel_disable(void);
bool udi_dfu_atmel_setup(void);
uint8_t udi_dfu_atmel_getsetting(void);

//! Global structure which contains standard UDI interface for UDC
UDC_DESC_STORAGE udi_api_t udi_api_dfu_atmel = {
	.enable = udi_dfu_atmel_enable,
	.disable = udi_dfu_atmel_disable,
	.setup = udi_dfu_atmel_setup,
	.getsetting = udi_dfu_atmel_getsetting,
	.sof_notify = NULL,
};
//@}


/**
 * \ingroup udi_dfu_atmel_group
 * \defgroup udi_dfu_atmel_group_internal Implementation of UDI DFU Atmel Class
 *
 * Class internal implementation
 * @{
 */

/**
 * \name Internal routines to manage DFU requests
 */
//@{

//! \brief Resets DFU status and usb setup process callbacks
static void udi_dfu_atmel_reset_protocol(void);


//! Macro to simplify and to optimize command decode process
#define CAT_CMD(val1,val2) (((uint16_t)val1<<8)|(val2<<0))


/**
 * \name Internal variables to manage DFU requests
 */
//@{

//! Status of DFU process
static dfu_status_t udi_dfu_atmel_status;

#if (FLIP_PROTOCOL_VERSION == FLIP_PROTOCOL_VERSION_2)
#  ifdef UDI_DFU_SMALL_RAM
#    define DFU_ATMEL_BUF_TRANS_SIZE     (FLIP_V2_BUF_TRANS_SIZE/2)
#  else
#    define DFU_ATMEL_BUF_TRANS_SIZE     FLIP_V2_BUF_TRANS_SIZE
#  endif
#else // V1
#  define DFU_ATMEL_BUF_TRANS_SIZE     FLIP_V1_BUF_TRANS_SIZE
#endif


//! Notify a reset request to start
static bool udi_dfu_atmel_reset;

//! Store the current security level
static bool udi_dfu_atmel_security;

/**
 * \name To manage memories
 */
//@{
//! Memory ID selected to read, write or check blank
static isp_mem_t udi_dfu_atmel_mem_sel;
//@}

//@}


bool udi_dfu_atmel_enable(void)
{
	udi_dfu_atmel_reset = false;
	udi_dfu_atmel_reset_protocol();


	udi_dfu_atmel_mem_sel = *isp_memories.list.flash;

	// Load chip information
	isp_init();
	udi_dfu_atmel_security = isp_is_security();
	return true; //(bool)UDI_DFU_ENABLE_EXT();
}


void udi_dfu_atmel_disable(void)
{
	UDI_DFU_DISABLE_EXT();
}

extern struct dfu_ctx dfu_ctx;
int dfu_handle_control( void* req, void *data );

bool udi_dfu_atmel_setup(void)
{
	return dfu_handle_control((void*) &udd_g_ctrlreq.req,  &dfu_ctx);
}


uint8_t udi_dfu_atmel_getsetting(void)
{
	return 0;
}


static void udi_dfu_atmel_reset_protocol(void)
{
	// Reset DFU status
	udi_dfu_atmel_status.bStatus = DFU_STATUS_OK;
	udi_dfu_atmel_status.bState = DFU_STATE_DFUIDLE;
	// These fields are not used and always set to zero:
	// bwPollTimeout[3]
	// iString

	// Reset all callbacks
	udd_g_ctrlreq.over_under_run = NULL;
	udd_g_ctrlreq.callback = NULL;
}

//@}
