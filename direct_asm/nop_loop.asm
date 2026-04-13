global nop1x3_asm
global nop1x9_asm

section .text

nop1x3_asm:
    xor rax, rax
.loop:
    nop
    nop
    nop 
    inc rax
    cmp rax, rsi
    jb .loop
    nop
    ret

nop1x9_asm:
    xor rax, rax
.loop:
    nop
    nop
    nop 
    nop
    nop
    nop 
    nop
    nop
    nop 
    inc rax
    cmp rax, rsi
    jb .loop
    ret


