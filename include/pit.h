#ifndef PIT_H
#define PIT_H

#include "kernel.h"

void pit_init(uint32_t hz);
void pit_on_tick(uint32_t esp);
extern volatile uint32_t timer_ticks;
extern uint32_t pit_hz;

#endif