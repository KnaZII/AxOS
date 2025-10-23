#ifndef TASK_H
#define TASK_H

#include "kernel.h"

#define MAX_TASKS 4
#define TASK_STACK_SIZE (4096)

typedef struct Task {
    uint32_t esp;
    void (*entry)(void);
    int used;
    int sleeping;            // 0/1
    uint32_t sleep_until;    // в тиках PIT
} Task;

void task_init();
int task_create(void (*entry)(void));
void schedule(uint32_t current_esp);

// Зарегистрировать kernel_main как планируемую задачу (slot 0)
void task_register_kernel();

// Сон задачи
void task_sleep_ticks(uint32_t ticks);
void task_sleep_ms(uint32_t ms);

#endif // TASK_H