bits 16

; ADD
add ax, bx
add bx, cx
add cx, dx
add ax, 1
add ax, 255
add ax, 256
add word [bx], ax
add ax, [bx]
add word [bx + 5], ax
add ax, [bx + 5]
add word [bx - 5], cx
add word [bx + 256], dx

; SUB
sub ax, bx
sub bx, cx
sub cx, dx
sub ax, 1
sub ax, 255
sub ax, 256
sub word [bx], ax
sub ax, [bx]
sub word [bx + 5], ax
sub ax, [bx + 5]

; AND
and ax, bx
and bx, cx
and ax, 1
and ax, 255
and word [bx], ax
and ax, [bx]
and word [bx + 5], ax

; OR
or ax, bx
or bx, cx
or ax, 1
or ax, 255
or word [bx], ax
or ax, [bx]

; XOR
xor ax, bx
xor bx, cx
xor ax, 1
xor ax, 255
xor word [bx], ax
xor ax, [bx]

; CMP
cmp ax, bx
cmp bx, cx
cmp ax, 1
cmp ax, 255
cmp ax, [bx]
cmp word [bx], ax
cmp ax, [bx + 5]

; INC
inc ax
inc bx
inc cx
inc dx
inc sp
inc bp
inc si
inc di
inc word [bx]
inc word [bx + 5]
inc word [bx - 5]

; DEC
dec ax
dec bx
dec cx
dec dx
dec sp
dec bp
dec si
dec di
dec word [bx]
dec word [bx + 5]
dec word [bx - 5]

; NEG
neg ax
neg bx
neg word [bx]
neg word [bx + 5]

; NOT
not ax
not bx
not word [bx]
not word [bx + 5]