# Kiibohd Bootloader

This is the bootloader used by all Kiibohd designed keyboards.
It is based on the [McHCK](https://github.com/mchck/mchck) dfu bootloader.
Much of the code-base has been rewritten, and it supports nearly all of the DFU features.
Including some additional extras required by the Kiibohd infrastructure.


## Supported Microcontrollers

Currently only Freescale/NXP Kinetis MCUs are supported.
Porting to other MCUs is doable, but likely a lot of work.
See each chip for specific details.

* [mk20dx128vlf5](Documentation/mk20dx128vlf5.md)
* [mk20dx256vlh7](Documentation/mk20dx256vlh7.md)



## Determining Bootloader Version

From `lsusb -d 1c11: -v` while the bootloader is active, look for bcdDevice.
For example:
```
bcdDevice            2.73
```
This is a 16 bit hex value (627) which corresponds to the git revision number.



## Specific Device Support

Some devices have additional bootloader features.
See each device for specific details.

* [K-Type](Documentation/K-Type.md)
* [Infinity 60 / Infinity 60 LED](Documentation/Infinity_60.md)
* [Infinity Ergodox](Documentation/Infinity_Ergodox.md)
* [WhiteFox](Documentation/WhiteFox.md)



## Flashing Adapter

It is not possible to update the bootloader unless you have an external flashing adapter.
Depending on the MCU SWD and/or JTAG are supported.
JTAG is *much* faster than SWD, but requires more pins.

You can find supported adapter configurations on the [manufacturing wiki](https://github.com/kiibohd/manufacturing/wiki).



## Features

The Kiibohd Bootloader supports most (if not all) of the [DFU Spec](http://www.usb.org/developers/docs/devclass_docs/DFU_1.1.pdf), as well as some additional extra features.


### Firmware Flashing

Firmware flashing is known as binary download in DFU terminology.

```bash
dfu-util -D <firmware image>
```

Firmware upload is also supported (this means downloading the current firmware image from the device).
When doing the upload, the device does not actually understand the size of the file, so the exact size will be the same size as the amount of firmware flash.

```bash
dfu-util -U <destination image>
```

Not all versions of the bootloader support firmware uploading.


### Protected Region

In order to prevent users from bricking their device, the bootloader section is protected.
It can only be overwritten if an external flashing tool is used.

This is to prevent both intentional and unintentional behaviour by the user :D

* [mk20dx128vlf5](Documentation/mk20dx128vlf5.md) (8 kB - 2 sectors)
* [mk20dx256vlh7](Documentation/mk20dx256vlh7.md) (8 kB - 1 sector)


### Resetting to Firmware

If you don't want to flash a firmware, but find yourself in bootloader mode, you have two options:

* Unplug and re-plug device
* Run `dfu-util -e` which calls DFU Detach. This forces a device reset.
* Press a dedicated key programmed to reset for the specific device.
  + This is usually `Esc`.


### Bootloader USB Descriptor

```
Bus 003 Device 091: ID 1c11:b007
Device Descriptor:
  bLength                18
  bDescriptorType         1
  bcdUSB               2.00
  bDeviceClass            0
  bDeviceSubClass         0
  bDeviceProtocol         0
  bMaxPacketSize0        64
  idVendor           0x1c11
  idProduct          0xb007
  bcdDevice            2.70
  iManufacturer           1 Kiibohd
  iProduct                2 Infinity_Ergodox
  iSerial                 3 F421F00070E1000B002B401831374E45 - mk20dx256vlh7
  bNumConfigurations      1
  Configuration Descriptor:
    bLength                 9
    bDescriptorType         2
    wTotalLength           27
    bNumInterfaces          1
    bConfigurationValue     1
    iConfiguration          5 qq
    bmAttributes         0x80
      (Bus Powered)
    MaxPower              100mA
    Interface Descriptor:
      bLength                 9
      bDescriptorType         4
      bInterfaceNumber        0
      bAlternateSetting       0
      bNumEndpoints           0
      bInterfaceClass       254 Application Specific Interface
      bInterfaceSubClass      1 Device Firmware Update
      bInterfaceProtocol      2
      iInterface              4 Kiibohd DFU
      Device Firmware Upgrade Interface Descriptor:
        bLength                             9
        bDescriptorType                    33
        bmAttributes                       11
          Will Detach
          Manifestation Intolerant
          Upload Supported
          Download Supported
        wDetachTimeout                      0 milliseconds
        wTransferSize                    1024 bytes
        bcdDFUVersion                   1.01
Device Status:     0x0000
  (Bus Powered)
```


### Secure DFU Flashing

In order to protect the device from malicious firmware after putting the device into flashing mode, the Kiibohd Bootloader supports secure keys.
The key must be retrieved from the firmware, it cannot be queried from the bootloader.
The key is randomly generated each time the device is reset (including after flashing).

If there is no firmware on the device or you use the physical reset switch, the secure mode is disabled.
While secure mode is disabled, the binary file does not need to include a prepended key block (i.e. optional, bootloader can distinguish what a key block is as opposed to random data).

Please refer to [Secure DFU](Documentation/SecureDFU.md) for details, including how to add a key to your firmware binary image.


### UART Debug

The bootloader has a basic debug UART which outputs some useful information while it's running.
You'll need to setup your device for [external debugging](https://github.com/kiibohd/controller/wiki/External-Debugging).

* [mk20dx128vlf5](Documentation/mk20dx128vlf5.md) - UART0 (Tx0/Rx0)
* [mk20dx256vlh7](Documentation/mk20dx256vlh7.md) - UART2 (Tx2/Rx2)


### Serial Number and Device Information

The bootloader has the ability to query and show the MCU serial number.
The serial number is available in both the USB descriptor, as well as in the dfu serial field.

Device and MCU descriptions can also be modified to better distinguish the keyboard while in the bootloader.


### Reset to Bootloader on Hard Fault

If there is an immediate problem with the firmware image, the IRQs are setup such that the bootloader will reset the kebyoard and restart in bootloader mode.
This is particularily important if garbage is flashed to the keyboard.

The bootloader will be key-disabled as there was no chance to retrieve a key.
This only occurs on a Watch Dog reset, which can only occur in the first few cycles of executing the firmware.


### Overrun Protection

If more data than what is available on the flash device is sent via dfu (DFU spec does not send total file size initially, so this has to be determined after flashing), flashing will fail and return a DFU error.

