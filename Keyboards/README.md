Keyboard Compiler Scripts
=========================

Scripts for major keyboards designed using the Kiibohd firmware.
Please refer to `<script> --help` for specific details.

Refer to the [wiki](https://github.com/kiibohd/controller/wiki) on setting up your system for compiling.


Build Steps
-----------

* Try to build once to make sure your system is setup correctly
* Add any .kll files in the build directory you want
* Edit `<script>` to include the new .kll files
* Rebuild


Example
-------

```bash
./infinity.bash
```


Projects
--------

* [ergodox.bash](ergodox.bash] (Infinity Ergodox 2015/08/15, builds both sides)

    - [ergodox-l.bash](ergodox-l.bash) (Left  Side)
    - [ergodox-r.bash](ergodox-r.bash) (Right Side)

* [infinity.bash](infinity.bash) (Infinity Keyboard 2014/10/15 (MD1), defaults to Alphabet)

    - [infinity.alphabet.bash](infinity.alphabet.bash) (Alphabet Layout)
    - [infinity.hacker.bash](infinity.hacker.bash)     (Hacker   Layout)
    - [infinity.standard.bash](infinity.standard.bash) (Standard Layout)

* [infinity_led.bash](infinity_led.bash) (Infinity Keyboard with LED backlight support (MD1.1), defaults to Alphabet)

    - [infinity_led.alphabet.bash](infinity_led.alphabet.bash) (Alphabet Layout)
    - [infinity_led.hacker.bash](infinity_led.hacker.bash)     (Hacker   Layout)
    - [infinity_led.standard.bash](infinity_led.standard.bash) (Standard Layout)

* [k-type.bash](k-type.bash)     (K-Type Keyboard, Production)
* [whitefox.bash](whitefox.bash) (WhiteFox Keyboard, defaults to TrueFox)

    - [whitefox.aria.bash](whitefox.aria.bash)                       (Aria               Layout)
    - [whitefox.iso.bash](whitefox.iso.bash)                         (ISO                Layout)
    - [whitefox.jackofalltrades.bash](whitefox.jackofalltrades.bash) (Jack Of All Trades Layout)
    - [whitefox.truefox.bash](whitefox.truefox.bash)                 (TrueFox            Layout)
    - [whitefox.vanilla.bash](whitefox.vanilla.bash)                 (Vanilla            Layout)
    - [whitefox.winkeyless.bash](whitefox.winkeyless.bash)           (Winkeyless         Layout)


**Extra files**

* [cmake.bash](cmake.bash)                 (Used by the compilation script, does nothing on it's own)
* [common.bash](common.bash)               (Script used during CI and testing)
* [icpad.bash](icpad.bash)
* [ic_keyboards.bash](ic_keyboards.bash)   (Script used during CI and testing)
* [k-type.p2.bash](k-type.p2.bash)         (K-Type Prototype 2)
* [k-type.p3.bash](k-type.p3.bash)         (K-Type Prototype 3)
* [kira.bash](kira.bash)
* [others.bash](others.bash)               (Script used during CI and testing)
* [ps2_converter.bash](ps2_converter.bash) (PS2 Converter for Teensy 3.1/3.2)
* [template.bash](template.bash)           (Example template for new keyboards)


**Tests**

* [Testing/klltest.bash](Testing/klltest.bash)       (Automated KLL Trigger:Result test, uses input KLL files to build test)
* [Testing/macrotest.bash](Testing/macrotest.bash)   (Basic host-side unit-tests)
* [Testing/mk20test.bash](Testing/mk20test.bash)     (mk20dx128vlh7 test build)
* [Testing/mk22test.bash](Testing/mk22test.bash)     (mk22fx512avlh12 test build)
* [Testing/mk64test.bash](Testing/mk64test.bash)     (mk64fx512 Teensy 3.5 test build)
* [Testing/mk66test.bash](Testing/mk66test.bash)     (mk66fx1m0 Teensy 3.6 test build)
* [Testing/none.bash](Testing/none.bash)             (Sanity build using mk20dx128vlf5, useful base for MCU porting)
* [Testing/sam4sd32c.bash](Testing/sam4sd32c.bash)   (sam4sd32c test build)
* [Testing/template.bash](Testing/template.bash)     (Example test template)
* [Testing/uartout.bash](Testing/uartout.bash)       (Test mk20dx128vlf5 with uartOut Output Module)
* [Testing/usbxuart.bash](Testing/usbxuart.bash)     (Test mk20dx128vlf5 with USBxUART Output Module)


Example Usage
-------------

An example of how to use these scripts to compile KLL files from the [Ergodox configurator](https://input.club/configurator-ergodox/)

1. Download firmware from GUI.
1. On the cmd line do `git clone https://github.com/kiibohd/controller.git`
1. create a directory for your layout in `controller/kll/layouts` called "mine" (just a suggestion)
1. move *.kll files from firmware download into `controller/kll/layouts/mine`
1. update the `BuildPath`s and `DefaultMap` and `PartialMaps` in `ergodox-l.bash` and `ergodox-r.bash`

    2. `BuildPath="my_layout-l"`
    2. `DefaultMap="mine/MDErgo1-Default-0 lcdFuncMap"`
    2. `PartialMaps[1]="mine/MDErgo1-Default-1 lcdFuncMap"`
    2. Repeat `PartialMaps[]` with a sequential number for each layer you have
    2. `BuildPath="my_layout-r"`
    2. Take note that all layouts will need the `lcdFuncMap` added for ErgoDox

1. cd into `controller/Keyboards `
1. run `./ergodox.bash`
1. Upload `kiibohd.dfu.bin` to the keyboard.

