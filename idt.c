#include "idt.h"
#include "string.h"
#include "console.h"
#include "pit.h"

static struct idt_entry idt[256];
static struct idt_ptr idtp;

extern void isr0(); extern void isr1(); extern void isr2(); extern void isr3();
extern void isr4(); extern void isr5(); extern void isr6(); extern void isr7();
extern void isr8(); extern void isr9(); extern void isr10(); extern void isr11();
extern void isr12(); extern void isr13(); extern void isr14(); extern void isr15();
extern void isr16(); extern void isr17(); extern void isr18(); extern void isr19();
extern void isr20(); extern void isr21(); extern void isr22(); extern void isr23();
extern void isr24(); extern void isr25(); extern void isr26(); extern void isr27();
extern void isr28(); extern void isr29(); extern void isr30(); extern void isr31();

extern void irq0(); extern void irq1(); extern void irq2(); extern void irq3();
extern void irq4(); extern void irq5(); extern void irq6(); extern void irq7();
extern void irq8(); extern void irq9(); extern void irq10(); extern void irq11();
extern void irq12(); extern void irq13(); extern void irq14(); extern void irq15();



void idt_set_gate(int num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = (uint16_t)(base & 0xFFFF);
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
    idt[num].base_high = (uint16_t)((base >> 16) & 0xFFFF);
}

static void idt_load() {
    __asm__ volatile ("lidt %0" : : "m"(idtp));
}

void idt_init() {
    memset(idt, 0, sizeof(idt));
    idtp.limit = sizeof(idt) - 1;
    idtp.base = (uint32_t)&idt;

    // Флаги: 0x8E = Present, DPL=0, 32-bit interrupt gate
    uint8_t flags = 0x8E;
    uint16_t sel = 0x08; // кодовый сегмент из GDT

    // Исключения
    idt_set_gate(0,  (uint32_t)isr0,  sel, flags);
    idt_set_gate(1,  (uint32_t)isr1,  sel, flags);
    idt_set_gate(2,  (uint32_t)isr2,  sel, flags);
    idt_set_gate(3,  (uint32_t)isr3,  sel, flags);
    idt_set_gate(4,  (uint32_t)isr4,  sel, flags);
    idt_set_gate(5,  (uint32_t)isr5,  sel, flags);
    idt_set_gate(6,  (uint32_t)isr6,  sel, flags);
    idt_set_gate(7,  (uint32_t)isr7,  sel, flags);
    idt_set_gate(8,  (uint32_t)isr8,  sel, flags);
    idt_set_gate(9,  (uint32_t)isr9,  sel, flags);
    idt_set_gate(10, (uint32_t)isr10, sel, flags);
    idt_set_gate(11, (uint32_t)isr11, sel, flags);
    idt_set_gate(12, (uint32_t)isr12, sel, flags);
    idt_set_gate(13, (uint32_t)isr13, sel, flags);
    idt_set_gate(14, (uint32_t)isr14, sel, flags);
    idt_set_gate(15, (uint32_t)isr15, sel, flags);
    idt_set_gate(16, (uint32_t)isr16, sel, flags);
    idt_set_gate(17, (uint32_t)isr17, sel, flags);
    idt_set_gate(18, (uint32_t)isr18, sel, flags);
    idt_set_gate(19, (uint32_t)isr19, sel, flags);
    idt_set_gate(20, (uint32_t)isr20, sel, flags);
    idt_set_gate(21, (uint32_t)isr21, sel, flags);
    idt_set_gate(22, (uint32_t)isr22, sel, flags);
    idt_set_gate(23, (uint32_t)isr23, sel, flags);
    idt_set_gate(24, (uint32_t)isr24, sel, flags);
    idt_set_gate(25, (uint32_t)isr25, sel, flags);
    idt_set_gate(26, (uint32_t)isr26, sel, flags);
    idt_set_gate(27, (uint32_t)isr27, sel, flags);
    idt_set_gate(28, (uint32_t)isr28, sel, flags);
    idt_set_gate(29, (uint32_t)isr29, sel, flags);
    idt_set_gate(30, (uint32_t)isr30, sel, flags);
    idt_set_gate(31, (uint32_t)isr31, sel, flags);

    // IRQ (после ремапа PIC будут 32..47)
    idt_set_gate(32, (uint32_t)irq0,  sel, flags);
    idt_set_gate(33, (uint32_t)irq1,  sel, flags);
    idt_set_gate(34, (uint32_t)irq2,  sel, flags);
    idt_set_gate(35, (uint32_t)irq3,  sel, flags);
    idt_set_gate(36, (uint32_t)irq4,  sel, flags);
    idt_set_gate(37, (uint32_t)irq5,  sel, flags);
    idt_set_gate(38, (uint32_t)irq6,  sel, flags);
    idt_set_gate(39, (uint32_t)irq7,  sel, flags);
    idt_set_gate(40, (uint32_t)irq8,  sel, flags);
    idt_set_gate(41, (uint32_t)irq9,  sel, flags);
    idt_set_gate(42, (uint32_t)irq10, sel, flags);
    idt_set_gate(43, (uint32_t)irq11, sel, flags);
    idt_set_gate(44, (uint32_t)irq12, sel, flags);
    idt_set_gate(45, (uint32_t)irq13, sel, flags);
    idt_set_gate(46, (uint32_t)irq14, sel, flags);
    idt_set_gate(47, (uint32_t)irq15, sel, flags);

    idt_load();
}

// Анти-спам и дружелюбные сообщения для критических исключений
static uint32_t gp_reported = 0;
static uint32_t pf_reported = 0;

void isr_handler(uint32_t int_no, uint32_t err_code, uint32_t esp) {
    if (int_no == 13) { // General Protection Fault
        if (!gp_reported) {
            uint32_t eip = *((uint32_t*)(esp + 44));
            uint32_t cs  = *((uint32_t*)(esp + 48));
            uint32_t efl = *((uint32_t*)(esp + 52));
            console_set_color(VGA_COLOR_LIGHT_RED);
            console_print("General Protection Fault (#13), err=");
            console_print_hex(err_code);
            console_print(" EIP=");
            console_print_hex(eip);
            console_print(" CS=");
            console_print_hex(cs);
            console_print(" EFLAGS=");
            console_print_hex(efl);
            console_print("\n");
            console_set_color(VGA_COLOR_LIGHT_GREY);
        }
        gp_reported = 1; // печатаем только один раз
        return; // не засоряем экран повторным выводом
    }
    if (int_no == 14) { // Page Fault
        if (!pf_reported) {
            console_set_color(VGA_COLOR_LIGHT_RED);
            console_print("Page Fault (#14), err=");
            console_print_hex(err_code);
            console_print("\n");
            console_set_color(VGA_COLOR_LIGHT_GREY);
        }
        pf_reported = 1;
        return;
    }

    // Остальные исключения — один раз, кратко
    static uint8_t shown[32] = {0};
    if (int_no < 32 && !shown[int_no]) {
        console_print("CPU exception: ");
        console_print(itoa((int)int_no, (char[12]){0}, 10));
        console_print("\n");
        shown[int_no] = 1;
    }
}

void irq_handler(uint32_t int_no, uint32_t esp) {
    if (int_no == 32) {
        pit_on_tick(esp);
    }
}