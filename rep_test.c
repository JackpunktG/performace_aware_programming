#include "repetition_tester.h"
#include <sys/mman.h>
#include "direct_asm/direct_asm.h"
#include <time.h>

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

typedef struct
{
    char* name;
    void (*function)(uint8_t* buffer, uint64_t amount);
} Test;

Test tests[] =
{
    {"8_bit", mov_8bit_asm},
    {"16_bit", mov_16bit_asm},
    {"32_bit", mov_32bit_asm},
    {"64_bit", mov_64bit_asm},
    {"128_bit", mov_128bit_asm},
    {"256_bit", mov_256bit_asm},
};

#define GB 1024*1024*1028

int main(int argc, const char* argv[])
{
    if (argc < 2)
        printf("WARNING: No args. Usage: [*testfile] (if applicable)\n");


    Repetition_tester tester = repetition_tester_init(argv[1], 0, 10);
    tester.bytes_expected = GB;
    uint8_t* buffer = mmap(NULL, 256 * 2, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, 0, 0);
    uint32_t cpu_1;
    uint32_t cpu_2;
    for (uint64_t i = 0; i < array_count(tests); ++i)
    {
        test_begin(&tester, tests[i].name);
        while(tester.state == TESTER_TESTING)
        {
            const uint64_t start = get_rdtscp(&cpu_1);
            tests[i].function(buffer, GB);
            const uint64_t end = get_rdtscp(&cpu_2);
            while_testing(&tester, end - start, GB, cpu_2 == cpu_1);
        }
        print_results(&tester);
    }
    munmap(buffer, 256*2);
    repetition_tester_close(&tester);

    return 0;
}
