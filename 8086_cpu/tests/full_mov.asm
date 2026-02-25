bits 16                        

; --- 1. MOV r/m, r  (register to register)
mov cx, bx                      ; 89 D9

; --- 2. MOV r/m, imm  (immediate to register/memory)
mov cx, 12                      ; C7 C1 0C 00

; --- 3. MOV r, imm  (immediate to register, short form)
mov cx, 12                      ; B9 0C 00
mov cl, 12                      ; B1 0C

; --- 4. MOV AX, [addr]  (memory to accumulator)
mov ax, [0x1234]                ; A1 34 12

; --- 5. MOV [addr], AX  (accumulator to memory)
mov [0x1234], ax                ; A3 34 12

; --- 6. MOV r/m, sr  (segment register to register)
mov ax, ds                      ; 8C D8

; --- 7. MOV sr, r/m  (register to segment register)
mov ds, ax                      ; 8E D8

; --- effective address variants ---

; MOD=00 no displacement
mov ax, [bx]                    ; 8B 07
mov ax, [bx + si]               ; 8B 00
mov ax, [bx + di]               ; 8B 01
mov ax, [bp + si]               ; 8B 02
mov ax, [bp + di]               ; 8B 03
mov ax, [si]                    ; 8B 04
mov ax, [di]                    ; 8B 05

; MOD=00 RM=110 special case - direct address NOT [bp]
mov ax, [0x1234]                ; 8B 06 34 12
mov [0x1234], ax                ; 

; MOD=01 8-bit displacement
mov ax, [bx + 5]                ; 8B 47 05
mov ax, [bp + 5]                ; 8B 46 05
mov ax, [si + 5]                ; 8B 44 05
mov ax, [di + 5]                ; 8B 45 05
mov ax, [bx + si + 5]          ; 8B 40 05
mov ax, [bx + di + 5]          ; 8B 41 05
mov ax, [bp + si + 5]          ; 8B 42 05
mov ax, [bp + di + 5]          ; 8B 43 05

; MOD=10 16-bit displacement
mov ax, [bx + 256]              ; 8B 87 00 01
mov ax, [bp + 256]              ; 8B 86 00 01
mov ax, [si + 256]              ; 8B 84 00 01
mov ax, [di + 256]              ; 8B 85 00 01
mov ax, [bx + si + 256]        ; 8B 80 00 01
mov ax, [bx + di + 256]        ; 8B 81 00 01
mov ax, [bp + si + 256]        ; 8B 82 00 01
mov ax, [bp + di + 256]        ; 8B 83 00 01

; --- byte versions (W=0) ---
mov cl, bl                      ; 88 D9
mov cl, 12                      ; B1 0C
mov cl, [bx]                    ; 8A 07
mov [bx], cl                    ; 88 0F

