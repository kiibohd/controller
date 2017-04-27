#!/usr/bin/env python3
'''
HID-IO Python Interface
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

import os
import sys



### Decorators ###

## Print Decorator Variables
ERROR = '\033[5;1;31mERROR\033[0m:'
WARNING = '\033[5;1;33mWARNING\033[0m:'



### Variables ###

### Classes ###

class HIDIO:
	'''
	HID-IO Interface Class

	Detects available HID-IO compatible devices (or sockets).
	Establishes a connection with one or more of them.
	HIDIOClient is used to query the HIDIO instance.

	The HID-IO Interface deals with OS level control as well as inter-node communication.
	'''

class HIDIONode:
	'''
	HID-IO Interface Node

	Each HID-IO interface (physical or virtual) is given a HID-IO Node instance.
	It is used to maintain the I/O buffers as well as the protocol level responses of the HID-IO protocol.
	'''

### Main Entry Point ###

if __name__ == '__main__':
	print( "{0} Do not call directly.".format( ERROR ) )
	sys.exit( 1 )

