# Kiibohd Controller - Macro Modules

Macro Modules are the main event controllers within the Kiibohd Controller.
This is where keyboard events are processed, queued and resulting actions are generated.

The two main areas for Macro Modules: KLL trigger:result and KLL pixel control.

PixelMap depends on PartialMap as PixelMap enables support for the Pixel Control aspects of KLL.


## Modules

Brief descriptions of each of the modules.

* [PartialMap](PartialMap) - trigger:result + layer implementation of KLL (Keyboard Layout Language)
* [PixelMap](PixelMap) - Pixel control implemenation of KLL

### Deprecated

* [buffer](buffer) - Original macro/mapping control for the Kiibohd Controller (pre-dates KLL).

