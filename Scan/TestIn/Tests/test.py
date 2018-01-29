#!/usr/bin/env python3
'''
Example test case for Host-side KLL
'''

# Copyright (C) 2016-2018 by Jacob Alexander
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

import logging
import os

import interface as i
import kiilogger

from common import (check, result, header)



### Setup ###

# Logger (current file and parent directory only)
logger = kiilogger.get_logger(os.path.join(os.path.split(__file__)[0], os.path.basename(__file__)))
logging.root.setLevel(logging.INFO)


# Reference to callback datastructure
data = i.control.data

# Enabled macro debug mode - Enabled USB Output, show debug
i.control.cmd('setMacroDebugMode')( 2 )

# Enabled vote debug mode
i.control.cmd('setVoteDebugMode')( 1 )



### Test ###

logger.info(header("-- 1 key test --"))

# Drop to cli, type exit in the displayed terminal to continue
#i.control.cli()

# Read current keyboard state
logger.info(data.usb_keyboard())


# Press key 0x00
i.control.cmd('addScanCode')( 0x01 )

# Run processing loop
i.control.loop(1)

logger.info(" TPending {}", data.pending_trigger_list())
logger.info(data.usb_keyboard())
logger.info(data.usb_keyboard_data)
check( set( data.usb_keyboard()[1] ) >= set([ 41 ]) ) # Check if [41] is a subset of the usb keyboard data

# Release key 0x00
i.control.cmd('removeScanCode')( 0x01 )

# Run processing loop once, only needs to transition from hold to release
i.control.loop(1)

logger.info(data.usb_keyboard())



### Test 3 keys at same time ###

logger.info(header("-- 3 key test --"))

# press keys
i.control.cmd('addScanCode')( 0x01 )
i.control.cmd('addScanCode')( 0x06 )
i.control.cmd('addScanCode')( 0x04 )

# Run processing loop
logger.info("Press State")
i.control.loop(1)
logger.info(data.usb_keyboard())
logger.info(" TPending {}", data.pending_trigger_list())
check( len( data.pending_trigger_list() ) == 3 )



# Run processing loop
logger.info("Hold State")
i.control.loop(1)
logger.info(data.usb_keyboard())
logger.info(" TPending {}", data.pending_trigger_list())
check( len( data.pending_trigger_list() ) == 3 )



# Release keys
i.control.cmd('removeScanCode')( 0x01 )
i.control.cmd('removeScanCode')( 0x06 )
i.control.cmd('removeScanCode')( 0x04 )
i.control.cmd('removeScanCode')( 0x05 ) # Extra key (purposefully not pressed earlier to simulate bug)

# Run processing loop
logger.info("Release State")
i.control.loop(1)
logger.info(data.usb_keyboard())
logger.info(" TPending {}", data.pending_trigger_list())
check( len( data.pending_trigger_list() ) == 0 )


### Combo Test ###

logger.info(header("-- 2 key combo test --"))
# TODO
# - Combo
# - Delayed combo
# - Sequence

result()

