/* Copyright (C) 2013-2018 by Jacob Alexander
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


// This include file decides which set of compiler family include files to use on typical Scan modules.
// Additional includes should only be added to this file if they should be added to *all* Scan modules.

#pragma once

// ----- Includes -----

#include <Lib/mcu_compat.h>
#include <Lib/Interrupts.h>



// Kinetis (ARM)
#if defined(_kinetis_)

#include <Lib/kinetis.h>
#include <Lib/time.h>

#endif



// SAM (ARM)
#if defined(_sam_)

#include <Lib/sam.h>
#include <Lib/time.h>

#endif



// NRF5 (ARM)
#if defined(_nrf_)

#include <Lib/nrf5.h>
#include <Lib/time.h>

#endif



// AVR
#if defined(_avr_at_)

#include <avr/io.h>

#endif



// Host
#if defined(_host_)

#include <Lib/host.h>

#endif

