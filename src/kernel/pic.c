#include "pic.h"
#include "console.h"

static inline void outb_pic(uint16_t port, uint8_t val) {
    outb(port, val);
}

void pic_remap() {
    outb_pic(0x20, 0x11);
    outb_pic(0xA0, 0x11);
    outb_pic(0x21, 0x20);
    outb_pic(0xA1, 0x28);
    outb_pic(0x21, 0x04);
    outb_pic(0xA1, 0x02);
    outb_pic(0x21, 0x01);
    outb_pic(0xA1, 0x01);

    outb_pic(0x21, 0xFE); // 11111110
    outb_pic(0xA1, 0xFF);
}

void pic_set_mask(uint8_t master_mask, uint8_t slave_mask) {
    outb_pic(0x21, master_mask);
    outb_pic(0xA1, slave_mask);
}

void pic_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb_pic(0xA0, 0x20);
    }
    outb_pic(0x20, 0x20);
}