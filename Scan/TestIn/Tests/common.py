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

from collections import namedtuple

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
            logger.info("{}: {}", header("Running"), testname)
            if not test.run():
                logger.error("'{}' failed.", testname)
                self.overall = False
                passed = False
            logger.info("{} has finished", header(testname))
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
    def __init__(self, tests=None, test=0):
        '''
        KLLTest base class constructor

        @param tests: Specify number of sub-tests to run, None if all of them.
        @param test:  Specify a specific test to start from
        '''
        import interface
        # kll.json helper file
        self.klljson = interface.control.json_input

        # Reference to callback datastructure
        self.data = interface.control.data

        # Artificial limit for sub-tests
        self.test = test
        self.tests = tests

        # Test Results
        self.testresults = []
        self.overall = None

        # Used by sub-test children for debugging
        self.cur_test = None

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
        import interface

        overall = True
        curtest = 0
        for index, test in enumerate(self.testresults):
            # Skip tests before specified index
            if index < self.test:
                continue

            logger.info("{}:{} Layer({}) {} {} -> {}",
                header("Sub-test"),
                index,
                test.layer,
                blued(test.unit.__class__.__name__),
                test.unit.key,
                test.unit.entry['kll'],
            )

            # Set current test, used by sub-test children for debugging
            self.cur_test = index

            # Prepare layer setting
            # Run loop, to make sure layer is engaged already
            interface.control.cmd('lockLayer')(test.layer)

            # Run test, and record result
            test.result = test.unit.run()

            # Cleanup layer manipulations
            interface.control.cmd('clearLayers')()

            # Check if test has failed
            if not test.result:
                overall = False

            curtest += 1
            if self.tests is not None and curtest >= self.tests:
                logger.warning("Stopping at test #{}", curtest)
                break

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
    def __init__(self, parent, json_key, json_entry, schedule=None):
        '''
        Constructor

        @param parent:     Parent object
        @param json_key:   String name for trigger:result pair (unique id for layer)
        @param json_entry: Dictionary describing trigger:result pair
        @param schedule:   Alternate Trigger schedule (Defaults to None), used in destructive testing
        '''
        self.parent = parent
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
        self.trigger = TriggerEval(self, self.entry['trigger'], self.schedule)

        # Prepare result
        self.result = ResultMonitor(self, self.entry['result'])

    def step(self, positionstep=None):
        '''
        Evaluate a single step of the Trigger:Result pair

        @param positionstep: Start from a given positionstep (will set positionstep)

        @return: True if successful/valid execution, False for a failure
        '''
        import interface as i

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

            # Clear callback history, in case this is a sequence
            i.control.data.capability_history.unread()
            i.control.data.capability_history.prune()

        # Increment step position within libkiibohd
        i.control.loop(1)
        self.positionstep += 1

        # Cleanup step, if necessary
        self.trigger.cleanup()
        if not self.result.monitor_cleanup():
            logger.error("{} Cleanup Monitor failure", self.__class__.__name__)
            return False

        return True

    def run(self):
        '''
        Evaluate/run Trigger:Result pair

        @return: True if successful, False if not
        '''
        while not self.done:
            logger.debug("{}:Step {}:{}", self.__class__.__name__, self.positionstep, self.key)

            if not self.step():
                self.clean()
                return False

        # Cleanup after test
        self.clean()
        return True

    def clean(self):
        '''
        Cleanup between tests

        Capability history needs to be cleared between tests.
        '''
        import interface as i

        # Read all callbacks
        i.control.data.capability_history.unread()

        # Prune callback history
        i.control.data.capability_history.prune()


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
    def __init__(self, parent, json_entry, schedule=None):
        '''
        Constructor

        @param parent:     Parent object
        @param json_entry: Trigger json entry
        @param schedule:   Alternate Trigger schedule, if set to None will be determined from json_entry
        '''
        self.parent = parent
        self.entry = json_entry
        self.schedule = schedule

        # Step determines which part of the sequence we are trying to evaluate
        self.step = 0
        self.clean = -1
        self.cleaned = -1

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
                ncombo.append(TriggerElem(self, elem, elemschedule))
            self.trigger.append(ncombo)

    def eval(self):
        '''
        Attempt to evaluate TriggerEval
        Only a single step.

        @return: True on valid execution, False if something unexpected ocurred
        '''
        # Fail test if we have incremented too many steps
        if self.step > len(self.trigger):
            return False

        # Attempt to evaluate each element in the current combo
        finished = True
        for elem in self.trigger[self.step]:
            if not elem.eval():
                finished = False

        # Increment step if finished
        if finished:
            self.step += 1
            self.clean += 1

        # Always return True (even if not finished)
        # Only return False on an unexpected error
        return True

    def cleanup(self):
        '''
        Cleanup previously evaluated step

        Uses the previous step to determine what to cleanup.
        Does not cleanup if it's not necessary (i.e. no "double frees")
        '''
        # Don't bother doing double cleanup
        if self.cleaned == self.clean:
            return

        # Clean for the given step
        for elem in self.trigger[self.clean]:
            elem.cleanup()

        self.cleaned += 1

        # Reset the cleaning state
        if self.clean >= len(self.trigger):
            self.clean = -1
            self.cleaned = -1

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
        return self.step >= len(self.trigger)


class ResultMonitor:
    '''
    Monitors a KLL result from a trigger:result pair
    '''
    def __init__(self, parent, json_entry):
        '''
        Constructor

        @param parent:     Parent object
        @param json_entry: Result json entry
        '''
        self.parent = parent
        self.entry = json_entry

        # Step determines which part of the sequence we are trying to monitor
        self.step = 0
        self.clean = -1
        self.cleaned = -1

        # Build sequence of combos
        self.result = []
        for comboindex, combo in enumerate(self.entry):
            ncombo = []
            for elemindex, elem in enumerate(combo):
                elemschedule = ScheduleElem(elem)
                # Append element to combo
                ncombo.append(ResultElem(self, elem, elemschedule))
            self.result.append(ncombo)

    def monitor(self):
        '''
        Monitors a single step of the Result.

        @return: True if step has completed, False if schedule is out of bounds (failed).
        '''
        # Fail test if we have incremented too many steps
        if self.step > len(self.result):
            return False

        # Monitor current step in the Result
        finished = True
        for elem in self.result[self.step]:
            if not elem.monitor():
                return False

        # Increment step if finished
        if finished:
            self.step += 1
            self.clean += 1

        # TODO Determine if we are outside the bounds of the schedule
        # i.e. when to return False

        # Return true even if not finished
        return True

    def monitor_cleanup(self):
        '''
        Monitors the cleanup of the previous monitor step

        @return: True if cleanup was successful, False otherwise.
        '''
        # Don't bother doing double cleanup
        if self.cleaned == self.clean:
            return True

        # Clean for the given step
        clean_result = True
        for elem in self.result[self.clean]:
            if not elem.monitor_cleanup():
                clean_result = False

        self.cleaned += 1

        # Reset the cleaning state
        if self.clean >= len(self.result):
            self.clean = -1
            self.cleaned = -1

        return clean_result

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
        return self.step >= len(self.result)


class TriggerElem:
    '''
    Handles individual trigger elements and how to interface with libkiibohd
    '''
    def __init__(self, parent, elem, schedule):
        '''
        Intializer

        @param parent:   Parent object
        @param elem:     Trigger element
        @param schedule: Trigger element conditions
        '''
        self.parent = parent
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

    def __repr__(self):
        '''
        String representation of TriggerElem
        '''
        output = "{} {}".format(
            self.elem,
            self.schedule,
        )
        return output


class ResultElem:
    '''
    Handles individual result elements and how to monitor libkiibohd
    '''
    def __init__(self, parent, elem, schedule):
        '''
        Intializer

        @param parent:   Parent object
        @param elem:     Result element
        @param schedule: Result element conditions
        '''
        self.parent = parent
        self.elem = elem
        self.schedule = schedule
        self.validation = NoVerification(self)

        import interface as i

        # Lookup Capability (if necessary)
        elemtype = self.elem['type']
        self.name = i.control.json_input['CodeLookup'][elemtype]
        self.expected_args = None
        if self.name is None:
            # Capability type has the name and arg fields
            if elemtype == 'Capability':
                self.name = self.elem['name']
                self.expected_args = self.elem['args']
                new_args = []

                # Some args have types
                for arg in self.expected_args:
                    if isinstance(arg, dict):
                        new_args.append(arg['value'])

                # If any processing of args was necessary
                if len(new_args) > 0:
                    self.expected_args = new_args
        elif elemtype == 'Animation':
            # Expected arg needs to be looked up for Animations
            value = i.control.json_input['AnimationSettings'][self.elem['setting']]
            self.expected_args = [value]
        else:
            # Otherwise uid is used for the arg
            self.expected_args = [self.elem['uid']]

        logger.debug("ResultElem monitor {} {} {} {}", self.elem, self.schedule, self.name, self.expected_args)
        assert self.name is not None, "Invalid Result type {}".format(self.elem)

        # Determine if there is capability verification available
        if elemtype == 'Animation':
            self.validation = AnimationVerification(self)
        elif elemtype == 'USBCode':
            self.validation = USBCodeVerification(self)
        elif elemtype == 'ConsCode':
            self.validation = ConsumerCodeVerification(self)
        elif elemtype == 'SysCode':
            self.validation = SystemCodeVerification(self)
        elif elemtype == 'Capability':
            # Determine if general capability has verification available
            # Layer Verification
            if self.name in ['layerShift', 'layerLock', 'layerLatch']:
                self.validation = LayerVerification(self)

    def monitor_state(self, state):
        '''
        Monitors result state

        @param state: Value of state to monitor

        @returns: True if valid, False if not
        '''
        # TODO (HaaTa) Handle scheduling
        import interface as i

        # Lookup capability history, success if any capabilities match
        match = None
        for cap in i.control.data.capability_history.all():
            data = cap.callbackdata
            # Validate state and capability name
            if data.state == state and data.read_capability()[0] == self.name:
                # Validate args
                match_args = True
                for index in range(len(self.expected_args)):
                    if data.args[index] != self.expected_args[index]:
                        match_args = False
                        break

                # Determine if we've found a match
                if match_args:
                    match = data
                    logger.debug("Matched History {}", match)
                    break

        # Increment test count with pass/fail
        result = True
        if match is None:
            result = False
            # Print out capability history
            for cap in i.control.data.capability_history.all():
                logger.warning(cap)

        return check(result, "test:{} expected:{}({}) state({}) - found:{}".format(
            self.parent.parent.parent.cur_test,
            self.name,
            self.expected_args,
            state,
            match,
        ))

    def monitor(self):
        '''
        Monitors for ResultElem
        Must satisfy schedule in order to complete.

        @return: True if found, False if not.
        '''
        # TODO (HaaTa) Handle scheduling

        # Check capability state
        result = self.monitor_state(1)

        # Validate capability result
        result = result and self.validation.verify(1)

        return result

    def monitor_cleanup(self):
        '''
        Cleanup monitor for ResultElem

        @return: True if expected cleanup occured, False if not.
        '''
        # TODO (HaaTa) Handle scheduling

        # Check capability state
        result = self.monitor_state(3)

        # Validate capability result cleanup
        result = result and self.validation.verify(3)

        return result

    def __repr__(self):
        '''
        String representation of ResultElem
        '''
        output = "{} {} {}".format(self.elem, self.schedule, self.validation)
        return output


class CapabilityVerification:
    '''
    Base class for capability functional verification
    '''
    def __init__(self, parent):
        '''
        @param parent: ResultElem object used for introspection
        '''
        self.parent = parent

    def verify(self, state):
        '''
        Verify result capability

        @param: State of result

        @return: True if verification is successful, False otherwise
        '''
        logger.warning("verify not implemented for {}", self.__class__.__name__)
        return True


class NoVerification(CapabilityVerification):
    '''
    No verification implemented
    '''
    def verify(self, state):
        '''
        Ignore verification

        @param: State of result

        @return: Always True
        '''
        return True


class LayerVerification(CapabilityVerification):
    '''
    Layer capability verification

    Validates that a specified capability behaved as expected
    '''
    def verify(self, state):
        '''
        Verify Layer Function

        Depending on the layer function used, validate that the expected behaviour occured.

        @param: State of result

        @return: True if verification is successful, False otherwise
        '''
        import interface as i

        # Get current and previous layer states
        states = i.control.data.layer_history.last(3)
        cur = states[0]
        last = states[1]

        # Get expected layer manipulation
        type_lookup = {
            'layerShift': 1,
            'layerLatch': 2,
            'layerLock': 3,
        }
        expected_type = self.parent.name
        expected_layer = self.parent.expected_args[0]

        # Ignore release for lock
        if expected_type == 'layerLock' and state == 3:
            return True

        # Ignore press for latch
        if expected_type == 'layerLatch' and state == 1:
            return True

        # Ignore layer if invalid
        if expected_layer > len(cur.state):
            logger.info("Ignoring {}:{} - Invalid layer", expected_type, expected_layer)
            return True

        # Determine expected layer result, XOR last with type function layer
        # XXX This may get confused in some situations
        #     i.e. Two shift capabilities activated at the same time for the same layer
        computed = last.state[expected_layer] ^ (1 << type_lookup[expected_type] - 1)

        # Determine if layer should be in the stack or not
        in_stack = False
        if computed != 0:
            in_stack = True

        # Match expected with actual
        result = True
        if cur.state[expected_layer] != computed:
            result = False

        # Determine if in stack or not
        if in_stack:
            if expected_layer not in cur.stack:
                result = False
        else:
            if expected_layer in cur.stack:
                result = False

        # Debug if failed
        if not result:
            logger.warning("Prev Layer {}", last)
            logger.warning("Cur  Layer {}", cur)

        return check(result, "test:{} expectedlayer:{} expectedtype:{} expectedstate:{} instack:{} state:{} - foundstate:{}".format(
            self.parent.parent.parent.parent.cur_test,
            expected_layer,
            expected_type,
            computed,
            in_stack,
            state,
            cur.state[expected_layer],

        ))


class USBCodeVerification(CapabilityVerification):
    '''
    USBCode capability verification

    Validates that a specified capability behaved as expected
    '''
    def verify(self, state):
        '''
        Verify USB Code

        @param: State of result

        @return: True if verification is successful, False otherwise
        '''
        import interface as i

        # Get expected data (only one argument)
        expected = self.parent.expected_args[0]

        # Get USB Keyboard data
        data = i.control.data.usb_keyboard()

        logger.debug("Data: {}  Expected: {}", data, expected)

        # Check for expected data
        result = False
        match = False
        if expected in data.keyboardcodes:
            match = True

        # If the keycode is 0, return true regardless
        # There may not be any keycodes in this case (but there may be some due to other macros)
        if expected == 0:
            result = True

        # Off or Release
        if state == 0 or state == 3:
            if not match:
                result = True

        # Press or Hold
        elif state == 1 or state == 2:
            if match:
                result = True

        return check(result, "test:{} expectedusb:{} state({}) - found:{}".format(
            self.parent.parent.parent.parent.cur_test,
            expected,
            state,
            data,
        ))


class ConsumerCodeVerification(CapabilityVerification):
    '''
    ConsCode capability verification

    Validates that a specified capability behaved as expected
    '''
    def verify(self, state):
        '''
        Verify Consumer Code

        @param: State of result

        @return: True if verification is successful, False otherwise
        '''
        import interface as i

        # Get expected data (only one argument)
        expected = self.parent.expected_args[0]

        # Get USB Keyboard data
        data = i.control.data.usb_keyboard()

        logger.debug("Data: {}  Expected: {}", data, expected)

        # Check for expected data
        result = False
        match = False
        if expected == data.consumercode:
            match = True

        # Off or Release
        if state == 0 or state == 3:
            if not match:
                result = True

        # Press or Hold
        elif state == 1 or state == 2:
            if match:
                result = True

        return check(result, "test:{} expectedconsumer:{} state({}) - found:{}".format(
            self.parent.parent.parent.parent.cur_test,
            expected,
            state,
            data,
        ))


class SystemCodeVerification(CapabilityVerification):
    '''
    SysCode capability verification

    Validates that a specified capability behaved as expected
    '''
    def verify(self, state):
        '''
        Verify System Code

        @param: State of result

        @return: True if verification is successful, False otherwise
        '''
        import interface as i

        # Get expected data (only one argument)
        expected = self.parent.expected_args[0]

        # Get USB Keyboard data
        data = i.control.data.usb_keyboard()

        logger.debug("Data: {}  Expected: {}", data, expected)

        # Check for expected data
        result = False
        match = False
        if expected == data.systemcode:
            match = True

        # Off or Release
        if state == 0 or state == 3:
            if not match:
                result = True

        # Press or Hold
        elif state == 1 or state == 2:
            if match:
                result = True

        return check(result, "test:{} expectedsystem:{} state({}) - found:{}".format(
            self.parent.parent.parent.parent.cur_test,
            expected,
            state,
            data
        ))


class AnimationVerification(CapabilityVerification):
    '''
    Animation capability verification

    Validates that a specified capability behaved as expected
    '''
    def get_modifier_setting(self, animation, name):
        '''
        Retrieve modifier setting using the setting name

        @param animation: Json representation of the animation setting
        @param name:      Name of the animation setting modifier

        @return: NamedTuple of (arg, subarg), if not found will be (None, None)
        '''
        ntuple = namedtuple('AnimationSettingModifierArg', ['arg', 'subarg'])
        output = ntuple(None, None)
        for modifier in animation['modifiers']:
            if modifier['name'] == name:
                output = ntuple(modifier['value']['arg'], modifier['value']['subarg'])
                break

        return output

    def verify(self, state):
        '''
        Verify animation was added to the stack

        @param: State of result
        '''
        # No need to validate, other than during press
        if state != 1:
            return True

        import interface as i

        # Get expected index
        expected_index = self.parent.expected_args[0]

        # Determine settings from index
        animation = i.control.json_input['AnimationSettingsIndex'][expected_index]
        animation_id = i.control.json_input['AnimationIds'][animation['name']]
        framedelay = self.get_modifier_setting(animation, 'framedelay').arg
        frameoptions = animation['frameoptions']
        ffunc = self.get_modifier_setting(animation, 'ffunc').arg
        if ffunc is None:
            ffunc = 'off'
        pfunc = self.get_modifier_setting(animation, 'pfunc').arg
        if pfunc is None:
            pfunc = 'off'

        # Get animation stack
        stack_obj = i.control.cmd('animationStackInfo')()
        size = stack_obj.size
        stack = stack_obj.stack.contents

        # Search animation stack for animation
        # Match
        # - Animation index
        # - framedelay
        # - frameoptions
        # - ffunc
        # - pfunc
        result = False
        for index in range(size):
            elem = stack[index]
            if elem.index != animation_id:
                continue
            if elem.framedelay != framedelay:
                continue
            if elem.ffunc_lookup() != ffunc:
                continue
            if elem.pfunc_lookup() != pfunc:
                continue
            if set(elem.frameoption_lookup()) != set(frameoptions):
                continue

            # Matched
            result = True
            break

        # Print out stack if no match was found
        if not result:
            logger.warning("Animation not found, printing stack")
            for index in range(size):
                logger.warning(stack[index])

        return check(result, "test:{} expected:{}:{}".format(
            self.parent.parent.parent.parent.cur_test,
            animation_id,
            animation
        ))



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

def check(condition, info=None):
    '''
    Checks whether the function passed
    Adds to global pass/fail counters

    @param condition: Boolean condition
    @param info: Additional debugging information

    @returns: Result of boolean condition
    '''
    # Prepare info
    info_str = ""
    if info is not None:
        info_str = " - {}".format(info)

    # Only print stack (show full calling function) info if in debug mode
    if logger.isEnabledFor(logging.DEBUG):
        parentstack_info = inspect.stack()[-1]
        logger.debug("{} {}:{}{}",
            parentstack_info.code_context[0][:-1],
            parentstack_info.filename,
            parentstack_info.lineno,
            info_str,
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

        logger.error("Test failed! {}:{} {}{}",
            header(line_file),
            blued(line_no),
            line_info[:-1],
            info_str,
        )

        # Store info for final report
        test_fail_info.append( (frame, line_file, line_no, line_info, info_str) )

    return condition


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
        for (frame, line_file, line_no, line_info, info_str) in test_fail_info:
            logger.error( "{0}:{1} {2}{3}", header(line_file), blued(line_no), line_info[:-1], info_str)

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

