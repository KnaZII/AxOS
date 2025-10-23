#ifndef IDT_H
#define IDT_H

#include "kernel.h"

struct idt_entry {
    uint16_t base_low;
    uint16_t sel;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

void idt_init();
void idt_set_gate(int num, uint32_t base, uint16_t sel, uint8_t flags);

// Глобальный флаг для переключения стека задач из обработчика IRQ
extern volatile uint32_t switch_to_esp;

// Обработчики верхнего уровня на C
void isr_handler(uint32_t int_no, uint32_t err_code, uint32_t esp);
void irq_handler(uint32_t int_no, uint32_t esp);

#endif // IDT_H