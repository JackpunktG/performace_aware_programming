global mov_cond_jump_asm

section .text

mov_cond_jump_asm:
    xor rax, rax
.loop:
    mov r8, [rdi + rax]
    inc rax
    test r8, 1
    jnz .skip
    nop
.skip:
    cmp rax, rsi
    jb .loop
    ret
