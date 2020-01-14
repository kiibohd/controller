# Kiibohd Controller - USB Output Module

The USB Output Module is a heavily modified implementation of the PJRC's Teensy USB keyboard stack.
Most of the code has either been optimized or changed at this point to better work with keyboards.

Key features of this module:

* Automatic switching of 6KRO (boot) and NKRO keyboard modes
  - Capability to toggle between modes
  - KLL define to set which mode is enable on power-on (NKRO as per the USB spec is the default)
  - Supports System Control codes (e.g. Sleep key)
  - Supports Consumer Control codes (e.g. Play, Next, Pause)
* USB Suspend support
  - Detects when the USB bus goes into suspend/low power mode and can trigger other modules
* USB Mouse support
  - Basic mouse movement and button support
  - Vertical and horizontal mouse wheel movement
* RawIO support
  - USB virtual Tx/Rx interface used for HID-IO
  - Debug cli


## KLL Features

List here are KLL options/capabilities available, see [capabilities.kll](capabilities.kll) for details.

* keyboardLocale
* usbIdle
* usbIdleForce
* usbProtocol
* secureBootloaderEnabled
* flashModeEnabled
* enableUSBSuspend
* enableUSBLowPowerNegotiation
* enableDeviceRestartOnUSBTimeout
* enableUSBResume


### USB Descriptor Enable/Disable

* Output_USBEnabled
* enableMouse
* enableKeyboard
* enableRawIO


### Capabilities

* consCtrlOut
* noneOut
* sysCtrlOut
* usbKeyOut
* mouseOut
* mouseWheelOut
* kbdProtocolBoot
* kbdProtocolNKRO
* toggleKbdProtocol
* flashMode


## Debugging USB Issues

Not all computers and devices handle USB the same way.
Sometimes it's hardware, but most of the time it's a bug in the USB chipset or host drivers, possibly even the BIOS.

Remember, you can always ask a question on [Kono Community](https://community.kono.store) (or better yet, check if someone else has already asked it!).


### Keyboard Doesn't Type in BIOS/PS4/etc.

Not all computers (in all modes) support the NKRO USB descriptor we use.
Even though it completely follows the USB spec.
The USB spec even has a requirement that when you use the boot/6kro/compatibility descriptor the host must request it.
Sadly, there are bad BIOSs, systems and OSs out there.

You have a few options to see if this is the problem.

* Map a NKRO/6KRO toggle to your keyboard. Can also be done from the configurator.

```
U"A" : toggleKbdProtocol();
```

* Force the keyboard to only use 6KRO/Boot Mode. Can also be done from the configurator.

```
usbProtocol = 0;
```


### Computer does not boot when keyboard is plugged in

There has been at least one report of a Lenovo X260 not being able to boot when a WhiteFox is plugged in.
It's not entirely certain as to why, but it seems that disabling some of the extra USB descriptors helps.
This does not affect typing, but will disable mouse control and the debug cli.

* Disable some of the USB descriptors. Can also be done from the configurator.

```
enableMouse = 0;
enableRawIO = 0;
```


### Keyboard cannot wake computer

It's really annoying when your keyboard can't wake your computer from sleep.
Unfortunately, this can be caused by a few things.

* Bug in the firmware! Yep, there may be a bug. My general request is that you try it on another computer (different OS, preferably) first, but usually that's enough info.
* Bug in the OS! Yep, macOS Sierra apparently had a bug that would prevent external keyboards from waking MacBooks. Not much can be done here other than reproducing with another keyboard and trying to get support from Apple...

