; output from encoding add_sub_cmp to assembly

bits 16

add bx, [bx + si]
add bx, [bp + 0]
add ax, 2
add ax, 2
add ax, 8
add bx, [bp + 0]
add cx, [bx + 2]
add bh, [bp + si + 4]
add di, [bp + di + 6]
add [bx + si], bx
add [bp + 0], bx
add [bp + 0], bx
add [bx + 2], cx
add [bp + si + 4], bh
add [bp + di + 6], di
add byte [bx], 34
add word [bp + si + 1000], 29
add ax, [bp + 0]
add al, [bx + si]
add ax, bx
add al, ah
add ax, 1000
add ax, 226
add ax, 9
sub bx, [bx + si]
sub bx, [bp + 0]
sub bp, 2
sub bp, 2
sub bp, 8
sub bx, [bp + 0]
sub cx, [bx + 2]
sub bh, [bp + si + 4]
sub di, [bp + di + 6]
sub [bx + si], bx
sub [bp + 0], bx
sub [bp + 0], bx
sub [bx + 2], cx
sub [bp + si + 4], bh
sub [bp + di + 6], di
sub byte [bx], 34
sub word [bx + di], 29
sub ax, [bp + 0]
sub al, [bx + si]
sub ax, bx
sub al, ah
sub ax, 1000
sub ax, 226
sub ax, 9
cmp bx, [bx + si]
cmp bx, [bp + 0]
cmp di, 2
cmp di, 2
cmp di, 8
cmp bx, [bp + 0]
cmp cx, [bx + 2]
cmp bh, [bp + si + 4]
cmp di, [bp + di + 6]
cmp [bx + si], bx
cmp [bp + 0], bx
cmp [bp + 0], bx
cmp [bx + 2], cx
cmp [bp + si + 4], bh
cmp [bp + di + 6], di
cmp byte [bx], 34
cmp word [4834], 29
cmp ax, [bp + 0]
cmp al, [bx + si]
cmp ax, bx
cmp al, ah
cmp ax, 1000
cmp ax, 226
cmp ax, 9
