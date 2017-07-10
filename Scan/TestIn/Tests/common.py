#!/usr/bin/env python3
'''
Common functions for Host-side KLL tests
'''

# Copyright (C) 2016-2017 by Jacob Alexander
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

import inspect
import linecache
import sys



### Decorators ###

## Print Decorator Variables
ERROR = '\033[5;1;31mERROR\033[0m:'
WARNING = '\033[5;1;33mWARNING\033[0m:'



### Functions ###

test_pass = 0
test_fail = 0
test_fail_info = []

def check( condition ):
	'''
	Checks whether the function passed
	Adds to global pass/fail counters
	'''
	if condition:
		global test_pass
		test_pass += 1
	else:
		global test_fail
		test_fail += 1

		# Collect failure info
		frame = inspect.currentframe().f_back
		line_file = inspect.getframeinfo( frame ).filename
		line_no = inspect.getlineno( frame )
		line_info = linecache.getline( line_file, line_no )

		print( "{0} Test failed! \033[1;m{1}:\033[1;34m{2}\033[0m {3}".format( ERROR, line_file, line_no, line_info ), end='' )

		# Store info for final report
		test_fail_info.append( (frame, line_file, line_no, line_info) )


def result():
	'''
	Sums up test results and displays
	Exits Python with with success (0) if there were no test failures
	Exits with failure (1) otherwise
	'''
	print("----Results----")
	print("{0}/{1}".format(
		test_pass,
		test_pass + test_fail,
	) )

	if test_fail == 0:
		sys.exit( 0 )
	else:
		# Print report
		print("----Failed Tests----")
		for (frame, line_file, line_no, line_info) in test_fail_info:
			print( "\033[1;m{0}:\033[1;34m{1}\033[0m {2}".format( line_file, line_no, line_info ), end='' )

		sys.exit( 1 )


def header( val ):
	'''
	Bold prints a string to stdout
	'''
	print( "\033[1m{0}\033[0m".format( val ) )

