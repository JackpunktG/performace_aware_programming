#ifndef MATH_LIB
#define MATH_LIB
#pragma once
#include <math.h>
#include <stdio.h>
#include <stdint.h>
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

typedef struct
{
    f64 min, max;
    f64 (*func)(f64);
} Truth_Function;

const Truth_Function truth[] =
{
    {-1.570796f, 1.570796f, cos},
    {-3.140970f, 3.140203f, sin},
    {0, 1, asin},
    {0, 1, sqrt}
};

typedef struct
{
    f64 (*func)(f64);
    Func_Type type;
    const char* desc;
} Funtions;

const Funtions tests[] =
{
    {cos, Cos, "math.lib"},
    {sin, Sin, "math.lib"},
    {asin, Asin, "math.lib"},
    {sqrt, Sqrt, "math.lib"}
};




/* Testing Harnes for own library */

#define TOLLERANCE 0.00000f
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

#define TEST_AMOUNT 1000
void test_math_h()
{
    for (u8 i = 0; i < array_count(tests); ++i)
    {
        printf("Testing function %s with %s\n", tests[i].desc, test_names[tests[i].type]);
        f64 difference = 0,  where;
        u64 count = 0;

        f64 inc = (truth[tests[i].type].max - truth[tests[i].type].min) / TEST_AMOUNT;
        f64 val = truth[tests[i].type].min;

        for (u64 k = 0; k < TEST_AMOUNT; ++k)
        {
            if (!almost_equal(tests[tests[i].type].func(val), tests[i].func(val)))
            {
                if (difference_val(tests[tests[i].type].func(val), tests[i].func(val)) > difference)
                {
                    difference = tests[tests[i].type].func(val), tests[i].func(val);
                    where = val;
                }
                ++count;
            }
            val += inc;
        }
        if (count)
        {
            printf("discrepencies found!\n");
            printf("\tbiggests differnce %f, at input %f\n", difference, where);
            printf("\tcount %lu\n", count);
        }
        else
        {
            printf("no descrepencies found!\n");
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
