BITS 32
GLOBAL _start
SECTION .data
msg db "RESULT:", 0x20
msg_len equ $-msg
expr db "12+3-5+10",0
buf db 12 dup(0)
SECTION .text
_start:
mov esi, expr
mov ebx, 1
call parse
mov ebx, buf
call itoa
mov ebx, msg
mov ecx, msg_len
mov eax, 1
int 0x80
mov ebx, buf
mov ecx, [buf_len]
mov eax, 1
int 0x80
mov eax, 2
xor ebx, ebx
int 0x80
parse:
xor edx, edx
mov ecx, 0
mov ebx, 1
.loop:
lodsb
cmp al, 0
je .end
cmp al, '+'
je .apply_plus
cmp al, '-'
je .apply_minus
cmp al, '0'
jb .loop
cmp al, '9'
ja .loop
sub al, '0'
movzx eax, al
imul ecx, ecx, 10
add ecx, eax
jmp .loop
.apply_plus:
mov eax, ebx
imul eax, ecx
add edx, eax
mov ecx, 0
mov ebx, 1
jmp .loop
.apply_minus:
mov eax, ebx
imul eax, ecx
add edx, eax
mov ecx, 0
mov ebx, -1
jmp .loop
.end:
mov eax, ebx
imul eax, ecx
add edx, eax
mov eax, edx
ret
itoa:
mov edi, ebx
mov ebp, 10
cmp eax, 0
jne .conv
mov byte [edi], '0'
mov dword [buf_len], 1
ret
.conv:
mov ecx, 0
.push_loop:
xor edx, edx
div ebp
add dl, '0'
push edx
inc ecx
test eax, eax
jne .push_loop
.pop_loop:
pop edx
mov [edi], dl
inc edi
loop .pop_loop
mov eax, edi
sub eax, ebx
mov [buf_len], eax
ret
SECTION .bss
buf_len resd 1