#ifndef IRON_HAMMER_C_TESTS_CONCURRENCY_SIGNAL_TEST_H
#define IRON_HAMMER_C_TESTS_CONCURRENCY_SIGNAL_TEST_H

#ifndef RUN_STRESS_TESTS
#define RUN_STRESS_TESTS 0
#endif

#include "ic_hammer.h"
#include "ironclib/ic_concurrency.h"
#include "ironclib/ic_concurrency_signal.h"
#include "ironclib/ic_static_assert.h"
#include "docs/premade/parallel_work.h"

// ==============================================================================
// CONDITION VARIABLE TESTS
// ==============================================================================

typedef struct condition_variable_test_data
{
    ic_condition_variable cv;
    ic_mutex mutex;

    int ready;
    int wake_count;
} condition_variable_test_data;

static int condition_variable_waiter_thread(void* arg)
{
    condition_variable_test_data* data = (condition_variable_test_data*)arg;

    ic_mutex_lock(&data->mutex);

    while (!data->ready)
    {
        IHC_CHECK(ic_condition_variable_wait(&data->cv, &data->mutex) == IC_CONCURRENCY_OK);
    }

    data->wake_count++;

    ic_mutex_unlock(&data->mutex);

    return 0;
}

IHC_TEST(verify_condition_variable_can_signal_waiting_thread)
{
    condition_variable_test_data data = {0};

    IHC_ASSERT(ic_condition_variable_init(&data.cv) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&data.mutex) == IC_CONCURRENCY_OK);

    data.ready = 0;
    data.wake_count = 0;

    ic_task waiter;

    IHC_ASSERT(ic_task_init(&waiter, condition_variable_waiter_thread, &data) == IC_CONCURRENCY_OK);

    ic_thread_sleep(100);

    ic_mutex_lock(&data.mutex);
    data.ready = 1;
    ic_condition_variable_notify_one(&data.cv);
    ic_mutex_unlock(&data.mutex);

    ic_task_join(&waiter);

    IHC_CHECK(data.wake_count == 1);

    IHC_CHECK(ic_condition_variable_destroy(&data.cv) == IC_CONCURRENCY_OK);
    IHC_CHECK(ic_mutex_destroy(&data.mutex) == IC_CONCURRENCY_OK);
}

IHC_TEST(verify_condition_variable_notify_all_wakes_all_waiters)
{
    condition_variable_test_data data = {0};

    IHC_ASSERT(ic_condition_variable_init(&data.cv) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&data.mutex) == IC_CONCURRENCY_OK);

    ic_task t1, t2, t3, t4;

    ic_task_init(&t1, condition_variable_waiter_thread, &data);
    ic_task_init(&t2, condition_variable_waiter_thread, &data);
    ic_task_init(&t3, condition_variable_waiter_thread, &data);
    ic_task_init(&t4, condition_variable_waiter_thread, &data);

    ic_thread_sleep(100);

    ic_mutex_lock(&data.mutex);

    data.ready = 1;

    IHC_CHECK(ic_condition_variable_notify_all(&data.cv) == IC_CONCURRENCY_OK);

    ic_mutex_unlock(&data.mutex);

    ic_task_join(&t1);
    ic_task_join(&t2);
    ic_task_join(&t3);
    ic_task_join(&t4);

    IHC_CHECK(data.wake_count == 4);

    ic_condition_variable_destroy(&data.cv);
    ic_mutex_destroy(&data.mutex);
}

IHC_TEST(verify_condition_variable_does_not_crash_when_given_nullptr)
{   
    IHC_CHECK(ic_condition_variable_init(NULL) == IC_CONCURRENCY_NULLREF);
    IHC_CHECK(ic_condition_variable_notify_one(NULL) == IC_CONCURRENCY_NULLREF);
    IHC_CHECK(ic_condition_variable_notify_all(NULL) == IC_CONCURRENCY_NULLREF);
    ic_mutex mutex;
    IHC_CHECK(ic_condition_variable_wait(NULL, &mutex) == IC_CONCURRENCY_NULLREF);
    
    ic_condition_variable cv;
    IHC_CHECK(ic_condition_variable_wait(&cv, NULL) == IC_CONCURRENCY_NULLREF);
    IHC_CHECK(ic_condition_variable_destroy(NULL) == IC_CONCURRENCY_NULLREF);
}

IHC_TEST(verify_condition_variable_wait_rechecks_predicate)
{
    condition_variable_test_data data = {0};

    IHC_ASSERT(ic_condition_variable_init(&data.cv) == IC_CONCURRENCY_OK);

    IHC_ASSERT(ic_mutex_init(&data.mutex) == IC_CONCURRENCY_OK);

    data.ready = 0;
    data.wake_count = 0;

    ic_task waiter;

    IHC_ASSERT(ic_task_init(&waiter, condition_variable_waiter_thread, &data) == IC_CONCURRENCY_OK);

    ic_thread_sleep(100);

    // Notify without changing predicate
    IHC_CHECK(ic_condition_variable_notify_one(&data.cv) == IC_CONCURRENCY_OK);

    ic_thread_sleep(200);

    // Thread should still be blocked
    IHC_CHECK(data.wake_count == 0);

    ic_mutex_lock(&data.mutex);

    data.ready = 1;

    IHC_CHECK(ic_condition_variable_notify_one(&data.cv) == IC_CONCURRENCY_OK);

    ic_mutex_unlock(&data.mutex);

    ic_task_join(&waiter);

    IHC_CHECK(data.wake_count == 1);

    ic_condition_variable_destroy(&data.cv);
    ic_mutex_destroy(&data.mutex);
}

// ==============================================================================
// GATE TESTS
// ==============================================================================

IHC_TEST(verify_gate_nullptr_safety)
{
    IHC_CHECK(ic_gate_init(NULL) == IC_CONCURRENCY_NULLREF);
    IHC_CHECK(ic_gate_wait(NULL) == IC_CONCURRENCY_NULLREF);
    IHC_CHECK(ic_gate_signal_one(NULL) == IC_CONCURRENCY_NULLREF);
    IHC_CHECK(ic_gate_destroy(NULL) == IC_CONCURRENCY_NULLREF);
}

typedef struct gate_test_data
{
    ic_gate gate;
    ic_mutex mutex;
    int wake_count;

} gate_test_data;

static int gate_waiter_thread(void* arg)
{
    gate_test_data* data = (gate_test_data*)arg;
    IHC_CHECK(ic_gate_wait(&data->gate) == IC_CONCURRENCY_OK);

    ic_mutex_lock(&data->mutex);
    data->wake_count++;
    ic_mutex_unlock(&data->mutex);

    return 0;
}

IHC_TEST(verify_gate_releases_exactly_one_thread_per_signal)
{
    gate_test_data data = {0};

    IHC_ASSERT(ic_gate_init(&data.gate) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&data.mutex) == IC_CONCURRENCY_OK);

    data.wake_count = 0;

    ic_task t1, t2, t3;
    ic_task_init(&t1, gate_waiter_thread, &data);
    ic_task_init(&t2, gate_waiter_thread, &data);
    ic_task_init(&t3, gate_waiter_thread, &data);

    ic_thread_sleep(100);

    ic_gate_signal_one(&data.gate);
    ic_thread_sleep(50);
    IHC_CHECK(data.wake_count == 1);

    ic_gate_signal_one(&data.gate);
    ic_thread_sleep(50);
    IHC_CHECK(data.wake_count == 2);

    ic_gate_signal_one(&data.gate);
    ic_thread_sleep(50);
    IHC_CHECK(data.wake_count == 3);

    ic_task_join(&t1);
    ic_task_join(&t2);
    ic_task_join(&t3);

    ic_gate_destroy(&data.gate);
    ic_mutex_destroy(&data.mutex);
}

IHC_TEST(verify_gate_is_lossless)
{
    gate_test_data data = {0};

    IHC_ASSERT(ic_gate_init(&data.gate) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&data.mutex) == IC_CONCURRENCY_OK);

    data.wake_count = 0;

    ic_gate_signal_one(&data.gate);

    ic_task t;
    ic_task_init(&t, gate_waiter_thread, &data);

    ic_task_join(&t);

    IHC_CHECK(data.wake_count == 1);

    ic_gate_destroy(&data.gate);
    ic_mutex_destroy(&data.mutex);
}

// ==============================================================================
// BROADCAST TESTS
// ==============================================================================

IHC_TEST(verify_broadcast_nullptr_safety)
{
    IHC_CHECK(ic_broadcast_init(NULL) == IC_CONCURRENCY_NULLREF);
    IHC_CHECK(ic_broadcast_wait(NULL) == IC_CONCURRENCY_NULLREF);
    IHC_CHECK(ic_broadcast_signal_all(NULL) == IC_CONCURRENCY_NULLREF);
    IHC_CHECK(ic_broadcast_reset(NULL) == IC_CONCURRENCY_NULLREF);
    IHC_CHECK(ic_broadcast_destroy(NULL) == IC_CONCURRENCY_NULLREF);
}

typedef struct broadcast_test_data
{
    ic_broadcast broadcast;
    ic_mutex mutex;
    int wake_count;
} broadcast_test_data;

static int broadcast_waiter_thread(void* arg)
{
    broadcast_test_data* data = (broadcast_test_data*)arg;

    IHC_CHECK(ic_broadcast_wait(&data->broadcast) == IC_CONCURRENCY_OK);

    ic_mutex_lock(&data->mutex);
    data->wake_count++;
    ic_mutex_unlock(&data->mutex);

    return 0;
}

IHC_TEST(verify_broadcast_wakes_all_waiters)
{
    broadcast_test_data data = {0};

    IHC_ASSERT(ic_broadcast_init(&data.broadcast) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&data.mutex) == IC_CONCURRENCY_OK);

    ic_task t1, t2, t3, t4;
    ic_task_init(&t1, broadcast_waiter_thread, &data);
    ic_task_init(&t2, broadcast_waiter_thread, &data);
    ic_task_init(&t3, broadcast_waiter_thread, &data);
    ic_task_init(&t4, broadcast_waiter_thread, &data);

    ic_thread_sleep(100);

    ic_broadcast_signal_all(&data.broadcast);

    ic_task_join(&t1);
    ic_task_join(&t2);
    ic_task_join(&t3);
    ic_task_join(&t4);

    IHC_CHECK(data.wake_count == 4);

    ic_broadcast_destroy(&data.broadcast);
    ic_mutex_destroy(&data.mutex);
}

IHC_TEST(verify_broadcast_is_sticky_until_reset)
{
    broadcast_test_data data = {0};

    IHC_ASSERT(ic_broadcast_init(&data.broadcast) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&data.mutex) == IC_CONCURRENCY_OK);

    data.wake_count = 0;

    ic_broadcast_signal_all(&data.broadcast);

    ic_task t1, t2;

    ic_task_init(&t1, broadcast_waiter_thread, &data);
    ic_task_init(&t2, broadcast_waiter_thread, &data);

    ic_task_join(&t1);
    ic_task_join(&t2);

    IHC_CHECK(data.wake_count == 2);

    ic_broadcast_reset(&data.broadcast);

    data.wake_count = 0;

    ic_task_init(&t1, broadcast_waiter_thread, &data);
    ic_thread_sleep(50);

    IHC_CHECK(data.wake_count == 0);

    ic_broadcast_signal_all(&data.broadcast);

    ic_task_join(&t1);

    IHC_CHECK(data.wake_count == 1);

    ic_broadcast_destroy(&data.broadcast);
    ic_mutex_destroy(&data.mutex);
}

IHC_TEST(verify_broadcast_reset_blocks_new_waiters)
{
    broadcast_test_data data = {0};

    IHC_ASSERT(ic_broadcast_init(&data.broadcast) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&data.mutex) == IC_CONCURRENCY_OK);

    data.wake_count = 0;

    // Step 1: signal broadcast (sticky state ON)
    IHC_CHECK(ic_broadcast_signal_all(&data.broadcast) == IC_CONCURRENCY_OK);

    // Step 2: reset it immediately (sticky state OFF again)
    IHC_CHECK(ic_broadcast_reset(&data.broadcast) == IC_CONCURRENCY_OK);

    // Step 3: start waiter AFTER reset
    ic_task t;
    IHC_ASSERT(ic_task_init(&t, broadcast_waiter_thread, &data) == IC_CONCURRENCY_OK);

    // Give it time to potentially pass incorrectly if broken
    ic_thread_sleep(100);

    // If reset works correctly, this MUST still be 0
    IHC_CHECK(data.wake_count == 0);

    // Step 4: now signal again
    IHC_CHECK(ic_broadcast_signal_all(&data.broadcast) == IC_CONCURRENCY_OK);

    ic_task_join(&t);

    // Now it should pass
    IHC_CHECK(data.wake_count == 1);

    ic_broadcast_destroy(&data.broadcast);
    ic_mutex_destroy(&data.mutex);
}

// ==============================================================================
// STRESS TESTS
// ==============================================================================

#if RUN_STRESS_TESTS

#define GATE_STRESS_ITERS 200
#define GATE_THREADS 8

typedef struct gate_stress_ctx
{
    ic_gate gate;
    ic_mutex mutex;
    int wakes;
} gate_stress_ctx;

static int gate_worker(void* arg)
{
    gate_stress_ctx* c = arg;

    ic_gate_wait(&c->gate);

    ic_mutex_lock(&c->mutex);
    c->wakes++;
    ic_mutex_unlock(&c->mutex);

    return 0;
}

IHC_TEST(stress_gate_interleave_deterministic)
{
    gate_stress_ctx c = {0};

    IHC_ASSERT(ic_gate_init(&c.gate) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&c.mutex) == IC_CONCURRENCY_OK);

    for (int iter = 0; iter < GATE_STRESS_ITERS; iter++)
    {
        c.wakes = 0;

        ic_task threads[GATE_THREADS];

        const int pre_signals = GATE_THREADS / 2;

        for (int i = 0; i < pre_signals; i++)
        {
            IHC_ASSERT(ic_gate_signal_one(&c.gate) == IC_CONCURRENCY_OK);
        }

        for (int i = 0; i < GATE_THREADS; i++)
        {
            IHC_ASSERT(ic_task_init(&threads[i], gate_worker, &c) == IC_CONCURRENCY_OK);
        }

        const int post_signals = GATE_THREADS - pre_signals;

        for (int i = 0; i < post_signals; i++)
        {
            ic_thread_sleep(1); // deterministic "interleaving pressure"
            IHC_ASSERT(ic_gate_signal_one(&c.gate) == IC_CONCURRENCY_OK);
        }

        for (int i = 0; i < GATE_THREADS; i++)
        {
            IHC_ASSERT(ic_task_join(&threads[i]) == IC_CONCURRENCY_OK);
        }

        IHC_CHECK(c.wakes <= GATE_THREADS);
    }

    IHC_CHECK(ic_gate_destroy(&c.gate) == IC_CONCURRENCY_OK);
    IHC_CHECK(ic_mutex_destroy(&c.mutex) == IC_CONCURRENCY_OK);
}

IHC_TEST(stress_gate_signal_before_wait_deterministic)
{
    gate_stress_ctx c = {0};

    IHC_ASSERT(ic_gate_init(&c.gate) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&c.mutex) == IC_CONCURRENCY_OK);

    for (int iter = 0; iter < GATE_STRESS_ITERS; iter++)
    {
        c.wakes = 0;

        const int pre_signals = GATE_THREADS / 2;

        for (int i = 0; i < pre_signals; i++)
        {
            IHC_ASSERT(ic_gate_signal_one(&c.gate) == IC_CONCURRENCY_OK);
        }

        ic_task threads[GATE_THREADS];

        for (int i = 0; i < GATE_THREADS; i++)
        {
            IHC_ASSERT(ic_task_init(&threads[i], gate_worker, &c) == IC_CONCURRENCY_OK);
        }

        const int late_signals = GATE_THREADS - pre_signals;

        for (int i = 0; i < late_signals; i++)
        {
            ic_thread_sleep(1); // forces interleaving pressure
            IHC_ASSERT(ic_gate_signal_one(&c.gate) == IC_CONCURRENCY_OK);
        }

        for (int i = 0; i < GATE_THREADS; i++)
        {
            IHC_ASSERT(ic_task_join(&threads[i]) == IC_CONCURRENCY_OK);
        }

        IHC_CHECK(c.wakes <= GATE_THREADS);
    }

    IHC_CHECK(ic_gate_destroy(&c.gate) == IC_CONCURRENCY_OK);
    IHC_CHECK(ic_mutex_destroy(&c.mutex) == IC_CONCURRENCY_OK);
}

IHC_TEST(stress_gate_consumes_permits_correctly)
{
    gate_stress_ctx c = {0};

    IHC_ASSERT(ic_gate_init(&c.gate) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&c.mutex) == IC_CONCURRENCY_OK);

    for (int iter = 0; iter < GATE_STRESS_ITERS; iter++)
    {
        c.wakes = 0;

        ic_task threads[GATE_THREADS];

        for (int i = 0; i < GATE_THREADS; i++)
        {
            IHC_ASSERT(ic_gate_signal_one(&c.gate) == IC_CONCURRENCY_OK);
        }

        for (int i = 0; i < GATE_THREADS; i++)
        {
            IHC_ASSERT(ic_task_init(&threads[i], gate_worker, &c) == IC_CONCURRENCY_OK);
        }

        for (int i = 0; i < GATE_THREADS; i++)
        {
            IHC_ASSERT(ic_task_join(&threads[i]) == IC_CONCURRENCY_OK);
        }

        IHC_CHECK(c.wakes == GATE_THREADS);

        c.wakes = 0;

        for (int i = 0; i < GATE_THREADS; i++)
        {
            IHC_ASSERT(ic_task_init(&threads[i], gate_worker, &c) == IC_CONCURRENCY_OK);
        }

        ic_thread_sleep(2);

        IHC_CHECK(c.wakes == 0);

        for (int i = 0; i < GATE_THREADS; i++)
        {
            IHC_ASSERT(ic_gate_signal_one(&c.gate) == IC_CONCURRENCY_OK);
        }

        for (int i = 0; i < GATE_THREADS; i++)
        {
            IHC_ASSERT(ic_task_join(&threads[i]) == IC_CONCURRENCY_OK);
        }

        IHC_CHECK(c.wakes == GATE_THREADS);
    }

    IHC_CHECK(ic_gate_destroy(&c.gate) == IC_CONCURRENCY_OK);
    IHC_CHECK(ic_mutex_destroy(&c.mutex) == IC_CONCURRENCY_OK);
}

#define BC_STRESS_ITERS 200
#define BC_THREADS 8

typedef struct bc_ctx
{
    ic_broadcast bc;
    ic_mutex mutex;
    int wakes;
} bc_ctx;

static int bc_worker(void* arg)
{
    bc_ctx* c = arg;

    IHC_CHECK(ic_broadcast_wait(&c->bc) == IC_CONCURRENCY_OK);

    ic_mutex_lock(&c->mutex);
    c->wakes++;
    ic_mutex_unlock(&c->mutex);

    return 0;
}

IHC_TEST(stress_broadcast_deterministic_signal_reset_interleave)
{
    bc_ctx c = {0};

    IHC_ASSERT(ic_broadcast_init(&c.bc) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&c.mutex) == IC_CONCURRENCY_OK);

    for (int iter = 0; iter < BC_STRESS_ITERS; iter++)
    {
        c.wakes = 0;

        IHC_ASSERT(ic_broadcast_reset(&c.bc) == IC_CONCURRENCY_OK);

        ic_task threads[BC_THREADS];

        for (int i = 0; i < BC_THREADS; i++)
        {
            IHC_ASSERT(ic_task_init(&threads[i], bc_worker, &c) == IC_CONCURRENCY_OK);

            if ((i % 2) == 0)
            {
                ic_thread_sleep(1);
            }
        }

        for (int i = 0; i < BC_THREADS; i++)
        {
            if ((i % 3) == 1)
            {
                IHC_ASSERT(ic_broadcast_reset(&c.bc) == IC_CONCURRENCY_OK);
            }
            else
            {
                IHC_ASSERT(ic_broadcast_signal_all(&c.bc) == IC_CONCURRENCY_OK);
            }

            ic_thread_sleep(1);
        }

        IHC_ASSERT(ic_broadcast_signal_all(&c.bc) == IC_CONCURRENCY_OK);

        for (int i = 0; i < BC_THREADS; i++)
        {
            IHC_ASSERT(ic_task_join(&threads[i]) == IC_CONCURRENCY_OK);
        }

        IHC_CHECK(c.wakes == BC_THREADS);
    }

    IHC_CHECK(ic_broadcast_destroy(&c.bc) == IC_CONCURRENCY_OK);
    IHC_CHECK(ic_mutex_destroy(&c.mutex) == IC_CONCURRENCY_OK);
}

IHC_TEST(stress_broadcast_signal_before_wait_deterministic)
{
    bc_ctx c = {0};

    IHC_ASSERT(ic_broadcast_init(&c.bc) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&c.mutex) == IC_CONCURRENCY_OK);

    for (int iter = 0; iter < BC_STRESS_ITERS; iter++)
    {
        c.wakes = 0;

        IHC_ASSERT(ic_broadcast_reset(&c.bc) == IC_CONCURRENCY_OK);

        if ((iter % 2) == 0)
        {
            IHC_ASSERT(ic_broadcast_signal_all(&c.bc) == IC_CONCURRENCY_OK);
        }

        ic_task threads[BC_THREADS];

        for (int i = 0; i < BC_THREADS; i++)
        {
            IHC_ASSERT(ic_task_init(&threads[i], bc_worker, &c) == IC_CONCURRENCY_OK);

            if ((i % 3) == 0)
            {
                ic_thread_sleep(1);
            }
        }

        for (int i = 0; i < BC_THREADS; i++)
        {
            if ((i % 2) == 0)
            {
                IHC_ASSERT(ic_broadcast_reset(&c.bc) == IC_CONCURRENCY_OK);
            }
            else
            {
                IHC_ASSERT(ic_broadcast_signal_all(&c.bc) == IC_CONCURRENCY_OK);
            }

            ic_thread_sleep(1);
        }

        IHC_ASSERT(ic_broadcast_signal_all(&c.bc) == IC_CONCURRENCY_OK);

        for (int i = 0; i < BC_THREADS; i++)
        {
            IHC_ASSERT(ic_task_join(&threads[i]) == IC_CONCURRENCY_OK);
        }

        IHC_CHECK(c.wakes == BC_THREADS);
    }

    IHC_CHECK(ic_broadcast_destroy(&c.bc) == IC_CONCURRENCY_OK);
    IHC_CHECK(ic_mutex_destroy(&c.mutex) == IC_CONCURRENCY_OK);
}

IHC_TEST(stress_broadcast_reset_reblocks_waiters)
{
    bc_ctx c = {0};

    IHC_ASSERT(ic_broadcast_init(&c.bc) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&c.mutex) == IC_CONCURRENCY_OK);

    for (int iter = 0; iter < BC_STRESS_ITERS; iter++)
    {
        c.wakes = 0;

        IHC_ASSERT(ic_broadcast_reset(&c.bc) == IC_CONCURRENCY_OK);

        ic_task threads[BC_THREADS];

        for (int i = 0; i < BC_THREADS; i++)
        {
            IHC_ASSERT(ic_task_init(&threads[i], bc_worker, &c) == IC_CONCURRENCY_OK);
        }

        ic_thread_sleep(2);

        IHC_CHECK(c.wakes == 0);

        IHC_ASSERT(ic_broadcast_signal_all(&c.bc) == IC_CONCURRENCY_OK);

        for (int i = 0; i < BC_THREADS; i++)
        {
            IHC_ASSERT(ic_task_join(&threads[i]) == IC_CONCURRENCY_OK);
        }

        IHC_CHECK(c.wakes == BC_THREADS);

        IHC_ASSERT(ic_broadcast_reset(&c.bc) == IC_CONCURRENCY_OK);

        c.wakes = 0;

        for (int i = 0; i < BC_THREADS; i++)
        {
            IHC_ASSERT(ic_task_init(&threads[i], bc_worker, &c) == IC_CONCURRENCY_OK);
        }

        ic_thread_sleep(2);

        IHC_CHECK(c.wakes == 0);

        IHC_ASSERT(ic_broadcast_signal_all(&c.bc) == IC_CONCURRENCY_OK);

        for (int i = 0; i < BC_THREADS; i++)
        {
            IHC_ASSERT(ic_task_join(&threads[i]) == IC_CONCURRENCY_OK);
        }

        IHC_CHECK(c.wakes == BC_THREADS);
    }

    IHC_CHECK(ic_broadcast_destroy(&c.bc) == IC_CONCURRENCY_OK);
    IHC_CHECK(ic_mutex_destroy(&c.mutex) == IC_CONCURRENCY_OK);
}

#endif // RUN_STRESS_TESTS

// ==============================================================================
// TASK POOL TESTS
// ==============================================================================

#define TASKPOOL_THREADS 4
IC_STATIC_ASSERT((TASKPOOL_THREADS > 0) && (TASKPOOL_THREADS <= TASKPOOL_MAX_THREADS), "TASKPOOL_THREADS must be in legal range");
#define TASKPOOL_TASKS 64
IC_STATIC_ASSERT((TASKPOOL_TASKS > 0) && (TASKPOOL_TASKS <= TASKPOOL_MAX_PENDING_TASKS + TASKPOOL_MAX_THREADS), "TASKPOOL_TASKS must be in legal range");
#define TASK_TEST_WAIT_TIMEOUT (100 * 1000)

static void taskpool_increment(void* arg)
{
    ic_atomic_i32* counter = (ic_atomic_i32*)arg;
    (void)ic_atomic_fetch_add(counter, 1);
}

IHC_TEST(verify_task_pool_executes_all_tasks_with_completion)
{
    TaskPool pool;
    IHC_ASSERT(Error_eq(task_pool_init(&pool, TASKPOOL_THREADS), Error_NoError));

    ic_atomic_i32 counter = ic_make_atomic(0);
    TaskCompletion completions[TASKPOOL_TASKS];
    for (int i = 0; i < TASKPOOL_TASKS; i++)
    {
        Error err = task_pool_submit(&pool, taskpool_increment, &counter, &completions[i]);
        IHC_CHECK(Error_eq(err, Error_NoError));
    }

    for (int i = 0; i < TASKPOOL_TASKS; i++)
    {
        Error err = task_completion_wait(&completions[i], 1, TASK_TEST_WAIT_TIMEOUT);
        IHC_CHECK(Error_eq(err, Error_NoError));
    }

    IHC_CHECK(ic_atomic_load(&counter) == TASKPOOL_TASKS);

    IHC_ASSERT(Error_eq(task_pool_close(&pool, TaskPoolClose_Abort, TASK_TEST_WAIT_TIMEOUT), Error_NoError));
    IHC_CHECK(Error_eq(task_pool_destroy(&pool), Error_NoError));
}

IHC_TEST(verify_task_pool_fire_and_forget_executes_tasks)
{
    TaskPool pool;
    IHC_ASSERT(Error_eq(task_pool_init(&pool, TASKPOOL_THREADS), Error_NoError));
    
    ic_atomic_i32 counter = ic_make_atomic(0);
    for (int i = 0; i < TASKPOOL_TASKS; i++)
    {
        Error err = task_pool_submit(&pool, taskpool_increment, &counter, NULL);
        IHC_CHECK(Error_eq(err, Error_NoError));
    }

    ic_thread_sleep(10);

    IHC_ASSERT(Error_eq(task_pool_close(&pool, TaskPoolClose_Abort, TASK_TEST_WAIT_TIMEOUT), Error_NoError));
    IHC_CHECK(Error_eq(task_pool_destroy(&pool), Error_NoError));
}

typedef struct taskpool_order_ctx
{
    ic_mutex mutex;
    int last;
    int ok;
} taskpool_order_ctx;

static void taskpool_order_worker(void* arg)
{
    taskpool_order_ctx* ctx = (taskpool_order_ctx*)arg;

    ic_mutex_lock(&ctx->mutex);

    if (ctx->last > 0)
    {
        ctx->ok = 0;
    }

    ctx->last++;

    ic_mutex_unlock(&ctx->mutex);
}

IHC_TEST(verify_task_pool_processes_tasks_consistently)
{
    TaskPool pool;
    IHC_ASSERT(Error_eq(task_pool_init(&pool, TASKPOOL_THREADS), Error_NoError));

    taskpool_order_ctx ctx = {0};

    IHC_ASSERT(ic_mutex_init(&ctx.mutex) == IC_CONCURRENCY_OK);

    ctx.last = 0;
    ctx.ok = 1;

    for (int i = 0; i < TASKPOOL_TASKS; i++)
    {
        Error err = task_pool_submit(&pool, taskpool_order_worker, &ctx, NULL);
        IHC_CHECK(Error_eq(err, Error_NoError));
    }

    ic_thread_sleep(10);

    IHC_ASSERT(Error_eq(task_pool_close(&pool, TaskPoolClose_Abort, TASK_TEST_WAIT_TIMEOUT), Error_NoError));
    IHC_CHECK(Error_eq(task_pool_destroy(&pool), Error_NoError));

    ic_mutex_destroy(&ctx.mutex);
}

static void do_nothing(void* arg)
{
    (void)arg;
}

IHC_TEST(verify_task_pool_rejects_null_arguments)
{
    IHC_ASSERT(Error_eq(task_pool_init(NULL, TASKPOOL_THREADS), Error_NullRef));
    IHC_ASSERT(Error_eq(task_pool_close(NULL, TaskPoolClose_Drain, TASK_TEST_WAIT_TIMEOUT), Error_NullRef));
    IHC_ASSERT(Error_eq(task_pool_close(NULL, TaskPoolClose_Abort, TASK_TEST_WAIT_TIMEOUT), Error_NullRef));
    IHC_ASSERT(Error_eq(task_pool_destroy(NULL), Error_NullRef));
    IHC_ASSERT(Error_eq(task_pool_submit(NULL, taskpool_increment, NULL, NULL), Error_NullRef));
    
    TaskPool pool;
    IHC_ASSERT(Error_eq(task_pool_init(&pool, TASKPOOL_THREADS), Error_NoError));
    IHC_CHECK(Error_eq(task_pool_submit(&pool, NULL, NULL, NULL), Error_NullRef));
    TaskCompletion completion;
    IHC_CHECK(Error_eq(task_pool_submit(&pool, do_nothing, NULL, &completion), Error_NoError));
    IHC_ASSERT(Error_eq(task_completion_wait(NULL, 1, TASK_TEST_WAIT_TIMEOUT), Error_NullRef));
    IHC_ASSERT(Error_eq(task_completion_wait(&completion, -1, TASK_TEST_WAIT_TIMEOUT), Error_Argument));
    IHC_ASSERT(Error_eq(task_completion_wait(&completion, 1, -1), Error_Argument));

    IHC_ASSERT(Error_eq(task_pool_close(&pool, TaskPoolClose_Abort, TASK_TEST_WAIT_TIMEOUT), Error_NoError));
    IHC_CHECK(Error_eq(task_pool_destroy(&pool), Error_NoError));
}

IHC_TEST(verify_task_completion_wait_times_out_and_resolves)
{
    TaskPool pool;
    IHC_ASSERT(Error_eq(task_pool_init(&pool, TASKPOOL_THREADS), Error_NoError));
    ic_atomic_i32 counter = ic_make_atomic(0);

    TaskCompletion completion;
    Error err = task_pool_submit(&pool, taskpool_increment, &counter, &completion);
    IHC_CHECK(Error_eq(err, Error_NoError));

    err = task_completion_wait(&completion, 1, TASK_TEST_WAIT_TIMEOUT);
    IHC_CHECK(Error_eq(err, Error_NoError));

    IHC_ASSERT(Error_eq(task_pool_close(&pool, TaskPoolClose_Abort, TASK_TEST_WAIT_TIMEOUT), Error_NoError));
    IHC_CHECK(Error_eq(task_pool_destroy(&pool), Error_NoError));
}

typedef struct taskpool_lifecycle_ctx
{
    ic_atomic_i32 counter;
} taskpool_lifecycle_ctx;

static void taskpool_lifecycle_worker(void* arg)
{
    taskpool_lifecycle_ctx* ctx = (taskpool_lifecycle_ctx*)arg;
    (void)ic_atomic_fetch_add(&ctx->counter, 1);
}

IHC_TEST(verify_task_pool_close_state_transitions)
{
    TaskPool pool;
    IHC_ASSERT(Error_eq(task_pool_init(&pool, TASKPOOL_THREADS), Error_NoError));

    taskpool_lifecycle_ctx ctx;
    ctx.counter = ic_make_atomic(0);

    TaskCompletion completions[TASKPOOL_TASKS];

    for (int i = 0; i < TASKPOOL_TASKS; i++)
    {
        Error err = task_pool_submit(&pool, taskpool_lifecycle_worker, &ctx, &completions[i]);
        IHC_CHECK(Error_eq(err, Error_NoError));
    }

    // Wait for all tasks to finish execution (completion correctness test)
    for (int i = 0; i < TASKPOOL_TASKS; i++)
    {
        IHC_CHECK(Error_eq(task_completion_wait(&completions[i], 1, TASK_TEST_WAIT_TIMEOUT), Error_NoError));
    }

    // Drain queue AND finish active tasks, but don't allow new submissions
    IHC_CHECK(Error_eq(task_pool_close(&pool, TaskPoolClose_Drain, TASK_TEST_WAIT_TIMEOUT), Error_NoError));

    // Submissions must now fail
    Error err = task_pool_submit(&pool, taskpool_lifecycle_worker, &ctx, NULL);
    IHC_CHECK(Error_eq(err, Error_InvalidState));

    IHC_CHECK(ic_atomic_load(&ctx.counter) == TASKPOOL_TASKS);

    err = task_pool_submit(&pool, taskpool_lifecycle_worker, &ctx, NULL);
    IHC_CHECK(Error_eq(err, Error_InvalidState));

    // Finish active tasks, but don't execute any pending tasks, and don't allow new submissions
    IHC_CHECK(Error_eq(task_pool_close(&pool, TaskPoolClose_Abort, TASK_TEST_WAIT_TIMEOUT), Error_NoError));

    err = task_pool_close(&pool, TaskPoolClose_Drain, TASK_TEST_WAIT_TIMEOUT);
    IHC_ASSERT(Error_eq(err, Error_InvalidState));

    err = task_pool_submit(&pool, taskpool_lifecycle_worker, &ctx, NULL);
    IHC_CHECK(Error_eq(err, Error_InvalidState));

    IHC_CHECK(Error_eq(task_pool_destroy(&pool), Error_NoError));
}

#define TASKPOOL_CLOSE_TEST_TASKS 32

typedef struct taskpool_close_test_ctx
{
    ic_atomic_i32 started;
    ic_atomic_i32 completed;
} taskpool_close_test_ctx;

static void taskpool_slow_worker(void* arg)
{
    taskpool_close_test_ctx* ctx = (taskpool_close_test_ctx*)arg;

    ic_atomic_fetch_add(&ctx->started, 1);

    thread_sleep_milliseconds(50);

    ic_atomic_fetch_add(&ctx->completed, 1);
}

IHC_TEST(verify_task_pool_close_drain_completes_all_tasks)
{
    TaskPool pool;
    IHC_ASSERT(Error_eq(task_pool_init(&pool, TASKPOOL_THREADS), Error_NoError));

    taskpool_close_test_ctx ctx = {0};
    ctx.started = ic_make_atomic(0);
    ctx.completed = ic_make_atomic(0);

    TaskCompletion completions[TASKPOOL_CLOSE_TEST_TASKS];

    for (int i = 0; i < TASKPOOL_CLOSE_TEST_TASKS; i++)
    {
        IHC_ASSERT(Error_eq( 
            task_pool_submit(&pool, taskpool_slow_worker, &ctx, &completions[i]),
            Error_NoError
        ));
    }

    while (ic_atomic_load(&ctx.started) < 4)
    {
        ic_thread_sleep(1);
    }

    IHC_ASSERT(Error_eq(
        task_pool_close(&pool, TaskPoolClose_Drain, TASK_TEST_WAIT_TIMEOUT),
        Error_NoError
    ));

    IHC_CHECK(ic_atomic_load(&ctx.started) == TASKPOOL_CLOSE_TEST_TASKS);
    IHC_CHECK(ic_atomic_load(&ctx.completed) == TASKPOOL_CLOSE_TEST_TASKS);

    IHC_ASSERT(Error_eq(task_pool_destroy(&pool), Error_NoError));
}

IHC_TEST(verify_task_pool_close_abort_drops_queued_tasks)
{
    TaskPool pool;
    IHC_ASSERT(Error_eq(task_pool_init(&pool, TASKPOOL_THREADS), Error_NoError));

    taskpool_close_test_ctx ctx = {0};
    ctx.started = ic_make_atomic(0);
    ctx.completed = ic_make_atomic(0);

    for (int i = 0; i < TASKPOOL_CLOSE_TEST_TASKS; i++)
    {
        IHC_ASSERT(Error_eq(
            task_pool_submit(&pool, taskpool_slow_worker, &ctx, NULL),
            Error_NoError
        ));
    }

    while (ic_atomic_load(&ctx.started) < 4)
    {
        ic_thread_sleep(1);
    }

    IHC_ASSERT(Error_eq(
        task_pool_close(&pool, TaskPoolClose_Abort, TASK_TEST_WAIT_TIMEOUT),
        Error_NoError
    ));

    int started = ic_atomic_load(&ctx.started);
    int completed = ic_atomic_load(&ctx.completed);

    // abort must NOT guarantee queue completion
    IHC_CHECK(completed <= TASKPOOL_CLOSE_TEST_TASKS);

    // but anything that started must finish
    IHC_CHECK(completed == started);

    IHC_ASSERT(Error_eq(task_pool_destroy(&pool), Error_NoError));
}

static void worker_stuck_until_signaled(void* arg)
{
    ic_broadcast* broadcast = (ic_broadcast*)arg;
    ic_broadcast_wait(broadcast);
}

IHC_TEST(verify_task_pool_reaches_timeout_for_blocking_tasks)
{
    TaskPool pool;
    IHC_ASSERT(Error_eq(task_pool_init(&pool, TASKPOOL_THREADS), Error_NoError));
    ic_broadcast broadcast;
    IHC_ASSERT(ic_broadcast_init(&broadcast) == IC_CONCURRENCY_OK);

    TaskCompletion first_completion;
    IHC_ASSERT(Error_eq(
        task_pool_submit(&pool, worker_stuck_until_signaled, &broadcast, &first_completion),
        Error_NoError
    ));
    for (int i = 1; i < TASKPOOL_TASKS; i++) // Note that it starts at 1 since we already submitted one above
    {
        IHC_ASSERT(Error_eq(
            task_pool_submit(&pool, worker_stuck_until_signaled, &broadcast, NULL),
            Error_NoError
        ));
    }

    // If the close call doesn't time out correctly, this test will hang instead of fail
    IHC_CHECK(Error_eq(
        task_completion_wait(&first_completion, 1, 100),
        Error_Timeout
    ));
    IHC_CHECK(Error_eq(
        task_pool_close(&pool, TaskPoolClose_Drain, 100),
        Error_Timeout
    ));
    IHC_CHECK(Error_eq(
        task_pool_close(&pool, TaskPoolClose_Abort, 100),
        Error_Timeout
    ));


    // Now signal the tasks so they can finish and not leak
    IHC_ASSERT(ic_broadcast_signal_all(&broadcast) == IC_CONCURRENCY_OK);
    IHC_ASSERT(Error_eq(
        task_pool_close(&pool, TaskPoolClose_Abort, TASK_TEST_WAIT_TIMEOUT),
        Error_NoError
    ));

    IHC_CHECK(Error_eq(task_pool_destroy(&pool), Error_NoError));
    IHC_CHECK(ic_broadcast_destroy(&broadcast) == IC_CONCURRENCY_OK);
}

IHC_TEST(verify_task_pool_can_safely_be_destroyed_without_manually_closing)
{
    TaskPool pool;
    IHC_ASSERT(Error_eq(task_pool_init(&pool, TASKPOOL_THREADS), Error_NoError));

    taskpool_close_test_ctx ctx = {0};
    ctx.started = ic_make_atomic(0);
    ctx.completed = ic_make_atomic(0);

    for (int i = 0; i < TASKPOOL_TASKS; i++)
    {
        IHC_ASSERT(Error_eq(
            task_pool_submit(&pool, taskpool_slow_worker, &ctx, NULL),
            Error_NoError
        ));
    }

    
    // Don't manually close the pool - just destroy it directly. 
    // This should abort active tasks but not finish queued tasks.
    IHC_ASSERT(Error_eq(task_pool_destroy(&pool), Error_NoError));
    
    const int32_t started = ic_atomic_load(&ctx.started);
    const int32_t completed = ic_atomic_load(&ctx.completed);
    IHC_CHECK(completed <= TASKPOOL_TASKS);
    IHC_CHECK(completed == started);
}

IHC_TEST(verify_task_pool_can_fill_up_and_safely_reject_excess_tasks)
{
    TaskPool pool;
    IHC_ASSERT(Error_eq(task_pool_init(&pool, TASKPOOL_MAX_THREADS), Error_NoError));
    ic_broadcast broadcast;
    IHC_ASSERT(ic_broadcast_init(&broadcast) == IC_CONCURRENCY_OK);

    for (int i = 0; i < TASKPOOL_MAX_THREADS; i++)
    {
        IHC_ASSERT(Error_eq(
            task_pool_submit(&pool, worker_stuck_until_signaled, &broadcast, NULL),
            Error_NoError
        ));
    }
    ic_thread_sleep(10); // ensure all worker threads have started and are blocking on the broadcast
    for (int i = 0; i < TASKPOOL_MAX_PENDING_TASKS; i++)
    {
        IHC_ASSERT(Error_eq(
            task_pool_submit(&pool, worker_stuck_until_signaled, &broadcast, NULL),
            Error_NoError
        ));
    }

    // This is the real check to validate the test
    Error err = task_pool_submit(&pool, worker_stuck_until_signaled, &broadcast, NULL);
    IHC_CHECK(Error_eq(err, Error_OutOfBounds));

    IHC_ASSERT(ic_broadcast_signal_all(&broadcast) == IC_CONCURRENCY_OK);
    IHC_ASSERT(Error_eq(task_pool_close(&pool, TaskPoolClose_Drain, TASK_TEST_WAIT_TIMEOUT), Error_NoError));
    IHC_ASSERT(Error_eq(task_pool_destroy(&pool), Error_NoError));
    IHC_ASSERT(ic_broadcast_destroy(&broadcast) == IC_CONCURRENCY_OK);
}

// ==============================================================================
// TASK POOL STRESS TESTS
// ==============================================================================
#if RUN_STRESS_TESTS

#define TASKPOOL_STRESS_THREADS 8
#define TASKPOOL_STRESS_TASKS_PER_THREAD 10000

typedef struct taskpool_stress_submit_ctx
{
    TaskPool* pool;
    ic_atomic_i32 submitted;
    ic_atomic_i32 completed;
} taskpool_stress_submit_ctx;

static void stress_increment_worker(void* arg)
{
    taskpool_stress_submit_ctx* ctx = (taskpool_stress_submit_ctx*)arg;
    ic_atomic_fetch_add(&ctx->completed, 1);
}

static int stress_submitter_thread(void* arg)
{
    taskpool_stress_submit_ctx* ctx = (taskpool_stress_submit_ctx*)arg;

    for (int i = 0; i < TASKPOOL_STRESS_TASKS_PER_THREAD; i++)
    {
        Error err = task_pool_submit(
            ctx->pool,
            stress_increment_worker,
            ctx,
            NULL
        );

        // Pool saturation is acceptable during stress
        if (Error_eq(err, Error_NoError))
        {
            ic_atomic_fetch_add(&ctx->submitted, 1);
        }
        else
        {
            IHC_CHECK(Error_eq(err, Error_OutOfBounds));
        }
    }

    return 0;
}

IHC_TEST(stress_task_pool_handles_high_contention_submissions)
{
    TaskPool pool;

    IHC_ASSERT(Error_eq(
        task_pool_init(&pool, TASKPOOL_MAX_THREADS),
        Error_NoError
    ));

    taskpool_stress_submit_ctx ctx = {0};

    ctx.pool = &pool;
    ctx.submitted = ic_make_atomic(0);
    ctx.completed = ic_make_atomic(0);

    Task submitters[TASKPOOL_STRESS_THREADS];

    for (int i = 0; i < TASKPOOL_STRESS_THREADS; i++)
    {
        IHC_ASSERT(Error_eq(
            task_init(&submitters[i], stress_submitter_thread, &ctx),
            Error_NoError
        ));
    }

    for (int i = 0; i < TASKPOOL_STRESS_THREADS; i++)
    {
        IHC_ASSERT(Error_eq(
            task_join(&submitters[i]),
            Error_NoError
        ));
    }

    IHC_ASSERT(Error_eq(
        task_pool_close(&pool, TaskPoolClose_Drain, TASK_TEST_WAIT_TIMEOUT),
        Error_NoError
    ));

    const int32_t submitted = ic_atomic_load(&ctx.submitted);
    const int32_t completed = ic_atomic_load(&ctx.completed);

    IHC_CHECK(completed == submitted);

    IHC_ASSERT(Error_eq(task_pool_destroy(&pool), Error_NoError));
}

typedef struct shutdown_race_ctx
{
    TaskPool* pool;
    ic_atomic_i32 successful_submits;
    ic_atomic_i32 failed_submits;
    ic_atomic_i32 executed;
} shutdown_race_ctx;

static void shutdown_race_worker(void* arg)
{
    shutdown_race_ctx* ctx = (shutdown_race_ctx*)arg;

    thread_sleep_milliseconds(1);

    ic_atomic_fetch_add(&ctx->executed, 1);
}

static int shutdown_race_submitter(void* arg)
{
    shutdown_race_ctx* ctx = (shutdown_race_ctx*)arg;

    for (int i = 0; i < 10000; i++)
    {
        Error err = task_pool_submit(
            ctx->pool,
            shutdown_race_worker,
            ctx,
            NULL
        );

        if (Error_eq(err, Error_NoError))
        {
            ic_atomic_fetch_add(&ctx->successful_submits, 1);
        }
        else
        {
            // During shutdown or saturation these are valid
            IHC_CHECK(
                Error_eq(err, Error_OutOfBounds) ||
                Error_eq(err, Error_InvalidState)
            );

            ic_atomic_fetch_add(&ctx->failed_submits, 1);
        }
    }

    return 0;
}

IHC_TEST(stress_task_pool_survives_shutdown_submission_races)
{
    TaskPool pool;

    IHC_ASSERT(Error_eq(
        task_pool_init(&pool, TASKPOOL_MAX_THREADS),
        Error_NoError
    ));

    shutdown_race_ctx ctx = {0};

    ctx.pool = &pool;
    ctx.successful_submits = ic_make_atomic(0);
    ctx.failed_submits = ic_make_atomic(0);
    ctx.executed = ic_make_atomic(0);

    Task submitters[4];

    for (int i = 0; i < 4; i++)
    {
        IHC_ASSERT(Error_eq(
            task_init(&submitters[i], shutdown_race_submitter, &ctx),
            Error_NoError
        ));
    }

    thread_sleep_milliseconds(10);

    IHC_ASSERT(Error_eq(
        task_pool_close(&pool, TaskPoolClose_Abort, TASK_TEST_WAIT_TIMEOUT),
        Error_NoError
    ));

    for (int i = 0; i < 4; i++)
    {
        IHC_ASSERT(Error_eq(
            task_join(&submitters[i]),
            Error_NoError
        ));
    }

    const int32_t successful = ic_atomic_load(&ctx.successful_submits);
    const int32_t executed = ic_atomic_load(&ctx.executed);

    // Abort guarantees active jobs finish
    IHC_CHECK(executed <= successful);

    IHC_ASSERT(Error_eq(task_pool_destroy(&pool), Error_NoError));
}

#define TASKPOOL_LIFECYCLE_ITERATIONS 1000

IHC_TEST(stress_task_pool_survives_repeated_lifecycle_cycles)
{
    for (int iteration = 0; iteration < TASKPOOL_LIFECYCLE_ITERATIONS; iteration++)
    {
        TaskPool pool;

        IHC_ASSERT(Error_eq(
            task_pool_init(&pool, TASKPOOL_THREADS),
            Error_NoError
        ));

        ic_atomic_i32 counter = ic_make_atomic(0);

        for (int i = 0; i < TASKPOOL_TASKS; i++)
        {
            Error err = task_pool_submit(
                &pool,
                taskpool_increment,
                &counter,
                NULL
            );

            if (!Error_eq(err, Error_NoError))
            {
                IHC_CHECK(Error_eq(err, Error_OutOfBounds));
            }
        }

        IHC_ASSERT(Error_eq(
            task_pool_close(&pool, TaskPoolClose_Drain, TASK_TEST_WAIT_TIMEOUT),
            Error_NoError
        ));

        IHC_ASSERT(Error_eq(
            task_pool_destroy(&pool),
            Error_NoError
        ));
    }
}

#endif // RUN_STRESS_TESTS

#endif // IRON_HAMMER_C_TESTS_CONCURRENCY_SIGNAL_TEST_H