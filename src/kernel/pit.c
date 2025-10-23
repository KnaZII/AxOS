#include "pit.h"
#include "console.h"
#include "idt.h"
#include "task.h"

volatile uint32_t timer_ticks = 0;
uint32_t pit_hz = 0;

void pit_init(uint32_t hz) {
    uint32_t divisor = 1193182 / hz;
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
    pit_hz = hz;
}

void pit_on_tick(uint32_t esp) {
    timer_ticks++;
    schedule(esp);
}