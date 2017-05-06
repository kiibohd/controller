#ifndef PS2KEYBOARD_H
#define PS2KEYBOARD_H

#include <stdint.h>
#include <delay.h> // adds millis()

/*
  This header file is licensed under the same terms as the PS2Keyboard.c file.
  Copyright (c) 2017 Ivor Wanders

  PS2Keyboard.c is a butchered version of the the PS2Keyboard library.
  For the complete version of the Arduino Library to read PS/2 protocols,
  please see:
    https://github.com/PaulStoffregen/PS2Keyboard
*/

uint8_t ps2data_read(void);
void ps2interrupt(void);
uint8_t get_scan_code(void);

#endif