The Kiibohd Controller
======================

This is the main Kiibohd Firmware.
In general, this should be the **only** git repo you need to clone.
The [KLL](https://github.com/kiibohd/kll) git repo is automatically cloned during the build process.

Please refer to the [KLL](https://github.com/kiibohd/kll) repo or [kiibohd.com](http://kiibohd.com) for details on the KLL (Keyboard Layout Language) Spec.

[![Travis Status](https://travis-ci.org/kiibohd/controller.svg?branch=master)](https://travis-ci.org/kiibohd/controller) [![Appveyor Status](https://ci.appveyor.com/api/projects/status/67yk8tiyt88xmd15?svg=true)](https://ci.appveyor.com/project/kiibohd/controller/branch/master)

[![Visit our IRC channel](https://kiwiirc.com/buttons/irc.freenode.net/input.club.png)](https://kiwiirc.com/client/irc.freenode.net/#input.club)

[Visit our Discord Channel](https://discord.gg/GACJa4f)



Official Keyboards
------------------

* [Infinity 60%](https://input.club/devices/infinity-keyboard/)
* [Infinity 60% LED](https://input.club/devices/infinity-keyboard/)
* [Infinity Ergodox](https://input.club/devices/infinity-ergodox/)
* [K-Type](https://input.club/k-type/)
* [WhiteFox](https://input.club/whitefox/)


The Kiibohd firmware supports a lot of other keyboards, but these are more obscure/custom/lesser known.



Compilation
-----------

Compilation is possible and tested on Windows/Linux/macOS.
However, the recommended method is using a [Dockerfile](Dockerfiles).

Then, once you have a docker environment, you can select a build script [here](Keyboards).

To compile natively for your platform, refer to the [wiki](../../wiki).



Supported Microcontrollers
--------------------------

* [Teensy 2.0](https://www.pjrc.com/store/teensy.html) (Deprecated)
* [Teensy 2.0++](https://www.pjrc.com/store/teensypp.html) (Deprecated)
* [Teensy 3.0](https://www.pjrc.com/store/teensy3.html)
* Teensy [3.1](https://www.pjrc.com/store/teensy31.html)/[3.2](https://www.pjrc.com/store/teensy32.html)
* [mk20dx128vlf5](https://www.nxp.com/part/MK20DX128VLF5)
* [mk20dx256vlh7](https://www.nxp.com/part/MK20DX256VLH7)


Adding support for more microcontrollers is possible.
Some considerations for minimum specs:

* ~16 kB of SRAM
* ~64 kB of Flash

It's possible to port chips with lower specs, but will be more effort and have fewer features.



Modules
-------

```
           +------------------------------------------------+
           |     Lib                              Debug     |
           +------------------------------------------------+

           +-------------+  +-------------+  +--------------+
Input +---->    Scan     +--+    Macro    +--+    Output    +----> Output
Data       | +---------+ |  | +--------+  |  |              |      Data
           | | Devices +------+ Pixels |  |  |              |
           | +----+----+ |  | +--------+  |  |              |
           +------|------+  +-------------+  +--------------+
                  |
                  v

               Hardware
               Control

```

* [Debug Modules](Debug) - Debug support modules (e.g. cli)
* [Scan Modules](Scan) - Defines keyboard behaviour (e.g. K-Type)
* [Macro Modules](Macro) - KLL support modules
* [Output Modules](Output) - Defines what the keyboard talks over (e.g. USB)

General Code can be found in [Lib](Lib).



Bootloader
----------

A custom bootloader (based off of [McHCK](https://github.com/mchck/mchck)) is available.
This is only necessary when assembling a keyboard with a blank MCU or if you're attempting to re-flash your bootloader (requires external tools).

[Bootloader](Bootloader)



Contributions
-------------

Contributions welcome!

* Bug reports
* Documentation and Wiki editing
* Patches (including new features)



Licensing
---------

Licensing is done on a per-file basis.
Some of the source code is from [PJRC/Teensy](http://pjrc.com), other source code is from the [McHck Project](https://mchck.org).
Code written specifically for the Kiibohd Controller use the following licenses:

* MIT
* GPLv3
* Public Domain
