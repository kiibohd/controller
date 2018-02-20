# Kiibohd Controller - TestOut Output Module

The TestOut output module is a host configuration of the Kiibohd Controller.
This means it does not run on a keyboard, but on the host computer itself.
It is most useful in unit testing various pieces of control logic within the Kiibohd Controller codebase.
The code is compiled into a shared library.

All system calls that would have been implemented using microcontroller hardware are replaced using Python callbacks.

For usage, please see the [TestIn](../../Scan/TestIn) scan module.


## Files

* [capabilities.kll](capabilities.kll) - KLL capabilities file for the TestOut Scan Module.
* [host.py](host.py) - Python commands and callbacks for the TestOut module.
* [output_com.c](output_com.c) - Stub functions for Output module.
* output_testout[.h](output_testout.h)/[.c](output_testout.c) - TestOut module implementation and overrides.
* [setup.cmake](setup.cmake) - CMake configuration for TestOut module.

