# Kiibohd Controller - Output Modules

Output Modules handle outgoing/host control.
Generally dealing with outgoing signals and incoming control signals.
The most common usage is a USB HID driver stack.


## Modules

Brief descriptions of each of the modules.

### Input Club Supported

* [HID-IO](HID-IO) - HID-IO support module. Host-to-Device RPC mechanism.
* [Interface](Interface) - Base interface used by main modules (sub-module)
* [TestOut](TestOut) - Host-Side KLL Output Module.
* [UARTOut](UARTOut) - UART-only output module, useful during early MCU porting efforts.
* [USB](USB) - USB stack implementation originally based on PJRC's Teensy USB stack (heavily modified).
* [USBxUART](USBxUART) - USB+UART output module, useful when debugging USB issues.

