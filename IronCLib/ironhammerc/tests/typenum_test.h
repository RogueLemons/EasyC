#ifndef IRON_HAMMER_C_TESTS_TYPENUM_TEST_H
#define IRON_HAMMER_C_TESTS_TYPENUM_TEST_H

#include "ic_hammer.h"
#include "ironclib/ic_typenum.h"
#include <stdint.h>
#include <string.h>

#define USE_MODE_LIST(X, Type) \
    X(Type, Quality, 0, "Usage optimized for quality") \
    X(Type, Performance, 1, "Usage optimized for performance") \
    X(Type, Battery, 2, "Usage optimized for battery duration") 
IC_TYPENUM_FULL(UseMode, uint8_t, USE_MODE_LIST)

IHC_TEST(verify_typenum_internal_values_are_correct)
{
    const uint8_t quality_val = UseMode_get(UseMode_Quality);
    IHC_CHECK(quality_val == 0);

    const uint8_t performance_val = UseMode_get(UseMode_Performance);
    IHC_CHECK(performance_val == 1);

    const uint8_t battery_val = UseMode_get(UseMode_Battery);
    IHC_CHECK(battery_val == 2);
}

IHC_TEST(verify_typenum_equality_function_works)
{
    UseMode use_mode = UseMode_Quality;
    IHC_CHECK(UseMode_eq(use_mode, UseMode_Quality));
    
    use_mode = UseMode_Performance;
    IHC_CHECK(UseMode_eq(use_mode, UseMode_Performance));

    use_mode = UseMode_Battery;
    IHC_CHECK(UseMode_eq(use_mode, UseMode_Battery));

    IHC_CHECK(!UseMode_eq(UseMode_Quality, UseMode_Performance));
    IHC_CHECK(!UseMode_eq(UseMode_Quality, UseMode_Battery));
    IHC_CHECK(!UseMode_eq(UseMode_Battery, UseMode_Performance));
}

IHC_TEST(verify_typenum_provides_correct_strings)
{
    const char quality_str[] = "Usage optimized for quality";
    const char* const quality_typenum_str = UseMode_to_string(UseMode_Quality);
    IHC_CHECK(strcmp(quality_str, quality_typenum_str) == 0);

    const char performance_str[] = "Usage optimized for performance";
    const char* const performance_typenum_str = UseMode_to_string(UseMode_Performance);
    IHC_CHECK(strcmp(performance_str, performance_typenum_str) == 0);

    const char battery_str[] = "Usage optimized for battery duration";
    const char* const battery_typenum_str = UseMode_to_string(UseMode_Battery);
    IHC_CHECK(strcmp(battery_str, battery_typenum_str) == 0);
}

#endif // IRON_HAMMER_C_TESTS_TYPENUM_TEST_H