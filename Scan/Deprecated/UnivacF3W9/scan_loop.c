/* Copyright (C) 2012,2014 by Jacob Alexander
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

// ----- Includes -----

// Compiler Includes
#include <Lib/ScanLib.h>

// Project Includes
#include <led.h>
#include <print.h>

// Local Includes
#include "scan_loop.h"



// ----- Defines -----

// Pinout Defines
#define REQUEST_PORT PORTD
#define REQUEST_DDR  DDRD
#define REQUEST_PIN  3
#define DATA_READ    PIND
#define DATA_PORT    PORTD
#define DATA_DDR     DDRD
#define DATA_PIN     2

#define MAX_SAMPLES    10
#define MAX_FAILURES   3731
#define PACKET_STORAGE 24   // At worst only 8 packets, but with you keypresses you can get more


// ----- Macros -----

#define READ_DATA         DATA_READ &   (1 << DATA_PIN) ? 0 : 1

#define REQUEST_DATA()  REQUEST_DDR &= ~(1 << REQUEST_PIN) // Start incoming keyboard transfer
#define    STOP_DATA()  REQUEST_DDR |=  (1 << REQUEST_PIN) // Stop incoming keyboard data



// ----- Variables -----

// Buffer used to inform the macro processing module which keys have been detected as pressed
volatile uint8_t KeyIndex_Buffer[KEYBOARD_BUFFER];
volatile uint8_t KeyIndex_BufferUsed;



// ----- Function Declarations -----

void processPacketValue( uint16_t packetValue );



// ----- Interrupt Functions -----

// XXX - None Required



// ----- Functions -----

// Setup
// This setup is very simple, as there is no extra hardware used in this scan module, other than GPIOs.
// To be nice, we wait a little bit after powering on, and dump any of the pending keyboard data.
// Afterwards (as long as no keys were being held), the keyboard should have a clean buffer, and be ready to go.
// (Even if keys were held down, everything should probably still work...)
inline void Scan_setup()
{
	// Setup the DATA pin
	DATA_DDR  &= ~(1 << DATA_PIN); // Set to input
	DATA_PORT |=  (1 << DATA_PIN); // Set to pull-up resistor

	// Setup the REQUEST pin
	REQUEST_PORT |= (1 << REQUEST_PIN); // Set to output
	STOP_DATA(); // Set the line high to stop incoming data
	REQUEST_DATA();

	DDRD |= (1 << 4);
	PORTD &= ~(1 << 4);

	// Message
	info_print("Pins Setup");

	// Reset the keyboard before scanning, we might be in a wierd state
	_delay_ms( 50 );
	//Scan_resetKeyboard();

	// Message
	info_print("Keyboard Buffer Flushed");
}


// Main Detection Loop
// The Univac-Sperry F3W9 has a convenient feature, an internal 8 key buffer
// This buffer is only emptied (i.e. sent over the bus) when the REQUEST line is held high
// Because of this, we can utilize the Scan_loop to do all of the critical processing,
//  without having to resort to interrupts, giving the data reading 100% of the CPU.
// This is because the USB interrupts can wait until the Scan_loop is finished to continue.
//
// Normally, this approach isn't taken, as it's easier/faster/safer to use Teensy hardware shift registers
//  for serial data transfers.
// However, since the Univac-Sperry F3W9 sends 20 bit packets (including the start bit), the Teensy
//  doesn't have a shift register large enough (9 bit max), to hold the data.
// So the line must be polled manually using CPU cycles
//
// Another interesting feature is that there are 2 data lines.
// Output and /Output (NOT'ted version).
// Not really useful here, but could be used for error checking, or eliminating an external NOT gate if
//  we were using (but can't...) a hardware decoder like a USART.
inline uint8_t Scan_loop()
{
	return 0;
	// Protocol Notes:
	// - Packets are 20 bits long, including the start bit
	// - Each bit is ~105 usecs in length
	// - Thus the average packet length is 2.205 msecs
	// - Each packet is separated by at least 240 usecs (during a buffer unload)
	// - While holding the key down, each packet has a space of about 910 usecs
	// - A max of 8 keys can be sent at once (note, the arrow keys seem use 2 packets each, and thus take up twice as much buffer)
	// - There is no timing danger for holding the request line, just that data may come in when you don't want it

	// Now that the scan loop has been entered, we don't have to worry about interrupts stealing
	//  precious cycles.
	REQUEST_DATA();

	// = Delays =
	//
	// For these calculations to work out properly, then Teensy should be running at 16 MHz
	// - 1 bit         : 105   usecs is 16 000 000 * 0.000105  =   1680 instructions
	// - Bit centering :  52.5 usecs is 16 000 000 * 0.0000525 =    840 instructions
	// - Delay         :   5   msecs is 16 000 000 * 0.005     = 80 000 instructions
	// - Microsecond   :   1   usec  is 16 000 000 * 0.000001  =     16 instructions
	//
	// Now, either I can follow these exactly, or based upon the fact that I have >840 tries to find the
	//  the start bit, and >1680 tries to read the subsequent bits, I have some "flex" time.
	// Knowing this, I can make some assumptions that because I'm only reading a total of 20 bits, and will
	//  be re-centering for each packet.
	// This will allow for less worrying about compiler optimizations (and porting!).

	// The basic idea is to find a "reliable" value for the start bit, e.g. read it ~10 times.
	// Using a for-loop and some addition counters, this should eat up approximately 20-30 instructions per read
	//  (very loose estimation).
	// So reading 10 * 30 instructions = 300 instructions, which is much less than 840 instructions to where the
	//  bit center is, but is close enough that further delays of ~>1680 instructions will put the next read
	//  within the next bit period.
	// This is all possible because interrupts are disabled at this point, otherwise, all of this reasoning
	//  would fall apart.
	// _delay_us is available to use, fortunately.

	// Input Packet Storage (before being processed)
	uint16_t incomingPacket[PACKET_STORAGE];
	uint8_t  numberOfIncomingPackets = 0;

	// Sample the data line for ~5 ms, looking for a start bit
	//  - Sampling every 1 usecs, looking for 10 good samples
	//  - Accumulated samples will dumped if a high is detected
	uint8_t  samples  = 0;
	uint16_t failures = 0;

	// Continue waiting for a start bit until MAX_FAILURES has been reached (~5ms of nothing)
	while ( failures <= MAX_FAILURES )
	{
		// Attempt to find the start bit
		while ( samples < MAX_SAMPLES )
		{
			// Delay first
			_delay_us( 1 );

			// If data is valid, increment
			if ( READ_DATA )
			{
				samples++;
			}
			// Reset
			else
			{
				samples = 0;
				failures++;

				// After ~5ms of failures, break the loop
				// Each failure is approx 5 instructions + 1 usec, or approximately 1.34 usec)
				// So ~3731 failures for ~5ms
				// Being exact doesn't matter, as this is just to let the other parts of the
				//  controller do some processing
				if ( failures > MAX_FAILURES )
					break;
			}
		}

		// If 10 valid samples of the start bit were obtained, 
		if ( samples >= MAX_SAMPLES )
		{
			// Clean out the old packet memory
			incomingPacket[numberOfIncomingPackets] = 0;

			// Read the next 19 bits into memory (bit 0 is the start bit, which is always 0)
			for ( uint8_t c = 1; c < 20; c++ )
			{
				// Wait until the middle of the next bit
				_delay_us( 105 );

				// Append the current bit value
				incomingPacket[numberOfIncomingPackets] |= (READ_DATA << c);
			}

			// Packet finished, increment counter
			numberOfIncomingPackets++;
		}
	}

	// Stop the keyboard input
	STOP_DATA();

	// Finished receiving data from keyboard, start packet processing
	for ( uint8_t packet = 0; packet < numberOfIncomingPackets; packet++ )
		processPacketValue( incomingPacket[packet] );

	return 0;
}

// Read in the Packet Data, and decide what to do with it
void processPacketValue( uint16_t packetValue )
{
	// = Packet Layout =
	//
	// A is the first bit received (bit 0), T is the last
	//
	//   |  Modifier?  |  ??   |   Scan Code   |
	//  A B C D E F G H I J K L M N O P Q R S T
	//
	// A      - Start bit
	//          - Always Low
	// B -> H - Modifier enabled bits
	//          - Each bit represents a different modifier "mode"
	//          - B -> Shift/Lock
	//          - C -> ??
	//          - D -> Func
	//          - E -> ??
	//          - F -> ??
	//          - G -> ??
	//          - H -> ??
	// I -> L - ?? No idea yet...
	//          - The bits change for some combinations, but not pattern has been found yet...
	//          - I -> ??
	//          - J -> ??
	//          - K -> ??
	//          - L -> ??
	// M -> T - Scan Code
	//          - Bits are organized from low to high (8 bit value)
	//          - M -> Bit 1
	//          - N -> Bit 2
	//          - O -> Bit 3
	//          - P -> Bit 4
	//          - Q -> Bit 5
	//          - R -> Bit 6
	//          - S -> Bit 7
	//          - T -> Bit 8

	// Separate packet into sections
	uint8_t scanCode  = (packetValue & 0xFF000) << 12;
	uint8_t modifiers = (packetValue & 0x000FE);
	uint8_t extra     = (packetValue & 0x00F00) << 8;

	// Debug Info
	char tmpStr1[3];
	char tmpStr2[3];
	char tmpStr3[3];
	hexToStr_op( scanCode, tmpStr1, 2 );
	hexToStr_op( modifiers, tmpStr2, 2 );
	hexToStr_op( extra, tmpStr3, 2 );
	dbug_dPrint( "Scancode: 0x", tmpStr1, " Modifiers: 0x", tmpStr2, " Extra: 0x", tmpStr3 );
	dbug_dPrint( "Packet: 0x", tmpStr2, tmpStr3, tmpStr1 );

	// TODO List
	// - Modifier keys
	// - Key Release mechanism

	// Compute Modifier keys
	// TODO

	// Deal with special scan codes
	switch ( scanCode )
	{
	default:
		//Macro_bufferAdd( scanCode ); TODO - Uncomment when ready for USB output
		break;
	}
}

// Send data
// NOTE: Does nothing with the Univac-Sperry F3W9
uint8_t Scan_sendData( uint8_t dataPayload )
{
	return 0;
}

// Signal KeyIndex_Buffer that it has been properly read
inline void Scan_finishedWithBuffer( uint8_t sentKeys )
{
	return;
}

// Signal that the keys have been properly sent over USB
// TODO
inline void Scan_finishedWithUSBBuffer( uint8_t sentKeys )
{
	/*
	uint8_t foundModifiers = 0;

	// Look for all of the modifiers present, there is a max of 8 (but only keys for 5 on the HASCI version)
	for ( uint8_t c = 0; c < KeyIndex_BufferUsed; c++ )
	{
		// The modifier range is from 0x80 to 0x8F (well, the last bit is the ON/OFF signal, but whatever...)
		if ( KeyIndex_Buffer[c] <= 0x8F && KeyIndex_Buffer[c] >= 0x80 )
		{
			// Add the modifier back into the the Key Buffer
			KeyIndex_Buffer[foundModifiers] = KeyIndex_Buffer[c];
			foundModifiers++;
		}
	}

	// Adjust the size of the new Key Buffer
	KeyIndex_BufferUsed = foundModifiers;
	*/
}

// Reset/Hold keyboard
// NOTE: Does nothing with the Univac-Sperry F3W9
void Scan_lockKeyboard( void )
{
}

// NOTE: Does nothing with the Univac-Sperry F3W9
void Scan_unlockKeyboard( void )
{
}

// Reset Keyboard
// - Holds the input read line high to flush the buffer
// - This does not actually reset the keyboard, but always seems brings it to a sane state
// - Won't work fully if keys are being pressed done at the same time
void Scan_resetKeyboard( void )
{
	// Initiate data request line, but don't read the incoming data
	REQUEST_DATA();

	// We shouldn't be receiving more than 8 packets (and maybe +1 error signal)
	// This is around 22 ms of data, so a delay of 50 ms should be sufficient.
	_delay_ms( 50 );

	// Stop request line
	STOP_DATA();
}

