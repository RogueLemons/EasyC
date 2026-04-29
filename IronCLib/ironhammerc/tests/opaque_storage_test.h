#ifndef IRON_HAMMER_C_TESTS_OPAQUE_STORAGE_TEST_H
#define IRON_HAMMER_C_TESTS_OPAQUE_STORAGE_TEST_H

#include "ic_hammer.h"
#include "opaque_structs.h"

IHC_TEST(verify_opaque_strict_object_can_be_initiated_with_zeroes)
{
    StrictObject obj;
    StrictObject_init(&obj);
    IHC_CHECK(StrictObject_get_float_a(&obj) == 0);
    IHC_CHECK(StrictObject_get_float_b(&obj) == 0);
    IHC_CHECK(StrictObject_get_float_c(&obj) == 0);
    IHC_CHECK(StrictObject_get_double_a(&obj) == 0);
    IHC_CHECK(StrictObject_get_double_b(&obj) == 0);
}

IHC_TEST(verify_opaque_strict_object_can_set_and_get_values)
{
    StrictObject obj;
    StrictObject_init(&obj);

    StrictObject_set_floats(&obj, 5.0f, 6.0f, 7.0f);
    IHC_CHECK(StrictObject_get_float_a(&obj) == 5.0f);
    IHC_CHECK(StrictObject_get_float_b(&obj) == 6.0f);
    IHC_CHECK(StrictObject_get_float_c(&obj) == 7.0f);

    StrictObject_set_doubles(&obj, 11.5, 12.5);
    IHC_CHECK(StrictObject_get_double_a(&obj) == 11.5);
    IHC_CHECK(StrictObject_get_double_b(&obj) == 12.5);
}

IHC_TEST(verify_opaque_fast_object_can_be_initiated_with_zeroes)
{
    FastObject obj;
    FastObject_init(&obj);
    IHC_CHECK(FastObject_get_float_a(&obj) == 0);
    IHC_CHECK(FastObject_get_float_b(&obj) == 0);
    IHC_CHECK(FastObject_get_float_c(&obj) == 0);
    IHC_CHECK(FastObject_get_double_a(&obj) == 0);
    IHC_CHECK(FastObject_get_double_b(&obj) == 0);
}

IHC_TEST(verify_opaque_fast_object_can_set_and_get_values)
{
    FastObject obj;
    FastObject_init(&obj);

    FastObject_set_floats(&obj, 5.0f, 6.0f, 7.0f);
    IHC_CHECK(FastObject_get_float_a(&obj) == 5.0f);
    IHC_CHECK(FastObject_get_float_b(&obj) == 6.0f);
    IHC_CHECK(FastObject_get_float_c(&obj) == 7.0f);

    FastObject_set_doubles(&obj, 11.5, 12.5);
    IHC_CHECK(FastObject_get_double_a(&obj) == 11.5);
    IHC_CHECK(FastObject_get_double_b(&obj) == 12.5);
}

#endif // IRON_HAMMER_C_TESTS_OPAQUE_STORAGE_TEST_H