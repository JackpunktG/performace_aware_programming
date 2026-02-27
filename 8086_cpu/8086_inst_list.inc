#ifndef INSTRUCTION
#define INSTRUCTION(Mnemonic, Encoding, ...) {Op_##Mnemonic, Encoding, __VA_ARGS__},
#endif

#ifndef INST
#define INST INSTRUCTION
#endif

#define B(Bits) {Bits_OP, sizeof(#Bits)-1, 0, 0b##Bits}
#define BL(Bits) {Bits_Literal, sizeof(#Bits)-1, 0, 0b##Bits}
#define D {Bits_D, 1}
#define W {Bits_W, 1}
#define S {Bits_S, 1}


#define RM {Bits_RM, 3}
#define MOD {Bits_MOD, 2}
#define REG {Bits_REG, 3}
#define SR {Bits_SR, 2}

#define IP8 {Bits_IP_INC8, 8}

#define DISP_L {Bits_Disp_L, 8}
#define DISP_H {Bits_Disp_H, 8}
#define DATA_L {Bits_Data_L, 8}
#define DATA_H {Bits_Data_H, 8}

#define IMP_D(value) {Bits_D, 1, 0, value}
#define IMP_REG(value) {Bits_REG, 3, 0, value}
#define IMP_MOD(value) {Bits_MOD, 2, 0, value}
#define IMP_RM(value) {Bits_RM, 3, 0, value}

INSTRUCTION(mov, {B(100010), D, W, MOD, REG, RM, DISP_L, DISP_H})
INST(mov, {B(1100011), W, MOD, BL(000), RM, DISP_L, DISP_H, DATA_L, DATA_H})
INST(mov, {B(1011), W, REG, DATA_L, DATA_H})
INST(mov, {B(1010000), W, DISP_L, DISP_H})
INST(mov, {B(1010001), W, DISP_L, DISP_H})
INST(mov, {B(10001110), MOD, BL(0), SR, RM, DISP_L, DISP_H})
INST(mov, {B(10001100), MOD, BL(0), SR, RM, DISP_L, DISP_H})

// Arithmetic and Logical Instructions
INSTRUCTION(add, {B(000000), D, W, MOD, REG, RM, DISP_L, DISP_H})
INST(add, {B(100000), S, W, MOD, BL(000), RM, DISP_L, DISP_H, DATA_L, DATA_H})
INST(add, {B(0000010), W, DATA_L, DATA_H})
INSTRUCTION(or, {B(000010), D, W, MOD, REG, RM, DISP_L, DISP_H})
INST(or, {B(100000), S, W, MOD, BL(001), RM, DISP_L, DISP_H, DATA_L, DATA_H})
INST(or, {B(0000110), W, DATA_L, DATA_H})
INSTRUCTION(adc, {B(000100), D, W, MOD, REG, RM, DISP_L, DISP_H})
INST(adc, {B(100000), S, W, MOD, BL(010), RM, DISP_L, DISP_H, DATA_L, DATA_H})
INST(adc, {B(0001010), W, DATA_L, DATA_H})
INSTRUCTION(sbb, {B(000110), D, W, MOD, REG, RM, DISP_L, DISP_H})
INST(sbb, {B(100000), S, W, MOD, BL(011), RM, DISP_L, DISP_H, DATA_L, DATA_H})
INST(sbb, {B(0001110), W, DATA_L, DATA_H})
INSTRUCTION(and, {B(001000), D, W, MOD, REG, RM, DISP_L, DISP_H})
INST(and, {B(100000), S, W, MOD, BL(100), RM, DISP_L, DISP_H, DATA_L, DATA_H})
INST(and, {B(0010010), W, DATA_L, DATA_H})
INSTRUCTION(sub, {B(001010), D, W, MOD, REG, RM, DISP_L, DISP_H})
INST(sub, {B(100000), S, W, MOD, BL(101), RM, DISP_L, DISP_H, DATA_L, DATA_H})
INST(sub, {B(0010110), W, DATA_L, DATA_H})
INSTRUCTION(xor, {B(001100), D, W, MOD, REG, RM, DISP_L, DISP_H})
INST(xor, {B(100000), S, W, MOD, BL(110), RM, DISP_L, DISP_H, DATA_L, DATA_H})
INST(xor, {B(0011010), W, DATA_L, DATA_H})
INSTRUCTION(cmp, {B(001110), D, W, MOD, REG, RM, DISP_L, DISP_H})
INST(cmp, {B(100000), S, W, MOD, BL(111), RM, DISP_L, DISP_H, DATA_L, DATA_H})
INST(cmp, {B(0011110), W, DATA_L, DATA_H})
INSTRUCTION(test, {B(000100), D, W, MOD, REG, RM, DISP_L, DISP_H})
INST(test, {B(1111011), W, MOD, BL(000), RM, DISP_L, DISP_H, DATA_L, DATA_H})
INST(test, {B(1010100), W, DATA_L})
INSTRUCTION(inc, {B(1111111), W, MOD, BL(000), RM, DISP_L, DISP_H})
INST(inc, {B(01000), REG})
INSTRUCTION(dec, {B(1111111), W, MOD, BL(001), RM, DISP_L, DISP_H})
INST(dec, {B(01001), REG})
INSTRUCTION(neg, {B(1111011), W, MOD, BL(011), RM, DISP_L, DISP_H})
INSTRUCTION(aaa, {B(00110111)})
INSTRUCTION(daa, {B(00100111)})

// conditional jumps
INSTRUCTION(je, {B(01110100), IP8})
INSTRUCTION(jl, {B(01111100), IP8})
INSTRUCTION(jle, {B(01111110), IP8})
INSTRUCTION(jb, {B(01110010), IP8})
INSTRUCTION(jbe, {B(01110110), IP8})
INSTRUCTION(jp, {B(01111010), IP8})
INSTRUCTION(jo, {B(01110000), IP8})
INSTRUCTION(js, {B(01111000), IP8})
INSTRUCTION(jne, {B(01110101), IP8})
INSTRUCTION(jnl, {B(01111101), IP8})
INSTRUCTION(jnle, {B(01111111), IP8})
INSTRUCTION(jnb, {B(01110011), IP8})
INSTRUCTION(jnbe, {B(01110111), IP8})
INSTRUCTION(jnp, {B(01111011), IP8})
INSTRUCTION(jno, {B(01110001), IP8})
INSTRUCTION(jns, {B(01111001), IP8})
INSTRUCTION(loop, {B(11100010), IP8})
INSTRUCTION(loopz, {B(11100001), IP8})
INSTRUCTION(loopnz, {B(11100000), IP8})
INSTRUCTION(jcxz, {B(11100011), IP8})

#undef INST
#undef INSTRUCTION

#undef B
#undef D
#undef S
#undef W

#undef IMP_D
#undef IMP_REG
#undef IMP_MOD
#undef IMP_RM

#undef RM
#undef MOD
#undef REG

#undef DISP_L
#undef DISP_H
#undef DATA_L
#undef DATA_H

