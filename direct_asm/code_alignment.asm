global aligned_asm
global plus1_aligned_asm
global plus15_aligned_asm
global plus31_aligned_asm
global plus63_aligned_asm

section .text


aligned_asm:
    xor rax, rax
align 64
.loop:
    inc rax
    cmp rax, rdi
    jb .loop
    ret

plus1_aligned_asm:
    xor rax, rax
align 64
nop
.loop:
    inc rax
    cmp rax, rdi
    jb .loop
    ret

plus15_aligned_asm:
    xor rax, rax
align 64
%rep 15
nop
%endrep
.loop:
    inc rax
    cmp rax, rdi
    jb .loop
    ret

plus31_aligned_asm:
    xor rax, rax
align 64
%rep 31
nop
%endrep
.loop:
    inc rax
    cmp rax, rdi
    jb .loop
    ret


plus63_aligned_asm:
    xor rax, rax
align 64
%rep 63
nop
%endrep
.loop:
    inc rax
    cmp rax, rdi
    jb .loop
    ret







