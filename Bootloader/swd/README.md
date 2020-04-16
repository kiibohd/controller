# Kiibohd Bootloader SWD Module

This module is a repurposing of ARM's [DAPLink](https://github.com/ARMmbed/DAPLink).
It currently is only customized to work with ATSAM4S talking with nRF52 chips.

The reset line of the ATSAM4S is used as the external reset.
This required a few code changes in how the hardware reset was called.
The ATSAM4S external reset can only be asserted for a fixed amount of time.
There is no way to deassert this reset (at least not in a straight-forward way).

**Useful files**

- [IO_Config.h](IO_Config.h) - Handles SWDIO and SWCLK pin positions
- [DAP_config.h](DAP_config.h) - GPIO functionality (where the hw reset logic was changed). Also includes the clock speed, and SWD clock speed.
- [swd_host.h](swd_host.h) - Main list of SWD functions (this is where you should start)

