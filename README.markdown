The Kiibohd Controller
======================

This README is a bit long, just look at the sections you are interested in.
You only need to install avr-gcc if you want to build for the Teensy 2.0/2.0++.
Everything else needs an arm-none-eabi-gcc compiler (e.g. Infinity keyboard,
Teensy 3.0/3.1, McHCK).

Linux is the ideal build environment (preferably recent'ish). In the near
future I'll make available an Arch Linux VM for building/manufacturing tests.

Building on Mac should be ok for 99% of users with Macports or Homebrew. For
Homebrew, use `brew tap PX4/homebrew-px4` to get the arm-none-eabi-gcc installer.
The dfu Bootloader will not build correctly with the old version of
arm-none-eabi-gcc that Macports currently has (4.7.3). This is due to a bug
with lto (link time optimizations) which makes the resulting binary too big to
fit on the chip (must be less than 4096 Bytes).

Building on Windows should also be fine for 99% of users, but takes a bunch of
work to setup (because Windows is a crappy dev environment).  Cygwin is
currently required along with some non-Cygwin compilers and utilities (because
they are not available for Cygwin).  The dfu Bootloader will not build because
of a Make 3.81+ bug/feature that removed support for non-Unix (Windows)
filenames as dependencies of targets.  If you [replace the version of Make in
Cygwin](http://stackoverflow.com/questions/601516/cygwin-make-error-target-pattern-contains-no)
it should work.  However, make sure that the flash size is no larger than 4096
Bytes or the bootloader will not work. Things will likely break if there are
**SPACES IN YOUR PATHS**. I install cygwin to `C:\cygwin64`.  If you are brave
and have programming knowledge, I will accept patches to fix any issues
regarding spaces in paths.

Please give authors credit for modules used if you use in a distributed
product :D


General Dependencies
--------------------

Below listed are the Arch Linux pacman names, AUR packages may be required.

These depend a bit on which targets you are trying to build, but the general
one:

- cmake (2.8 and higher)
- git
- ctags (recommended, not required)
- python3
- libusb1.0 (and -devel)
- make

AVR Specific (Teensy 1.0/++,2.0/++) (try to use something recent, suggested
versions below)

- avr-gcc      (~4.8.0)
- avr-binutils (~2.23.2)
- avr-libc     (~1.8.0)

ARM Specific (Teensy 3.0/3.1, Infinity Keyboard, McHCK)

- Arch Linux / Mac Ports
    - arm-none-eabi-gcc
    - arm-none-eaby-binutils

- Windows (https://launchpad.net/gcc-arm-embedded/+download)
    - gcc-arm-none-eabi (win32.zip)


Windows Setup
-------------

Compiling on Windows does work, just it's a bunch more work.

First make sure Cygwin is installed - http://www.cygwin.com/ - 32bit or 64bit
is fine. Make sure the following are installed:

- make
- git (needed for some compilation info)
- cmake
- gcc-core
- gcc-g++
- libusb1.0
- libusb1.0-devel
- python3
- ctags (recommended, not required)

Please note, I use cygwin term exclusively for any command line options.
Unless mentioned otherwise, use it.  Do NOT use CMD or Powershell.

Also install the [Windows version of CMake](http://cmake.org/cmake/resources/software.html)
(3+ is ideal) - Select "Do not add CMake to system PATH".  This is in addition
to the Cygwin version. This is an easier alternative to installing another C
compiler.  Add the following line to your .bashrc, making sure the CMake path
is correct:

    echo "alias wincmake=\"PATH='/cygdrive/c/Program Files (x86)/CMake'/bin:'${PATH}' cmake -G 'Unix Makefiles'\"" >> ~/.bashrc

Install the [PJRC Virtual Serial Port Driver](http://pjrc.com/teensy/serial_install.exe).

Next, install the compiler(s) you want.


### AVR GCC

You just need the
[Atmel AVR 8-bit Toolchain](http://www.atmel.com/tools/atmelavrtoolchainforwindows.aspx).
The latest should be fine, as of writing it was 3.4.3.

Extract the files to a directory, say `C:\avr8-gnu-toolchain`. Then copy all
the folders in that directory to the Cygwin `/usr/local` directory.  Mine is
`C:\cygwin64\usr\local`. (You can also just setup the paths, but this is
faster/simpler. Might screw up your Cygwin though).


### ARM EABI

Download the latest
[GNU Tools for Embedded Processors
gcc-arm-none-eabi](https://launchpad.net/gcc-arm-embedded/+download).

Download `gcc-arm-none-eabi*win32.zip`.

Then extract all the folders/files in the zip to the Cygwin `/usr/local`
directory.  Mine is `C:\cygwin64\usr\local`.  Or, you can setup paths using
the installer (you have to be more careful, avoid spaces in paths).


CMake Info
----------

One of the big benefits of using CMake is the ability to build multiple
configurations (for different microcontrollers) at the same time.  The
following sections explain in detail what each CMakeLists.txt configuration
option does and what you can change it to.  However, it is possible to
configure each of these options using the `-D` command line flag.

For example, to build the Infinity Keyboard default configuration:

```bash
$ mkdir build_infinity
$ cd build_infinity
$ cmake -DCHIP=mk20dx128vlf5 -DScanModule=MD1 -DMacroModule=PartialMap \
        -DOutputModule=pjrcUSB -DDebugModule=full -DBaseMap=defaultMap \
        -DDefaultMap="md1Overlay stdFuncMap" -DPartialMaps="hhkbpro2" \
        ..
$ make
```

CMake defaults to the values specified in CMakeLists.txt if not overridden via
the command line.

> NOTE: On Windows, you will have to use "wincmake" instead of "cmake".


Selecting Microcontroller
-------------------------

This is where you select the chip you want to compile for.  The build system
will automatically select the compiler needed to compile for your chip.

Open up CMakeLists.txt in your favourite text editor. You are looking for:

```cmake
###
# Chip Selection
#

#| You _MUST_ set this to match the microcontroller you are trying to compile for
#| You _MUST_ clean the build directory if you change this value
#|
set( CHIP
#	"at90usb162"       # Teensy   1.0 (avr)
#	"atmega32u4"       # Teensy   2.0 (avr)
#	"at90usb646"       # Teensy++ 1.0 (avr)
#	"at90usb1286"      # Teensy++ 2.0 (avr)
#	"mk20dx128"        # Teensy   3.0 (arm)
    "mk20dx128vlf5"    # McHCK    mk20dx128vlf5
#	"mk20dx256"        # Teensy   3.1 (arm)
    CACHE STRING "Microcontroller Chip" )
```

Just uncomment the chip you want, and comment out the old one.

> NOTE: If you change this option, you will *need* to delete the build
> directory that is created in the Building sections below.


Selecting Modules
-----------------

> WARNING: Not all modules are compatible, and some modules may have
> dependencies on other modules.

This is where the options start getting interesting.  The Kiibohd Controller
is designed around a set of 4 types of modules that correspond to different
functionality:

- Scan Module
- Macro Module
- Output Module
- Debug Module

The Scan Module is where the most interesting stuff happens. These modules
take in "keypress data".  A converter Scan Module will interpret a protocol
into key press/releases.  A matrix Scan Module may inherit from the matrix
module to scan keypress from a matrix This module just has to give
press/release codes, but does have some callback control to other modules
depending on the lifecycle for press/release codes (this can be very
complicated depending on the protocol).  Each Scan Module has it's own default
keymap/modifier map. (TODO recommend keymap changing in the Macro Module).

Some scan modules have very specialized hardware requirements, each module
directory should have at least a link to the needed parts and/or schematics
(TODO!).

The Macro Module takes care of the mapping of the key press/release code into
an Output (USB) scan code.  Any layering, macros, keypress
intelligence/reaction is done here.

The Output Module is the module dealing with output from the microcontroller.
Currently USB is the only output protocol.  Different USB output
implementations are available, pjrc being the safest/least featureful one.
Debug capabilities may depend on the module selected.

The Debug Module enables various things like the Teensy LED on errors, debug
terminal output.  (TODO get true UART working in avr, not just arm)

Open up CMakeLists.txt in your favourite text editor.  Look for:

```cmake
###
# Project Modules
#

#| Note: This is the only section you probably want to modify
#| Each module is defined by it's own folder (e.g. Scan/Matrix represents the "Matrix" module)
#| All of the modules must be specified, as they generate the sources list of files to compile
#| Any modifications to this file will cause a complete rebuild of the project

#| Please look at the {Scan,Macro,Output,Debug} for information on the modules and how to create new ones

##| Deals with acquiring the keypress information and turning it into a key index
set(   ScanModule "MD1"
    CACHE STRING "Scan Module" )

##| Provides the mapping functions for DefaultMap and handles any macro processing before sending to the OutputModule
set(  MacroModule "PartialMap"
    CACHE STRING "Macro Module" )

##| Sends the current list of usb key codes through USB HID
set( OutputModule "pjrcUSB"
    CACHE STRING "Output Module" )

##| Debugging source to use, each module has it's own set of defines that it sets
set(  DebugModule "full"
    CACHE STRING "Debug Module" )
```

Look at each module individually for it's requirements. There is
chip/architecture dependency checking but some permutations of modules may not
be tested/compile.

There are also CMake options for temporarily selecting modules. But it's
easier to just edit the file. e.g. `cmake -DScanModuleOverride=<module name>`.


Keymap Configuration
--------------------

This is where you define the layout for your keyboard.
Currently, the only way to define kebyoard layouts is using [KLL](https://www.overleaf.com/read/zzqbdwqjfwwf).

KLL is built up of 3 different kinds of keymaps in total.
The BaseMap, DefaultMap and PartialMaps.

For each type of keymap, it is possible to combine multiple .kll files together to create new ones using
the compiler. The order of the files matter, as the right-most file will overwrite any setting in the
previous files.

> NOTE: Each keymap is done after the entire file is processed. This means that within the file the order
>       of assignment doesa *not* matter (if you assign the same thing twice, then yes the most recent one
>       takes priority).


BaseMap defines what the keyboard can do. This includes specific capabilities of the keyboard (such as USB),
the mapping of Scan Codes to USB Codes and any specific configurations for the keyboard.
In general, the BaseMap rarely needs to be changed. Usually only when adding a new keyboard to the firmware
does the Basemap need any modification.
The BaseMap is what both DefaultMap and PartialMaps are based upon. This allows for a common reference
when defining custom keymappings.

> NOTE: Don't use defaultMap.kll to change your layouts. This will work, but they will not be portable.


The DefaultMap is the normal state of the keyboard, i.e. your default layer.
Using the BaseMap as a base, the DefaultMap is a modification of the BaseMap to what the keyboard should do.
Since the DefaultMap uses USB Code to USB Code translations, this means that keymaps used for one keyboard
will work with another keyboard.
For example, I use Colemak, so this means I only have to define Colemak once for every keyboard that supports
the kiibohd firmware. This is possible because every BaseMap defines the keyboard as a US ANSI like keyboard
layout.
The DefaultMap can also be thought of as Layer 0.


PartialMaps are optional keymaps that can be "stacked" on top of the DefaultMap.
They can be dynamically swapped out using the layer control capabilities:

- layerLatch( `<layer number>` )
- layerLock( `<layer number>` )
- layerShift( `<layer number>` )

layerShift is usually what you want as it works just like a standard shift key.
layerLock is similar to the CapsLock key. While layerLatch is a latch, where only the next key you press
will use that layer (e.g. stickykeys).

A unique aspect of KLL layers is that it's a true stack of layers.
When a layer is activated, only the keys that are specified by the layer will change.
This means, if you define a layer that only sets `CapsLock -> LCtrl` and `LCtrl->Capslock` only those keys
will change when you active the layer. All the other keys will use the layer that is "underneath" to
lookup the keypress (usually the DefaultMap).

This means that you can combine .kll files statically using the compiler or dynamically using the firmware.

You can set the max number of layers by changing the `stateWordSize` define in one of your kll files.
By default it is set to 8 in Macro/PartialMap/capabilities.kll. This means you can have up to 256 layers
total (this includes the DefaultMap).
You can increase this number to either 16 or 32 (this will use more Flash and RAM btw) which will give you
2^16 and 2^32 possible layers respectively (65 535 and 4 294 967 295).


```cmake
###
# Keymap Configuration (do not include the .kll extension)
#

#| Do not include the .kll extension
#| * BaseMap maps the native keyboard scan codes to USB Codes so the layout is compatible with all other layouts
#| * DefaultMap allows the default keymap to be modified from the BaseMap
#| * PartialMaps is a set of dynamically set layers (there is no limit, but too many may use up too much RAM...)
#| BaseMap generally does not need to be changed from "defaultMap"
#|
#| Syntax:
#|  myMap
#|    * defines a single .kll layout file, double-quotes are needed to distinguish between layers
#|  "myMap specialLayer"
#|    * defines myMap to be the main layout, then replace specialLayers on top of it
#|
#| - Only for PartialMaps -
#|  "myMap specialLayer" "myMap colemak" dvorak
#|    * As before, but also generates a second layer at index 2 and third at index 3
#|
#| NOTE:  Remember to add key(s) to enable each Partial Layer
#| NOTE2: Layers are always based up the BaseMap (which should be an ANSI-like mapping)
#| NOTE3: Compiler looks in kll/layouts and the build directory for layout files (precedence on build directory)

##| Set the base keyboard .kll map, defaults to "defaultMap" if not found
##| Looks in Scan/<Module Name> for the available BaseMaps
set(     BaseMap "defaultMap"
        CACHE STRING "KLL BaseMap/Scancode Keymapping" )

##| Layer additonal .kll maps on the BaseMap, layers are in order from 1st to nth
##| Can be set to ""
set(  DefaultMap "md1Overlay stdFuncMap"
        CACHE STRING "KLL DefaultMap" )

##| ParitalMaps available on top of the BaseMap. See above for syntax on specifying multiple layers vs. layering
##| Can be set to ""
set( PartialMaps "hhkbpro2"
        CACHE STRING "KLL PartialMaps/Layer Definitions" )
```


Linux Building
--------------

From this directory.

```bash
$ mkdir build
$ cd build
$ cmake ..
$ make
```

Example output:

```
$ cmake ..
-- Compiler Family:
arm
-- Chip Selected:
mk20dx128vlf5
-- Chip Family:
mk20dx
-- CPU Selected:
cortex-m4
-- Compiler Source Files:
Lib/mk20dx.c;Lib/delay.c
-- Bootloader Type:
dfu
-- Detected Scan Module Source Files:
Scan/MD1/scan_loop.c;Scan/MD1/../MatrixARM/matrix_scan.c
-- Detected Macro Module Source Files:
Macro/PartialMap/macro.c
-- Detected Output Module Source Files:
Output/pjrcUSB/output_com.c;Output/pjrcUSB/arm/usb_desc.c;Output/pjrcUSB/arm/usb_dev.c;
Output/pjrcUSB/arm/usb_keyboard.c;Output/pjrcUSB/arm/usb_mem.c;Output/pjrcUSB/arm/usb_serial.c
-- Detected Debug Module Source Files:
Debug/full/../cli/cli.c;Debug/full/../led/led.c;Debug/full/../print/print.c
-- Found Git: /usr/bin/git (found version "2.2.1")
-- Found Ctags: /usr/bin/ctags (found version "5.8")
-- Checking for latest kll version:
Current branch master is up to date.
-- Detected Layout Files:
/home/hyatt/Source/controller/Macro/PartialMap/capabilities.kll
/home/hyatt/Source/controller/Output/pjrcUSB/capabilities.kll
/home/hyatt/Source/controller/Scan/MD1/defaultMap.kll
/home/hyatt/Source/controller/kll/layouts/md1Overlay.kll
/home/hyatt/Source/controller/kll/layouts/stdFuncMap.kll
/home/hyatt/Source/controller/kll/layouts/hhkbpro2.kll
-- Configuring done
-- Generating done
-- Build files have been written to: /home/hyatt/Source/controller/build
[master]: make                                [~/Source/controller/build](hyatt@x230mas:pts/6)
[  5%] Generating KLL Layout
Scanning dependencies of target kiibohd.elf
[ 11%] Building C object CMakeFiles/kiibohd.elf.dir/main.c.o
[ 17%] Building C object CMakeFiles/kiibohd.elf.dir/Lib/mk20dx.c.o
[ 23%] Building C object CMakeFiles/kiibohd.elf.dir/Lib/delay.c.o
[ 29%] Building C object CMakeFiles/kiibohd.elf.dir/Scan/MD1/scan_loop.c.o
[ 35%] Building C object CMakeFiles/kiibohd.elf.dir/Scan/MatrixARM/matrix_scan.c.o
[ 41%] Building C object CMakeFiles/kiibohd.elf.dir/Macro/PartialMap/macro.c.o
[ 47%] Building C object CMakeFiles/kiibohd.elf.dir/Output/pjrcUSB/output_com.c.o
[ 52%] Building C object CMakeFiles/kiibohd.elf.dir/Output/pjrcUSB/arm/usb_desc.c.o
[ 58%] Building C object CMakeFiles/kiibohd.elf.dir/Output/pjrcUSB/arm/usb_dev.c.o
[ 64%] Building C object CMakeFiles/kiibohd.elf.dir/Output/pjrcUSB/arm/usb_keyboard.c.o
[ 70%] Building C object CMakeFiles/kiibohd.elf.dir/Output/pjrcUSB/arm/usb_mem.c.o
[ 76%] Building C object CMakeFiles/kiibohd.elf.dir/Output/pjrcUSB/arm/usb_serial.c.o
[ 82%] Building C object CMakeFiles/kiibohd.elf.dir/Debug/cli/cli.c.o
[ 88%] Building C object CMakeFiles/kiibohd.elf.dir/Debug/led/led.c.o
[ 94%] Building C object CMakeFiles/kiibohd.elf.dir/Debug/print/print.c.o
Linking C executable kiibohd.elf
[ 94%] Built target kiibohd.elf
Scanning dependencies of target SizeAfter
[100%] Chip usage for mk20dx128vlf5
     SRAM:  32%     5384/16384      bytes
    Flash:  18%     23384/126976    bytes
[100%] Built target SizeAfter
```

Linux Loading Firmware
----------------------

First place the keyboard into re-flash mode.  This can be done either by
pressing the re-flash button on the PCB/Teensy.  Or by entering the Kiibohd
Virtual Serial Port and using the 'reload' command.

The `load` script that is created during the build can load the firmware over
USB.  Either run it with sudo, or install the `98-kiibohd.rules` to
`/etc/udev/rules.d` and run: `udevadm control --reload-rules`.

To load the newly built firmware: `./load`.


Linux Building Bootloader
-------------------------

> NOTE: Does not apply to Teensy based builds.

From this directory.

```bash
$ cd Bootloader
$ mkdir build
$ cd build
$ cmake ..
$ make
```

Example output:

```bash
$ cmake ..
-- Compiler Family:
arm
-- Chip Selected:
mk20dx128vlf5
-- Chip Family:
mk20dx
-- CPU Selected:
cortex-m4
-- Compiler Source Files:
Lib/mk20dx.c;Lib/delay.c
-- Bootloader Type:
dfu
-- Bootloader Source Files:
main.c;dfu.c;dfu.desc.c;flash.c;kinetis.c;usb.c
-- Found Git: /usr/bin/git (found version "2.2.1")
-- Found Ctags: /usr/bin/ctags (found version "5.8")
-- Configuring done
-- Generating done
-- Build files have been written to: /home/hyatt/Source/controller/Bootloader/build
[master]: make                                 [~/Source/controller/Bootloader/build](hyatt@x230mas:pts/6)
Scanning dependencies of target kiibohd_bootloader.elf
[ 11%] Building C object CMakeFiles/kiibohd_bootloader.elf.dir/main.c.o
[ 22%] Building C object CMakeFiles/kiibohd_bootloader.elf.dir/dfu.c.o
[ 33%] Building C object CMakeFiles/kiibohd_bootloader.elf.dir/dfu.desc.c.o
[ 44%] Building C object CMakeFiles/kiibohd_bootloader.elf.dir/flash.c.o
[ 55%] Building C object CMakeFiles/kiibohd_bootloader.elf.dir/kinetis.c.o
[ 66%] Building C object CMakeFiles/kiibohd_bootloader.elf.dir/usb.c.o
[ 77%] Building C object CMakeFiles/kiibohd_bootloader.elf.dir/home/hyatt/Source/controller/Lib/mk20dx.c.o
[ 88%] Building C object CMakeFiles/kiibohd_bootloader.elf.dir/home/hyatt/Source/controller/Lib/delay.c.o
Linking C executable kiibohd_bootloader.elf
[ 88%] Built target kiibohd_bootloader.elf
Scanning dependencies of target SizeAfter
[100%] Chip usage for mk20dx128vlf5
     SRAM:  19%     3176/16384      bytes
    Flash:  2%      3736/126976     bytes
[100%] Built target SizeAfter
```


Linux Loading Bootloader
------------------------

> NOTE: Does not apply to Teensy based builds.

It's recommended to use an SWD-type flasher like a Bus Pirate.  There is a
convenience script for loading the firmware once the system is setup.

```bash
$ cd Bootloader/Scripts
$ ./swdLoad.bash
```

The above script requires Ruby, Ruby serial port module, git, and a
`/dev/buspirate` udev rule.

Additional Notes:

* https://github.com/mchck/mchck/wiki/Getting-Started (See Bus-Pirate section)
* https://wiki.archlinux.org/index.php/Bus_pirate


Windows Building
----------------

From this directory.

```bash
$ mkdir build
$ cd build
$ wincmake ..
$ make
```

Example output:

```bash
$ wincmake ..
-- Compiler Family:
arm
-- Chip Selected:
mk20dx128vlf5
-- Chip Family:
mk20dx
-- CPU Selected:
cortex-m4
-- Compiler Source Files:
Lib/mk20dx.c;Lib/delay.c
-- Bootloader Type:
dfu
-- Detected Scan Module Source Files:
Scan/MD1/scan_loop.c;Scan/MD1/../MatrixARM/matrix_scan.c
-- Detected Macro Module Source Files:
Macro/PartialMap/macro.c
-- Detected Output Module Source Files:
Output/pjrcUSB/output_com.c;Output/pjrcUSB/arm/usb_desc.c;Output/pjrcUSB/arm/usb_dev.c;Output/pjrcUSB/arm/usb_keyboard.c;Output/pjrcUSB/arm/usb_mem.c;Output/pjrcUSB/arm/usb_serial.c
-- Detected Debug Module Source Files:
Debug/full/../cli/cli.c;Debug/full/../led/led.c;Debug/full/../print/print.c
-- Found Git: C:/cygwin64/bin/git.exe (found version "2.1.1")
-- Found Ctags: C:/cygwin64/bin/ctags.exe (found version "5.8")
-- Checking for latest kll version:
Current branch master is up to date.
-- Detected Layout Files:
C:/cygwin64/home/Jacob/controller/Macro/PartialMap/capabilities.kll
C:/cygwin64/home/Jacob/controller/Output/pjrcUSB/capabilities.kll
C:/cygwin64/home/Jacob/controller/Scan/MD1/defaultMap.kll
C:/cygwin64/home/Jacob/controller/kll/layouts/md1Overlay.kll
C:/cygwin64/home/Jacob/controller/kll/layouts/stdFuncMap.kll
C:/cygwin64/home/Jacob/controller/kll/layouts/hhkbpro2.kll
-- Configuring done
-- Generating done
-- Build files have been written to: C:/cygwin64/home/Jacob/controller/build

$ make
[  5%] Generating KLL Layout
Scanning dependencies of target kiibohd.elf
[ 11%] Building C object CMakeFiles/kiibohd.elf.dir/main.c.obj
[ 17%] Building C object CMakeFiles/kiibohd.elf.dir/Lib/mk20dx.c.obj
[ 23%] Building C object CMakeFiles/kiibohd.elf.dir/Lib/delay.c.obj
[ 29%] Building C object CMakeFiles/kiibohd.elf.dir/Scan/MD1/scan_loop.c.obj
[ 35%] Building C object CMakeFiles/kiibohd.elf.dir/Scan/MatrixARM/matrix_scan.c.obj
[ 41%] Building C object CMakeFiles/kiibohd.elf.dir/Macro/PartialMap/macro.c.obj
[ 47%] Building C object CMakeFiles/kiibohd.elf.dir/Output/pjrcUSB/output_com.c.obj
[ 52%] Building C object CMakeFiles/kiibohd.elf.dir/Output/pjrcUSB/arm/usb_desc.c.obj
[ 58%] Building C object CMakeFiles/kiibohd.elf.dir/Output/pjrcUSB/arm/usb_dev.c.obj
[ 64%] Building C object CMakeFiles/kiibohd.elf.dir/Output/pjrcUSB/arm/usb_keyboard.c.obj
[ 70%] Building C object CMakeFiles/kiibohd.elf.dir/Output/pjrcUSB/arm/usb_mem.c.obj
[ 76%] Building C object CMakeFiles/kiibohd.elf.dir/Output/pjrcUSB/arm/usb_serial.c.obj
[ 82%] Building C object CMakeFiles/kiibohd.elf.dir/Debug/cli/cli.c.obj
[ 88%] Building C object CMakeFiles/kiibohd.elf.dir/Debug/led/led.c.obj
[ 94%] Building C object CMakeFiles/kiibohd.elf.dir/Debug/print/print.c.obj
Linking C executable kiibohd.elf
[ 94%] Built target kiibohd.elf
Scanning dependencies of target SizeAfter
[100%] Chip usage for mk20dx128vlf5
     SRAM:  32%     5384/16384      bytes
    Flash:  18%     23296/126976    bytes
[100%] Built target SizeAfter
```

### NOTES:

If you get the following error, you have not setup wincmake correctly:

```bash
$ make
[  5%] Generating KLL Layout
Scanning dependencies of target kiibohd.elf
[ 11%] Building C object CMakeFiles/kiibohd.elf.dir/main.c.o
../main.c:28:19: fatal error: macro.h: No such file or directory
 #include <macro.h>
                   ^
compilation terminated.
CMakeFiles/kiibohd.elf.dir/build.make:67: recipe for target 'CMakeFiles/kiibohd.elf.dir/main.c.o' failed
make[2]: *** [CMakeFiles/kiibohd.elf.dir/main.c.o] Error 1
CMakeFiles/Makefile2:98: recipe for target 'CMakeFiles/kiibohd.elf.dir/all' failed
make[1]: *** [CMakeFiles/kiibohd.elf.dir/all] Error 2
Makefile:75: recipe for target 'all' failed
make: *** [all] Error 2
```

If you have already added the line to your `~/.bashrc` try restarting your
cygwin shell.


Windows Loading Firmware
------------------------

First place the keyboard into re-flash mode.  This can be done either by
pressing the re-flash button on the PCB/Teensy.  Or by entering the Kiibohd
Virtual Serial Interface and using the `reload` command.

The `load` script that is created during the build can load the firmware over
USB.

To load the newly built firmware: `./load`

Be patient the couple of times, Windows is slow at installing drivers...


Mac OS X Building
-----------------

From this directory.

```bash
$ mkdir build
$ cd build
$ cmake ..
$ make
```

Example output:

> TODO


Mac OS X Loading Firmware
-------------------------

First place the keyboard into re-flash mode.  This can be done either by
pressing the re-flash button on the PCB/Teensy.  Or by entering the Kiibohd
Virtual Serial Port and using the `reload` command.

The `load` script that is created during the build can load the firmware over
USB.

To load the newly built firmware: `./load`.


Virtual Serial Port - CLI
-------------------------

Rather than use a special program that can interpret Raw HID, this controller exposes a USB Serial CDC endpoint.
This allows for you to use a generic serial terminal to debug/control the keyboard firmware (e.g. Tera Term, minicom, screen)


### Linux

I generally use screen.  You will need sudo/root priviledges if you haven't
installed the `98-kiibohd.rules` file to `/etc/udev/rules.d`.

```
$ screen /dev/ttyACM0
# (Might be ACM1, ACM2, etc.)
```

### Windows

Make sure the Teensy Virtual Serial Port driver is installed.  If possible use
screen (as part of Cygwin).  Check which COM port the virtual serial port has
been assigned to: `Device Manager->Ports (COM & LPT)->Teensy USB Serial`. In
brackets it will say which COM port (e.g. COM3)

putty works well when using DTR/DSR or RTS/CTS flow control.

| Setting         | Value                                 |
| --------------- | ------------------------------------- |
| Connection type | Serial                                |
| Serial line     | Your COM port, e.g. COM3              |
| Speed           | doesn't matter, it's auto-negotiated  |

Under `Category->Connections->Serial`: `Flow control: DTR/DSR`.

If stuff is hard to read (you have a dumb colour scheme):
`Category->Window->Colours->Use system color`.  That seems to make text at
least readable 

> I use a custom colour scheme that makes each colour easy to see.
> -HaaTa.

Unfortunately, screen for Cygwin seems to be broken for serial ports, but you
can try it...

```bash
$ screen /dev/ttyS2
# Might be a different file, ttyS0, ttyACM0, ttyUSB0, etc.
```

Gnu screen doesn't seem to echo all the characters (it works though).
I believe it's a problem with stty, but I don't know how to fix it...

### Mac OS X

I recommend screen (can be installed via Macports).

```bash
$ screen /dev/tty.<usb something>
```
