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
    MOV = 0b10001000
} Instruction;
#define INSTRUCTION_BITMASK 0b11111100

char* instruction_string(Instruction inst)
{
    switch (inst)
    {
    case MOV:
        return "MOV";
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
        return "AX";
    case REG_CX:
        return "CX";
    case REG_DX:
        return "DX";
    case REG_BX:
        return "BX";
    case REG_SP:
        return "SP";
    case REG_BP:
        return "BP";
    case REG_SI:
        return "SI";
    case REG_DI:
        return "DI";
    case REG_AL:
        return "AL";
    case REG_CL:
        return "CL";
    case REG_DL:
        return "DL";
    case REG_BL:
        return "BL";
    case REG_AH:
        return "AH";
    case REG_CH:
        return "CH";
    case REG_DH:
        return "DH";
    case REG_BH:
        return "BH";
    default:
        assert(0 && "ERROR - Invalid register byte\n");
    }
}
#define REG_BITMASK 0b00111000
#define DM_BITMASK  0b00000111

typedef struct
{
    Instruction inst;
    Inst_Mod mod;
    REG_Address operate_1;
    REG_Address operate_2;
} Assembly_Inst;

#define NEW_INSTRUCTION      0
#define DIRECTION_BIT_ON    (1<<0)
#define WORD_BIT_ON         (1<<1)
#define SECOND_BYTE         (1<<31)
#define INSTRUCTION_READY   (1<<30)
int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Args = file path\n");
        return 0;
    }
    FILE* file_ptr = fopen(argv[1], "rb");

    uint8_t read_byte;
    uint32_t read_flags = 0;
    Assembly_Inst inst = {0};

    printf("; output from encoding %s to assembly\n\nbits 16\n\n", argv[1]);
    while(fread(&read_byte, 1, 1, file_ptr) > 0)
    {
        //print_binary_8(read_byte);
        if(read_flags == NEW_INSTRUCTION)
        {
            inst.inst = read_byte & INSTRUCTION_BITMASK;
            if (read_byte & DIRECTION_BIT)
                read_flags |= DIRECTION_BIT_ON;
            if (read_byte & WORD_BIT)
                read_flags |= WORD_BIT_ON;
            read_flags |= SECOND_BYTE;
        }
        else if (read_flags & SECOND_BYTE)
        {
            inst.mod = read_byte & INST_MOD_BITMASK;

            switch (inst.mod)
            {
            case REGISTER_MODE:
            {
                inst.operate_1 = read_byte & REG_BITMASK;
                inst.operate_1 >>= 3;
                if (read_flags & WORD_BIT_ON)
                    inst.operate_1 |= WORD_VAULE_ON;
                // printf("op1: ");
                // print_binary_8(inst.operate_1);
                inst.operate_2  = read_byte & DM_BITMASK;
                if (read_flags & WORD_BIT_ON)
                    inst.operate_2 |= WORD_VAULE_ON;
                // printf("op2: ");
                // print_binary_8(inst.operate_2);
                read_flags |= INSTRUCTION_READY;
                break;
            }
            case NO_DISPLACEMENT:
            case _8_BIT_DISPLACEMENT:
            case _16_BIT_DISPLACEMENT:
                assert(0 && "ERROR - mod not yet implemented\n");
            }
        }
        else
            assert(0 && "ERROR - incorrectly parse instruction");

        if (read_flags & INSTRUCTION_READY)
        {
            printf("%s %s, %s\n", instruction_string(inst.inst),
                   register_address_string((read_flags & DIRECTION_BIT_ON) ? inst.operate_1 : inst.operate_2),
                   register_address_string((read_flags & DIRECTION_BIT_ON) ? inst.operate_2 : inst.operate_1));
            memset(&inst, 0, sizeof(Assembly_Inst));
            read_flags = NEW_INSTRUCTION;
        }
    }

    return 0;
}

