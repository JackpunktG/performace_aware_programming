#include "repetition_tester.h"


int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        printf("ERROR: No args. Usage: [testfile]");
        return 1;
    }



    Repetition_tester tester = repetition_tester_init(argv[1]);
    struct rusage before;
    struct rusage after;

    while(1)
    {
        test_begin(&tester, "read");
        while (tester.state == TESTER_TESTING)
        {
            getrusage(RUSAGE_SELF, &before);
            const uint64_t start = get_rdtsc();
            const uint64_t bytes = (uint64_t)read(tester.fd, tester.dest_buffer, tester.bytes_expected);
            const uint64_t end   = get_rdtsc();
            getrusage(RUSAGE_SELF, &after);
            //printf("before: %lu, after: %lu\n", before.ru_minflt, after.ru_minflt);
            while_testing(&tester, end-start, bytes, after.ru_minflt - before.ru_minflt);
        }

        print_results(&tester);

        test_begin(&tester, "read_malloc");
        while (tester.state == TESTER_TESTING)
        {
            uint64_t* buffer = (uint64_t*)malloc(tester.bytes_expected);
            getrusage(RUSAGE_SELF, &before);
            const uint64_t start = get_rdtsc();
            const uint64_t bytes = (uint64_t)read(tester.fd, buffer, tester.bytes_expected);
            const uint64_t end   = get_rdtsc();
            getrusage(RUSAGE_SELF, &after);
            while_testing(&tester, end-start, bytes, after.ru_minflt - before.ru_minflt);
            free(buffer);
        }

        print_results(&tester);

        test_begin(&tester, "read_malloc_pre_mapping");
        while (tester.state == TESTER_TESTING)
        {
            uint64_t* buffer = (uint64_t*)malloc(tester.bytes_expected);
            for (uint64_t i = 0; i <  tester.bytes_expected / sizeof(uint64_t); ++i)
                buffer[i] = i;
            getrusage(RUSAGE_SELF, &before);
            const uint64_t start = get_rdtsc();
            const uint64_t bytes = (uint64_t)read(tester.fd, buffer, tester.bytes_expected);
            const uint64_t end   = get_rdtsc();
            getrusage(RUSAGE_SELF, &after);
            while_testing(&tester, end-start, bytes, after.ru_minflt - before.ru_minflt);
            free(buffer);
        }

        print_results(&tester);

        test_begin(&tester, "fread");
        while (tester.state == TESTER_TESTING)
        {
            getrusage(RUSAGE_SELF, &before);
            const uint64_t start = get_rdtsc();
            const uint8_t read   = fread(tester.dest_buffer, tester.bytes_expected, 1, tester.fp);
            const uint64_t end   = get_rdtsc();
            getrusage(RUSAGE_SELF, &after);
            while_testing(&tester, end-start, read * tester.bytes_expected, after.ru_minflt - before.ru_minflt);
        }
        print_results(&tester);

        test_begin(&tester, "fread_malloc");
        while (tester.state == TESTER_TESTING)
        {
            uint64_t* buffer = (uint64_t*)malloc(tester.bytes_expected);
            getrusage(RUSAGE_SELF, &before);
            const uint64_t start = get_rdtsc();
            const uint8_t read   = fread(buffer, tester.bytes_expected, 1, tester.fp);
            const uint64_t end   = get_rdtsc();
            getrusage(RUSAGE_SELF, &after);
            while_testing(&tester, end-start, read * tester.bytes_expected, after.ru_minflt - before.ru_minflt);
            free(buffer);
        }
        print_results(&tester);

        test_begin(&tester, "fread_malloc_pre_mapping");
        while (tester.state == TESTER_TESTING)
        {
            uint64_t* buffer = (uint64_t*)malloc(tester.bytes_expected);
            for (uint64_t i = 0; i <  tester.bytes_expected / sizeof(uint64_t); ++i)
                buffer[i] = i;
            getrusage(RUSAGE_SELF, &before);
            const uint64_t start = get_rdtsc();
            const uint8_t read   = fread(buffer, tester.bytes_expected, 1, tester.fp);
            const uint64_t end   = get_rdtsc();
            getrusage(RUSAGE_SELF, &after);
            while_testing(&tester, end-start, read * tester.bytes_expected, after.ru_minflt - before.ru_minflt);
            free(buffer);
        }
        print_results(&tester);
    }


    repetition_tester_close(&tester);

    return 0;
}
