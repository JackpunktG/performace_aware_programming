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
} Test;

const Test tests[] =
{
    {"16Kib", 16 * KiB},
    {"32Kib", 32 * KiB},
    {"128Kib", 128 * KiB},
    {"256Kib", 256 * KiB},
    {"512Kib", 512 * KiB},
    {"1MiB", 1 * MiB},
    {"1.5MiB", 1.5 * MiB},
    {"2MiB", 2 * MiB},
    {"3MiB", 3 * MiB},
    {"4MiB", 4 * MiB}
};

uint64_t simulate_work(uint8_t* info, uint64_t index)
{
    uint64_t results0 = 0;

    for (uint64_t i = 0; i < tests[index].size; ++i)
    {
        results0 += info[i];
    }

    return results0;
}

#include <pthread.h>
enum : uint8_t
{
    BUFFER_CALCULATED,
    BUFFER_READ
};

typedef struct
{
    uint8_t* bufferA;
    uint8_t* bufferB;
    int fd;
    uint32_t index;
    uint64_t total_size;
    uint8_t* bufferA_state;
    uint8_t* bufferB_state;
    uint8_t* finish;
    pthread_mutex_t* mutex;
} Thread_Info;

void* read_buffer(void* args)
{
    Thread_Info* info = (Thread_Info*)args;

    uint64_t bytes_read = 0;
    while (bytes_read < info->total_size)
    {
        if (*info->bufferA_state == BUFFER_CALCULATED)
        {
            bytes_read += (uint64_t)read(info->fd, info->bufferA, tests[info->index].size);
            *info->bufferA_state = BUFFER_READ;
        }
        if (*info->bufferB_state == BUFFER_CALCULATED)
        {
            bytes_read += (uint64_t)read(info->fd, info->bufferB, tests[info->index].size);
            *info->bufferB_state = BUFFER_READ;
        }
    }
    *info->finish = 1;
    return NULL;
}


int main(int argc, const char* argv[])
{
    if (argc < 2)
        printf("WARNING: No args. Usage: [*testfile] (if applicable)\n");


    Repetition_tester tester = repetition_tester_init(argv[1], TEST_FILE | TEST_PAGE_FAULTS, 5);
    printf("expected bytes: %lu\n", tester.bytes_expected);

    uint8_t* buffer = mmap(NULL, tester.bytes_expected, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    read(tester.fd, buffer, tester.bytes_expected);
    uint64_t truth = 0;
    for (uint64_t index = 0; index < tester.bytes_expected; ++index)
    {
        truth += buffer[index];
    }

    printf("truth: %lu\n", truth);
    munmap(buffer, tester.bytes_expected);

    lseek(tester.fd, 0, SEEK_SET);



    uint32_t cpu_1;
    uint32_t cpu_2;


    float gbs[array_count(tests)] = {0};

    for (uint32_t i = 0; i < array_count(tests); ++i)
    {
        test_begin(&tester, tests[i].name);
        while(tester.state == TESTER_TESTING)
        {
            const uint64_t start = get_rdtscp(&cpu_1);
            uint8_t* bufferA = mmap(NULL, tests[i].size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
            uint8_t* bufferB = mmap(NULL, tests[i].size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
            uint64_t bytes_read = 0;
            uint64_t sum = 0;

            pthread_t thread1;
            pthread_mutex_t mutex;
            uint8_t bufferA_state = 0;
            uint8_t bufferB_state = 0;
            uint8_t finished = 0;
            Thread_Info info = {bufferA, bufferB, tester.fd, i, tester.bytes_expected, &bufferA_state, &bufferB_state, &finished, &mutex};
            pthread_create(&thread1, NULL, read_buffer, &info);
            pthread_detach(thread1);

            while (!finished)
            {
                if (bufferA_state)
                {
                    sum += simulate_work(bufferA, i);
                    bufferA_state = BUFFER_CALCULATED;

                }

                if (bufferB_state)
                {
                    sum += simulate_work(bufferB, i);
                    bufferB_state = BUFFER_CALCULATED;
                }
            }
            printf("sum: %lu\n", sum);
            const uint64_t end = get_rdtscp(&cpu_2);
            //assert(sum == truth);
            while_testing(&tester, end - start, tester.bytes_expected, cpu_2 == cpu_1);
            munmap(bufferA, tests[i].size);
            munmap(bufferB, tests[i].size);
        }
        gbs[i] = ((float)tester.bytes_expected / (1024*1024*1024))/ ((float)tester.results.min / tester.rdtsc_freq);

        print_results(&tester);
    }

    printf("\n\nchuck size,gbs\n");
    for (uint64_t i = 0; i < array_count(tests); ++i)
        printf("%s,%0.4f\n", tests[i].name, gbs[i]);


    repetition_tester_close(&tester);

    return 0;
}
