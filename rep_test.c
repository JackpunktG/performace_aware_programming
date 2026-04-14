#include "repetition_tester.h"
#include <sys/mman.h>
#include "direct_asm/direct_asm.h"
#include <time.h>

#include <assert.h>


const char* tests[] =
{"all 0", "all 1", "2nd", "3rd", "4th", "random"};

enum : uint8_t
{
    ALL_0,
    ALL_1,
    EVERY_2,
    EVERY_3,
    EVERY_4,
    RANDOM,


    TEST_COUNT
};

void fill_buffer(uint8_t* buffer, uint64_t size, uint8_t type)
{
    switch (type)
    {
    case ALL_0:
    case ALL_1:
        memset(buffer, type == ALL_1 ? 1 : 0, sizeof(uint8_t) * size);
        break;
    case EVERY_2:
    case EVERY_3:
    case EVERY_4:
        for (uint64_t i = 0; i < size; ++i)
        {
            if (type == EVERY_2 && i % 2 == 0)
                buffer[i] = 1;
            else if (type == EVERY_3 && i % 3 == 0)
                buffer[i] = 1;
            else if (type == EVERY_4 && i % 4 == 0)
                buffer[i] = 1;
            else
                buffer[i] = 0;
        }
        break;
    case RANDOM:
        srand(time(NULL));
        for (uint64_t i = 0; i < size; ++i)
        {
            buffer[i] = (uint8_t)rand();
            //printf("%hhu\n",buffer[i]);
        }
        break;
    default:
        assert(0 && "ERROR\n");
    }
}


int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        printf("ERROR: No args. Usage: [testfile]");
        return 1;
    }


    Repetition_tester tester = repetition_tester_init(argv[1], TEST_SELF_BUFFER, 5);
    {

        // for (uint8_t i = 5; i < TEST_COUNT; ++i)
        // {
        //     fill_buffer(tester.dest_buffer, tester.bytes_expected, i);
        //     test_begin(&tester, tests[i]);
        //     while (tester.state == TESTER_TESTING)
        //     {
        //         uint32_t cpu_id;
        //         uint32_t cpu_id2;
        //         const uint64_t start = get_rdtscp(&cpu_id);
        //         plus63_aligned_asm(tester.dest_buffer, tester.bytes_expected);
        //         const uint64_t end   = get_rdtscp(&cpu_id2);
        //         while_testing(&tester, end-start, tester.bytes_expected, cpu_id == cpu_id2);
        //     }
        //     print_results(&tester);
        // }

        const uint64_t size = 1024 * 1024 *1024;

        {
            test_begin(&tester, "aligned");
            uint32_t cpu_id;
            uint32_t cpu_id2;
            while(tester.state == TESTER_TESTING)
            {
                const uint64_t start = get_rdtscp(&cpu_id);
                aligned_asm(size);
                const uint64_t end   = get_rdtscp(&cpu_id2);
                while_testing(&tester, end-start, tester.bytes_expected, cpu_id == cpu_id2);
            }
            print_results(&tester);
        }
        {
            test_begin(&tester, "1 plus");
            uint32_t cpu_id;
            uint32_t cpu_id2;
            while(tester.state == TESTER_TESTING)
            {
                const uint64_t start = get_rdtscp(&cpu_id);
                plus1_aligned_asm(size);
                const uint64_t end   = get_rdtscp(&cpu_id2);
                while_testing(&tester, end-start, tester.bytes_expected, cpu_id == cpu_id2);
            }
            print_results(&tester);
        }
        {
            test_begin(&tester, "15 plus");
            uint32_t cpu_id;
            uint32_t cpu_id2;
            while(tester.state == TESTER_TESTING)
            {
                const uint64_t start = get_rdtscp(&cpu_id);
                plus15_aligned_asm(size);
                const uint64_t end   = get_rdtscp(&cpu_id2);
                while_testing(&tester, end-start, tester.bytes_expected, cpu_id == cpu_id2);
            }
            print_results(&tester);
        }
        {
            test_begin(&tester, "31 plus");
            uint32_t cpu_id;
            uint32_t cpu_id2;
            while(tester.state == TESTER_TESTING)
            {
                const uint64_t start = get_rdtscp(&cpu_id);
                plus31_aligned_asm(size);
                const uint64_t end   = get_rdtscp(&cpu_id2);
                while_testing(&tester, end-start, tester.bytes_expected, cpu_id == cpu_id2);
            }
            print_results(&tester);
        }
        {
            test_begin(&tester, "63 plus");
            uint32_t cpu_id;
            uint32_t cpu_id2;
            while(tester.state == TESTER_TESTING)
            {
                const uint64_t start = get_rdtscp(&cpu_id);
                plus63_aligned_asm(size);
                const uint64_t end   = get_rdtscp(&cpu_id2);
                while_testing(&tester, end-start, tester.bytes_expected, cpu_id == cpu_id2);
            }
            print_results(&tester);
        }
    }
    repetition_tester_close(&tester);

    return 0;
}
