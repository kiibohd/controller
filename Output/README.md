# Kiibohd Controller - Output Modules

Output Modules handle outgoing/host control.
Generally dealing with outgoing signals and incoming control signals.
The most common usage is a USB HID driver stack.


## Modules

Brief descriptions of each of the modules.

### Input Club Supported

* [HID-IO](HID-IO) - HID-IO support module. Host-to-Device RPC mechanism.
* [pjrcUSB](pjrcUSB) - USB HID stack implementation based on PJRC's Teensy stack (heavily modified).
* [TestOut](TestOut) - Host-Side KLL output module.
* [uartOut](uartOut) - UART-only output module, useful during early MCU porting efforts.
* [usbMuxUart](usbMuxUart) - USB+UART output module, useful when debugging USB issues.

