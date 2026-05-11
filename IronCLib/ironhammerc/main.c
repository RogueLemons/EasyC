
// Define printing macro for test results
#define IHC_PRINT(ctx, test, msg, value, has_value) \
do { \
    printf("%-12s %-20s %-20s", ctx, test, msg); \
    if (has_value) printf(" %u", value); \
    printf("\n"); \
} while (0)

// Set to 1 to enable stress tests, which can be time-consuming; 0 to disable them for quicker runs
#define RUN_STRESS_TESTS 0

// Includes
#include <stdio.h>
#include "ic_hammer.h"
#include "ic_static_assert.h"

// Test cases
#include "tests/inline_test.h"
#include "tests/memory_test.h"
#include "tests/bounded_loop_test.h"
#include "tests/typenum_test.h"
#include "tests/num_cast_test.h"
#include "tests/opaque_storage_test.h"
#include "tests/result_test.h"
#include "tests/concurrency_test.h"
#include "tests/concurrency_signal_test.h"

IC_STATIC_ASSERT(8 == 2 * 4, "A simple verification that the static assert macro works correctly");

IHC_TEST(verify_static_assert_works_inside_functions) 
{
    IC_STATIC_ASSERT(2 == 1 + 1, "Simple nested static assert test");
}

int main(void) {

    const ihc_test_case iron_c_lib_tests[] = 
    {
        // ic_static_assert.h
        IHC_TEST_ENTRY(verify_static_assert_works_inside_functions),
        // ic_inline.h
        IHC_TEST_ENTRY(verify_header_functions_can_be_used_in_multiple_source_files),
        // ic_memory.h
        IHC_TEST_ENTRY(verify_memory_protects_against_negative_size),
        IHC_TEST_ENTRY(verify_memory_protects_against_zero_size),
        IHC_TEST_ENTRY(verify_memory_protects_against_overflow),
        IHC_TEST_ENTRY(verify_memory_allocates_valid_array),
        // ic_bounded_loop.h
        IHC_TEST_ENTRY(verify_bounded_while_prevents_infinite_loops),
        IHC_TEST_ENTRY(verify_bounded_while_works_as_normal_within_bounds),
        IHC_TEST_ENTRY(verify_bounded_while_treats_non_positive_bounds_as_zero),
        IHC_TEST_ENTRY(verify_bounded_do_while_prevents_infinite_loops),
        IHC_TEST_ENTRY(verify_bounded_do_while_works_as_normal_within_bounds),
        IHC_TEST_ENTRY(verify_bounded_do_while_treats_non_positive_bounds_as_zero),
        // ic_typenum.h
        IHC_TEST_ENTRY(verify_typenum_internal_values_are_correct),
        IHC_TEST_ENTRY(verify_typenum_equality_function_works),
        IHC_TEST_ENTRY(verify_typenum_provides_correct_strings),
        // ic_num_cast.h
        IHC_TEST_ENTRY(verify_simple_same_bit_cast_gives_expected_value),
        IHC_TEST_ENTRY(verify_floating_to_unsigned_stays_within_bounds),
        IHC_TEST_ENTRY(verify_floating_infinity_and_nan_becomes_bounded),
        IHC_TEST_ENTRY(verify_signed_integer_conversions_for_large_type_to_small_type),
        IHC_TEST_ENTRY(verify_unsigned_integer_conversions_for_large_type_to_small_type),
        IHC_TEST_ENTRY(verify_floating_point_conversions_for_large_type_to_small_type),
        // ic_opaque_storage.h
        IHC_TEST_ENTRY(verify_opaque_strict_object_can_be_initiated_with_zeroes),
        IHC_TEST_ENTRY(verify_opaque_strict_object_can_set_and_get_values),
        IHC_TEST_ENTRY(verify_opaque_fast_object_can_be_initiated_with_zeroes),
        IHC_TEST_ENTRY(verify_opaque_fast_object_can_set_and_get_values),
        // ic_result.h
        IHC_TEST_ENTRY(verify_generation_of_simple_result_type_and_generated_functions),
        IHC_TEST_ENTRY(verify_ok_and_err_functions_work_for_typenum_error),
        IHC_TEST_ENTRY(verify_optional_accessor_macros_work),
        IHC_TEST_ENTRY(verify_try_macro_propagates_error_in_same_error_type_context),
        // ic_concurrency.h
        IHC_TEST_ENTRY(verify_atomic_int_can_get_and_set_values_correctly),
        IHC_TEST_ENTRY(verify_atomic_does_not_crash_when_given_nullptr),
        IHC_TEST_ENTRY(verify_atomic_int_can_handle_full_32_bit_range),
        IHC_TEST_ENTRY(verify_task_can_run_and_join_thread),
        IHC_TEST_ENTRY(verify_task_does_not_crash_when_given_nullptr),
        IHC_TEST_ENTRY(verify_mutex_can_be_locked_and_unlocked),
        IHC_TEST_ENTRY(verify_mutex_does_not_crash_when_given_nullptr),
        IHC_TEST_ENTRY(verify_atomic_is_thread_safe_under_contention),
        IHC_TEST_ENTRY(verify_mutex_prevents_race_conditions),
        IHC_TEST_ENTRY(verify_task_cannot_be_joined_twice),
        IHC_TEST_ENTRY(verify_get_result_fails_if_task_running),
        IHC_TEST_ENTRY(verify_memory_visibility_with_atomics),
        IHC_TEST_ENTRY(verify_mutex_trylock_fails_under_contention),
        // ic_concurrency_signal.h
        IHC_TEST_ENTRY(verify_condition_variable_can_signal_waiting_thread),
        IHC_TEST_ENTRY(verify_condition_variable_notify_all_wakes_all_waiters),
        IHC_TEST_ENTRY(verify_condition_variable_does_not_crash_when_given_nullptr),
        IHC_TEST_ENTRY(verify_condition_variable_wait_rechecks_predicate),
        IHC_TEST_ENTRY(verify_gate_nullptr_safety),
        IHC_TEST_ENTRY(verify_gate_releases_exactly_one_thread_per_signal),
        IHC_TEST_ENTRY(verify_gate_is_lossless),
        IHC_TEST_ENTRY(verify_broadcast_nullptr_safety),
        IHC_TEST_ENTRY(verify_broadcast_wakes_all_waiters),
        IHC_TEST_ENTRY(verify_broadcast_is_sticky_until_reset),
        IHC_TEST_ENTRY(verify_broadcast_reset_blocks_new_waiters),
        #if RUN_STRESS_TESTS
        IHC_TEST_ENTRY(stress_gate_interleave_deterministic),
        IHC_TEST_ENTRY(stress_gate_signal_before_wait_deterministic),
        IHC_TEST_ENTRY(stress_gate_consumes_permits_correctly),
        IHC_TEST_ENTRY(stress_broadcast_deterministic_signal_reset_interleave),
        IHC_TEST_ENTRY(stress_broadcast_signal_before_wait_deterministic),
        IHC_TEST_ENTRY(stress_broadcast_reset_reblocks_waiters),
        #endif
        IHC_TEST_ENTRY(verify_task_pool_executes_all_tasks_with_completion),
        IHC_TEST_ENTRY(verify_task_pool_fire_and_forget_executes_tasks),
        IHC_TEST_ENTRY(verify_task_pool_processes_tasks_consistently),
        IHC_TEST_ENTRY(verify_task_pool_rejects_null_arguments),
        IHC_TEST_ENTRY(verify_task_completion_wait_times_out_and_resolves),
        IHC_TEST_ENTRY(verify_task_pool_close_state_transitions),
        IHC_TEST_ENTRY(verify_task_pool_close_drain_completes_all_tasks),
        IHC_TEST_ENTRY(verify_task_pool_close_abort_drops_queued_tasks),
        IHC_TEST_ENTRY(verify_task_pool_reaches_timeout_for_blocking_tasks),
        IHC_TEST_ENTRY(verify_task_pool_can_safely_be_destroyed_without_manually_closing),
        IHC_TEST_ENTRY(verify_task_pool_can_fill_up_and_safely_reject_excess_tasks),
        #if RUN_STRESS_TESTS
        IHC_TEST_ENTRY(stress_task_pool_handles_high_contention_submissions),
        IHC_TEST_ENTRY(stress_task_pool_survives_shutdown_submission_races),
        IHC_TEST_ENTRY(stress_task_pool_survives_repeated_lifecycle_cycles)
        #endif
    };

    IHC_RUN(iron_c_lib_tests);
    IHC_REPORT();

    return ihc_failures();
}