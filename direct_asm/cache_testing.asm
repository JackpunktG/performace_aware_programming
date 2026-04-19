global cache_test_asm 
global cache_test_unaligned_asm
global cache_test_mask_asm
global cache_sets_test_error_asm
global cache_sets_test_asm
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

; rdi: buffer pt
; rsi: loop size (must be divisable by 256)
; rdx: loop count
; rcx: offset
cache_test_unaligned_asm:
align 64
xor r8, r8
.loop2:
mov rax, rdi
xor r9, r9
    ; loop for 256 byte at a time
    .loop:
        vmovdqu ymm0, [rax + rcx]
        vmovdqu ymm1, [rax + rcx + 0x20]
        vmovdqu ymm0, [rax + rcx + 0x40]
        vmovdqu ymm1, [rax + rcx + 0x60]
        vmovdqu ymm0, [rax + rcx + 0x80]
        vmovdqu ymm1, [rax + rcx + 0xa0]
        vmovdqu ymm0, [rax + rcx + 0xc0]
        vmovdqu ymm1, [rax + rcx + 0xe0]
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


;rdi buffer
;rsi inner loop count -> x 64 is the total almount we are pulling in
;rdx outer count
;rcx buffer advance
cache_sets_test_asm:
align 64
.outer:
mov r8, rsi
mov rax, rdi
    .loop:
        vmovdqu ymm0, [rax]
        vmovdqu ymm1, [rax + 0x20]
        add rax, rcx
        dec r8
        jnz .loop
    
    dec rdx
    jnz .outer
    ret     
