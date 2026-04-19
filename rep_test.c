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
    void (*function)(uint8_t* buffer, uint64_t tests_size, uint64_t test_count);
} Test;

Test tests[] =
{
    {"16Kib", 16 * KiB, cache_test_asm},
    {"32Kib", 32 * KiB, cache_test_asm},
    {"33Kib", 33 * KiB, cache_test_asm},
    {"34KiB", 33 * KiB, cache_test_asm},
    {"128KiB", 128 * KiB, cache_test_asm},
    {"512KiB", 512 * KiB, cache_test_asm},
    {"612KiB", 612 * KiB, cache_test_asm},
    {"1MiB", 1 * MiB, cache_test_asm},
    {"2MiB", 2 * MiB, cache_test_asm},
    {"3MiB", 3 * MiB, cache_test_asm},
    {"4MiB", 4 * MiB, cache_test_asm},
    {"5MiB", 5 * MiB, cache_test_asm},
    {"10MiB", 10 * MiB, cache_test_asm},
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
    test_begin(&tester, "L1 test");
    while(tester.state == TESTER_TESTING)
    {
        const uint64_t start = get_rdtscp(&cpu_1);
        mov_256bit_asm(buffer, GiB);
        const uint64_t end = get_rdtscp(&cpu_2);
        while_testing(&tester, end - start, GiB, cpu_2 == cpu_1);
    }
    print_results(&tester);
    for (uint64_t i = 0; i < array_count(tests); ++i)
    {
        const uint64_t loop_count =  GiB / (tests[i].size);
        tester.bytes_expected = loop_count * tests[i].size;
        printf("count: %lu, bytes: %lu\n", loop_count, tester.bytes_expected);
        test_begin(&tester, tests[i].name);
        while(tester.state == TESTER_TESTING)
        {
            const uint64_t start = get_rdtscp(&cpu_1);
            tests[i].function(buffer, tests[i].size, loop_count);
            const uint64_t end = get_rdtscp(&cpu_2);
            while_testing(&tester, end - start, tester.bytes_expected, cpu_2 == cpu_1);
        }
        print_results(&tester);
    }
    uint64_t bytes[20] = {0};
    float gbs[20] = {0};
    munmap(buffer,GiB);

    printf("\n\nbytes,gb/s\n");
    for (uint64_t i= 0; i< 20; ++i)
        printf("%lu,%0.4f\n", bytes[i], gbs[i]);
    repetition_tester_close(&tester);

    return 0;
}
