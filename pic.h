#ifndef PIC_H
#define PIC_H

#include "kernel.h"

void pic_remap();
void pic_set_mask(uint8_t master_mask, uint8_t slave_mask);
void pic_eoi(uint8_t irq);

#endif // PIC_H