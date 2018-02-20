# Build Architecture

The main purpose of this git repository is to build firmware for ARM Cortex-based microcontrollers.
The main compiler is `arm-none-eabi-gcc`; however, there is some support for `clang` as well on some platforms (primarily Linux).

In addition, this repository supports host builds of key portions of the firmware.
The intention here is to be able to test key portions of the logic (e.g. KLL) while using tools such as Python, gdb and memory monitors.

The goal of this document is to describe how code is constructed from the various parts and where to look for various components.


## Build Process

1. Construct a CMake build expression (generally by calling a [build script](../Keyboards)).
1. Scan selected modules using `setup.cmake` files.

    1. Each module may define multiple sub-modules.
    1. Sub-modules may have KLL files that are overridden by the overall KLL layout files.
    1. Some modules override weak symbols in sub-modules, this is common with [Output Modules](../Output).
    1. Some modules define additional files to be generated (e.g. Python files for host builds).

1. Run [KLL compiler](https://github.com/kiibohd/kll) on all the scanned KLL files.
1. Compile source files into binary.


## Files

Each type of build has files dedicated to it.
ARM targets may have specific configurations depending on the vendor (e.g. Kinetis, SAM4S, etc.).
Host builds require additional infrastructure in order to handle loading the generated shared library.


### ARM Firmware Build Files

Here is a list of useful ARM specific files to consider.

* [Lib/CMake/arm.cmake](../Lib/CMake/arm.cmake) - `arm-none-eabi` specific CMake build specifications.
* [Lib/CMake/kinetis.cmake](../Lib/CMake/kinetis.cmake) - Kinetis ARM Cortex CMake build specifications.
* [Lib/CMake/nrf5.cmake](../Lib/CMake/nrf5.cmake) - Nordic nRF5 ARM Cortex CMake build specifications.
* [Lib/CMake/sam.cmake](../Lib/CMake/sam.cmake) - Microchip SAM ARM Cortex CMake build specifications.
* Lib/kinetis.[c](../Lib/kinetis.c)/[h](../Lib/kinetis.h) - Kinetis ARM Cortex specific registers and functions.
* Lib/nrf5.[c](../Lib/nrf5.c)/[h](../Lib/nrf5.h) - Nordic nRF5 ARM Cortex specific registers and functions.
* Lib/sam.[c](../Lib/sam.c)/[h](../Lib/sam.h) - Microchip SAM ARM Cortex specific registers and functions.


### Host Build Build Files

Useful files to consider when working with host builds.

* [Lib/CMake/host.cmake](../Lib/CMake/host.cmake) - Host build CMake build specifications.
* Lib/host.[c](../Lib/host.c)/[h](../Lib/host.h) - Host specific registers and functions.
* [Lib/host.py](../Lib/host.py) - General host specific Python library.
* [Scan/TestIn/host.py](../Scan/TestIn/host.py) - TestIn Scan module Python library.
* [Scan/TestIn/interface.py](../Scan/TestIn/interface.py) - Build template that defines where to locate all the host Python libraries.
* [Scan/TestIn/setup.cmake](../Scan/TestIn/setup.cmake) - Defines which Python files to template into the build.
* [Scan/TestIn/Tests/common.py](../Scan/TestIn/Tests/common.py) - Common test library for host-side tests.
* [Output/TestOut/host.py](../Output/TestOut/host.py) - TestOut Output module Python library.

