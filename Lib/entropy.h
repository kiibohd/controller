/* Entropy - A entropy (random number) generator for the Arduino
 * Forked from https://github.com/pmjdebruijn/Arduino-Entropy-Library (2017)
 *
 *   The latest version of this library will always be stored in the following
 *   google code repository:
 *     http://code.google.com/p/avr-hardware-random-number-generation/source/browse/#git%2FEntropy
 *   with more information available on the libraries wiki page
 *     http://code.google.com/p/avr-hardware-random-number-generation/wiki/WikiAVRentropy
 *
 * Copyright 2014 by Walter Anderson
 * Modifications 2017 by Jacob Alexander
 *
 * Entropy is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Entropy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Entropy.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

// ----- Includes -----

#include <stdint.h>



// ----- Functions -----

void rand_initialize();
void rand_disable();

uint8_t rand_available();

uint32_t rand_value32();

