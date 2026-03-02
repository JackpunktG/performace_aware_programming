bits 16

; PUSH register
push ax
push bx
push cx
push dx
push sp
push bp
push si
push di

; PUSH segment
push cs
push ds
push es
push ss

; PUSH memory
push word [bx]
push word [bx + 5]
push word [bx - 5]
push word [bx + 256]

; POP register
pop ax
pop bx
pop cx
pop dx
pop sp
pop bp
pop si
pop di

; POP segment
pop ds
pop es
pop ss

; POP memory
pop word [bx]
pop word [bx + 5]
pop word [bx - 5]
pop word [bx + 256]

; XCHG accumulator short form
xchg ax, ax
xchg ax, bx
xchg ax, cx
xchg ax, dx
xchg ax, sp
xchg ax, bp
xchg ax, si
xchg ax, di

; XCHG register
xchg bx, ax
xchg cx, bx

; XCHG memory
xchg [bx], ax
xchg [bx + 5], cx
xchg [bx - 5], dx
xchg [bx + 256], bx