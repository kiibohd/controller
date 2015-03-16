/* Copyright (c) 2011,2012 Simon Schubert <2@0x2c.org>.
 * Modifications by Jacob Alexander 2014 <haata@kiibohd.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __USB_H
#define __USB_H

// ----- Compiler Includes -----

#include <sys/types.h>



// ----- Local Includes -----

#include "mchck.h"
#include "usb-common.h"



// ----- Defines -----

#define USB_CTRL_REQ_DIR_SHIFT 0
#define USB_CTRL_REQ_TYPE_SHIFT 1
#define USB_CTRL_REQ_RECP_SHIFT 3
#define USB_CTRL_REQ_CODE_SHIFT 8
#define USB_CTRL_REQ(req_inout, req_type, req_code)                     \
	(uint16_t)                                                      \
	((USB_CTRL_REQ_##req_inout << USB_CTRL_REQ_DIR_SHIFT)           \
	 |(USB_CTRL_REQ_##req_type << USB_CTRL_REQ_TYPE_SHIFT)          \
	 |(USB_CTRL_REQ_##req_code << USB_CTRL_REQ_CODE_SHIFT))



// ----- Macros -----

#define USB_DESC_STRING(s)                                      \
	(const void *)&(const struct {                          \
			struct usb_desc_string_t dsc;           \
			char16_t str[sizeof(s) / 2 - 1];        \
	}) {{                                           \
			.bLength = sizeof(struct usb_desc_string_t) +   \
				sizeof(s) - 2,                  \
			.bDescriptorType = USB_DESC_STRING,     \
			},                                      \
			s                                       \
	}
#define USB_DESC_STRING_LANG_ENUS USB_DESC_STRING(u"\x0409")
#define USB_DESC_STRING_SERIALNO ((const void *)1)

#define USB_FUNCTION_IFACE(iface, iface_off, tx_ep_off, rx_ep_off)      \
	((iface_off) + (iface))
#define USB_FUNCTION_TX_EP(ep, iface_off, tx_ep_off, rx_ep_off) \
	((tx_ep_off) + (ep))
#define USB_FUNCTION_RX_EP(ep, iface_off, tx_ep_off, rx_ep_off) \
	((rx_ep_off) + (ep))


#define USB__INCREMENT(i, _0) (i + 1)
#define USB__COUNT_IFACE_EP(i, e)                       \
	__DEFER(USB__COUNT_IFACE_EP_)(__EXPAND i, e)
#define USB__COUNT_IFACE_EP_(iface, tx_ep, rx_ep, func)   \
	(iface + USB_FUNCTION_ ## func ## _IFACE_COUNT,   \
	 tx_ep + USB_FUNCTION_ ## func ## _TX_EP_COUNT,   \
	 rx_ep + USB_FUNCTION_ ## func ## _RX_EP_COUNT)
#define USB__GET_FUNCTION_IFACE_COUNT(iter, func)       \
	USB_FUNCTION_ ## func ## _IFACE_COUNT +

#define USB__DEFINE_FUNCTION_DESC(iter, func)                           \
	USB_FUNCTION_DESC_ ## func ## _DECL __CAT(__usb_func_desc, __COUNTER__);
#define USB__INIT_FUNCTION_DESC(iter, func)     \
	USB_FUNCTION_DESC_ ## func iter,

#define USB__DEFINE_CONFIG_DESC(confignum, name, ...)                   \
	&((const struct name {                                          \
		struct usb_desc_config_t config;                        \
		__REPEAT_INNER(, __EAT, USB__DEFINE_FUNCTION_DESC, __VA_ARGS__) \
	}){                                                             \
		.config = {                                             \
			.bLength = sizeof(struct usb_desc_config_t),    \
			.bDescriptorType = USB_DESC_CONFIG,             \
			.wTotalLength = sizeof(struct name),            \
			.bNumInterfaces = __REPEAT_INNER(, __EAT, USB__GET_FUNCTION_IFACE_COUNT, __VA_ARGS__) 0, \
			.bConfigurationValue = confignum,               \
			.iConfiguration = 0,                            \
			.one = 1,                                       \
			.bMaxPower = 50                                 \
		},                                                      \
		__REPEAT_INNER((0, 0, 0), USB__COUNT_IFACE_EP, USB__INIT_FUNCTION_DESC, __VA_ARGS__) \
	}).config


#define USB__DEFINE_CONFIG(iter, args)                          \
	__DEFER(USB__DEFINE_CONFIG_)(iter, __EXPAND args)

#define USB__DEFINE_CONFIG_(confignum, initfun, ...)                    \
	&(const struct usbd_config){                                    \
		.init = initfun,                                        \
		.desc = USB__DEFINE_CONFIG_DESC(                        \
			confignum,                                      \
			__CAT(__usb_desc, __COUNTER__),                 \
			__VA_ARGS__)                                    \
	},

#define USB_INIT_DEVICE(vid, pid, manuf, product, ...)                  \
	{                                                               \
		.dev_desc = &(const struct usb_desc_dev_t){             \
			.bLength = sizeof(struct usb_desc_dev_t),       \
			.bDescriptorType = USB_DESC_DEV,                \
			.bcdUSB = { .maj = 2 },                         \
			.bDeviceClass = USB_DEV_CLASS_SEE_IFACE,        \
			.bDeviceSubClass = USB_DEV_SUBCLASS_SEE_IFACE,  \
			.bDeviceProtocol = USB_DEV_PROTO_SEE_IFACE,     \
			.bMaxPacketSize0 = EP0_BUFSIZE,                 \
			.idVendor = vid,                                \
			.idProduct = pid,                               \
			.bcdDevice = { .raw = 0 },                      \
			.iManufacturer = 1,                             \
			.iProduct = 2,                                  \
			.iSerialNumber = 3,                             \
			.bNumConfigurations = __PP_NARG(__VA_ARGS__),   \
		},                                                      \
		.string_descs = (const struct usb_desc_string_t * const []){ \
			USB_DESC_STRING_LANG_ENUS,                      \
			USB_DESC_STRING(manuf),                         \
			USB_DESC_STRING(product),                       \
			USB_DESC_STRING_SERIALNO,                       \
			NULL                                            \
		},                                                      \
		.configs = {                                            \
			__REPEAT(1, USB__INCREMENT, USB__DEFINE_CONFIG, __VA_ARGS__) \
			NULL                                            \
		}                                                       \
	}



// ----- Structs & Enumerations -----

/**
 * Note: bitfields ahead.
 * GCC fills the fields lsb-to-msb on little endian.
 */

/**
 * USB descriptors
 */

enum usb_desc_type {
	USB_DESC_DEV = 1,
	USB_DESC_CONFIG = 2,
	USB_DESC_STRING = 3,
	USB_DESC_IFACE = 4,
	USB_DESC_EP = 5,
	USB_DESC_DEVQUAL = 6,
	USB_DESC_OTHERSPEED = 7,
	USB_DESC_POWER = 8
};

struct usb_desc_type_t {
	UNION_STRUCT_START(8);
	enum usb_desc_type id : 5;
	enum usb_desc_type_type {
		USB_DESC_TYPE_STD = 0,
		USB_DESC_TYPE_CLASS = 1,
		USB_DESC_TYPE_VENDOR = 2
	} type_type : 2;
	uint8_t _rsvd0 : 1;
	UNION_STRUCT_END;
};
CTASSERT_SIZE_BYTE(struct usb_desc_type_t, 1);

enum usb_dev_class {
	USB_DEV_CLASS_SEE_IFACE = 0,
	USB_DEV_CLASS_APP = 0xfe,
	USB_DEV_CLASS_VENDOR = 0xff
};

enum usb_dev_subclass {
	USB_DEV_SUBCLASS_SEE_IFACE = 0,
	USB_DEV_SUBCLASS_VENDOR = 0xff
};

enum usb_dev_proto {
	USB_DEV_PROTO_SEE_IFACE = 0,
	USB_DEV_PROTO_VENDOR = 0xff
};

struct usb_bcd_t {
	UNION_STRUCT_START(16);
	struct {
		uint8_t sub : 4;
		uint8_t min : 4;
		uint16_t maj : 8;
	};
	UNION_STRUCT_END;
};
CTASSERT_SIZE_BYTE(struct usb_bcd_t, 2);

struct usb_desc_generic_t {
	uint8_t bLength;
	struct usb_desc_type_t bDescriptorType;
	uint8_t data[];
};
CTASSERT_SIZE_BYTE(struct usb_desc_generic_t, 2);

struct usb_desc_dev_t {
	uint8_t bLength;
	enum usb_desc_type bDescriptorType : 8; /* = USB_DESC_DEV */
	struct usb_bcd_t bcdUSB;             /* = 0x0200 */
	enum usb_dev_class bDeviceClass : 8;
	enum usb_dev_subclass bDeviceSubClass : 8;
	enum usb_dev_proto bDeviceProtocol : 8;
	uint8_t bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	struct usb_bcd_t bcdDevice;
	uint8_t iManufacturer;
	uint8_t iProduct;
	uint8_t iSerialNumber;
	uint8_t bNumConfigurations;
};
CTASSERT_SIZE_BYTE(struct usb_desc_dev_t, 18);

struct usb_desc_ep_t {
	uint8_t bLength;
	enum usb_desc_type bDescriptorType : 8; /* = USB_DESC_EP */
	union {
		struct {
			uint8_t ep_num : 4;
			uint8_t _rsvd0 : 3;
			uint8_t in : 1;
		};
		uint8_t bEndpointAddress;
	};
	struct {
		enum usb_ep_type {
			USB_EP_CONTROL = 0,
			USB_EP_ISO = 1,
			USB_EP_BULK = 2,
			USB_EP_INTR = 3
		} type : 2;
		enum usb_ep_iso_synctype {
			USB_EP_ISO_NOSYNC = 0,
			USB_EP_ISO_ASYNC = 1,
			USB_EP_ISO_ADAPTIVE = 2,
			USB_EP_ISO_SYNC = 3
		} sync_type : 2;
		enum usb_ep_iso_usagetype {
			USB_EP_ISO_DATA = 0,
			USB_EP_ISO_FEEDBACK = 1,
			USB_EP_ISO_IMPLICIT = 2
		} usage_type : 2;
		uint8_t _rsvd1 : 2;
	};
	struct {
		uint16_t wMaxPacketSize : 11;
		uint16_t _rsvd2 : 5;
	};
	uint8_t bInterval;
} __packed;
CTASSERT_SIZE_BYTE(struct usb_desc_ep_t, 7);

struct usb_desc_iface_t {
	uint8_t bLength;
	enum usb_desc_type bDescriptorType : 8; /* = USB_DESC_IFACE */
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	enum usb_dev_class bInterfaceClass : 8;
	enum usb_dev_subclass bInterfaceSubClass: 8;
	enum usb_dev_proto bInterfaceProtocol : 8;
	uint8_t iInterface;
};
CTASSERT_SIZE_BYTE(struct usb_desc_iface_t, 9);

struct usb_desc_config_t {
	uint8_t bLength;
	enum usb_desc_type bDescriptorType : 8; /* = USB_DESC_CONFIG */
	uint16_t wTotalLength;       /* size of config, iface, ep */
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	struct {
		uint8_t _rsvd0 : 5;
		uint8_t remote_wakeup : 1;
		uint8_t self_powered : 1;
		uint8_t one : 1; /* = 1 for historical reasons */
	};
	uint8_t bMaxPower;      /* units of 2mA */
} __packed;
CTASSERT_SIZE_BYTE(struct usb_desc_config_t, 9);

struct usb_desc_string_t {
	uint8_t bLength;
	enum usb_desc_type bDescriptorType : 8; /* = USB_DESC_STRING */
	const char16_t bString[];
};
CTASSERT_SIZE_BYTE(struct usb_desc_string_t, 2);

struct usb_ctrl_req_t {
	union /* reqtype and request & u16 */ {
		struct /* reqtype and request */ {
			union /* reqtype in bitfield & u8 */ {
				struct /* reqtype */ {
					enum usb_ctrl_req_recp {
						USB_CTRL_REQ_DEV = 0,
						USB_CTRL_REQ_IFACE = 1,
						USB_CTRL_REQ_EP = 2,
						USB_CTRL_REQ_OTHER = 3
					} recp : 5;
					enum usb_ctrl_req_type {
						USB_CTRL_REQ_STD = 0,
						USB_CTRL_REQ_CLASS = 1,
						USB_CTRL_REQ_VENDOR = 2
					} type : 2;
					enum usb_ctrl_req_dir {
						USB_CTRL_REQ_OUT = 0,
						USB_CTRL_REQ_IN = 1
					} in : 1;
				};
				uint8_t bmRequestType;
			}; /* union */
			enum usb_ctrl_req_code {
				USB_CTRL_REQ_GET_STATUS = 0,
				USB_CTRL_REQ_CLEAR_FEATURE = 1,
				USB_CTRL_REQ_SET_FEATURE = 3,
				USB_CTRL_REQ_SET_ADDRESS = 5,
				USB_CTRL_REQ_GET_DESCRIPTOR = 6,
				USB_CTRL_REQ_SET_DESCRIPTOR = 7,
				USB_CTRL_REQ_GET_CONFIGURATION = 8,
				USB_CTRL_REQ_SET_CONFIGURATION = 9,
				USB_CTRL_REQ_GET_INTERFACE = 10,
				USB_CTRL_REQ_SET_INTERFACE = 11,
				USB_CTRL_REQ_SYNC_FRAME = 12
			} bRequest : 8;
		}; /* struct */
		uint16_t type_and_req;
	}; /* union */
	union {
		uint16_t wValue;
		struct {
			uint8_t wValueLow;
			uint8_t wValueHigh;
		};
	};
	uint16_t wIndex;
	uint16_t wLength;
};
CTASSERT_SIZE_BYTE(struct usb_ctrl_req_t, 8);

/**
 * status replies for GET_STATUS.
 */

struct usb_ctrl_req_status_dev_t {
	uint16_t self_powered : 1;
	uint16_t remote_wakeup : 1;
	uint16_t _rsvd : 14;
};
CTASSERT_SIZE_BIT(struct usb_ctrl_req_status_dev_t, 16);

struct usb_ctrl_req_status_iface_t {
	uint16_t _rsvd;
};
CTASSERT_SIZE_BIT(struct usb_ctrl_req_status_iface_t, 16);

struct usb_ctrl_req_status_ep_t {
	uint16_t halt : 1;
	uint16_t _rsvd : 15;
};
CTASSERT_SIZE_BIT(struct usb_ctrl_req_status_ep_t, 16);

/**
 * Descriptor type (in req->value) for GET_DESCRIPTOR.
 */
struct usb_ctrl_req_desc_t {
	uint8_t idx;
	enum usb_desc_type type : 8;
};
CTASSERT_SIZE_BIT(struct usb_ctrl_req_desc_t, 16);

/**
 * Feature selector (in req->value) for CLEAR_FEATURE.
 */
enum usb_ctrl_req_feature {
	USB_CTRL_REQ_FEAT_EP_HALT = 0,
	USB_CTRL_REQ_FEAT_DEV_REMOTE_WKUP = 1,
	USB_CTRL_REQ_FEAT_TEST_MODE = 2
};


struct usb_xfer_info;
typedef void (*ep_callback_t)(void *buf, ssize_t len, void *data);

/**
 * (Artificial) function.  Aggregates one or more interfaces.
 */
struct usbd_function {
	int (*configure)(int orig_iface, int iface, int altsetting, void *data);
	int (*control)(struct usb_ctrl_req_t *, void *);
	int interface_count;
	int ep_rx_count;
	int ep_tx_count;
};

struct usbd_function_ctx_header {
	struct usbd_function_ctx_header *next;
	const struct usbd_function *function;
	int interface_offset;
	int ep_rx_offset;
	int ep_tx_offset;
};


typedef void (usbd_init_fun_t)(int);
typedef void (usbd_suspend_resume_fun_t)(void);

/**
 * Configuration.  Contains one or more functions which all will be
 * active concurrently.
 */
struct usbd_config {
	usbd_init_fun_t *init;
	usbd_suspend_resume_fun_t *suspend;
	usbd_suspend_resume_fun_t *resume;
	/**
	 * We will not set a config for now, because there is not much to
	 * configure, except for power
	 *
	 * const struct usb_desc_config_t *config_desc;
	 */
	const struct usb_desc_config_t *desc;
	const struct usbd_function *function[];
};


/**
 * Device.  Contains one or more configurations, out of which only one
 * is active at a time.
 */
struct usbd_device {
	const struct usb_desc_dev_t *dev_desc;
	const struct usb_desc_string_t * const *string_descs;
	const struct usbd_config *configs[];
};



/* Provided by MD code */
struct usbd_ep_pipe_state_t;



// ----- Functions -----

void *usb_get_xfer_data(struct usb_xfer_info *);
enum usb_tok_pid usb_get_xfer_pid(struct usb_xfer_info *);
int usb_get_xfer_ep(struct usb_xfer_info *);
enum usb_ep_dir usb_get_xfer_dir(struct usb_xfer_info *);
void usb_enable_xfers(void);
void usb_set_addr(int);
void usb_ep_stall(int);
size_t usb_ep_get_transfer_size(struct usbd_ep_pipe_state_t *);
void usb_queue_next(struct usbd_ep_pipe_state_t *, void *, size_t);
void usb_pipe_stall(struct usbd_ep_pipe_state_t *);
void usb_pipe_unstall(struct usbd_ep_pipe_state_t *);
void usb_pipe_enable(struct usbd_ep_pipe_state_t *s);
void usb_pipe_disable(struct usbd_ep_pipe_state_t *s);
#ifdef VUSB
void vusb_main_loop(void);
#else
void usb_poll(void);
#endif
int usb_tx_serialno(size_t reqlen);

/* Provided by MI code */
void usb_init(const struct usbd_device *);
void usb_attach_function(const struct usbd_function *function, struct usbd_function_ctx_header *ctx);
void usb_handle_transaction(struct usb_xfer_info *);
void usb_setup_control(void);
void usb_handle_control_status_cb(ep_callback_t cb);
void usb_handle_control_status(int);
struct usbd_ep_pipe_state_t *usb_init_ep(struct usbd_function_ctx_header *ctx, int ep, enum usb_ep_dir dir, size_t size);
int usb_rx(struct usbd_ep_pipe_state_t *, void *, size_t, ep_callback_t, void *);
int usb_tx(struct usbd_ep_pipe_state_t *, const void *, size_t, size_t, ep_callback_t, void *);

int usb_ep0_rx(void *, size_t, ep_callback_t, void *);
void *usb_ep0_tx_inplace_prepare(size_t len);
int usb_ep0_tx(void *buf, size_t len, size_t reqlen, ep_callback_t cb, void *cb_data);
int usb_ep0_tx_cp(const void *, size_t, size_t, ep_callback_t, void *);



// ----- DFU USB Additional Includes -----

#include "dfu.h"

#endif

