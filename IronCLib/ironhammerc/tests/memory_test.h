#ifndef IRON_HAMMER_C_TESTS_MEMORY_TEST_H
#define IRON_HAMMER_C_TESTS_MEMORY_TEST_H

#include "ic_hammer.h"
#include "ic_memory.h"

IHC_TEST(verify_memory_protects_against_negative_size) 
{
    int* const negative_size_array = IC_MALLOC_ARRAY(int, -5);
    IHC_CHECK(negative_size_array == NULL);
    free(negative_size_array); // in case memory was allocated
}

IHC_TEST(verify_memory_protects_against_zero_size) 
{
    int* const zero_size_array = IC_MALLOC_ARRAY(int, 0);
    IHC_CHECK(zero_size_array == NULL);
    free(zero_size_array); // in case memory was allocated
}

IHC_TEST(verify_memory_protects_against_overflow) 
{
    const size_t max_size = (size_t)-1;
    double* const overflow_array = IC_MALLOC_ARRAY(double, (max_size / sizeof(double)) + 1);
    IHC_CHECK(overflow_array == NULL);
    free(overflow_array); // in case memory was allocated
}

IHC_TEST(verify_memory_allocates_valid_array) 
{
    char* const valid_array = IC_MALLOC_ARRAY(char, 10);
    IHC_CHECK(valid_array != NULL);
    valid_array[0] = 'H';
    valid_array[1] = 'i';
    IHC_CHECK(valid_array[0] == 'H');
    IHC_CHECK(valid_array[1] == 'i');
    free(valid_array);
}

#endif // IRON_HAMMER_C_TESTS_MEMORY_TEST_H