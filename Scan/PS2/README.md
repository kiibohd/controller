PS2
===

This is a Scan module for the PS/2 keyboard protocol. It is based on the (PS/2 Keyboard)[https://github.com/PaulStoffregen/PS2Keyboard] Library for Arduino/Teensy.

Both standard one byte scancodes are supported as are most of the extended scancodes.

Hardware
--------
The code expects the PS/2 clock pin on port B, pin 16 (Teensy pin 0). The data pin should be on port B, pin 17 (Teensy pin 1). You should be able t change these easily in the header file, but remember that if you change the port, you also have to rename the ISR function in the `scan_loop.c` file.

Limitations
-----------
No keyboard-led support. No pause or printscreen keys as they caused problems with the statemachine.

License
-------
PS2Keyboard.c is LGPL, the other files MIT. Copyright (c) 2017 Ivor Wanders