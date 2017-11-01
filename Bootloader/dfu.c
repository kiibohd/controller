/* Copyright (c) 2011,2012 Simon Schubert <2@0x2c.org>.
 * Modifications by Jacob Alexander 2014-2017 <haata@kiibohd.com>
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
	if ( ctx->status == DFU_STATUS_OK )
	{
		switch ( ctx->state )
		{
		case DFU_STATE_dfuDNBUSY:
			ctx->state = DFU_STATE_dfuDNLOAD_IDLE;
			break;
		default:
			break;
		}
	} else
	{
		ctx->state = DFU_STATE_dfuERROR;
	}
}

static void dfu_dnload_complete( void *buf, ssize_t len, void *cbdata )
{
	struct dfu_ctx *ctx = cbdata;

	if ( len > 0 )
	{
		ctx->state = DFU_STATE_dfuDNBUSY;
	}
	else
	{
		ctx->state = DFU_STATE_dfuMANIFEST;
	}
	ctx->status = ctx->finish_write( buf, ctx->off, len );

	// If this is the first block (and was used for key validation), don't increment offset
	switch ( ctx->verified )
	{
	case DFU_VALIDATION_PENDING:
		ctx->verified = DFU_VALIDATION_OK;
		break;
	case DFU_VALIDATION_OK:
		ctx->off += len;
		break;
	default:
		break;
	}
	ctx->len = len;

	if ( ctx->status != DFU_STATUS_async )
	{
		dfu_write_done( ctx->status, ctx );
	}

	usb_handle_control_status( ctx->state == DFU_STATE_dfuERROR );

	// If we failed validation, reset
	if ( ctx->verified == DFU_VALIDATION_FAILED )
	{
		SOFTWARE_RESET();
	}
}

static void dfu_reset_system( void *buf, ssize_t len, void *cbdata )
{
	SOFTWARE_RESET();
}

static int dfu_handle_control( struct usb_ctrl_req_t *req, void *data )
{
	struct dfu_ctx *ctx = data;
	int fail = 1;

	switch ( (enum dfu_ctrl_req_code)req->bRequest )
	{
	// On Detach, just reset MCU and (attempt to) boot to firmware
	case USB_CTRL_REQ_DFU_DETACH:
		ctx->state = DFU_STATE_dfuIDLE;
		usb_handle_control_status_cb(dfu_reset_system);
		goto out_no_status;

	case USB_CTRL_REQ_DFU_DNLOAD:
	{
		void *buf;

		switch ( ctx->state )
		{
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
		ctx->status = ctx->setup_write( ctx->off, req->wLength, &buf );
		if ( ctx->status != DFU_STATUS_OK )
		{
			ctx->state = DFU_STATE_dfuERROR;
			goto err_have_status;
		}

		if ( req->wLength > 0 )
		{
			usb_ep0_rx( buf, req->wLength, dfu_dnload_complete, ctx );
		}
		else
		{
			dfu_dnload_complete( NULL, 0, ctx );
		}
		goto out_no_status;
	}
	case USB_CTRL_REQ_DFU_UPLOAD:
	{
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

		// Compute the requested offset
		ctx->off = USB_DFU_TRANSFER_SIZE * req->wValue;

		// Find which sector to read
		ctx->status = ctx->setup_read( ctx->off, &len, &buf );
#ifdef FLASH_DEBUG
		print("UPLOAD req:");
		printHex( req->wValue );
		print(" off:");
		printHex( ctx->off );
		print(" len:");
		printHex( len );
		print(" reqlen: ");
		printHex( req->wLength );
		print(" addr:");
		printHex( (uint32_t)buf );
#endif

		if ( ctx->status != DFU_STATUS_OK || len > req->wLength )
		{
			ctx->state = DFU_STATE_dfuERROR;
			goto err_have_status;
		}

		// Send bytes to Host
		// Successfully transferred data to USB
		if ( usb_ep0_tx( buf, len, len, NULL, NULL ) != -1 )
		{
			ctx->state = len < req->wLength
				? DFU_STATE_dfuIDLE
				: DFU_STATE_dfuUPLOAD_IDLE;
			fail = 0;
		}
		// Problem transferring via USB
		else
		{
			ctx->state = DFU_STATE_dfuERROR;
		}
#ifdef FLASH_DEBUG
		print(" state:");
		printHex( ctx->state );
		print( NL );
#endif
		goto out;
	}
	case USB_CTRL_REQ_DFU_GETSTATUS:
	{
		struct dfu_status_t st;

		st.bState = ctx->state;
		st.bStatus = ctx->status;
		st.bwPollTimeout = 1000; /* XXX */

		/**
		 * If we're in DFU_STATE_dfuMANIFEST, we just finished
		 * the download, and we're just about to send our last
		 * status report.  Once the report has been sent, go
		 * and reset the system to put the new firmware into
		 * effect.
		 */
		usb_ep0_tx_cp( &st, sizeof(st), req->wLength, NULL, NULL );
		switch ( ctx->state )
		{
		case DFU_STATE_dfuMANIFEST:
			ctx->state = DFU_STATE_dfuMANIFEST_WAIT_RESET;
			break;
		case DFU_STATE_dfuMANIFEST_WAIT_RESET:
			ctx->state = DFU_STATE_dfuIDLE;
			usb_handle_control_status_cb(dfu_reset_system);
			goto out_no_status;
		default:
			break;
		}
		break;
	}
	case USB_CTRL_REQ_DFU_CLRSTATUS:
		ctx->state = DFU_STATE_dfuIDLE;
		ctx->status = DFU_STATUS_OK;
		break;

	case USB_CTRL_REQ_DFU_GETSTATE:
	{
		uint8_t st = ctx->state;
		usb_ep0_tx_cp( &st, sizeof(st), req->wLength, NULL, NULL );
		break;
	}
	case USB_CTRL_REQ_DFU_ABORT:
		switch ( ctx->state )
		{
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
		return 0;
	}

	fail = 0;
	goto out;

err:
	ctx->status = DFU_STATUS_errSTALLEDPKT;
err_have_status:
	ctx->state = DFU_STATE_dfuERROR;
out:
	usb_handle_control_status( fail );
out_no_status:
	return 1;
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

