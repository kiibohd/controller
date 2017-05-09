/*
 * Configured by Nazar Mokrynskyi
 *
 * License: Public Domain
 */

#pragma once

// ----- Includes -----

// Project Includes
#include <matrix_setup.h>

/**
 * ----- Matrix Definition -----
 *  Rapoo E9070
 *  Columns (Strobe)  19
 *  Rows    (Sense)   8
 *
 * Keyboard loop only contains 17 lanes, but PCB has 19 labeled contacts, so let's assume there are 19 of them
 * and connect all for the sake of simplicity.
 * Also matrixInfo only reports 13 columns, so I have no idea what wires from those 19 are actually required,
 * but it works when all of them are connected, so I didn't bother checking this.
 */

/**
 * Pins on Teensy 3.2     : 0   1   2   3   4   5   6   7   8   9   10   11   12   24   25   26   27   28   29
 * Pins on keyboard's PCB : KC0 KC1 KC2 KC3 KC4 KC5 KC6 KC7 KC8 KC9 KC10 KC11 KC12 KC13 KC14 KC15 KC16 KC17 KC18
 */
GPIO_Pin Matrix_cols[] = {
  gpio(B,16), gpio(B,17), gpio(D,0), gpio(A,12), gpio(A,13), gpio(D,7), gpio(D,4), gpio(D,2), gpio(D,3), gpio(C,3),
  gpio(C,4), gpio(C,6), gpio(C,7), gpio(A,5), gpio(B,19), gpio(E,1), gpio(C,9), gpio(C,8), gpio(C,10)
};
/**
 * Pins on Teensy 3.2     : 21  20  19  18  17  16  15  14
 * Pins on keyboard's PCB : KR0 KR1 KR2 KR3 KR4 KR5 KR6 KR7
 */
GPIO_Pin Matrix_rows[] = {
  gpio(D,6), gpio(D,5), gpio(B,2), gpio(B,3), gpio(B,1), gpio(B,0), gpio(C,0), gpio(D,1)
};

// Define type of scan matrix
Config Matrix_type = Config_Pullup;

// Define this if your matrix has ghosting (i.e. regular keyboard without diodes)
// this will enable the anti-ghosting code
#define GHOSTING_MATRIX

// delay in microseconds before and after each strobe change during matrix scan
#define STROBE_DELAY  10
