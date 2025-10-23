#include "serial.h"
#include "console.h"
#define COM1 0x3F8
void serial_init(){outb(COM1+1,0x00);outb(COM1+3,0x80);outb(COM1+0,0x01);outb(COM1+1,0x00);outb(COM1+3,0x03);outb(COM1+2,0xC7);outb(COM1+4,0x0B);} 
int serial_ready(){return inb(COM1+5)&0x20;} 
void serial_write_char(char c){while(!serial_ready());outb(COM1,c);} 
void serial_write(const char* s){while(*s){serial_write_char(*s++);} } 
int serial_read_ready(){return inb(COM1+5)&0x01;} 
char serial_read_char(){while(!serial_read_ready());return inb(COM1);}