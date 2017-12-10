#!/usr/bin/env python3
'''
Host-Side Setup Routines for KLL
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

import ast
import io

from importlib.abc import SourceLoader



### Decorators ###

## Print Decorator Variables
ERROR = '\033[5;1;31mERROR\033[0m:'
WARNING = '\033[5;1;33mWARNING\033[0m:'



### Loader ###

# Maintains compatibility to Python 3.2
class FileLoader:
    '''
    Modified version of FileLoader from Python 3.3
    '''
    def __init__( self, fullname, path ):
        self.name = fullname
        self.path = path

    def load_module( self, fullname ):
        return super( FileLoader, self ).load_module( fullname )

    def get_filename( self, fullname ):
        return self.path

    def get_data( self, path ):
        with io.FileIO( path, 'r' ) as file:
            return file.read()

class CustomLoader( FileLoader, SourceLoader ):
    '''
    Compatibility Class
    Used to work around importlib.util.module_from_spec() not being available.
    '''
    def get_code( self, fullname ):
        source = self.get_source( fullname )
        path = self.get_filename( fullname )
        parsed = ast.parse( source )
        return compile( parsed, path, 'exec', dont_inherit=True, optimize=0 )



### Setup ###

# Import necessary Python modules
ScanModule = "@CMAKE_SOURCE_DIR@/@ScanModulePath@/host.py"
OutputModule = "@CMAKE_SOURCE_DIR@/@OutputModulePath@/host.py"
LibModule = "@CMAKE_SOURCE_DIR@/Lib/host.py"

# Import built kiibohd library
libkiibohd_path = '@CMAKE_BINARY_DIR@/@CMAKE_SHARED_LIBRARY_PREFIX@kiibohd@CMAKE_SHARED_LIBRARY_SUFFIX@'

# Json helper
jsonInput = "@CMAKE_BINARY_DIR@/kll.json"

# Setup Library Module
lib = CustomLoader( "Lib", LibModule ).load_module("Lib")

# Initialize libkiibohd
control = lib.Control( ScanModule, OutputModule, libkiibohd_path, CustomLoader, jsonInput )
control.process_args()
control.process()

