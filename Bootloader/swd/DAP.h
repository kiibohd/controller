/*
 * Copyright (c) 2013-2019 ARM Limited. All rights reserved.
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
 * $Date:        26. November 2019
 * $Revision:    V2.0.0
 *
 * Project:      CMSIS-DAP Include
 * Title:        DAP.h Definitions
 *
 *---------------------------------------------------------------------------*/

#ifndef __DAP_H__
#define __DAP_H__


// DAP Transfer Request
#define DAP_TRANSFER_APnDP              (1U<<0)
#define DAP_TRANSFER_RnW                (1U<<1)
#define DAP_TRANSFER_A2                 (1U<<2)
#define DAP_TRANSFER_A3                 (1U<<3)
#define DAP_TRANSFER_MATCH_VALUE        (1U<<4)
#define DAP_TRANSFER_MATCH_MASK         (1U<<5)
#define DAP_TRANSFER_TIMESTAMP          (1U<<7)

// DAP Transfer Response
#define DAP_TRANSFER_OK                 (1U<<0)
#define DAP_TRANSFER_WAIT               (1U<<1)
#define DAP_TRANSFER_FAULT              (1U<<2)
#define DAP_TRANSFER_ERROR              (1U<<3)
#define DAP_TRANSFER_MISMATCH           (1U<<4)


// Debug Port Register Addresses
#define DP_IDCODE                       0x00U   // IDCODE Register (SW Read only)
#define DP_ABORT                        0x00U   // Abort Register (SW Write only)
#define DP_CTRL_STAT                    0x04U   // Control & Status
#define DP_WCR                          0x04U   // Wire Control Register (SW Only)
#define DP_SELECT                       0x08U   // Select Register (JTAG R/W & SW W)
#define DP_RESEND                       0x08U   // Resend (SW Read Only)
#define DP_RDBUFF                       0x0CU   // Read Buffer (Read Only)

// SWD Sequence Info
#define SWD_SEQUENCE_CLK                0x3FU   // SWCLK count
#define SWD_SEQUENCE_DIN                0x80U   // SWDIO capture


#include <stddef.h>
#include <stdint.h>
#include <sam4s.h>

// DAP Data structure
typedef struct {
  uint8_t     fast_clock;                       // Fast Clock Flag
  uint32_t   clock_delay;                       // Clock Delay
  uint32_t     timestamp;                       // Last captured Timestamp
  struct {                                      // Transfer Configuration
    uint8_t   idle_cycles;                      // Idle cycles after transfer
  } transfer;
  struct {                                      // SWD Configuration
    uint8_t    turnaround;                      // Turnaround period
    uint8_t    data_phase;                      // Always generate Data Phase
  } swd_conf;
} DAP_Data_t;

extern          DAP_Data_t DAP_Data;            // DAP Data


// Functions
extern void     SWJ_Sequence    (uint32_t count, const uint8_t *data);
extern void     SWD_Sequence    (uint32_t info,  const uint8_t *swdo, uint8_t *swdi);
extern uint8_t  SWD_Transfer    (uint32_t request, uint32_t *data);

extern void     DAP_Setup (void);

// Configurable delay for clock generation
#ifndef DELAY_SLOW_CYCLES
#define DELAY_SLOW_CYCLES       3U      // Number of cycles for one iteration
#endif
__STATIC_FORCEINLINE void PIN_DELAY_SLOW (uint32_t delay) {
  __ASM volatile (
  ".syntax unified\n"
  "0:\n\t"
    "subs %0,%0,#1\n\t"
    "bne  0b\n"
  : "+l" (delay) : : "cc"
  );
}

// Fixed delay for fast clock generation
#ifndef DELAY_FAST_CYCLES
#define DELAY_FAST_CYCLES       0U      // Number of cycles: 0..3
#endif
__STATIC_FORCEINLINE void PIN_DELAY_FAST (void) {
#if (DELAY_FAST_CYCLES >= 1U)
  __NOP();
#endif
#if (DELAY_FAST_CYCLES >= 2U)
  __NOP();
#endif
#if (DELAY_FAST_CYCLES >= 3U)
  __NOP();
#endif
}


#endif  /* __DAP_H__ */
