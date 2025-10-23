#include "syscall.h"
#include "console.h"
#include "vfs.h"
static syscall_t table[256];
void syscall_register(uint32_t id, syscall_t fn){table[id]=fn;}
uint32_t syscall_dispatch(uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e,uint32_t f){syscall_t h=table[a&0xFF];return h? h(b,c,d,e,f,0): (uint32_t)-1;}
static uint32_t sys_write(uint32_t p,uint32_t len,uint32_t x,uint32_t y,uint32_t z,uint32_t w){const char* s=(const char*)p;for(uint32_t i=0;i<len;i++) console_putchar(s[i]);return len;}
static uint32_t sys_exit(uint32_t code,uint32_t x,uint32_t y,uint32_t z,uint32_t w,uint32_t q){(void)x;(void)y;(void)z;(void)w;(void)q; for(;;){} return code;}
void syscall_init(){for(int i=0;i<256;i++) table[i]=0; syscall_register(1,sys_write); syscall_register(2,sys_exit);} 
extern void isr128();
#include "idt.h"
void syscall_setup_idt(){extern void isr128(); idt_set_gate(128,(uint32_t)isr128,0x08,0xEE);} 
void syscall_handler(uint32_t esp){uint32_t* p=(uint32_t*)esp;uint32_t edi=p[2];uint32_t esi=p[3];uint32_t ebp=p[4];uint32_t espv=p[5];uint32_t ebx=p[6];uint32_t edx=p[7];uint32_t ecx=p[8];uint32_t eax=p[9];uint32_t r=syscall_dispatch(eax,ebx,ecx,edx,esi,edi);p[9]=r;(void)ebp;(void)espv;}