#include "8086_sim_inst.h"


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

    read_file(&memory, argv[1]);

    printf("; Disassembly of %s\nbits 16\n\n", argv[1]);
    decode_instruction_stream(&memory);

    free_memory(&memory);
    return 0;
}

