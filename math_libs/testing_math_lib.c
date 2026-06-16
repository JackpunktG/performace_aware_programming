#include "math_lib.h"
Funtions tests[] =
{
    {sqrt_ce, Sqrt, "Intrinsic sqrt"},
    {sqrt_ce_compact, Sqrt, "compact"},
    {sin_ce_polynomial_approx, Sin, "very rought approximation based on polynomial"},
    {sin_ce_polynomial_cubic, Sin, "Cubic varient"},
    {cos_ce_polynomal_cubic, Cos, "cubic calling sin shifted"}
};





int main()
{

    test_taylor_series();

    // test_math_h(tests, array_count(tests));

    // test_speed(tests, array_count(tests), 1000000000);
}
