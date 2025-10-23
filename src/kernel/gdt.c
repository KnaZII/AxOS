#include "gdt.h"
#include "string.h"
struct gdt_entry{uint16_t limit_low;uint16_t base_low;uint8_t base_mid;uint8_t access;uint8_t gran;uint8_t base_high;}__attribute__((packed));
struct gdt_ptr{uint16_t limit;uint32_t base;}__attribute__((packed));
struct tss_entry{uint32_t prev;uint32_t esp0;uint32_t ss0;uint32_t esp1;uint32_t ss1;uint32_t esp2;uint32_t ss2;uint32_t cr3;uint32_t eip;uint32_t eflags;uint32_t eax;uint32_t ecx;uint32_t edx;uint32_t ebx;uint32_t esp;uint32_t ebp;uint32_t esi;uint32_t edi;uint32_t es;uint32_t cs;uint32_t ss;uint32_t ds;uint32_t fs;uint32_t gs;uint32_t ldt;uint16_t trap;uint16_t iomap;}__attribute__((packed));
static struct gdt_entry gdt[6];
static struct gdt_ptr gp;
static struct tss_entry tss;
static void set_entry(int i,uint32_t base,uint32_t limit,uint8_t access,uint8_t gran){gdt[i].limit_low=limit&0xFFFF;gdt[i].base_low=base&0xFFFF;gdt[i].base_mid=(base>>16)&0xFF;gdt[i].access=access;gdt[i].gran=((limit>>16)&0x0F)|(gran&0xF0);gdt[i].base_high=(base>>24)&0xFF;}
static void write_tss(){uint32_t base=(uint32_t)&tss;uint32_t limit=sizeof(tss)-1;set_entry(5,base,limit,0x89,0x00);}
void tss_set_kernel_stack(uint32_t esp0){tss.esp0=esp0;}
void gdt_init(){gp.limit=sizeof(gdt)-1;gp.base=(uint32_t)&gdt;set_entry(0,0,0,0,0);set_entry(1,0,0xFFFF,0x9A,0xCF);set_entry(2,0,0xFFFF,0x92,0xCF);set_entry(3,0,0xFFFF,0xFA,0xCF);set_entry(4,0,0xFFFF,0xF2,0xCF);memset(&tss,0,sizeof(tss));tss.ss0=0x10;tss.esp0=0x90000;write_tss();__asm__ __volatile__("lgdt (%0)"::"r"(&gp));__asm__ __volatile__("mov $0x10,%ax;mov %ax,%ds;mov %ax,%es;mov %ax,%fs;mov %ax,%gs;mov %ax,%ss");__asm__ __volatile__("mov $0x28,%ax;ltr %ax");}