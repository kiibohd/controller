/*
 * Copyright (C) 2014 Jan Rychter
 * Modifications (C) 2015-2016 Jacob Alexander
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

// Compiler Includes
#include <Lib/ScanLib.h>



// ----- Defines -----

// Channel status definitions. These are not enumerated, as I want them to be uint8_t.
#define I2C_AVAILABLE 0
#define I2C_BUSY 1
#define I2C_ERROR 2


#define I2C_RESTART 1<<8
#define I2C_READ    2<<8



// ----- Structs -----

typedef struct {
	uint16_t *sequence;
	uint16_t *sequence_end;
	uint8_t *received_data;
	void (*callback_fn)(void*);
	void *user_data;
	uint8_t reads_ahead;
	uint8_t status;
	uint8_t txrx;
} I2C_Channel;



// ----- Functions -----

/*
 * I2C Module Setup
 */

void i2c_setup();


/*
 * Sends a command/data sequence that can include restarts, writes and reads. Every transmission begins with a START,
 * and ends with a STOP so you do not have to specify that.
 *
 * sequence is the I2C operation sequence that should be performed. It can include any number of writes, restarts and
 * reads. Note that the sequence is composed of uint16_t, not uint8_t. This is because we have to support out-of-band
 * signalling of I2C_RESTART and I2C_READ operations, while still passing through 8-bit data.
 *
 * sequence_length is the number of sequence elements (not bytes). Sequences of arbitrary length are supported. The
 * minimum sequence length is (rather obviously) 2.
 *
 * received_data should point to a buffer that can hold as many bytes as there are I2C_READ operations in the
 * sequence. If there are no reads, 0 can be passed, as this parameter will not be used.
 *
 * callback_fn is a pointer to a function that will get called upon successful completion of the entire sequence. If 0 is
 * supplied, no function will be called. Note that the function will be called fron an interrupt handler, so it should do
 * the absolute minimum possible (such as enqueue an event to be processed later, set a flag, exit sleep mode, etc.)
 *
 * user_data is a pointer that will be passed to the callback_fn.
 */

int32_t i2c_send_sequence(
	uint8_t ch,
	uint16_t *sequence,
	uint32_t sequence_length,
	uint8_t *received_data,
	void (*callback_fn)(void*),
	void *user_data
);

/*
 * Convenience macros
 */
#define i2c_send(ch, seq, seq_len)      i2c_send_sequence( ch, seq, seq_len, 0, 0, 0 )
#define i2c_read(ch, seq, seq_len, rec) i2c_send_sequence( ch, seq, seq_len, rec, 0, 0 )

/*
 * Check if busy
 */
uint8_t i2c_busy( uint8_t ch );
uint8_t i2c_any_busy();

/*
 * Reset i2c buses after an error
 */
void i2c_reset();

