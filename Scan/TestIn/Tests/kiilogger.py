'''
Logging Infrastructure for Kiibohd related Python

Adapted from https://stackoverflow.com/a/36294984
and
https://stackoverflow.com/questions/1343227/can-pythons-logging-format-be-modified-depending-on-the-message-log-level
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

import functools
import logging
import sys
import types



### Logger ###

class KiiFormatter(logging.Formatter):
    '''
    Conditional formatter used for logger.
    '''
    FORMATS = {
        logging.ERROR:   '\033[5;1;31mERROR\033[0m',
        logging.WARNING: '\033[1;33mWARNING\033[0m',
        logging.INFO:    '\033[1;32mINFO\033[0m',
        logging.DEBUG:   '\033[1;35mDEBUG\033[0m',

    }

    def format(self, record):
        '''
        Depending on the level, change the formatting.
        '''
        self._fmt = self.FORMATS.get(record.levelno, record.levelno) + ':%(name)s:%(funcName)s:%(lineno)s|%(message)s'
        self._style = logging.PercentStyle(self._fmt)

        return logging.Formatter.format(self, record)



### Setup ###

formatter = KiiFormatter()
handler = logging.StreamHandler(sys.stdout)
handler.setFormatter(formatter)
logging.root.addHandler(handler)
logging.root.setLevel(logging.DEBUG)



### Call Override ###

def _get_message(record):
    '''
    Replacement for logging.LogRecord.getMessage that uses the new-style string formatting
    for it's messages.
    '''
    msg = str(record.msg)
    args = record.args
    if args:
        if not isinstance(args, tuple):
            args = (args,)
        msg = msg.format(*args)
    return msg

def _handle_wrap(fcn):
    '''
    Wrap the handle function to replace the passed in record's getMessage function before
    calling handle.
    '''
    @functools.wraps(fcn)
    def handle(record):
        record.getMessage = types.MethodType(_get_message, record)
        return fcn(record)
    return handle

def get_logger(name=None):
    '''
    Get a logger instance that uses new-style string formatting
    '''
    log = logging.getLogger(name)
    if not hasattr(log, "_newstyle"):
        log.handle = _handle_wrap(log.handle)
    log._newstyle = True
    return log


