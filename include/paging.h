#ifndef PAGING_H
#define PAGING_H
#include "kernel.h"
void paging_init();
uint32_t* paging_new_pd();
void paging_mark_user(uint32_t* pd,uint32_t start,uint32_t size);
void paging_switch(uint32_t* pd);
uint32_t* paging_get_kernel_pd();
#endif