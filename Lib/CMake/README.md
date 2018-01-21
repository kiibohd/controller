# Kiibohd Controller - CMake Support

General build and CMake files.


## CMake Files

* [arm.cmake](arm.cmake) - ARM build support.
* [avr.cmake](avr.cmake) - AVR build support.
* [build.cmake](build.cmake) - Defines actual build targets.
* [buildinfo.cmake](buildinfo.cmake) - Generates compile-time information.
* [FindCtags.cmake](FindCtags.cmake) - ctags detection module.
* [FindDFUSuffix.cmake](FindDFUSuffix.cmake) - dfu-suffix detection module.
* [FindLSB.cmake](FindLSB.cmake) - lsb_release detection for Linux.
* [host.cmake](host.cmake) - Host build support (Windows/Linux/macOS).
* [initialize.cmake](initialize.cmake) - Early build setup.
* [kll.cmake](kll.cmake) - KLL file searching and argument setup.
* [modules.cmake](modules.cmake) - Module support.


## Misc Files

* [dfuMessage](dfuMessage) - Secure DFU build-time message.
* [prependKey](prependKey) - Prepends the given key to a .dfu.bin file.
* [sizeCalculator](sizeCalculator) - Determines the flash usage and estimated ram usage.
* [writer](writer) - Outputs value to a specified file.

