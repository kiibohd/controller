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

// ----- Defines -----

#define usb_xfer_info USB_STAT_t



// ----- Local Includes -----

#include "mchck.h"
#include "usb-internal.h"



// ----- Functions -----

/**
 * Kinetis USB driver notes:
 * We need to manually maintain the DATA0/1 toggling for the SIE.
 * SETUP transactions always start with a DATA0.
 *
 * The SIE internally uses pingpong (double) buffering, which is
 * easily confused with DATA0/DATA1 toggling, and I think even the
 * Freescale docs confuse the two notions.  When BD->DTS is set,
 * BD->DATA01 will be used to verify/discard incoming DATAx and it
 * will set the DATAx PID for outgoing tokens.  This is not described
 * as such in the Freescale Kinetis docs, but the Microchip PIC32 OTG
 * docs are more clear on this;  it seems that both Freescale and
 * Microchip use different versions of the same USB OTG IP core.
 *
 * http://ww1.microchip.com/downloads/en/DeviceDoc/61126F.pdf
 *
 * Clear CTL->TOKEN_BUSY after SETUP tokens.
 */


static struct USB_BD_t bdt[USB_MAX_EP * 2 *2] __attribute__((section(".usbdescriptortable")));

static struct USB_BD_t *
usb_get_bd(struct usbd_ep_pipe_state_t *s)
{
	return (&bdt[(s->ep_num << 2) | (s->ep_dir << 1) | s->pingpong]);
}

static struct USB_BD_t *
usb_get_bd_stat(struct USB_STAT_t *stat)
{
	return (((void *)(uintptr_t)bdt + (stat->raw << 1)));
}

void *usb_get_xfer_data(struct usb_xfer_info *i)
{
	return (usb_get_bd_stat(i)->addr);
}

enum usb_tok_pid usb_get_xfer_pid(struct usb_xfer_info *i)
{
	return (usb_get_bd_stat(i)->tok_pid);
}

int usb_get_xfer_ep(struct usb_xfer_info *i)
{
	return (i->ep);
}

enum usb_ep_dir usb_get_xfer_dir(struct usb_xfer_info *i)
{
	return (i->dir);
}

void usb_enable_xfers(void)
{
	USB0.ctl.raw = ((struct USB_CTL_t){
			.txd_suspend = 0,
				.usben = 1
				}).raw;
}

void usb_set_addr(int addr)
{
	USB0.addr.raw = addr;
}


void usb_pipe_stall(struct usbd_ep_pipe_state_t *s)
{
	volatile struct USB_BD_t *bd = usb_get_bd(s);
	bd->raw = ((struct USB_BD_BITS_t){
			.stall = 1,
				.own = 1
				}).raw;
}

void usb_pipe_unstall(struct usbd_ep_pipe_state_t *s)
{
	volatile struct USB_BD_t *bd = usb_get_bd(s);
	struct USB_BD_BITS_t state = { .raw = bd->raw };

	if (state.own && state.stall)
		bd->raw = 0;
}

void usb_pipe_enable(struct usbd_ep_pipe_state_t *s)
{
	USB0.endpt[s->ep_num].raw |= ((struct USB_ENDPT_t){
			.eptxen = s->ep_dir == USB_EP_TX,
				.eprxen = s->ep_dir == USB_EP_RX,
				.ephshk = 1, /* XXX ISO */
				.epctldis = s->ep_num != 0
				}).raw;
}

void usb_pipe_disable(struct usbd_ep_pipe_state_t *s)
{
	USB0.endpt[s->ep_num].raw &= ~((struct USB_ENDPT_t){
			.eptxen = s->ep_dir == USB_EP_TX,
				.eprxen = s->ep_dir == USB_EP_RX,
				.epctldis = 1
				}).raw;
}

size_t usb_ep_get_transfer_size(struct usbd_ep_pipe_state_t *s)
{
	struct USB_BD_t *bd = usb_get_bd(s);
	return (bd->bc);
}

void usb_queue_next(struct usbd_ep_pipe_state_t *s, void *addr, size_t len)
{
	volatile struct USB_BD_t *bd = usb_get_bd(s);

	bd->addr = addr;
	/* damn you bitfield problems */
	bd->raw = ((struct USB_BD_BITS_t){
			.dts = 1,
				.own = 1,
				.data01 = s->data01,
				.bc = len,
				}).raw;
}

static void usb_reset(void)
{
	/* reset pingpong state */
	/* For some obscure reason, we need to use or here. */
	USB0.ctl.raw |= ((struct USB_CTL_t){
			.txd_suspend = 1,
				.oddrst = 1,
				}).raw;

	/* clear all interrupt bits - not sure if needed */
	USB0.istat.raw = 0xff;
	USB0.errstat.raw = 0xff;
	USB0.otgistat.raw = 0xff;

	/* zap also BDT pingpong & queued transactions */
	memset(bdt, 0, sizeof(bdt));
	USB0.addr.raw = 0;

	usb_restart();

	USB0.ctl.raw = ((struct USB_CTL_t){
				.txd_suspend = 0,
				.usben = 1
				}).raw;

	/* we're only interested in reset and transfers */
	USB0.inten.raw = ((struct USB_ISTAT_t){
				.tokdne = 1,
				.usbrst = 1,
				.stall = 1,
				.sleep = 1,
				}).raw;

	USB0.usbtrc0.usbresmen = 0;
	USB0.usbctrl.susp = 0;
}

void usb_enable(void)
{
	SIM.sopt2.usbsrc = 1;   /* usb from mcg */
	SIM.scgc4.usbotg = 1;   /* enable usb clock */

	/* reset module - not sure if needed */
	USB0.usbtrc0.raw = ((struct USB_USBTRC0_t){
				.usbreset = 1,
				.usbresmen = 1
				}).raw;
	while (USB0.usbtrc0.usbreset)
		/* NOTHING */;

	USB0.bdtpage1 = (uintptr_t)bdt >> 8;
	USB0.bdtpage2 = (uintptr_t)bdt >> 16;
	USB0.bdtpage3 = (uintptr_t)bdt >> 24;

	USB0.control.raw = ((struct USB_CONTROL_t){
				.dppullupnonotg = 1 /* enable pullup */
				}).raw;

	USB0.usbctrl.raw = 0; /* resume peripheral & disable pulldowns */
	usb_reset();          /* this will start usb processing */

	/* really only one thing we want */
	USB0.inten.raw = ((struct USB_ISTAT_t){
				.usbrst = 1,
				}).raw;

	/**
	 * Suspend transceiver now - we'll wake up at reset again.
	 */
	// TODO - Possible removal
	USB0.usbctrl.susp = 1;
	USB0.usbtrc0.usbresmen = 1;
}

void USB0_Handler(void)
{
	struct USB_ISTAT_t stat = {.raw = USB0.istat.raw };

	if (stat.usbrst) {
		usb_reset();
		return;
	}
	if (stat.stall) {
		/* XXX need more work for non-0 ep */
		volatile struct USB_BD_t *bd = usb_get_bd(&usb.ep_state[0].rx);
		if (bd->stall)
			usb_setup_control();
	}
	if (stat.tokdne) {
		struct usb_xfer_info stat = USB0.stat;
		usb_handle_transaction(&stat);
	}
	if (stat.sleep) {
		USB0.inten.sleep = 0;
		USB0.inten.resume = 1;
		USB0.usbctrl.susp = 1;
		USB0.usbtrc0.usbresmen = 1;

		/**
		 * Clear interrupts now so that we can detect a fresh
		 * resume later on.
		 */
		USB0.istat.raw = stat.raw;

		const struct usbd_config *c = usb_get_config_data(-1);
		if (c && c->suspend)
			c->suspend();
	}
	/**
	 * XXX it is unclear whether we will receive a synchronous
	 * resume interrupt if we were in sleep.  This code assumes we
	 * do.
	 */
	if (stat.resume || USB0.usbtrc0.usb_resume_int) {
		USB0.inten.resume = 0;
		USB0.inten.sleep = 1;
		USB0.usbtrc0.usbresmen = 0;
		USB0.usbctrl.susp = 0;

		const struct usbd_config *c = usb_get_config_data(-1);
		if (c && c->resume)
			c->resume();

		stat.resume = 1; /* always clear bit */
	}
	USB0.istat.raw = stat.raw;
}

void usb_poll(void)
{
	USB0_Handler();
}

int usb_tx_serialno(size_t reqlen)
{
	struct usb_desc_string_t *d;
	const size_t nregs = 3;
	/**
	 * actually 4, but UIDH is 0xffffffff.  Also our output buffer
	 * is only 64 bytes, and 128 bit + desc header exceeds this by
	 * 2 bytes.
	 */
	const size_t len = nregs * 4 * 2 * 2 + sizeof(*d);

	d = usb_ep0_tx_inplace_prepare(len);

	if (d == NULL)
		return (-1);

	d->bLength = len;
	d->bDescriptorType = USB_DESC_STRING;

	size_t bufpos = 0;
	for (size_t reg = 0; reg < nregs; ++reg) {
		/* registers run MSW first */
		uint32_t val = (&SIM.uidmh)[reg];

		for (size_t bits = 32; bits > 0; bits -= 4, val <<= 4) {
			int nibble = val >> 28;

			if (nibble > 9)
				nibble += 'a' - '9' - 1;
			((char16_t *)d->bString)[bufpos++] = nibble + '0';
		}
	}
	usb_ep0_tx(d, len, reqlen, NULL, NULL);
	return (0);
}

