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

// ----- Compiler Includes -----

#include <sys/types.h>
#include <inttypes.h>
#include <string.h>


// ----- Local Includes -----

#include "usb.h"
#include "usb-internal.h"



// ----- Variables -----

static uint8_t ep0_buf[2][EP0_BUFSIZE] __attribute__((aligned(4)));
struct usbd_t usb;



// ----- Functions -----

/**
 * Returns: 0 when this is was the last transfer, 1 if there is still
 * more to go.
 */
/* Defaults to EP0 for now */
static int usb_tx_next(struct usbd_ep_pipe_state_t *s)
{

	/**
	 * Us being here means the previous transfer just completed
	 * successfully.  That means the host just toggled its data
	 * sync bit, and so do we.
	 */
	s->data01 ^= 1;

	if (s->transfer_size > 0) {
		size_t thislen = s->transfer_size;

		if (thislen > s->ep_maxsize)
			thislen = s->ep_maxsize;

		void *addr = s->data_buf + s->pos;

		if (s->copy_source) {
			/* Bounce buffer mode */
			addr = s->data_buf;
			memcpy(addr, s->copy_source + s->pos, thislen);
		}
		s->pos += thislen;
		s->transfer_size -= thislen;

		usb_queue_next(s, addr, thislen);
		s->pingpong ^= 1;

		return (1);
	}

	/**
	 * All data has been shipped.  Do we need to send a short
	 * packet?
	 */
	if (s->short_transfer) {
		s->short_transfer = 0;
		usb_queue_next(s, NULL, 0);
		s->pingpong ^= 1;
		return (1);
	}

	if (s->callback)
		s->callback(s->data_buf, s->pos, s->callback_data);

	return (0);
}

static void setup_tx(struct usbd_ep_pipe_state_t *s, const void *buf, size_t len, size_t reqlen, ep_callback_t cb, void *cb_data)
{
	s->data_buf = (void *)buf;
	s->copy_source = NULL;
	s->transfer_size = len;
	s->pos = 0;
	s->callback = cb;
	s->callback_data = cb_data;
	if (s->transfer_size > reqlen)
		s->transfer_size = reqlen;
	if (s->transfer_size < reqlen && s->transfer_size % s->ep_maxsize == 0)
		s->short_transfer = 1;
	else
		s->short_transfer = 0;
}

static void submit_tx(struct usbd_ep_pipe_state_t *s)
{
	/* usb_tx_next() flips the data toggle, so invert this here. */
	s->data01 ^= 1;
	usb_tx_next(s);
}

/**
 * send USB data (IN device transaction)
 *
 * So far this function is specialized for EP 0 only.
 *
 * Returns: size to be transfered, or -1 on error.
 */
int usb_tx(struct usbd_ep_pipe_state_t *s, const void *buf, size_t len, size_t reqlen, ep_callback_t cb, void *cb_data)
{
	setup_tx(s, buf, len, reqlen, cb, cb_data);
	submit_tx(s);
	return (s->transfer_size);
}


/**
 * Returns: 0 when this is was the last transfer, 1 if there is still
 * more to go.
 */
/* Defaults to EP0 for now */
/* XXX pass usb_stat to validate pingpong */
static int usb_rx_next(struct usbd_ep_pipe_state_t *s)
{
	/**
	 * Us being here means the previous transfer just completed
	 * successfully.  That means the host just toggled its data
	 * sync bit, and so do we.
	 */
	s->data01 ^= 1;

	size_t thislen = usb_ep_get_transfer_size(s);

	s->transfer_size -= thislen;
	s->pos += thislen;

	/**
	 * We're done with this buffer now.  Switch the pingpong now
	 * before we might have to receive the next piece of data.
	 */
	s->pingpong ^= 1;

	/**
	 * If this is a short transfer, or we received what we
	 * expected, we're done.
	 */
	if (thislen < s->ep_maxsize || s->transfer_size == 0) {
		if (s->callback)
			s->callback(s->data_buf, s->pos, s->callback_data);
		return (0);
	}

	/**
	 * Otherwise we still need to receive more data.
	 */
	size_t nextlen = s->transfer_size;

	if (nextlen > s->ep_maxsize)
		nextlen = s->ep_maxsize;

	void *addr = s->data_buf + s->pos;
	usb_queue_next(s, addr, nextlen);

	return (1);
}

/**
 * Receive USB data (OUT device transaction)
 *
 * Returns: size to be received, or -1 on error.
 */
int usb_rx(struct usbd_ep_pipe_state_t *s, void *buf, size_t len, ep_callback_t cb, void *cb_data)
{
	s->data_buf = buf;
	s->transfer_size = len;
	s->pos = 0;
	s->callback = cb;
	s->callback_data = cb_data;

	size_t thislen = s->transfer_size;
	if (thislen > s->ep_maxsize)
		thislen = s->ep_maxsize;

	usb_queue_next(s, s->data_buf, thislen);
	return (len);
}

int usb_ep0_tx_cp(const void *buf, size_t len, size_t reqlen, ep_callback_t cb, void *cb_data)
{
	struct usbd_ep_pipe_state_t *s = &usb.ep_state[0].tx;
	enum usb_ep_pingpong pp = s->pingpong;

	setup_tx(s, ep0_buf[pp], len, reqlen, cb, cb_data);
	s->copy_source = buf;
	submit_tx(s);
	return (s->transfer_size);
}

void *usb_ep0_tx_inplace_prepare(size_t len)
{
	enum usb_ep_pingpong pp = usb.ep_state[0].tx.pingpong;

	if (len > EP0_BUFSIZE)
		return (NULL);

	return (ep0_buf[pp]);
}

int usb_ep0_tx(void *buf, size_t len, size_t reqlen, ep_callback_t cb, void *cb_data)
{
	return (usb_tx(&usb.ep_state[0].tx, buf, len, reqlen, cb, cb_data));
}

int usb_ep0_rx(void *buf, size_t len, ep_callback_t cb, void *cb_data)
{
	return (usb_rx(&usb.ep_state[0].rx, buf, len, cb, cb_data));
}


const struct usbd_config *
usb_get_config_data(int config)
{
	if (config <= 0)
		config = usb.config;

	if (config != 0)
		return (usb.identity->configs[config - 1]);
	else
		return (NULL);
}

static int usb_set_config(int config)
{
	const struct usbd_config *config_data;

	if (usb.config != 0) {
		config_data = usb_get_config_data(-1);
		if (config_data != NULL && config_data->init != NULL)
			config_data->init(0);
	}

	if (config != 0) {
		/* XXX overflow */
		config_data = usb_get_config_data(config);
		if (config_data != NULL && config_data->init != NULL)
			config_data->init(1);
	}
	usb.config = config;
	return (0);
}

static int usb_set_interface(int iface, int altsetting)
{
	int iface_count = 0;

	for (struct usbd_function_ctx_header *fh = &usb.functions;
	     fh != NULL;
	     fh = fh->next, iface_count += fh->function->interface_count) {
		if (iface - iface_count < fh->function->interface_count) {
			if (fh->function->configure != NULL)
				return (fh->function->configure(iface,
								iface - iface_count,
								altsetting,
								fh));

			/* Default to a single altsetting */
			if (altsetting != 0)
				return (-1);
			else
				return (0);
		}
	}

	return (-1);
}

static int usb_tx_config_desc(int idx, int reqlen)
{
	const struct usb_desc_config_t *d = usb.identity->configs[idx]->desc;

	usb_ep0_tx_cp(d, d->wTotalLength, reqlen, NULL, NULL);
	return (0);
}

static int usb_tx_string_desc(int idx, int reqlen)
{
	const struct usb_desc_string_t * const *d;

	for (d = usb.identity->string_descs; idx != 0 && *d != NULL; ++d)
		--idx;
	switch ((uintptr_t)*d) {
	case (uintptr_t)NULL:
		return (-1);
	case (uintptr_t)USB_DESC_STRING_SERIALNO:
		return (usb_tx_serialno(reqlen));
	default:
		usb_ep0_tx_cp(*d, (*d)->bLength, reqlen, NULL, NULL);
		return (0);
	}
}


static void usb_handle_control_done(void *data, ssize_t len, void *cbdata)
{
	if (usb.state == USBD_STATE_SETTING_ADDRESS) {
		usb.state = USBD_STATE_ADDRESS;
		usb_set_addr(usb.address);
	}
	usb_setup_control();
}

void usb_handle_control_status_cb(ep_callback_t cb)
{
	/* empty status transfer */
	switch (usb.ctrl_dir) {
	case USB_CTRL_REQ_IN:
		usb.ep_state[0].rx.data01 = USB_DATA01_DATA1;
		usb_rx(&usb.ep_state[0].rx, NULL, 0, cb, NULL);
		break;

	default:
		usb.ep_state[0].tx.data01 = USB_DATA01_DATA1;
		usb_ep0_tx_cp(NULL, 0, 1 /* short packet */, cb, NULL);
		break;
	}
}

void usb_handle_control_status(int fail)
{
	if (fail) {
		usb_pipe_stall(&usb.ep_state[0].rx);
		usb_pipe_stall(&usb.ep_state[0].tx);
	} else {
		usb_handle_control_status_cb(usb_handle_control_done);
	}
}


/**
 * Dispatch non-standard request to registered USB functions.
 */
static void usb_handle_control_nonstd(struct usb_ctrl_req_t *req)
{
	/* XXX filter by interface/endpoint? */
	for (struct usbd_function_ctx_header *fh = &usb.functions; fh != NULL; fh = fh->next) {
		/* ->control() returns != 0 if it handled the request */
		if (fh->function->control != NULL &&
		    fh->function->control(req, fh))
			return;
	}

	usb_handle_control_status(-1);
}


/**
 *
 * Great resource: http://wiki.osdev.org/Universal_Serial_Bus
 *
 * Control Transfers
 * -----------------
 *
 * A control transfer consists of a SETUP transaction (1), zero or
 * more data transactions (IN or OUT) (2), and a final status
 * transaction (3).
 *
 * Token sequence (data toggle):
 * 1.  SETUP (0)
 * (2a. OUT (1) ... (toggling))
 * 3a. IN (1)
 *
 * or
 * 1.  SETUP (0)
 * 2b. IN (1) ... (toggling)
 * 3b. OUT (1)
 *
 * Report errors by STALLing the control EP after (1) or (2), so that
 * (3) will STALL.  Seems we need to clear the STALL after that so
 * that the next SETUP can make it through.
 *
 *
 */

/**
 * The following code is not written defensively, but instead only
 * asserts values that are essential for correct execution.  It
 * accepts a superset of the protocol defined by the standard.  We do
 * this to save space.
 */

static void usb_handle_control(void *data, ssize_t len, void *cbdata)
{
	struct usb_ctrl_req_t *req = data;
	uint16_t zero16 = 0;
	int fail = 1;

	usb.ctrl_dir = req->in;

	if (req->type != USB_CTRL_REQ_STD) {
		usb_handle_control_nonstd(req);
		return;
	}

	/* Only STD requests here */
	switch (req->bRequest) {
	case USB_CTRL_REQ_GET_STATUS:
		/**
		 * Because we don't support remote wakeup or
		 * self-powered operation, and we are specialized to
		 * only EP 0 so far, all GET_STATUS replies are just
		 * empty.
		 */
		usb_ep0_tx_cp(&zero16, sizeof(zero16), req->wLength, NULL, NULL);
		break;

	case USB_CTRL_REQ_CLEAR_FEATURE:
	case USB_CTRL_REQ_SET_FEATURE:
		/**
		 * Nothing to do.  Maybe return STALLs on illegal
		 * accesses?
		 */
		break;

	case USB_CTRL_REQ_SET_ADDRESS:
		/**
		 * We must keep our previous address until the end of
		 * the status stage;  therefore we can't set the
		 * address right now.  Since this is a special case,
		 * the EP 0 handler will take care of this later on.
		 */
		usb.address = req->wValue & 0x7f;
		usb.state = USBD_STATE_SETTING_ADDRESS;
		break;

	case USB_CTRL_REQ_GET_DESCRIPTOR:
		switch (req->wValue >> 8) {
		case USB_DESC_DEV:
			usb_ep0_tx_cp(usb.identity->dev_desc, usb.identity->dev_desc->bLength,
				      req->wLength, NULL, NULL);
			fail = 0;
			break;
		case USB_DESC_CONFIG:
			fail = usb_tx_config_desc(req->wValue & 0xff, req->wLength);
			break;
		case USB_DESC_STRING:
			fail = usb_tx_string_desc(req->wValue & 0xff, req->wLength);
			break;
		default:
			fail = -1;
			break;
		}
		/* we set fail already, so we can go directly to `err' */
		goto err;

	case USB_CTRL_REQ_GET_CONFIGURATION:
		usb_ep0_tx_cp(&usb.config, 1, req->wLength, NULL, NULL); /* XXX implicit LE */
		break;

	case USB_CTRL_REQ_SET_CONFIGURATION:
		if (usb_set_config(req->wValue) < 0)
			goto err;
		break;

	case USB_CTRL_REQ_GET_INTERFACE:
		/* We only support iface setting 0 */
		usb_ep0_tx_cp(&zero16, 1, req->wLength, NULL, NULL);
		break;

	case USB_CTRL_REQ_SET_INTERFACE:
		if (usb_set_interface(req->wIndex, req->wValue) < 0)
			goto err;
		break;

	default:
		goto err;
	}

	fail = 0;

err:
	usb_handle_control_status(fail);
}

void usb_setup_control(void)
{
	void *buf = ep0_buf[usb.ep_state[0].rx.pingpong];

	usb.ep_state[0].rx.data01 = USB_DATA01_DATA0;
	usb.ep_state[0].tx.data01 = USB_DATA01_DATA1;
	usb_rx(&usb.ep_state[0].rx, buf, EP0_BUFSIZE, usb_handle_control, NULL);
}


/**
 * This is called by the interrupt handler
 */
void usb_handle_transaction(struct usb_xfer_info *info)
{
	enum usb_tok_pid pid = usb_get_xfer_pid(info);
	struct usbd_ep_state_t *eps = &usb.ep_state[usb_get_xfer_ep(info)];
	struct usbd_ep_pipe_state_t *s = &eps->pipe[usb_get_xfer_dir(info)];

	switch (pid) {
	case USB_PID_SETUP:
	case USB_PID_OUT:
		/**
		 * If we receive a SETUP transaction, but don't expect
		 * it (callback set to somewhere else), stall the EP.
		 */
		if (pid == USB_PID_SETUP && s->callback != usb_handle_control)
			usb_handle_control_status(1);
		else
			usb_rx_next(s);
		if (pid == USB_PID_SETUP)
			usb_enable_xfers();
		break;
	case USB_PID_IN:
		usb_tx_next(s);
		break;
	default:
		break;
	}
}

struct usbd_ep_pipe_state_t *usb_init_ep(struct usbd_function_ctx_header *ctx, int ep, enum usb_ep_dir dir, size_t size)
{
	struct usbd_ep_pipe_state_t *s;

	if (dir == USB_EP_RX)
		s = &usb.ep_state[ctx->ep_rx_offset + ep].rx;
	else
		s = &usb.ep_state[ctx->ep_tx_offset + ep].tx;

	memset(s, 0, sizeof(*s));
	s->ep_maxsize = size;
	s->ep_num = ep;
	s->ep_dir = dir;
	usb_pipe_enable(s);
	return (s);
}

void usb_restart(void)
{
	const struct usbd_device *identity = usb.identity;
	/* XXX reset existing functions? */
	memset(&usb, 0, sizeof(usb));
	usb.functions.function = &usb.control_function;
	usb.identity = identity;
	usb_init_ep(&usb.functions, 0, USB_EP_RX, EP0_BUFSIZE);
	usb_init_ep(&usb.functions, 0, USB_EP_TX, EP0_BUFSIZE);
	usb_setup_control();
}

void usb_attach_function(const struct usbd_function *function, struct usbd_function_ctx_header *ctx)
{
	/* XXX right now this requires a sequential initialization */
	struct usbd_function_ctx_header *prev = &usb.functions;

	while (prev->next != NULL)
		prev = prev->next;
	ctx->next = NULL;
	ctx->function = function;
	ctx->interface_offset = prev->interface_offset + prev->function->interface_count;
	ctx->ep_rx_offset = prev->ep_rx_offset + prev->function->ep_rx_count;
	ctx->ep_tx_offset = prev->ep_tx_offset + prev->function->ep_tx_count;
	prev->next = ctx;
}

void usb_init(const struct usbd_device *identity)
{
	usb.identity = identity;
	usb_enable();
}

