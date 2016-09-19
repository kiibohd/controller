#!/usr/bin/env python3
'''
Host-Side Setup Routines for KLL
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

import importlib.util



### Decorators ###

## Print Decorator Variables
ERROR = '\033[5;1;31mERROR\033[0m:'
WARNING = '\033[5;1;33mWARNING\033[0m:'



### Setup ###

# Import necessary Python modules
ScanModule = "@CMAKE_SOURCE_DIR@/@ScanModulePath@/host.py"
OutputModule = "@CMAKE_SOURCE_DIR@/@OutputModulePath@/host.py"
LibModule = "@CMAKE_SOURCE_DIR@/Lib/host.py"

# Import built kiibohd library
libkiibohd_path = '@CMAKE_BINARY_DIR@/@CMAKE_SHARED_LIBRARY_PREFIX@kiibohd@CMAKE_SHARED_LIBRARY_SUFFIX@'

# Setup Library Module
spec = importlib.util.spec_from_file_location( "Lib", LibModule )
lib = importlib.util.module_from_spec( spec )
spec.loader.exec_module( lib )

# Initialize libkiibohd
control = lib.Control( ScanModule, OutputModule, libkiibohd_path )
control.process_args()
control.process()

