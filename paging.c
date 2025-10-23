#include "paging.h"
#include "string.h"
#include "console.h"

static uint32_t page_directory[1024] __attribute__((aligned(4096)));
static uint32_t first_table[1024] __attribute__((aligned(4096)));

void paging_init() {
    memset(page_directory, 0, sizeof(page_directory));
    memset(first_table, 0, sizeof(first_table));

    for (uint32_t i = 0; i < 1024; i++) {
        first_table[i] = (i * 0x1000) | 3; // present + writable, identity map 0..4MB
    }
    page_directory[0] = ((uint32_t)first_table) | 3; // present + writable

    // Загрузить CR3
    __asm__ volatile ("mov %0, %%cr3" : : "r"(page_directory));
    // Включить бит PG в CR0
    uint32_t cr0;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ volatile ("mov %0, %%cr0" : : "r"(cr0));

    console_print("Paging enabled (identity 0..4MB)\n");
}