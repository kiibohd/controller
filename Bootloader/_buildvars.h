/* Copyright (C) 2013-2022 by Jacob Alexander
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



// ----- Defines -----

// Helpers
#define _STR_HELPER(x) #x
#define _STR(x) _STR_HELPER(x)

// You can change these to give your code its own name.
#define STR_MANUFACTURER        u"@MANUFACTURER@"
#define STR_PRODUCT             u"@BOOT_PRODUCT_STR@"
#define STR_ALT_PRODUCT         u"@BOOT_ALT_PRODUCT_STR@"
#define STR_SERIAL              u"00000000000000000000000000000000 - @CHIP@:0000"
#define STR_SERIAL_BLE          u"00000000000000000000000000000000 - @CHIP@:0000 | 0000000000000000 - nRF52810:0000"
#define STR_CONFIG_NAME         u"@FLASHING_STATION_ID@"
#define STR_ALTNAME             u"@BOOT_DFU_ALTNAME@"
#define STR_ALTNAME2            u"@BOOT_DFU_ALTNAME2@"


// Mac OS-X and Linux automatically load the correct drivers.  On
// Windows, even though the driver is supplied by Microsoft, an
// INF file is needed to load the driver.  These numbers need to
// match the INF file.
#define STR_WCID_DRIVER         "LIBUSB0\0"
#define VENDOR_ID               @BOOT_VENDOR_ID@
#define PRODUCT_ID              @BOOT_PRODUCT_ID@
#define ALT_PRODUCT_ID          @BOOT_ALT_PRODUCT_ID@
#define BCD_VERSION             @Git_Commit_Number@



// ----- Structs -----

// This struct is stored in the user signature area and can be accessed by the bootloader
typedef struct FirmwareInfo {
	uint16_t revision;      // Build revision number
	uint16_t reserved;      // RESERVED (user signature must be written in 32-bit chunks)
} __attribute__((packed)) __attribute__((aligned(4))) FirmwareInfo;

