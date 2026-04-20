global save_multi_nt_asm
global save_multi_asm
global prefetch_test_asm
global basic_summation_asm

section .text

;rdi: ptr to place to store
;rsi: ptr to 64byte to store
;rdx: how many time
save_multi_nt_asm:
align 64
vmovdqu ymm0, [rsi]
vmovdqu ymm1, [rsi + 32]
.loop:
    vmovntdq [rdi], ymm0
    vmovntdq [rdi + 32], ymm1
    add rdi, 64
    dec rdx
    jnz .loop
ret

save_multi_asm:
align 64
vmovdqu ymm0, [rsi]
vmovdqu ymm1, [rsi + 32]
.loop:
    vmovdqu [rdi], ymm0
    vmovdqu [rdi + 32], ymm1
    add rdi, 64
    dec rdx
    jnz .loop
ret


prefetch_test_asm:
prefetcht0 [rdi]
ret

basic_summation_asm:
align 64
xor rax, rax
mov r8, 10000
.loop:
    movzx rcx, byte[rdi]
    add rax, rcx
    movzx rcx, byte[rdi + 1]
    add rax, rcx
    movzx rcx, byte[rdi + 2]
    add rax, rcx
    movzx rcx, byte[rdi + 3]
    add rax, rcx
    dec r8
    jnz .loop 
    ret

