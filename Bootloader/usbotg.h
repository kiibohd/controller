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

// ----- Local Includes -----

#include "mchck.h"
#include "usb-common.h"



// ----- Structs -----

/**
 * Hardware structures
 */

struct USB_ADDINFO_t {
	UNION_STRUCT_START(8);
	uint8_t iehost : 1;
	uint8_t _rsvd0 : 2;
	uint8_t irqnum : 5;
	UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct USB_ADDINFO_t, 8);

struct USB_OTGSTAT_t {
	UNION_STRUCT_START(8);
	uint8_t avbus : 1;
	uint8_t _rsvd0 : 1;
	uint8_t b_sess : 1;
	uint8_t sessvld : 1;
	uint8_t _rsvd1 : 1;
	uint8_t line_state : 1;
	uint8_t onemsec : 1;
	uint8_t idchg : 1;
	UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct USB_OTGSTAT_t, 8);

struct USB_OTGCTL_t {
	UNION_STRUCT_START(8);
	uint8_t _rsvd0 : 2;
	uint8_t otgen : 1;
	uint8_t _rsvd1 : 1;
	uint8_t dmlow : 1;
	uint8_t dplow : 1;
	uint8_t _rsvd2 : 1;
	uint8_t dphigh : 1;
	UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct USB_OTGCTL_t, 8);

struct USB_ISTAT_t {
	UNION_STRUCT_START(8);
	uint8_t usbrst : 1;
	uint8_t error : 1;
	uint8_t softok : 1;
	uint8_t tokdne : 1;
	uint8_t sleep : 1;
	uint8_t resume : 1;
	uint8_t attach : 1;
	uint8_t stall : 1;
	UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct USB_ISTAT_t, 8);

struct USB_ERRSTAT_t {
	UNION_STRUCT_START(8);
	uint8_t piderr : 1;
	uint8_t crc5eof : 1;
	uint8_t crc16 : 1;
	uint8_t dfn8 : 1;
	uint8_t btoerr : 1;
	uint8_t dmaerr : 1;
	uint8_t _rsvd0 : 1;
	uint8_t btserr : 1;
	UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct USB_ERRSTAT_t, 8);

struct USB_STAT_t {
	UNION_STRUCT_START(8);
	uint8_t _rsvd0 : 2;
	enum usb_ep_pingpong pingpong : 1;
	enum usb_ep_dir dir : 1;
	uint8_t ep : 4;
	UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct USB_STAT_t, 8);

struct USB_CTL_t {
	union {
		struct /* common */ {
			uint8_t _rsvd1 : 1;
			uint8_t oddrst : 1;
			uint8_t resume : 1;
			uint8_t _rsvd2 : 3;
			uint8_t se0 : 1;
			uint8_t jstate : 1;
		};
		struct /* host */ {
			uint8_t sofen : 1;
			uint8_t _rsvd3 : 2;
			uint8_t hostmodeen : 1;
			uint8_t reset : 1;
			uint8_t token_busy : 1;
			uint8_t _rsvd4 : 2;
		};
		struct /* device */ {
			uint8_t usben : 1;
			uint8_t _rsvd5 : 4;
			uint8_t txd_suspend : 1;
			uint8_t _rsvd6 : 2;
		};
		uint8_t raw;
	};
};
CTASSERT_SIZE_BIT(struct USB_CTL_t, 8);

struct USB_ADDR_t {
	UNION_STRUCT_START(8);
	uint8_t addr : 7;
	uint8_t lsen : 1;
	UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct USB_ADDR_t, 8);

struct USB_TOKEN_t {
	UNION_STRUCT_START(8);
	uint8_t endpt : 4;
	enum usb_tok_pid pid : 4;
	UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct USB_TOKEN_t, 8);

struct USB_ENDPT_t {
	UNION_STRUCT_START(8);
	uint8_t ephshk : 1;
	uint8_t epstall : 1;
	uint8_t eptxen : 1;
	uint8_t eprxen : 1;
	uint8_t epctldis : 1;
	uint8_t _rsvd0 : 1;
	uint8_t retrydis : 1;
	uint8_t hostwohub : 1;
	UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct USB_ENDPT_t, 8);

struct USB_USBCTRL_t {
	UNION_STRUCT_START(8);
	uint8_t _rsvd0 : 6;
	uint8_t pde : 1;
	uint8_t susp : 1;
	UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct USB_USBCTRL_t, 8);

struct USB_OBSERVE_t {
	UNION_STRUCT_START(8);
	uint8_t _rsvd0 : 4;
	uint8_t dmpd : 1;
	uint8_t _rsvd1 : 1;
	uint8_t dppd : 1;
	uint8_t dppu : 1;
	UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct USB_OBSERVE_t, 8);

struct USB_CONTROL_t {
	UNION_STRUCT_START(8);
	uint8_t _rsvd0 : 4;
	uint8_t dppullupnonotg : 1;
	uint8_t _rsvd1 : 3;
	UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct USB_CONTROL_t, 8);

struct USB_USBTRC0_t {
	UNION_STRUCT_START(8);
	uint8_t usb_resume_int : 1;
	uint8_t sync_det : 1;
	uint8_t _rsvd0 : 3;
	uint8_t usbresmen : 1;
	uint8_t _rsvd1 : 1;
	uint8_t usbreset : 1;
	UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct USB_USBTRC0_t, 8);

struct USB_t {
	uint8_t perid;
	uint8_t _pad0[3];
	uint8_t idcomp;
	uint8_t _pad1[3];
	uint8_t rev;
	uint8_t _pad2[3];
	struct USB_ADDINFO_t addinfo;
	uint8_t _pad3[3];
	struct USB_OTGSTAT_t otgistat;
	uint8_t _pad4[3];
	struct USB_OTGSTAT_t otgicr;
	uint8_t _pad5[3];
	struct USB_OTGSTAT_t otgstat;
	uint8_t _pad6[3];
	struct USB_OTGCTL_t otgctl;
	uint8_t _pad7[3];
	uint8_t _pad8[0x80 - 0x20];
	struct USB_ISTAT_t istat;
	uint8_t _pad9[3];
	struct USB_ISTAT_t inten;
	uint8_t _pad10[3];
	struct USB_ERRSTAT_t errstat;
	uint8_t _pad11[3];
	struct USB_ERRSTAT_t erren;
	uint8_t _pad12[3];
	struct USB_STAT_t stat;
	uint8_t _pad13[3];
	struct USB_CTL_t ctl;
	uint8_t _pad14[3];
	struct USB_ADDR_t addr;
	uint8_t _pad15[3];
	uint8_t bdtpage1;
	uint8_t _pad16[3];
	uint8_t frmnuml;
	uint8_t _pad17[3];
	struct {
		uint8_t frmnumh : 3;
		uint8_t _rsvd0 : 5;
	};
	uint8_t _pad18[3];
	struct USB_TOKEN_t token;
	uint8_t _pad19[3];
	uint8_t softhld;
	uint8_t _pad20[3];
	uint8_t bdtpage2;
	uint8_t _pad21[3];
	uint8_t bdtpage3;
	uint8_t _pad22[3];
	uint8_t _pad23[0xc0 - 0xb8];
	struct {
		struct USB_ENDPT_t;
		uint8_t _pad24[3];
	} endpt[16];
	struct USB_USBCTRL_t usbctrl;
	uint8_t _pad25[3];
	struct USB_OBSERVE_t observe;
	uint8_t _pad26[3];
	struct USB_CONTROL_t control;
	uint8_t _pad27[3];
	struct USB_USBTRC0_t usbtrc0;
	uint8_t _pad28[3];
	uint8_t _pad29[4];
	uint8_t usbfrmadjust;
	uint8_t _pad30[3];
};
CTASSERT_SIZE_BYTE(struct USB_t, 0x118);

struct USB_BD_t {
	struct USB_BD_BITS_t {
		union {
			struct {
				uint32_t _rsvd0  : 2;
				uint32_t stall   : 1;
				uint32_t dts     : 1;
				uint32_t ninc    : 1;
				uint32_t keep    : 1;
				enum usb_data01 data01 : 1;
				uint32_t own     : 1;
				uint32_t _rsvd1  : 8;
				uint32_t bc      : 10;
				uint32_t _rsvd2  : 6;
			};
			struct /* processor */ {
				uint32_t _rsvd5  : 2;
				enum usb_tok_pid tok_pid : 4;
				uint32_t _rsvd6  : 26;
			};
			uint32_t raw;
		};
	};
	void *addr;
};
CTASSERT_SIZE_BYTE(struct USB_BD_t, 8);

extern volatile struct USB_t USB0;

