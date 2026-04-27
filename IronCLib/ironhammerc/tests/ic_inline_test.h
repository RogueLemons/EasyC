#ifndef IRONHAMMERC_TESTS_IC_INLINE_TEST_H
#define IRONHAMMERC_TESTS_IC_INLINE_TEST_H

#include "ic_hammer.h"
#include "inline_func_wrapper_a.h"
#include "inline_func_wrapper_b.h"

IHC_TEST(verify_header_functions_can_be_used_in_multiple_source_files) 
{
    IHC_CHECK(inline_add_wrapper_a(5, 5) == 10);
    IHC_CHECK(inline_add_wrapper_b(5, 5) == 10);
}

#endif // IRONHAMMERC_TESTS_IC_INLINE_TEST_H