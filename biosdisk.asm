[BITS 32]

global bios_read_lba

SECTION .bss
align 16
dap: resb 16
gdtr_save: resb 6

SECTION .text
bios_read_lba:
    push ebp
    mov ebp, esp
    pushad
    cli
    sgdt [gdtr_save]
    mov eax, [ebp+8]
    mov dword [dap+8], eax
    mov dword [dap+12], 0
    movzx ecx, word [ebp+12]
    mov word [dap+2], cx
    mov eax, [ebp+16]
    mov edx, eax
    and edx, 0xF
    mov word [dap+4], dx
    mov edx, eax
    shr edx, 4
    mov word [dap+6], dx
    mov byte [dap], 16
    mov byte [dap+1], 0
    mov eax, cr0
    and eax, 0xFFFFFFFE
    mov cr0, eax
    jmp 0x0000:rm_entry

[BITS 16]
rm_entry:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov si, dap
    mov dl, [0x0500]
    mov ah, 0x42
    int 0x13
    jc rm_fail
    mov word [rm_status], 1
    jmp rm_back

rm_fail:
    mov word [rm_status], 0

rm_back:
    lgdt [gdtr_save]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp dword 0x08:pm_entry

rm_status dw 0

[BITS 32]
pm_entry:
    sti
    mov ax, [rm_status]
    movzx eax, ax
    popad
    mov esp, ebp
    pop ebp
    ret