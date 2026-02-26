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

    read_file(&memory, argv[1]);

    decode_instruction_stream(&memory);

    free_memory(&memory);
    return 0;
}

