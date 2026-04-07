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
    uint64_t page_faults_min, page_faults_total;
} Repetition_result;

typedef struct
{
    FILE* fp;
    int fd;
    uint64_t bytes_expected;
    uint64_t rdtsc_freq;
    uint64_t rdtsc_elapsed;
    uint8_t state;
    char test_string[17];
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
#define TEST_SECONDS 2

Repetition_tester repetition_tester_init(const char* file_path)
{
    Repetition_tester tester = {0};
    tester.fp                = fopen(file_path, "rb");
    tester.fd                = open(file_path, O_RDONLY);

    if(tester.fp && tester.fp)
    {
        struct stat file_info;
        stat(file_path, &file_info);

        tester.bytes_expected = file_info.st_size;
        tester.rdtsc_freq     = test_rdtsc_frequency(1000);
        tester.dest_buffer    = (uint8_t*)malloc(sizeof(uint8_t)*tester.bytes_expected);
        tester.state          = TESTER_READY;
    }
    else
        fprintf(stderr, "ERROR: unable to open file %s\n", file_path);

    return tester;
}

void repetition_tester_close(Repetition_tester* tester)
{
    fclose(tester->fp);
    close(tester->fd);

    free(tester->dest_buffer);
    tester->dest_buffer = NULL;
}
void test_begin(Repetition_tester* tester, const char* test_string)
{
    memset(&tester->results, 0, sizeof(Repetition_result));
    strncpy(tester->test_string, test_string, 17);
    rewind(tester->fp);

    tester->state = TESTER_TESTING;
}

void while_testing(Repetition_tester* tester, const uint64_t rdtsc_test_ticks, const uint64_t bytes_read, const uint64_t page_faults)
{


    if (tester->state == TESTER_TESTING)
    {
        if (bytes_read != tester->bytes_expected)
        {
            printf("ERROR: incorrect bytes read, bytes_read: %lu - bytes_expected: %lu\n", bytes_read, tester->bytes_expected);
            tester->state = TESTER_ERROR;
            return;
        }

        //printf("rdtsc_test_ticks %lu\n", rdtsc_test_ticks);
        Repetition_result* results = &tester->results;
        tester->rdtsc_elapsed      += rdtsc_test_ticks;

        if (rdtsc_test_ticks < results->min || results->min == 0)
        {
            results->min             = rdtsc_test_ticks;
            results->page_faults_min = page_faults;
            tester->rdtsc_elapsed    = 0; //reset the timer

            printf("\r%-80s", "");   // overwrite with spaces
            printf("\r\t%s, min: %lu - ~%0.4fsec w/ page_faults: %lu", tester->test_string, tester->results.min, (float)tester->results.min / tester->rdtsc_freq, tester->results.page_faults_min);
            fflush(stdout);
        }
        if (rdtsc_test_ticks > results->max)
            results->max = rdtsc_test_ticks;

        results->total             += rdtsc_test_ticks;
        results->page_faults_total += page_faults;
        ++results->count;


        if (tester->rdtsc_elapsed > tester->rdtsc_freq * TEST_SECONDS)
            tester->state = TESTER_FINISHED;

        rewind(tester->fp);
        lseek(tester->fd, 0, SEEK_SET);
    }
}

void print_results(Repetition_tester* tester)
{
    if (tester->state != TESTER_FINISHED)
    {
        printf("ERROR: test wasn't properly finished\n");
        return;
    }

    printf("\r%-80s\r", "");   // overwrite with spaces
    printf("\t%s, min: %lu - ~%0.4fsec w/ page_faults: %lu, max: %lu - ~%0.4fsec", tester->test_string, tester->results.min, (float)tester->results.min / tester->rdtsc_freq, tester->results.page_faults_min, tester->results.max, (float)tester->results.max / tester->rdtsc_freq);
    printf("\n\t\ttotal: %lu, count: %lu, avg: %.4lf, total page faults: %lu, size: %lu\n\n",tester->results.total, tester->results.count, (double)tester->results.total / tester->results.count, tester->results.page_faults_total, tester->bytes_expected);
}
