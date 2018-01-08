'''
Common functions for Host-side KLL tests
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

import inspect
import json
import linecache
import logging
import sys

import kiilogger



### Logger ###

logger = kiilogger.get_logger('Tests/common.py')



### Classes ###

class KLLTestRunner:
    '''
    Runs KLLTest classes

    Given a list of test classes objects, run the tests and query the result.
    '''
    def __init__(self, tests):
        '''
        Initialize KLLTestRunner objects

        @param tests: List of KLLTest objects
        '''
        # Intialize
        self.test_results = []
        self.tests = tests
        self.overall = None

        # Prepare each of the tests
        for test in self.tests:
            logger.info("Initializing: {}", test.__class__.__name__)
            if not test.prepare():
                logger.warning("'{}' failed prepare step.".format(test.__class__.__name__))

    def run(self):
        '''
        Run list of tests.

        @return: Overall test result.
        '''
        self.overall = True
        for test in self.tests:
            testname = test.__class__.__name__
            passed = True
            logger.info("Running: {}", testname)
            if not test.run():
                logger.error("'{}' failed.", testname)
                self.overall = False
                passed = False
            logger.info("{} has finished", testname)
            self.test_results.append((testname, passed, test.results()))

        return self.overall

    def results(self):
        '''
        Returns a list of KLLTestResult objects for the tests.
        Empty list if the self.run() hasn't been called yet.

        @return: List of KLLTestResult objects
        '''
        return self.test_results

    def passed(self):
        '''
        Returns a list of KLLTestResult objects that have passed.

        @return: List of passing KLLTestResult objects
        '''
        passed = []
        for test in self.test_results:
            if test[2]:
                passed.append(test)
        return passed

    def failed(self):
        '''
        Returns a list of KLLTestResult objects that have failed.

        @return: List of failing KLLTestResult objects
        '''
        failed = []
        for test in self.test_results:
            if not test[2]:
                failed.append(test)
        return failed


class KLLTestUnitResult:
    '''
    KLLTestUnitResult Container Class

    Contains the results of a single test
    '''
    def __init__(self, parent, result, unit, layer=None):
        '''
        Constructor

        @param parent: Parent class, i.e. the class of the test
        @param result: Boolean, True if test passed, False if it didn't
        @param unit:   Unit test object
        @param layer:  Layer under test, set to None if not used
        '''
        self.parent = parent
        self.result = result
        self.unit = unit
        self.layer = layer


class KLLTest:
    '''
    KLLTest base class

    Derive all tests from this class
    '''
    def __init__(self):
        import interface
        # kll.json helper file
        with open(interface.jsonInput, 'r') as jsoninput:
            self.klljson = json.load(jsoninput)

        # Reference to callback datastructure
        self.data = interface.control.data

        # Test Results
        self.testresults = []
        self.overall = None

    def prepare(self):
        '''
        Prepare to run test

        Does necessary initialization before attempting to run.
        Must be done before run.

        @return: True if ready to run test, False otherwise
        '''
        return True

    def run(self):
        '''
        Evaluate tests

        Iterates over prepared tests

        @return: Result of the test
        '''
        overall = True
        for test in self.testresults:
            logger.info("Sub-test: {} {}", test.unit.__class__.__name__, test.unit.key)

            # Run test, and record result
            test.result = test.unit.run()

            # Check if test has failed
            if not test.result:
                overall = False

        return overall

    def results(self):
        '''
        Returns a list of KLLTestResult objects
        These objects have the results of each test
        '''
        return self.testresults


class TriggerResultEval:
    '''
    Takes a trigger:result pair and processes it.

    Given a pair, it can be specified to do correct *or* incorrect scheduling.
    '''
    def __init__(self, json_key, json_entry, schedule=None):
        '''
        Constructor

        @param json_key:   String name for trigger:result pair (unique id for layer)
        @param json_entry: Dictionary describing trigger:result pair
        @param schedule:   Alternate Trigger schedule (Defaults to None), used in destructive testing
        '''
        self.key = json_key
        self.entry = json_entry
        self.schedule = schedule

        # PositionStep is the "cycle counter"
        # For each element of a sequence, this counter is incremented
        # When transitioning between trigger to result, the counter is incremented
        # It does not have to follow the internal KLL cycle counter as the schedule may be skewed due to timing constraints
        self.positionstep = 0

        # Set to True when the test has completed
        self.done = False

        # Prepare trigger
        self.trigger = TriggerEval(self.entry['trigger'], self.schedule)

        # Prepare result
        self.result = ResultMonitor(self.entry['result'])

    def step(self, positionstep=None):
        '''
        Evaluate a single step of the Trigger:Result pair

        @param positionstep: Start from a given positionstep (will set positionstep)

        @return: True if successful/valid execution, False for a failure
        '''
        # Check for positionstep
        if positionstep is not None:
            self.positionstep = positionstep

        # If positionstep is 0, reset trigger step position
        if self.positionstep == 0:
            self.trigger.reset()

        # Trigger Evaluation
        if not self.trigger.done():
            if not self.trigger.eval():
                return False

            # Reset result position, if the trigger has finished
            if self.trigger.done():
                logger.debug("{} Trigger Complete", self.__class__.__name__)
                self.result.reset()

        # Result Monitoring
        elif not self.result.done():
            if not self.result.monitor():
                return False

            # Mark Trigger:Result as done
            if self.result.done():
                logger.debug("{} Result Complete", self.__class__.__name__)
                self.done = True
                return True

        # Increment step position
        self.positionstep += 1

        return True

    def run(self):
        '''
        Evaluate/run Trigger:Result pair

        @return: True if successful, False if not
        '''
        while not self.done:
            logger.debug("{}:Step {}:{}", self.__class__.__name__, self.positionstep, self.key)

            if not self.step():
                return False

        return True


class Schedule:
    '''
    Handles KLL Schedule processing
    '''
    def __init__(self):
        '''
        TODO
        '''


class ScheduleElem:
    '''
    Scheduling for an individual element
    '''
    def __init__(self, json_entry):
        '''
        TODO

        By default, if no schedule is given, it is on press at the start of the cycle
        '''
        self.entry = json_entry

    def initial(self):
        '''
        TODO

        Initial evaluation of schedule.
        Resets internal tracking.

        @return: True if ready to evaluate/execute
        '''
        return True

    def update(self):
        '''
        TODO

        Updates the internal tracking

        @return: True if ready to evaluate/execute
        '''
        return True


class TriggerEval:
    '''
    Evaluates a KLL trigger from a trigger:result pair
    '''
    def __init__(self, json_entry, schedule=None):
        '''
        Constructor

        @param json_entry: Trigger json entry
        @param schedule:   Alternate Trigger schedule, if set to None will be determined from json_entry
        '''
        self.entry = json_entry
        self.schedule = schedule

        # Step determines which part of the sequence we are trying to evaluate
        self.step = 0

        # Build sequence of combos
        self.trigger = []
        for comboindex, combo in enumerate(self.entry):
            ncombo = []
            for elemindex, elem in enumerate(combo):
                # Determine schedule (use external schedule if specified)
                elemschedule = ScheduleElem(elem)
                if self.schedule is not None:
                    elemschedule = self.schedule[comboindex][elemindex]

                # Append element to combo
                ncombo.append(TriggerElem(elem, elemschedule))
            self.trigger.append(ncombo)

    def eval(self):
        '''
        Attempt to evaluate TriggerEval
        Only a single step.

        @return: True on valid execution, False if something unexpected ocurred
        '''
        # Attempt to evaluate each element in the current combo
        finished = True
        for elem in self.trigger[self.step]:
            if not elem.eval():
                finished = False

        # Increment step if finished
        if finished:
            self.step += 1

        # Always return True (even if not finished)
        # Only return False on an unexpected error
        return True

    def reset(self):
        '''
        Reset step position
        '''
        logger.debug("{} Reset", self.__class__.__name__)
        self.step = 0

    def done(self):
        '''
        Determine if the Trigger Evaluation is complete

        @return: True if complete, False otherwise
        '''
        # TODO
        return False


class ResultMonitor:
    '''
    Monitors a KLL result from a trigger:result pair
    '''
    def __init__(self, json_entry):
        '''
        Constructor

        @param json_entry: Result json entry
        '''
        self.entry = json_entry

        # Step determines which part of the sequence we are trying to monitor
        self.step = 0

        # Build sequence of combos
        self.result = []
        for comboindex, combo in enumerate(self.entry):
            ncombo = []
            for elemindex, elem in enumerate(combo):
                elemschedule = ScheduleElem(elem)
                # Append element to combo
                ncombo.append(ResultElem(elem, elemschedule))
            self.result.append(ncombo)

    def monitor(self):
        '''
        Monitors a single step of the Result.

        @return: True if step has completed, False if schedule is out of bounds (failed).
        '''
        # Monitor current step in the Result
        finished = True
        for elem in self.result[self.step]:
            if not elem.monitor():
                finished = False

        # Increment step if finished
        if finished:
            self.step += 1

        # TODO Determine if we are outside the bounds of the schedule
        # i.e. when to return False

        # Return true even if not finished
        return True

    def reset(self):
        '''
        Reset step position
        '''
        logger.debug("{} Reset", self.__class__.__name__)
        self.step = 0

    def done(self):
        '''
        Determine if the Result Monitor is complete

        @return: True if complete, False otherwise
        '''
        # TODO
        return True


class TriggerElem:
    '''
    Handles individual trigger elements and how to interface with libkiibohd
    '''
    def __init__(self, elem, schedule):
        '''
        Intializer

        @param elem:     Trigger element
        @param schedule: Trigger element conditions
        '''
        self.elem = elem
        self.schedule = schedule

    def eval(self):
        '''
        Evaluates TriggerElem
        Must satisify schedule in order to complete.

        @return: True if evaluated, False if not.
        '''
        # TODO (HaaTa) Handle scheduling
        import interface as i

        logger.debug("TriggerElem eval {} {}", self.elem, self.schedule)

        # Determine which kind trigger element
        # ScanCode
        if self.elem['type'] == 'ScanCode':
            # Press given ScanCode
            i.control.cmd('addScanCode')(self.elem['uid'])

        # Unknown TriggerElem
        else:
            logger.warning("Unknown TriggerElem {}", self.elem)

        return True

    def cleanup(self):
        '''
        Completes the opposing action for a scheduled TriggerElem eval

        @return: True, unless there were probablems completing operation
        '''
        # TODO (HaaTa) Handle scheduling
        import interface as i

        logger.debug("TriggerElem cleanup {} {}", self.elem, self.schedule)

        # Determine which kind trigger element
        # ScanCode
        if self.elem['type'] == 'ScanCode':
            # Press given ScanCode
            i.control.cmd('removeScanCode')(self.elem['uid'])

        # Unknown TriggerElem
        else:
            logger.warning("Unknown TriggerElem {}", self.elem)

        return True


class ResultElem:
    '''
    Handles individual result elements and how to monitor libkiibohd
    '''
    def __init__(self, elem, schedule):
        '''
        Intializer

        @param elem:     Result element
        @param schedule: Result element conditions
        '''
        self.elem = elem
        self.schedule = schedule

    def monitor(self):
        '''
        Monitors for ResultElem
        Must satisfy schedule in order to complete.

        @return: True if found, False if not.
        '''
        # TODO 
        return True



### Functions ###

test_pass = 0
test_fail = 0
test_fail_info = []

def fail_tests( number ):
    '''
    Update fail count
    '''
    # TODO Add info
    global test_fail
    test_fail += number

def pass_tests( number ):
    '''
    Update pass count
    '''
    global test_pass
    test_pass += number

def check( condition ):
    '''
    Checks whether the function passed
    Adds to global pass/fail counters
    '''
    # Only print stack (show full calling function) info if in debug mode
    if logger.isEnabledFor(logging.DEBUG):
        parentstack_info = inspect.stack()[-1]
        logger.debug("{} {}:{}",
            parentstack_info.code_context[0][:-1],
            parentstack_info.filename,
            parentstack_info.lineno
        )

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

        logger.error("Test failed! {}:{} {}", header(line_file), blued(line_no), line_info[:-1])

        # Store info for final report
        test_fail_info.append( (frame, line_file, line_no, line_info) )


def result():
    '''
    Sums up test results and displays
    Exits Python with with success (0) if there were no test failures
    Exits with failure (1) otherwise
    '''
    logger.info(header("----Results----"))
    logger.info("{0}/{1}".format(
        test_pass,
        test_pass + test_fail,
    ) )

    if test_fail == 0:
        sys.exit( 0 )
    else:
        # Print report
        logger.error(header("----Failed Tests----"))
        for (frame, line_file, line_no, line_info) in test_fail_info:
            logger.error( "{0}:{1} {2}", header(line_file), blued(line_no), line_info[:-1])

        sys.exit( 1 )


def header( val ):
    '''
    Emboldens a string for stdout
    '''
    val = "\033[1m{0}\033[0m".format( val )
    return val

def blued( val ):
    '''
    Emboldens a string blue for stdout
    '''
    val = "\033[1;34m{0}\033[0m".format( val )
    return val

