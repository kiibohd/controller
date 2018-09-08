/**
 * \file
 *
 * \brief USB Device Firmware Upgrade (DFU) protocol definitions.
 *
 * Copyright (c) 2011-2014 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
 /**
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#ifndef _USB_PROTOCOL_DFU_H_
#define _USB_PROTOCOL_DFU_H_


/**
 * \ingroup usb_protocol_group
 * \defgroup usb_dfu_protocol USB Device Firmware Upgrade (DFU)
 * protocol definitions
 *
 * @{
 */

/**
 * \name Class value
 */
//@{
#define  DFU_CLASS      0xFE
//@}

/**
 * \name SubClass value
 */
//@{
#define  DFU_SUBCLASS   0x01
//@}

/**
 * \name protocol value
 */
//@{
#define  DFU_PROTOCOL   0x01
//@}


/**
 * \brief DFU USB requests (bRequest)
 */
enum usb_reqid_dfu {
	USB_REQ_DFU_DETACH = 0x00,
	USB_REQ_DFU_DNLOAD = 0x01,
	USB_REQ_DFU_UPLOAD = 0x02,
	USB_REQ_DFU_GETSTATUS = 0x03,
	USB_REQ_DFU_CLRSTATUS = 0x04,
	USB_REQ_DFU_GETSTATE = 0x05,
	USB_REQ_DFU_ABORT = 0x06,
};

/**
 * \brief DFU USB descriptor types
 */
enum usb_descriptor_type_dfu {
	USB_DT_DFU_FUNCTIONAL = 0x21,
};


//! \name USB DFU Status IDs
//@{
#define  DFU_STATUS_OK                    0x00
#define  DFU_STATUS_ERRTARGET             0x01
#define  DFU_STATUS_ERRFILE               0x02
#define  DFU_STATUS_ERRWRITE              0x03
#define  DFU_STATUS_ERRERASE              0x04
#define  DFU_STATUS_ERRCHECK_ERASED       0x05
#define  DFU_STATUS_ERRPROG               0x06
#define  DFU_STATUS_ERRVERIFY             0x07
#define  DFU_STATUS_ERRADDRESS            0x08
#define  DFU_STATUS_ERRNOTDONE            0x09
#define  DFU_STATUS_ERRFIRMWARE           0x0A
#define  DFU_STATUS_ERRVENDOR             0x0B
#define  DFU_STATUS_ERRUSBR               0x0C
#define  DFU_STATUS_ERRPOR                0x0D
#define  DFU_STATUS_ERRUNKNOWN            0x0E
#define  DFU_STATUS_ERRSTALLEDPK          0x0F
//@}

//! \name USB DFU State IDs
//@{
#define DFU_STATE_APPIDLE                 0x00
#define DFU_STATE_APPDETACH               0x01
#define DFU_STATE_DFUIDLE                 0x02
#define DFU_STATE_DFUDNLOAD_SUNC          0x03
#define DFU_STATE_DFUDNBUSY               0x04
#define DFU_STATE_DFUDNLOAD_IDLE          0x05
#define DFU_STATE_DFUMANIFEST_SYNC        0x06
#define DFU_STATE_DFUMANIFEST             0x07
#define DFU_STATE_DFUMANIFEST_WAIT_RESET  0x08
#define DFU_STATE_DFUUPLOAD_IDLE          0x09
#define DFU_STATE_DFUERROR                0x0A
//@}


/**
 * \brief DFU Functional attributes
 */
enum usb_dfu_functional_attributes {
	USB_DFU_FONC_CAN_DNLOAD = (1 << 0),
	USB_DFU_FONC_CAN_UPLOAD = (1 << 1),
	USB_DFU_FONC_MANIFEST_TOLERANT = (1 << 2),
	USB_DFU_FONC_WILL_DETACH = (1 << 3),
};

//! Value for fields bcdDFU(Version)
#define  USB_DFU_V1_0    0x0100	//!< USB DFU Specification version 1.0

COMPILER_PACK_SET(1)

/**
 * \brief DFU Descriptor
 */
typedef struct {
	uint8_t bLength;        //!< Size of this descriptor in bytes
	uint8_t bDescriptorType;//!< DFU functional descriptor type
	uint8_t bmAttributes;   //!< DFU attributes
	le16_t wDetachTimeOut;  //!< Detach timeout
	le16_t wTransferSize;   //!< Maximum number of bytes that the device can accept
	le16_t bcdDFUVersion;   //!< DFU specification release
} usb_dfu_functional_desc_t;

/**
 * \brief DFU Status
 */
typedef struct {
	uint8_t bStatus;
	uint8_t bwPollTimeout[3];
	uint8_t bState;
	uint8_t iString;
} dfu_status_t;

/**
 * \brief DFU File Suffix
 */
typedef struct {
	le32_t dwCRC;              //!< Number The CRC of the entire file, excluding dwCRC.
	uint8_t bLength;           //!< The length of this DFU suffix including dwCRC.
	uint8_t ucDfuSignature[3]; //!< The unique DFU signature field.
	le16_t bcdDFU;             //!< DFU specification number.
	le16_t idVendor;           //!< The vendor ID associated with this file.
	//!< Either FFFFh or must match device's vendor ID.
	le16_t idProduct;          //!< The product ID associated with this file.
	//!< Either FFFFh or must match device's product ID.
	le16_t bcdDevice;          //!< The release number of the device
	//!< Either FFFFh or a BCD firmware release or version number.
} dfu_file_suffix_t;

COMPILER_PACK_RESET()

//@}

#endif // _USB_PROTOCOL_DFU_H_
