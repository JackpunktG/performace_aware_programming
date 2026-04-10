global mov_all_bytes_asm
global nop_all_bytes_asm
global cmp_all_bytes_asm
global dec_all_bytes_asm

section .text

mov_all_bytes_asm:
    xor rax, rax
.loop:
    mov [rdi + rax], al
    inc rax
    cmp rax, rsi
    jb .loop
    ret

nop_all_bytes_asm:
    xor rax, rax
.loop:
    db 0x0f, 0x1f, 0x00 
    inc rax
    cmp rax, rsi
    jb .loop
    ret

cmp_all_bytes_asm:
    xor rax, rax
.loop:
    inc rax
    cmp rax, rdi
    jb .loop
    ret

dec_all_bytes_asm:
.loop:
    dec rdi
    jnz .loop
    ret
