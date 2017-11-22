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

* Infinity 60%
* Infinity 60% LED
* Infinity Ergodox
* K-Type
* WhiteFox


The Kiibohd firmware supports a lot of other keyboards, but these are more obscure/custom/lesser known.



Compilation
-----------

Compilation is possible and tested on Windows/Linux/Mac.
Linux is the easiest using this [VM](https://s3.amazonaws.com/configurator-assets/ArchLinux_kiibohd_2015-02-13.tar.gz).

For most people refer [here](https://github.com/kiibohd/controller/tree/master/Keyboards).

For the full compilation details, please refer to the [wiki](https://github.com/kiibohd/controller/wiki).



Supported Microcontrollers
--------------------------

* Teensy 2.0 (Deprecated)
* Teensy 2.0++ (Deprecated)
* Teensy 3.0
* Teensy 3.1/3.2
* mk20dx128vlf5
* mk20dx256vlh7


Adding support for more microcontrollers is possible.
Some considerations for minimum specs:

* ~8  kB of SRAM
* ~25 kB of Flash

It's possible to port chips with lower specs, but will be more effort and have fewer features.



Modules
-------

Scan <-> Macro <-> Output

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
