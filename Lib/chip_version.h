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

#if defined(_kinetis_)

// ----- Defines -----

//
// - SIM_SDID Lookup -
//

// ----- Includes -----

#include <stdint.h>
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
uint16_t ChipVersion_nvmsize[] = {
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
uint16_t ChipVersion_pflashsize[] = {
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
uint16_t ChipVersion_eepromsize[] = {
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
uint16_t ChipVersion_ramsize[] = {
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
			return "mk20dx128vlh7";
		case 256:
			return "mk20dx256vlh7";
		default:
			return "mk20dxXXXvlh7";
		}
	}
	else if ( SIM_SDID & 0x00000099 ) // TODO Might be 0x97 81/121-pin
	{
		switch ( pflash )
		{
		case 256:
			return "mk20dx256vmc7";
		default:
			return "mk20dxXXXvmc7";
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

