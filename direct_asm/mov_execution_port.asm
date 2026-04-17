global mov_8x1_asm
global mov_8x2_asm
global mov_8x3_asm
global mov_8x4_asm
global mov_1x2_asm
global write_8x1_asm
global write_8x2_asm
global write_8x3_asm
global write_8x4_asm
global write_1x2_asm


section .text

mov_8x1_asm:
align 64
    mov rsi, 100000000
.loop:
    mov rax, [rdi]
    sub rsi, 1
    jnle .loop
    ret

mov_8x2_asm:
align 64
    mov rsi, 100000000
.loop:
    mov rax, [rdi]
    mov rax, [rdi]
    sub rsi, 2
    jnle .loop
    ret

mov_8x3_asm:
align 64
    mov rsi, 100000000
.loop:
    mov rax, [rdi]
    mov rax, [rdi]
    mov rax, [rdi]
    sub rsi, 3
    jnle .loop
    ret

mov_8x4_asm:
align 64
    mov rsi, 100000000
.loop:
    mov rax, [rdi]
    mov rax, [rdi]
    mov rax, [rdi]
    mov rax, [rdi]
    sub rsi, 4
    jnle .loop
    ret

mov_1x2_asm:
align 64
    mov rsi, 100000000
.loop:
    mov al, [rdi]
    mov al, [rdi]
    sub rsi, 2
    jnle .loop
    ret
    
write_8x1_asm:
align 64
    mov rsi, 100000000
.loop:
    mov [rdi], rax
    sub rsi, 1
    jnle .loop
    ret

write_8x2_asm:
align 64
    mov rsi, 100000000
.loop:
    mov [rdi], rax
    mov [rdi], rax
    sub rsi, 2
    jnle .loop
    ret

write_8x3_asm:
align 64
    mov rsi, 100000000
.loop:
    mov [rdi], rax
    mov [rdi], rax
    mov [rdi], rax
    sub rsi, 3
    jnle .loop
    ret

write_8x4_asm:
align 64
    mov rsi, 100000000
.loop:
    mov [rdi], rax
    mov [rdi], rax
    mov [rdi], rax
    mov [rdi], rax
    sub rsi, 4
    jnle .loop
    ret

write_1x2_asm:
align 64
    mov rsi, 100000000
.loop:
    mov [rdi], al
    mov [rdi], al
    sub rsi, 2
    jnle .loop
    ret
       
