#ifndef PS2KEYBOARD_H
#define PS2KEYBOARD_H

#include <stdint.h>
#include <delay.h> // adds millis()

uint8_t ps2data_read(void);
void ps2interrupt(void);
uint8_t get_scan_code(void);

#endif