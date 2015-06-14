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

// ----- Enumerations -----

/**
 * USB request data structures.
 */

enum usb_tok_pid {
	USB_PID_TIMEOUT = 0,
	USB_PID_OUT = 1,
	USB_PID_ACK = 2,
	USB_PID_DATA0 = 3,
	USB_PID_IN = 9,
	USB_PID_NAK = 10,
	USB_PID_DATA1 = 11,
	USB_PID_SETUP = 13,
	USB_PID_STALL = 14,
	USB_PID_DATAERR = 15
};

enum usb_ep_pingpong {
	USB_EP_PINGPONG_EVEN = 0,
	USB_EP_PINGPONG_ODD = 1
};

enum usb_ep_dir {
	USB_EP_RX = 0,
	USB_EP_TX = 1
};

enum usb_data01 {
	USB_DATA01_DATA0 = 0,
	USB_DATA01_DATA1 = 1
};

enum {
	EP0_BUFSIZE = 64
};

