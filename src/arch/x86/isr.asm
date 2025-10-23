BITS 32

GLOBAL isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7
GLOBAL isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15
GLOBAL isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23
GLOBAL isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
GLOBAL irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7
GLOBAL irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15
GLOBAL _isr0, _isr1, _isr2, _isr3, _isr4, _isr5, _isr6, _isr7
GLOBAL _isr8, _isr9, _isr10, _isr11, _isr12, _isr13, _isr14, _isr15
GLOBAL _isr16, _isr17, _isr18, _isr19, _isr20, _isr21, _isr22, _isr23
GLOBAL _isr24, _isr25, _isr26, _isr27, _isr28, _isr29, _isr30, _isr31
GLOBAL _irq0, _irq1, _irq2, _irq3, _irq4, _irq5, _irq6, _irq7
GLOBAL _irq8, _irq9, _irq10, _irq11, _irq12, _irq13, _irq14, _irq15

EXTERN _isr_handler
EXTERN _irq_handler
EXTERN _switch_to_esp
EXTERN _switch_to_cr3

%macro ISR_NOERR 1
isr%1:
_isr%1:
    push dword 0
    push dword %1
    pusha
    push ds
    push es
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov eax, esp
    push eax
    push dword 0
    push dword %1
    call _isr_handler
    add esp, 12
    pop es
    pop ds
    popa
    add esp, 8
    iretd
%endmacro

%macro ISR_ERR 1
isr%1:
_isr%1:
    pusha
    push ds
    push es
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov eax, esp
    push eax
    mov eax, [esp + 44]
    push eax
    push dword %1
    call _isr_handler
    add esp, 12
    pop es
    pop ds
    popa
    add esp, 4
    iretd
%endmacro

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_ERR   21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

%macro IRQ_STUB 1
irq%1:
_irq%1:
    pusha
    push ds
    push es
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov eax, esp
    add eax, 8
    push eax
    push dword (32 + %1)
    call _irq_handler
    add esp, 8
    %if %1 >= 8
        mov al, 0x20
        out 0xA0, al
    %endif
        mov al, 0x20
        out 0x20, al
    pop es
    pop ds
    mov eax, [_switch_to_cr3]
    test eax, eax
    jz .no_cr3
    mov dword [_switch_to_cr3], 0
    mov cr3, eax
.no_cr3:
    mov eax, [_switch_to_esp]
    test eax, eax
    jz .no_switch
    mov dword [_switch_to_esp], 0
    mov esp, eax
.no_switch:
    popa
    iretd
%endmacro

IRQ_STUB 0
IRQ_STUB 1
IRQ_STUB 2
IRQ_STUB 3
IRQ_STUB 4
IRQ_STUB 5
IRQ_STUB 6
IRQ_STUB 7
IRQ_STUB 8
IRQ_STUB 9
IRQ_STUB 10
IRQ_STUB 11
IRQ_STUB 12
IRQ_STUB 13
IRQ_STUB 14
IRQ_STUB 15
GLOBAL isr128
GLOBAL _isr128
EXTERN _syscall_handler

isr128:
_isr128:
    pusha
    push ds
    push es
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov eax, esp
    push eax
    call _syscall_handler
    add esp, 4
    pop es
    pop ds
    popa
    iretd