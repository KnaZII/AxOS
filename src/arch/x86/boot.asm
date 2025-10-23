[BITS 16]
[ORG 0x7C00]

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    
    mov [boot_drive], dl
    mov [0x0500], dl
    
    mov ah, 0x00
    mov al, 0x03
    int 0x10
    
    mov si, welcome_msg
    call print_string
    
    mov ah, 0x00
    mov dl, [boot_drive]
    int 0x13
    
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    
    mov ch, 0
    mov dh, 0
    mov cl, 2
    mov dl, [boot_drive]

    mov si, 64
.load_loop:
    mov al, 1
    call read_sectors_16
    jnc .ok_load
    jmp disk_error
.ok_load:
    ; advance ES:BX by 512 bytes
    add bx, 512
    jc .inc_es
    jmp .advance
.inc_es:
    mov ax, es
    add ax, 0x20
    mov es, ax
.advance:
    ; advance CHS
    inc cl
    cmp cl, 19
    jb .cont_load
    mov cl, 1
    inc dh
    cmp dh, 2
    jb .cont_load
    mov dh, 0
    inc ch
.cont_load:
    dec si
    jnz .load_loop

.read_ok:
    mov si, success_msg
    call print_string
    
    cli
    lgdt [gdt_descriptor]
    
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    jmp 0x08:protected_mode

disk_error:
    mov si, disk_error_msg
    call print_string
    
    mov al, ah
    call print_hex_byte
    
    mov si, crlf
    call print_string
    hlt
    jmp $

print_string:
    mov ah, 0x0E
.loop:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .loop
.done:
    ret

read_sectors_16:
    pusha
    mov si, 2
.rs_try:
    mov ah, 0x02
    int 0x13
    jnc .rs_ok
    dec si
    js .rs_done
    xor ah, ah
    mov dl, [boot_drive]
    int 0x13
    jnc .rs_try
    jmp .rs_done
.rs_ok:
    clc
.rs_done:
    popa
    ret

print_hex_byte:
    push ax
    push bx
    mov bh, al
    mov ah, 0x0E
    
    mov al, bh
    shr al, 4
    call .nibble
    int 0x10
    
    mov al, bh
    and al, 0x0F
    call .nibble
    int 0x10
    
    pop bx
    pop ax
    ret
.nibble:
    cmp al, 9
    jbe .dec
    add al, 7
.dec:
    add al, '0'
    ret

welcome_msg db 'AxOS Bootloader v1.0', 13, 10, 'Loading kernel...', 13, 10, 0
success_msg db 'Kernel loaded successfully!', 13, 10, 0
disk_error_msg db 'Disk error: ', 0
crlf db 13, 10, 0
boot_drive db 0

gdt_start:
    dd 0x0
    dd 0x0
    
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00
    
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[BITS 32]
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    jmp 0x10000

times 510-($-$$) db 0
dw 0xAA55