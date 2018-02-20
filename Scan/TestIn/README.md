# Kiibohd Controller - TestIn Scan Module

The TestIn scan module is a host configuration of the Kiibohd Controller.
This means it does not run on a keyboard, but on the host computer itself.
It is most useful in unit testing various pieces of control logic within the Kiibohd Controller codebase.
The code is compiled into a shared library.

All system calls that would have been implemented using microcontroller hardware are replaced using Python callbacks.

Various test cases are available to be run from the [Tests](Tests) directory.
All of the Tests are configured and copied at build time, so don't try to run them from this directory.


## Directories

* [Tests](Tests) - Contains KLL TestIn unit and functional tests.


## Files

* [capabilities.kll](capabilities.kll) - KLL capabilities file for the TestIn Scan Module.
* [gdb](gdb) - Convenience script to call gdb with a given test script e.g. `./gdb Tests/kll.py`.
* [host.py](host.py) - Python commands and callbacks for the TestIn module.
* [interface.py](interface.py) - Build-time template to configure host side KLL. Used to set defaults in [Tests](Tests).
* scan_loop[.h](scan_loop.h)/[.c](scan_loop.c) - Basic scan loop to handle key input for the TestIn module.
* [scancode_map.kll](scancode_map.kll) - Default layout configuration for TestIn module.
* [setup.cmake](setup.cmake) - CMake configuration for TestIn module.

