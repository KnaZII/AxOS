#ifndef KERNEL_H
#define KERNEL_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

#define NULL ((void*)0)
#define TRUE 1
#define FALSE 0

void kernel_main();
void command_loop();
void read_command();
void process_command(const char* cmd);
void show_help();
void ping_command();
void show_version();
void show_about();
void clear_command_buffer();
void delay(int ms);
void simple_keyboard_test();

char keyboard_getchar();
void keyboard_init();

#endif