/* Copyright (C) 2013-2017 by Jacob Alexander
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


// This include file unifies some of the nomenclature between the AVR and ARM compilers


// ----- Includes -----

#pragma once

#include <Lib/mcu_compat.h>

// ARM
#if defined(_kinetis_)

#include <Lib/kinetis.h>

// AVR
#elif defined(_avr_at_)

#include <avr/interrupt.h>

// Host
#elif defined(_host_)

#include <Lib/host.h>

#endif



// ----- Defines -----

// ARM
#if defined(_kinetis_)

// Map the Interrupt Enable/Disable to the AVR names
#define cli() __disable_irq()
#define sei() __enable_irq()


// AVR
#elif defined(_avr_at_)


// Host
#elif defined(_host_)


#endif

