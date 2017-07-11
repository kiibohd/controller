#!/usr/bin/env python3
'''
HID-IO Test Cases for Host-side KLL
'''

# Copyright (C) 2017 by Jacob Alexander
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This file is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this file.  If not, see <http://www.gnu.org/licenses/>.

### Imports ###

import sys
import time
import interface as i

from common import (ERROR, WARNING, check, result, header)


### Test ###

# Reference to callback datastructure
data = i.control.data

# Drop to cli, type exit in the displayed terminal to continue
#i.control.cli()


## Loopback Tests ##
header("-- RawIO Loopback tests --")

i.control.cmd('setRawIOPacketSize')( 64 )
i.control.cmd('setRawIOLoopback')( True )

# Send basic test packet, 1 byte length, payload 0xAC
header("- Single byte packet payload -")
i.control.cmd('HIDIO_test_2_request')( 1, 0xAC )


# A single processing loop (receivves, then sends packets)
i.control.loop(1)

# Check contents of incoming buffer (should be empty in loopback mode)
print( "Incoming Buf:", data.rawio_incoming_buffer )
check( len( data.rawio_incoming_buffer ) == 0 )

# Check contents of outgoing buffer (should have packet in loopback mode)
print( "Outgoing Buf:", data.rawio_outgoing_buffer )
check( len( data.rawio_outgoing_buffer ) == 1 )
check( data.rawio_outgoing_buffer[0][0].len == 3 )
check( data.rawio_outgoing_buffer[0][0].type == 0 )
check( data.rawio_outgoing_buffer[0][0].cont == 0 )
check( data.rawio_outgoing_buffer[0][1] == 2 ) # Id check
check( len( data.rawio_outgoing_buffer[0][2] ) == 1 )
check( data.rawio_outgoing_buffer[0][2][0] == 0xAC ) # Payload check


# A single processing loop (receives, then sends packets)
i.control.loop(1)

# Check contents of incoming buffer (should be empty in loopback mode)
print( "Incoming Buf:", data.rawio_incoming_buffer )
check( len( data.rawio_incoming_buffer ) == 0 )

# Check contents of outgoing buffer (should have ACK packet in loopback mode)
print( "Outgoing Buf:", data.rawio_outgoing_buffer )
check( len( data.rawio_outgoing_buffer ) == 1 )
check( data.rawio_outgoing_buffer[0][0].len == 3 )
check( data.rawio_outgoing_buffer[0][0].type == 1 )
check( data.rawio_outgoing_buffer[0][0].cont == 0 )
check( data.rawio_outgoing_buffer[0][1] == 2 ) # Id check
check( len( data.rawio_outgoing_buffer[0][2] ) == 1 )
check( data.rawio_outgoing_buffer[0][2][0] == 0xAC ) # Payload check


# A single processing loop (receives, then sends packets)
i.control.loop(1)

# Check contents of incoming buffer (should be empty)
print( "Incoming Buf:", data.rawio_incoming_buffer )
check( len( data.rawio_incoming_buffer ) == 0 )

# Check contents of outgoing buffer (should be empty)
print( "Outgoing Buf:", data.rawio_outgoing_buffer )
check( len( data.rawio_outgoing_buffer ) == 0 )



# Send continued packet sequence (2 packets, payload length of 110 bytes, 64 byte packet length)
print("")
header("- Two packet continued payload -")
i.control.cmd('HIDIO_test_2_request')( 110, 0xAC )


# A single processing loop (receives, then sends packets)
i.control.loop(1)
header("Check data packet 1")

# Check contents of incoming buffer (should be empty in loopback mode)
print( "Incoming Buf:", data.rawio_incoming_buffer )
check( len( data.rawio_incoming_buffer ) == 0 )

# Check contents of outgoing buffer (should have packet in loopback mode)
print( "Outgoing Buf:", data.rawio_outgoing_buffer )
check( len( data.rawio_outgoing_buffer ) == 1 )
check( data.rawio_outgoing_buffer[0][0].len == 62 )
check( data.rawio_outgoing_buffer[0][0].type == 0 )
check( data.rawio_outgoing_buffer[0][0].cont == 1 )
check( data.rawio_outgoing_buffer[0][1] == 2 ) # Id check
check( len( data.rawio_outgoing_buffer[0][2] ) == 60 )
check( data.rawio_outgoing_buffer[0][2][0] == 0xAC ) # Payload check


# A single processing loop (receives, then sends packets)
i.control.loop(1)
header("Check data packet 1 ACK")

# Check contents of incoming buffer (should be empty in loopback mode)
print( "Incoming Buf:", data.rawio_incoming_buffer )
check( len( data.rawio_incoming_buffer ) == 0 )

# Check contents of outgoing buffer (should have an ACK packet in loopback mode)
print( "Outgoing Buf:", data.rawio_outgoing_buffer )
check( len( data.rawio_outgoing_buffer ) == 1 )
check( data.rawio_outgoing_buffer[0][0].len == 2 )
check( data.rawio_outgoing_buffer[0][0].type == 1 )
check( data.rawio_outgoing_buffer[0][0].cont == 0 )
check( data.rawio_outgoing_buffer[0][1] == 2 ) # Id check
check( len( data.rawio_outgoing_buffer[0][2] ) == 0 )


# A single processing loop (receives, then sends packets)
i.control.loop(1)
header("Check continued data packet 2")

# Check contents of incoming buffer (should be empty in loopback mode)
print( "Incoming Buf:", data.rawio_incoming_buffer )
check( len( data.rawio_incoming_buffer ) == 0 )

# Check contents of outgoing buffer (should have packet in loopback mode)
print( "Outgoing Buf:", data.rawio_outgoing_buffer )
check( len( data.rawio_outgoing_buffer ) == 1 )
check( data.rawio_outgoing_buffer[0][0].len == 52 )
check( data.rawio_outgoing_buffer[0][0].type == 4 )
check( data.rawio_outgoing_buffer[0][0].cont == 0 )
check( data.rawio_outgoing_buffer[0][1] == 2 ) # Id check
check( len( data.rawio_outgoing_buffer[0][2] ) == 50 )
check( data.rawio_outgoing_buffer[0][2][0] == 0xAC ) # Payload check


# A single processing loop (receives, then sends packets)
i.control.loop(1)
header("Check ACK packets")

# Check contents of incoming buffer (should be empty in loopback mode)
print( "Incoming Buf:", data.rawio_incoming_buffer )
check( len( data.rawio_incoming_buffer ) == 0 )

# Check contents of outgoing buffer (should have ACK packet in loopback mode)
print( "Outgoing Buf:", data.rawio_outgoing_buffer )
check( len( data.rawio_outgoing_buffer ) == 2 )
check( data.rawio_outgoing_buffer[0][0].len == 62 )
check( data.rawio_outgoing_buffer[0][0].type == 1 )
check( data.rawio_outgoing_buffer[0][0].cont == 1 )
check( data.rawio_outgoing_buffer[0][1] == 2 ) # Id check
check( len( data.rawio_outgoing_buffer[0][2] ) == 60 )
check( data.rawio_outgoing_buffer[0][2][0] == 0xAC ) # Payload check
check( data.rawio_outgoing_buffer[1][0].len == 52 )
check( data.rawio_outgoing_buffer[1][0].type == 4 )
check( data.rawio_outgoing_buffer[1][0].cont == 0 )
check( data.rawio_outgoing_buffer[1][1] == 2 ) # Id check
check( len( data.rawio_outgoing_buffer[1][2] ) == 50 )
check( data.rawio_outgoing_buffer[1][2][0] == 0xAC ) # Payload check


# A single processing loop (receives, then sends packets)
i.control.loop(1)
header("Check buffers are empty")

# Check contents of incoming buffer (should be empty)
print( "Incoming Buf:", data.rawio_incoming_buffer )
check( len( data.rawio_incoming_buffer ) == 0 )

# Check contents of outgoing buffer (should be empty)
print( "Outgoing Buf:", data.rawio_outgoing_buffer )
check( len( data.rawio_outgoing_buffer ) == 0 )



# Send continued packet sequence (3 packets, payload length of 160 bytes, 64 byte packet length)
print("")
header("- Three packet continued payload -")
i.control.cmd('HIDIO_test_2_request')( 160, 0xAC )


# A single processing loop (receives, then sends packets)
i.control.loop(1)
header("Check data packet 1")

# Check contents of incoming buffer (should be empty in loopback mode)
print( "Incoming Buf:", data.rawio_incoming_buffer )
check( len( data.rawio_incoming_buffer ) == 0 )

# Check contents of outgoing buffer (should have packet in loopback mode)
print( "Outgoing Buf:", data.rawio_outgoing_buffer )
check( len( data.rawio_outgoing_buffer ) == 1 )
check( data.rawio_outgoing_buffer[0][0].len == 62 )
check( data.rawio_outgoing_buffer[0][0].type == 0 )
check( data.rawio_outgoing_buffer[0][0].cont == 1 )
check( data.rawio_outgoing_buffer[0][1] == 2 ) # Id check
check( len( data.rawio_outgoing_buffer[0][2] ) == 60 )
check( data.rawio_outgoing_buffer[0][2][0] == 0xAC ) # Payload check


# A single processing loop (receives, then sends packets)
i.control.loop(1)
header("Check data packet 1 ACK")

# Check contents of incoming buffer (should be empty in loopback mode)
print( "Incoming Buf:", data.rawio_incoming_buffer )
check( len( data.rawio_incoming_buffer ) == 0 )

# Check contents of outgoing buffer (should have an ACK packet in loopback mode)
print( "Outgoing Buf:", data.rawio_outgoing_buffer )
check( len( data.rawio_outgoing_buffer ) == 1 )
check( data.rawio_outgoing_buffer[0][0].len == 2 )
check( data.rawio_outgoing_buffer[0][0].type == 1 )
check( data.rawio_outgoing_buffer[0][0].cont == 0 )
check( data.rawio_outgoing_buffer[0][1] == 2 ) # Id check
check( len( data.rawio_outgoing_buffer[0][2] ) == 0 )


# A single processing loop (receives, then sends packets)
i.control.loop(1)
header("Check data packet 2")

# Check contents of incoming buffer (should be empty in loopback mode)
print( "Incoming Buf:", data.rawio_incoming_buffer )
check( len( data.rawio_incoming_buffer ) == 0 )

# Check contents of outgoing buffer (should have packet in loopback mode)
print( "Outgoing Buf:", data.rawio_outgoing_buffer )
check( len( data.rawio_outgoing_buffer ) == 1 )
check( data.rawio_outgoing_buffer[0][0].len == 62 )
check( data.rawio_outgoing_buffer[0][0].type == 4 )
check( data.rawio_outgoing_buffer[0][0].cont == 1 )
check( data.rawio_outgoing_buffer[0][1] == 2 ) # Id check
check( data.rawio_outgoing_buffer[0][2][0] == 0xAC ) # Payload check
check( len( data.rawio_outgoing_buffer[0][2] ) == 60 )


# A single processing loop (receives, then sends packets)
i.control.loop(1)
header("Check data packet 2 ACK")

# Check contents of incoming buffer (should be empty in loopback mode)
print( "Incoming Buf:", data.rawio_incoming_buffer )
check( len( data.rawio_incoming_buffer ) == 0 )

# Check contents of outgoing buffer (should have an ACK packet in loopback mode)
print( "Outgoing Buf:", data.rawio_outgoing_buffer )
check( len( data.rawio_outgoing_buffer ) == 1 )
check( data.rawio_outgoing_buffer[0][0].len == 2 )
check( data.rawio_outgoing_buffer[0][0].type == 1 )
check( data.rawio_outgoing_buffer[0][0].cont == 0 )
check( data.rawio_outgoing_buffer[0][1] == 2 ) # Id check
check( len( data.rawio_outgoing_buffer[0][2] ) == 0 )


# A single processing loop (receives, then sends packets)
i.control.loop(1)
header("Check continued data packet 3")

# Check contents of incoming buffer (should be empty in loopback mode)
print( "Incoming Buf:", data.rawio_incoming_buffer )
check( len( data.rawio_incoming_buffer ) == 0 )

# Check contents of outgoing buffer (should have packet in loopback mode)
print( "Outgoing Buf:", data.rawio_outgoing_buffer )
check( len( data.rawio_outgoing_buffer ) == 1 )
check( data.rawio_outgoing_buffer[0][0].len == 42 )
check( data.rawio_outgoing_buffer[0][0].type == 4 )
check( data.rawio_outgoing_buffer[0][0].cont == 0 )
check( data.rawio_outgoing_buffer[0][1] == 2 ) # Id check
check( len( data.rawio_outgoing_buffer[0][2] ) == 40 )
check( data.rawio_outgoing_buffer[0][2][0] == 0xAC ) # Payload check


# A single processing loop (receives, then sends packets)
i.control.loop(1)
header("Check ACK packets")

# Check contents of incoming buffer (should be empty in loopback mode)
print( "Incoming Buf:", data.rawio_incoming_buffer )
check( len( data.rawio_incoming_buffer ) == 0 )

# Check contents of outgoing buffer (should have ACK packet in loopback mode)
print( "Outgoing Buf:", data.rawio_outgoing_buffer )
check( len( data.rawio_outgoing_buffer ) == 3 )
check( data.rawio_outgoing_buffer[0][0].len == 62 )
check( data.rawio_outgoing_buffer[0][0].type == 1 )
check( data.rawio_outgoing_buffer[0][0].cont == 1 )
check( data.rawio_outgoing_buffer[0][1] == 2 ) # Id check
check( len( data.rawio_outgoing_buffer[0][2] ) == 60 )
check( data.rawio_outgoing_buffer[0][2][0] == 0xAC ) # Payload check
check( data.rawio_outgoing_buffer[1][0].len == 62 )
check( data.rawio_outgoing_buffer[1][0].type == 4 )
check( data.rawio_outgoing_buffer[1][0].cont == 1 )
check( data.rawio_outgoing_buffer[1][1] == 2 ) # Id check
check( len( data.rawio_outgoing_buffer[1][2] ) == 60 )
check( data.rawio_outgoing_buffer[1][2][0] == 0xAC ) # Payload check
check( data.rawio_outgoing_buffer[2][0].len == 42 )
check( data.rawio_outgoing_buffer[2][0].type == 4 )
check( data.rawio_outgoing_buffer[2][0].cont == 0 )
check( data.rawio_outgoing_buffer[2][1] == 2 ) # Id check
check( len( data.rawio_outgoing_buffer[2][2] ) == 40 )
check( data.rawio_outgoing_buffer[2][2][0] == 0xAC ) # Payload check


# A single processing loop (receives, then sends packets)
i.control.loop(1)
header("Check buffers are empty")

# Check contents of incoming buffer (should be empty)
print( "Incoming Buf:", data.rawio_incoming_buffer )
check( len( data.rawio_incoming_buffer ) == 0 )

# Check contents of outgoing buffer (should be empty)
print( "Outgoing Buf:", data.rawio_outgoing_buffer )
check( len( data.rawio_outgoing_buffer ) == 0 )



# Invalid Id Test
print("")
header("- Invalid Id Test -");
i.control.cmd('HIDIO_invalid_65535_request')()


# A single processing loop (receivves, then sends packets)
i.control.loop(1)
header("Check data packet")

# Check contents of incoming buffer (should be empty in loopback mode)
print( "Incoming Buf:", data.rawio_incoming_buffer )
check( len( data.rawio_incoming_buffer ) == 0 )

# Check contents of outgoing buffer (should have packet in loopback mode)
print( "Outgoing Buf:", data.rawio_outgoing_buffer )
check( len( data.rawio_outgoing_buffer ) == 1 )
check( data.rawio_outgoing_buffer[0][0].len == 2 )
check( data.rawio_outgoing_buffer[0][0].type == 0 )
check( data.rawio_outgoing_buffer[0][0].cont == 0 )
check( data.rawio_outgoing_buffer[0][1] == 65535 ) # Id check
check( len( data.rawio_outgoing_buffer[0][2] ) == 0 )


# A single processing loop (receivves, then sends packets)
i.control.loop(1)
header("Check NAK packet")

# Check contents of incoming buffer (should be empty in loopback mode)
print( "Incoming Buf:", data.rawio_incoming_buffer )
check( len( data.rawio_incoming_buffer ) == 0 )

# Check contents of outgoing buffer (should have ACK packet in loopback mode)
print( "Outgoing Buf:", data.rawio_outgoing_buffer )
check( len( data.rawio_outgoing_buffer ) == 1 )
check( data.rawio_outgoing_buffer[0][0].len == 2 )
check( data.rawio_outgoing_buffer[0][0].type == 2 )
check( data.rawio_outgoing_buffer[0][0].cont == 0 )
check( data.rawio_outgoing_buffer[0][1] == 65535 ) # Id check
check( len( data.rawio_outgoing_buffer[0][2] ) == 0 )


# A single processing loop (receives, then sends packets)
i.control.loop(1)
header("Check buffers are empty")

# Check contents of incoming buffer (should be empty)
print( "Incoming Buf:", data.rawio_incoming_buffer )
check( len( data.rawio_incoming_buffer ) == 0 )

# Check contents of outgoing buffer (should be empty)
print( "Outgoing Buf:", data.rawio_outgoing_buffer )
check( len( data.rawio_outgoing_buffer ) == 0 )



# Worst-case Through-put Test (single byte payload)
print("")
header("- Worst-case Through-put Test -")
time_secs = 0.5
time_end = time.time() + time_secs
bytes_sent = 0
bytes_rcvd = 0
loops = 0
payload_len = 1

# Run through-put test for time duration
while time.time() < time_end:
	# Send packet
	i.control.cmd('HIDIO_test_2_request')( payload_len, 0x42 )
	i.control.loop(1, quiet=True)

	# Check packet
	check( len( data.rawio_outgoing_buffer ) == 1 )
	bytes_sent += data.rawio_outgoing_buffer[0][0].len
	i.control.loop(1, quiet=True)

	# Check ACK
	check( len( data.rawio_outgoing_buffer ) == 1 )
	bytes_rcvd += data.rawio_outgoing_buffer[0][0].len
	i.control.loop(1, quiet=True)

	# Check empty
	check( len( data.rawio_outgoing_buffer ) == 0 )
	loops += 1

header("Results")
print( " Payload: {0} bytes".format( payload_len ) )
print( " Time:    {0} secs".format( time_secs ) )
print( " Loops:   {0}".format( loops ) )
print( " Sent:    {0} bytes/sec ({1} bytes)".format( bytes_sent / time_secs, bytes_sent ) )
print( " Rcvd:    {0} bytes/sec ({1} bytes)".format( bytes_rcvd / time_secs, bytes_rcvd ) )



# Full Packet Through-put Test (max packet payload)
print("")
header("- Full Packet Through-put Test -")
time_secs = 0.5
time_end = time.time() + time_secs
bytes_sent = 0
bytes_rcvd = 0
loops = 0
payload_len = 60

# Run through-put test for time duration
while time.time() < time_end:
	# Send packet
	i.control.cmd('HIDIO_test_2_request')( payload_len, 0x13 )
	i.control.loop(1, quiet=True)

	# Check packet
	check( len( data.rawio_outgoing_buffer ) == 1 )
	bytes_sent += data.rawio_outgoing_buffer[0][0].len
	i.control.loop(1, quiet=True)

	# Check ACK
	check( len( data.rawio_outgoing_buffer ) == 1 )
	bytes_rcvd += data.rawio_outgoing_buffer[0][0].len
	i.control.loop(1, quiet=True)

	# Check empty
	check( len( data.rawio_outgoing_buffer ) == 0 )
	loops += 1

header("Results")
print( " Payload: {0} bytes".format( payload_len ) )
print( " Time:    {0} secs".format( time_secs ) )
print( " Loops:   {0}".format( loops ) )
print( " Sent:    {0} bytes/sec ({1} bytes)".format( bytes_sent / time_secs, bytes_sent ) )
print( " Rcvd:    {0} bytes/sec ({1} bytes)".format( bytes_rcvd / time_secs, bytes_rcvd ) )



# Continued Two Packet Through-put Test (max packet payload)
print("")
header("- Continued Two Packet Through-put Test -")
time_secs = 0.5
time_end = time.time() + time_secs
bytes_sent = 0
bytes_rcvd = 0
loops = 0
payload_len = 120

# Run through-put test for time duration
while time.time() < time_end:
	# Send packet
	i.control.cmd('HIDIO_test_2_request')( payload_len, 0x13 )
	i.control.loop(1, quiet=True)

	# Check packet
	check( len( data.rawio_outgoing_buffer ) == 1 )
	bytes_sent += data.rawio_outgoing_buffer[0][0].len
	i.control.loop(1, quiet=True)

	# Check initial ACK
	check( len( data.rawio_outgoing_buffer ) == 1 )
	bytes_rcvd += data.rawio_outgoing_buffer[0][0].len
	i.control.loop(1, quiet=True)

	# Check packet
	check( len( data.rawio_outgoing_buffer ) == 1 )
	bytes_sent += data.rawio_outgoing_buffer[0][0].len
	i.control.loop(1, quiet=True)

	# Check ACKs
	check( len( data.rawio_outgoing_buffer ) == 2 )
	bytes_rcvd += data.rawio_outgoing_buffer[0][0].len
	bytes_rcvd += data.rawio_outgoing_buffer[1][0].len
	i.control.loop(1, quiet=True)

	# Check empty
	check( len( data.rawio_outgoing_buffer ) == 0 )
	loops += 1

header("Results")
print( " Payload: {0} bytes".format( payload_len ) )
print( " Time:    {0} secs".format( time_secs ) )
print( " Loops:   {0}".format( loops ) )
print( " Sent:    {0} bytes/sec ({1} bytes)".format( bytes_sent / time_secs, bytes_sent ) )
print( " Rcvd:    {0} bytes/sec ({1} bytes)".format( bytes_rcvd / time_secs, bytes_rcvd ) )



# Continued Three Packet Through-put Test (max packet payload)
print("")
header("- Continued Three Packet Through-put Test -")
time_secs = 0.5
time_end = time.time() + time_secs
bytes_sent = 0
bytes_rcvd = 0
loops = 0
payload_len = 180

# Run through-put test for time duration
while time.time() < time_end:
	# Send packet
	i.control.cmd('HIDIO_test_2_request')( payload_len, 0x13 )
	i.control.loop(1, quiet=True)

	# Check packet
	check( len( data.rawio_outgoing_buffer ) == 1 )
	bytes_sent += data.rawio_outgoing_buffer[0][0].len
	i.control.loop(1, quiet=True)

	# Check initial ACK
	check( len( data.rawio_outgoing_buffer ) == 1 )
	bytes_rcvd += data.rawio_outgoing_buffer[0][0].len
	i.control.loop(1, quiet=True)

	# Check packet
	check( len( data.rawio_outgoing_buffer ) == 1 )
	bytes_sent += data.rawio_outgoing_buffer[0][0].len
	i.control.loop(1, quiet=True)

	# Check initial ACK
	check( len( data.rawio_outgoing_buffer ) == 1 )
	bytes_rcvd += data.rawio_outgoing_buffer[0][0].len
	i.control.loop(1, quiet=True)

	# Check packet
	check( len( data.rawio_outgoing_buffer ) == 1 )
	bytes_sent += data.rawio_outgoing_buffer[0][0].len
	i.control.loop(1, quiet=True)

	# Check ACKs
	check( len( data.rawio_outgoing_buffer ) == 3 )
	bytes_rcvd += data.rawio_outgoing_buffer[0][0].len
	bytes_rcvd += data.rawio_outgoing_buffer[1][0].len
	bytes_rcvd += data.rawio_outgoing_buffer[2][0].len
	i.control.loop(1, quiet=True)

	# Check empty
	check( len( data.rawio_outgoing_buffer ) == 0 )
	loops += 1

header("Results")
print( " Payload: {0} bytes".format( payload_len ) )
print( " Time:    {0} secs".format( time_secs ) )
print( " Loops:   {0}".format( loops ) )
print( " Sent:    {0} bytes/sec ({1} bytes)".format( bytes_sent / time_secs, bytes_sent ) )
print( " Rcvd:    {0} bytes/sec ({1} bytes)".format( bytes_rcvd / time_secs, bytes_rcvd ) )



# TODO
# - SYNC Test
# - Timeout test

result()

