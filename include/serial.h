#ifndef SERIAL_H
#define SERIAL_H
#include "kernel.h"
void serial_init();
int serial_ready();
void serial_write_char(char c);
void serial_write(const char* s);
int serial_read_ready();
char serial_read_char();
#endif