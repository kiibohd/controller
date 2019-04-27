/*
 * Copyright (C) 2019 Jacob Alexander
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#pragma once

// ----- Includes ----

// ASF Includes
#include <sam/drivers/pdc/pdc.h>

// Compiler Includes
#include <Lib/ScanLib.h>



// ----- Defines -----

// ----- Enumerations -----

typedef enum DAC_Sample_Status {
	DAC_Sample_Status_None = 0,     // Sample not yet added
	DAC_Sample_Status_Queued = 1,   // Sample queued, but not yet started
	DAC_Sample_Status_Started = 2,  // Sample started using pdc
	DAC_Sample_Status_Finished = 3, // Sample finished successfully
	DAC_Sample_Status_Stopped = 4,  // Sample either was stopped partway through or unqueued (not successful)
} DAC_Sample_Status;



// ----- Structs -----

typedef struct DAC_Sample {
	DAC_Sample_Status status;
	pdc_packet_t tx_buffer;
} DAC_Sample;



// ----- Functions -----

void dac_setup();

uint8_t dac_set_sample(volatile DAC_Sample *sample);

