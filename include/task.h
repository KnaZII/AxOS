#ifndef TASK_H
#define TASK_H

#include "kernel.h"

#define MAX_TASKS 4
#define TASK_STACK_SIZE (4096)

typedef struct Task {
    uint32_t esp;
    void (*entry)(void);
    int used;
    int sleeping;
    uint32_t sleep_until;
    uint32_t* pd;
} Task;

void task_init();
int task_create(void (*entry)(void));
void schedule(uint32_t current_esp);

void task_register_kernel();

void task_sleep_ticks(uint32_t ticks);
void task_sleep_ms(uint32_t ms);

void task_set_current_pd(uint32_t* pd);

#endif