[BITS 32]

extern _kernel_main

global _start
_start:
    mov esp, 0x90000
    
    call _kernel_main
    
    cli
    hlt

hang:
    hlt
    jmp hang