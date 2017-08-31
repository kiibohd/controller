# Infinity Ergodox Bootloader

The Infinity Ergodox bootloader was the first instance of porting the [mk20dx128vlf5](mk20dx128vlf5.md) bootloader to the [mk20dx256vlh7](mk20dx256vlh7).
The McHCK project did/does not support the mk20dx256vlh7, so this was a rather large effort.


## History

* 2015 - Initial support
* 2016 - Upload support, descriptor cleanup, serial number
* 2017 - Secure dfu flashing, keyboard name inside descriptor and dfu info, reset to firmware with dfu-util


## Notes

* The flash sector size of the mk20dx256vlh7 is half the size of the flexRAM store size (dfu.h:USB_DFU_TRANSFER_SIZE)


## Specific Features

* Debug LED turns on when in bootloader mode
* LCD backlight turns red when in bootloader mode
* Reset to firmware with switch S7
  + Top left key on the left hand, top right key on the right hand
* [Secure DFU](SecureDFU.md)


## TODO

* Nothing, unless there are requests

