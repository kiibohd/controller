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
import interface as i

from common import (ERROR, WARNING, check, result)


### Test ###

# Reference to callback datastructure
data = i.control.data

# Drop to cli, type exit in the displayed terminal to continue
#i.control.cli()


## Loopback Tests ##
print("-- RawIO Loopback tests --")

i.control.cmd('setRawIOLoopback')( True )

# Send basic test packet, 1 byte length, payload 0xAC
print("- Single byte packet payload -")
i.control.cmd('HIDIO_test_2_request')( 1, 0xAC )


# A single processing loop (recieves, then sends packets)
i.control.loop(1)

# Check contents of incoming buffer (should be empty in loopback mode)
print( "Incoming Buf:", data.rawio_incoming_buffer )
check( len( data.rawio_incoming_buffer ) == 0 )

# Check contents of outgoing buffer (should have packet in loopback mode)
print( "Outgoing Buf:", data.rawio_outgoing_buffer )
check( len( data.rawio_outgoing_buffer ) == 1 )
check( data.rawio_outgoing_buffer[0][0].len == 2 )
check( data.rawio_outgoing_buffer[0][0].type == 0 )
check( data.rawio_outgoing_buffer[0][1] == 2 ) # Id check
check( data.rawio_outgoing_buffer[0][2][0] == 0xAC ) # Payload check


# A single processing loop (recieves, then sends packets)
i.control.loop(1)

# Check contents of incoming buffer (should be empty in loopback mode)
print( "Incoming Buf:", data.rawio_incoming_buffer )
check( len( data.rawio_incoming_buffer ) == 0 )

# Check contents of outgoing buffer (should have ACK packet in loopback mode)
print( "Outgoing Buf:", data.rawio_outgoing_buffer )
check( len( data.rawio_outgoing_buffer ) == 1 )
check( data.rawio_outgoing_buffer[0][0].len == 2 )
check( data.rawio_outgoing_buffer[0][0].type == 1 )
check( data.rawio_outgoing_buffer[0][1] == 2 ) # Id check
check( data.rawio_outgoing_buffer[0][2][0] == 0xAC ) # Payload check


# A single processing loop (recieves, then sends packets)
i.control.loop(1)

# Check contents of incoming buffer (should be empty)
print( "Incoming Buf:", data.rawio_incoming_buffer )
check( len( data.rawio_incoming_buffer ) == 0 )

# Check contents of outgoing buffer (should be empty)
print( "Outgoing Buf:", data.rawio_outgoing_buffer )
check( len( data.rawio_outgoing_buffer ) == 0 )



# Send continued packet sequence (2 packets)
print("- Two packet continued payload -")
#sys.exit(0)
#i.control.cmd('HIDIO_test_2_request')( 120, 0xAC )
# TODO







# TODO
# - Implement Python through-put meter (at driver level)
# - Send one packet
# - Redirect packet back
# - Validate packet (internal and by Python)
# - Receive ACK redirect back
# - Validate ACK (internal and by Python)

# TODO
# - Single packet
# - Single continued packet
# - 2 packets
# - Throughput test, 10 seconds, calculate throughput
# - Packet validation for all packet types
#   * Call
#   * Ack
#   * Nak
# - SYNC Test
# - Timeout test

result()

