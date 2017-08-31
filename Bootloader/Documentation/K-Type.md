# K-Type Bootloader

Since the K-Type was in development for so long, it has gone through a variety of changes.
Due to the dual Type C connectors, additional logic is required inside the bootloader in order to determine which USB port is plugged in before starting the dfu code path.
Much of [Secure DFU](SecureDFU.md) was designed with the K-Type in mind.
It is a [mk20dx256vlh7](mk20dx256vlh7) bootloader.


## History

* 2015 - Started work, much of the serial number, upload support and descriptor cleanup was driven by the K-Type
* 2016 - Continued work
* 2017 - Initial version, includes Secure DFU


## Notes

* The flash sector size of the mk20dx256vlh7 is half the size of the flexRAM store size (dfu.h:USB_DFU_TRANSFER_SIZE)
* The K-Type has two USB ports, this requires the bootloader to scan which connector is plugged in.
  + This may result in the connection flapping if the OS is too slow
  + To aid in this, the flapping slows down on each try until a port is found
  + If both ports are plugged in, the first one to try the connection will get the USB connection.
    - If the USB port that received the connection is unplugged, the DFU will fail and the keyboard must be powered off before working again.


## Specific Features

* Debug LED turns on when in bootloader mode
* Reset to firmware with Esc key (top left corner) switch S1
* Dual Type C support for flashing firmware
* [Secure DFU](SecureDFU.md)


## TODO

* Nothing, unless there are requests

