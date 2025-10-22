#include "disk.h"
#include "console.h"
static inline uint16_t inw(uint16_t port){uint16_t r;__asm__ volatile("inw %1,%0":"=a"(r):"Nd"(port));return r;}
static inline void outw(uint16_t port,uint16_t val){__asm__ volatile("outw %0,%1"::"a"(val),"Nd"(port));}
int disk_read_lba(uint32_t lba,uint16_t count,void* buffer){uint8_t* b=(uint8_t*)buffer;for(uint16_t s=0;s<count;s++){outb(0x1F6,0xE0|((lba>>24)&0x0F));outb(0x1F2,1);outb(0x1F3,(uint8_t)(lba&0xFF));outb(0x1F4,(uint8_t)((lba>>8)&0xFF));outb(0x1F5,(uint8_t)((lba>>16)&0xFF));outb(0x1F7,0x20);while(inb(0x1F7)&0x80){}while(!(inb(0x1F7)&0x08)){}for(int i=0;i<256;i++){uint16_t w=inw(0x1F0);b[i*2]=(uint8_t)(w&0xFF);b[i*2+1]=(uint8_t)((w>>8)&0xFF);}b+=512;lba++;}return 1;}