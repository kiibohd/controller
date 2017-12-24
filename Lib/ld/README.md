# Kiibohd Controller - Linker Scripts

Linker scripts used for compilation.
These are necessary for all but host-specific (Windows/Linux/macOS) builds.


## MCUs

* [mk20dx128vlf5.bootloader.ld](mk20dx128vlf5.bootloader.ld) - Bootloader for QFP-48 mk20dx128 48 MHz
* [mk20dx128vlf5.ld](mk20dx128vlf5.ld) - Firmware for QFP-48 mk20dx128 MHz
* [mk20dx128vlh7.bootloader.ld](mk20dx128vlh7.bootloader.ld) - Bootloader for QFP-64 mk20dx128 72 MHz
* [mk20dx128vlh7.ld](mk20dx128vlh7.ld) - Firmware for QFP-64 mk20dx128 72 MHz
* [mk20dx256vlh7.bootloader.ld](mk20dx256vlh7.bootloader.ld) - Bootloader for QFP-64 mk20dx256 72 MHz
* [mk20dx256vlh7.ld](mk20dx256vlh7.ld) - Firmware for QFP-64 mk20dx256 72 MHz
* [mk22fx512avlh12.bootloader.ld](mk22fx512avlh12.bootloader.ld) - Bootloader for QFP-64 mk22fx512a 120 MHz
* [mk22fx512avlh12.ld](mk22fx512avlh12.ld) - Firmware for QFP-64 mk22fx512a 120 MHz


## SAM

Sam-specific linker scripts.

* [sam4s8b.ld](sam4s8b.ld) - Firmware for QFP/BGA-64 sam4s8 120 MHz
* [sam4s8c.ld](sam4s8c.ld) - Firmware for QFP/BGA-100 sam4s8 120 MHz
* [sam4s16b.ld](sam4s16b.ld) - Firmware for QFP/BGA-64 sam4s16 120 MHz
* [sam4s16c.ld](sam4s16c.ld) - Firmware for QFP/BGA-100 sam4s16 120 MHz
* [sam4sa16b.ld](sam4sa16b.ld) - Firmware for QFP/BGA-64 sam4s16 120Hz
* [sam4sa16c.ld](sam4sa16c.ld) - Firmware for QFP/BGA-100 sam4s16 120 MHz
* [sam4sd16b.ld](sam4sd16b.ld) - Firmware for QFP/BGA-64 sam4s16 120 MHz
* [sam4sd16c.ld](sam4sd16c.ld) - Firmware for QFP/BGA-100 sam4s16 120 MHz
* [sam4sd32b.ld](sam4sd32b.ld) - Firmware for QFP/BGA-64 sam4s32 120 MHz
* [sam4sd32c.ld](sam4sd32c.ld) - Firmware for QFP/BGA-100 sam4s32 120 MHz


## Teensy

Teensy-specific linker scripts.

* [mk20dx128.ld](mk20dx128.ld) - Teensy 3.0
* [mk20dx256.ld](mk20dx256.ld) - Teensy 3.1/3.2
* [mk64fx512.ld](mk64fx512.ld) - Teensy 3.5
* [mk66fx1m0.ld](mk66fx1m0.ld) - Teensy 3.6

