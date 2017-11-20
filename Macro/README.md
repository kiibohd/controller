# Kiibohd Controller - Scan Modules

Scan Modules are the main control module within the Kiibohd Controller.
They are given preferential control over the main execution loop.
This is important for timing sensitive operations such as key matrix scanning and protocol converters.

There are two types of Scan Modules: main and sub-modules.

The main modules are what defines the behaviour of the execution loop, such as WhiteFox or K-Type.

The sub-modules are usually found inside the [Devices](Devices) folder.
Device sub-modules are drivers or sub-functionality that may be used by main Scan Modules.


## Modules

Brief descriptions of each of the modules.

### Input Club Supported

* [Infinity_60](Infinity_60) - Infinity 60% Scan Module
* [Infinity_60_LED](Infinity_60_LED) - Infinity 60% with LED support
* [Infinity_Ergodox](Infinity_Ergodox) - Infinity Ergodox Scan Module
* [Kira](Kira) - Kira Scan Module
* [K-Type](K-Type) - K-Type Scan Module
* [WhiteFox](WhiteFox) - WhiteFox Scan Module


### Sub-Modules

* [Devices](Devices) - Sub-module device drivers


### Other

* [CK3](CK3) - Ghosted matrix keyboard (2KRO)
* [Deprecated](Deprecated) - Old Scan Modules that are no longer maintained but may still have useful code.
* [PS2](PS2) - PS/2 Converter using a Teensy 3.2

