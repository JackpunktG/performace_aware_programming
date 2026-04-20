global save_multi_nt_asm
global save_multi_asm

section .text

;rdi: ptr to place to store
;rsi: ptr to 64byte to store
;rdx: how many time
save_multi_nt_asm:
align 64
xor rax, rax
xor rcx, rcx
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
xor rax, rax
xor rcx, rcx
vmovdqu ymm0, [rsi]
vmovdqu ymm1, [rsi + 32]
.loop:
    vmovdqu [rdi], ymm0
    vmovdqu [rdi + 32], ymm1
    add rdi, 64
    dec rdx
    jnz .loop
ret


