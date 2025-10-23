#include "task.h"
#include "string.h"
#include "idt.h"
#include "pit.h"

static Task tasks[MAX_TASKS];
static int current = -1;

// Глобальная переменная, используемая в ASM для переключения стека
volatile uint32_t switch_to_esp = 0;

void task_init() {
    memset(tasks, 0, sizeof(tasks));
    current = -1;
}

// Зарегистрировать kernel_main как задачу в слоте 0.
// Её ESP будет сохранён при первом тике, когда сработает schedule().
void task_register_kernel() {
    tasks[0].used = 1;
    tasks[0].entry = 0;
    tasks[0].sleeping = 0;
    tasks[0].sleep_until = 0;
    tasks[0].esp = 0; // заполнится при первом сохранении
    current = 0;      // считаем, что сейчас выполняется ядро
}

int task_create(void (*entry)(void)) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (!tasks[i].used) {
            tasks[i].used = 1;
            tasks[i].entry = entry;
            tasks[i].sleeping = 0;
            tasks[i].sleep_until = 0;
            // Выделяем стек из статического массива
            static uint8_t stacks[MAX_TASKS][TASK_STACK_SIZE];
            uint32_t* sp = (uint32_t*)(stacks[i] + TASK_STACK_SIZE);
            // Формируем начальное состояние стека как его увидит обработчик IRQ:
            // [EDI][ESI][EBP][ESP-DUMP][EBX][EDX][ECX][EAX][EIP][CS][EFLAGS]
            // Сначала кадр для iret (верх):
            *--sp = 0x202;                      // EFLAGS (IF=1)
            *--sp = 0x08;                       // CS (кодовый сегмент)
            *--sp = (uint32_t)entry;            // EIP
            // Затем регистры в порядке popa (в обратном push'у в pusha):
            *--sp = 0x0; // EAX
            *--sp = 0x0; // ECX
            *--sp = 0x0; // EDX
            *--sp = 0x0; // EBX
            *--sp = 0x0; // ESP (dump, игнорируется popa)
            *--sp = 0x0; // EBP
            *--sp = 0x0; // ESI
            *--sp = 0x0; // EDI
            // Но наш обработчик после pusha делает push ds; push es — поэтому ESP,
            // который он передаёт в C, указывает на [EDI]. Сохраняем именно его.
            tasks[i].esp = (uint32_t)sp;
            return i;
        }
    }
    return -1;
}

void schedule(uint32_t current_esp) {
    // Сохраняем стек текущей задачи
    if (current >= 0 && tasks[current].used) {
        tasks[current].esp = current_esp;
    }
    // Найти следующую используемую задачу (круговой планировщик) среди бодрствующих
    int start = current;
    for (int shift = 1; shift <= MAX_TASKS; shift++) {
        int next = ((start + shift) % MAX_TASKS);
        if (tasks[next].used) {
            if (tasks[next].sleeping && timer_ticks < tasks[next].sleep_until) {
                continue; // ещё спит
            }
            // Проснулся (если время прошло) или уже бодрствует
            tasks[next].sleeping = 0;
            current = next;
            switch_to_esp = tasks[next].esp;
            return;
        }
    }
    // никого не нашли — остаёмся на текущей задаче
}

void task_sleep_ticks(uint32_t ticks) {
    int me = current;
    if (me < 0) return;
    tasks[me].sleep_until = timer_ticks + (ticks ? ticks : 1);
    tasks[me].sleeping = 1;
    // Ждём пробуждения, экономя CPU при помощи HLT
    while (tasks[me].sleeping) {
        __asm__ __volatile__("hlt");
    }
}

void task_sleep_ms(uint32_t ms) {
    uint32_t ticks = (pit_hz ? ((ms * pit_hz + 999) / 1000) : ms / 10);
    if (ticks == 0) ticks = 1;
    task_sleep_ticks(ticks);
}