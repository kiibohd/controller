#!/usr/bin/env python3
'''
Example test case for Host-side KLL
'''

# Copyright (C) 2016 by Jacob Alexander
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

import interface as i

from common import (ERROR, WARNING, check, result)



### Test ###

print("-- 1 key test --")

# Reference to callback datastructure
data = i.control.data

# Drop to cli, type exit in the displayed terminal to continue
#i.control.cli()

# Read current keyboard state
print( data.usb_keyboard() )


# Press key 0x00
i.control.cmd('addScanCode')( 0x01 )

# Run processing loop twice, needs to run twice in order to reach the Hold state
i.control.loop(2)

print( data.usb_keyboard() )
print( data.usb_keyboard_data )
check( set( data.usb_keyboard()[1] ) >= set([ 41 ]) ) # Check if [41] is a subset of the usb keyboard data

# Release key 0x00
i.control.cmd('removeScanCode')( 0x01 )

# Run processing loop once, only needs to transition from hold to release
i.control.loop(1)

print( data.usb_keyboard() )


### Test 3 keys at same time ###

print("-- 3 key test --")

# press keys
i.control.cmd('addScanCode')( 0x01 )
i.control.cmd('addScanCode')( 0x06 )
i.control.cmd('addScanCode')( 0x04 )

# Run processing loop
print("Press State")
i.control.loop(1)
print( data.usb_keyboard() )
print( " Triggers", data.trigger_list_buffer() )
print( " TPending", data.pending_trigger_list() )
check( len( data.pending_trigger_list() ) == 3 )
#print( " RPending", data.pending_result_list() )



# Run processing loop
print("Hold State")
i.control.loop(1)
print( data.usb_keyboard() )
print( " Triggers", data.trigger_list_buffer() )
print( " TPending", data.pending_trigger_list() )
check( len( data.pending_trigger_list() ) == 0 )



# Release keys
i.control.cmd('removeScanCode')( 0x01 )
i.control.cmd('removeScanCode')( 0x06 )
i.control.cmd('removeScanCode')( 0x04 )
i.control.cmd('removeScanCode')( 0x05 ) # Extra key (purposefully not pressed earlier to simulate bug)

# Run processing loop
print("Release State")
i.control.loop(1)
print( data.usb_keyboard() )


### Combo Test ###

print("-- 2 key combo test --")
# TODO
# - Combo
# - Delayed combo
# - Sequence

result()

