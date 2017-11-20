# Kiibohd Controller - Scan Devices Sub-Modules

Device drivers used by Scan Modules.


## Sub-Modules

Brief descriptions of each of the sub-modules.

### Input Club Supported

* [ISSILed](ISSILed) - ISSI LED Driver support module

    - ISSI 31FL371
    - ISSI 31FL372
    - ISSI 31FL373

* [MatrixARMPeriodic](MatrixARMPeriodic) - ARM scan module for key matrices
* [PortSwap](PortSwap) - Handles port swapping functionality (e.g. K-Type USB and Interconnect)
* [STLcd](STLcd) - Newhaven Display support module

    - NHD-C12832A1Z-FS(RGB)-FBW-3V

* [UARTConnect](UARTConnect) - Interconnect support module (e.g. Infinity Ergodox, K-Type)


### Deprecated

No longer supported.

* [MatrixARM](MatrixARM) - Old ARM scan module, deprecated in favour of [MatrixARMPeriodic](MatrixARMPeriodic)



