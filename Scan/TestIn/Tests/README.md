# Kiibohd Controller - TestIn Scan Module - Tests

These are TestIn tests used to evaluate Kiibohd Controller functionality without having to flash firmware to a keyboard.
They are useful in testing a broad range of features quickly.


## General

* [common.py](common.py) - General support code and classes for tests.
* [kiilogger.py](kiilogger.py) - Logging infrastructure.


## Tests

* [animation.py](animation.py) - Basic animation tests. Best used with a 32-bit color terminal (e.g. iterm2, Konsole, etc.).
* [animation2.py](animation2.py) - Quick animation tests, less comprehensive.
* [cli.py](cli.py) - CLI functionality test.
* [hidio.py](hidio.py) - HID-IO functionality and protocol tests.
* [kll.py](kll.py) - KLL functionality testing. Utilizes the input KLL layout configuration to build test cases automatically.
* [test.py](test.py) - Very simple sanity check for TestIn module.


## Writing Custom Tests

Writing your own Kiibohd Firmware tests is quite straight forward.
Please refer to the [Test](../../../Documentation/TestArchitecture.md) and [Build](../../../Documentation/BuildArchitecture.md) documents first to understand how the code is organized.


### Basic Format

This is the basic format of a test file.
Much of the functionality and command-line arguments are already handled for you.

Using the Python [ctypes](https://docs.python.org/3/library/ctypes.html) interface it's possible to access both C functions and variables.
Though remember, care must be taken to make sure the Python code understands the C data format and widths.

```Python
### Imports ###
import logging
import os

import interface as i
import kiilogger

from common import (check, result, header)


### Setup ###

# Setup logging module
logger = kiilogger.get_logger(os.path.join(os.path.split(__file__)[0], os.path.basename(__file__)))
logging.root.setLevel(logging.INFO) # Comment out for full debug output


# Reference to callback datastructure (access to kiibohd.so)
data = i.control.data


### Kiibohd Debugging Options ###

# Enabled macro debug mode - Enabled USB Output, show debug
i.control.cmd('setMacroDebugMode')(2)

# Enabled vote debug mode
i.control.cmd('setVoteDebugMode')(1)


### Basic Test ###

logger.info(header("Basic Test")) # Bolded debug info

# Press key 0x01
i.control.cmd('addScanCode')(0x01) # Uses a Python library Command to interface with kiibohd.so function

# Run one processing loop
# kiibohd.so is not free running like how it is when compiled as firmware.
# This is useful in timing difficult situations for testing.
i.control.loop(1)

# Check for 1 pending trigger (this defines pass/fail criteria)
check(len(data.pending_trigger_list()) == 1)

# Print out USB output data
logger.info(data.usb_keyboard())
logger.info(data.usb_keyboard_data)

# Release key 0x01
i.control.cmd('removeScanCode')(0x01)

# Run processing loop again to remove scan code
i.control.loop(1)

# Show that there is no USB data left
logger.info(data.usb_keyboard())

# Show results of the test
result()
```


### CLI Mode

It is also possible to access the Kiibohd Controller CLI shell in host-mode.
This can be done one of two ways.

1. Call any test script with `--cli` option.
2. Drop to a cli during any script.

```Python
i.control.cli()
```

When activated, a message will be displayed indicating which tty you can connect to.

```bash
: Tests/test.py -c
INFO:Lib/host.py:process_args:483|Enabling Virtual Serial Port
INFO:Lib/host.py:virtual_serialport_setup:596|/dev/pts/4
INFO:Lib/host.py:process:500|Host_init
```

Then in another terminal, open up the tty with a program such as `screen`.

```bash
screen /dev/pts/4
```


### Retrieving KLL Information

Often during a test it's useful to query information from the original KLL files.
Instead of having to write another parser, the KLL compiler has a JSON output file which contains detailed information computed during compilation called `kll.json`.
There is much more detailed information here than what is available within kiibohd.so.

This is how [kll.py](kll.py) automatically generates test cases given a set of KLL layout files.
Much of this implementation can be found in [common.py](common.py).

If additional information is required, it's possible to add more fields using the [kiibohd](https://github.com/kiibohd/kll/blob/master/emitters/kiibohd/kiibohd.py) KLL emitter.

