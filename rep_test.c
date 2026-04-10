#include "repetition_tester.h"
#include <sys/mman.h>
#include "direct_asm/direct_asm.h"

#include <assert.h>

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        printf("ERROR: No args. Usage: [testfile]");
        return 1;
    }



    Repetition_tester tester = repetition_tester_init(argv[1], TEST_PAGE_FAULTS | TEST_FILE, 5);
    {

        test_begin(&tester, "C loop");
        while (tester.state == TESTER_TESTING)
        {
            uint32_t cpu_id;
            uint32_t cpu_id2;
            uint8_t* buffer = tester.dest_buffer;
            const uint64_t size = tester.bytes_expected;
            const uint64_t start = get_rdtscp(&cpu_id);
            for(uint64_t i = 0; i < size; ++i)
                buffer[i] = (uint8_t)i;
            const uint64_t end   = get_rdtscp(&cpu_id2);
            while_testing(&tester, end-start, tester.bytes_expected, cpu_id == cpu_id2);
        }

        print_results(&tester);
        test_begin(&tester, "mov_all");
        while (tester.state == TESTER_TESTING)
        {
            uint32_t cpu_id;
            uint32_t cpu_id2;
            const uint64_t start = get_rdtscp(&cpu_id);
            mov_all_bytes_asm(tester.dest_buffer, tester.bytes_expected);
            const uint64_t end   = get_rdtscp(&cpu_id2);
            while_testing(&tester, end-start, tester.bytes_expected, cpu_id == cpu_id2);
        }

        print_results(&tester);

        test_begin(&tester, "nop_all");
        while (tester.state == TESTER_TESTING)
        {
            uint32_t cpu_id;
            uint32_t cpu_id2;
            const uint64_t start = get_rdtscp(&cpu_id);
            nop_all_bytes_asm(tester.dest_buffer, tester.bytes_expected);
            const uint64_t end   = get_rdtscp(&cpu_id2);
            while_testing(&tester, end-start, tester.bytes_expected, cpu_id == cpu_id2);
        }

        print_results(&tester);
        test_begin(&tester, "cmp_all");
        while (tester.state == TESTER_TESTING)
        {
            uint32_t cpu_id;
            uint32_t cpu_id2;
            const uint64_t start = get_rdtscp(&cpu_id);
            cmp_all_bytes_asm(tester.bytes_expected);
            const uint64_t end   = get_rdtscp(&cpu_id2);
            while_testing(&tester, end-start, tester.bytes_expected, cpu_id == cpu_id2);
        }

        print_results(&tester);
        test_begin(&tester, "dec_all");
        while (tester.state == TESTER_TESTING)
        {
            uint32_t cpu_id;
            uint32_t cpu_id2;
            const uint64_t start = get_rdtscp(&cpu_id);
            dec_all_bytes_asm(tester.bytes_expected);
            const uint64_t end   = get_rdtscp(&cpu_id2);
            while_testing(&tester, end-start, tester.bytes_expected, cpu_id == cpu_id2);
        }

        print_results(&tester);


    }
    repetition_tester_close(&tester);

    return 0;
}
