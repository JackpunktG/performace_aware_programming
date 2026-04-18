global mov_8bit_asm
global mov_16bit_asm
global mov_32bit_asm
global mov_64bit_asm
global mov_128bit_asm
global mov_256bit_asm

section .text

mov_8bit_asm:
xor r11, r11
align 64
.loop: 
    mov r9b, [rdi] 
    mov r9b, [rdi + 1]
    add r11, 2
    cmp r11, rsi
    jb .loop
    ret

mov_16bit_asm:
xor r11, r11
align 64
.loop: 
    mov r9d, [rdi] 
    mov r9d, [rdi + 2]
    add r11, 4
    cmp r11, rsi
    jb .loop
    ret

mov_32bit_asm:
xor r11, r11
align 64
.loop: 
    mov r9d, [rdi] 
    mov r9d, [rdi + 4]
    add r11, 8
    cmp r11, rsi
    jb .loop
    ret

mov_64bit_asm:
xor r11, r11
align 64
.loop: 
    mov r9, [rdi] 
    mov r9, [rdi + 8]
    add r11, 16
    cmp r11, rsi
    jb .loop
    ret

mov_128bit_asm:
xor r11, r11
align 64
.loop: 
    vmovdqu xmm0, [rdi] 
    vmovdqu xmm0, [rdi + 16]
    add r11, 32
    cmp r11, rsi
    jb .loop
    ret


mov_256bit_asm:
xor r11, r11
align 64
.loop: 
    vmovdqu ymm0, [rdi] 
    vmovdqu ymm1, [rdi + 32]
    add r11, 64
    cmp r11, rsi
    jb .loop
    ret
