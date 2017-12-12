# Kiibohd Controller - MatrixARMPeriodic Sub-Module

The MatrixARMPeriodic Scan Sub-Module is a re-design of the original key matrix scanning module MatrixARM.

Key features of this module:

* Single strobe scan loop
  - Allows for very short duration scans
  - Must be called multiple times to iterate over the entire matrix once (number of strobes)
* Continuous state debounce algorithm
  - Uses history from prior scans to maintain state decisions
  - Reduces number of scans required to make a decision
  - Penalizes bouncy switches
* Debounce time requirement
  - Even if debounce has made a decision, locks out decision until the required time has elapsed.
    + i.e. 5 ms debounce requirement of Cherry MX switches


## KLL Features

* MinDebounceTime
* PeriodicCycles
* StrobeDelay

See [capabilities.kll](capabilities.kll) for more details.


## Example Matrix Setup

To use MatrixARMPeriodic in your own keyboard, you'll need to define your own matrix.h

For example:
```c
#pragma once

// ----- Includes -----

// Project Includes
#include <matrix_setup.h>



// ----- Matrix Definition -----

// Freescale ARM MK20's support GPIO PTA, PTB, PTC, PTD and PTE 0..31
// Not all chips have access to all of these pins (most don't have 160 pins :P)
//
// NOTE:
// Before using a pin, make sure it supports being a GPIO *and* doesn't have a default pull-up/pull-down
// Checking this is completely on the ownness of the user

// MD1
//
// Columns (Strobe)
//  PTB0..3,16,17
//  PTC4,5
//  PTD0
//
// Rows (Sense)
//  PTD1..7

// Define Rows (Sense) and Columns (Strobes)
GPIO_Pin Matrix_cols[] = { gpio(B,0), gpio(B,1), gpio(B,2), gpio(B,3), gpio(B,16), gpio(B,17), gpio(C,4), gpio(C,5), gpio(D,0) };
GPIO_Pin Matrix_rows[] = { gpio(D,1), gpio(D,2), gpio(D,3), gpio(D,4), gpio(D,5), gpio(D,6), gpio(D,7) };

// Define type of scan matrix
Config Matrix_type = Config_Pulldown;
```

`Config Matrix_type` defines whether an internal pull resistor is used.
In general, you should be using an internal pull-down resistor as per the example above.
There are special situations where this does not apply, but unless you know what you're doing, this isn't what you want.


## Debugging Keypress Issues

Sometimes your keyboard may exhibit typing problems such as:

* Extra keypresses (of same character) - Debounce, USB
* Nearby keypresses (keys you did not press) - Ghosting, Strobe Delay (bad pull-up resistors)
* Missing keypresses - Debounce, USB, Scan Rate


### Debounce

Keyboard switches generally have a minimum **debounce time**.
This means that a key press or release may require a delay before the value is certain.

In most cases this is ~5ms (which is the default).

* [Cherry MX](https://cdn.sparkfun.com/datasheets/Components/Switches/MX%20Series.pdf)

The debounce time can be adjusted using KLL (default) or using the cli (dynamic).
When testing new values, using the cli is the easiest.

```c
MinDebounceTime = 5; # 5 ms
```

```bash
: debounce
INFO - Debounce Timer: 5ms
: debounce 7
INFO - Debounce Timer: 7ms
```


### Strobe Delay

Some MCUs have slow pull-up resistors (or possibly transistors).
This is usually quite rare, but there is a work around.
The idea is to pause before strobing each column such that there is time for the previous strobe to turn off.
**Note**: This shouldn't be necessary in nearly all cases.

It is disabled (set to 0) by default.

```c
StrobeDelay = 0; # Disabled, 0us
```

```bash
: strobeDelay
INFO - Strobe Delay: 0us
: strobeDelay 10
INFO - Strobe Delay: 10us
```


### Scan Rate

While not specific to this module, this firmware has a notion of a periodic scan rate.
Between each strobe a given amount of cycles are given to other functions of the keyboard (though other interrupts can interrupt a periodic scan as well).

If you are getting dropped keypresses, you can attempt to decrease the number of clock cycles to wait.
If you are getting bad display performance (e.g. LEDs), you can try to increase the number of clock cycles to wait (gives more time to process effects).

```c
PeriodicCycles = 1000; # 1000 cycles
```

```bash
: periodic
INFO - Period Clock Cycles: 1000
: periodic 2000
INFO - Period Clock Cycles: 2000
```


### Other Sources of Problems

Other issues include:

* Bad/noise USB bus
  + Bad cable
  + Bad connector
  + Bad port
  + Bad USB chipset
* Keyboard bugs
  + Usually fixable with a list of reproduction steps
* OS ignoring USB packets
  + Sometimes fixable on the keyboard side, but may require some trade-offs
* Incompatible USB descriptor
  + Even if the USB HID descriptor is valid, it doesn't mean an OS supports it...

