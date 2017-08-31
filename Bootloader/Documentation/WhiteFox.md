# WhiteFox Bootloader

The WhiteFox bootloader was originally a direct copy of the [Infinity Ergodox](Infinity_Ergodox.md) bootloader.
At that time, there was no keyboard differentiation between bootloaders.
It is a [mk20dx256vlh7](mk20dx256vlh7) bootloader.


## History

* 2015 - Initial support
* 2016 - Upload support, descriptor cleanup, serial number
* 2017 - Secure dfu flashing, keyboard name inside descriptor and dfu info, reset to firmware with dfu-util


## Notes

* The flash sector size of the mk20dx256vlh7 is half the size of the flexRAM store size (dfu.h:USB_DFU_TRANSFER_SIZE)


## Specific Features

* Debug LED turns on when in bootloader mode
* Reset to firmware with Esc key (top left corner) switch S1
* [Secure DFU](SecureDFU.md)


## TODO

* Nothing, unless there are requests

