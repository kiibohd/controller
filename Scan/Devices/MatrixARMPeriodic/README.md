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

