bits 16

; SHL
shl ax, 1
shl bx, 1
shl word [bx], 1
shl word [bx + 5], 1
shl ax, cl
shl word [bx], cl

; SHR
shr ax, 1
shr bx, 1
shr word [bx], 1
shr ax, cl
shr word [bx], cl

; SAR
sar ax, 1
sar bx, 1
sar word [bx], 1
sar ax, cl
sar word [bx], cl

; ROL
rol ax, 1
rol bx, 1
rol word [bx], 1
rol ax, cl
rol word [bx], cl

; ROR
ror ax, 1
ror bx, 1
ror word [bx], 1
ror ax, cl
ror word [bx], cl

; RCL
rcl ax, 1
rcl bx, 1
rcl word [bx], 1
rcl ax, cl
rcl word [bx], cl

; RCR
rcr ax, 1
rcr bx, 1
rcr word [bx], 1
rcr ax, cl
rcr word [bx], cl