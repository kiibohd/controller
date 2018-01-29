#!/usr/bin/env python3
'''
Selt-test unit test for Host-side KLL

Uses the trigger:result syntax to self-test KLL file.
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
import sys

import interface as i
import kiilogger

from common import (
    check,
    fail_tests,
    header,
    pass_tests,
    result,
    KLLTest,
    KLLTestUnitResult,
    KLLTestRunner,
    TriggerResultEval,
)



### Setup ###

# Logger (current file and parent directory only)
logger = kiilogger.get_logger(os.path.join(os.path.split(__file__)[0], os.path.basename(__file__)))
logging.root.setLevel(logging.INFO)

# Reference to callback datastructure
data = i.control.data

# Enabled macro debug mode - Enabled USB Output, show debug
i.control.cmd('setMacroDebugMode')(2)

# Enabled vote debug mode
i.control.cmd('setVoteDebugMode')(1)

# Enable layer debug mode
i.control.cmd('setLayerDebugMode')(1)

# Enable pending trigger debug mode
i.control.cmd('setTriggerPendingDebugMode')(1)

# Enable output module debug mode
i.control.cmd('setOutputDebugMode')(2)



class SimpleLayerTest(KLLTest):
    '''
    Simple layer test

    For each layer, individually test each trigger, validating the capability and the output.

    This test is looking for evaluation correctness.
    '''
    def prepare(self):
        '''
        Prepare to run test

        Does necessary initialization before attempting to run.
        Must be done before run.

        @return: True if ready to run test, False otherwise
        '''
        # Iterate over each layer
        for layer, layerdata in sorted(self.klljson['Layers'].items()):
            logger.debug(header("Layer {}".format(layer)))
            for trigger, triggerdata in sorted(layerdata.items()):
                logger.debug("Trigger: {}", trigger)
                # Prepare a TriggerResultEval
                evalpair = TriggerResultEval(self, trigger, triggerdata)

                # Prepare TestResult
                testresult = KLLTestUnitResult(self, None, evalpair, layer)
                self.testresults.append(testresult)

        return True



### Test ###

testrunner = KLLTestRunner([
    SimpleLayerTest(),
    #SimpleLayerTest(tests=1, test=154),
    #SimpleLayerTest(tests=20, test=92),
])
testrunner.run()

# Results of test
result()

