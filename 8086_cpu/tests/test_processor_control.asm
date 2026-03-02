bits 16

; flag operations
clc
stc
cmc
cld
std
cli
sti
hlt
wait

; interrupts
int 0
int 1
int 2
int 3
int 4
int 0x10
int 0x21
int 0xFF

; breakpoint
int 3

; overflow interrupt
into

; interrupt return
iret