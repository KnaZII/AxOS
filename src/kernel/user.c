#include "user.h"
void enter_user_mode(uint32_t entry,uint32_t stack){__asm__ __volatile__(
    "cli\n"
    "mov $0x23,%%ax\n"
    "mov %%ax,%%ds\n"
    "mov %%ax,%%es\n"
    "pushl $0x23\n"
    "pushl %0\n"
    "pushfl\n"
    "orl $0x200,(%%esp)\n"
    "pushl $0x1B\n"
    "pushl %1\n"
    "iret\n"
    : : "r"(stack),"r"(entry) : "ax");}