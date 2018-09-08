/**
 * \file
 *
 * Copyright (c) 2012-2018 Microchip Technology Inc. and its subsidiaries.
 * Modified by Jacob Alexander 2018
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#pragma once

#if defined _sam4sa16b_
  #include "sam4sa16b.h"
#elif defined _sam4sa16c_
  #include "sam4sa16c.h"
#elif defined _sam4sd16b_
  #include "sam4sd16b.h"
#elif defined _sam4sd16c_
  #include "sam4sd16c.h"
#elif defined _sam4sd32b_
  #include "sam4sd32b.h"
#elif defined _sam4sd32c_
  #include "sam4sd32c.h"
#elif defined _sam4s2a_
  #include "sam4s2a.h"
#elif defined _sam4s2b_
  #include "sam4s2b.h"
#elif defined _sam4s2c_
  #include "sam4s2c.h"
#elif defined _sam4s4a_
  #include "sam4s4a.h"
#elif defined _sam4s4b_
  #include "sam4s4b.h"
#elif defined _sam4s4c_
  #include "sam4s4c.h"
#elif defined _sam4s8b_
  #include "sam4s8b.h"
#elif defined _sam4s8c_
  #include "sam4s8c.h"
#elif defined _sam4s16b_
  #include "sam4s16b.h"
#elif defined _sam4s16c_
  #include "sam4s16c.h"
#else
  #error Library does not support the specified device.
#endif

