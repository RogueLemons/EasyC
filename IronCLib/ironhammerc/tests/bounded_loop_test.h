#ifndef IRON_HAMMER_C_TESTS_BOUNDED_LOOP_TEST_H
#define IRON_HAMMER_C_TESTS_BOUNDED_LOOP_TEST_H

#include "ic_hammer.h"
#include "ironclib/ic_bounded_loop.h"

IHC_TEST(verify_bounded_while_prevents_infinite_loops) 
{
    const int always_true = 1;
    const int max_loops = 100;
    int iterations = 0;
    IC_BOUNDED_WHILE(always_true, max_loops) 
    {
        iterations++;
    }
    IHC_CHECK(iterations == max_loops);
}

IHC_TEST(verify_bounded_while_works_as_normal_within_bounds) 
{
    int count = 0;
    IC_BOUNDED_WHILE(count < 5, 10) 
    {
        count++;
    }
    IHC_CHECK(count == 5);
}

IHC_TEST(verify_bounded_while_treats_non_positive_bounds_as_zero) 
{
    int count = 0;
    IC_BOUNDED_WHILE(count < 5, -1) 
    {
        count++;
    }
    IHC_CHECK(count == 0);
}

IHC_TEST(verify_bounded_do_while_prevents_infinite_loops) 
{
    const int always_true = 1;
    const int max_loops = 100;
    int iterations = 0;
    IC_BOUNDED_DO_WHILE(always_true, max_loops) 
    {
        iterations++;
    };
    IHC_CHECK(iterations == max_loops);
}

IHC_TEST(verify_bounded_do_while_works_as_normal_within_bounds) 
{
    int count = 0;
    IC_BOUNDED_DO_WHILE(count < 5, 10) 
    {
        count++;
    };
    IHC_CHECK(count == 5);

    const int always_false = 0;
    count = 0;
    IC_BOUNDED_DO_WHILE(always_false, 10) 
    {
        count++;
    };
    IHC_CHECK(count == 1);
}

IHC_TEST(verify_bounded_do_while_treats_non_positive_bounds_as_zero) 
{
    int count = 0;
    IC_BOUNDED_DO_WHILE(count < 5, -1) 
    {
        count++;
    };
    IHC_CHECK(count == 1);
}


#endif // IRON_HAMMER_C_TESTS_BOUNDED_LOOP_TEST_H