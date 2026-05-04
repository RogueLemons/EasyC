#ifndef IRON_HAMMER_C_TESTS_CONCURRENCY_TEST_H
#define IRON_HAMMER_C_TESTS_CONCURRENCY_TEST_H

#include "ic_hammer.h"
#include "ironclib/ic_concurrency.h"

IHC_TEST(verify_atomic_int_can_get_and_set_values_correctly)
{
    ic_atomic_i32 atomic = ic_make_atomic(42);
    IHC_CHECK(ic_atomic_load(&atomic) == 42);
    ic_atomic_store(&atomic, 100);
    IHC_CHECK(ic_atomic_load(&atomic) == 100);
    IHC_CHECK(ic_atomic_fetch_add(&atomic, 23) == 100);
    IHC_CHECK(ic_atomic_load(&atomic) == 123);
}

static int thread_function(void* arg)
{
    int* num = (int*)arg;
    for (int i = 0; i < 1000000; i++)
    {
        (*num)++;
    }
    return 42;
}

IHC_TEST(verify_task_can_run_and_join_thread)
{
    ic_task task;
    int num = 0;
    IHC_CHECK(ic_task_init(&task, thread_function, &num) == IC_CONCURRENCY_OK);
    IHC_CHECK(ic_task_join(&task) == IC_CONCURRENCY_OK);
    IHC_CHECK(num == 1000000);
    int result = -1;
    IHC_CHECK(ic_task_get_result(&task, &result) == IC_CONCURRENCY_OK);
    IHC_CHECK(result == 42);
}

IHC_TEST(verify_mutex_can_be_locked_and_unlocked)
{
    ic_mutex mutex;
    IHC_CHECK(ic_mutex_init(&mutex) == IC_CONCURRENCY_OK);
    ic_mutex_lock(&mutex);
    IHC_CHECK(ic_mutex_trylock(&mutex) == IC_CONCURRENCY_FAILURE);
    ic_mutex_unlock(&mutex);
    IHC_CHECK(ic_mutex_trylock(&mutex) == IC_CONCURRENCY_OK);
    ic_mutex_unlock(&mutex);
    IHC_CHECK(ic_mutex_destroy(&mutex) == IC_CONCURRENCY_OK);
}

#endif // IRON_HAMMER_C_TESTS_CONCURRENCY_TEST_H