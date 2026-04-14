#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <sys/resource.h>

#define true  1
#define false 0

enum : uint8_t
{
    TESTER_ERROR,
    TESTER_READY,
    TESTER_TESTING,
    TESTER_FINISHED
};

typedef struct
{
    uint64_t min, max, total, count;
    uint64_t page_faults_min, page_faults_max, page_faults_last, page_faults_total;
    uint64_t majflt_min, majflt_max, majflt_last, majflt_total;
} Repetition_result;

typedef struct
{
    FILE* fp;
    int fd;
    uint64_t flags;
    uint64_t bytes_expected;
    uint64_t rdtsc_freq;
    uint64_t rdtsc_elapsed;
    uint64_t not_same_core;
    uint8_t state;
    uint8_t trying_time;
    char test_string[16];
    uint8_t* dest_buffer;
    Repetition_result results;
} Repetition_tester;

#define OS_TIMER_FREQ (uint64_t)1000000
static inline uint64_t os_timer()
{
    struct timeval Value;
    gettimeofday(&Value, 0);

    return (OS_TIMER_FREQ*(uint64_t)Value.tv_sec) + (uint64_t)Value.tv_usec;
}
static inline uint64_t get_rdtsc()
{
    return __rdtsc();
}
static inline uint64_t get_rdtscp(uint32_t* id)
{
    return __rdtscp(id);
}
static uint64_t test_rdtsc_frequency(uint32_t milli_sec)
{
    uint64_t waiting_time = milli_sec * (OS_TIMER_FREQ / 1000);


    uint64_t os_start = os_timer();
    uint64_t rdtsc_start = get_rdtsc();
    uint64_t os_end = 0;
    uint64_t os_elapsed = 0;
    while(os_elapsed < waiting_time)
    {
        os_end = os_timer();
        os_elapsed = os_end - os_start;
    }

    uint64_t rdtsc_end = get_rdtsc();

    uint64_t rdtsc_freq;
    if(os_elapsed)
    {
        rdtsc_freq = OS_TIMER_FREQ * (rdtsc_end - rdtsc_start) / os_elapsed;
    }
    return rdtsc_freq;
}
#define TEST_FILE        (1 << 0)
#define TEST_PAGE_FAULTS (1 << 1)
#define TEST_SELF_BUFFER (1 << 2) // will alway set GB

Repetition_tester repetition_tester_init(const char* file_path, const uint64_t flags, const uint8_t time_trying)
{
    Repetition_tester tester = {0};
    tester.rdtsc_freq        = test_rdtsc_frequency(1000);
    tester.flags             = flags;
    tester.trying_time       = time_trying;
    if (flags & TEST_FILE)
    {
        if (file_path == NULL)
        {
            printf("ERROR: no filepath given\n");
            return tester;
        }
        else
        {
            tester.fp = fopen(file_path, "rb");
            tester.fd = open(file_path, O_RDONLY);

            if(tester.fp && tester.fp)
            {
                struct stat file_info;
                stat(file_path, &file_info);
                tester.bytes_expected = file_info.st_size;

                tester.dest_buffer = (uint8_t*)malloc(sizeof(uint8_t)*tester.bytes_expected);
            }
            else
            {
                fprintf(stderr, "ERROR: unable to open file %s\n", file_path);
                return tester;
            }
        }
    }
    else if (flags & TEST_SELF_BUFFER)
    {
        tester.bytes_expected = 1024*1024*1024;
        tester.dest_buffer = (uint8_t*)malloc(sizeof(uint8_t)*tester.bytes_expected);
    }

    tester.state = TESTER_READY;
    printf("estimated RDTSC frequency: %lu\n\n", tester.rdtsc_freq);

    return tester;
}

void repetition_tester_close(Repetition_tester* tester)
{
    fclose(tester->fp);
    close(tester->fd);

    if (tester->flags & TEST_SELF_BUFFER || tester->flags & TEST_FILE)
        free(tester->dest_buffer);
    tester->dest_buffer = NULL;
}
void test_begin(Repetition_tester* tester, const char* test_string)
{
    if (tester->state == TESTER_ERROR)
        return;

    memset(&tester->results, 0, sizeof(Repetition_result));
    strncpy(tester->test_string, test_string, 15);
    if (tester->fp != 0)
        rewind(tester->fp);

    tester->not_same_core = 0;
    if (tester->flags & TEST_PAGE_FAULTS)
    {
        struct rusage info;
        getrusage(RUSAGE_SELF, &info);
        tester->results.page_faults_last = info.ru_minflt;
        tester->results.majflt_last = info.ru_majflt;
    }

    tester->state = TESTER_TESTING;
}

void while_testing(Repetition_tester* tester, const uint64_t rdtsc_test_ticks, const uint64_t bytes_read, uint8_t same_core)
{
    if (tester->state == TESTER_TESTING)
    {
        // disregade tests where rdtsc came from different cores
        if (!same_core)
        {
            ++tester->not_same_core;
            if (tester->fp != 0)
            {
                rewind(tester->fp);
                lseek(tester->fd, 0, SEEK_SET);
            }
            if (tester->rdtsc_elapsed > tester->rdtsc_freq * tester->trying_time && tester->results.count > 0)
                tester->state = TESTER_FINISHED;
            return;
        }

        if (bytes_read != tester->bytes_expected && tester->flags & TEST_FILE)
        {
            printf("ERROR: incorrect bytes read, bytes_read: %lu - bytes_expected: %lu\n", bytes_read, tester->bytes_expected);
            tester->state = TESTER_ERROR;
            return;
        }

        Repetition_result* results = &tester->results;
        tester->rdtsc_elapsed      += rdtsc_test_ticks;

        uint32_t run_faults = 0;
        uint32_t run_majflt = 0;
        if (tester->flags & TEST_PAGE_FAULTS)
        {
            struct rusage info;
            getrusage(RUSAGE_SELF, &info);
            run_faults = info.ru_minflt - results->page_faults_last;
            run_majflt = info.ru_majflt - results->majflt_last;
            results->page_faults_total += run_faults;
            results->majflt_total      += run_majflt;

            results->majflt_last      = info.ru_majflt;
            results->page_faults_last = info.ru_minflt;
        }

        if (rdtsc_test_ticks < results->min || results->min == 0)
        {
            results->min             = rdtsc_test_ticks;
            results->page_faults_min = run_faults;
            results->majflt_min      = run_majflt;
            tester->rdtsc_elapsed    = 0; //reset the timer

            printf("\r%-100s", "");
            float bandwidth = ((float)bytes_read / (1024*1024*1024))/ ((float)tester->results.min / tester->rdtsc_freq);
            printf("\r\t%s, min: %lu - ~%0.4fsec %0.4fGb/s, %s", tester->test_string, tester->results.min, (float)tester->results.min / tester->rdtsc_freq,
                   bandwidth,  tester->flags & TEST_PAGE_FAULTS ? "page_faults: " : "");
            if (tester->flags & TEST_PAGE_FAULTS)
                printf("%u, maj: %u", run_faults, run_majflt);
            fflush(stdout);
        }
        if (rdtsc_test_ticks > results->max)
        {
            results->max             = rdtsc_test_ticks;
            results->majflt_max      = run_majflt;
            results->page_faults_max = run_faults;
        }

        results->total += rdtsc_test_ticks;
        ++results->count;


        if (tester->rdtsc_elapsed > tester->rdtsc_freq * tester->trying_time)
            tester->state = TESTER_FINISHED;

        if (tester->fp != 0)
        {
            rewind(tester->fp);
            lseek(tester->fd, 0, SEEK_SET);
        }
    }
}

void print_results(Repetition_tester* tester)
{
    if (tester->state != TESTER_FINISHED)
    {
        printf("ERROR: test wasn't properly finished\n");
        return;
    }
    printf("\n\t\tmax: %lu - ~%0.4fsec", tester->results.max, (float)tester->results.max / tester->rdtsc_freq);
    if (tester->flags & TEST_PAGE_FAULTS)
        printf(", page_faults: %lu, maj: %lu", tester->results.page_faults_max, tester->results.majflt_max);
    printf("\n\t\t\ttotal: %lu, count: %lu, avg: %.4lf, size: %lu",tester->results.total, tester->results.count, (double)tester->results.total / tester->results.count, tester->bytes_expected);
    if (tester->flags & TEST_PAGE_FAULTS)
        printf("\n\t\t\tpage_faults total: %lu, maj: %lu", tester->results.page_faults_total, tester->results.majflt_total);
    if (tester->not_same_core)
        printf(", test failed to run %lu time",tester->not_same_core);
    printf("\n\n");
}
