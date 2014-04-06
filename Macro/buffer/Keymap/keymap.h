/* Copyright (C) 2011-2012 by Jacob Alexander
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

#ifndef __KEYMAP_H
#define __KEYMAP_H

// ----- Includes -----

#include "usb_keys.h"



// ----- Defines -----



// ----- Variables -----

// Lots of these variables are not used, so ignore gcc unused warnings
// But just for the variables in this file (and those included into it)
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic push


// See files for full layout descriptions
#include "avrcapsense.h"
#include "betkb.h"
#include "budkeypad.h"
#include "epsonqx10.h"
#include "facom6684.h"
#include "heathzenith.h"
#include "hp150.h"
#include "ibmconvertible.h"
#include "kaypro1.h"
#include "mbc55x.h"
#include "microswitch8304.h"
#include "skm67001.h"
#include "sonynews.h"
#include "sonyoas3400.h"
#include "tandy1000.h"
#include "univacf3w9.h"



// Only ignore unused warnings for the above variables
#pragma GCC diagnostic pop



#endif

