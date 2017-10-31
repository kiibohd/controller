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

* infinity.bash     (Infinity Keyboard 2014/10/15 (MD1))
* infinity_led.bash (Infinity Keyboard with LED backlight support (MD1.1))
* ergodox.bash      (Infinity Ergodox 2015/08/15)
* template.bash     (Example template for new keyboards)
* whitefox.bash     (WhiteFox Keyboard)


**Extra files**

* cmake.bash (Used by the compilation script, does nothing on it's own)

Example Usage
-------------

An example of how to use these scripts to compile KLL files from the [Ergodox configurator](https://input.club/configurator-ergodox/)

1. Download firmware from GUI.
1. On the cmd line do `git clone https://github.com/kiibohd/controller.git`
1. create a directory for your layout in `controller/kll/layouts` called "mine" (just a suggestion)
1. move *.kll files from firmware download into `controller/kll/layouts/mine`
1. update the `BuildPath`s and `DefaultMap` and `PartialMaps` in `ergodox.bash`
    
    2. `BuildPath="my_layout-l"`
    2. `DefaultMap="mine/MDErgo1-Default-0 lcdFuncMap"`
    2. `PartialMaps[1]="mine/MDErgo1-Default-1 lcdFuncMap"`
    2. Repeat `PartialMaps[]` with a sequential number for each layer you have
    2. `BuildPath="my_layout-r"`
    2. Take note that all layouts will need the `lcdFuncMap` added for ErgoDox
1. cd into `controller/Keyboards `
1. run `./ergodox.bash`
1. Upload `kiibohd.dfu.bin` to the keyboard.
