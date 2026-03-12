#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>
#include <immintrin.h>
#include <pthread.h>



/* PC-Specs */
#define MAX_CORES 4
#define THREAD_MAX 8
/* Sizes in bytes */
#define L1_CACHE 131072
#define L2_CACHE 2097152
#define L3_CACHE 4194304 // shared
#define CPU_MAX_MHZ 4386.4722

uint32_t basic_for_loop_summation_iplusplus(uint32_t* arr, uint32_t count)
{
    uint32_t sum = 0;
    for(uint32_t i = 0; i < count; i++)
    {
        sum += arr[i];
    }
    return sum;
}

uint32_t basic_for_loop_summation_plusplusi(uint32_t* arr, uint32_t count)
{
    uint32_t sum = 0;
    for(uint32_t i = 0; i < count; ++i)
    {
        sum += arr[i];
    }
    return sum;
}

uint32_t basic_for_loop_summation_2_unrolled(uint32_t* arr, uint32_t count)
{
    uint32_t sum = 0;
    for(uint32_t i = 0; i < count; i+=2)
    {
        sum += arr[i];
        sum += arr[i + 1];
    }
    return sum;
}

uint32_t basic_for_loop_summation_4_unrolled(uint32_t* arr, uint32_t count)
{
    uint32_t sum = 0;
    for(uint32_t i = 0; i < count; i+=4)
    {
        sum += arr[i];
        sum += arr[i + 1];
        sum += arr[i + 2];
        sum += arr[i + 3];
    }
    return sum;
}
uint32_t basic_for_loop_summation_8_unrolled(uint32_t* arr, uint32_t count)
{
    uint32_t sum = 0;
    for(uint32_t i = 0; i < count; i+=8)
    {
        sum += arr[i];
        sum += arr[i + 1];
        sum += arr[i + 2];
        sum += arr[i + 3];
        sum += arr[i + 4];
        sum += arr[i + 5];
        sum += arr[i + 6];
        sum += arr[i + 7];
    }
    return sum;
}

uint32_t basic_for_loop_summation_16_unrolled(uint32_t* arr, uint32_t count)
{
    uint32_t sum = 0;
    for(uint32_t i = 0; i < count; i+=16)
    {
        sum += arr[i];
        sum += arr[i + 1];
        sum += arr[i + 2];
        sum += arr[i + 3];
        sum += arr[i + 4];
        sum += arr[i + 5];
        sum += arr[i + 6];
        sum += arr[i + 7];
        sum += arr[i + 8];
        sum += arr[i + 9];
        sum += arr[i + 10];
        sum += arr[i + 11];
        sum += arr[i + 12];
        sum += arr[i + 13];
        sum += arr[i + 14];
        sum += arr[i + 15];
    }
    return sum;
}

uint32_t basic_for_loop_summation_32_unrolled(uint32_t* arr, uint32_t count)
{
    uint32_t sum = 0;
    for(uint32_t i = 0; i < count; i+=32)
    {
        sum += arr[i];
        sum += arr[i + 1];
        sum += arr[i + 2];
        sum += arr[i + 3];
        sum += arr[i + 4];
        sum += arr[i + 5];
        sum += arr[i + 6];
        sum += arr[i + 7];
        sum += arr[i + 8];
        sum += arr[i + 9];
        sum += arr[i + 10];
        sum += arr[i + 11];
        sum += arr[i + 12];
        sum += arr[i + 13];
        sum += arr[i + 14];
        sum += arr[i + 15];
        sum += arr[i + 16];
        sum += arr[i + 17];
        sum += arr[i + 18];
        sum += arr[i + 19];
        sum += arr[i + 20];
        sum += arr[i + 21];
        sum += arr[i + 22];
        sum += arr[i + 23];
        sum += arr[i + 24];
        sum += arr[i + 25];
        sum += arr[i + 26];
        sum += arr[i + 27];
        sum += arr[i + 28];
        sum += arr[i + 29];
        sum += arr[i + 30];
        sum += arr[i + 31];
    }
    return sum;
}

uint32_t break_dependancy_summation(uint32_t* arr, uint32_t count)
{
    uint32_t sum1 = 0;
    uint32_t sum2 = 0;
    for(uint32_t i = 0; i < count; i+=2)
    {
        sum1 += arr[i];
        sum2 += arr[i + 1];
    }
    return sum1 + sum2;
}
uint32_t break_dependancy_summation_4_unrolled(uint32_t* arr, uint32_t count)
{
    uint32_t sum1 = 0;
    uint32_t sum2 = 0;
    uint32_t sum3 = 0;
    uint32_t sum4 = 0;
    for(uint32_t i = 0; i < count; i+=4)
    {
        sum1 += arr[i];
        sum2 += arr[i + 1];
        sum3 += arr[i + 2];
        sum4 += arr[i + 3];
    }
    return sum1 + sum2 + sum3 + sum4;
}

uint32_t break_dependancy_summation_8_unrolled(uint32_t* arr, uint32_t count)
{
    uint32_t sum1 = 0;
    uint32_t sum2 = 0;
    uint32_t sum3 = 0;
    uint32_t sum4 = 0;
    uint32_t sum5 = 0;
    uint32_t sum6 = 0;
    uint32_t sum7 = 0;
    uint32_t sum8 = 0;
    for(uint32_t i = 0; i < count; i+=8)
    {
        sum1 += arr[i];
        sum2 += arr[i + 1];
        sum3 += arr[i + 2];
        sum4 += arr[i + 3];
        sum5 += arr[i + 4];
        sum6 += arr[i + 5];
        sum7 += arr[i + 6];
        sum8 += arr[i + 7];
    }
    return sum1 + sum2 + sum3 + sum4 + sum5 + sum6 + sum7 + sum8;
}

uint32_t simd_summation(uint32_t* arr, uint32_t count)
{
    uint32_t sum = 0;
    __m256i vec_sum = _mm256_setzero_si256();
    for(uint32_t i = 0; i < count; i+=8)
    {
        __m256i vec = _mm256_loadu_si256((__m256i*)&arr[i]);
        vec_sum = _mm256_add_epi32(vec_sum, vec);
    }
    // Horizontal add to get the final sum
    __m128i low = _mm256_castsi256_si128(vec_sum);
    __m128i high = _mm256_extracti128_si256(vec_sum, 1);
    low = _mm_add_epi32(low, high);
    low = _mm_hadd_epi32(low, low);
    low = _mm_hadd_epi32(low, low);
    sum = _mm_cvtsi128_si32(low);
    return sum;
}

uint32_t simd_summation_unrolled(uint32_t* arr, uint32_t count)
{
    uint32_t sum = 0;
    __m256i vec_sum1 = _mm256_setzero_si256();
    __m256i vec_sum2 = _mm256_setzero_si256();
    for(uint32_t i = 0; i < count; i+=16)
    {
        __m256i vec1 = _mm256_loadu_si256((__m256i*)&arr[i]);
        __m256i vec2 = _mm256_loadu_si256((__m256i*)&arr[i + 8]);
        vec_sum1 = _mm256_add_epi32(vec_sum1, vec1);
        vec_sum2 = _mm256_add_epi32(vec_sum2, vec2);
    }
    // Horizontal add to get the final sum
    __m128i low1 = _mm256_castsi256_si128(vec_sum1);
    __m128i high1 = _mm256_extracti128_si256(vec_sum1, 1);
    low1 = _mm_add_epi32(low1, high1);
    low1 = _mm_hadd_epi32(low1, low1);
    low1 = _mm_hadd_epi32(low1, low1);

    __m128i low2 = _mm256_castsi256_si128(vec_sum2);
    __m128i high2 = _mm256_extracti128_si256(vec_sum2, 1);
    low2 = _mm_add_epi32(low2, high2);
    low2 = _mm_hadd_epi32(low2, low2);
    low2 = _mm_hadd_epi32(low2, low2);

    sum = _mm_cvtsi128_si32(low1) + _mm_cvtsi128_si32(low2);
    return sum;
}

uint32_t simd_summation_4_unrolled(uint32_t* arr, uint32_t count)
{
    __m256i vec_sum1 = _mm256_setzero_si256();
    __m256i vec_sum2 = _mm256_setzero_si256();
    __m256i vec_sum3 = _mm256_setzero_si256();
    __m256i vec_sum4 = _mm256_setzero_si256();
    for(uint32_t i = 0; i < count; i+=32)
    {
        __m256i vec1 = _mm256_loadu_si256((__m256i*)&arr[i]);
        __m256i vec2 = _mm256_loadu_si256((__m256i*)&arr[i + 8]);
        __m256i vec3 = _mm256_loadu_si256((__m256i*)&arr[i + 16]);
        __m256i vec4 = _mm256_loadu_si256((__m256i*)&arr[i + 24]);
        vec_sum1 = _mm256_add_epi32(vec_sum1, vec1);
        vec_sum2 = _mm256_add_epi32(vec_sum2, vec2);
        vec_sum3 = _mm256_add_epi32(vec_sum3, vec3);
        vec_sum4 = _mm256_add_epi32(vec_sum4, vec4);
    }
    // Horizontal add to get the final sum
    __m128i low1 = _mm256_castsi256_si128(vec_sum1);
    __m128i high1 = _mm256_extracti128_si256(vec_sum1, 1);
    low1 = _mm_add_epi32(low1, high1);
    low1 = _mm_hadd_epi32(low1, low1);
    low1 = _mm_hadd_epi32(low1, low1);

    __m128i low2 = _mm256_castsi256_si128(vec_sum2);
    __m128i high2 = _mm256_extracti128_si256(vec_sum2, 1);
    low2 = _mm_add_epi32(low2, high2);
    low2 = _mm_hadd_epi32(low2, low2);
    low2 = _mm_hadd_epi32(low2, low2);

    __m128i low3 = _mm256_castsi256_si128(vec_sum3);
    __m128i high3 = _mm256_extracti128_si256(vec_sum3, 1);
    low3 = _mm_add_epi32(low3, high3);
    low3 = _mm_hadd_epi32(low3, low3);
    low3 = _mm_hadd_epi32(low3, low3);

    __m128i low4 = _mm256_castsi256_si128(vec_sum4);
    __m128i high4 = _mm256_extracti128_si256(vec_sum4, 1);
    low4 = _mm_add_epi32(low4, high4);
    low4 = _mm_hadd_epi32(low4, low4);
    low4 = _mm_hadd_epi32(low4, low4);

    return _mm_cvtsi128_si32(low1) + _mm_cvtsi128_si32(low2) + _mm_cvtsi128_si32(low3) + _mm_cvtsi128_si32(low4);
}
typedef struct
{
    uint32_t* arr;
    uint32_t count;
} Thread_Data;

void* thread_func(void* data)
{
    Thread_Data* thread_data = (Thread_Data*)data;

    return (void*)(uintptr_t)simd_summation_unrolled(thread_data->arr, thread_data->count);
}

uint32_t multi_threaded_2_summation(uint32_t* arr, uint32_t count)
{
    pthread_t threads;
    Thread_Data thread_data;

    thread_data.arr = arr + (count / 2);
    thread_data.count = count / 2;
    pthread_create(&threads, NULL, thread_func, &thread_data);

    uint32_t sum1 = simd_summation_unrolled(arr, count / 2);

    void* result;
    pthread_join(threads, &result);

    return sum1 + (uint32_t)(uintptr_t)result;
}

uint32_t multi_threaded_4_summation(uint32_t* arr, uint32_t count)
{
    pthread_t threads[3];
    Thread_Data thread_data[3];

    for (int i = 0; i < 3; ++i)
    {
        thread_data[i].arr = arr + ((i + 1) * count / 4);
        thread_data[i].count = count / 4;
        pthread_create(&threads[i], NULL, thread_func, &thread_data[i]);
    }

    uint32_t sum1 = simd_summation_unrolled(arr, count / 4);

    uint32_t sum2 = 0;
    for (int i = 0; i < 3; ++i)
    {
        void* result;
        pthread_join(threads[i], &result);
        sum2 += (uint32_t)(uintptr_t)result;
    }

    return sum1 + sum2;
}

uint32_t multi_threaded_8_summation(uint32_t* arr, uint32_t count)
{
    pthread_t threads[7];
    Thread_Data thread_data[7];

    for (int i = 0; i < 7; ++i)
    {
        thread_data[i].arr = arr + ((i + 1) * count / 8);
        thread_data[i].count = count / 8;
        pthread_create(&threads[i], NULL, thread_func, &thread_data[i]);
    }

    uint32_t sum1 = simd_summation_unrolled(arr, count / 8);

    uint32_t sum2 = 0;
    for (int i = 0; i < 7; ++i)
    {
        void* result;
        pthread_join(threads[i], &result);
        sum2 += (uint32_t)(uintptr_t)result;
    }

    return sum1 + sum2;
}


typedef enum
{
    BASIC_LOOP_SUMMATION,
    BASIC_LOOP_SUMMATION_2_UNROLLED,
    BASIC_LOOP_SUMMATION_4_UNROLLED,
    BASIC_LOOP_SUMMATION_8_UNROLLED,
    BREAK_DEPENDANCY_SUMMATION,
    BREAK_DEPENDANCY_SUMMATION_4_UNROLLED,
    BREAK_DEPENDANCY_SUMMATION_8_UNROLLED,
    SIMD_SUMMATION,
    SIMD_SUMMATION_UNROLLED,
    SIMD_SUMMATION_4_UNROLLED,
    MUTI_THREAD_SUMMATION_2,
    MUTI_THREAD_SUMMATION_4,
    MUTI_THREAD_SUMMATION_8,
    TEST_TOTAL
} Test_Type;

uint64_t test_disbatch(Test_Type type, uint32_t* arr, uint32_t count)
{
    struct timespec before, after;
    clock_gettime(CLOCK_MONOTONIC, &before);
    switch (type)
    {
    case BASIC_LOOP_SUMMATION:
        basic_for_loop_summation_iplusplus(arr, count);
        break;
    case BASIC_LOOP_SUMMATION_2_UNROLLED:
        basic_for_loop_summation_2_unrolled(arr, count);
        break;
    case BASIC_LOOP_SUMMATION_4_UNROLLED:
        basic_for_loop_summation_4_unrolled(arr, count);
        break;
    case BASIC_LOOP_SUMMATION_8_UNROLLED:
        basic_for_loop_summation_8_unrolled(arr, count);
        break;
    case BREAK_DEPENDANCY_SUMMATION:
        break_dependancy_summation(arr, count);
        break;
    case BREAK_DEPENDANCY_SUMMATION_4_UNROLLED:
        break_dependancy_summation_4_unrolled(arr, count);
        break;
    case BREAK_DEPENDANCY_SUMMATION_8_UNROLLED:
        break_dependancy_summation_8_unrolled(arr, count);
        break;
    case SIMD_SUMMATION:
        simd_summation(arr, count);
        break;
    case SIMD_SUMMATION_UNROLLED:
        simd_summation_unrolled(arr, count);
        break;
    case SIMD_SUMMATION_4_UNROLLED:
        simd_summation_4_unrolled(arr, count);
        break;
    case MUTI_THREAD_SUMMATION_2:
        multi_threaded_2_summation(arr, count);
        break;
    case MUTI_THREAD_SUMMATION_4:
        multi_threaded_4_summation(arr, count);
        break;
    case MUTI_THREAD_SUMMATION_8:
        multi_threaded_8_summation(arr, count);
        break;

    default:
        assert(0 && "ERROR - test disbatch unknown\n");
    }
    clock_gettime(CLOCK_MONOTONIC, &after);
    return ((after.tv_sec - before.tv_sec) * 1000000000LL
            + (after.tv_nsec - before.tv_nsec));
}
#define TEST_COUNT 100000

char* test_name_string(Test_Type type)
{
    switch (type)
    {
    case BASIC_LOOP_SUMMATION:
        return "Basic Loop Summation";
    case BASIC_LOOP_SUMMATION_2_UNROLLED:
        return "Basic Loop Summation 2 Unrolled";
    case BASIC_LOOP_SUMMATION_4_UNROLLED:
        return "Basic Loop Summation 4 Unrolled";
    case BASIC_LOOP_SUMMATION_8_UNROLLED:
        return "Basic Loop Summation 8 Unrolled";
    case BREAK_DEPENDANCY_SUMMATION:
        return "Break Dependency Summation";
    case BREAK_DEPENDANCY_SUMMATION_4_UNROLLED:
        return "Break Dependency Summation 4 Unrolled";
    case BREAK_DEPENDANCY_SUMMATION_8_UNROLLED:
        return "Break Dependency Summation 8 Unrolled";
    case SIMD_SUMMATION:
        return "SIMD Summation";
    case SIMD_SUMMATION_UNROLLED:
        return "SIMD Summation Unrolled";
    case SIMD_SUMMATION_4_UNROLLED:
        return "SIMD Summation 4 Unrolled";
    case MUTI_THREAD_SUMMATION_2:
        return "Multi Threaded Summation 2 Threads";
    case MUTI_THREAD_SUMMATION_4:
        return "Multi Threaded Summation 4 Threads";
    case MUTI_THREAD_SUMMATION_8:
        return "Multi Threaded Summation 8 Threads";
    default:
        assert(0 && "ERROR - test name string unknown\n");

    }
}


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Args required\n");
        return -1;
    }

    uint32_t count = atoi(argv[1]);
    uint32_t* arr = (uint32_t*)malloc(sizeof(uint32_t) * count);
    for (uint32_t i = 0; i < count; ++i)
        arr[i] = i +1;

    printf("Running tests with %u elements\nTotal memory used for array: %lu bytes\n\n", count, sizeof(uint32_t) * count);
    printf("Fits in L1 cache: %s\n", (sizeof(uint32_t) * count) <= L1_CACHE ? "Yes" : "No");
    printf("Fits in L2 cache: %s\n", (sizeof(uint32_t) * count) <= L2_CACHE ? "Yes" : "No");
    printf("Fits in L3 cache: %s\n", (sizeof(uint32_t) * count) <= L3_CACHE ? "Yes" : "No");
    printf("--------------------------------------------------\n\n");

    for(uint32_t k = 0; k < TEST_TOTAL; ++k)
    {
        uint64_t quickest_test;
        uint64_t longest_test;
        uint64_t sum_test = 0;

        for (uint32_t i = 0; i < TEST_COUNT; ++i)
        {

            uint64_t test_time = test_disbatch(k, arr, count);
            sum_test += test_time;
            if (i == 0)
            {
                quickest_test = test_time;
                longest_test = test_time;
            }
            else
            {
                if (test_time < quickest_test)
                    quickest_test = test_time;
                else if (test_time > longest_test)
                    longest_test = test_time;
            }
        }
        double avg_test = (double)sum_test / count;


        printf("Test results for: %s\n", test_name_string(k));
        printf("\tQuickest test: %lu ns, ADD per clock cycle: ~%f\n", quickest_test, (double)TEST_COUNT / ((double)quickest_test * ((double)CPU_MAX_MHZ/1000)));
        printf("\tLongest test: %lu ns, ADD per clock cycle: ~%f\n", longest_test, (double)TEST_COUNT / ((double)longest_test * ((double)CPU_MAX_MHZ/1000)));
        printf("\tAverage test: %f ns, ADD per clock cycle: ~%f\n", avg_test, (double)TEST_COUNT / ((double)avg_test * ((double)CPU_MAX_MHZ/1000)));
        printf("--------------------------------------------------\n\n");
    }
    free(arr);

    return 0;
}


