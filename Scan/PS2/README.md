PS/2
====

This is a Scan module for the PS/2 keyboard protocol. It is based on the [PS/2 Keyboard](https://github.com/PaulStoffregen/PS2Keyboard) Library for Arduino/Teensy.

Use-case / disclaimer
---------------------
I tried several PS2-USB converters to connect my beloved PS/2 Keyboard via USB but they all introduced noticable lag. As a last-ditch effort I tried to make one myself by combining the Kiibohd stack with Paul Stoffregen's PS2Keyboard library. The end result is a PS/2 to USB converter that introduces virtually no lag.

One disclaimer: on a few very rare (and non-reproducible) occassions I've experienced a key stuck in the 'depressed' state. Please check if this works well enough for your taste with your own keyboard before doing critical work.

Hardware
--------
The PS/2 clock pin is expected to be on port B, pin 16 (Teensy 3.x, pin 0). The data pin should be on port B, pin 17 (Teensy 3.x, pin 1). You should be able to change these easily in the header file, but remember that if you change the port, you also have to rename the ISR function in the `scan_loop.c` file.

Limitations
-----------
No support for the keyboard's leds; this requires writing to the keyboard which is not implemented at the moment. The pause and printscreen keys are not handled as they caused problems with the statemachine.

License
-------
PS2Keyboard.c is LGPL, the other files MIT. Copyright (c) 2017 Ivor Wanders