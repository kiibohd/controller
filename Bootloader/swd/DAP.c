/*
 * Copyright (c) 2013-2017 ARM Limited. All rights reserved.
 * Copyright 2019, Cypress Semiconductor Corporation
 * or a subsidiary of Cypress Semiconductor Corporation.
 * Modifications by Jacob Alexander 2020
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ----------------------------------------------------------------------
 *
 * $Date:        1. December 2017
 * $Revision:    V2.0.0
 *
 * Project:      CMSIS-DAP Source
 * Title:        DAP.c CMSIS-DAP Commands
 *
 *---------------------------------------------------------------------------*/

#include <string.h>
#include "DAP_config.h"
#include "DAP.h"
#include <debug.h>


// Clock Macros

#define MAX_SWJ_CLOCK(delay_cycles) \
  ((CPU_CLOCK/2U) / (IO_PORT_WRITE_CYCLES + delay_cycles))

#define CLOCK_DELAY(swj_clock) \
 (((CPU_CLOCK/2U) / swj_clock) - IO_PORT_WRITE_CYCLES)


DAP_Data_t DAP_Data;           // DAP Data


// Setup DAP
void DAP_Setup(void) {

  // Default settings
  DAP_Data.fast_clock  = 0U;
  DAP_Data.clock_delay = CLOCK_DELAY(DAP_DEFAULT_SWJ_CLOCK);
  DAP_Data.transfer.idle_cycles = 0U;
  DAP_Data.swd_conf.turnaround  = 1U;
  DAP_Data.swd_conf.data_phase  = 0U;
}
