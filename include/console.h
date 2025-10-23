#ifndef CONSOLE_H
#define CONSOLE_H

#include "kernel.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define VGA_COLOR_BLACK         0
#define VGA_COLOR_BLUE          1
#define VGA_COLOR_GREEN         2
#define VGA_COLOR_CYAN          3
#define VGA_COLOR_RED           4
#define VGA_COLOR_MAGENTA       5
#define VGA_COLOR_BROWN         6
#define VGA_COLOR_LIGHT_GREY    7
#define VGA_COLOR_DARK_GREY     8
#define VGA_COLOR_LIGHT_BLUE    9
#define VGA_COLOR_LIGHT_GREEN   10
#define VGA_COLOR_LIGHT_CYAN    11
#define VGA_COLOR_LIGHT_RED     12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_LIGHT_BROWN   14
#define VGA_COLOR_WHITE         15

#define VGA_COLOR(fg, bg) ((bg << 4) | fg)

void console_init();
void console_clear();
void console_putchar(char c);
void console_print(const char* str);
void console_print_dec(int num);
void console_print_hex(uint32_t num);
void scroll_screen();
void console_set_color(uint8_t color);
uint8_t console_get_color();
void update_cursor();

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);

#endif