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

#ifndef __SIM_H
#define __SIM_H

// ----- Local Includes -----

#include "mchck.h"



// ----- Structs -----

struct SIM_t {
	struct SIM_SOPT1_t {
		UNION_STRUCT_START(32);
		uint32_t _rsvd0 : 12;
		enum {
			SIM_RAMSIZE_8KB = 1,
			SIM_RAMSIZE_16KB = 3
		} ramsize : 4;
		uint32_t _rsvd1 : 2;
		enum {
			SIM_OSC32KSEL_SYSTEM = 0,
			SIM_OSC32KSEL_RTC = 2,
			SIM_OSC32KSEL_LPO = 3
		} osc32ksel : 2;
		uint32_t _rsvd2 : 9;
		uint32_t usbvstby : 1;
		uint32_t usbsstby : 1;
		uint32_t usbregen : 1;
		UNION_STRUCT_END;
	} sopt1;
	struct SIM_SOPT1CFG_t {
		UNION_STRUCT_START(32);
		uint32_t _rsvd0 : 24;
		uint32_t urwe : 1;
		uint32_t uvswe : 1;
		uint32_t usswe : 1;
		uint32_t _rsvd1 : 5;
		UNION_STRUCT_END;
	} sopt1cfg;
	uint32_t _pad0[(0x1004 - 0x8) / 4];
	struct SIM_SOPT2_t {
		UNION_STRUCT_START(32);
		uint32_t _rsvd0 : 4;
		enum {
			SIM_RTCCLKOUTSEL_1HZ = 0,
			SIM_RTCCLKOUTSEL_32KHZ = 1
		} rtcclkoutsel : 1;
		enum {
			SIM_CLKOUTSEL_FLASH = 2,
			SIM_CLKOUTSEL_LPO = 3,
			SIM_CLKOUTSEL_MCG = 4,
			SIM_CLKOUTSEL_RTC = 5,
			SIM_CLKOUTSEL_OSC = 6
		} clkoutsel : 3;
		uint32_t _rsvd1 : 3;
		enum {
			SIM_PTD7PAD_SINGLE = 0,
			SIM_PTD7PAD_DOUBLE = 1
		} ptd7pad : 1;
		enum {
			SIM_TRACECLKSEL_MCG = 0,
			SIM_TRACECLKSEL_CORE = 1
		} traceclksel : 1;
		uint32_t _rsvd2 : 3;
		enum {
			SIM_PLLFLLSEL_FLL = 0,
			SIM_PLLFLLSEL_PLL = 1
		} pllfllsel : 1;
		uint32_t _rsvd3 : 1;
		enum {
			SIM_USBSRC_EXTERNAL = 0,
			SIM_USBSRC_MCG = 1
		} usbsrc : 1;
		uint32_t _rsvd4 : 13;
		UNION_STRUCT_END;
	} sopt2;
	uint32_t _pad1;
	struct SIM_SOPT4_t {
		UNION_STRUCT_START(32);
		enum sim_ftmflt {
			SIM_FTMFLT_FTM = 0,
			SIM_FTMFLT_CMP = 1
		} ftm0flt0 : 1;
		enum sim_ftmflt ftm0flt1 : 1;
		uint32_t _rsvd0 : 2;
		enum sim_ftmflt ftm1flt0 : 1;
		uint32_t _rsvd1 : 13;
		enum {
			SIM_FTMCHSRC_FTM = 0,
			SIM_FTMCHSRC_CMP0 = 1,
			SIM_FTMCHSRC_CMP1 = 2,
			SIM_FTMCHSRC_USBSOF = 3
		} ftm1ch0src : 2;
		uint32_t _rsvd2 : 4;
		enum sim_ftmclksel {
			SIM_FTMCLKSEL_CLK0 = 0,
			SIM_FTMCLKSEL_CLK1 = 1
		} ftm0clksel : 1;
		enum sim_ftmclksel ftm1clksel : 1;
		uint32_t _rsvd3 : 2;
		enum {
			SIM_FTMTRGSRC_HSCMP0 = 0,
			SIM_FTMTRGSRC_FTM1 = 1
		} ftm0trg0src : 1;
		uint32_t _rsvd4 : 3;
		UNION_STRUCT_END;
	} sopt4;
	struct SIM_SOPT5_t {
		UNION_STRUCT_START(32);
		enum sim_uarttxsrc {
			SIM_UARTTXSRC_UART = 0,
			SIM_UARTTXSRC_FTM = 1
		} uart0txsrc : 1;
		uint32_t _rsvd0 : 1;
		enum sim_uartrxsrc {
			SIM_UARTRXSRC_UART = 0,
			SIM_UARTRXSRC_CMP0 = 1,
			SIM_UARTRXSRC_CMP1 = 2
		} uart0rxsrc : 2;
		enum sim_uarttxsrc uart1txsrc : 1;
		uint32_t _rsvd1 : 1;
		enum sim_uartrxsrc uart1rxsrc : 2;
		uint32_t _rsvd2 : 24;
		UNION_STRUCT_END;
	} sopt5;
	uint32_t _pad2;
	struct SIM_SOPT7_t {
		UNION_STRUCT_START(32);
		enum {
			SIM_ADCTRGSEL_PDB = 0,
			SIM_ADCTRGSEL_HSCMP0 = 1,
			SIM_ADCTRGSEL_HSCMP1 = 2,
			SIM_ADCTRGSEL_PIT0 = 4,
			SIM_ADCTRGSEL_PIT1 = 5,
			SIM_ADCTRGSEL_PIT2 = 6,
			SIM_ADCTRGSEL_PIT3 = 7,
			SIM_ADCTRGSEL_FTM0 = 8,
			SIM_ADCTRGSEL_FTM1 = 9,
			SIM_ADCTRGSEL_RTCALARM = 12,
			SIM_ADCTRGSEL_RTCSECS = 13,
			SIM_ADCTRGSEL_LPTIMER = 14
		} adc0trgsel : 4;
		enum {
			SIM_ADCPRETRGSEL_A = 0,
			SIM_ADCPRETRGSEL_B = 1
		} adc0pretrgsel : 1;
		uint32_t _rsvd0 : 2;
		enum {
			SIM_ADCALTTRGEN_PDB = 0,
			SIM_ADCALTTRGEN_ALT = 1
		} adc0alttrgen : 1;
		uint32_t _rsvd1 : 24;
		UNION_STRUCT_END;
	} sopt7;
	uint32_t _pad3[(0x1024 - 0x101c) / 4];
	struct SIM_SDID_t {
		UNION_STRUCT_START(32);
		enum {
			SIM_PINID_32 = 2,
			SIM_PINID_48 = 4,
			SIM_PINID_64 = 5
		} pinid : 4;
		enum {
			SIM_FAMID_K10 = 0,
			SIM_FAMID_K20 = 1
		} famid : 3;
		uint32_t _rsvd1 : 5;
		uint32_t revid : 4;
		uint32_t _rsvd2 : 16;
		UNION_STRUCT_END;
	} sdid;
	uint32_t _pad4[(0x1034 - 0x1028) / 4];
	struct SIM_SCGC4_t {
		UNION_STRUCT_START(32);
		uint32_t _rsvd0 : 1;
		uint32_t ewm : 1;
		uint32_t cmt : 1;
		uint32_t _rsvd1 : 3;
		uint32_t i2c0 : 1;
		uint32_t _rsvd2 : 3;
		uint32_t uart0 : 1;
		uint32_t uart1 : 1;
		uint32_t uart2 : 1;
		uint32_t _rsvd3 : 5;
		uint32_t usbotg : 1;
		uint32_t cmp : 1;
		uint32_t vref : 1;
		uint32_t _rsvd4 : 11;
		UNION_STRUCT_END;
	} scgc4;
	struct SIM_SCGC5_t {
		UNION_STRUCT_START(32);
		uint32_t lptimer : 1;
		uint32_t _rsvd0 : 4;
		uint32_t tsi : 1;
		uint32_t _rsvd1 : 3;
		uint32_t porta : 1;
		uint32_t portb : 1;
		uint32_t portc : 1;
		uint32_t portd : 1;
		uint32_t porte : 1;
		uint32_t _rsvd2 : 18;
		UNION_STRUCT_END;
	} scgc5;
	struct SIM_SCGC6_t {
		UNION_STRUCT_START(32);
		uint32_t ftfl : 1;
		uint32_t dmamux : 1;
		uint32_t _rsvd0 : 10;
		uint32_t spi0 : 1;
		uint32_t _rsvd1 : 2;
		uint32_t i2s : 1;
		uint32_t _rsvd2 : 2;
		uint32_t crc : 1;
		uint32_t _rsvd3 : 2;
		uint32_t usbdcd : 1;
		uint32_t pdb : 1;
		uint32_t pit : 1;
		uint32_t ftm0 : 1;
		uint32_t ftm1 : 1;
		uint32_t _rsvd4 : 1;
		uint32_t adc0 : 1;
		uint32_t _rsvd5 : 1;
		uint32_t rtc : 1;
		uint32_t _rsvd6 : 2;
		UNION_STRUCT_END;
	} scgc6;
	struct SIM_SCGC7_t {
		UNION_STRUCT_START(32);
		uint32_t _rsvd0 : 1;
		uint32_t dma : 1;
		uint32_t _rsvd1 : 30;
		UNION_STRUCT_END;
	} scgc7;
	struct SIM_CLKDIV1_t {
		UNION_STRUCT_START(32);
		uint32_t _rsvd0 : 16;
		uint32_t outdiv4 : 4;
		uint32_t _rsvd1 : 4;
		uint32_t outdiv2 : 4;
		uint32_t outdiv1 : 4;
		UNION_STRUCT_END;
	} clkdiv1;
	struct SIM_CLKDIV2_t {
		UNION_STRUCT_START(32);
		uint32_t usbfrac : 1;
		uint32_t usbdiv : 3;
		uint32_t _rsvd0 : 28;
		UNION_STRUCT_END;
	} clkdiv2;
	struct SIM_FCFG1_t {
		UNION_STRUCT_START(32);
		uint32_t flashdis : 1;
		uint32_t flashdoze : 1;
		uint32_t _rsvd0 : 6;

		/* the following enum is analogous to enum
		 * FTFL_FLEXNVM_PARTITION in ftfl.h, but that one is padded
		 * with four 1-bits to make an 8-bit value.
		 */
		enum SIM_FLEXNVM_PARTITION {
			SIM_FLEXNVM_DATA_32_EEPROM_0  = 0x0,
			SIM_FLEXNVM_DATA_24_EEPROM_8  = 0x1,
			SIM_FLEXNVM_DATA_16_EEPROM_16 = 0x2,
			SIM_FLEXNVM_DATA_8_EEPROM_24  = 0x9,
			SIM_FLEXNVM_DATA_0_EEPROM_32  = 0x3
		} depart : 4;

		uint32_t _rsvd1 : 4;
		enum {
			SIM_EESIZE_2KB = 3,
			SIM_EESIZE_1KB = 4,
			SIM_EESIZE_512B = 5,
			SIM_EESIZE_256B = 6,
			SIM_EESIZE_128B = 7,
			SIM_EESIZE_64B = 8,
			SIM_EESIZE_32B = 9,
			SIM_EESIZE_0B = 15
		} eesize : 4;
		uint32_t _rsvd2 : 4;
		enum {
			SIM_PFSIZE_32KB = 3,
			SIM_PFSIZE_64KB = 5,
			SIM_PFSIZE_128KB = 7
		} pfsize : 4;
		enum {
			SIM_NVMSIZE_0KB = 0,
			SIM_NVMSIZE_32KB = 3
		} nvmsize : 4;
		UNION_STRUCT_END;
	} fcfg1;
	struct SIM_FCFG2_t {
		UNION_STRUCT_START(32);
		uint32_t _rsvd0 : 16;
		uint32_t maxaddr1 : 7;
		enum {
			SIM_PFLSH_FLEXNVM = 0,
			SIM_PFLSH_PROGRAM = 1
		} pflsh : 1;
		uint32_t maxaddr0 : 7;
		uint32_t _rsvd1 : 1;
		UNION_STRUCT_END;
	} fcfg2;
	uint32_t uidh;
	uint32_t uidmh;
	uint32_t uidml;
	uint32_t uidl;
};
CTASSERT_SIZE_BYTE(struct SIM_t, 0x1064);

extern volatile struct SIM_t SIM;

#endif

