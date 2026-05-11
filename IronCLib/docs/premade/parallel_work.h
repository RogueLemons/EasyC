#ifndef PREMADE_PARALLEL_WORK_H
#define PREMADE_PARALLEL_WORK_H

#ifndef IC_CONCURRENCY_NULLPTR_PANIC
#include <stdio.h>
#define IC_CONCURRENCY_NULLPTR_PANIC(msg) \
    do { fprintf(stderr, "[CONCURRENCY] NULLPTR: %s\n", msg); } while (0)
#endif

#define USE_C11_THREADING 0 // Set to 1 to use C11 threads and atomics, otherwise will use platform-specific backends (Win / pthreads)
#if USE_C11_THREADING
    #define IC_USE_C11_THREADS_AND_ATOMICS
#endif

#include "ironclib/ic_inline.h"
#include "ironclib/ic_concurrency.h"
#include "ironclib/ic_concurrency_signal.h"
#include "ironclib/ic_typenum.h"
#include "global_error.h"
#include <stdint.h>
#include <stdbool.h>

IC_HEADER_FUNC Error concurrency_result_to_error(const int concurrency_result)
{
    switch (concurrency_result)
    {
        case IC_CONCURRENCY_OK: return Error_NoError;
        case IC_CONCURRENCY_NULLREF: return Error_NullRef;
        case IC_CONCURRENCY_FAILURE: return Error_Runtime;
        case IC_CONCURRENCY_ALREADY_JOINED: return Error_InvalidState;
        case IC_CONCURRENCY_ALREADY_LOCKED: return Error_InvalidState;
        default: return Error_Unknown;
    }
}

// SLEEP API
IC_HEADER_FUNC void thread_sleep_milliseconds(const int32_t milliseconds)               { ic_thread_sleep(milliseconds); }

// ATOMIC API
typedef ic_atomic_i32 AtomicI32;
IC_HEADER_FUNC Error atomic_init(AtomicI32* const out_atom, const int32_t value)        { return concurrency_result_to_error(ic_atomic_init(out_atom, value)); }
IC_HEADER_FUNC AtomicI32 atomic_make(const int32_t value)                               { return ic_make_atomic(value); }
IC_HEADER_FUNC int32_t atomic_load(const AtomicI32* const atom)                         { return ic_atomic_load(atom); }
IC_HEADER_FUNC void atomic_store(AtomicI32* const atom, const int32_t value)            { ic_atomic_store(atom, value); }
IC_HEADER_FUNC int32_t atomic_fetch_add(AtomicI32* const atom, const int32_t value)     { return ic_atomic_fetch_add(atom, value); }
IC_HEADER_FUNC int32_t atomic_exchange(AtomicI32* const atom, const int32_t value)      { return ic_atomic_exchange(atom, value); }

// TASK API
typedef ic_task Task;
typedef ic_task_function TaskFunction;
IC_HEADER_FUNC Error task_init(Task* const out_task, const TaskFunction function, void* const arg)  { return concurrency_result_to_error(ic_task_init(out_task, function, arg)); }
IC_HEADER_FUNC bool task_is_running(const Task* const task)                                         { return (bool)ic_task_is_running(task); }
IC_HEADER_FUNC Error task_get_result(const Task* const task, int* const out_result)                 { return concurrency_result_to_error(ic_task_get_result(task, out_result)); }
IC_HEADER_FUNC Error task_join(Task* const task)                                                    { return concurrency_result_to_error(ic_task_join(task)); }

// MUTEX API
typedef ic_mutex Mutex;
IC_HEADER_FUNC Error mutex_init(Mutex* const out_mutex)         { return concurrency_result_to_error(ic_mutex_init(out_mutex)); }
IC_HEADER_FUNC void mutex_lock(Mutex* const mutex)              { ic_mutex_lock(mutex); } 
IC_HEADER_FUNC Error mutex_trylock(Mutex* const mutex)          { return concurrency_result_to_error(ic_mutex_trylock(mutex)); }
IC_HEADER_FUNC void mutex_unlock(Mutex* const mutex)            { ic_mutex_unlock(mutex); }
IC_HEADER_FUNC Error mutex_destroy(Mutex* const mutex)          { return concurrency_result_to_error(ic_mutex_destroy(mutex)); }

// CONDITION VARIABLE API
typedef ic_condition_variable ConditionVariable;
IC_HEADER_FUNC Error condition_variable_init(ConditionVariable* const cv)                       { return concurrency_result_to_error(ic_condition_variable_init(cv)); }
IC_HEADER_FUNC Error condition_variable_notify_one(ConditionVariable* const cv)                 { return concurrency_result_to_error(ic_condition_variable_notify_one(cv)); }
IC_HEADER_FUNC Error condition_variable_notify_all(ConditionVariable* const cv)                 { return concurrency_result_to_error(ic_condition_variable_notify_all(cv)); }
IC_HEADER_FUNC Error condition_variable_wait(ConditionVariable* const cv, Mutex* const mutex)   { return concurrency_result_to_error(ic_condition_variable_wait(cv, mutex)); }
IC_HEADER_FUNC Error condition_variable_destroy(ConditionVariable* const cv)                    { return concurrency_result_to_error(ic_condition_variable_destroy(cv)); }

// GATE API
typedef ic_gate Gate;
IC_HEADER_FUNC Error gate_init(Gate* const out_gate)            { return concurrency_result_to_error(ic_gate_init(out_gate)); }
IC_HEADER_FUNC Error gate_wait(Gate* const gate)                { return concurrency_result_to_error(ic_gate_wait(gate)); }
IC_HEADER_FUNC Error gate_signal_one(Gate* const gate)          { return concurrency_result_to_error(ic_gate_signal_one(gate)); }
IC_HEADER_FUNC Error gate_destroy(Gate* const gate)             { return concurrency_result_to_error(ic_gate_destroy(gate)); }

// BROADCAST API
typedef ic_broadcast Broadcast;
IC_HEADER_FUNC Error broadcast_init(Broadcast* const out_broadcast)             { return concurrency_result_to_error(ic_broadcast_init(out_broadcast)); }
IC_HEADER_FUNC Error broadcast_wait(Broadcast* const broadcast)                 { return concurrency_result_to_error(ic_broadcast_wait(broadcast)); }
IC_HEADER_FUNC Error broadcast_signal_all(Broadcast* const broadcast)           { return concurrency_result_to_error(ic_broadcast_signal_all(broadcast)); }
IC_HEADER_FUNC Error broadcast_reset(Broadcast* const broadcast)                { return concurrency_result_to_error(ic_broadcast_reset(broadcast)); }
IC_HEADER_FUNC Error broadcast_destroy(Broadcast* const broadcast)              { return concurrency_result_to_error(ic_broadcast_destroy(broadcast)); }

// TASK POOL API

// typedef struct TaskPool TaskPool;
// typedef void (*TaskPoolFunction)(void* arg);
// #ifndef TASKPOOL_MAX_THREADS
// #ifndef TASKPOOL_MAX_PENDING_TASKS
// Error task_pool_init(TaskPool* const out_pool, const uint32_t worker_count);
// Error task_pool_submit(TaskPool* const pool, const TaskPoolFunction func, void* const arg, TaskCompletion* const out_completion);
// Error task_pool_destroy(TaskPool* const pool);

// typedef struct TaskCompletion TaskCompletion;
// Error task_completion_wait(const TaskCompletion* const completion, const int32_t retry_period_ms, const int64_t timeout_ms);

// ==============================================================================
// TASK POOL IMPLEMENTATION
// ==============================================================================

#ifndef TASKPOOL_MAX_THREADS
#define TASKPOOL_MAX_THREADS 8
#endif

#ifndef TASKPOOL_MAX_PENDING_TASKS
#define TASKPOOL_MAX_PENDING_TASKS 1024
#endif

typedef void (*TaskPoolFunction)(void* arg);

typedef enum TaskPoolState
{
    TASKPOOL_RUNNING = 0,
    TASKPOOL_CLOSING_DRAIN = 1,
    TASKPOOL_CLOSING_ABORT = 2

} TaskPoolState;

typedef struct TaskCompletion
{
    AtomicI32 PRIVATE_completed;

} TaskCompletion;

typedef struct TaskPoolJob
{
    TaskPoolFunction PRIVATE_func;
    void* PRIVATE_arg;
    TaskCompletion* PRIVATE_completion;

} TaskPoolJob;

typedef struct TaskPool
{
    Task PRIVATE_workers[TASKPOOL_MAX_THREADS];
    uint32_t PRIVATE_worker_count;

    Mutex PRIVATE_mutex;
    Gate PRIVATE_work_gate;

    AtomicI32 PRIVATE_state;
    AtomicI32 PRIVATE_active_jobs;

    TaskPoolJob PRIVATE_jobs[TASKPOOL_MAX_PENDING_TASKS];

    uint32_t PRIVATE_head;
    uint32_t PRIVATE_tail;
    uint32_t PRIVATE_count;

} TaskPool;

// ==============================================================================
// INTERNAL WORKER
// ==============================================================================

static int PRIVATE_task_pool_worker_main(void* arg)
{
    TaskPool* p = (TaskPool*)arg;

    while (1)
    {
        if (!Error_eq(gate_wait(&p->PRIVATE_work_gate), Error_NoError))
        {
            continue;
        }

        mutex_lock(&p->PRIVATE_mutex);

        const int32_t state = atomic_load(&p->PRIVATE_state);

        // Abort mode ignores remaining queued jobs immediately
        if (state == TASKPOOL_CLOSING_ABORT)
        {
            mutex_unlock(&p->PRIVATE_mutex);
            break;
        }

        // No queued jobs available
        if (p->PRIVATE_count == 0)
        {
            mutex_unlock(&p->PRIVATE_mutex);

            // Drain mode exits once queue becomes empty
            if (state == TASKPOOL_CLOSING_DRAIN)
            {
                break;
            }

            continue;
        }

        TaskPoolJob job = p->PRIVATE_jobs[p->PRIVATE_head];

        p->PRIVATE_head = (p->PRIVATE_head + 1) % TASKPOOL_MAX_PENDING_TASKS;
        p->PRIVATE_count--;

        atomic_fetch_add(&p->PRIVATE_active_jobs, 1);

        mutex_unlock(&p->PRIVATE_mutex);

        job.PRIVATE_func(job.PRIVATE_arg);

        if (job.PRIVATE_completion)
        {
            atomic_store(&job.PRIVATE_completion->PRIVATE_completed, 1);
        }

        (void)atomic_fetch_add(&p->PRIVATE_active_jobs, -1);

        // Wake sleeping workers during drain shutdown so they can exit
        if (atomic_load(&p->PRIVATE_state) == TASKPOOL_CLOSING_DRAIN)
        {
            gate_signal_one(&p->PRIVATE_work_gate);
        }
    }

    return 0;
}

// ==============================================================================
// COMPLETION WAIT
// ==============================================================================

#ifndef TASK_COMPLETION_WAIT_FOREVER
#define TASK_COMPLETION_WAIT_FOREVER 0
#endif

IC_HEADER_FUNC Error task_completion_wait(const TaskCompletion* const completion, const int32_t retry_period_ms, const int64_t timeout_ms)
{
    if (!completion)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("task_completion_wait: completion is null");
        return Error_NullRef;
    }

    if (retry_period_ms < 0)
    {
        return Error_Argument;
    }

    if (timeout_ms < 0)
    {
        return Error_Argument;
    }

    // -------------------------------------------------------------------------
    // FAST SPIN FIRST
    // -------------------------------------------------------------------------

    for (uint32_t i = 0; i < 128; i++)
    {
        if (atomic_load(&completion->PRIVATE_completed))
        {
            return Error_NoError;
        }
    }

    // -------------------------------------------------------------------------
    // WAIT WITH BACKOFF
    // -------------------------------------------------------------------------

    uint32_t it = 0;
    int64_t elapsed_ms = 0;
    int32_t current_sleep_ms = retry_period_ms;

    while (!atomic_load(&completion->PRIVATE_completed))
    {
        thread_sleep_milliseconds(current_sleep_ms);

        if (timeout_ms != TASK_COMPLETION_WAIT_FOREVER)
        {
            // Saturating overflow-safe accumulation
            if (elapsed_ms > INT64_MAX - current_sleep_ms)
            {
                elapsed_ms = INT64_MAX;
            }
            else
            {
                elapsed_ms += current_sleep_ms;
            }

            if (elapsed_ms >= timeout_ms)
            {
                return Error_Timeout;
            }
        }

        it++;

        if ((it % 10) == 0)
        {
            if (current_sleep_ms < 64)
            {
                current_sleep_ms *= 2;

                if (current_sleep_ms > 64)
                {
                    current_sleep_ms = 64;
                }
            }
        }
    }

    return Error_NoError;
}

// ==============================================================================
// INIT
// ==============================================================================

IC_HEADER_FUNC Error task_pool_init(TaskPool* const out_pool, uint32_t worker_count)
{
    if (!out_pool)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("task_pool_init: out_pool is null");
        return Error_NullRef;
    }

    if (worker_count == 0 || worker_count > TASKPOOL_MAX_THREADS)
    {
        return Error_Argument;
    }

    out_pool->PRIVATE_worker_count = worker_count;

    out_pool->PRIVATE_head = 0;
    out_pool->PRIVATE_tail = 0;
    out_pool->PRIVATE_count = 0;

    atomic_store(&out_pool->PRIVATE_state, TASKPOOL_RUNNING);
    atomic_store(&out_pool->PRIVATE_active_jobs, 0);

    Error err = mutex_init(&out_pool->PRIVATE_mutex);

    if (!Error_eq(err, Error_NoError))
    {
        return err;
    }

    err = gate_init(&out_pool->PRIVATE_work_gate);

    if (!Error_eq(err, Error_NoError))
    {
        mutex_destroy(&out_pool->PRIVATE_mutex);
        return err;
    }

    for (uint32_t i = 0; i < worker_count; i++)
    {
        err = task_init(&out_pool->PRIVATE_workers[i], PRIVATE_task_pool_worker_main, out_pool);

        if (!Error_eq(err, Error_NoError))
        {
            atomic_store(&out_pool->PRIVATE_state, TASKPOOL_CLOSING_ABORT);

            for (uint32_t j = 0; j < worker_count; j++)
            {
                (void)gate_signal_one(&out_pool->PRIVATE_work_gate);
            }

            for (uint32_t j = 0; j < i; j++)
            {
                (void)task_join(&out_pool->PRIVATE_workers[j]);
            }

            (void)gate_destroy(&out_pool->PRIVATE_work_gate);
            (void)mutex_destroy(&out_pool->PRIVATE_mutex);

            return err;
        }
    }

    return Error_NoError;
}

// ==============================================================================
// CLOSE MODE ENUM
// ==============================================================================

#define TASK_POOL_CLOSE_LIST(X, Type) \
    X(Type, Drain, 0, "Finish queued and active tasks") \
    X(Type, Abort, 1, "Finish only active tasks")

IC_TYPENUM_FULL(TaskPoolClose, uint8_t, TASK_POOL_CLOSE_LIST)

// ==============================================================================
// CLOSE
// ==============================================================================

IC_HEADER_FUNC Error task_pool_close(TaskPool* const pool, const TaskPoolClose close_mode, const int64_t timeout_ms)
{
    if (!pool)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("task_pool_close: pool is null");
        return Error_NullRef;
    }

    if (timeout_ms < 0 && timeout_ms != TASK_COMPLETION_WAIT_FOREVER)
    {
        return Error_Argument;
    }

    int32_t set_state = -1;

    if (TaskPoolClose_eq(close_mode, TaskPoolClose_Drain))
    {
        if (atomic_load(&pool->PRIVATE_state) == TASKPOOL_CLOSING_ABORT)
        {
            return Error_InvalidState;
        }

        set_state = TASKPOOL_CLOSING_DRAIN;
    }
    else if (TaskPoolClose_eq(close_mode, TaskPoolClose_Abort))
    {
        set_state = TASKPOOL_CLOSING_ABORT;
    }
    else
    {
        return Error_Argument;
    }

    atomic_store(&pool->PRIVATE_state, set_state);

    for (uint32_t i = 0; i < pool->PRIVATE_worker_count; i++)
    {
        const Error res = gate_signal_one(&pool->PRIVATE_work_gate);
        if (!Error_eq(res, Error_NoError))
        {
            return res;
        }
    }

    // ==========================================================================
    // WAIT FOR SHUTDOWN CONDITION
    // ==========================================================================

    uint32_t sleep_ms = 1;
    int64_t elapsed_ms = 0;

    if (set_state == TASKPOOL_CLOSING_DRAIN)
    {
        while (1)
        {
            mutex_lock(&pool->PRIVATE_mutex);

            const uint32_t queued = pool->PRIVATE_count;

            mutex_unlock(&pool->PRIVATE_mutex);

            const int32_t active = atomic_load(&pool->PRIVATE_active_jobs);

            if (queued == 0 && active == 0)
            {
                break;
            }

            thread_sleep_milliseconds(sleep_ms);

            if (timeout_ms != TASK_COMPLETION_WAIT_FOREVER)
            {
                if (elapsed_ms > INT64_MAX - sleep_ms)
                {
                    elapsed_ms = INT64_MAX;
                }
                else
                {
                    elapsed_ms += sleep_ms;
                }

                if (elapsed_ms >= timeout_ms)
                {
                    return Error_Timeout;
                }
            }

            if (sleep_ms < 16)
            {
                sleep_ms *= 2;
            }
        }
    }
    else
    {
        while (atomic_load(&pool->PRIVATE_active_jobs) != 0)
        {
            thread_sleep_milliseconds(sleep_ms);

            if (timeout_ms != TASK_COMPLETION_WAIT_FOREVER)
            {
                if (elapsed_ms > INT64_MAX - sleep_ms)
                {
                    elapsed_ms = INT64_MAX;
                }
                else
                {
                    elapsed_ms += sleep_ms;
                }

                if (elapsed_ms >= timeout_ms)
                {
                    return Error_Timeout;
                }
            }

            if (sleep_ms < 16)
            {
                sleep_ms *= 2;
            }
        }
    }

    // ==========================================================================
    // FINAL WAKEUP FOR EXITING WORKERS
    // ==========================================================================

    for (uint32_t i = 0; i < pool->PRIVATE_worker_count; i++)
    {
        const Error res = gate_signal_one(&pool->PRIVATE_work_gate);
        if (!Error_eq(res, Error_NoError))
        {
            return res;
        }
    }

    return Error_NoError;
}

// ==============================================================================
// SUBMIT
// ==============================================================================

IC_HEADER_FUNC Error task_pool_submit(TaskPool* const pool, const TaskPoolFunction func, void* const arg, TaskCompletion* const out_completion)
{
    if (!pool || !func)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("task_pool_submit: pool or func is null");
        return Error_NullRef;
    }

    if (atomic_load(&pool->PRIVATE_state) != TASKPOOL_RUNNING)
    {
        return Error_InvalidState;
    }

    if (out_completion)
    {
        atomic_store(&out_completion->PRIVATE_completed, 0);
    }

    mutex_lock(&pool->PRIVATE_mutex);

    if (pool->PRIVATE_count >= TASKPOOL_MAX_PENDING_TASKS)
    {
        mutex_unlock(&pool->PRIVATE_mutex);
        return Error_OutOfBounds;
    }

    pool->PRIVATE_jobs[pool->PRIVATE_tail] = (TaskPoolJob)
    {
        .PRIVATE_func = func,
        .PRIVATE_arg = arg,
        .PRIVATE_completion = out_completion
    };

    pool->PRIVATE_tail = (pool->PRIVATE_tail + 1) % TASKPOOL_MAX_PENDING_TASKS;
    pool->PRIVATE_count++;

    mutex_unlock(&pool->PRIVATE_mutex);

    return gate_signal_one(&pool->PRIVATE_work_gate);
}

// ==============================================================================
// DESTROY
// ==============================================================================

IC_HEADER_FUNC Error task_pool_destroy(TaskPool* const pool)
{
    if (!pool)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("task_pool_destroy: pool is null");
        return Error_NullRef;
    }

    const int64_t thirty_seconds = 30 * 1000;
    Error err = task_pool_close(pool, TaskPoolClose_Abort, thirty_seconds);
    if (!Error_eq(err, Error_NoError))
    {
        return err;
    }

    for (uint32_t i = 0; i < pool->PRIVATE_worker_count; i++)
    {
        err = task_join(&pool->PRIVATE_workers[i]);
    }
    if (!Error_eq(err, Error_NoError))
    {
        return Error_Irrecoverable;
    }

    err = gate_destroy(&pool->PRIVATE_work_gate);
    if (!Error_eq(err, Error_NoError))
    {
        return Error_Irrecoverable;
    }

    err = mutex_destroy(&pool->PRIVATE_mutex);
    if (!Error_eq(err, Error_NoError))
    {
        return Error_Irrecoverable;
    }
    return Error_NoError;
}

#endif // PREMADE_PARALLEL_WORK_H