#include "repetition_tester.h"
#include <sys/mman.h>

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
        test_begin(&tester, "read");
        while (tester.state == TESTER_TESTING)
        {
            uint32_t cpu_id;
            uint32_t cpu_id2;
            const uint64_t start = get_rdtscp(&cpu_id);
            const uint64_t bytes = (uint64_t)read(tester.fd, tester.dest_buffer, tester.bytes_expected);
            const uint64_t end   = get_rdtscp(&cpu_id2);
            while_testing(&tester, end-start, bytes, cpu_id == cpu_id2);
        }

        print_results(&tester);

        test_begin(&tester, "self_for_loop");
        while (tester.state == TESTER_TESTING)
        {
            uint32_t cpu_id;
            uint32_t cpu_id2;
            uint8_t* buffer = tester.dest_buffer;
            uint64_t i = 0;
            const uint64_t start = get_rdtscp(&cpu_id);
            for ( ; i < tester.bytes_expected; ++i)
                buffer[i] = (uint8_t)i;
            const uint64_t end   = get_rdtscp(&cpu_id2);
            while_testing(&tester, end-start, i, cpu_id == cpu_id2);
        }

        print_results(&tester);

        test_begin(&tester, "self_while_loop");
        while (tester.state == TESTER_TESTING)
        {
            uint32_t cpu_id;
            uint32_t cpu_id2;
            uint8_t* buffer = tester.dest_buffer;
            uint64_t i = 0;
            const uint64_t start = get_rdtscp(&cpu_id);
            while(i < tester.bytes_expected)
                buffer[i++] = (uint8_t)i;
            const uint64_t end   = get_rdtscp(&cpu_id2);
            while_testing(&tester, end-start, i, cpu_id == cpu_id2);
        }

        print_results(&tester);
        return 0;


        test_begin(&tester, "read_malloc");
        while (tester.state == TESTER_TESTING)
        {
            uint32_t cpu_id;
            uint32_t cpu_id2;
            uint64_t* buffer = (uint64_t*)malloc(tester.bytes_expected);
            const uint64_t start = get_rdtscp(&cpu_id);
            const uint64_t bytes = (uint64_t)read(tester.fd, buffer, tester.bytes_expected);
            const uint64_t end   = get_rdtscp(&cpu_id2);
            free(buffer);
            while_testing(&tester, end-start, bytes, cpu_id == cpu_id2);

        }

        print_results(&tester);

        test_begin(&tester, "read_mmap");
        while (tester.state == TESTER_TESTING)
        {
            uint32_t cpu_id;
            uint32_t cpu_id2;
            uint64_t* buffer = mmap(NULL, tester.bytes_expected, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
            const uint64_t start = get_rdtscp(&cpu_id);
            const uint64_t bytes = (uint64_t)read(tester.fd, buffer, tester.bytes_expected);
            const uint64_t end   = get_rdtscp(&cpu_id2);
            munmap(buffer, tester.bytes_expected);
            while_testing(&tester, end-start, bytes, cpu_id == cpu_id2);

        }

        print_results(&tester);

    }
    repetition_tester_close(&tester);

    return 0;
}
