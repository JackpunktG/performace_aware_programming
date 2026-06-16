#ifndef MATH_LIB
#define MATH_LIB
#pragma once
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>
#include <sys/time.h>
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t  i8;

typedef float  f32;
typedef double f64;

typedef uint8_t b8;


#define array_count(arr) (sizeof(arr) / sizeof(arr[0]))

typedef enum
{
    Cos,
    Sin,
    Asin,
    Sqrt,
    Tests_Count
} Func_Type;

const char* test_names[] =
{
    "Cos",
    "Sin",
    "Asin",
    "Sqrt"
};

typedef f64 Math_func(f64);

typedef struct
{
    f64 min, max;
    Math_func* func;
} Truth_Function;

const Truth_Function truth[] =
{
    {-1.57079632679f, 1.57079632679f, cos},
    {-3.14159265359f, 3.14159265359f, sin},
    {0, 1, asin},
    {0, 1, sqrt}
};

typedef struct
{
    Math_func* func;
    Func_Type type;
    const char* desc;
} Funtions;

/* Testing Harnes for own library */

#define TOLLERANCE 0.000001f
static inline b8 almost_equal(f64 a, f64 b)
{
    return b >= (a - TOLLERANCE) && b <= (a + TOLLERANCE);
}

static inline b8 test_function(f64 (*test_func)(f64), f64 (*truth_func)(f64), f64 value)
{
    return almost_equal(test_func(value), truth_func(value));
}

static inline f64 difference_val(f64 a, f64 b)
{
    if (a > b)
        return a - b;
    else
        return b - a;
}

#define TEST_AMOUNT 100000000.0f
void test_math_h(Funtions* tests, u64 count)
{
    for (u8 i = 0; i < count; ++i)
    {
        printf("Testing accuracy of function %s with %s\n", tests[i].desc, test_names[tests[i].type]);
        f64 difference = 0, where;
        u64 count = 0;

        f64 inc = (truth[tests[i].type].max - truth[tests[i].type].min) / TEST_AMOUNT;
        f64 val = truth[tests[i].type].min;

        for (u64 k = 0; k < TEST_AMOUNT; ++k)
        {
            if (!almost_equal(truth[tests[i].type].func(val), tests[i].func(val)))
            {
                if (difference_val(truth[tests[i].type].func(val), tests[i].func(val)) > difference)
                {
                    difference = difference_val(truth[tests[i].type].func(val), tests[i].func(val));
                    where = val;
                }
                ++count;
            }
            val += inc;
        }
        if (count)
        {
            printf("discrepancy found!\n");
            printf("\tbiggests differnce %f, at input %f\n", difference, where);
            printf("\ttruth: %f, output: %f\n", truth[tests[i].type].func(where), tests[i].func(where));
        }
        else
        {
            printf("no discrepancy found!\n");
        }
        printf("\n");
    }
}

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
void test_speed(Funtions* tests, u64 count, u64 amount)
{
    const uint64_t rdtsc_freq_est = test_rdtsc_frequency(500);
    for (u8 i = 0; i < count; ++i)
    {
        printf("Testing speed of function %s with %s, count %lu\n", tests[i].desc, test_names[tests[i].type], amount);
        u8 type = tests[i].type;
        f64 res = 0;
        u64 r1 = __rdtsc();
        for (u64 k = 0; k < amount; ++k)
            res += truth[type].func(k);
        u64 r2 = __rdtsc();

        printf("\ttruth function: %0.4f - res %f\n", (f64)(r2-r1)/rdtsc_freq_est, res);
        res = 0;

        r1 = __rdtsc();
        for (u64 k = 0; k < amount; ++k)
            res += tests[i].func(i);

        r2 = __rdtsc();

        printf("\t%s function: %0.4f - res %f\n\n", tests[i].desc, (f64)(r2-r1)/rdtsc_freq_est, res);
    }

}

/* Own Sin */
#define CONST_A_NEG 0.405284734569f
#define CONST_A_POS -0.405284734569f
#define CONST_B     1.27323954474f



f64 sin_ce_polynomial_approx(f64 input) // very rough approximation based on parabola
{
    __m128d packed = _mm_set_pd(input, input*input);
    packed = _mm_mul_pd(packed, _mm_set_pd(CONST_B, input > 0 ? CONST_A_POS : CONST_A_NEG));
    packed = _mm_hadd_pd(packed, packed);

    return _mm_cvtsd_f64(packed);
}

f64 sin_ce_polynomial_cubic(f64 input) // very rough approximation based on x^3
{
    if (input > M_PI*0.5)
        input = M_PI - input;
    else if (input < -(M_PI*0.5))
        input = -(M_PI + input);
    __m128d packed = _mm_set_pd(input, input*input*input);
    packed = _mm_mul_pd(packed, _mm_set_pd(0.985, -0.141999));
    packed = _mm_hadd_pd(packed, packed);

    return _mm_cvtsd_f64(packed);
}

f64 cos_ce_polynomal_cubic(f64 input)
{
    return sin_ce_polynomial_cubic(input += M_PI*0.5);
}

/* Own Sqrt */

f64 sqrt_ce(f64 input)
{
    __m128d x = _mm_set_sd(input);
    x =_mm_sqrt_pd(x);

    return _mm_cvtsd_f64(x);
}

f64 sqrt_ce_compact(f64 input)
{
    return _mm_cvtsd_f64(_mm_sqrt_pd(_mm_set_sd(input)));
}

/* Taylor Series */
f64 power_of(f64 x, u64 power)
{
    f64 res = x;
    for (u64 i = 1; i < power; ++i)
        res *= x;

    return res;
}

u64 factorial(u64 x)
{
    if (x == 0)
        return 0;

    if (x == 1)
        return 1;

    return x*factorial(x-1);
}

f64 taylor_series_sin(f64 x, u64 length)
{
    if (length == 0)
    {
        printf("WARNING taylor_series_sin len = 0, defualting to 1\n");
        length = 1;
    }

    u64 exponent = 1;
    f64 res      = 0;
    b8 plus      = 1;
    while (exponent <= length)
    {
        if (plus)
        {
            res += (power_of(x, exponent)/factorial(exponent));
            plus = 0;
        }
        else
        {
            res -= (power_of(x, exponent)/factorial(exponent));
            plus = 1;
        }
        exponent += 2;
    }
    return res;
}

void test_taylor_series()
{
    for (u8 i = 1; i < 20; ++i)
    {
        printf("Testing accuracy of taylor_series with %hhu exponent\n", i);
        f64 difference = 0, where;
        u64 count = 0;

        f64 inc = (truth[Sin].max - truth[Sin].min) / TEST_AMOUNT;
        f64 val = truth[Sin].min;

        for (u64 k = 0; k < TEST_AMOUNT; ++k)
        {
            if (!almost_equal(truth[Sin].func(val), taylor_series_sin(val, i)))
            {
                if (difference_val(truth[Sin].func(val), taylor_series_sin(val, i)) > difference)
                {
                    difference = difference_val(truth[Sin].func(val), taylor_series_sin(val, i));
                    where = val;
                }
                ++count;
            }
            val += inc;
        }
        if (count)
        {
            printf("discrepancy found!\n");
            printf("\tbiggests differnce %f, at input %f\n", difference, where);
            printf("\ttruth: %f, output: %f\n", truth[Sin].func(where), taylor_series_sin(where, i));
        }
        else
        {
            printf("no discrepancy found!\n");
        }
        printf("\n");
    }
}

/* TESTING Math.h */

typedef struct
{
    f64 input_rad;
    f64 truth;
} Testing_Table;

const Testing_Table cos_table[] =
{
    {-1.5707963268f, 0.0f},
    {-1.5f, 0.0707372017f},
    {-1.0f, 0.5403023059},
    {0.0f, 1.0f},
    {1.0f, 0.5403023059},
    {1.5f, 0.0707372017},
    { 1.5707963268f, 0.0f}
};


const Testing_Table sin_table[] =
{
    {-3.1415926536f,  0.0f},
    {-3.14f, -0.0015926529f},
    {-2.0f,  -0.9092974268f},
    {-1.5f,  -0.9974949866f},
    {-1.0f,  -0.8414709848f},
    {0.0f,    0.0f},
    {1.0f,    0.8414709848f},
    {1.5f,    0.9974949866f},
    {2.0f,    0.9092974268f},
    {3.14f,   0.0015926529f},
    { 3.1415926536f,  0.0f},
};

const Testing_Table asin_table[] =
{
    {-1.0f, -1.5707963268f},
    {-0.75f, -0.8480620790f},
    {-0.5f, -0.5235987756f},
    {0.0f, 0.0f},
    {0.5f, 0.5235987756f},
    {0.75f, 0.8480620790f},
    {1.0f, 1.5707963268f},
};

const Testing_Table sqrt_table[] =
{
    {0.0f, 0.0f},
    {0.0625f, 0.25f},
    {0.25f, 0.5f},
    {0.5f, 0.7071067812f},
    {0.75f, 0.8660254038f},
    {1.0f, 1.0f}
};


void test_truth_math_h()
{
    printf("checking cos with math.h\n");
    for (u8 i = 0; i < array_count(cos_table); ++i)
    {
        if (!almost_equal(cos(cos_table[i].input_rad), cos_table[i].truth))
            printf("\tdiscrepency! %f, truth: %f, %f\n", cos_table[i].input_rad, cos_table[i].truth, cos(cos_table[i].input_rad));
        else
            printf("\tpassed! %f, truth: %f, %f\n", cos_table[i].input_rad, cos_table[i].truth, cos(cos_table[i].input_rad));
    }
    printf("\n");
    printf("checking sin with math.h\n");
    for (u8 i = 0; i < array_count(sin_table); ++i)
    {
        if (!almost_equal(sin(sin_table[i].input_rad), sin_table[i].truth))
            printf("\tdiscrepency! %f, truth: %f, %f\n", sin_table[i].input_rad, sin_table[i].truth, sin(sin_table[i].input_rad));
        else
            printf("\tpassed! %f, truth: %f, %f\n", sin_table[i].input_rad, sin_table[i].truth, sin(sin_table[i].input_rad));
    }
    printf("\n");
    printf("checking asin with math.h\n");
    for (u8 i = 0; i < array_count(asin_table); ++i)
    {
        if (!almost_equal(asin(asin_table[i].input_rad), asin_table[i].truth))
            printf("\tdiscrepency! %f, truth: %f, %f\n", asin_table[i].input_rad, asin_table[i].truth, asin(asin_table[i].input_rad));
        else
            printf("\tpassed! %f, truth: %f, %f\n", asin_table[i].input_rad, asin_table[i].truth, asin(asin_table[i].input_rad));
    }
    printf("\n");
    printf("checking sqrt with math.h\n");
    for (u8 i = 0; i < array_count(sqrt_table); ++i)
    {
        if (!almost_equal(sqrt(sqrt_table[i].input_rad), sqrt_table[i].truth))
            printf("\tdiscrepency! %f, truth: %f, %f\n", sqrt_table[i].input_rad, sqrt_table[i].truth, sqrt(sqrt_table[i].input_rad));
        else
            printf("\tpassed! %f, truth: %f, %f\n", sqrt_table[i].input_rad, sqrt_table[i].truth, sqrt(sqrt_table[i].input_rad));
    }
    printf("\n");
}
#endif
