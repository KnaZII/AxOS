#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "kernel.h"

#define KEYBOARD_DATA_PORT    0x60
#define KEYBOARD_STATUS_PORT  0x64

void keyboard_init();
char keyboard_getchar();
int keyboard_key_pressed();
int keyboard_shift_pressed();
int keyboard_ctrl_pressed();
int keyboard_alt_pressed();

#endif