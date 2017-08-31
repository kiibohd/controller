# mk20dx128vlf5 Bootloader

**NOTE** Many features are not currently ported to the [mk20dx128vlf5](mk20dx128vlf5.md) yet.
This is mostly for testing reasons.

This was the original bootloader used by the Kiibohd project and the [Infinity 60%](Infinity_60.md) keyboard.
Originally it was limited to the 4 kB protected region; however, now this change has been relaxed to 8 kB so many of the [mk20dx256vlh7](mk20dx256vlh7) specific features may now be enabled (with some testing).

For the most part this bootloader will only received new features if they are easy to port/test unless there are requests.


## History

* 2014 - Basic firmware flash support only
* 2015 - Small fixes, descriptor cleanups
* 2016 - Serial numbers
* 2017 - Expansion of region (8 kB), upload support, keyboard name inside descriptor and dfu info, reset to firmware with dfu-util


## Notes

* The flash sector size of the mk20dx128vlf5 is the same as the flexRAM store size (dfu.h:USB_DFU_TRANSFER_SIZE)
* McHCK used PTB16 instead of PTA19 as the debug LED


## Specific Features

* Debug LED (PTA19) turns on when in bootloader mode


## TODO

* Nothing, unless there are requests.

