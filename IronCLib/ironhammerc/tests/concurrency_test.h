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
    IHC_CHECK(ic_atomic_exchange(&atomic, 7) == 123);
    IHC_CHECK(ic_atomic_load(&atomic) == 7);
}

IHC_TEST(verify_atomic_does_not_crash_when_given_nullptr)
{
    IHC_CHECK(ic_atomic_load(NULL) == 0);
    ic_atomic_store(NULL, 100);
    IHC_CHECK(ic_atomic_fetch_add(NULL, 23) == 0);
    IHC_CHECK(ic_atomic_exchange(NULL, 7) == 0);
}

IHC_TEST(verify_atomic_int_can_handle_full_32_bit_range)
{
    ic_atomic_i32 atomic = ic_make_atomic(0);
    ic_atomic_store(&atomic, INT32_MAX);
    IHC_CHECK(ic_atomic_load(&atomic) == INT32_MAX);
    IHC_CHECK(ic_atomic_fetch_add(&atomic, 1) == INT32_MAX);
    IHC_CHECK(ic_atomic_load(&atomic) == INT32_MIN);
    IHC_CHECK(ic_atomic_exchange(&atomic, INT32_MIN) == INT32_MIN);
    IHC_CHECK(ic_atomic_load(&atomic) == INT32_MIN);
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
    IHC_ASSERT(ic_task_init(&task, thread_function, &num) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_task_join(&task) == IC_CONCURRENCY_OK);
    IHC_CHECK(num == 1000000);
    int result = -1;
    IHC_CHECK(ic_task_get_result(&task, &result) == IC_CONCURRENCY_OK);
    IHC_CHECK(result == 42);
}

IHC_TEST(verify_task_does_not_crash_when_given_nullptr)
{
    IHC_CHECK(ic_task_init(NULL, thread_function, NULL) == IC_CONCURRENCY_NULLREF);
    ic_task task;
    IHC_CHECK(ic_task_init(&task, NULL, NULL) == IC_CONCURRENCY_NULLREF);
    IHC_CHECK(ic_task_join(NULL) == IC_CONCURRENCY_NULLREF);
    IHC_CHECK(ic_task_get_result(&task, NULL) == IC_CONCURRENCY_NULLREF);
    int result;
    IHC_CHECK(ic_task_get_result(NULL, &result) == IC_CONCURRENCY_NULLREF);
}

IHC_TEST(verify_mutex_can_be_locked_and_unlocked)
{
    ic_mutex mutex;
    IHC_ASSERT(ic_mutex_init(&mutex) == IC_CONCURRENCY_OK);
    ic_mutex_lock(&mutex);
    IHC_CHECK(ic_mutex_trylock(&mutex) == IC_CONCURRENCY_ALREADY_LOCKED);
    ic_mutex_unlock(&mutex);
    IHC_CHECK(ic_mutex_trylock(&mutex) == IC_CONCURRENCY_OK);
    ic_mutex_unlock(&mutex);
    IHC_CHECK(ic_mutex_destroy(&mutex) == IC_CONCURRENCY_OK);
}

IHC_TEST(verify_mutex_does_not_crash_when_given_nullptr)
{
    IHC_CHECK(ic_mutex_init(NULL) == IC_CONCURRENCY_NULLREF);
    ic_mutex_lock(NULL);
    ic_mutex_unlock(NULL);
    IHC_CHECK(ic_mutex_trylock(NULL) == IC_CONCURRENCY_NULLREF);
    IHC_CHECK(ic_mutex_destroy(NULL) == IC_CONCURRENCY_NULLREF);
}

static int atomic_increment_thread(void* arg)
{
    ic_atomic_i32* a = (ic_atomic_i32*)arg;

    for (int i = 0; i < 100000; i++)
    {
        ic_atomic_fetch_add(a, 1);
    }

    return 0;
}

IHC_TEST(verify_atomic_is_thread_safe_under_contention)
{
    ic_atomic_i32 atomic = ic_make_atomic(0);

    ic_task t1, t2, t3, t4;

    ic_task_init(&t1, atomic_increment_thread, &atomic);
    ic_task_init(&t2, atomic_increment_thread, &atomic);
    ic_task_init(&t3, atomic_increment_thread, &atomic);
    ic_task_init(&t4, atomic_increment_thread, &atomic);

    ic_task_join(&t1);
    ic_task_join(&t2);
    ic_task_join(&t3);
    ic_task_join(&t4);

    IHC_CHECK(ic_atomic_load(&atomic) == 400000);
}

typedef struct shared_data 
{
    int value;
    ic_mutex* mutex;
} shared_data;

static int mutex_increment_thread(void* arg)
{
    shared_data* data = (shared_data*)arg;

    for (int i = 0; i < 100000; i++)
    {
        ic_mutex_lock(data->mutex);
        data->value++;
        ic_mutex_unlock(data->mutex);
    }

    return 0;
}

IHC_TEST(verify_mutex_prevents_race_conditions)
{
    ic_mutex mutex;
    ic_mutex_init(&mutex);

    shared_data data = {0, &mutex};

    ic_task t1, t2, t3, t4;

    ic_task_init(&t1, mutex_increment_thread, &data);
    ic_task_init(&t2, mutex_increment_thread, &data);
    ic_task_init(&t3, mutex_increment_thread, &data);
    ic_task_init(&t4, mutex_increment_thread, &data);

    ic_task_join(&t1);
    ic_task_join(&t2);
    ic_task_join(&t3);
    ic_task_join(&t4);

    IHC_CHECK(data.value == 400000);

    ic_mutex_destroy(&mutex);
}

IHC_TEST(verify_task_cannot_be_joined_twice)
{
    ic_task task;
    int num = 0;

    ic_task_init(&task, thread_function, &num);

    IHC_CHECK(ic_task_join(&task) == IC_CONCURRENCY_OK);
    IHC_CHECK(ic_task_join(&task) == IC_CONCURRENCY_ALREADY_JOINED);
}

static int slow_thread(void* arg)
{
    (void)arg;
    ic_thread_sleep(500);
    return 7;
}

IHC_TEST(verify_get_result_fails_if_task_running)
{
    ic_task task;
    ic_task_init(&task, slow_thread, NULL);

    ic_thread_sleep(200); // Give the thread time to start and be in the middle of sleeping
    int result;
    int res = ic_task_get_result(&task, &result);

    IHC_CHECK(res == IC_CONCURRENCY_FAILURE);

    ic_task_join(&task);
}

typedef struct visibility_test_obj 
{
    ic_atomic_i32 flag;
    int data;
} visibility_test_obj;

static int visibility_test_writer_func(void* arg)
{
    visibility_test_obj* t = (visibility_test_obj*)arg;
    t->data = 123;
    ic_atomic_store(&t->flag, 1);
    return 0;
}

static int visibility_test_reader_func(void* arg)
{
    visibility_test_obj* t = (visibility_test_obj*)arg;

    while (ic_atomic_load(&t->flag) == 0) {}

    // If memory ordering is broken, this *might* see stale data
    IHC_CHECK(t->data == 123);

    return 0;
}

IHC_TEST(verify_memory_visibility_with_atomics)
{
    visibility_test_obj vis_obj;
    ic_atomic_init(&vis_obj.flag, 0);
    vis_obj.data = 0;

    ic_task writer, reader;

    ic_task_init(&reader, visibility_test_reader_func, &vis_obj);
    ic_task_init(&writer, visibility_test_writer_func, &vis_obj);

    ic_task_join(&writer);
    ic_task_join(&reader);
}

static int mutex_holder_thread(void* arg)
{
    ic_mutex* m = (ic_mutex*)arg;

    ic_mutex_lock(m);

    // Hold the mutex long enough for the other thread to attempt trylock
    ic_thread_sleep(300);

    ic_mutex_unlock(m);
    return 0;
}

IHC_TEST(verify_mutex_trylock_fails_under_contention)
{
    ic_mutex mutex;
    IHC_ASSERT(ic_mutex_init(&mutex) == IC_CONCURRENCY_OK);

    ic_task task;
    IHC_ASSERT(ic_task_init(&task, mutex_holder_thread, &mutex) == IC_CONCURRENCY_OK);

    // Give the spawned thread time to acquire the mutex first
    ic_thread_sleep(100);

    // This should fail because the other thread is holding the lock
    int result = ic_mutex_trylock(&mutex);
    IHC_CHECK(result == IC_CONCURRENCY_ALREADY_LOCKED);

    IHC_ASSERT(ic_task_join(&task) == IC_CONCURRENCY_OK);

    // After the thread releases the mutex, trylock should succeed
    result = ic_mutex_trylock(&mutex);
    IHC_CHECK(result == IC_CONCURRENCY_OK);

    ic_mutex_unlock(&mutex);

    IHC_CHECK(ic_mutex_destroy(&mutex) == IC_CONCURRENCY_OK);
}

#endif // IRON_HAMMER_C_TESTS_CONCURRENCY_TEST_H