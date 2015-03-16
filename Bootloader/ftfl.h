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

#ifndef __FTFL_H
#define __FTFL_H

// ----- Local Includes -----

#include "mchck-cdefs.h"



// ----- Structs -----

struct FTFL_FSTAT_t {
	UNION_STRUCT_START(8);
	uint8_t mgstat0 : 1;
	uint8_t _rsvd0 : 3;
	uint8_t fpviol : 1;
	uint8_t accerr : 1;
	uint8_t rdcolerr : 1;
	uint8_t ccif : 1;
	UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct FTFL_FSTAT_t, 8);

struct FTFL_FCNFG_t {
	UNION_STRUCT_START(8);
	uint8_t eeerdy : 1;
	uint8_t ramrdy : 1;
	uint8_t pflsh : 1;
	uint8_t _rsvd0 : 1;
	uint8_t erssusp : 1;
	uint8_t ersareq : 1;
	uint8_t rdcollie : 1;
	uint8_t ccie : 1;
	UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct FTFL_FCNFG_t, 8);

struct FTFL_FSEC_t {
	UNION_STRUCT_START(8);
	enum {
		FTFL_FSEC_SEC_UNSECURE = 2,
		FTFL_FSEC_SEC_SECURE = 3
	} sec : 2;
	enum {
		FTFL_FSEC_FSLACC_DENY = 1,
		FTFL_FSEC_FSLACC_GRANT = 3
	} fslacc : 2;
	enum {
		FTFL_FSEC_MEEN_DISABLE = 2,
		FTFL_FSEC_MEEN_ENABLE = 3
	} meen : 2;
	enum {
		FTFL_FSEC_KEYEN_DISABLE = 1,
		FTFL_FSEC_KEYEN_ENABLE = 2
	} keyen : 2;
	UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct FTFL_FSEC_t, 8);

struct FTFL_FOPT_t {
	UNION_STRUCT_START(8);
	uint8_t lpboot : 1;
	uint8_t ezport_dis : 1;
	uint8_t nmi_dis : 1;
	uint8_t _rsvd0 : 5;
	UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct FTFL_FOPT_t, 8);

/**
 * The FCOOB is a weird register file, because it is double big endian,
 * which makes for odd gaps and for some data that is big endian, and for
 * some that is little endian.
 */
union FTFL_FCCOB_t {
	struct ftfl_generic {
		uint32_t addr : 24;
		enum FTFL_FCMD {
			FTFL_FCMD_READ_1s_BLOCK = 0x00,
			FTFL_FCMD_READ_1s_SECTION = 0x01,
			FTFL_FCMD_PROGRAM_CHECK = 0x02,
			FTFL_FCMD_READ_RESOURCE = 0x03,
			FTFL_FCMD_PROGRAM_LONGWORD = 0x06,
			FTFL_FCMD_ERASE_BLOCK = 0x08,
			FTFL_FCMD_ERASE_SECTOR = 0x09,
			FTFL_FCMD_PROGRAM_SECTION = 0x0b,
			FTFL_FCMD_READ_1s_ALL_BLOCKS = 0x40,
			FTFL_FCMD_READ_ONCE = 0x41,
			FTFL_FCMD_PROGRAM_ONCE = 0x43,
			FTFL_FCMD_ERASE_ALL_BLOCKS = 0x44,
			FTFL_FCMD_VERIFY_KEY = 0x45,
			FTFL_FCMD_PROGRAM_PARTITION = 0x80,
			FTFL_FCMD_SET_FLEXRAM = 0x81
		} fcmd : 8;
		uint8_t data_be[8];
	} generic;
	struct {
		uint32_t addr : 24;
		enum FTFL_FCMD fcmd : 8;
		uint8_t _rsvd0[3];
		enum FTFL_MARGIN_CHOICE {
			FTFL_MARGIN_NORMAL = 0x00,
			FTFL_MARGIN_USER = 0x01,
			FTFL_MARGIN_FACTORY = 0x02
		} margin : 8;
	} read_1s_block;
	struct ftfl_data_num_words {
		uint32_t addr : 24;
		enum FTFL_FCMD fcmd : 8;
		uint8_t _rsvd0;
		enum FTFL_MARGIN_CHOICE margin : 8;
		uint16_t num_words;
	} read_1s_section;
	struct {
		uint32_t addr : 24;
		enum FTFL_FCMD fcmd : 8;
		uint8_t _rsvd0[3];
		enum FTFL_MARGIN_CHOICE margin : 8;
		uint8_t data_be[4];
	} program_check;
	struct {
		uint32_t addr : 24;
		enum FTFL_FCMD fcmd : 8;
		uint32_t data;
		uint8_t _rsvd0[3];
		enum FTFL_RESOURCE_SELECT {
			FTFL_RESOURCE_IFR = 0x00,
			FTFL_RESOURCE_VERSION = 0x01
		} resource_select : 8;
	} read_resource;
	struct {
		uint32_t addr : 24;
		enum FTFL_FCMD fcmd : 8;
		uint8_t data_be[4];
	} program_longword;
	struct {
		uint32_t addr : 24;
		enum FTFL_FCMD fcmd : 8;
	} erase;
	struct ftfl_data_num_words program_section;
	struct {
		uint8_t _rsvd0[2];
		enum FTFL_MARGIN_CHOICE margin : 8;
		enum FTFL_FCMD fcmd : 8;
	} read_1s_all_blocks;
	struct ftfl_cmd_once {
		uint8_t _rsvd0[2];
		uint8_t idx;
		enum FTFL_FCMD fcmd : 8;
		uint8_t data_be[4];
	} read_once;
	struct ftfl_cmd_once program_once;
	struct {
		uint8_t _rsvd0[3];
		enum FTFL_FCMD fcmd : 8;
	} erase_all;
	struct {
		uint8_t _rsvd0[3];
		enum FTFL_FCMD fcmd : 8;
		uint8_t key_be[8];
	} verify_key;
	struct {
		uint8_t _rsvd0[3];
		enum FTFL_FCMD fcmd : 8;
		uint8_t _rsvd1[2];

		/* the following enum is analogous to enum
		 * SIM_FLEXNVM_PARTITION in sim.h, but this one is padded
		 * with four 1-bits to make an 8-bit value.
		 */

		enum FTFL_FLEXNVM_PARTITION {
			FTFL_FLEXNVM_DATA_32_EEPROM_0 = 0xF0,
			FTFL_FLEXNVM_DATA_24_EEPROM_8 = 0xF1,
			FTFL_FLEXNVM_DATA_16_EEPROM_16 = 0xF2,
			FTFL_FLEXNVM_DATA_8_EEPROM_24 = 0xF9,
			FTFL_FLEXNVM_DATA_0_EEPROM_32 = 0xF3
		} flexnvm_partition : 8;
		enum FTFL_EEPROM_SIZE {
			FTFL_EEPROM_SIZE_0 = 0x3f,
			FTFL_EEPROM_SIZE_32 = 0x39,
			FTFL_EEPROM_SIZE_64 = 0x38,
			FTFL_EEPROM_SIZE_128 = 0x37,
			FTFL_EEPROM_SIZE_256 = 0x36,
			FTFL_EEPROM_SIZE_512 = 0x35,
			FTFL_EEPROM_SIZE_1024 = 0x34,
			FTFL_EEPROM_SIZE_2048 = 0x33
		} eeprom_size : 8;
	} program_partition;
	struct {
		uint8_t _rsvd0[2];
		enum FTFL_FLEXRAM_FUNCTION {
			FTFL_FLEXRAM_EEPROM = 0x00,
			FTFL_FLEXRAM_RAM = 0xff
		} flexram_function : 8;
		enum FTFL_FCMD fcmd : 8;
	} set_flexram;
};
CTASSERT_SIZE_BYTE(union FTFL_FCCOB_t, 12);

struct FTFL_t {
	struct FTFL_FSTAT_t fstat;
	struct FTFL_FCNFG_t fcnfg;
	struct FTFL_FSEC_t fsec;
	struct FTFL_FOPT_t fopt;
	union FTFL_FCCOB_t fccob;
	uint8_t fprot_be[4];
	uint8_t feprot;
	uint8_t fdprot;
};
CTASSERT_SIZE_BYTE(struct FTFL_t, 0x18);

/* Flash Configuration Field, see Sub-Family Reference Manual, section 28.3.1 */
struct FTFL_CONFIG_t {
	uint8_t key[8];
	uint8_t fprot[4];
	struct FTFL_FSEC_t fsec;
	struct FTFL_FOPT_t fopt;
	uint8_t feprot;
	uint8_t fdprot;
};
CTASSERT_SIZE_BYTE(struct FTFL_CONFIG_t, 16);

extern volatile struct FTFL_t FTFL;
extern char FlexRAM[];
extern struct FTFL_CONFIG_t FTFL_CONFIG;

#endif

