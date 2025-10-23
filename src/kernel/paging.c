#include "paging.h"
#include "string.h"
#include "console.h"
static uint32_t page_directory[1024] __attribute__((aligned(4096)));
static uint32_t first_table[1024] __attribute__((aligned(4096)));
static uint32_t* make_pd(){uint32_t* pd=(uint32_t*)page_directory;uint32_t* ft=(uint32_t*)first_table;memset(pd,0,4096);memset(ft,0,4096);for(uint32_t i=0;i<1024;i++){ft[i]=(i*0x1000)|3;}pd[0]=(uint32_t)ft|3;return pd;}
void paging_init(){make_pd();__asm__ volatile ("mov %0, %%cr3" : : "r"(page_directory));uint32_t cr0;__asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));cr0|=0x80000000;__asm__ volatile ("mov %0, %%cr0" : : "r"(cr0));console_print("Paging enabled (identity 0..4MB)\n");}
uint32_t* paging_new_pd(){static uint32_t new_pd[1024] __attribute__((aligned(4096)));static uint32_t new_ft[1024] __attribute__((aligned(4096)));uint32_t* pd=new_pd;uint32_t* ft=new_ft;memset(pd,0,4096);memset(ft,0,4096);for(uint32_t i=0;i<1024;i++){ft[i]=(i*0x1000)|3;}pd[0]=(uint32_t)ft|7;return pd;}
void paging_mark_user(uint32_t* pd,uint32_t start,uint32_t size){uint32_t* ft=(uint32_t*)(pd[0]&~0xFFF);pd[0]|=4;uint32_t s=(start>>12);uint32_t e=((start+size+0xFFF)>>12);for(uint32_t i=s;i<e&&i<1024;i++){ft[i]|=4;}}
void paging_switch(uint32_t* pd){__asm__ volatile ("mov %0, %%cr3" : : "r"(pd));}
uint32_t* paging_get_kernel_pd(){return page_directory;}