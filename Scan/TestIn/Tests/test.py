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

# Reference to callback datastructure
data = i.control.data

# Drop to cli, type exit in the displayed terminal to continue
#i.control.cli()

# Read current keyboard state
print( data.usb_keyboard() )


# Press key 0x00
i.control.cmd('addScanCode')( 0x00 )

# Run processing loop twice, needs to run twice in order to reach the Hold state
i.control.loop(2)

print( data.usb_keyboard() )
print( data.usb_keyboard_data )
check( set( data.usb_keyboard()[1] ) >= set([ 41 ]) ) # Check if [41] is a subset of the usb keyboard data

# Release key 0x00
i.control.cmd('removeScanCode')( 0x00 )

# Run processing loop once, only needs to transition from hold to release
i.control.loop(1)

print( data.usb_keyboard() )

result()

