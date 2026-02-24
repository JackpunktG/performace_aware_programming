; output from encoding basic_mov_inst to assembly

bits 16

mov ax, bx
mov bx, ax
mov cx, dx
mov dx, cx
mov si, di
mov di, si
mov bp, sp
mov sp, bp
mov al, bl
mov ah, bh
mov cl, dl
mov dh, ch
mov bl, al
mov bh, ah
mov al, 0
mov al, 127
mov al, 255
mov al, 128
mov cl, 12
mov ch, 244
mov dl, 127
mov dh, 128
mov ax, 0
mov ax, 1
mov ax, 32767
mov ax, 65535
mov ax, 32768
mov cx, 12
mov cx, 65524
mov dx, 3948
mov dx, 61588
mov si, 48879
mov di, 65499
mov al, [bx + si]
mov al, [bx + di]
mov al, [bp + si]
mov al, [bp + di]
mov al, [si]
mov al, [di]
mov al, [bx]
mov al, [bp + 0]
mov ax, [bx + si]
mov ax, [bx + di]
mov ax, [bp + si]
mov ax, [bp + di]
mov ax, [si]
mov ax, [di]
mov ax, [bx]
mov ax, [bp + 0]
mov al, [bx + si + 4]
mov al, [bx + di + 127]
mov al, [bp + si + 1]
mov al, [bp + di]
mov al, [si + 5]
mov al, [di + 6]
mov al, [bx + 7]
mov al, [bp + 8]
mov al, [bx + si + -1]
mov al, [bx + di + -37]
mov al, [bp + si + -128]
mov al, [si + -12]
mov al, [bp + -2]
mov ax, [bx + si + 4999]
mov ax, [bx + di + 300]
mov ax, [bp + si + 1234]
mov ax, [bp + di + 8738]
mov ax, [si + 901]
mov ax, [di + 902]
mov ax, [bx + 903]
mov ax, [bp + 904]
mov ax, [bx + si + -300]
mov ax, [bp + di + -3948]
mov ax, [si + -901]
mov ax, [bp + -500]
mov [bx + si], ax
mov [bx + di], cx
mov [bp + si], dx
mov [bp + di], si
mov [si], di
mov [di], bp
mov [bx], sp
mov [bp + 0], bx
mov [bx + di], cl
mov [bp + si], ch
mov [si], al
mov [di], ah
mov [bx + si + 4], ax
mov [bp + di + 8], cx
mov [si + 1], dl
mov [bp + -2], dh
mov [bx + -32], al
mov [di + -37], ah
mov [di + 901], word 347
mov [bx + si + 4999], dx
mov [bp + di + -300], ax
mov [si + -901], cx
mov [bp + di], byte 7
mov [bx + si], byte -1
mov [si + 5], byte -128
mov [di + 6], byte 127
mov [bp + di], word 347
mov [bx + si + 10], word -12
mov [bp + -2], word -32768
mov [di + 901], word -37
