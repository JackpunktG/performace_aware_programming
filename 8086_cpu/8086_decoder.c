/* =================================================================
    8086 instruction decoder
    outputs the assembly instructions for a given binary file containing 8086 machine code,
    ================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#define PAP_HELPER_IMPLEMENTATION
#include "../pap_helper.h"

#define WORD_BIT      (1<<0)
#define DIRECTION_BIT (1<<1)
typedef enum
{
    CONDITIONAL_RET_INST,
    MATH_OP_REG_MEM_EITHER,
    MATH_OP_IMM_REG_MEM,
    MATH_OP_IMM_ACC,
    MOV_REG_MEM,       //  = 0b100010,
    MOV_IMM_REG,       // = 0b1011,
    MOV_IMM_REG_MEM,  //  = 0b1100011,
    MOV_MEM_ACC,       //  = 0b1010000,
    MOV_ACC_MEM,
} Instruction;
#define INSTRUCTION_BITMASK 0b11111100


typedef enum
{
    JO     = 0x70,
    JNO    = 0x71,
    JB     = 0x72,
    JNB    = 0x73,
    JE     = 0x74,
    JNE    = 0x75,
    JBE    = 0x76,
    JNBE   = 0x77,
    JS     = 0x78,
    JNS    = 0x79,
    JP     = 0x7A,
    JNP    = 0x7B,
    JL     = 0x7C,
    JNL    = 0x7D,
    JLE    = 0x7E,
    JNLE   = 0x7F,
    LOOPNE = 0xE0,
    LOOPE  = 0xE1,
    LOOP   = 0xE2,
    JCXZ   = 0xE3

} CONDITIONAL_RET;

char* conditional_ret_string(CONDITIONAL_RET inst)
{
    switch (inst)
    {
    case LOOPNE:
        return "loopne"; //loop while now zero or equal
    case LOOPE:
        return "loope"; // loop while zero or equal
    case LOOP:
        return "loop"; // loop CX times
    case JCXZ:
        return "jcxz"; // jump on CX zero
    case JO:
        return "jo";   // jump if overflow
    case JNO:
        return "jno";  // jump if not overflow
    case JB:
        return "jb";   // jump if below (unsigned <)
    case JNB:
        return "jnb";  // jump if not below (unsigned >=)
    case JE:
        return "je";   // jump if equal (zero)
    case JNE:
        return "jne";  // jump if not equal (not zero)
    case JBE:
        return "jbe";  // jump if below or equal (unsigned <=)
    case JNBE:
        return "jnbe"; // jump if not below or equal (unsigned >)
    case JS:
        return "js";   // jump if sign (negative)
    case JNS:
        return "jns";  // jump if not sign (non-negative)
    case JP:
        return "jp";   // jump if parity (parity even)
    case JNP:
        return "jnp";  // jump if not parity (parity odd)
    case JL:
        return "jl";   // jump if less (signed <)
    case JNL:
        return "jnl";  // jump if not less (signed >=)
    case JLE:
        return "jle";  // jump if less or equal (signed <=)
    case JNLE:
        return "jnle"; // jump if not less or equal (signed >)
    default:
        assert(0);
    }
}

static inline uint8_t bit_check(const uint8_t byte, const uint8_t bit_check)
{
    return byte & (1<<bit_check) ? 1 : 0;
}

Instruction instruction_from_byte(uint8_t byte)
{
    //print_binary_8(byte);

    if ((byte >= JO && byte <= JNLE) || (byte >= LOOPNE && byte <= JCXZ))
        return CONDITIONAL_RET_INST;

    // msb
    if (bit_check(byte, 7))
    {
        // 2 msb
        if(bit_check(byte, 6))
        {
            // 3 msb
            if (bit_check(byte, 5))
            {
                // 4 msb
                if (bit_check(byte, 4))
                {}
            }
            else
            {
                // 4 msb
                if (bit_check(byte, 4))
                {}
                else
                {
                    // 5 msb
                    if (bit_check(byte, 3))
                    {}
                    else
                    {
                        // 6 msb
                        if (bit_check(byte, 2))
                        {
                            // 7 msb
                            if (bit_check(byte, 1))
                                return MOV_IMM_REG_MEM;
                        }
                    }
                }
            }
        }
        else
        {
            // 3 msb
            if (bit_check(byte, 5))
            {
                // 4 msb
                if (bit_check(byte, 4))
                    return MOV_IMM_REG;
                else
                {
                    // 3 msb
                    if(bit_check(byte, 3))
                    {}
                    else
                    {
                        // 2 msb
                        if(bit_check(byte, 2))
                        {}
                        else
                        {
                            // 1 msb
                            if (bit_check(byte, 1))
                                return MOV_ACC_MEM;
                            else
                                return MOV_MEM_ACC;
                        }
                    }
                }
            }
            else
            {
                // 4 msb
                if (bit_check(byte, 4))
                {}
                else
                {
                    // 5 msb
                    if (bit_check(byte, 3))
                    {
                        // 6 msb
                        if (bit_check(byte, 2))
                        {}
                        else
                            return MOV_REG_MEM;
                    }
                    else
                    {
                        if (bit_check(byte, 2))
                        {}
                        else
                            return MATH_OP_IMM_REG_MEM;
                    }
                }
            }
        }
    }
    else
    {
        if (bit_check(byte, 6))
        {}
        else
        {
            if (bit_check(byte, 5))
            {
                if (bit_check(byte, 4))
                {
                    if(bit_check(byte, 3))
                    {
                        if (bit_check(byte, 2))
                        {
                            if (bit_check(byte, 1))
                            {}
                            else
                                return MATH_OP_IMM_ACC;
                        }
                        return MATH_OP_REG_MEM_EITHER;
                    }
                    else
                    {
                        // if (bit_check(byte, 3))
                        // {
                        //     if (bit_check(byte, 2))
                        //     {
                        //         if (bit_check(byte, 1))
                        //         {}
                        //         else
                        //             return MATH_OP_IMM_ACC;
                        //     }
                        //     else
                        //         return MATH_OP_REG_MEM_EITHER;
                        // }
                    }
                }
                else
                {
                    if (bit_check(byte, 3))
                    {
                        if (bit_check(byte, 2))
                        {
                            if (bit_check(byte, 1))
                            {}
                            else
                                return MATH_OP_IMM_ACC;

                        }
                        else
                            return MATH_OP_REG_MEM_EITHER;
                    }

                }
            }
            else
            {
                if (bit_check(byte, 4))
                {}
                else
                {
                    if(bit_check(byte, 3))
                    {}
                    else
                    {
                        if (bit_check(byte, 2))
                        {
                            if (bit_check(byte, 1))
                            {}
                            else
                                return MATH_OP_IMM_ACC;
                        }
                        else
                            return MATH_OP_REG_MEM_EITHER;
                    }
                }
            }

        }
    }
    assert(0 && "ERROR Instruction couldn't be parsed from byte, or hasn't yet been implemented\n");
}

// typedef struct Instruction_Node Instruction_Node;
// typedef struct Instruction_Node
// {
//     Instruction inst;
//     Instruction_Node* zero;
//     Instruction_Node* one;
// } Instruction_Node;
//
// Instruction_Node* create_inst_tree(Arena* arena)
// {
//
// }

static inline short high_low_combine_signed(uint8_t low, uint8_t high)
{
    if (high == 0 && low > 127)
        return -(256 -low);
    else
        return ((high << 8) | low);
}

static inline short high_low_combine_unsigned(uint8_t low, uint8_t high)
{
    return ((high << 8) | low);
}

char* instruction_string(Instruction inst)
{
    switch (inst)
    {
    case MOV_MEM_ACC:
    case MOV_ACC_MEM:
    case MOV_IMM_REG_MEM:
    case MOV_IMM_REG:
    case MOV_REG_MEM:
        return "mov";
    default:
        assert(0 && "ERROR - failed to get instruction_string\n");
    }
}

#define WORD_VAULE_ON (1<<7)
typedef enum
{
    REG_AL = 0b00000000,
    REG_CL = 0b00000001,
    REG_DL = 0b00000010,
    REG_BL = 0b00000011,
    REG_AH = 0b00000100,
    REG_CH = 0b00000101,
    REG_DH = 0b00000110,
    REG_BH = 0b00000111,
    REG_AX = 0b10000000,
    REG_CX = 0b10000001,
    REG_DX = 0b10000010,
    REG_BX = 0b10000011,
    REG_SP = 0b10000100,
    REG_BP = 0b10000101,
    REG_SI = 0b10000110,
    REG_DI = 0b10000111
} REG_Address;



char* register_address_string(REG_Address address)
{
    switch(address)
    {
    case REG_AX:
        return "ax";
    case REG_CX:
        return "cx";
    case REG_DX:
        return "dx";
    case REG_BX:
        return "bx";
    case REG_SP:
        return "sp";
    case REG_BP:
        return "bp";
    case REG_SI:
        return "si";
    case REG_DI:
        return "di";
    case REG_AL:
        return "al";
    case REG_CL:
        return "cl";
    case REG_DL:
        return "dl";
    case REG_BL:
        return "bl";
    case REG_AH:
        return "ah";
    case REG_CH:
        return "ch";
    case REG_DH:
        return "dh";
    case REG_BH:
        return "bh";
    default:
        assert(0 && "ERROR - Invalid register byte\n");
    }
}

char* effective_address_calculation(REG_Address rm_value)
{
    switch(rm_value)
    {
    case REG_AL:
    case REG_AX:
        return "bx + si";
    case REG_CL:
    case REG_CX:
        return "bx + di";
    case REG_DL:
    case REG_DX:
        return "bp + si";
    case REG_BL:
    case REG_BX:
        return "bp + di";
    case REG_AH:
    case REG_SP:
        return "si";
    case REG_CH:
    case REG_BP:
        return "di";
    case REG_DH:
    case REG_SI:
        return "bp";
    case REG_BH:
    case REG_DI:
        return "bx";
    default:
        assert(0 && "ERROR - Invalid register byte\n");
    }
}

typedef enum
{
    NO_DISPLACEMENT      = 0b00000000,
    _8_BIT_DISPLACEMENT  = 0b01000000,
    _16_BIT_DISPLACEMENT = 0b10000000,
    REGISTER_MODE        = 0b11000000,
} Inst_Mod;
#define INST_MOD_BITMASK 0b11000000

enum
{
    NEW_INSTRUCTION,
    SECOND_BYTE,
    THIRD_BYTE,
    FOURTH_BYTE,
    FIFTH_BYTE,
    SIXTH_BYTE
};

typedef enum
{
    MATH_ADD = 0b00000000,
    MATH_SUB = 0b00101000,
    MATH_CMP = 0b00111000
} Math_typ;



typedef enum
{
    DIRECTION_BIT_ON    = (1<<0),
    WORD_BIT_ON         = (1<<1),
    SIGN_EXTENDED       = (1<<2),
    INSTRUCTION_READY   = (1<<31)
} Inst_Flags;

typedef struct
{
    Instruction inst;
    Inst_Mod mod;
    REG_Address operand_1;
    REG_Address operand_2_rm;
    Math_typ math_typ;
    uint8_t disp_l;
    uint8_t disp_h;
    uint8_t data_l;
    uint8_t data_h;
    uint32_t inst_flags;
    uint8_t byte_count;
} Assembly_Inst;


#define SIGN_EXTENDED_BIT    (1<<1)
#define REG_BITMASK          0b00111000
#define RM_BITMASK           0b00000111
#define RM_NO_DISP_EXCEPTION 0b00000110


char* math_op_type_string(Assembly_Inst* inst)
{
    switch (inst->math_typ)
    {
    case MATH_ADD:
        return "add";
    case MATH_CMP:
        return "cmp";
    case MATH_SUB:
        return "sub";
    }
    assert(0);
}

void mod_reg_rm_byte(const uint8_t read_byte, Assembly_Inst* inst)
{
    inst->operand_1 = read_byte & REG_BITMASK;
    inst->operand_1 >>= 3;
    if (inst->inst_flags & WORD_BIT_ON)
        inst->operand_1 |= WORD_VAULE_ON;
    inst->operand_2_rm  = read_byte & RM_BITMASK;

    inst->mod = read_byte & INST_MOD_BITMASK;
}

#define MATH_OP_BITMASK 0b00111000
static inline Math_typ math_op_type(const uint8_t read_byte)
{
    return read_byte & MATH_OP_BITMASK;
}

/* ===================================================
   MATH OP - Register/Memory with either
   =================================================*/
void math_op_reg_mem_either(const uint8_t read_byte, Assembly_Inst* inst)
{
    switch (inst->byte_count)
    {
    case NEW_INSTRUCTION:
        inst->math_typ = math_op_type(read_byte);
        if (read_byte & WORD_BIT)
            inst->inst_flags |= WORD_BIT_ON;
        if (read_byte & DIRECTION_BIT)
            inst->inst_flags |= DIRECTION_BIT_ON;
        break;
    case SECOND_BYTE:
        mod_reg_rm_byte(read_byte, inst);
        switch (inst->mod)
        {
        case REGISTER_MODE:
            if (inst->inst_flags & WORD_BIT_ON)
                inst->operand_2_rm |= WORD_VAULE_ON;
            inst->inst_flags |= INSTRUCTION_READY;
            break;
        case NO_DISPLACEMENT:
            if (!(inst->operand_2_rm == RM_NO_DISP_EXCEPTION))
                inst->inst_flags |= INSTRUCTION_READY;
            if (inst->inst_flags & WORD_BIT_ON)
                inst->operand_2_rm |= WORD_VAULE_ON;
            break;
        case _8_BIT_DISPLACEMENT:
        case _16_BIT_DISPLACEMENT:
            break;
        }
        break;
    case THIRD_BYTE:
        inst->disp_l = read_byte;
        if (inst->mod == _8_BIT_DISPLACEMENT)
            inst->inst_flags |= INSTRUCTION_READY;
        break;
    case FOURTH_BYTE:
        inst->disp_h = read_byte;
        inst->inst_flags |= INSTRUCTION_READY;
        break;
    }
}
void print_math_op_reg_mem_either(Assembly_Inst* inst)
{
    switch (inst->mod)
    {
    case REGISTER_MODE:
        printf("%s %s, %s\n", math_op_type_string(inst),
               register_address_string((inst->inst_flags & DIRECTION_BIT_ON) ? inst->operand_1 : inst->operand_2_rm),
               register_address_string((inst->inst_flags & DIRECTION_BIT_ON) ? inst->operand_2_rm : inst->operand_1));
        break;
    case NO_DISPLACEMENT:
        if (inst->operand_2_rm == RM_NO_DISP_EXCEPTION || inst->operand_2_rm == (RM_NO_DISP_EXCEPTION | WORD_VAULE_ON))
        {
            printf("%s %s, [%hu]\n", math_op_type_string(inst),
                   register_address_string((inst->inst_flags & DIRECTION_BIT_ON) ? inst->operand_1 : inst->operand_2_rm), high_low_combine_unsigned(inst->disp_l, inst->disp_h));
        }
        else
        {
            if (inst->inst_flags & DIRECTION_BIT_ON)
            {
                printf("%s %s, [%s]\n", math_op_type_string(inst), register_address_string(inst->operand_1),
                       effective_address_calculation(inst->operand_2_rm));
            }
            else
            {
                printf("%s [%s], %s\n", math_op_type_string(inst),
                       effective_address_calculation(inst->operand_2_rm), register_address_string(inst->operand_1));
            }
        }
        break;
    case _8_BIT_DISPLACEMENT:
    case _16_BIT_DISPLACEMENT:
        if (inst->inst_flags & DIRECTION_BIT_ON)
        {
            printf("%s %s, [%s + %hd]\n", math_op_type_string(inst),
                   register_address_string((inst->inst_flags & DIRECTION_BIT_ON) ? inst->operand_1 : inst->operand_2_rm), effective_address_calculation((inst->inst_flags & DIRECTION_BIT_ON) ? inst->operand_2_rm : inst->operand_1), high_low_combine_signed(inst->disp_l, inst->disp_h));
        }
        else
        {
            printf("%s [%s + %hd], %s\n", math_op_type_string(inst),
                   effective_address_calculation(inst->operand_2_rm), high_low_combine_signed(inst->disp_l, inst->disp_h), register_address_string(inst->operand_1));
        }
        break;
    }
}

/* ===================================================
   MATH OP - Immediate to memory/register
   =================================================*/

void math_op_imm_reg_mem(const uint8_t read_byte, Assembly_Inst* inst)
{
    switch (inst->byte_count)
    {
    case NEW_INSTRUCTION:
        if (read_byte & WORD_BIT)
            inst->inst_flags |= WORD_BIT_ON;
        if (read_byte & SIGN_EXTENDED_BIT)
            inst->inst_flags |= SIGN_EXTENDED;
        break;
    case SECOND_BYTE:
        inst->math_typ = math_op_type(read_byte);
        mod_reg_rm_byte(read_byte, inst);
        if (inst->inst_flags & WORD_BIT_ON)
            inst->operand_2_rm |= WORD_VAULE_ON;
        break;
    case THIRD_BYTE:
        switch (inst->mod)
        {
        case REGISTER_MODE:
            inst->data_l = read_byte;
            if (inst->inst_flags & WORD_BIT_ON && !(inst->inst_flags & SIGN_EXTENDED));
            else
                inst->inst_flags |= INSTRUCTION_READY;
            break;
        case NO_DISPLACEMENT:
            if (inst->operand_2_rm == RM_NO_DISP_EXCEPTION || (inst->inst_flags & WORD_BIT_ON && inst->operand_2_rm == (RM_NO_DISP_EXCEPTION | WORD_VAULE_ON)))
                inst->disp_l = read_byte;
            else
            {
                inst->data_l = read_byte;
                if (inst->inst_flags & WORD_BIT_ON && !(inst->inst_flags & SIGN_EXTENDED));
                else
                    inst->inst_flags |= INSTRUCTION_READY;
            }
            break;
        case _8_BIT_DISPLACEMENT:
        case _16_BIT_DISPLACEMENT:
            inst->disp_l = read_byte;
            break;
        }
        break;
    case FOURTH_BYTE:
        switch (inst->mod)
        {
        case REGISTER_MODE:
            inst->data_h = read_byte;
            inst->inst_flags |= INSTRUCTION_READY;
            break;
        case NO_DISPLACEMENT:
            if (inst->operand_2_rm == RM_NO_DISP_EXCEPTION || (inst->inst_flags & WORD_BIT_ON && inst->operand_2_rm == (RM_NO_DISP_EXCEPTION | WORD_VAULE_ON)))
                inst->disp_h = read_byte;
            else
            {
                inst->data_h = read_byte;
                inst->inst_flags |= INSTRUCTION_READY;
            }
            break;
        case _8_BIT_DISPLACEMENT:
            inst->data_l = read_byte;
            if (inst->inst_flags & WORD_BIT_ON && !(inst->inst_flags & SIGN_EXTENDED));
            else
                inst->inst_flags |= INSTRUCTION_READY;
            break;
        case _16_BIT_DISPLACEMENT:
            inst->disp_h = read_byte;
            break;
        }
        break;
    case FIFTH_BYTE:
        switch (inst->mod)
        {
        case REGISTER_MODE:
            assert(0);
        case NO_DISPLACEMENT:
            inst->data_l = read_byte;
            if (inst->inst_flags & WORD_BIT_ON && !(inst->inst_flags & SIGN_EXTENDED));
            else
                inst->inst_flags |= INSTRUCTION_READY;
            break;
        case _8_BIT_DISPLACEMENT:
            inst->data_h = read_byte;
            inst->inst_flags |= INSTRUCTION_READY;
            break;
        case _16_BIT_DISPLACEMENT:
            inst->data_l = read_byte;
            if (inst->inst_flags & WORD_BIT_ON && !(inst->inst_flags & SIGN_EXTENDED));
            else
                inst->inst_flags |= INSTRUCTION_READY;
            break;
        }
        break;
    case SIXTH_BYTE:
        switch (inst->mod)
        {
        case REGISTER_MODE:
        case _8_BIT_DISPLACEMENT:
            assert(0);
        case NO_DISPLACEMENT:
        case _16_BIT_DISPLACEMENT:
            inst->data_h = read_byte;
            inst->inst_flags |= INSTRUCTION_READY;
            break;
        }
    }
}

void print_math_op_imm_reg_mem(Assembly_Inst* inst)
{
    switch (inst->mod)
    {
    case REGISTER_MODE:
        printf("%s %s, %hd\n", math_op_type_string(inst),  register_address_string(inst->operand_1), high_low_combine_signed(inst->data_l, inst->data_h));
        break;
    case NO_DISPLACEMENT:
        if (inst->operand_2_rm == RM_NO_DISP_EXCEPTION || (inst->inst_flags & WORD_BIT_ON && inst->operand_2_rm == (RM_NO_DISP_EXCEPTION | WORD_VAULE_ON)))
        {
            printf("%s %s [%hd], %hd\n", math_op_type_string(inst),inst->inst_flags & WORD_BIT_ON ? "word" : "byte", high_low_combine_signed(inst->disp_l, inst->disp_h), high_low_combine_signed(inst->data_l, inst->data_h));
        }
        else
        {
            printf("%s %s [%s], %hd\n", math_op_type_string(inst),inst->inst_flags & WORD_BIT_ON ? "word" : "byte",
                   effective_address_calculation(inst->operand_2_rm), high_low_combine_signed(inst->data_l, inst->data_h));
        }
        break;
    case _8_BIT_DISPLACEMENT:
    case _16_BIT_DISPLACEMENT:
        printf("%s %s [%s + %hd], %hd\n", math_op_type_string(inst),inst->inst_flags & WORD_BIT_ON ? "word" : "byte",
               effective_address_calculation(inst->operand_2_rm), high_low_combine_signed(inst->disp_l, inst->disp_h),  high_low_combine_signed(inst->data_l, inst->data_h));
        break;
    }
}

/* ===================================================
   MATH OP - Immediate to accumulator
   =================================================*/
void math_op_imm_acc_parse(const uint8_t read_byte, Assembly_Inst* inst)
{
    switch (inst->byte_count)
    {
    case NEW_INSTRUCTION:
        inst->math_typ = math_op_type(read_byte);
        if (read_byte & WORD_BIT)
            inst->inst_flags |= WORD_BIT_ON;
        break;
    case SECOND_BYTE:
        inst->data_l = read_byte;
        if (!(inst->inst_flags & WORD_BIT_ON))
            inst->inst_flags |= INSTRUCTION_READY;
        break;
    case THIRD_BYTE:
        inst->data_h = read_byte;
        inst->inst_flags |= INSTRUCTION_READY;
        break;
    }
}

/* ===================================================
   MOV - Accumulator/Memory to memory/accumulator
   =================================================*/
void mov_accumulator_memory_parse(const uint8_t read_byte, Assembly_Inst* inst)
{
    switch(inst->byte_count)
    {
    case NEW_INSTRUCTION:
        if (read_byte & WORD_BIT)
            inst->inst_flags |= WORD_BIT_ON;
        break;
    case SECOND_BYTE:
        inst->disp_l = read_byte;
        if (!(inst->inst_flags & WORD_BIT_ON))
            inst->inst_flags |= INSTRUCTION_READY;
        break;
    case THIRD_BYTE:
        inst->disp_h = read_byte;
        inst->inst_flags |= INSTRUCTION_READY;
        break;
    }
}


/* ===================================================
   MOV - Immediate to registar/memory
   =================================================*/
void mov_imm_reg_mem_parse(const uint8_t read_byte, Assembly_Inst* inst)
{
    switch(inst->byte_count)
    {
    case NEW_INSTRUCTION:
        if (read_byte & WORD_BIT)
            inst->inst_flags |= WORD_BIT_ON;
        break;
    case SECOND_BYTE:
        if (inst->inst_flags & WORD_BIT_ON)
            inst->operand_1 |= WORD_VAULE_ON;
        inst->operand_2_rm  = read_byte & RM_BITMASK;

        inst->mod = read_byte & INST_MOD_BITMASK;
        break;
    case THIRD_BYTE:
        switch (inst->mod)
        {
        case REGISTER_MODE:
        case NO_DISPLACEMENT:
            if (inst->operand_2_rm == RM_NO_DISP_EXCEPTION && inst->mod == NO_DISPLACEMENT)
            {
                inst->disp_l = read_byte;
            }
            else
            {
                inst->data_l = read_byte;
                if (!(inst->inst_flags & WORD_BIT_ON))
                    inst->inst_flags |= INSTRUCTION_READY;
            }
            break;
        case _8_BIT_DISPLACEMENT:
        case _16_BIT_DISPLACEMENT:
            inst->disp_l = read_byte;
            break;
        }
        break;
    case FOURTH_BYTE:
        switch (inst->mod)
        {
        case REGISTER_MODE:
        case NO_DISPLACEMENT:
            if (inst->operand_2_rm == RM_NO_DISP_EXCEPTION && inst->mod == NO_DISPLACEMENT)
            {
                inst->disp_h = read_byte;
            }
            else
            {
                inst->data_h = read_byte;
                inst->inst_flags |= INSTRUCTION_READY;
            }
            break;
        case _8_BIT_DISPLACEMENT:
            inst->data_l = read_byte;
            if (!(inst->inst_flags & WORD_BIT_ON))
                inst->inst_flags |= INSTRUCTION_READY;
            break;
        case _16_BIT_DISPLACEMENT:
            inst->disp_h = read_byte;
            break;
        }
        break;
    case FIFTH_BYTE:
        switch (inst->mod)
        {
        case REGISTER_MODE:
            assert(0);
        case NO_DISPLACEMENT:
        case _16_BIT_DISPLACEMENT:
            inst->data_l = read_byte;
            if (!(inst->inst_flags & WORD_BIT_ON))
                inst->inst_flags |= INSTRUCTION_READY;
            break;
        case _8_BIT_DISPLACEMENT:
            inst->data_h = read_byte;
            inst->inst_flags |= INSTRUCTION_READY;
            break;
        }
        break;
    case SIXTH_BYTE:
        switch (inst->mod)
        {
        case REGISTER_MODE:
        case _8_BIT_DISPLACEMENT:
            assert(0);
        case NO_DISPLACEMENT:
        case _16_BIT_DISPLACEMENT:
            inst->data_h = read_byte;
            inst->inst_flags |= INSTRUCTION_READY;
            break;
        }
        break;
    }
}

void print_mov_imm_reg_mem(Assembly_Inst* inst)
{
    switch (inst->mod)
    {
    case REGISTER_MODE:
        printf("%s %s, %s %hd\n", instruction_string(inst->inst),
               register_address_string(inst->operand_1), inst->data_h == 0 ? "byte" : "word", high_low_combine_signed(inst->data_l, inst->data_h));
        break;
    case NO_DISPLACEMENT:
        if (inst->operand_2_rm == RM_NO_DISP_EXCEPTION)
        {
            printf("%s [%hd], %s %hd\n", instruction_string(inst->inst),
                   high_low_combine_signed(inst->disp_l, inst->disp_h), inst->data_h == 0 ? "byte" : "word", high_low_combine_signed(inst->data_l, inst->data_h));
        }
        else
        {
            printf("%s [%s], %s %hd\n", instruction_string(inst->inst),
                   effective_address_calculation(inst->operand_2_rm),inst->data_h == 0 ? "byte" : "word", high_low_combine_signed(inst->data_l, inst->data_h));
        }
        break;
    case _8_BIT_DISPLACEMENT:
    case _16_BIT_DISPLACEMENT:
        printf("%s [%s + %hd], %s %hd\n", instruction_string(inst->inst),
               effective_address_calculation(inst->operand_2_rm), high_low_combine_signed(inst->disp_l, inst->disp_h), inst->data_h == 0 ? "byte" : "word", high_low_combine_signed(inst->data_l, inst->data_h));
        break;
    }
}

/* ===================================================
   MOV - Register/memory to/from registar
   =================================================*/

void mov_reg_mem_parse(const uint8_t read_byte, Assembly_Inst* inst)
{
    switch(inst->byte_count)
    {
    case NEW_INSTRUCTION:
        if (read_byte & DIRECTION_BIT)
            inst->inst_flags |= DIRECTION_BIT_ON;
        if (read_byte & WORD_BIT)
            inst->inst_flags |= WORD_BIT_ON;
        break;
    case SECOND_BYTE:
        inst->operand_1 = read_byte & REG_BITMASK;
        inst->operand_1 >>= 3;
        if (inst->inst_flags & WORD_BIT_ON)
            inst->operand_1 |= WORD_VAULE_ON;
        inst->operand_2_rm  = read_byte & RM_BITMASK;

        inst->mod = read_byte & INST_MOD_BITMASK;
        switch (inst->mod)
        {
        case REGISTER_MODE:
            if (inst->inst_flags & WORD_BIT_ON)
                inst->operand_2_rm |= WORD_VAULE_ON;
            inst->inst_flags |= INSTRUCTION_READY;
            break;
        case NO_DISPLACEMENT:
            if (!(inst->operand_2_rm == RM_NO_DISP_EXCEPTION))
                inst->inst_flags |= INSTRUCTION_READY;
            if (inst->inst_flags & WORD_BIT_ON)
                inst->operand_2_rm |= WORD_VAULE_ON;
            break;
        case _8_BIT_DISPLACEMENT:
        case _16_BIT_DISPLACEMENT:
            break;
        }
        break;
    case THIRD_BYTE:
        inst->disp_l = read_byte;
        if (inst->mod == _8_BIT_DISPLACEMENT)
            inst->inst_flags |= INSTRUCTION_READY;
        break;
    case FOURTH_BYTE:
        inst->disp_h = read_byte;
        inst->inst_flags |= INSTRUCTION_READY;
        break;
    }
    // printf("op1: ");
    // print_binary_8(inst.operand_1);
    // printf("op2: ");
    // print_binary_8(inst.operand_2_rm);
}


void print_mov_reg_mem(Assembly_Inst* inst)
{
    switch (inst->mod)
    {
    case REGISTER_MODE:
        printf("%s %s, %s\n", instruction_string(inst->inst),
               register_address_string((inst->inst_flags & DIRECTION_BIT_ON) ? inst->operand_1 : inst->operand_2_rm),
               register_address_string((inst->inst_flags & DIRECTION_BIT_ON) ? inst->operand_2_rm : inst->operand_1));
        break;
    case NO_DISPLACEMENT:
        if (inst->operand_2_rm == RM_NO_DISP_EXCEPTION || inst->operand_2_rm == (RM_NO_DISP_EXCEPTION | WORD_VAULE_ON))
        {
            printf("%s %s, [%hu]\n", instruction_string(inst->inst),
                   register_address_string((inst->inst_flags & DIRECTION_BIT_ON) ? inst->operand_1 : inst->operand_2_rm), high_low_combine_unsigned(inst->disp_l, inst->disp_h));
        }
        else
        {
            if (inst->inst_flags & DIRECTION_BIT_ON)
            {
                printf("%s %s, [%s]\n", instruction_string(inst->inst), register_address_string(inst->operand_1),
                       effective_address_calculation(inst->operand_2_rm));
            }
            else
            {
                printf("%s [%s], %s\n", instruction_string(inst->inst),
                       effective_address_calculation(inst->operand_2_rm), register_address_string(inst->operand_1));
            }
        }
        break;
    case _8_BIT_DISPLACEMENT:
    case _16_BIT_DISPLACEMENT:
        if (inst->inst_flags & DIRECTION_BIT_ON)
        {
            printf("%s %s, [%s + %hd]\n", instruction_string(inst->inst),
                   register_address_string((inst->inst_flags & DIRECTION_BIT_ON) ? inst->operand_1 : inst->operand_2_rm), effective_address_calculation((inst->inst_flags & DIRECTION_BIT_ON) ? inst->operand_2_rm : inst->operand_1), high_low_combine_signed(inst->disp_l, inst->disp_h));
        }
        else
        {
            printf("%s [%s + %hd], %s\n", instruction_string(inst->inst),
                   effective_address_calculation(inst->operand_2_rm), high_low_combine_signed(inst->disp_l, inst->disp_h), register_address_string(inst->operand_1));
        }
        break;
    }
}



/* ===================================================
   MOV - Immediate to registar instruction
   =================================================*/
#define IMM_REG_BITMASK 0b00000111
void mov_imm_reg_parse(const uint8_t read_byte, Assembly_Inst* inst)
{
    switch (inst->byte_count)
    {
    case NEW_INSTRUCTION:
        inst->operand_1 = (read_byte & IMM_REG_BITMASK);
        if (read_byte & (1<<3))
        {
            inst->inst_flags |= WORD_BIT_ON;
            inst->operand_1 |= WORD_VAULE_ON;
        }
        break;
    case SECOND_BYTE:
        inst->data_l = read_byte;
        if (!(inst->inst_flags & WORD_BIT_ON))
            inst->inst_flags |= INSTRUCTION_READY;
        break;
    case THIRD_BYTE:
        inst->data_h = read_byte;
        inst->inst_flags |= INSTRUCTION_READY;
        break;
    }

}
/* ===================================================
   Conditional Return from CALL
   =================================================*/
void conditional_ret_inst_parse(const uint8_t read_byte, Assembly_Inst* inst)
{
    switch(inst->byte_count)
    {
    case NEW_INSTRUCTION:
        inst->disp_l = read_byte;
        break;
    case SECOND_BYTE:
        inst->disp_h = read_byte;
        inst->inst_flags |= INSTRUCTION_READY;
        break;
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Args = file path\n");
        return 0;
    }

    FILE* file_ptr = fopen(argv[1], "rb");

    uint8_t read_byte;
    Assembly_Inst inst = {0};

    printf("; output from encoding %s to assembly\n\nbits 16\n\n", argv[1]);
    while(fread(&read_byte, 1, 1, file_ptr) > 0)
    {
        //print_binary_8(read_byte);
        if(inst.byte_count == NEW_INSTRUCTION)
        {
            inst.inst = instruction_from_byte(read_byte);
            switch (inst.inst)
            {
            case MOV_IMM_REG:
                mov_imm_reg_parse(read_byte, &inst);
                break;
            case MOV_REG_MEM:
                mov_reg_mem_parse(read_byte, &inst);
                break;
            case MOV_IMM_REG_MEM:
                mov_imm_reg_mem_parse(read_byte, &inst);
                break;
            case MOV_ACC_MEM:
            case MOV_MEM_ACC:
                mov_accumulator_memory_parse(read_byte, &inst);
                break;
            case MATH_OP_IMM_ACC:
                math_op_imm_acc_parse(read_byte, &inst);
                break;
            case MATH_OP_REG_MEM_EITHER:
                math_op_reg_mem_either(read_byte, &inst);
                break;
            case MATH_OP_IMM_REG_MEM:
                math_op_imm_reg_mem(read_byte, &inst);
                break;
            case CONDITIONAL_RET_INST:
                conditional_ret_inst_parse(read_byte, & inst);
                break;
            default:
                assert(0 && "ERROR whilst parsing byte\n");
            }
            ++inst.byte_count;
        }
        else
        {
            switch (inst.inst)
            {
            case MOV_REG_MEM:
                mov_reg_mem_parse(read_byte, &inst);
                break;
            case MOV_IMM_REG:
                mov_imm_reg_parse(read_byte, &inst);
                break;
            case MOV_IMM_REG_MEM:
                mov_imm_reg_mem_parse(read_byte, &inst);
                break;
            case MOV_ACC_MEM:
            case MOV_MEM_ACC:
                mov_accumulator_memory_parse(read_byte, &inst);
                break;
            case MATH_OP_IMM_ACC:
                math_op_imm_acc_parse(read_byte, &inst);
                break;
            case MATH_OP_REG_MEM_EITHER:
                math_op_reg_mem_either(read_byte, &inst);
                break;
            case MATH_OP_IMM_REG_MEM:
                math_op_imm_reg_mem(read_byte, &inst);
                break;
            case CONDITIONAL_RET_INST:
                conditional_ret_inst_parse(read_byte, & inst);
                break;
            default:
                assert(0 && "ERROR whilst parsing byte\n");
            }
            ++inst.byte_count;
        }

        if (inst.inst_flags & INSTRUCTION_READY)
        {
            switch (inst.inst)
            {
            case MOV_REG_MEM:
                print_mov_reg_mem(&inst);
                break;
            case MOV_IMM_REG_MEM:
                print_mov_imm_reg_mem(&inst);
                break;
            case MOV_IMM_REG:
                printf("%s %s, %hu\n", instruction_string(inst.inst),
                       register_address_string(inst.operand_1),
                       high_low_combine_unsigned(inst.data_l, inst.data_h));
                break;
            case MOV_ACC_MEM:
                printf("%s [%hu], ax\n", instruction_string(inst.inst),
                       high_low_combine_unsigned(inst.disp_l, inst.disp_h));
                break;
            case MOV_MEM_ACC:
                printf("%s ax, [%hu]\n", instruction_string(inst.inst),
                       high_low_combine_unsigned(inst.disp_l, inst.disp_h));
                break;
            case MATH_OP_IMM_ACC:
                printf("%s ax, %u\n", math_op_type_string(&inst),
                       high_low_combine_unsigned(inst.data_l, inst.data_h));
                break;
            case MATH_OP_REG_MEM_EITHER:
                print_math_op_reg_mem_either(&inst);
                break;
            case MATH_OP_IMM_REG_MEM:
                print_math_op_imm_reg_mem(&inst);
                break;
            case CONDITIONAL_RET_INST:
                printf("%s %hd\n", conditional_ret_string(inst.disp_l), inst.disp_h > 127 ? (short)inst.disp_h - 256 : inst.disp_h);
                break;
            default:
                assert(0 && "ERROR whilst parsing byte\n");
            }
            memset(&inst, 0, sizeof(Assembly_Inst));
        }
    }

    return 0;
}

