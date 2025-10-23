#ifndef SYSCALL_H
#define SYSCALL_H
#include "kernel.h"
#include "vfs.h"
typedef uint32_t (*syscall_t)(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
void syscall_init();
void syscall_setup_idt();
void syscall_register(uint32_t id, syscall_t fn);
uint32_t syscall_dispatch(uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e,uint32_t f);
#endif