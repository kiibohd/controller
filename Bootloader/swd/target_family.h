/**
 * @file    target_family.h
 * @brief
 *
 * DAPLink Interface Firmware
 * Copyright (c) 2018-2019, ARM Limited, All Rights Reserved
 * Modifications by Jacob Alexander 2020
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TARGET_FAMILY_H
#define TARGET_FAMILY_H

#include <stdint.h>

//! @brief States into which the target can be placed.
//!
//! These enums are passed to target_set_state() and indicate the desired state into which
//! the target should be reset.
typedef enum _target_state {
    RESET_HOLD,              //!< Hold target in reset
    RESET_PROGRAM,           //!< Reset target and setup for flash programming
    RESET_RUN,               //!< Reset target and run normally
    NO_DEBUG,                //!< Disable debug on running target
    DEBUG,                   //!< Enable debug on running target
    HALT,                    //!< Halt the target without resetting it
    RUN,                     //!< Resume the target without resetting it
    POST_FLASH_RESET,        //!< Reset target after flash programming
    POWER_ON,                //!< Poweron the target
    SHUTDOWN,                //!< Poweroff the target
} target_state_t;

//! @brief Controls reset of the target.
void swd_set_target_reset(uint8_t asserted);

#endif
