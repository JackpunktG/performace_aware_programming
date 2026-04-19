global cache_test_asm 
global cache_test_mask_asm

section .text

; rdi: buffer pt
; rsi: loop size (must be divisable by 256)
; rdx: loop count
cache_test_asm:
align 64
xor r8, r8
.loop2:
mov rax, rdi
xor r9, r9
    ; loop for 256 byte at a time
    .loop:
        vmovdqu ymm0, [rax]
        vmovdqu ymm1, [rax + 0x20]
        vmovdqu ymm0, [rax + 0x40]
        vmovdqu ymm1, [rax + 0x60]
        vmovdqu ymm0, [rax + 0x80]
        vmovdqu ymm1, [rax + 0xa0]
        vmovdqu ymm0, [rax + 0xc0]
        vmovdqu ymm1, [rax + 0xe0]
        add rax, 0x100
        add r9, 0x100
        cmp r9, rsi
        jb .loop
    inc r8
    cmp r8, rdx
    jb .loop2
    ret

cache_test_mask_asm:
align 64
xor r8, r8
mov rax, rdi
.loop:
    vmovdqu ymm0, [rax]
    vmovdqu ymm1, [rax + 0x20]
    vmovdqu ymm0, [rax + 0x40]
    vmovdqu ymm1, [rax + 0x60]
    vmovdqu ymm0, [rax + 0x80]
    vmovdqu ymm1, [rax + 0xa0]
    vmovdqu ymm0, [rax + 0xc0]
    vmovdqu ymm1, [rax + 0xe0]

    add r8, 0x100
    and r8, rdx

    mov rax, rdi
    add rax, r8

    sub rsi, 0x100
    jnz .loop
    ret       
