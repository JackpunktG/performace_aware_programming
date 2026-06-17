#include "math_lib.h"
Funtions tests[] =
{
    // {sqrt_ce, Sqrt, "Intrinsic sqrt"},
    // {sqrt_ce_compact, Sqrt, "compact"},
    //{sin_ce_polynomial_approx, Sin, "very rought approximation based on polynomial"},
    //{sin_ce_polynomial_cubic, Sin, "Cubic varient"},
    //{cos_ce_polynomal_cubic, Cos, "cubic calling sin shifted"},
    {sin_fma_lookup_written_9, Sin, "hadwritten sin - 9 coefficient"},
    {arcsine_handwritten, Asin, "hadwritten arc - 13 coefficient"},
};





int main()
{

// #define test 10000000
//     const uint64_t rdtsc_freq_est = test_rdtsc_frequency(500);
//     f64 res = 0;
//     u64 r1 = __rdtsc();
//     for (u32 i = 0; i < test; ++i)
//         res += sin(1.0f);
//     u64 r2 = __rdtsc();
//     printf("\ttruth function: %0.4f, res %f\n", (f64)(r2-r1)/rdtsc_freq_est, res);
//     res = 0;
//     r1 = __rdtsc();
//     for (u32 i = 0; i < test; ++i)
//         res += taylor_series_sin(1.0f, 15);
//     r2 = __rdtsc();
//     printf("\ttaylors function: %0.4f %f\n", (f64)(r2-r1)/rdtsc_freq_est, res);
//
//     res = 0;
//     r1 = __rdtsc();
//     for (u32 i = 0; i < test; ++i)
//         res += sin_horners_rule(1.0f, 15);
//     r2 = __rdtsc();
//     printf("\thorner function: %0.4f, res %f\n", (f64)(r2-r1)/rdtsc_freq_est, res);
//
//     res = 0;
//     r1 = __rdtsc();
//     for (u32 i = 0; i < test; ++i)
//         res += sin_horners_fmadd(1.0f, 15);
//     r2 = __rdtsc();
//     printf("\tfmadd function: %0.4f, res %f\n", (f64)(r2-r1)/rdtsc_freq_est, res);
//
//     res = 0;
//     r1 = __rdtsc();
//     for (u32 i = 0; i < test; ++i)
//         res += sin_horners_fma(1.0f, 15);
//     r2 = __rdtsc();
//     printf("\tfma function: %0.4f, res %f\n", (f64)(r2-r1)/rdtsc_freq_est, res);
//
//     res = 0;
//     r1 = __rdtsc();
//     for (u32 i = 0; i < test; ++i)
//         res += sin_horners_fma_taylor_lookup(1.0f, 15);
//     r2 = __rdtsc();
//     printf("\ttaylor lookup function: %0.4f, res %f\n", (f64)(r2-r1)/rdtsc_freq_est, res);
//
//
//     for (u8 c = 2; c < 11; ++c)
//     {
//         res = 0;
//         r1 = __rdtsc();
//         for (u32 i = 0; i < test; ++i)
//             res += sin_horners_fma_dimetry_lookup(1.0f, c, c);
//         r2 = __rdtsc();
//         printf("\tdimetry lookup (%hhu c) function: %0.4f, res %f\n", c, (f64)(r2-r1)/rdtsc_freq_est, res);
//     }
//     res = 0;
//     r1 = __rdtsc();
//     for (u32 i = 0; i < test; ++i)
//         res += sin_fma_lookup_written(1.0f);
//     r2 = __rdtsc();
//     printf("\thandwriten lookup function: %0.4f, res %f\n", (f64)(r2-r1)/rdtsc_freq_est, res);

    test_math_h(tests, array_count(tests));
    // arcsine_test();
    // sine_test();

    // test_speed(tests, array_count(tests), 1000000000);
}
