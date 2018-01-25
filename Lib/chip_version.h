/* Copyright (C) 2017 by Jacob Alexander
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

// ----- Includes -----

#include <stdint.h>


#if defined(_sam_)

// ----- Includes -----

#include <Lib/sam.h>



// ----- Variables -----

// See 30.3.1 CHIPID_CIDR | EPROC | Embedded Processor
// http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-11100-32-bit%20Cortex-M4-Microcontroller-SAM4S_Datasheet.pdf
const char *ChipVersion_proctype[] = {
	"SAMx7",     // 0 - 000 - Cortex-M7
	"ARM946ES",  // 1 - 001
	"ARM7TDMI",  // 2 - 010
	"CM3",       // 3 - 011 - Cortex-M3
	"ARM920T",   // 4 - 100
	"ARM926EJS", // 5 - 101
	"CA5",       // 6 - 110 - Cortex-A5
	"CM4",       // 7 - 111 - Cortex-M4
};

// See 30.3.1 CHIPID_CIDR | NVPSIZ | NVPSIZ2 | Nonvolatile Program Memory Size
// http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-11100-32-bit%20Cortex-M4-Microcontroller-SAM4S_Datasheet.pdf
// Mark reserved as 0xFFFF
const uint16_t ChipVersion_nvmsize[] = {
	0,      // 0  - 0000 -    0 kB
	8,      // 1  - 0001 -    8 kB
	16,     // 2  - 0010 -   16 kB
	32,     // 3  - 0011 -   32 kB
	0xFFFF, // 4  - 0100
	64,     // 5  - 0101 -   64 kB
	0xFFFF, // 6  - 0110
	128,    // 7  - 0111 -  128 kB
	160,    // 8  - 1000 -  160 kB
	256,    // 9  - 1001 -  256 kB
	512,    // 10 - 1010 -  512 kB
	0xFFFF, // 11 - 1011
	1024,   // 12 - 1100 - 1024 kB
	0xFFFF, // 13 - 1101
	2048,   // 14 - 1110 - 2048 kB
	0xFFFF, // 15 - 1111
};

// See 30.3.1 CHIPID_CIDR | SRAMSIZ | Internal SRAM Size
// http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-11100-32-bit%20Cortex-M4-Microcontroller-SAM4S_Datasheet.pdf
// Mark reserved as 0xFFFF
const uint16_t ChipVersion_sramsize[] = {
	48,  // 0  - 0000 -  48 kB
	192, // 1  - 0001 - 192 kB
	384, // 2  - 0010 - 384 kB
	6,   // 3  - 0011 -   6 kB
	24,  // 4  - 0100 -  24 kB
	4,   // 5  - 0101 -   4 kB
	80,  // 6  - 0110 -  80 kB
	160, // 7  - 0111 - 160 kB
	8,   // 8  - 1000 -   8 kB
	16,  // 9  - 1001 -  16 kB
	32,  // 10 - 1010 -  32 kB
	64,  // 11 - 1011 -  64 kB
	128, // 12 - 1100 - 128 kB
	256, // 13 - 1101 - 256 kB
	96,  // 14 - 1110 -  96 kB
	512, // 15 - 1111 - 512 kB
};

// See 30.3.1 CHIPID_CIDR | ARCH | Architecture Identifier
// http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-11100-32-bit%20Cortex-M4-Microcontroller-SAM4S_Datasheet.pdf
// XXX (HaaTa) - Only from 0x88 to 0x8A, make sure to update offset when adding more
const char *ChipVersion_archid[] = {
	"SAM4SxA", // 0x88 - 136 - 1000 1000 - 48-pin
	"SAM4SxB", // 0x89 - 137 - 1000 1001 - 64-pin
	"SAM4SxC", // 0x8A - 138 - 1000 1010 - 100-pin
};

// See 30.3.1 CHIPID_CIDR | NVPTYP | Nonvolatile Program Memory Type
// http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-11100-32-bit%20Cortex-M4-Microcontroller-SAM4S_Datasheet.pdf
// Mark reserved as --
const char *ChipVersion_nvmtype[] = {
	"ROM",       // 0 - 000 - ROM
	"ROMLESS",   // 1 - 001 - ROMless or on-chip Flash
	"FLASH",     // 2 - 010 - Embedded Flash Memory
	"ROM_FLASH", // 3 - 011 - ROM adn Embedded Flash Memory (NVPSIZ is ROM, NVPSIZ2 is Flash)
	"SRAM",      // 4 - 100 - SRAM emulating ROM
	"--",        // 5 - 101
	"--",        // 6 - 110
	"--",        // 7 - 111
};




// ----- Function Declarations -----

// ----- Functions -----

#elif defined(_nrf_)

// ----- Includes -----



// ----- Variables -----

// See FICR | INFO.PART
// http://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.nrf52810.ps%2Fficr.html&cp=2_2_0_3_3_0_15&anchor=register.INFO.PART
// http://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.nrf52832.ps.v1.1%2Fficr.html&cp=2_1_0_12_0_15&anchor=register.INFO.PART
// http://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.nrf52840.ps%2Fficr.html&anchor=register.INFO.PART
// Hex encoding of partnumber string (e.g. 0x52810, 0x52832, 0x52840

// See FICR | INFO.VARIANT | Part Variant, Hardware version and Production configuration
// http://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.nrf52810.ps%2Fficr.html&cp=2_2_0_3_3_0_15&anchor=register.INFO.VARIANT
// http://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.nrf52832.ps.v1.1%2Fficr.html&cp=2_1_0_12_0_15&anchor=register.INFO.VARIANT
// http://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.nrf52840.ps%2Fficr.html&anchor=register.INFO.VARIANT
// ASCII encoding variant
// AAAA 0x41414141
// AAA0 0x41414130
// AABA 0x41414241
// AABB 0x41414242
// AAB0 0x41414230
// AACA 0x41414341
// AACB 0x41414342
// AAC0 0x41414330
// Unspecified 0xFFFFFFFF

// See FICR | INFO.PACKAGE | Package option
// http://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.nrf52810.ps%2Fficr.html&cp=2_2_0_3_3_0_15&anchor=register.INFO.PACKAGE
// http://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.nrf52832.ps.v1.1%2Fficr.html&cp=2_1_0_12_0_15&anchor=register.INFO.PACKAGE
// http://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.nrf52840.ps%2Fficr.html&anchor=register.INFO.PACKAGE
const char *ChipVersion_package[] = {
	"QF", // 0x2000     - QFxx - 48-pin QFN
	"CH", // 0x2001     - CHxx - 56-pin WLCSP
	"CI", // 0x2002     - CIxx - 56-pin WLCSP
	"QC", // 0x2003     - QCxx - 32-pin QFN
	"QI", // 0x2004     - QIxx - 73-pin AQFN
	"CK", // 0x2005     - CKxx - 56-pin WLCSP
	"--", // 0xFFFFFFFF - Unspecified
};

// See FICR | INFO.RAM | RAM variant
// http://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.nrf52810.ps%2Fficr.html&cp=2_2_0_3_3_0_15&anchor=register.INFO.RAM
// http://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.nrf52832.ps.v1.1%2Fficr.html&cp=2_1_0_12_0_15&anchor=register.INFO.RAM
// http://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.nrf52840.ps%2Fficr.html&anchor=register.INFO.RAM
// Mark reserved as --
const char *ChipVersion_ramsize[] = {
	"K16",  // 0x010      -  16 kB RAM
	"K32",  // 0x020      -  32 kB RAM
	"K64",  // 0x040      -  64 kB RAM
	"K128", // 0x080      - 128 kB RAM
	"K256", // 0x100      - 256 kB RAM
	"--",   // 0xFFFFFFFF - Unspecified
};

// See FICR | INFO.FLASH | Flash variant
// http://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.nrf52810.ps%2Fficr.html&cp=2_2_0_3_3_0_15&anchor=register.INFO.FLASH
// http://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.nrf52832.ps.v1.1%2Fficr.html&cp=2_1_0_12_0_15&anchor=register.INFO.FLASH
// http://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.nrf52840.ps%2Fficr.html&anchor=register.INFO.FLASH
// Mark reserved as --
const char *ChipVersion_flashsize[] = {
	"K128",  // 0x080      -  128 kByte Flash
	"K256",  // 0x100      -  256 kByte Flash
	"K512",  // 0x200      -  512 kByte Flash
	"K1024", // 0x400      - 1024 kByte Flash
	"K2048", // 0x800      - 2048 kByte Flash
	"--",    // 0xFFFFFFFF - Unspecified
};




// ----- Function Declarations -----

// ----- Functions -----

#elif defined(_kinetis_)

// ----- Includes -----

#include <Lib/kinetis.h>



// ----- Variables -----

// See 12.2.7 SIM_SDID - https://www.nxp.com/docs/en/reference-manual/K22P64M120SF5V2RM.pdf
// Can be updated as long as it doesn't conflict with older versions
// Mark reserved as --
// PINID
const char *ChipVersion_pincount[] = {
	"--",         //  0 - 0000
	"--",         //  1 - 0001
	"32-pin",     //  2 - 0010
	"--",         //  3 - 0011
	"48-pin",     //  4 - 0100
	"64-pin",     //  5 - 0101
	"80-pin",     //  6 - 0110
	"81/121-pin", //  7 - 0111
	"100-pin",    //  8 - 1000
	"121-pin",    //  9 - 1001
	"144-pin",    // 10 - 1010
	"WLCSP",      // 11 - 1011 - Custom pinout (WLCSP)
	"--",         // 12 - 1100
	"--",         // 13 - 1101
	"256-pin",    // 14 - 1110
	"--",         // 15 - 1111
};

// See 12.2.7 SIM_SDID - https://www.nxp.com/docs/en/reference-manual/K22P64M120SF5V2RM.pdf
// Can be updated as long as it doesn't conflict with older versions
// Mark reserved as --
// FAMID
const char *ChipVersion_familyid[] = {
	"K10/12",    // 0 - 000 - K10, K12        Family
	"K20/22",    // 1 - 001 - K20, K22        Family
	"K30/11/61", // 2 - 010 - K30, K11 or K61 Family
	"K40/21",    // 3 - 011 - K40, K21        Family
	"K60/62",    // 4 - 100 - K60, K62        Family
	"K70",       // 5 - 101 - K70             Family
	"--",        // 6 - 110
	"--",        // 7 - 111
};

// See 12.2.17 SIM_FCFG1 - https://www.nxp.com/docs/en/reference-manual/K22P64M120SF5V2RM.pdf
// Can be updated as long as it doesn't conflict with older versions
// Mark reserved as 0xFFFF
// NVMSIZE
const uint16_t ChipVersion_nvmsize[] = {
	0,      // 0000 -   0 kB FlexNVM
	0xFFFF, // 0001
	0xFFFF, // 0010
	32,     // 0011 -  32 kB FlexNVM
	64,     // 0101 -  64 kB FlexNVM
	128,    // 0111 - 128 kB FlexNVM
	0xFFFF, // 1000
	256,    // 1001 - 256 kB FlexNVM
	0xFFFF, // 1010
	512,    // 1011 - 512 kB FlexNVM
	0xFFFF, // 1100
	0xFFFF, // 1101
	0xFFFF, // 1110
	0xFFFF, // 1111
	512,    // 1111 - 512 kB FlexNVM
};

// See 12.2.17 SIM_FCFG1 - https://www.nxp.com/docs/en/reference-manual/K22P64M120SF5V2RM.pdf
// Can be updated as long as it doesn't conflict with older versions
// Mark reserved as 0xFFFF
// PFSIZE
const uint16_t ChipVersion_pflashsize[] = {
	0xFFFF, // 0000
	0xFFFF, // 0001
	0xFFFF, // 0010
	32,     // 0011 -   32 kB Program Flash
	0xFFFF, // 0100
	64,     // 0101 -   64 kB Program Flash
	0xFFFF, // 0110
	128,    // 0111 -  128 kB Program Flash
	0xFFFF, // 1000
	256,    // 1001 -  256 kB Program Flash
	0xFFFF, // 1010
	512,    // 1011 -  512 kB Program Flash
	0xFFFF, // 1100
	1024,   // 1101 - 1024 kB Program Flash
	0xFFFF, // 1110
	1024,   // 1111 - 1024 kB Program Flash
};

// See 12.2.17 SIM_FCFG1 - https://www.nxp.com/docs/en/reference-manual/K22P64M120SF5V2RM.pdf
// Can be updated as long as it doesn't conflict with older versions
// Mark reserved as 0xFFFF
// EESIZE
const uint16_t ChipVersion_eepromsize[] = {
	16384,  // 0000 -  16 kB EEPROM
	8192,   // 0001 -   8 kB EEPROM
	4096,   // 0010 -   4 kB EEPROM
	2048,   // 0011 -   2 kB EEPROM
	1024,   // 0100 -   1 kB EEPROM
	512,    // 0101 - 512 B  EEPROM
	256,    // 0110 - 256 B  EEPROM
	128,    // 0111 - 128 B  EEPROM
	64,     // 1000 -  64 B  EEPROM
	32,     // 1001 -  32 B  EEPROM
	0xFFFF, // 1010
	0xFFFF, // 1011
	0xFFFF, // 1100
	0xFFFF, // 1101
	0xFFFF, // 1110
	0,      // 1111 -    0 B  EEPROM
};

// See 12.2.1 SIM_SOPT1 - https://www.nxp.com/docs/en/reference-manual/K22P64M120SF5V2RM.pdf
// Can be updated as long as it doesn't conflict with older versions
// Mark reserved as 0xFFFF
// RAMSIZE
const uint16_t ChipVersion_ramsize[] = {
	0xFFFF, // 0000
	8,      // 0001 -   8 kB RAM
	0xFFFF, // 0010
	16,     // 0011 -  16 kB RAM
	24,     // 0100 -  24 kB RAM
	32,     // 0101 -  32 kB RAM
	48,     // 0110 -  48 kB RAM
	64,     // 0111 -  64 kB RAM
	96,     // 1000 -  96 kB RAM
	128,    // 1001 - 128 kB RAM
	0xFFFF, // 1010
	256,    // 1011 - 256 kB RAM
	0xFFFF, // 1100
	0xFFFF, // 1101
	0xFFFF, // 1110
	0xFFFF, // 1111
};



// ----- Function Declarations -----

// ----- Functions -----

const char *ChipVersion_cpuid_partno()
{
	switch ( SCB_CPUID_PARTNO )
	{
	case 0xC24:
		return "Cortex-M4";
	default:
		return "--";
	}
}

const char *ChipVersion_cpuid_implementor()
{
	switch ( SCB_CPUID_IMPLEMENTOR )
	{
	case 0x41:
		return "ARM";
	default:
		return "--";
	}
}

// Attempts to detect the chip part number using registers only
// Not perfect, but is quite useful
// Returns -- if unknown
const char *ChipVersion_lookup()
{
	// Kinetis K-Series pg. 7
	// http://cache.freescale.com/files/product/doc/BRORDERINFO.pdf
	//
	// The first m stands for production, the other option is p (unqualified sample)

	//uint8_t family = SIM_SDID_FAMID; // e.g. k20
	// TODO e.g. d (Cortex-M4)
	//uint8_t flex = SIM_FCFG1_NVMSIZE ? 1 : 0; // e.g. x (Flex memory)
	uint16_t pflash = ChipVersion_pflashsize[ SIM_FCFG1_PFSIZE ]; // e.g. 256 (256 kB)
	// TODO e.g. a (3rd rev)
	// TODO e.g. v (-40C to +105C)
	//uint8_t pincount = SIM_SDID_PINID; // e.g. LH (64 LQFP)
	// TODO e.g. 7 (70 MHz)

	// Using openocd method of using the SDID mask
	// Not perfect, but generally ok, also using the pincount
	if ( SIM_SDID & 0x00000095 )
	{
		switch ( pflash )
		{
		case 128:
			switch ( SIM_SDID_PINID )
			{
			case 4:
				return "mk20dx128vlf5";
			case 5:
				return "mk20dx128vlh7";
			default:
				return "mk20dx128XXXX";
			}
		case 256:
			switch ( SIM_SDID_PINID )
			{
			case 5:
				return "mk20dx256vlh7";
			case 9:
				return "mk20dx256vmc7";
			default:
				return "mk20dx256XXXX";
			}
		default:
			return "mk20dxXXXvlh7";
		}
	}
	else if ( SIM_SDID & 0x00000315 )
	{
		switch ( pflash )
		{
		case 512:
			return "mk22fx512avlh12";
		default:
			return "mk22fxXXXavlh12";
		}
	}

	return "--";
}

#endif

