; Test file for register allocation and instruction selection

bits 16

mov ax, bx
mov bx, cx
mov cx, dx
mov dx, si
mov si, di
mov di, bp
mov bp, sp
mov sp, ax
mov ax, si
mov ax, di
mov bx, bp
mov cx, sp
mov dx, bp
mov si, ax
mov di, bx
mov bp, cx
mov sp, dx
mov ax, ax
mov bx, bx
mov sp, sp
mov bp, bp
mov al, bl
mov ah, cl
mov bl, dh
mov bh, al
mov cl, ah
mov ch, bh
mov dl, ch
mov dh, dl
mov al, al
mov ah, ah
mov ch, ch
mov dl, dl
