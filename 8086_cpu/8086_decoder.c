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
    MOV_REG_MEM = 0b10001000,
    MOV_IMM_REG = 0b10110000,
    MOV_IMM_REG_MEM = 0b1100011
} Instruction;
#define INSTRUCTION_BITMASK 0b11111100



static inline uint8_t bit_check(const uint8_t byte, const uint8_t bit_check)
{
    return byte & (1<<bit_check) ? 1 : 0;
}

Instruction instruction_from_byte(uint8_t byte)
{
    //print_binary_8(byte);
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
                }
            }
        }
    }
    else
    {}
    assert(0 && "ERROR Instruction couldn't be parsed from byte, or hasn't yet been implemented\n");
}

char* instruction_string(Instruction inst)
{
    switch (inst)
    {
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

typedef enum
{
    NO_DISPLACEMENT      = 0b00000000,
    _8_BIT_DISPLACEMENT  = 0b01000000,
    _16_BIT_DISPLACEMENT = 0b10000000,
    REGISTER_MODE        = 0b11000000,
} Inst_Mod;
#define INST_MOD_BITMASK 0b11000000

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
enum
{
    NEW_INSTRUCTION,
    SECOND_BYTE,
    THIRD_BYTE
};

typedef enum
{
    DIRECTION_BIT_ON    = (1<<0),
    WORD_BIT_ON         = (1<<1),
    INSTRUCTION_READY   = (1<<31)
} Inst_Flags;

typedef struct
{
    Instruction inst;
    Inst_Mod mod;
    REG_Address operand_1;
    REG_Address operand_2;
    uint8_t data_l;
    uint8_t data_h;
    uint32_t inst_flags;
    uint8_t byte_count;
} Assembly_Inst;

#define REG_BITMASK 0b00111000
#define DM_BITMASK  0b00000111
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
        inst->mod = read_byte & INST_MOD_BITMASK;
        switch (inst->mod)
        {
        case REGISTER_MODE:
        {
            inst->operand_1 = read_byte & REG_BITMASK;
            inst->operand_1 >>= 3;
            if (inst->inst_flags & WORD_BIT_ON)
                inst->operand_1 |= WORD_VAULE_ON;
            // printf("op1: ");
            // print_binary_8(inst.operand_1);
            inst->operand_2  = read_byte & DM_BITMASK;
            if (inst->inst_flags & WORD_BIT_ON)
                inst->operand_2 |= WORD_VAULE_ON;
            // printf("op2: ");
            // print_binary_8(inst.operand_2);
            inst->inst_flags |= INSTRUCTION_READY;
            break;
        }
        case NO_DISPLACEMENT:
        case _8_BIT_DISPLACEMENT:
        case _16_BIT_DISPLACEMENT:
            assert(0 && "ERROR - mod not yet implemented\n");
        }
        break;
    }
}

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
        print_binary_8(read_byte);
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
                printf("%s %s, %s\n", instruction_string(inst.inst),
                       register_address_string((inst.inst_flags & DIRECTION_BIT_ON) ? inst.operand_1 : inst.operand_2),
                       register_address_string((inst.inst_flags & DIRECTION_BIT_ON) ? inst.operand_2 : inst.operand_1));
                break;
            case MOV_IMM_REG:
                printf("%s %s, %u\n", instruction_string(inst.inst),
                       register_address_string(inst.operand_1),
                       inst.inst_flags & WORD_BIT_ON ? inst.data_h + inst.data_l : inst.data_l);
                break;
            default:
                assert(0 && "ERROR whilst parsing byte\n");
            }
            memset(&inst, 0, sizeof(Assembly_Inst));
        }
    }

    return 0;
}

