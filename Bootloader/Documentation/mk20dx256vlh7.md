# mkd20dx256vlh7 Bootloader

This bootloader was ported from the [mk20dx128vlf5](mk20dx128vlf5.md) bootloader.
It receives the most support as most of Input Club's keyboards use the mk20dx256vlh7.
Currently the minimum protected region (8 kB) is sufficient for all the features in the bootloader with quite a bit of room to spare.


## History

* 2015 - Initial support
* 2016 - Upload support, descriptor cleanup, serial number
* 2017 - Secure dfu flashing, keyboard name inside descriptor and dfu info, reset to firmware with dfu-util


## Notes

* The flash sector size of the mk20dx256vlh7 is half the size of the flexRAM store size (dfu.h:USB_DFU_TRANSFER_SIZE)


## Specific Features

* Debug LED (PTA5) turns on when in bootloader mode
* [Secure DFU](SecureDFU.md)


## TODO

* Nothing, unless there are requests.

