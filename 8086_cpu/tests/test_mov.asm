bits 16

; register to register
mov ax, bx
mov bx, cx
mov cx, dx
mov dx, ax
mov sp, bp
mov bp, si
mov si, di
mov di, sp

; immediate to register
mov ax, 0
mov bx, 1
mov cx, 255
mov dx, 256
mov sp, 1000
mov bp, 32767
mov si, 100
mov di, 200

; immediate to memory
mov word [bx], 0
mov word [bx], 255
mov word [bx], 256
mov word [bx + 5], 100
mov word [bx - 5], 200
mov word [bx + 256], 1000

; register to memory
mov [bx], ax
mov [bx], bx
mov [bx + 5], cx
mov [bx - 5], dx
mov [bx + 256], sp

; memory to register
mov ax, [bx]
mov bx, [bx + 5]
mov cx, [bx - 5]
mov dx, [bx + 256]
mov sp, [bx + 10]

; segment registers
mov ax, cs
mov ax, ds
mov ax, es
mov ax, ss
mov ds, ax
mov es, ax
mov ss, ax