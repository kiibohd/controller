# Keyboard Compiler Scripts

Scripts for major keyboards designed using the Kiibohd firmware.
Please refer to `<script> --help` for specific details.

Refer to the [wiki](../../../wiki) on setting up your system for compiling.
[Docker](../Dockerfiles) is the recommended environment for manually compiling firmware and running tests.


## Build Steps

* Try to build once to make sure your system is setup correctly
* Add any .kll files in the build directory you want
* Edit `<script>` to include the new .kll files
* Rebuild


## Example

```bash
./infinity.bash
```


## Projects

* [ergodox.bash](ergodox.bash) (Infinity Ergodox, builds both sides)

    - [ergodox-l.bash](ergodox-l.bash) (Left  Side)
    - [ergodox-r.bash](ergodox-r.bash) (Right Side)

* [infinity.bash](infinity.bash) (Infinity Keyboard, defaults to Alphabet)

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


### Extra files

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


### Tests

* [Testing/klltest.bash](Testing/klltest.bash)       (Automated KLL Trigger:Result test, uses input KLL files to build test)
* [Testing/macrotest.bash](Testing/macrotest.bash)   (Basic host-side unit-tests)
* [Testing/mk20test.bash](Testing/mk20test.bash)     (mk20dx128vlh7 test build)
* [Testing/mk22test.bash](Testing/mk22test.bash)     (mk22fx512avlh12 test build)
* [Testing/mk64test.bash](Testing/mk64test.bash)     (mk64fx512 Teensy 3.5 test build)
* [Testing/mk66test.bash](Testing/mk66test.bash)     (mk66fx1m0 Teensy 3.6 test build)
* [Testing/none.bash](Testing/none.bash)             (Sanity build using mk20dx128vlf5, useful base for MCU porting)
* [Testing/nrf52832.bash](Testing/nrf52832.bash)     (nrf52832 test build)
* [Testing/sam4sd32c.bash](Testing/sam4sd32c.bash)   (sam4sd32c test build)
* [Testing/template.bash](Testing/template.bash)     (Example test template)
* [Testing/uartout.bash](Testing/uartout.bash)       (Test mk20dx128vlf5 with uartOut Output Module)
* [Testing/usbxuart.bash](Testing/usbxuart.bash)     (Test mk20dx128vlf5 with USBxUART Output Module)


## Self-Testing a KLL Layout

It's possible to self-test KLL files before loading them onto your keyboard to look for any bugs or functional issues.
Normally this isn't required, but it is extremely helpful in isolating bugs to specific KLL expressions.
These tests are run automatically for the default layouts, but your own layout may have untested expressions (as it's not possible to test every permutation easily).

[Docker](../Dockerfiles) is the recommended testing environment.
However, as long as your environment is setup correctly running the self-test is quite easy.
It runs on Linux, macOS and Cygwin (though Cygwin is tricky to setup and doesn't work well on Travis-CI), just make sure you've installed all the dependencies mentioned in the [wiki](../../../wiki).

Make sure you can run `klltest.bash` first before trying out custom KLL layouts.
```bash
cd Testing
./klltest.bash
```
Running just the host-side KLL test script.
You can find the test scripts [here](../Scan/TestIn/Tests).
In most cases you'll want to use `kll.py`.
```bash
EnableHostOnlyBuild=true HostTest=kll.py ./whitefox.truefox.bash
```
If there are no errors, then the KLL layout files have no known bugs.
Now, if you do find an error, please file an [Issue](../../../issues), making sure to include your environment, command you ran, the error message as well as the custom KLL files that were used.

It's also possible to pre-check a firmware build using host-side KLL first.
```bash
EnableHostBuild=true HostTest=kll.py ./whitefox.truefox.bash
```


## Example Usage with Web-Configurator Layout Files

An example of how to use these scripts to compile KLL files from the [Ergodox configurator](https://input.club/configurator-ergodox/)

1. Download firmware from GUI.
1. On the cmd line do `git clone https://github.com/kiibohd/controller.git`
1. create a directory for your layout in `controller/kll/layouts` called "mine" (just a suggestion)
1. move *.kll files from firmware download into `controller/kll/layouts/mine`
1. update the `BuildPath`s and `DefaultMap` and `PartialMaps` in `ergodox-l.bash` and `ergodox-r.bash`

    1. `BuildPath="my_layout-l"`
    1. `DefaultMap="mine/MDErgo1-Default-0 lcdFuncMap"`
    1. `PartialMaps[1]="mine/MDErgo1-Default-1 lcdFuncMap"`
    1. Repeat `PartialMaps[]` with a sequential number for each layer you have
    1. `BuildPath="my_layout-r"`
    1. Take note that all layouts will need the `lcdFuncMap` added for ErgoDox

1. cd into `controller/Keyboards `
1. run `./ergodox.bash`
1. Upload `kiibohd.dfu.bin` to the keyboard.

