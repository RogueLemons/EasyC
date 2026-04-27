
// Define printing macro for test results
#define IHC_PRINT(ctx, test, msg, value, has_value) \
do { \
    printf("%-12s %-20s %-20s", ctx, test, msg); \
    if (has_value) printf(" %u", value); \
    printf("\n"); \
} while (0)

// Includes
#include <stdio.h>
#include "ic_hammer.h"
#include "ic_static_assert.h"

// Test cases
#include "tests/ic_inline_test.h"

IC_STATIC_ASSERT(8 == 2*4, "A simple verification that the static assert macro works correctly");

int main(void) {

    const ihc_test_case my_tests[] = 
    {
        IHC_TEST_ENTRY(verify_header_functions_can_be_used_in_multiple_source_files),
    };

    IHC_RUN(my_tests);
    IHC_REPORT();

    return ihc_failures();
}