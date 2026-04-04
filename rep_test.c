#include "repetition_tester.h"


int main(int argc, const char* argv[])
{
    Repetition_tester tester = repetition_tester_init("test.json");

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
