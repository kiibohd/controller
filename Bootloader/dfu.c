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

// ----- Local Includes -----

#include "usb.h"
#include "dfu.h"
#include "debug.h"



// ----- Functions -----

void dfu_write_done( enum dfu_status err, struct dfu_ctx *ctx )
{
	ctx->status = err;
	if (ctx->status == DFU_STATUS_OK) {
		switch (ctx->state) {
		case DFU_STATE_dfuDNBUSY:
			ctx->state = DFU_STATE_dfuDNLOAD_IDLE;
			break;
		default:
			break;
		}
	} else {
		ctx->state = DFU_STATE_dfuERROR;
	}
}

static void dfu_dnload_complete( void *buf, ssize_t len, void *cbdata )
{
	struct dfu_ctx *ctx = cbdata;

	if (len > 0)
		ctx->state = DFU_STATE_dfuDNBUSY;
	else
		ctx->state = DFU_STATE_dfuMANIFEST;
	ctx->status = ctx->finish_write(buf, ctx->off, len);
	ctx->off += len;
	ctx->len = len;

	if (ctx->status != DFU_STATUS_async)
		dfu_write_done(ctx->status, ctx);

	usb_handle_control_status(ctx->state == DFU_STATE_dfuERROR);
}

static void dfu_reset_system( void *buf, ssize_t len, void *cbdata )
{
	SOFTWARE_RESET();
}

static int dfu_handle_control( struct usb_ctrl_req_t *req, void *data )
{
	struct dfu_ctx *ctx = data;
	int fail = 1;

	switch ((enum dfu_ctrl_req_code)req->bRequest)
	{
	case USB_CTRL_REQ_DFU_DNLOAD: {
		void *buf;

		switch (ctx->state) {
		case DFU_STATE_dfuIDLE:
			ctx->off = 0;
			break;
		case DFU_STATE_dfuDNLOAD_IDLE:
			break;
		default:
			goto err;
		}

		/**
		 * XXX we are not allowed to STALL here, and we need to eat all transferred data.
		 * better not allow setup_write to break the protocol.
		 */
		ctx->status = ctx->setup_write(ctx->off, req->wLength, &buf);
		if (ctx->status != DFU_STATUS_OK) {
			ctx->state = DFU_STATE_dfuERROR;
			goto err_have_status;
		}

		if (req->wLength > 0)
			usb_ep0_rx(buf, req->wLength, dfu_dnload_complete, ctx);
		else
			dfu_dnload_complete(NULL, 0, ctx);
		goto out_no_status;
	}
	case USB_CTRL_REQ_DFU_UPLOAD: {
		void *buf;
		size_t len = 0;

		switch ( ctx->state )
		{
		case DFU_STATE_dfuIDLE:
			break;
		case DFU_STATE_dfuUPLOAD_IDLE:
			break;
		default:
			goto err;
		}

		// Find which sector to read
		ctx->status = ctx->setup_read(ctx->off, &len, &buf);
		print("UPLOAD off:");
		printHex( ctx->off );
		print(" len:");
		printHex( len );
		print(" addr:");
		printHex( (uint32_t)buf );
		print( NL );

		if ( ctx->status != DFU_STATUS_OK || len > req->wLength )
		{
			ctx->state = DFU_STATE_dfuERROR;
			goto err_have_status;
		}

		// Send bytes to Host
		if ( len > 0 )
		{
			usb_ep0_rx( buf, len, NULL, NULL );
		}
		else
		{
			ctx->state = DFU_STATE_dfuIDLE;
		}

		goto out_no_status;
	}
	case USB_CTRL_REQ_DFU_GETSTATUS: {
		struct dfu_status_t st;

		st.bState = ctx->state;
		st.bStatus = ctx->status;
		st.bwPollTimeout = 1000; /* XXX */

		// XXX FAKE WRITE
		if ( ctx->state == DFU_STATE_dfuMANIFEST )
		{
			uint8_t data[] = { 0x10, 0x20, 0x30, 0x40 };
			flash_program_longword((uintptr_t)&_app_rom, data);
		}
		/*

			uint32_t *position = &_app_rom + 0x100;
		for ( ; position < &_app_rom + 0x200; position++ )
		//for ( ; position < &_app_rom + 0x800; position++ )
		{
			if ( *position != 0xFFFFFFFF )
			{
			while( 1 )
			{
				GPIOA_PTOR |= (1<<5);
				for (uint32_t d = 0; d < 7200000; d++ );
			}
			}
		}*/

		// Check to see if vector table was flashed correctly
		// Return a flash error if it was not
		if (_app_rom == 0xffffffff && ctx->state == DFU_STATE_dfuMANIFEST)
			st.bStatus = DFU_STATUS_errPROG;
		//}
		/*
		if (ctx->state == DFU_STATE_dfuMANIFEST)
		{
			uint8_t *addr = (uint8_t*)_app_rom;
			//while (*(addr++) != 0x80);
			//st.bStatus = DFU_STATUS_errPROG;
			st.bStatus = (uint8_t)((uint32_t)(&_app_rom) >> 16);
		}
		*/

		/**
		 * If we're in DFU_STATE_dfuMANIFEST, we just finished
		 * the download, and we're just about to send our last
		 * status report.  Once the report has been sent, go
		 * and reset the system to put the new firmware into
		 * effect.
		 */
		usb_ep0_tx_cp(&st, sizeof(st), req->wLength, NULL, NULL);
		if (ctx->state == DFU_STATE_dfuMANIFEST) {
			usb_handle_control_status_cb(dfu_reset_system);
			goto out_no_status;
		}
		break;
	}
	case USB_CTRL_REQ_DFU_CLRSTATUS:
		ctx->state = DFU_STATE_dfuIDLE;
		ctx->status = DFU_STATUS_OK;
		break;
	case USB_CTRL_REQ_DFU_GETSTATE: {
		uint8_t st = ctx->state;
		usb_ep0_tx_cp(&st, sizeof(st), req->wLength, NULL, NULL);
		break;
	}
	case USB_CTRL_REQ_DFU_ABORT:
		switch (ctx->state) {
		case DFU_STATE_dfuIDLE:
		case DFU_STATE_dfuDNLOAD_IDLE:
		case DFU_STATE_dfuUPLOAD_IDLE:
			ctx->state = DFU_STATE_dfuIDLE;
			break;
		default:
			goto err;
		}
		break;
	default:
		return (0);
	}

	fail = 0;
	goto out;

err:
	ctx->status = DFU_STATUS_errSTALLEDPKT;
err_have_status:
	ctx->state = DFU_STATE_dfuERROR;
out:
	usb_handle_control_status(fail);
out_no_status:
	return (1);
}

void dfu_init( dfu_setup_read_t setup_read, dfu_setup_write_t setup_write, dfu_finish_write_t finish_write, struct dfu_ctx *ctx )
{
	ctx->state = DFU_STATE_dfuIDLE;
	ctx->setup_read = setup_read;
	ctx->setup_write = setup_write;
	ctx->finish_write = finish_write;
	usb_attach_function(&dfu_function, &ctx->header);
}

const struct usbd_function dfu_function = {
	.control = dfu_handle_control,
	.interface_count = USB_FUNCTION_DFU_IFACE_COUNT,
};

