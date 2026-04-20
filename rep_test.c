#include "repetition_tester.h"
#include <sys/mman.h>
#include "direct_asm/direct_asm.h"
#include <time.h>
#include <math.h>

#include <assert.h>

#define array_count(arr) sizeof(arr) / sizeof(arr)[0]

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

#define GiB 1024*1024*1024
#define MiB 1024*1024
#define KiB 1024
typedef struct
{
    char* name;
    uint64_t size;
    void (*function)(uint8_t* buffer, uint64_t inner_loop, uint64_t outer_loop, uint64_t pt_adance);
} Test;

Test tests[] =
{
    {"16Kib", 16 * KiB, cache_sets_test_asm},
    {"32Kib", 32 * KiB, cache_sets_test_asm},
    {"256KiB", 256 * KiB, cache_sets_test_asm},
    {"2MiB", 2 * MiB, cache_sets_test_asm},
    {"10MiB", 10 * MiB, cache_sets_test_asm},
};


int main(int argc, const char* argv[])
{
    if (argc < 2)
        printf("WARNING: No args. Usage: [*testfile] (if applicable)\n");


    Repetition_tester tester = repetition_tester_init(argv[1], 0, 5);
    tester.bytes_expected = GiB;
    uint8_t* buffer = mmap(NULL, GiB, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, 0, 0);
    uint32_t cpu_1;
    uint32_t cpu_2;


    uint8_t* to_copy = mmap(NULL, 64, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, 0,0);
    for (uint64_t i = 0; i < 64; ++i)
        to_copy[i] = i;

    for (uint8_t i= 0; i < 64; ++i)
        printf("%hhu,", buffer[320 + i]);
    printf("\n");

    test_begin(&tester, "movnt");
    while(tester.state == TESTER_TESTING)
    {
        const uint64_t start = get_rdtscp(&cpu_1);
        save_multi_nt_asm(buffer, to_copy, 16777216);
        const uint64_t end = get_rdtscp(&cpu_2);
        while_testing(&tester, end - start, tester.bytes_expected, cpu_2 == cpu_1);
    }
    print_results(&tester);

    for (uint8_t i= 0; i < 64; ++i)
        printf("%hhu,", buffer[320 + i]);
    printf("\n");

    memset(buffer, 0, GiB);

    test_begin(&tester, "mov");
    while(tester.state == TESTER_TESTING)
    {
        const uint64_t start = get_rdtscp(&cpu_1);
        save_multi_asm(buffer, to_copy, 16777216);
        const uint64_t end = get_rdtscp(&cpu_2);
        while_testing(&tester, end - start, tester.bytes_expected, cpu_2 == cpu_1);
    }
    print_results(&tester);

    munmap(buffer,GiB);

    repetition_tester_close(&tester);

    return 0;
}
