global cache_test_asm 

section .text

cache_test_asm:
align 64
xor r8, r8
mov rdx, 10000
.loop2:
xor rax, rax
.loop:
    vmovdqu ymm0, [rdi + rax]
    vmovdqu ymm1, [rdi + rax + 32]
    add rax, 64
    cmp rax, rsi
    jb .loop
    inc r8
    cmp r8, rdx
    jb .loop2
    ret

        
