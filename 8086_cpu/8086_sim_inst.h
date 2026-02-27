#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#define PAP_HELPER_IMPLEMENTATION
#include "../pap_helper.h"

#define array_count(Array) (sizeof(Array) / sizeof((Array)[0]))
#ifdef DEBUG
#undef DEBUG
#define DEBUG(debug_print) fprintf(stderr, "[DEBUG] %s:%d:%s\n", __FILE__, __LINE__, __func__);\
        (debug_print);
#else
#define DEBUG(debug_print)
#endif

typedef enum
{
#define INSTRUCTION(Mnemonic, ...) Op_##Mnemonic,
#define INST(...)
#include "8086_inst_list.inc"
    Op_Count,
} Operation_Type;

char* instruction_string(Operation_Type type)
{
    switch (type)
    {
#define INSTRUCTION(Mnemonic, ...) case Op_##Mnemonic: return #Mnemonic;
#define INST(...)
#include "8086_inst_list.inc"
    default:
        assert(0 && "ERROR - failed to get instruction_string\n");
    }
}

typedef enum : uint8_t
{
    Not_Used,

    Bits_OP,
    Bits_Literal,

    Bits_D,
    Bits_W,
    Bits_S,

    Bits_MOD,
    Bits_REG,
    Bits_RM,
    Bits_SR,

    Bits_Disp_L,
    Bits_Disp_H,

    Bits_Data_L,
    Bits_Data_H,

    Bits_IP_INC8,

    BITS_TYPE_COUNT
} Bits_Usage;

char* bits_usage_string(Bits_Usage usage)
{
    switch (usage)
    {
    case Not_Used:
        return "Not_Used";
    case Bits_OP:
        return "OP";
    case Bits_Literal:
        return "Literal";
    case Bits_D:
        return "D";
    case Bits_W:
        return "W";
    case Bits_S:
        return "S";
    case Bits_MOD:
        return "MOD";
    case Bits_REG:
        return "REG";
    case Bits_RM:
        return "RM";
    case Bits_SR:
        return "SR";
    case Bits_Disp_L:
        return "Disp_L";
    case Bits_Disp_H:
        return "Disp_H";
    case Bits_Data_L:
        return "Data_L";
    case Bits_Data_H:
        return "Data_H";
    case Bits_IP_INC8:
        return "IP-INC8";
    default:
        assert(0 && "ERROR - failed to get bits_usage_string\n");
    }
}

typedef struct
{
    Bits_Usage usage;
    uint8_t   count;
    uint8_t   offset;
    uint8_t   value;
} Bits_Field;

typedef enum
{
    DIRECTION_BIT_ON    = (1<<0),
    WORD_BIT_ON         = (1<<1),
    SIGN_EXTENDED       = (1<<2),
    INSTRUCTION_READY   = (1<<31)
} Inst_Flags;

#define MAX_BITS_FIELD 10
typedef struct
{
    Operation_Type type;
    Bits_Field field[MAX_BITS_FIELD];
} Instruction_Code;

void debug_print_Assembly_Inst(Instruction_Code* inst)
{
    printf("Instruction: %s\n", instruction_string(inst->type));
    for (int i = 0; i < MAX_BITS_FIELD; i++)
    {
        printf("  Field %d: usage=%s, count=%d, offset=%d, value=0b", i, bits_usage_string(inst->field[i].usage), inst->field[i].count, inst->field[i].offset);
        print_binary_8(inst->field[i].value);
        printf("\n");
    }
}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
const Instruction_Code instruction_table[] =
{
#include "8086_inst_list.inc"
};
#pragma GCC diagnostic pop

void print_inst_table()
{
    for (uint32_t i = 0; i < array_count(instruction_table); ++i)
        debug_print_Assembly_Inst(&instruction_table[i]);
}

#define MEMORY_SIZE 1024*1024
typedef struct
{
    uint8_t* data;
    uint32_t bytes_used;
} Memory;

void read_file(Memory* memory, const char* file_path)
{
    memory->data = (uint8_t*)malloc(sizeof(uint8_t) * MEMORY_SIZE);
    memory->bytes_used = 0;

    FILE* file_ptr = fopen(file_path, "rb");
    uint8_t byte;

    while(fread(&byte, 1, 1, file_ptr) > 0)
        memory->data[memory->bytes_used++] = byte;

    fclose(file_ptr);
}

void free_memory(Memory* memory)
{
    free(memory->data);
    memory->data       = NULL;
    memory->bytes_used = 0;
}

int ffetch(Instruction_Code* inst, Bits_Usage field)
{
    for (uint8_t i = 0; i < array_count(inst->field); ++i)
        if (inst->field[i].usage == field)
            return inst->field[i].value;
    return -1;
}

const char* byte_registers[] =
{
    "al", // 0b000
    "cl", // 0b001
    "dl", // 0b010
    "bl", // 0b011
    "ah", // 0b100
    "ch", // 0b101
    "dh", // 0b110
    "bh"  // 0b111
};

const char* word_registers[] =
{
    "ax", // 0b000
    "cx", // 0b001
    "dx", // 0b010
    "bx", // 0b011
    "sp", // 0b100
    "bp", // 0b101
    "si", // 0b110
    "di"  // 0b111
};

const char* segment_registers[] =
{
    "es", // 0b00
    "cs", // 0b01
    "ss", // 0b10
    "ds"  // 0b11
};

const char* effective_addresses[] =
{
    "bx + si", // 0b000
    "bx + di", // 0b001
    "bp + si", // 0b010
    "bp + di", // 0b011
    "si",      // 0b100
    "di",      // 0b101
    "bp",      // 0b110  (when MOD=00 this is direct address, not BP)
    "bx"       // 0b111
};

typedef enum : uint8_t
{
    NO_DISPLACEMENT      = 0b00000000,
    _8_BIT_DISPLACEMENT  = 0b00000001,
    _16_BIT_DISPLACEMENT = 0b00000010,
    REGISTER_MODE        = 0b00000011
} Mod_Field;

#define MAX_SIZE_OF_OPPERANT 32
typedef struct
{
    Operation_Type mnemonic;
    char opperant1[MAX_SIZE_OF_OPPERANT];
    char opperant2[MAX_SIZE_OF_OPPERANT];
    uint32_t flags;
} Assembly_Inst;

enum Construction_Flags
{
    HAS_DIRECT_ADDRESS  = (1<<0),
    HAS_DISPLACEMENT    = (1<<1),
    DISPLACEMENT_IS_W   = (1<<2),
    DATA_IS_W           = (1<<3)
};

const char* get_register(uint8_t reg, uint8_t w)
{
    if (w)
        return word_registers[reg];
    else
        return byte_registers[reg];
}

const char* get_effective_address(uint8_t rm, uint8_t mod, int32_t disp, char* buf)
{
    // MOD=00, RM=110 is direct address special case
    if (mod == NO_DISPLACEMENT && rm == 0b110)
    {
        sprintf(buf, "[%d]", disp);
        return buf;
    }

    const char* ea = effective_addresses[rm];

    if (mod == NO_DISPLACEMENT)
        sprintf(buf, "[%s]", ea);
    else
        sprintf(buf, "[%s + %d]", ea, disp);

    return buf;
}

static inline uint16_t byte_calc(int32_t byte_l, int32_t byte_h)
{
    return (uint16_t)(byte_l | ((byte_h != -1 ? byte_h : 0) << 8));
}

static inline short disp_calc(int32_t disp_l, int32_t disp_h)
{
    if (disp_h == -1)
        return (short)(disp_l | (((int8_t)disp_l < 0 ? 0xFF : 0) << 8));
    else
        return (short)(disp_l | (disp_h  << 8));
}
void mov_construct(Assembly_Inst* assy, Instruction_Code* inst)
{
    int32_t d      = ffetch(inst, Bits_D);
    int32_t w      = ffetch(inst, Bits_W);
    int32_t mod    = ffetch(inst, Bits_MOD);
    int32_t reg    = ffetch(inst, Bits_REG);
    int32_t rm     = ffetch(inst, Bits_RM);
    int32_t sr     = ffetch(inst, Bits_SR);
    int32_t data_l = ffetch(inst, Bits_Data_L);
    int32_t data_h = ffetch(inst, Bits_Data_H);
    int32_t disp_l = ffetch(inst, Bits_Disp_L);
    int32_t disp_h = ffetch(inst, Bits_Disp_H);

    // segment register
    if (sr != -1)
    {
        uint8_t op_val     = (uint8_t)ffetch(inst, Bits_OP);
        const char* seg    = segment_registers[sr];
        char effective_address[MAX_SIZE_OF_OPPERANT];

        if (mod == REGISTER_MODE)
        {
            snprintf(effective_address, MAX_SIZE_OF_OPPERANT, "%s", word_registers[rm]);
        }
        else
        {
            // direct address
            if (mod == NO_DISPLACEMENT && rm == 0b110)
            {
                snprintf(effective_address, MAX_SIZE_OF_OPPERANT, "[%hu]", byte_calc(disp_l, disp_h));
            }
            else if (mod == NO_DISPLACEMENT)
            {
                snprintf(effective_address, MAX_SIZE_OF_OPPERANT, "[%s]",
                         effective_addresses[rm]);
            }
            else
            {
                snprintf(effective_address, MAX_SIZE_OF_OPPERANT, "[%s + %hd]",
                         effective_addresses[rm], disp_calc(disp_l, disp_h));
            }
        }
        if (op_val == 0b10001100) // mov r/m, sr
        {
            snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", effective_address);
            snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "%s", seg);
        }
        else // mov sr, r/m
        {
            snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", seg);
            snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "%s", effective_address);
        }
        return;
    }

    // immediate to register
    if (data_l != -1 && mod == -1)
    {
        const char* reg_name = w ? word_registers[reg] : byte_registers[reg];
        snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", reg_name);
        snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "%s %hu", w ? "word" : "byte", byte_calc(data_l, data_h));
        return;
    }

    // memory to/from accumulator
    if (disp_l != -1 && mod == -1)
    {
        uint16_t addr    = byte_calc(disp_l, disp_h);
        const char* acc  = (w > 0) ? "ax" : "al";
        int32_t op_val   = ffetch(inst, Bits_OP);

        if (op_val == 0b1010000) // mov acc, [addr]
        {
            snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", acc);
            snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "[%hu]", addr);
        }
        else // mov [addr], acc
        {
            snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "[%hu]", addr);
            snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "%s", acc);
        }
        return;
    }

    // register/memory encoding
    if (mod != -1 && reg != -1 && rm != -1)
    {
        char reg_name[MAX_SIZE_OF_OPPERANT];
        char effective_address[MAX_SIZE_OF_OPPERANT];

        snprintf(reg_name, MAX_SIZE_OF_OPPERANT, "%s",
                 (w > 0) ? word_registers[reg] : byte_registers[reg]);

        if (mod == REGISTER_MODE)
        {
            snprintf(effective_address, MAX_SIZE_OF_OPPERANT, "%s",
                     (w > 0) ? word_registers[rm] : byte_registers[rm]);
        }
        else
        {
            // direct address
            if (mod == NO_DISPLACEMENT && rm == 0b110)
            {
                snprintf(effective_address, MAX_SIZE_OF_OPPERANT, "[%hu]", byte_calc(disp_l, disp_h));
            }
            else if (mod == NO_DISPLACEMENT)
            {
                snprintf(effective_address, MAX_SIZE_OF_OPPERANT, "[%s]",
                         effective_addresses[rm]);
            }
            else
            {
                snprintf(effective_address, MAX_SIZE_OF_OPPERANT, "[%s + %hd]",
                         effective_addresses[rm], disp_calc(disp_l, disp_h));
            }
        }
        if (d > 0)
        {
            snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", reg_name);
            snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "%s", effective_address);
        }
        else
        {
            snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", effective_address);
            snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "%s", reg_name);
        }
        return;
    }
    assert(0 && "ERROR - when constructing mov\n");
}

void arithmetic_construct(Assembly_Inst* assy, Instruction_Code* inst)
{
    int32_t d      = ffetch(inst, Bits_D);
    int32_t w      = ffetch(inst, Bits_W);
    int32_t mod    = ffetch(inst, Bits_MOD);
    int32_t reg    = ffetch(inst, Bits_REG);
    int32_t rm     = ffetch(inst, Bits_RM);
    int32_t s      = ffetch(inst, Bits_S);
    int32_t data_l = ffetch(inst, Bits_Data_L);
    int32_t data_h = ffetch(inst, Bits_Data_H);
    int32_t disp_l = ffetch(inst, Bits_Disp_L);
    int32_t disp_h = ffetch(inst, Bits_Disp_H);

    // immediate to accumulator
    if (mod == -1 && reg == -1 && rm == -1)
    {
        const char* acc = (w) ? "ax" : "al";
        snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", acc);
        snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "%hu", byte_calc(data_l, data_h));
        return;
    }


    // Reg/Mem with register to either
    if (d != -1)
    {
        char reg_name[MAX_SIZE_OF_OPPERANT];
        char effective_address[MAX_SIZE_OF_OPPERANT];

        snprintf(reg_name, MAX_SIZE_OF_OPPERANT, "%s",
                 (w > 0) ? word_registers[reg] : byte_registers[reg]);

        switch (mod)
        {
        case REGISTER_MODE:
            snprintf(effective_address, MAX_SIZE_OF_OPPERANT, "%s", (w > 0) ? word_registers[rm] : byte_registers[rm]);
            break;
        case NO_DISPLACEMENT:
            if (rm == 0b110)
                snprintf(effective_address, MAX_SIZE_OF_OPPERANT, "[%hu]", byte_calc(disp_l, disp_h));
            else
                snprintf(effective_address, MAX_SIZE_OF_OPPERANT, "[%s]",
                         effective_addresses[rm]);
            break;
        case _8_BIT_DISPLACEMENT:
        case _16_BIT_DISPLACEMENT:
            snprintf(effective_address, MAX_SIZE_OF_OPPERANT, "[%s + %hd]", effective_addresses[rm], disp_calc(disp_l, disp_h));
            break;
        default:
            assert(0);
        }

        if (d > 0)
        {
            snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", reg_name);
            snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "%s", effective_address);
        }
        else
        {
            snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", effective_address);
            snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "%s", reg_name);
        }


        return;
    }

    // immediate to register/memory
    if (ffetch(inst, Bits_Literal) != -1)
    {
        char addr_loc[MAX_SIZE_OF_OPPERANT];

        switch (mod)
        {
        case REGISTER_MODE:
            snprintf(addr_loc, MAX_SIZE_OF_OPPERANT, "%s", (w > 0) ? word_registers[rm] : byte_registers[rm]);
            break;
        case NO_DISPLACEMENT:
            if (rm == 0b110)
                snprintf(addr_loc, MAX_SIZE_OF_OPPERANT, "[%hu]", byte_calc(disp_l, disp_h));
            else
                snprintf(addr_loc, MAX_SIZE_OF_OPPERANT, "[%s]", effective_addresses[rm]);
            break;
        case _8_BIT_DISPLACEMENT:
        case _16_BIT_DISPLACEMENT:
            snprintf(addr_loc, MAX_SIZE_OF_OPPERANT, "[%s + %hd]", effective_addresses[rm], disp_calc(disp_l, disp_h));
            break;
        default:
            assert(0);
        }

        snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", addr_loc);
        // 8-bit immediate, sign extended to 16bits
        if (w && s)
            snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "%s %hd", (w > 0) ? "word" : "byte", disp_calc(data_l, data_h));
        else
            snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "%s %hu", (w > 0) ? "word" : "byte", byte_calc(data_l, data_h));

        // changing the op_code to the proper mnemonic
        inst->type = (Operation_Type)(inst->type + ffetch(inst, Bits_Literal));

        return;

    }
    assert(0 && "ERROR - when constructing arithmetic op\n");
}

void jump_construct(Assembly_Inst* assy, Instruction_Code* inst)
{
    snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%hhd", (int8_t)ffetch(inst, Bits_IP_INC8));
}

void mod_rm_effective_address(Assembly_Inst* assy, Instruction_Code* inst)
{
    int32_t w      = ffetch(inst, Bits_W);
    int32_t mod    = ffetch(inst, Bits_MOD);
    int32_t rm     = ffetch(inst, Bits_RM);
    int32_t disp_l = ffetch(inst, Bits_Disp_L);
    int32_t disp_h = ffetch(inst, Bits_Disp_H);

    switch (mod)
    {
    case REGISTER_MODE:
        snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", (w > 0) ? word_registers[rm] : byte_registers[rm]);
        break;
    case NO_DISPLACEMENT:
        if (rm == 0b110)
            snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "[%hu]", byte_calc(disp_l, disp_h));
        else
            snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "[%s]", effective_addresses[rm]);
        break;
    case _8_BIT_DISPLACEMENT:
    case _16_BIT_DISPLACEMENT:
        snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "[%s + %hd]", effective_addresses[rm], disp_calc(disp_l, disp_h));
        break;
    }
}

static inline void swap_opperants(Assembly_Inst* assy)
{
    char temp[MAX_SIZE_OF_OPPERANT];
    snprintf(temp, MAX_SIZE_OF_OPPERANT, "%s", assy->opperant1);
    snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", assy->opperant2);
    snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "%s", temp);
}

void inc_dec_neg_construct(Assembly_Inst* assy, Instruction_Code* inst)
{
    int32_t reg = ffetch(inst, Bits_REG);

    if (reg == -1)
    {
        mod_rm_effective_address(assy, inst);
        // changing the op_code to the proper mnemonic
        if (inst->type != Op_neg)
            inst->type = (Operation_Type)(inst->type + ffetch(inst, Bits_Literal));
    }
    else
        snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", word_registers[reg]);
}

void print_assembly_inst(Assembly_Inst* assy)
{
    if (strlen(assy->opperant2) == 0)
        printf("%s %s\n", instruction_string(assy->mnemonic), assy->opperant1);
    else
        printf("%s %s, %s\n", instruction_string(assy->mnemonic), assy->opperant1, assy->opperant2);
}

void construct_assembly_inst(Instruction_Code* inst)
{
    Assembly_Inst assy;
    memset(&assy, 0, sizeof(Assembly_Inst));

    // conditional jump instructions
    if (inst->type >= Op_je && inst->type <= Op_jcxz)
        jump_construct(&assy, inst);
    else if (inst->field[1].usage == Not_Used)
        ; // fast path for only complete 1 byte ops
    else
        switch (inst->type)
        {
        case Op_mov:
            mov_construct(&assy, inst);
            break;
        case Op_test:
        case Op_add:
        case Op_adc:
        case Op_sub:
        case Op_sbb:
        case Op_cmp:
        case Op_and:
        case Op_or:
        case Op_xor:
            arithmetic_construct(&assy, inst);
            break;
        case Op_neg:
        case Op_dec:
        case Op_inc:
            inc_dec_neg_construct(&assy, inst);
            break;
        default:
            assert(0);
        }

    assy.mnemonic = inst->type;

    print_assembly_inst(&assy);
}



typedef enum
{
    CHECK_FOR_DIRECT_ADDRESS = (1<<0),
    WIDE
} Instruction_Flags;
#define DIRECT_ADDRESS 0b00000110

void unset_bits_field(Instruction_Code* inst, Bits_Usage field)
{
    for(uint8_t i = 0; i < array_count(inst->field); ++i)
    {
        if (inst->field[i].usage == field)
        {
            inst->field[i].usage = Not_Used;
            while (inst->field[i+1].usage != Not_Used)
            {
                if (i +1 == array_count(inst->field))
                    return;
                inst->field[i]         = inst->field[i +1];
                inst->field[++i].usage = Not_Used;
            }
            return;
        }
    }
    printf("WARNING - field not found to unset\n");
}

int decode_instruction(Memory* memory, const uint32_t memory_index, const uint32_t inst_index)
{
    // copying the "templete" instruction
    Instruction_Code inst = instruction_table[inst_index];


    uint32_t running_total = inst.field[0].count;
    uint8_t byte_number = 0;
    uint32_t inst_flags = 0;

    // if the op code is 8 bits
    if (running_total % 8 == 0)
        ++byte_number;

    // starting at the 2nd as we already got the op code from the table
    uint8_t field_index = 1;
    uint8_t saftey      = 0;
    while(1)
    {
        if (inst.field[field_index].usage == Not_Used)
            break;

        const uint8_t read_byte = memory->data[memory_index + byte_number];
        DEBUG(print_binary_8(read_byte))

        // calculating the offset and copying the next bit code into the table
        inst.field[field_index].offset = (running_total - (byte_number * 8));
        assert(inst.field[field_index].offset >= 0 && inst.field[field_index].offset < 8);
        uint8_t offset = inst.field[field_index].offset;

        // bit maskmasking and pushing the bit code down to be properly represented
        uint8_t bitmask = 0xFF;
        bitmask         >>= offset;

        if (offset + inst.field[field_index].count != 8)
            bitmask &= ~((1<<(inst.field[field_index].count)) -1);
        inst.field[field_index].value = (read_byte & bitmask) >> (8 - (inst.field[field_index].count + offset));

        // adding the count to the running total
        running_total += inst.field[field_index].count;

        // Logic to control the correct parsing
        switch (inst.field[field_index].usage)
        {
        case Bits_Literal:
            break;

        case Bits_D:
            break;
        case Bits_W:
            break;
        case Bits_S:
            if (inst.type != Op_cmp && inst.field[field_index].value)
                unset_bits_field(&inst, Bits_Data_H);
            break;

        case Bits_MOD:
            if (inst.field[field_index].value == NO_DISPLACEMENT)
                inst_flags |= CHECK_FOR_DIRECT_ADDRESS;
            else if (inst.field[field_index].value == _8_BIT_DISPLACEMENT)
                unset_bits_field(&inst, Bits_Disp_H);
            else if (inst.field[field_index].value == REGISTER_MODE)
            {
                unset_bits_field(&inst, Bits_Disp_H);
                unset_bits_field(&inst, Bits_Disp_L);
            }
            break;
        case Bits_REG:
            break;
        case Bits_SR:
            break;
        case Bits_RM:
            if (inst_flags & CHECK_FOR_DIRECT_ADDRESS)
                if (inst.field[field_index].value != DIRECT_ADDRESS)
                {
                    unset_bits_field(&inst, Bits_Disp_H);
                    unset_bits_field(&inst, Bits_Disp_L);
                }
            break;
        case Bits_Disp_L:
            break;
        case Bits_Disp_H:
            break;
        case Bits_Data_L:
            if (!ffetch(&inst, Bits_W))
                unset_bits_field(&inst, Bits_Data_H);
            break;
        case Bits_Data_H:
            break;
        case Bits_IP_INC8:
            break;

        // Cases should not come up / already handled
        case Bits_OP:
        case BITS_TYPE_COUNT:
        case Not_Used:
            assert(0);
        }

        // controll logic for moving to the next byte
        if (running_total % 8 == 0)
            ++byte_number;
        ++field_index;

        ++saftey;
        assert(saftey < 50 && "ERROR - loop saftey triggered in parsing of machine code\n");

    }
    DEBUG(debug_print_Assembly_Inst(&inst))

    construct_assembly_inst(&inst);
    return byte_number;
}


bool op_code_match(const uint8_t byte, Instruction_Code* inst)
{
    uint8_t op_code_test = byte >> (8 - inst->field[0].count);
    return op_code_test == inst->field[0].value;
}

void decode_instruction_stream(Memory* memory)
{
    uint32_t count = 0;
    while(count < memory->bytes_used)
    {
        DEBUG(print_binary_8(memory->data[count]))
        for(uint32_t i = 0; i < array_count(instruction_table); ++i)
        {
            if(op_code_match(memory->data[count], &instruction_table[i]))
            {
                count += decode_instruction(memory, count, i);
                break;
            }
            assert((i +1) != array_count(instruction_table) && "ERROR - unknown Op code\n");
        }
    }
}
