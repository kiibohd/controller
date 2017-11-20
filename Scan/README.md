# Kiibohd Controller - Macro Modules

Macro Modules do any internal processing of signals received by the Scan Modules before sending them elsewhere.
The results may be re-directed back to a Scan Module sub-driver or to an Output Module.

For most devices this is where KLL itself is implemented.


## Modules

Brief descriptions of each of the modules.

### Input Club Supported

* [PartialMap](PartialMap) - Main portion of KLL implementation
* [PixelMap](PixelMap) - Pixel control portion of KLL implementation, not used by all Scan Modules.


### Deprecated

* [buffer](buffer) - Pre-KLL macro sub-system (long-deprecated, has some useful code left)

