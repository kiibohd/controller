#!/usr/bin/env python3
'''
CLI test case for Host-side KLL
'''

# Copyright (C) 2018 by Jacob Alexander
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
import threading
import time

import interface as i
import kiilogger

from common import (check, result, header)



### Setup ###

# Logger (current file and parent directory only)
logger = kiilogger.get_logger(os.path.join(os.path.split(__file__)[0], os.path.basename(__file__)))
logging.root.setLevel(logging.INFO)


# Reference to callback datastructure
data = i.control.data



### Basic CLI test ###

logger.info(header("-- Basic CLI test --"))

# Start background thread to handle cli processing
def background():
    '''
    Drop to cli, type exit in the displayed terminal to continue
    '''
    i.control.cli()

thread = threading.Thread(target=background)
thread.daemon = True
thread.start()

# Wait for interface to initialize
time.sleep(0.5)

# Retrieve tty interface
tty_interface = open(i.control.cli_name(), "w")

# Start background serial flush
# This is needed on macOS and likely some Linux distros
def background_flush(interface):
    '''
    Constantly unload the serial buffer.
    '''
    tty_flush = open(interface, "r")
    while True:
        tty_flush.read()
        time.sleep(0.1)

thread_flush = threading.Thread(target=background_flush, args=(i.control.cli_name(),))
thread_flush.daemon = True
thread_flush.start()

# Send version command, every command must end with a \r
tty_interface.write("help\r")
tty_interface.write("version\r")

# Clear periodic interval
i.control.cmd('setPeriodic')(0)
check(i.control.cmd('getPeriodic')() == 0)

# Set periodic interval
period_value = 1234
tty_interface.write("periodic {}\r".format(period_value))

# Wait for commands to register/process
time.sleep(0.5)

# Check periodic interval
check(i.control.cmd('getPeriodic')() == period_value)



### Basic echo validation test ###

logger.info(header("-- Basic echo validation test --"))

# Send echo command
compare_string = "MyCommand"
tty_interface.write("echo {}\r".format(compare_string))

# Wait for commands to register/process
time.sleep(0.5)

# Check last output line
# XXX (HaaTa) - May need to clean up how to retrieve the last command
check(i.control.cli_buffer()[-5] == compare_string)



### Results ###

result()

