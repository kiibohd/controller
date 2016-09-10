#!/usr/bin/env python3
'''
Host-Side Python Commands for TestIn Scan Module
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

import argparse
import os
import sys




### Decorators ###

## Print Decorator Variables
ERROR = '\033[5;1;31mERROR\033[0m:'
WARNING = '\033[5;1;33mWARNING\033[0m:'


### Classes ###

class Commands:
	'''
	Container class of commands available to controll the host-side KLL implementation
	'''

	def mapping( self ):
		'''
		Returns a dictionary of the function:command mappings
		'''
		pass


	def addScanCode( self, scan_code ):
		'''
		Adds a Scan Code to the internal KLL buffer
		'''
		pass

	def removeScanCode( self, scan_code ):
		'''
		Removes a Scan Code from the internal KLL buffer
		Ignored if the Scan Code was not in the buffer
		'''


class Callbacks:
	'''
	Container class of commands required byt the host-side KLL implementation
	'''

	def mapping( self ):
		'''
		Returns a dictionary of the function:command mappings
		'''
		pass



### Main Entry Point ###

if __name__ == '__main__':
	print( "{0} Do not call directly.".format( ERROR ) )
	sys.exit( 1 )

