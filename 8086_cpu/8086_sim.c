#include "8086_decode.h"


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Args = file path\n");
        return 0;
    }

    //print_inst_table();
    Memory memory = {0};

    //print_inst_table();

    uint32_t flags = 0;
    if (argc > 2)
    {

        if (strcmp(argv[1], "-exec") == 0)
            flags |= EXECUTION_OF_INSTRUCTION;
        read_file(&memory, argv[2]);
        decode_instruction_stream(&memory, flags);
    }
    else
    {
        read_file(&memory, argv[1]);

        printf("; Disassembly of %s\nbits 16\n\n", argv[1]);
        decode_instruction_stream(&memory, flags);
    }

    free_memory(&memory);
    return 0;
}

