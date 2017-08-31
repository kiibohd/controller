# Secure DFU

After reading about the UHK's [security firmware upgrade](https://ultimatehackingkeyboard.com/blog/2015/02/26/securing-firmware-upgrades), I got thinking about how to make the Kiibohd project more secure when it comes to upgrading firmware.
The main problem is that if some malicious firmware made it's way onto the USB device the attacker has a high level of control, possibly remote physical access.
When it comes to most systems, when you have physical access it's basically all over (fortunately this is hard to pull off).

The part that the UHK's Laszlo Monda brings up is that you should have a physical response in order to allow a jump to the bootloader.
This is a very good precaution, something which the kiibohd firmware has by default.

* Use the physical reset switch on the back of the keyboard
* Program a key to jump to the bootloader

For debug/developer purposes, it's also possible to jump to the bootloader remotely.
Fortunately, this is disabled by default, [flashModeEnabled](https://github.com/kiibohd/controller/blob/master/Output/pjrcUSB/capabilities.kll).


## Signed Firmware Images

Yet, while thinking about how to have some host-side program securely request a jump to the bootloader, I realized there was another issue.
Once at the bootloader, anything running on the host may flash the device at any time.
This means you could be tricked into jumping to the bootloader and a malicious program could flash new firmware without your knowing.

The easiest way to solve this issue is to require firmware images be signed with a key only known to the device itself.
Instead of pre-programming a key to the EEPROM or Flash, which may be compromised at some point and need to be updated; the bootloader randomly generates a 64 bit number each time the device is restarted.
This number is only available to the firmware and requires a hand-shake to occur (using HID-IO) before the firmware will provide the key back to the host system.

Once the key is availble, it is prepended as the first block of the firmware image.
Then, while flashing, the bootloader will validate against the currently set key.

* If the key is valid, the firmware flashes
* If the key is invalid, dfu-util errors and restarts the already loaded firmware

This scheme is not perfect, but it makes it that much harder to load malicious firmware onto the device.


## Unsecure Mode

It is possible to disable the key check inside the bootloader.


### Physical Reset Button

The physical reset button on the keyboard disables the secure key validation.
This button is your "last resort" button that should always work, no matter what the scenario.
Requiring an external cable as a last resort is bad design.


### No Firmware

If there is no firmware flashed to the device (only the bootloader), the key validation will be disabled.
This is the situation during manufacturing flashing.


### Watchdog / Invalid Firmware

If the firmware crashes immediately after flashing, the bootloader is jumped too and the key validation is disabled.
While this can be exploited by attackers, usually the user is actually trying to flashing firmware during this situation so the risk is rather low.
In addition, if bad firmware is flashed there is no way to retrieve a new key (a new key is generated on each reset).

**TODO** Investigate whether maintaining the old key when a watchdog timeout is detected is better.


### Unset Secure Key in Firmware

Since the bootloader is static, and cannot be updated without an external adapter, it's hard to setup the infrastructure for secure flashing all at the same time.
Instead, there is a way to disable secure key validation using the bootloader.
See [secureBootloaderEnabled](https://github.com/kiibohd/controller/blob/master/Output/pjrcUSB/capabilities.kll).



## TODO

* Finish HID-IO handshake sequence
* Add more descriptive error message during dfu-util flashing when validation fails
* Investigate watchdog and key validation scenarios

