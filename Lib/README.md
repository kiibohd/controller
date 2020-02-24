# Kiibohd Controller - Libraries

General code and libraries needed for MCU support.


## CMake Support

[CMake](CMake) support files can be found in the [CMake](CMake) directory.


## Linker Scripts

Linker scripts can be found in the [ld](ld) directory.


## Module Headers

* [Interrupts.h](Interrupts.h) - Handles MCU includes for interrupts.
* [MacroLib.h](MacroLib.h) - Handles MCU includes for Macro Modules.
* [MainLib.h](MainLib.h) - Handles MCU includes for main.c.
* [OutputLib.h](OutputLib.h) - Handles MCU includes for Output Modules.
* [ScanLib.h](ScanLib.h) - Handles MCU includes for Scan Modules.


## General Includes

* [atomic.h](atomic.h) - Atomic operation support.
* [buildvars.h](_buildvars.h) - Build-time variables (git and version information).
* [chip_version.h](chip_version.h) - Lookup dictionaries useful for auto-detecting MCUs.
* [clang.c](clang.c) - Clang specific functions.
* delay.[c](delay.c)/[h](delay.h) - Delay functionality.
* entropy.[c](entropy.c)/[h](entropy.h) - Random number seeding and generation.
* host.[c](host.c)/[h](host.h)/[py](host.py) - Host-side KLL build support.
* [mcu_compat.h](mcu_compat.h) - MCU compatibility defines.
* periodic.[c](periodic.c)/[h](periodic.h) - Periodic timer support.
* storage.[c](storage.c)/[h](storage.h) - Non-volatile storage driver.
* [sysview.h](sysview.h) - Segger defines for supported MCUs.
* time.[c](time.c)/[h](time.h) - Time calculation support.
* utf8.[c](utf8.c)/[h](utf8.h) - UTF-8 functions.


## Libraries

* [ASF](ASF) - Microchip/Atmel Advanced Software Framework
* [CMSIS](CMSIS) - Cortex Microcontroller Software Interface Standard from Microchip/Atmel
* [nRF5_SDK](nRF5_SDK) - Nordic nRF5 SDK


## MCU Specific

* arm_cortex.[c](arm_cortex.c)/[h](arm_cortex.h)/[kll](arm_cortex.kll) - ARM Cortex specific functions, registers and configuration.
* [kinetis.c](kinetis.c) - Kinetis-based MCU initialization and IRQ setup.
* [kinetis.h](kinetis.h) - Register definitions for Kinetis-based MCUs.
* [nrf5.c](nrf5.c) - nRF5-based MCU initialization and IRQ setup.
* [nrf5.h](nrf5.h) - Register definitions for nRF5-based MCUs.
* [sam.c](sam.c) - SAM-based MCU initialization and IRQ setup.
* [sam.h](sam.c) - Register definitions for Sam-based MCUs.


## Misc

* [pin_map.teensy3](pin_map.teensy3) - Pin mapping for Teensy 3.0/3.1/3.2

