#include "task.h"
#include "string.h"
#include "idt.h"
#include "pit.h"
#include "paging.h"

static Task tasks[MAX_TASKS];
static int current = -1;

volatile uint32_t switch_to_esp = 0;
volatile uint32_t* switch_to_cr3 = 0;

void task_init() {
    memset(tasks, 0, sizeof(tasks));
    current = -1;
}

void task_register_kernel() {
    tasks[0].used = 1;
    tasks[0].entry = 0;
    tasks[0].sleeping = 0;
    tasks[0].sleep_until = 0;
    tasks[0].esp = 0;
    tasks[0].pd = paging_get_kernel_pd();
    current = 0;
}

int task_create(void (*entry)(void)) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (!tasks[i].used) {
            tasks[i].used = 1;
            tasks[i].entry = entry;
            tasks[i].sleeping = 0;
            tasks[i].sleep_until = 0;
            static uint8_t stacks[MAX_TASKS][TASK_STACK_SIZE];
            uint32_t* sp = (uint32_t*)(stacks[i] + TASK_STACK_SIZE);
            *--sp = 0x202;
            *--sp = 0x08;
            *--sp = (uint32_t)entry;
            *--sp = 0x0;
            *--sp = 0x0;
            *--sp = 0x0;
            *--sp = 0x0;
            *--sp = 0x0;
            *--sp = 0x0;
            *--sp = 0x0;
            *--sp = 0x0;
            tasks[i].esp = (uint32_t)sp;
            tasks[i].pd = paging_get_kernel_pd();
            return i;
        }
    }
    return -1;
}

void schedule(uint32_t current_esp) {
    if (current >= 0 && tasks[current].used) {
        tasks[current].esp = current_esp;
    }
    int start = current;
    for (int shift = 1; shift <= MAX_TASKS; shift++) {
        int next = ((start + shift) % MAX_TASKS);
        if (tasks[next].used) {
            if (tasks[next].sleeping && timer_ticks < tasks[next].sleep_until) {
                continue;
            }
            tasks[next].sleeping = 0;
            current = next;
            switch_to_cr3 = tasks[next].pd;
            switch_to_esp = tasks[next].esp;
            return;
        }
    }
}

void task_sleep_ticks(uint32_t ticks) {
    int me = current;
    if (me < 0) return;
    tasks[me].sleep_until = timer_ticks + (ticks ? ticks : 1);
    tasks[me].sleeping = 1;
    while (tasks[me].sleeping) {
        __asm__ __volatile__("hlt");
    }
}

void task_sleep_ms(uint32_t ms) {
    uint32_t ticks = (pit_hz ? ((ms * pit_hz + 999) / 1000) : ms / 10);
    if (ticks == 0) ticks = 1;
    task_sleep_ticks(ticks);
}

void task_set_current_pd(uint32_t* pd) {
    if (current >= 0 && tasks[current].used) {
        tasks[current].pd = pd;
    }
}