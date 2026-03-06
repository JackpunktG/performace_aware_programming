bits 16

; -------- Register to Register --------
mov ax, bx
mov bx, cx
mov cx, dx
mov dx, ax
mov sp, bp
mov bp, si
mov si, di
mov di, sp

; -------- Immediate to Register --------
mov ax, 0x1234
mov bx, 0xABCD
mov cx, 0x7FFF
mov dx, 0
mov si, 255
mov di, 256
mov bp, 32768
mov sp, 1

; -------- Immediate to Memory --------
mov bx, 0x200                  ; BX = 0x200 (test address)
mov word [bx], 0x1111          ; [0x200] = 0x1111
mov word [bx + 2], 0x2222      ; [0x202] = 0x2222
mov word [bx + 4], 0x3333      ; [0x204] = 0x3333

; -------- Register to Memory --------
mov ax, 0xAAAA
mov word [bx], ax              ; [0x200] = 0xAAAA
mov cx, 0xBBBB
mov word [bx + 2], cx          ; [0x202] = 0xBBBB

; -------- Memory to Register --------
mov dx, [bx]                   ; dx = [0x200] (should be 0xAAAA)
mov si, [bx + 2]               ; si = [0x202] (should be 0xBBBB)
mov di, [bx + 4]               ; di = [0x204] (should be 0x3333)

; -------- Segment Register Moves --------
mov ax, ds
mov ax, es
mov ax, ss
mov ax, cs

mov ds, ax
mov es, ax
mov ss, ax
