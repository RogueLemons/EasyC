#ifndef PREMADE_PARALLEL_WORK_H
#define PREMADE_PARALLEL_WORK_H

#ifndef IC_CONCURRENCY_NULLPTR_PANIC
#include <stdio.h>
#define IC_CONCURRENCY_NULLPTR_PANIC(msg) \
    do { fprintf(stderr, "[CONCURRENCY] NULLPTR: %s\n", msg); } while (0)
#endif

#include "ironclib/ic_inline.h"
#include "ironclib/ic_concurrency.h"
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
IC_HEADER_FUNC void atomic_store(AtomicI32* const atom, const int32_t value)            { IC_CONCURRENCY_NULLPTR_PANIC("atomic_store: atom is null"); ic_atomic_store(atom, value); }
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
IC_HEADER_FUNC void mutex_lock(Mutex* const mutex)              { IC_CONCURRENCY_NULLPTR_PANIC("mutex_lock: mutex is null"); ic_mutex_lock(mutex); } 
IC_HEADER_FUNC Error mutex_trylock(Mutex* const mutex)          { return concurrency_result_to_error(ic_mutex_trylock(mutex)); }
IC_HEADER_FUNC void mutex_unlock(Mutex* const mutex)            { IC_CONCURRENCY_NULLPTR_PANIC("mutex_unlock: mutex is null"); ic_mutex_unlock(mutex); }
IC_HEADER_FUNC Error mutex_destroy(Mutex* const mutex)          { return concurrency_result_to_error(ic_mutex_destroy(mutex)); }

// CONDITION VARIABLE API
// struct ConditionVariable;
// Error condition_variable_init(ConditionVariable* const cv);
// Error condition_variable_notify_one(ConditionVariable* const cv);
// Error condition_variable_notify_all(ConditionVariable* const cv);
// Error condition_variable_wait(ConditionVariable* const cv, Mutex* const mutex);

#if defined(IC_THREAD_C11)

typedef struct ConditionVariable { cnd_t UNSAFE_PRIVATE_ACCESS_COND; } ConditionVariable;

IC_HEADER_FUNC Error condition_variable_init(ConditionVariable* const cv)
{
    if (!cv)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("condition_variable_init");
        return Error_NullRef;
    }

    return (cnd_init(&cv->UNSAFE_PRIVATE_ACCESS_COND) == thrd_success)
        ? Error_NoError
        : Error_Runtime;
}

IC_HEADER_FUNC Error condition_variable_notify_one(ConditionVariable* const cv)
{
    if (!cv)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("condition_variable_notify_one");
        return Error_NullRef;
    }

    cnd_signal(&cv->UNSAFE_PRIVATE_ACCESS_COND);
    return Error_NoError;
}

IC_HEADER_FUNC Error condition_variable_notify_all(ConditionVariable* const cv)
{
    if (!cv)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("condition_variable_notify_all");
        return Error_NullRef;
    }

    cnd_broadcast(&cv->UNSAFE_PRIVATE_ACCESS_COND);
    return Error_NoError;
}

IC_HEADER_FUNC Error condition_variable_wait(ConditionVariable* const cv, Mutex* const m)
{
    if (!cv || !m)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("condition_variable_wait");
        return Error_NullRef;
    }

    return (cnd_wait(&cv->UNSAFE_PRIVATE_ACCESS_COND, &m->UNSAFE_PRIVATE_ACCESS_MUTEX) == thrd_success)
        ? Error_NoError
        : Error_Runtime;
}

#elif defined(IC_THREAD_PTHREAD)

typedef struct ConditionVariable { pthread_cond_t UNSAFE_PRIVATE_ACCESS_COND; } ConditionVariable;

IC_HEADER_FUNC Error condition_variable_init(ConditionVariable* const cv)
{
    if (!cv)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("condition_variable_init");
        return Error_NullRef;
    }

    return (pthread_cond_init(&cv->UNSAFE_PRIVATE_ACCESS_COND, NULL) == 0)
        ? Error_NoError
        : Error_Runtime;
}

IC_HEADER_FUNC Error condition_variable_notify_one(ConditionVariable* const cv)
{
    if (!cv)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("condition_variable_notify_one");
        return Error_NullRef;
    }

    return (pthread_cond_signal(&cv->UNSAFE_PRIVATE_ACCESS_COND) == 0)
        ? Error_NoError
        : Error_Runtime;
}

IC_HEADER_FUNC Error condition_variable_notify_all(ConditionVariable* const cv)
{
    if (!cv)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("condition_variable_notify_all");
        return Error_NullRef;
    }

    return (pthread_cond_broadcast(&cv->UNSAFE_PRIVATE_ACCESS_COND) == 0)
        ? Error_NoError
        : Error_Runtime;
}

IC_HEADER_FUNC Error condition_variable_wait(ConditionVariable* const cv, Mutex* const m)
{
    if (!cv || !m)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("condition_variable_wait");
        return Error_NullRef;
    }

    return (pthread_cond_wait(&cv->UNSAFE_PRIVATE_ACCESS_COND, &m->UNSAFE_PRIVATE_ACCESS_MUTEX) == 0)
        ? Error_NoError
        : Error_Runtime;
}

#elif defined(IC_THREAD_WIN)

typedef struct ConditionVariable { CONDITION_VARIABLE UNSAFE_PRIVATE_ACCESS_COND; } ConditionVariable;

IC_HEADER_FUNC Error condition_variable_init(ConditionVariable* const cv)
{
    if (!cv)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("condition_variable_init");
        return Error_NullRef;
    }

    InitializeConditionVariable(&cv->UNSAFE_PRIVATE_ACCESS_COND);
    return Error_NoError;
}

IC_HEADER_FUNC Error condition_variable_notify_one(ConditionVariable* const cv)
{
    if (!cv)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("condition_variable_notify_one");
        return Error_NullRef;
    }

    WakeConditionVariable(&cv->UNSAFE_PRIVATE_ACCESS_COND);
    return Error_NoError;
}

IC_HEADER_FUNC Error condition_variable_notify_all(ConditionVariable* const cv)
{
    if (!cv)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("condition_variable_notify_all");
        return Error_NullRef;
    }

    WakeAllConditionVariable(&cv->UNSAFE_PRIVATE_ACCESS_COND);
    return Error_NoError;
}

IC_HEADER_FUNC Error condition_variable_wait(ConditionVariable* const cv, Mutex* const m)
{
    if (!cv || !m)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("condition_variable_wait");
        return Error_NullRef;
    }

    return SleepConditionVariableSRW(
        &cv->UNSAFE_PRIVATE_ACCESS_COND,
        &m->UNSAFE_PRIVATE_ACCESS_MUTEX,
        INFINITE,
        0)
        ? Error_NoError
        : Error_Runtime;
}

#endif

#endif // PREMADE_PARALLEL_WORK_H