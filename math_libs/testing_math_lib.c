#include "math_lib.h"
Funtions tests[] =
{
    {sqrt_ce, Sqrt, "Intrinsic sqrt"},
    {sqrt_ce_compact, Sqrt, "compact"}
};





int main()
{
    test_math_h(tests, array_count(tests));

    test_speed(tests, array_count(tests), 1000000000);
}
