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

/* Declarations */

/*===================================================
  Memory
  =================================================*/
typedef struct Memory Memory;
void free_memory(Memory* memory);
void read_file(Memory* memory, const char* file_path);

/*===================================================
  Decoding unit
  =================================================*/
#define EXECUTION_OF_INSTRUCTION (1<<0)

typedef struct Assembly_Inst Assembly_Inst;

void print_inst_table();
void decode_instruction_stream(Memory* memory, uint32_t flags);


/*===================================================
  Execution unit
  =================================================*/
enum
{
    ax,
    bx,
    cx,
    dx,
    sp,
    bp,
    si,
    di,
    cs,
    ds,
    ss,
    es,
} Register_Index;

typedef enum
{
    CARRY_FLAG      = (1<<0),
    PARITY_FLAG     = (1<<2),
    AUXILIARY_FLAG = (1<<4),
    ZERO_FLAG       = (1<<6),
    SIGN_FLAG       = (1<<7),
    OVERFLOW_FLAG   = (1<<11),
    INTERRUPT_FLAG  = (1<<9),
    DIRECTION_FLAG  = (1<<10),
    TRAP_FLAG       = (1<<8)
} CPU_Flags;

typedef struct CP_units
{
    uint16_t reg[12];
    uint16_t ip;
    uint16_t flags;
    Memory* memory;
} CP_units;

CP_units* registers_init(Memory* memory);
void print_memory_status(CP_units* unit);

/* Implementation */

/*===================================================
  Memory
  =================================================*/
#define MEMORY_SIZE 1024*1024
typedef struct Memory
{
    uint8_t* data;
    uint32_t bytes_used; // only counting the number of bytes needed to store the program
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

/*===================================================
  Decoding unit
  =================================================*/
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
    Bits_V,
    Bits_Z,

    Bits_MOD,
    Bits_REG,
    Bits_RM, Bits_SR,

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
    case Bits_Z:
        return "Z";
    case Bits_V:
        return "V";
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
        print_binary_8(inst->field[i].value, LINE_P);
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


#define FIELD_NOT_SET -1
int ffetch(Instruction_Code* inst, Bits_Usage field)
{
    for (uint8_t i = 0; i < array_count(inst->field); ++i)
        if (inst->field[i].usage == field)
            return inst->field[i].value;
    return FIELD_NOT_SET;
}

typedef enum : uint16_t
{
    NO_LOCATION    = 0x0000,

    AX = 0xFFF0,
    CX = 0xFFF1,
    DX = 0xFFF2,
    BX = 0xFFF3,
    SP = 0xFFF4,
    BP = 0xFFF5,
    SI = 0xFFF6,
    DI = 0xFFF7,
    AL = 0xFF0F,
    CL = 0xFF1F,
    DL = 0xFF2F,
    BL = 0xFF3F,
    AH = 0xFF4F,
    CH = 0xFF5F,
    DH = 0xFF6F,
    BH = 0xFF7F,
    ES = 0xF0FF,
    CS = 0xF1FF,
    SS = 0xF2FF,
    DS = 0xF3FF,
    BX_SI = 0x0FFF,
    BX_DI = 0x1FFF,
    BP_SI = 0x2FFF,
    BP_DI = 0x3FFF,
    SI_ = 0x4FFF,
    DI_ = 0x5FFF,
    BP_ = 0x6FFF,
    BX_ = 0x7FFF,
    DIRECT_ADDRESS_LOCATION = 0x8FFF,
} Register_Location;

#define NOT_USED 0
enum
{
    FROM_IMMEDIATE  = (1<<0),
    FROM_REGISTER   = (1<<1),
    FROM_MEMORY     = (1<<2),
    TO_MEMORY       = (1<<3),
    WORD_OPPERATION = (1<<4)
};


typedef enum
{
    WORD_REGISTERS,
    BYTE_REGISTERS,
    SEGMENT_REGISTERS,
    EFFECTIVE_ADDRESSES
} Location_Type;

Register_Location location_exec(const Location_Type type, const uint8_t value)
{
    uint16_t location = 0xFFFF;

    switch (type)
    {
    case WORD_REGISTERS:
        location &= ~(0x000F);
        location |= (value);
        break;
    case BYTE_REGISTERS:
        location &= ~(0x00F0);
        location |= (value << 4);
        break;
    case SEGMENT_REGISTERS:
        location &= ~(0x0F00);
        location |= (value << 8);
        break;
    case EFFECTIVE_ADDRESSES:
        location &= ~(0xF000);
        location |= (value << 12);
        break;
    }
    return (Register_Location)location;
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
typedef struct Assembly_Inst
{
    Operation_Type mnemonic;
    char opperant1[MAX_SIZE_OF_OPPERANT];
    char opperant2[MAX_SIZE_OF_OPPERANT];
    bool printable;
} Assembly_Inst;

typedef enum
{
    SEGMENT_OVERRIDE_ES,
    SEGMENT_OVERRIDE_SC,
    SEGMENT_OVERRIDE_SS,
    SEGMENT_OVERRIDE_DS
} Segment_Override;

typedef struct
{
    int8_t segment_override;
} Decode_Unit;

Decode_Unit* decode_unit_init()
{
    Decode_Unit* d_unit = (Decode_Unit*)malloc(sizeof(Decode_Unit));
    d_unit->segment_override = -1;
    return d_unit;
}

bool seg_override(Decode_Unit* d_unit)
{
    return (d_unit->segment_override != -1);
}

void segment_override_flag(Assembly_Inst* assy, Instruction_Code* inst, Decode_Unit* d_unit)
{
    assy->printable = false;

    assert(!seg_override(d_unit));

    switch (inst->type)
    {
    case Op_es:
        d_unit->segment_override = SEGMENT_OVERRIDE_ES;
        return;
    case Op_sc:
        d_unit->segment_override = SEGMENT_OVERRIDE_SC;
        return;
    case Op_ss:
        d_unit->segment_override = SEGMENT_OVERRIDE_SS;
        return;
    case Op_ds:
        d_unit->segment_override = SEGMENT_OVERRIDE_DS;
        return;
    default:
        assert(0 && "ERROR - not a segment override instruction\n");
    }
}


bool operant_check(const char* string, const char needle)
{
    if (strlen(string) == 0)
        return false;

    uint32_t index = 0;
    while (string[index] != '\0')
    {
        if (string[index++] == needle)
            return true;
    }
    return false;
}

void segment_override_set(Assembly_Inst* assy, Decode_Unit* d_unit)
{
    assert(d_unit->segment_override >= SEGMENT_OVERRIDE_ES && d_unit->segment_override <= SEGMENT_OVERRIDE_DS);

    if (operant_check(assy->opperant1, '['))
    {
        char op1[MAX_SIZE_OF_OPPERANT];
        snprintf(op1, MAX_SIZE_OF_OPPERANT, "%s", assy->opperant1);

        snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "[%s:%s", segment_registers[d_unit->segment_override], op1 +1);
    }
    else if(operant_check(assy->opperant2, '['))
    {
        char op2[MAX_SIZE_OF_OPPERANT];
        snprintf(op2, MAX_SIZE_OF_OPPERANT, "%s", assy->opperant2);

        snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "[%s:%s", segment_registers[d_unit->segment_override], op2 +1);
    }

    d_unit->segment_override = -1;
}

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
    return (uint16_t)(byte_l | ((byte_h != FIELD_NOT_SET ? byte_h : 0) << 8));
}

static inline short disp_calc(int32_t disp_l, int32_t disp_h)
{
    if (disp_h == FIELD_NOT_SET)
        return (short)(disp_l | (((int8_t)disp_l < 0 ? 0xFF : 0) << 8));
    else
        return (short)(disp_l | (disp_h  << 8));
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
        snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", (w > 0 || w == FIELD_NOT_SET) ? word_registers[rm] : byte_registers[rm]);
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

void location_extract(Register_Location* Rm, Register_Location* Reg, Instruction_Code* inst)
{
    int32_t w      = ffetch(inst, Bits_W);
    int32_t mod    = ffetch(inst, Bits_MOD);
    int32_t reg    = ffetch(inst, Bits_REG);
    int32_t rm     = ffetch(inst, Bits_RM);
    int32_t disp_l = ffetch(inst, Bits_Disp_L);
    int32_t disp_h = ffetch(inst, Bits_Disp_H);

    switch (mod)
    {
    case REGISTER_MODE:
        *Rm = location_exec(w ? WORD_REGISTERS : BYTE_REGISTERS, rm);
        if (reg != NOT_USED)
            *Reg = location_exec(w ? WORD_REGISTERS : BYTE_REGISTERS, reg);
        break;
    case NO_DISPLACEMENT:
        if (rm == 0b110)
        {
            *Rm = DIRECT_ADDRESS_LOCATION;
            if (reg != NOT_USED)
                *Reg = location_exec(w ? WORD_REGISTERS : BYTE_REGISTERS, reg);
        }
        else
        {
            *Rm = location_exec(EFFECTIVE_ADDRESSES, rm);
            if (reg != NOT_USED)
                *Reg = location_exec(w ? WORD_REGISTERS : BYTE_REGISTERS, reg);
        }
        break;
    case _8_BIT_DISPLACEMENT:
    case _16_BIT_DISPLACEMENT:
        *Rm = location_exec(EFFECTIVE_ADDRESSES, rm);
        if (reg != NOT_USED)
            *Reg = location_exec(w ? WORD_REGISTERS : BYTE_REGISTERS, reg);
        break;
    }
}

void inst_exec(CP_units* exec, const Register_Location dest, const Register_Location src, const uint16_t value, const int16_t disp, const uint32_t flags, const Operation_Type op);
void mov_construct(Assembly_Inst* assy, Instruction_Code* inst, CP_units* exec)
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
    if (sr != FIELD_NOT_SET)
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

            if (exec != NULL)
            {
                {
                    Register_Location rm_location;
                    Register_Location reg_location;
                    location_extract(&rm_location, &reg_location, inst);

                    inst_exec(exec, rm_location, location_exec(SEGMENT_REGISTERS, sr), byte_calc(data_l, data_h), disp_calc(disp_l, disp_h), WORD_OPPERATION | FROM_REGISTER, Op_mov);

                }
            }
        }
        else // mov sr, r/m
        {
            snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", seg);
            snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "%s", effective_address);

            if (exec != NULL)
            {
                Register_Location rm_location;
                Register_Location reg_location;
                location_extract(&rm_location, &reg_location, inst);

                inst_exec(exec, location_exec(SEGMENT_REGISTERS, sr), rm_location, byte_calc(data_l, data_h), disp_calc(disp_l, disp_h), WORD_OPPERATION | FROM_REGISTER, Op_mov);
            }
        }

        return;
    }

    // immediate to register
    if (data_l != FIELD_NOT_SET && mod == FIELD_NOT_SET)
    {
        const char* reg_name = w ? word_registers[reg] : byte_registers[reg];
        snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", reg_name);
        snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "%s %hu", w ? "word" : "byte", byte_calc(data_l, data_h));

        if (exec != NULL)
        {
            inst_exec(exec, location_exec(w ? WORD_REGISTERS : BYTE_REGISTERS, (uint8_t)reg), NO_LOCATION, byte_calc(data_l, data_h), NOT_USED, (w ? WORD_OPPERATION : 0) | FROM_IMMEDIATE, Op_mov);
        }

        return;
    }

    // memory to/from accumulator
    if (disp_l != FIELD_NOT_SET && mod == FIELD_NOT_SET)
    {
        uint16_t addr    = byte_calc(disp_l, disp_h);
        const char* acc  = (w > 0) ? "ax" : "al";
        int32_t op_val   = ffetch(inst, Bits_OP);

        if (op_val == 0b1010000) // mov acc, [addr]
        {
            snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", acc);
            snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "[%hu]", addr);

            if (exec != NULL)
            {
                inst_exec(exec, w ? AX : AL, DIRECT_ADDRESS_LOCATION, NOT_USED, byte_calc(data_l, data_h), (w ? WORD_OPPERATION : 0) | FROM_MEMORY, Op_mov);
            }
        }
        else // mov [addr], acc
        {
            snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "[%hu]", addr);
            snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "%s", acc);

            if (exec != NULL)
            {
                inst_exec(exec, DIRECT_ADDRESS_LOCATION, w ? AX : AL, NOT_USED, byte_calc(data_l, data_h), (w ? WORD_OPPERATION : 0) | FROM_MEMORY, Op_mov);
            }
        }
        return;
    }

    // register/memory encoding
    if (mod != FIELD_NOT_SET && reg != FIELD_NOT_SET && rm != FIELD_NOT_SET)
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
            if (exec != NULL)
            {
                Register_Location rm_location;
                Register_Location reg_location;
                location_extract(&rm_location, &reg_location, inst);

                inst_exec(exec, reg_location, rm_location, byte_calc(data_l, data_h), disp_calc(disp_l, disp_h), (w ? WORD_OPPERATION : 0) | FROM_REGISTER, Op_mov);
            }
        }
        else
        {
            snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", effective_address);
            snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "%s", reg_name);
            if (exec != NULL)
            {
                Register_Location rm_location;
                Register_Location reg_location;
                location_extract(&rm_location, &reg_location, inst);

                inst_exec(exec, rm_location, reg_location, byte_calc(data_l, data_h), disp_calc(disp_l, disp_h), (w ? WORD_OPPERATION : 0) | FROM_REGISTER, Op_mov);
            }
        }
        return;
    }

    //immediate to reg/mem
    if (mod != FIELD_NOT_SET && data_l != FIELD_NOT_SET)
    {
        mod_rm_effective_address(assy, inst);

        snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "%s %hu", w ? "word" : "byte", byte_calc(data_l, data_h));

        if (exec != NULL)
        {
            Register_Location rm_location;
            Register_Location reg_location;
            location_extract(&rm_location, &reg_location, inst);

            inst_exec(exec, rm_location, NO_LOCATION, byte_calc(data_l, data_h), disp_calc(disp_l, disp_h), (w ? WORD_OPPERATION : 0) | FROM_IMMEDIATE, Op_mov);
        }
        return;
    }
    assert(0 && "ERROR - when constructing mov\n");
}

void arithmetic_construct(Assembly_Inst* assy, Instruction_Code* inst, Decode_Unit* d_unit, CP_units* exec)
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
    if (mod == FIELD_NOT_SET && reg == FIELD_NOT_SET && rm == FIELD_NOT_SET)
    {
        const char* acc = (w) ? "ax" : "al";
        snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", acc);
        snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "%hu", byte_calc(data_l, data_h));
        return;

        if (exec != NULL)
        {
            inst_exec(exec, w ? AX : AL, NO_LOCATION, byte_calc(data_l, data_h), NOT_USED, FROM_IMMEDIATE, inst->type);
        }
    }


    // Reg/Mem with register to either
    if (d != FIELD_NOT_SET)
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


        if (exec != NULL)
        {
            Register_Location rm_location;
            Register_Location reg_location;
            location_extract(&rm_location, &reg_location, inst);
            uint32_t mode = (rm_location >= BX_SI && rm_location <= DIRECT_ADDRESS_LOCATION) ? (d > 0) ? FROM_MEMORY : TO_MEMORY : FROM_REGISTER;

            inst_exec(exec, d > 0 ? reg_location : rm_location,  d > 0 ? rm_location : reg_location, byte_calc(data_l, data_h), NOT_USED, mode, inst->type);
        }

        return;
    }

    // immediate to register/memory
    if (ffetch(inst, Bits_Literal) != FIELD_NOT_SET)
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

        if (exec != NULL)
        {
            inst_exec(exec, location_exec(w ? WORD_REGISTERS : BYTE_REGISTERS, (uint8_t)rm), NO_LOCATION, (w && s) ? disp_calc(data_l, data_h) : byte_calc(data_l, data_h), NOT_USED, FROM_IMMEDIATE, inst->type);
        }

        return;

    }
    assert(0 && "ERROR - when constructing arithmetic op\n");
}

void cond_jump_construct(Assembly_Inst* assy, Instruction_Code* inst, CP_units* exec)
{
    snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%hhd", (int8_t)ffetch(inst, Bits_IP_INC8));

    if (exec != NULL)
        inst_exec(exec, NO_LOCATION, NO_LOCATION, (int8_t)ffetch(inst, Bits_IP_INC8), NOT_USED, 0, inst->type);
}



static inline void swap_opperants(Assembly_Inst* assy)
{
    char temp[MAX_SIZE_OF_OPPERANT];
    snprintf(temp, MAX_SIZE_OF_OPPERANT, "%s", assy->opperant1);
    snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", assy->opperant2);
    snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "%s", temp);
}

enum
{
    _movs = 0b1010010,
    _cmps = 0b1010011,
    _stds = 0b1010101,
    _lods = 0b1010110,
    _scas = 0b1010111
};
const char* string_manpi[] =
{"movs", "cmps", "stds", "lods", "scas"};

void string_mani_construct(Assembly_Inst* assy, Instruction_Code* inst)
{
    int32_t w = ffetch(inst, Bits_W);

    if (inst->type == Op_rep)
    {
        int32_t z = ffetch(inst, Bits_Z);
        uint8_t i;

        switch(inst->field[2].value)
        {
        case _movs:
            i = 0;
            break;
        case _cmps:
            i = 1;
            break;
        case _stds:
            i = 2;
            break;
        case _lods:
            i = 3;
            break;
        case _scas:
            i = 4;
            break;
        default:
            assert(0 && "ERROR - unknow type after rep op\n");
        }

        if (i == 1 || i == 4)
            snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", z ? "repe" : "repne");
        else
            snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", "rep");

        snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "%s%c", string_manpi[i], w ? 'w' : 'b');
    }
    else
    {
        snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s%c", instruction_string(inst->type), w ? 'w' : 'b');
    }

}

enum // for switching on op codes
{
    _mul  = 0b100,
    _imul = 0b101,
    _div  = 0b110,
    _idiv = 0b111,
    _not  = 0b010,
    _neg  = 0b011
};

void logic_construct(Assembly_Inst* assy, Instruction_Code* inst)
{
    mod_rm_effective_address(assy, inst);

    if (ffetch(inst, Bits_MOD) != 0b11)
    {
        int32_t w = ffetch(inst, Bits_W);
        char temp[MAX_SIZE_OF_OPPERANT];
        snprintf(temp, MAX_SIZE_OF_OPPERANT, "%s", assy->opperant1);
        snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s %.26s", w ? "word" : "byte", temp);
    }

    int32_t v = ffetch(inst, Bits_V);
    if (v == FIELD_NOT_SET)
    {
        uint8_t bl = (uint8_t)ffetch(inst, Bits_Literal);
        switch(bl)
        {
        case _mul:
            inst->type = Op_mul;
            break;
        case _imul:
            inst->type = Op_imul;
            break;
        case _div:
            inst->type = Op_div;
            break;
        case _idiv:
            inst->type = Op_idiv;
            break;
        case _not:
            inst->type = Op_not;
            break;
        case _neg:
            inst->type = Op_neg;
            break;
        default:
            assert(0);
        }
    }
    else
    {
        snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "%s", v ? "cl" : "1");
        // changing the op_code to the proper mnemonic
        inst->type = (Operation_Type)(inst->type + ffetch(inst, Bits_Literal));
    }

}

void inc_dec_neg_construct(Assembly_Inst* assy, Instruction_Code* inst)
{
    int32_t reg = ffetch(inst, Bits_REG);

    if (reg == FIELD_NOT_SET)
    {
        mod_rm_effective_address(assy, inst);
        // changing the op_code to the proper mnemonic
        if (inst->type != Op_neg)
            inst->type = (Operation_Type)(inst->type + ffetch(inst, Bits_Literal));
    }
    else
        snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", word_registers[reg]);
}


void out_in_construct(Assembly_Inst* assy, Instruction_Code* inst)
{
    if (ffetch(inst, Bits_W) == 1)
        snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "ax");
    else
        snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "al");

    int32_t data_l = ffetch(inst, Bits_Data_L);
    if (data_l != FIELD_NOT_SET)
        snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%hhu", (uint8_t)data_l);
}

void call_jump_construct(Assembly_Inst* assy, Instruction_Code* inst)
{
    int32_t bl       = ffetch(inst, Bits_Literal);
    int32_t disp_l   = ffetch(inst, Bits_Disp_L);
    int32_t disp_h   = ffetch(inst, Bits_Disp_H);
    int32_t offset_l = ffetch(inst, Bits_Data_L);
    int32_t offset_h = ffetch(inst, Bits_Data_H);

    // direct memory call / jump
    if (offset_l != FIELD_NOT_SET && offset_h != FIELD_NOT_SET)
    {
        snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%hd:%hd", byte_calc(offset_l, offset_h), byte_calc(disp_l, disp_h));
    }
    else if (bl == FIELD_NOT_SET)
    {
        snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "$+(%hd)", (short)((disp_h == FIELD_NOT_SET ? 2 : 3) + disp_calc(disp_l, disp_h)));

    }
    else
    {
        mod_rm_effective_address(assy, inst);
        char tmp[MAX_SIZE_OF_OPPERANT];
        snprintf(tmp, MAX_SIZE_OF_OPPERANT, "%s", assy->opperant1);
        snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s %.27s", bl == 0b011 || bl == 0b101 ? "far" : "word", tmp);


        // as inc / dec share the same op code
        if (bl == 0b110)
            inst->type = Op_push;
        else if (bl == 0b001)
            inst->type = Op_dec;
        else if (bl == 0b000)
            inst->type = Op_inc;
        else if (bl > 0b11)
            inst->type = (Operation_Type)(inst->type +1);
    }
}

void xchg_construct(Assembly_Inst* assy, Instruction_Code* inst)
{
    // register with accumulator
    if (inst->field[0].count == 5)
    {
        snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", "ax");
        snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "%s", word_registers[ffetch(inst, Bits_REG)]);
    }
    else
    {
        mod_rm_effective_address(assy, inst);
        snprintf(assy->opperant2, MAX_SIZE_OF_OPPERANT, "%s", word_registers[ffetch(inst, Bits_REG)]);

        // swap opperants for register mod
        if (ffetch(inst, Bits_MOD) == 0b11)
            swap_opperants(assy);
    }
}

void pop_push_construct(Assembly_Inst* assy, Instruction_Code* inst)
{
    int32_t sr  = ffetch(inst, Bits_SR);
    int32_t reg = ffetch(inst, Bits_REG);

    // segment register
    if (sr != FIELD_NOT_SET)
    {
        snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", segment_registers[sr]);

        if (ffetch(inst, Bits_Literal) == 0b111)
            inst->type = Op_pop;
    }
    // register
    else if (reg != FIELD_NOT_SET)
        snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "%s", word_registers[reg]);

    // memory
    else
    {
        mod_rm_effective_address(assy, inst);

        char tmp[MAX_SIZE_OF_OPPERANT];
        snprintf(tmp, MAX_SIZE_OF_OPPERANT, "%s", assy->opperant1);
        snprintf(assy->opperant1, MAX_SIZE_OF_OPPERANT, "word %.26s", tmp);
    }
}

void print_assembly_inst(Assembly_Inst* assy)
{
    if (assy->mnemonic >= Op_rep && assy->mnemonic <= Op_scas)
        printf("%s %s", assy->opperant1, strlen(assy->opperant2) == 0 ? "" : assy->opperant2);
    else if (strlen(assy->opperant1) == 0 && strlen(assy->opperant2) == 0)
        printf("%s", instruction_string(assy->mnemonic));
    else if (strlen(assy->opperant2) == 0)
        printf("%s %s", instruction_string(assy->mnemonic), assy->opperant1);
    else
        printf("%s %s, %s", instruction_string(assy->mnemonic), assy->opperant1, assy->opperant2);
}

void construct_assembly_inst(Instruction_Code* inst, Decode_Unit* d_unit, CP_units* exec)
{
    Assembly_Inst assy;
    memset(&assy, 0, sizeof(Assembly_Inst));
    assy.printable = true;

    CP_units old_state = {0};
    if (exec != NULL)
        memcpy(&old_state, exec, sizeof(CP_units));


    if (inst->type >= Op_je && inst->type <= Op_jcxz)
        cond_jump_construct(&assy, inst, exec);
    else if (inst->type >= Op_add && inst->type <= Op_test)
        arithmetic_construct(&assy, inst, d_unit, exec);
    else if (inst->type >= Op_mul && inst->type <= Op_sar)
        logic_construct(&assy, inst);
    else if (inst->type >= Op_rep && inst->type <= Op_scas)
        string_mani_construct(&assy, inst);
    else if (inst->type >= Op_es && inst->type <= Op_ds)
        segment_override_flag(&assy, inst, d_unit);
    else if (inst->field[1].usage == Not_Used || inst->field[1].usage == Bits_Literal)
        ; // fast path for only complete 1 byte ops - or hardcoded op
    else
        switch (inst->type)
        {
        case Op_pop:
        case Op_push:
            pop_push_construct(&assy, inst);
            break;
        case Op_out:
        case Op_in:
            out_in_construct(&assy, inst);
            break;
        case Op_xchg:
            xchg_construct(&assy, inst);
            break;
        case Op_lea:
        case Op_lds:
        case Op_les:
            mod_rm_effective_address(&assy, inst);
            break;
        case Op_mov:
            mov_construct(&assy, inst, exec);
            break;
        case Op_dec:
        case Op_inc:
            inc_dec_neg_construct(&assy, inst);
            break;
        case Op_ret:
        case Op_retf:
            snprintf(assy.opperant1, MAX_SIZE_OF_OPPERANT, "%hu", byte_calc(inst->field[1].value, inst->field[2].value));
            break;
        case Op_jmp:
        case Op_call:
            call_jump_construct(&assy, inst);
            break;
        case Op_int:
            if (ffetch(inst, Bits_Data_L) != -1)
                snprintf(assy.opperant1, MAX_SIZE_OF_OPPERANT, "%hhu", (uint8_t)ffetch(inst, Bits_Data_L));
            break;
        default:
            assert(0);
        }

    assy.mnemonic = inst->type;


    if (assy.printable)
    {
        if (seg_override(d_unit)) // Segmentation override
            segment_override_set(&assy, d_unit);

        print_assembly_inst(&assy);
        if (exec != NULL)
            print_register_change(&old_state, exec);
        printf("\n");
    }
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


int decode_instruction(Memory* memory, Decode_Unit* d_unit, const uint32_t memory_index, const uint32_t inst_index, CP_units* exec)
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
        DEBUG(print_binary_8(read_byte, NEWLINE_P))


        // calculating the offset and copying the next bit code into the table
        inst.field[field_index].offset = (running_total - (byte_number * 8));
        assert(inst.field[field_index].offset >= 0 && inst.field[field_index].offset < 8);
        uint8_t offset = inst.field[field_index].offset;

        // bit maskmasking and pushing the bit code down to be properly represented
        uint8_t bitmask = 0xFF;
        bitmask         >>= offset;

        if (offset + inst.field[field_index].count != 8 && bitmask != 0xFF)
            bitmask &= ~((1<<(inst.field[field_index].count)) -1);
        inst.field[field_index].value = (read_byte & bitmask) >> (8 - (inst.field[field_index].count + offset));

        // adding the count to the running total
        running_total += inst.field[field_index].count;

        // Logic to control the correct parsing
        switch (inst.field[field_index].usage)
        {
        case Bits_Literal:
            if (inst.type == Op_test && inst.field[field_index].value != 0b000)
            {
                inst.type = Op_imul;
                unset_bits_field(&inst, Bits_Data_L);
                unset_bits_field(&inst, Bits_Data_H);
            }
            break;
        case Bits_D:
            break;
        case Bits_W:
            break;
        case Bits_V:
            break;
        case Bits_Z:
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

    if (exec != NULL)
        exec->ip += byte_number;

    construct_assembly_inst(&inst, d_unit, exec);

    return byte_number;
}


bool op_code_match(const uint8_t byte, Instruction_Code* inst)
{
    uint8_t op_code_test = byte >> (8 - inst->field[0].count);
    return op_code_test == inst->field[0].value;
}


void decode_instruction_stream(Memory* memory, uint32_t flags)
{
    Decode_Unit* d_unit = decode_unit_init();
    uint32_t count      = 0;

    CP_units* exec = NULL;
    if (flags & EXECUTION_OF_INSTRUCTION)
        exec = registers_init(memory);


    while(count < memory->bytes_used)
    {
        DEBUG(print_binary_8(memory->data[count], NEWLINE_P))
        for(uint32_t i = 0; i < array_count(instruction_table); ++i)
        {
            if(op_code_match(memory->data[count], &instruction_table[i]))
            {
                if (exec != NULL)
                {
                    decode_instruction(memory, d_unit, exec->ip, i, exec);
                    count = exec->ip;
                }
                else
                    count += decode_instruction(memory, d_unit, count, i, exec);
                DEBUG(printf("bytes parsed count: %u, total memory: %u\n\n", count, memory->bytes_used))
                break;
            }
            assert((i +1) != array_count(instruction_table) && "ERROR - unknown Op code\n");
        }
    }

    if (flags & EXECUTION_OF_INSTRUCTION)
    {
        printf("\nFinal state of registers");
        print_memory_status(exec);
    }

    free(d_unit);
    if (exec != NULL)
        free(exec);
}


/*==========================================
  Simulation Unit
  ========================================*/

void print_memory(Memory* memory)
{
    bool data_found = false;
    for(uint32_t i = memory->bytes_used; i < MEMORY_SIZE; ++i)
        if (memory->data[i] != 0)
        {
            printf("\t[%u]: %hhu\n", i, memory->data[i]);
            data_found = true;
        }
    if (!data_found)
        printf("\tNo data found after program instruction\n");
}

CP_units* registers_init(Memory* memory)
{
    CP_units* r = (CP_units*)malloc(sizeof(CP_units));
    memset(r, 0, sizeof(CP_units));
    r->memory = memory;
    printf("\nInitial state of registers");
    print_memory_status(r);
    printf("\n");
    return r;
}

const char* register_string(uint8_t i)
{
    switch(i)
    {
    case 0:
        return "ax";
    case 1:
        return "bx";
    case 2:
        return "cx";
    case 3:
        return "dx";
    case 4:
        return "sp";
    case 5:
        return "bp";
    case 6:
        return "si";
    case 7:
        return "di";
    case 8:
        return "cs";
    case 9:
        return "ds";
    case 10:
        return "ss";
    case 11:
        return "es";
    default:
        assert(0);
    }
}


void print_set_flags(CP_units* exec)
{
    if (exec->flags & CARRY_FLAG)
        printf("C ");
    if (exec->flags & PARITY_FLAG)
        printf("P ");
    if (exec->flags & AUXILIARY_FLAG)
        printf("A ");
    if (exec->flags & ZERO_FLAG)
        printf("Z ");
    if (exec->flags & SIGN_FLAG)
        printf("S ");
    if (exec->flags & TRAP_FLAG)
        printf("T ");
    if (exec->flags & INTERRUPT_FLAG)
        printf("I ");
    if (exec->flags & DIRECTION_FLAG)
        printf("D ");
    if (exec->flags & OVERFLOW_FLAG)
        printf("O ");
}

void print_register_change(CP_units* old_state, CP_units* new_state)
{
    bool change = false;

    printf("   ;");
    for (uint8_t i = 0; i < array_count(old_state->reg); ++i)
    {
        if (old_state->reg[i] != new_state->reg[i])
        {
            if (change)
                printf("  | ");
            printf("(%s) 0x%04hx -> 0x%04hx", register_string(i), old_state->reg[i], new_state->reg[i]);
            change = true;
        }
    }

    if (change)
        printf("  | ");
    printf("ip: %hu -> %hu | ", old_state->ip, new_state->ip);
    printf("(flags) ");
    print_set_flags(old_state);
    if(old_state->flags != new_state->flags)
    {
        printf("-> ");
        print_set_flags(new_state);
    }
}

void print_memory_status(CP_units* unit)
{
    printf("\nHexadecimal register values:\n");
    for (uint8_t i = 0; i < array_count(unit->reg); ++i)
    {
        printf("\t\t%s: 0x%04hx\n", register_string(i), unit->reg[i]);
    }
    printf("\tip: %hu\n", unit->ip);
    printf("\tflags: ");
    print_set_flags(unit);
    printf("\n\t(binary) ");
    print_binary_16(unit->flags, LINE_P);
    printf("\n");
    print_memory(unit->memory);
    printf("\n");

}


// typedef enum : uint16_t
// {
//     AX = 0xFFF0,
//     CX = 0xFFF1,
//     DX = 0xFFF2,
//     BX = 0xFFF3,
//     SP = 0xFFF4,
//     BP = 0xFFF5,
//     SI = 0xFFF6,
//     DI = 0xFFF7,
//     AL = 0xFF0F,
//     CL = 0xFF1F,
//     DL = 0xFF2F,
//     BL = 0xFF3F,
//     AH = 0xFF4F,
//     CH = 0xFF5F,
//     DH = 0xFF6F,
//     BH = 0xFF7F,
//     ES = 0xF0FF,
//     CS = 0xF1FF,
//     SS = 0xF2FF,
//     DS = 0xF3FF
// } Register_Location;

uint8_t at_reg(Register_Location reg, uint16_t* bitmask, uint8_t* bit_shift)
{
    switch (reg)
    {
    case AX:
        return ax;
    case CX:
        return cx;
    case DX:
        return dx;
    case BX:
        return bx;
    case SP:
        return sp;
    case BP:
        return bp;
    case SI:
        return si;
    case DI:
        return di;
    case AL:
        if (bitmask != NULL)
            *bitmask  = 0x00FF;
        return ax;
    case CL:
        if (bitmask != NULL)
            *bitmask  = 0x00FF;
        return cx;
    case DL:
        if (bitmask != NULL)
            *bitmask  = 0x00FF;
        return dx;
    case BL:
        if (bitmask != NULL)
            *bitmask  = 0x00FF;
        return bx;
    case AH:
        if (bitmask != NULL)
            *bitmask  = 0xFF00;
        if (bit_shift != NULL)
            *bit_shift = 8;
        return ax;
    case CH:
        if (bitmask != NULL)
            *bitmask  = 0xFF00;
        if (bit_shift != NULL)
            *bit_shift = 8;
        return cx;
    case DH:
        if (bitmask != NULL)
            *bitmask  = 0xFF00;
        if (bit_shift != NULL)
            *bit_shift = 8;
        return dx;
    case BH:
        if (bitmask != NULL)
            *bitmask  = 0xFF00;
        if (bit_shift != NULL)
            *bit_shift = 8;
        return bx;
    case ES:
        return es;
    case CS:
        return cs;
    case SS:
        return ss;
    case DS:
        return ds;
    default:
        return 255;
    }
}

uint16_t effective_address_calculation(CP_units* exec, const Register_Location location, int16_t disp)
{
    switch (location)
    {
    case BX_SI:
        return exec->reg[bx] + exec->reg[di] + (disp == -1 ? 0 : disp);
    case BP_DI:
        return exec->reg[bp] + exec->reg[di] + (disp == -1 ? 0 : disp);
    case BP_SI:
        return exec->reg[bp] + exec->reg[si] + (disp == -1 ? 0 : disp);
    case SI_:
        return exec->reg[si] + (disp == -1 ? 0 : disp);
    case DI_:
        return exec->reg[di] + (disp == -1 ? 0 : disp);
    case BP_:
        return exec->reg[bp] + (disp == -1 ? 0 : disp);
    case BX_:
        return exec->reg[bx] + (disp == -1 ? 0 : disp);
    default:
        assert(0 && "ERROR - invalid location for displacement calculation\n");
    }
}

#define MSB (1<<15)
void arithmetic_set_flags(CP_units* exec, const int16_t result, const int16_t before, const int16_t value, const Operation_Type op)
{
    if (result == 0)
        exec->flags |= ZERO_FLAG;
    else
        exec->flags &= ~ZERO_FLAG;

    if (result & MSB)
        exec->flags |= SIGN_FLAG;
    else
        exec->flags &= ~SIGN_FLAG;

    uint8_t count = 0;
    for (uint8_t i = 0; i < 8; ++i)
        if (result & (1<<i))
            ++count;
    if (count % 2 == 0)
        exec->flags |= PARITY_FLAG;
    else
        exec->flags &= ~PARITY_FLAG;

    //printf("result: %d, before %d, amount: %d\n", result, before, value);
    switch(op)
    {
    case Op_add:
        // adding numbers with the same sign and the result is diff
        if (((before ^ result) & (value ^ result)) < 0)
            exec->flags |= OVERFLOW_FLAG;
        else
            exec->flags &= ~OVERFLOW_FLAG;
        if (result < before)
            exec->flags |= CARRY_FLAG;
        else
            exec->flags &= ~CARRY_FLAG;
        break;
    case Op_sub:
    case Op_cmp:
        // sub with different signed numbers and result sign diff from first
        if (((before ^ value) & (before ^ result)) < 0)
            exec->flags |= OVERFLOW_FLAG;
        else
            exec->flags &= ~OVERFLOW_FLAG;
        if (value > before)
            exec->flags |= CARRY_FLAG;
        else
            exec->flags &= ~CARRY_FLAG;
        break;
    default:

        break;
    }

}

static inline bool cond_jumps_check_flag(CP_units* exec, const Operation_Type jump, const uint16_t flag)
{
    switch (jump)
    {
    case Op_je:
        return flag & ZERO_FLAG;
    case Op_jne:
        return !(flag & ZERO_FLAG);
    case Op_jp:
        return (flag & PARITY_FLAG);
    case Op_jb:
        return flag & CARRY_FLAG;
    case Op_loop:
        --exec->reg[cx];
        return exec->reg[cx] != 0;
    case Op_loopz:
        --exec->reg[cx];
        return flag & ZERO_FLAG && exec->reg[cx] != 0;
    case Op_loopnz:
        --exec->reg[cx];
        return !(flag & ZERO_FLAG) && exec->reg[cx] != 0;

    default:
        assert(0 && "ERROR - jump instruction not yet implementated\n");
    }
}

void inst_exec(CP_units* exec, const Register_Location dest, const Register_Location src, const uint16_t value, const int16_t disp, const uint32_t flags, const Operation_Type op)
{

    if (op >= Op_je && op <= Op_jcxz)
    {
        if (cond_jumps_check_flag(exec, op, exec->flags))
            exec->ip += (int8_t)value;
        return;
    }

    uint8_t bit_shift = 0;
    uint16_t bitmask  = 0xFFFF;
    uint8_t r_dest    = at_reg(dest, &bitmask, &bit_shift);

    switch (op)
    {
    case Op_mov:
        if (dest >= BX_SI && dest <= DIRECT_ADDRESS_LOCATION)
        {
            if (dest == DIRECT_ADDRESS_LOCATION)
            {
                if (flags & FROM_IMMEDIATE)
                {
                    if (flags & WORD_OPPERATION)
                    {
                        exec->memory->data[(uint16_t)disp]    = (uint8_t)value;
                        exec->memory->data[(uint16_t)disp +1] = (uint8_t)(value >> 8);
                    }
                    else
                        exec->memory->data[(uint16_t)disp]    = (uint8_t)value;
                }
                else if (flags & FROM_REGISTER)
                {
                    uint8_t s_shift    = 0;
                    uint8_t r_src      = at_reg(src, NULL, &s_shift);
                    if (flags & WORD_OPPERATION)
                    {
                        exec->memory->data[(uint16_t)disp]    = (uint8_t)(exec->reg[r_src] >> s_shift);
                        exec->memory->data[(uint16_t)disp +1] = (uint8_t)((exec->reg[r_src] >> s_shift) >> 8);
                    }
                    else
                        exec->memory->data[(uint16_t)disp]    = (uint8_t)(exec->reg[r_src] >> s_shift);;
                }
                else
                    assert(0);
            }
            else
            {
                uint16_t memory_index = effective_address_calculation(exec, dest, disp);
                if (flags & FROM_IMMEDIATE)
                {
                    if (flags & WORD_OPPERATION)
                    {
                        exec->memory->data[memory_index]    = (uint8_t)value;
                        exec->memory->data[memory_index +1] = (uint8_t)(value >> 8);
                    }
                    else
                        exec->memory->data[(uint16_t)disp]    = (uint8_t)value;
                }
                else if (flags & FROM_REGISTER)
                {
                    uint8_t s_shift    = 0;
                    uint8_t r_src      = at_reg(src, NULL, &s_shift);
                    if (flags & WORD_OPPERATION)
                    {
                        exec->memory->data[memory_index]    = (uint8_t)(exec->reg[r_src] >> s_shift);
                        exec->memory->data[memory_index +1] = (uint8_t)((exec->reg[r_src] >> s_shift) >> 8);
                    }
                    else
                        exec->memory->data[(uint16_t)disp]    = (uint8_t)(exec->reg[r_src] >> s_shift);;
                }
                else
                    assert(0);
            }

        }
        else if (src >= BX_SI && src <= DIRECT_ADDRESS_LOCATION)
        {
            if (src == DIRECT_ADDRESS_LOCATION)
            {
                if (flags & FROM_REGISTER)
                {
                    uint8_t s_shift    = 0;
                    uint8_t r_src      = at_reg(src, NULL, &s_shift);
                    if (flags & WORD_OPPERATION)
                        exec->reg[r_dest] = (exec->memory->data[(uint16_t)disp] << 8) | exec->memory->data[(uint16_t)disp +1];
                    else
                        exec->reg[r_dest] = (exec->reg[r_dest] & ~bitmask) | (bitmask & (exec->memory->data[(uint16_t)disp] << bit_shift));
                }
                else
                    assert(0);
            }
            else
            {
                uint16_t memory_index = effective_address_calculation(exec, src, disp);
                if (flags & WORD_OPPERATION)
                    exec->reg[r_dest] = (exec->memory->data[memory_index +1] << 8) | exec->memory->data[memory_index];
                else
                    exec->reg[r_dest] = (exec->reg[r_dest] & ~bitmask) | (bitmask & (exec->memory->data[memory_index] << bit_shift));
            }
        }
        else if (flags & FROM_IMMEDIATE)
            exec->reg[r_dest]  = (exec->reg[r_dest] & ~bitmask ) | (bitmask & (value << bit_shift));
        else if (flags & FROM_REGISTER)
        {
            uint8_t s_shift    = 0;
            uint8_t r_src      = at_reg(src, NULL, &s_shift);

            exec->reg[r_dest]  = (exec->reg[r_dest] & ~bitmask ) | (bitmask & ((exec->reg[r_src] >> s_shift) << bit_shift));
        }
        else if (flags & FROM_MEMORY)
            assert(0 && "ERROR - Not yet implementated\n");
        else
            assert(0 && "ERROR - Not yet implementated\n");
        break;
    case Op_add:
    {
        uint16_t result;
        uint16_t amount;
        const uint16_t before = exec->reg[r_dest] >> bit_shift;
        if (flags & FROM_IMMEDIATE)
        {
            amount             = value;
            result             = (exec->reg[r_dest] >> bit_shift) + amount;
            exec->reg[r_dest]  = (exec->reg[r_dest] & ~bitmask ) | (bitmask & (result << bit_shift));
        }
        else if (flags & FROM_REGISTER)
        {
            uint8_t s_shift    = 0;
            uint8_t r_src      = at_reg(src, NULL, &s_shift);

            amount             = (exec->reg[r_src] >> s_shift);
            result             = (exec->reg[r_dest] >> bit_shift) + amount;
            exec->reg[r_dest]  = (exec->reg[r_dest] & ~bitmask ) | (bitmask & (result << bit_shift));

        }
        else if (flags & FROM_MEMORY)
        {
            uint16_t memory_index = effective_address_calculation(exec, src, disp);

            amount             = exec->memory->data[memory_index] | (flags & WORD_OPPERATION ? (exec->memory->data[memory_index +1] << 8) : 0);
            result             = (exec->reg[r_dest] >> bit_shift) + amount;
            exec->reg[r_dest]  = (exec->reg[r_dest] & ~bitmask ) | (bitmask & (result << bit_shift));
        }
        else if (flags & TO_MEMORY)
        {

        }
        else
            assert(0 && "ERROR - Not yet implementated\n");

        arithmetic_set_flags(exec, result, before, amount, Op_add);
        break;
    }
    case Op_sub:
    {
        uint16_t result;
        uint16_t amount;
        const uint16_t before = exec->reg[r_dest] >> bit_shift;
        if (flags & FROM_IMMEDIATE)
        {
            amount             = value;
            result             = (exec->reg[r_dest] >> bit_shift) - amount;
            exec->reg[r_dest]  = (exec->reg[r_dest] & ~bitmask ) | (bitmask & (result << bit_shift));

        }
        else if (flags & FROM_REGISTER)
        {
            uint8_t s_shift    = 0;
            uint8_t r_src      = at_reg(src, NULL, &s_shift);

            amount             = (exec->reg[r_src] >> s_shift);
            result             = (exec->reg[r_dest] >> bit_shift) - amount;
            exec->reg[r_dest]  = (exec->reg[r_dest] & ~bitmask ) | (bitmask & (result << bit_shift));


        }
        else if (flags & FROM_MEMORY)
            assert(0 && "ERROR - Not yet implementated\n");
        else
            assert(0 && "ERROR - Not yet implementated\n");
        arithmetic_set_flags(exec, result, before, amount, Op_sub);
        break;
    }
    case Op_cmp:
    {
        uint16_t result;
        uint16_t amount;
        const uint16_t before = exec->reg[r_dest] >> bit_shift;
        if (flags & FROM_IMMEDIATE)
        {
            amount = value;
            result = (exec->reg[r_dest] >> bit_shift) - amount;

        }
        else if (flags & FROM_REGISTER)
        {
            uint8_t s_shift    = 0;
            uint8_t r_src      = at_reg(src, NULL, &s_shift);

            amount = (exec->reg[r_src] >> s_shift);
            result = (exec->reg[r_dest] >> bit_shift) - amount;

        }
        else if (flags & FROM_MEMORY)
            assert(0 && "ERROR - Not yet implementated\n");
        else
            assert(0 && "ERROR - Not yet implementated\n");

        arithmetic_set_flags(exec, result, before, amount, Op_cmp);
        break;
    }
    default:
        assert(0 && "ERROR - Op code not yet implementated\n");
    }

}
