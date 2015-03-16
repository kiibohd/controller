The Kiibohd Controller
======================

This is the main Kiibohd Firmware.
In general, this should be the **only** git repo you need to clone.
The [KLL](https://github.com/kiibohd/kll) git repo is automatically cloned during the build process.

Please refer to the [KLL](https://github.com/kiibohd/kll) repo or [kiibohd.com](http://kiibohd.com) for details on the KLL (Keyboard Layout Language) Spec.



Official Keyboards
------------------

* MD1 (Infinity Keyboard 2014/10/15)


The Kiibohd firmware supports a lot of other keyboards, but these are more obscure/custom/lesser known.



Compilation
-----------

Compilation is possible and tested on Windows/Linux/Mac.
Linux is the easiest using this [VM](https://s3.amazonaws.com/configurator-assets/ArchLinux_kiibohd_2015-02-13.tar.gz).

For most people refer [here](https://github.com/kiibohd/controller/tree/master/Keyboards).

For the full compilation details, please refer to the [wiki](https://github.com/kiibohd/controller/wiki).



Supported Microcontrollers
--------------------------

* Teensy 2.0 (Partial)
* Teensy 2.0++
* Teesny 3.0
* Teensy 3.1
* mk20dx128vlf5
* mk20dx256vlh7


Adding support for more microcontrollers is possible.
Some considerations for minimum specs:

* ~8  kB of SRAM
* ~25 kB of Flash

It's possible to port chips with lower specs, but will be more effort and have fewer features.



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



Contact
-------

If you really need to get a hold of HaaTa, email is best: `haata@kiibohd.com`

IRC is likely faster though.
`#geekhack@irc.freenode.net`
`#deskthority@irc.freenode.net`

