#include "repetition_tester.h"


int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        printf("ERROR: No args. Usage: [testfile]");
        return 1;
    }



    Repetition_tester tester = repetition_tester_init(argv[1]);



    test_begin(&tester, "read");
    while (tester.state == TESTER_TESTING)
    {
        const uint64_t start = get_rdtsc();
        const uint64_t bytes = (uint64_t)read(tester.fd, tester.dest_buffer, tester.bytes_expected);
        const uint64_t end   = get_rdtsc();
        while_testing(&tester, end-start, bytes);
    }

    print_results(&tester);

    test_begin(&tester, "fread");
    while (tester.state == TESTER_TESTING)
    {
        const uint64_t start = get_rdtsc();
        const uint8_t read   = fread(tester.dest_buffer, tester.bytes_expected, 1, tester.fp);
        const uint64_t end   = get_rdtsc();
        while_testing(&tester, end-start, read * tester.bytes_expected);
    }

    print_results(&tester);
    repetition_tester_close(&tester);

    return 0;
}
