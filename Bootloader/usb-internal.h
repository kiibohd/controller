/* Copyright (c) 2011,2012 Simon Schubert <2@0x2c.org>.
 * Modifications by Jacob Alexander 2014-2015 <haata@kiibohd.com>
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

#pragma once

/**
 * Internal driver structures
 */

/**
 * USB state machine
 * =================
 *
 * Device configuration states:
 *
 * Attached <-> Powered
 * Powered -(reset)-> Default
 * Default -(SET_ADDRESS)-> Address
 * Address -(SET_CONFIGURATION)-> Configured
 * Configured -(SET_CONFIGURATION 0)-> Address
 * Address -(SET_ADDRESS 0)-> Default
 * [Default, Configured, Address] -(reset)-> Default
 */

// ----- Defines -----

#ifndef USB_MAX_EP
#define USB_MAX_EP 16
#endif



// ----- Structs -----

struct usbd_ep_pipe_state_t {
	enum usb_ep_pingpong pingpong; /* next desc to use */
	enum usb_data01 data01;
	size_t transfer_size;
	size_t pos;
	uint8_t *data_buf;
	const uint8_t *copy_source;
	int short_transfer;
	ep_callback_t callback;
	void *callback_data;
	size_t ep_maxsize;
	/* constant */
	int ep_num;
	enum usb_ep_dir ep_dir;
};

struct usbd_ep_state_t {
	union {
		struct usbd_ep_pipe_state_t pipe[2];
		struct {
			struct usbd_ep_pipe_state_t rx;
			struct usbd_ep_pipe_state_t tx;
		};
	};
};

struct usbd_t {
	struct usbd_function_ctx_header functions;
	struct usbd_function control_function;
	const struct usbd_device *identity;
	int address;
	int config;
	enum usbd_dev_state {
		USBD_STATE_DISABLED = 0,
		USBD_STATE_DEFAULT,
		USBD_STATE_SETTING_ADDRESS,
		USBD_STATE_ADDRESS,
		USBD_STATE_CONFIGURED
	} state;
	enum usb_ctrl_req_dir ctrl_dir;
	struct usbd_ep_state_t ep_state[USB_MAX_EP];
};

extern struct usbd_t usb;



// ----- Functions -----

void usb_restart(void);
void usb_enable(void);
const struct usbd_config *usb_get_config_data(int config);

