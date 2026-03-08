#include "8086_simulator.h"


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Args = file path\n");
        return 0;
    }

    //print_inst_table();
    Memory memory = {0};


    uint32_t flags = 0;
    if (argc > 2)
    {

        if (strcmp(argv[1], "-exec") == 0)
            flags = EXECUTION_OF_INSTRUCTION;
        else if (strcmp(argv[1], "-dump") == 0)
            flags = EXECUTION_OF_INSTRUCTION | DUMP_MEMORY_AFTER_EXEC;
        else if (strcmp(argv[1], "-clocks") == 0)
            flags = PRINT_CLOCKS;
        else
        {
            printf("ERROR - Unknown flag %s\n", argv[1]);
            return 0;
        }
        read_file(&memory, argv[2]);
        decode_instruction_stream(&memory, flags);
    }
    else
    {

        if (strcmp(argv[1], "-print_ops"))
        {
            print_inst_table();
            print_clocks_table();
            return 0;
        }

        read_file(&memory, argv[1]);

        printf("; Disassembly of %s\nbits 16\n\n", argv[1]);
        decode_instruction_stream(&memory, flags);
    }

    free_memory(&memory);
    return 0;
}

