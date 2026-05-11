#ifndef IC_CONCURRENCY_SIGNAL_H
#define IC_CONCURRENCY_SIGNAL_H

/*
===============================================================================
IC CONCURRENCY SYNCHRONIZATION PRIMITIVES

This header provides higher-level synchronization primitives built on top of:
- ic_mutex
- ic_condition_variable
as well as a portable condition variable implementation based on the underlying backend.

These abstractions exist to reduce the complexity and common misuse patterns
of raw condition variables by encapsulating stateful synchronization patterns.

The error codes are the same as in ic_concurrency.h, and the same 
IC_CONCURRENCY_NULLPTR_PANIC macro applies to these functions as well.

===============================================================================

TAGS
----------------------------------------
IC_CONCURRENCY_NULLPTR_PANIC(msg)
Macro invoked when a null pointer is passed into a concurrency function.
By default it is a no-op, but may be overridden to log, abort, etc.

===============================================================================

CONDITION VARIABLE API (LOW LEVEL PRIMITIVE)
----------------------------------------
struct ic_condition_variable;
int ic_condition_variable_init(ic_condition_variable* const cv);
int ic_condition_variable_notify_one(ic_condition_variable* const cv);
int ic_condition_variable_notify_all(ic_condition_variable* const cv);
int ic_condition_variable_wait(ic_condition_variable* const cv, ic_mutex* const mutex);
int ic_condition_variable_destroy(ic_condition_variable* const cv);

Notes:
- Waits on the condition variable using the provided mutex.
- This function does NOT perform any predicate checking.
- Users MUST implement a while-loop around this call.

Description:
    Low-level blocking primitive.
    Does not store state.
    Requires external synchronization discipline (mutex + predicate loop).

===============================================================================

GATE API (AUTO-RESET PERMIT-BASED SIGNAL)
----------------------------------------

struct ic_gate;
int ic_gate_init(ic_gate* const out_gate);
int ic_gate_wait(ic_gate* const gate);
int ic_gate_signal_one(ic_gate* const gate);
int ic_gate_destroy(ic_gate* const gate);

Behavior:
- Waits for a single available permit.
- Each call to ic_gate_signal releases exactly one waiting thread.
- If no thread is waiting, the signal is stored as a permit.
- Each permit is consumed by exactly one waiter.

Description:
    Auto-reset synchronization primitive.
    Equivalent in behavior to a counting semaphore with unit increments.
    Ensures one-to-one signaling between producer and consumer.

===============================================================================

BROADCAST API (MANUAL-RESET STICKY SIGNAL)
----------------------------------------
struct ic_broadcast;
int ic_broadcast_init(ic_broadcast* const out_broadcast);
int ic_broadcast_wait(ic_broadcast* const broadcast);
int ic_broadcast_signal_all(ic_broadcast* const broadcast);
int ic_broadcast_reset(ic_broadcast* const broadcast);
int ic_broadcast_destroy(ic_broadcast* const broadcast);

Behavior:
- Waits until broadcast is in a signaled state.
  If already signaled, returns immediately.
- Sets broadcast into a signaled state and wakes all waiting threads.
  State remains signaled until explicitly reset.
- Resets the broadcast back to a non-signaled state.
  Future waiters will block until signaled again.

Description:
    Manual-reset synchronization primitive.
    Once signaled, remains signaled indefinitely until reset().
    Wakes all current and future waiters while signaled.

===============================================================================
*/

#include "ic_concurrency_backend.h"
#include "ic_concurrency.h"
#include <stdint.h>

// ==============================================================================
//  CONDITION VARIABLE
// ==============================================================================

// CONDITION VARIABLE STRUCTURE
typedef struct ic_condition_variable
{
#if defined(IC_THREAD_C11)
    cnd_t UNSAFE_PRIVATE_ACCESS_COND;
#elif defined(IC_THREAD_PTHREAD)
    pthread_cond_t UNSAFE_PRIVATE_ACCESS_COND;
#elif defined(IC_THREAD_WIN)
    CONDITION_VARIABLE UNSAFE_PRIVATE_ACCESS_COND;
#endif
} ic_condition_variable;

// INIT
IC_HEADER_FUNC int ic_condition_variable_init(ic_condition_variable* const cv)
{
    if (!cv)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("condition_variable_init");
        return IC_CONCURRENCY_NULLREF;
    }

#if defined(IC_THREAD_C11)

    return (cnd_init(&cv->UNSAFE_PRIVATE_ACCESS_COND) == thrd_success)
        ? IC_CONCURRENCY_OK
        : IC_CONCURRENCY_FAILURE;

#elif defined(IC_THREAD_PTHREAD)

    return (pthread_cond_init(&cv->UNSAFE_PRIVATE_ACCESS_COND, NULL) == 0)
        ? IC_CONCURRENCY_OK
        : IC_CONCURRENCY_FAILURE;

#elif defined(IC_THREAD_WIN)

    InitializeConditionVariable(&cv->UNSAFE_PRIVATE_ACCESS_COND);
    return IC_CONCURRENCY_OK;

#endif
}

// NOTIFY ONE
IC_HEADER_FUNC int ic_condition_variable_notify_one(ic_condition_variable* const cv)
{
    if (!cv)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("condition_variable_notify_one");
        return IC_CONCURRENCY_NULLREF;
    }

#if defined(IC_THREAD_C11)

    cnd_signal(&cv->UNSAFE_PRIVATE_ACCESS_COND);
    return IC_CONCURRENCY_OK;

#elif defined(IC_THREAD_PTHREAD)

    return (pthread_cond_signal(&cv->UNSAFE_PRIVATE_ACCESS_COND) == 0)
        ? IC_CONCURRENCY_OK
        : IC_CONCURRENCY_FAILURE;

#elif defined(IC_THREAD_WIN)

    WakeConditionVariable(&cv->UNSAFE_PRIVATE_ACCESS_COND);
    return IC_CONCURRENCY_OK;

#endif
}

// NOTIFY ALL
IC_HEADER_FUNC int ic_condition_variable_notify_all(ic_condition_variable* const cv)
{
    if (!cv)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("condition_variable_notify_all");
        return IC_CONCURRENCY_NULLREF;
    }

#if defined(IC_THREAD_C11)

    cnd_broadcast(&cv->UNSAFE_PRIVATE_ACCESS_COND);
    return IC_CONCURRENCY_OK;

#elif defined(IC_THREAD_PTHREAD)

    return (pthread_cond_broadcast(&cv->UNSAFE_PRIVATE_ACCESS_COND) == 0)
        ? IC_CONCURRENCY_OK
        : IC_CONCURRENCY_FAILURE;

#elif defined(IC_THREAD_WIN)

    WakeAllConditionVariable(&cv->UNSAFE_PRIVATE_ACCESS_COND);
    return IC_CONCURRENCY_OK;

#endif
}

// WAIT
IC_HEADER_FUNC int ic_condition_variable_wait(ic_condition_variable* const cv, ic_mutex* const m)
{
    if (!cv || !m)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("condition_variable_wait");
        return IC_CONCURRENCY_NULLREF;
    }

#if defined(IC_THREAD_C11)

    return (cnd_wait(&cv->UNSAFE_PRIVATE_ACCESS_COND, &m->UNSAFE_PRIVATE_ACCESS_MUTEX) == thrd_success)
        ? IC_CONCURRENCY_OK
        : IC_CONCURRENCY_FAILURE;

#elif defined(IC_THREAD_PTHREAD)

    return (pthread_cond_wait(&cv->UNSAFE_PRIVATE_ACCESS_COND, &m->UNSAFE_PRIVATE_ACCESS_MUTEX) == 0)
        ? IC_CONCURRENCY_OK
        : IC_CONCURRENCY_FAILURE;

#elif defined(IC_THREAD_WIN)

    return SleepConditionVariableSRW(&cv->UNSAFE_PRIVATE_ACCESS_COND, &m->UNSAFE_PRIVATE_ACCESS_MUTEX, INFINITE, 0)
        ? IC_CONCURRENCY_OK
        : IC_CONCURRENCY_FAILURE;

#endif
}

// DESTROY
IC_HEADER_FUNC int ic_condition_variable_destroy(ic_condition_variable* const cv)
{
    if (!cv)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("ic_condition_variable_destroy");
        return IC_CONCURRENCY_NULLREF;
    }

#if defined(IC_THREAD_C11)

    cnd_destroy(&cv->UNSAFE_PRIVATE_ACCESS_COND);
    return IC_CONCURRENCY_OK;

#elif defined(IC_THREAD_PTHREAD)

    return (pthread_cond_destroy(&cv->UNSAFE_PRIVATE_ACCESS_COND) == 0)
        ? IC_CONCURRENCY_OK
        : IC_CONCURRENCY_FAILURE;

#elif defined(IC_THREAD_WIN)

    // No-op: CONDITION_VARIABLE has no destroy semantics
    return IC_CONCURRENCY_OK;

#endif
}

// ==============================================================================
//  GATE
// ==============================================================================

// GATE STRUCTURE
typedef struct ic_gate
{
    ic_mutex UNSAFE_PRIVATE_ACCESS_MUTEX;
    ic_condition_variable UNSAFE_PRIVATE_ACCESS_CV;
    uint32_t UNSAFE_PRIVATE_ACCESS_PERMITS; // increased by signal, decreased by wait; wait blocks when permits is zero.

} ic_gate;

// INIT
IC_HEADER_FUNC int ic_gate_init(ic_gate* const out_g)
{
    if (!out_g)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("ic_gate_init");
        return IC_CONCURRENCY_NULLREF;
    }

    out_g->UNSAFE_PRIVATE_ACCESS_PERMITS = 0;

    int r = ic_mutex_init(&out_g->UNSAFE_PRIVATE_ACCESS_MUTEX);
    if (r != IC_CONCURRENCY_OK)
    {
        return r;
    }

    r = ic_condition_variable_init(&out_g->UNSAFE_PRIVATE_ACCESS_CV);
    if (r != IC_CONCURRENCY_OK)
    {
        (void)ic_mutex_destroy(&out_g->UNSAFE_PRIVATE_ACCESS_MUTEX);
        return r;
    }

    return IC_CONCURRENCY_OK;
}

// WAIT
IC_HEADER_FUNC int ic_gate_wait(ic_gate* const g)
{
    if (!g)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("ic_gate_wait");
        return IC_CONCURRENCY_NULLREF;
    }

    ic_mutex_lock(&g->UNSAFE_PRIVATE_ACCESS_MUTEX);

    while (g->UNSAFE_PRIVATE_ACCESS_PERMITS == 0)
    {
        const int r = ic_condition_variable_wait(&g->UNSAFE_PRIVATE_ACCESS_CV, &g->UNSAFE_PRIVATE_ACCESS_MUTEX);

        if (r != IC_CONCURRENCY_OK)
        {
            ic_mutex_unlock(&g->UNSAFE_PRIVATE_ACCESS_MUTEX);
            return r;
        }
    }

    g->UNSAFE_PRIVATE_ACCESS_PERMITS--; 

    ic_mutex_unlock(&g->UNSAFE_PRIVATE_ACCESS_MUTEX);

    return IC_CONCURRENCY_OK;
}

// SIGNAL
IC_HEADER_FUNC int ic_gate_signal_one(ic_gate* const g)
{
    if (!g)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("ic_gate_signal_one");
        return IC_CONCURRENCY_NULLREF;
    }

    ic_mutex_lock(&g->UNSAFE_PRIVATE_ACCESS_MUTEX);

    g->UNSAFE_PRIVATE_ACCESS_PERMITS++;

    const int r = ic_condition_variable_notify_one(&g->UNSAFE_PRIVATE_ACCESS_CV);

    ic_mutex_unlock(&g->UNSAFE_PRIVATE_ACCESS_MUTEX);

    return r;
}

// DESTROY
IC_HEADER_FUNC int ic_gate_destroy(ic_gate* const g)
{
    if (!g)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("ic_gate_destroy");
        return IC_CONCURRENCY_NULLREF;
    }

    const int r1 = ic_condition_variable_destroy(&g->UNSAFE_PRIVATE_ACCESS_CV);
    const int r2 = ic_mutex_destroy(&g->UNSAFE_PRIVATE_ACCESS_MUTEX);
    return (r1 == IC_CONCURRENCY_OK && r2 == IC_CONCURRENCY_OK) ? IC_CONCURRENCY_OK : IC_CONCURRENCY_FAILURE;
}

// ==============================================================================
//  BROADCAST
// ==============================================================================

// BROADCAST STRUCTURE
typedef struct ic_broadcast
{
    ic_mutex UNSAFE_PRIVATE_ACCESS_MUTEX;
    ic_condition_variable UNSAFE_PRIVATE_ACCESS_CV;
    int UNSAFE_PRIVATE_ACCESS_SIGNALED; // 0 = not signaled, 1 = signaled

} ic_broadcast;

// INIT
IC_HEADER_FUNC int ic_broadcast_init(ic_broadcast* const out_b)
{
    if (!out_b)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("ic_broadcast_init");
        return IC_CONCURRENCY_NULLREF;
    }

    out_b->UNSAFE_PRIVATE_ACCESS_SIGNALED = 0;

    int r = ic_mutex_init(&out_b->UNSAFE_PRIVATE_ACCESS_MUTEX);
    if (r != IC_CONCURRENCY_OK)
        return r;

    r = ic_condition_variable_init(&out_b->UNSAFE_PRIVATE_ACCESS_CV);
    if (r != IC_CONCURRENCY_OK)
    {
        ic_mutex_destroy(&out_b->UNSAFE_PRIVATE_ACCESS_MUTEX);
        return r;
    }

    return IC_CONCURRENCY_OK;
}

// WAIT
IC_HEADER_FUNC int ic_broadcast_wait(ic_broadcast* const b)
{
    if (!b)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("ic_broadcast_wait");
        return IC_CONCURRENCY_NULLREF;
    }

    ic_mutex_lock(&b->UNSAFE_PRIVATE_ACCESS_MUTEX);

    while (!b->UNSAFE_PRIVATE_ACCESS_SIGNALED)
    {
        const int r = ic_condition_variable_wait(&b->UNSAFE_PRIVATE_ACCESS_CV, &b->UNSAFE_PRIVATE_ACCESS_MUTEX);

        if (r != IC_CONCURRENCY_OK)
        {
            ic_mutex_unlock(&b->UNSAFE_PRIVATE_ACCESS_MUTEX);
            return r;
        }
    }

    ic_mutex_unlock(&b->UNSAFE_PRIVATE_ACCESS_MUTEX);

    return IC_CONCURRENCY_OK;
}

// SIGNAL
IC_HEADER_FUNC int ic_broadcast_signal_all(ic_broadcast* const b)
{
    if (!b)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("ic_broadcast_signal_all");
        return IC_CONCURRENCY_NULLREF;
    }

    ic_mutex_lock(&b->UNSAFE_PRIVATE_ACCESS_MUTEX);

    b->UNSAFE_PRIVATE_ACCESS_SIGNALED = 1;

    const int r = ic_condition_variable_notify_all(&b->UNSAFE_PRIVATE_ACCESS_CV);

    ic_mutex_unlock(&b->UNSAFE_PRIVATE_ACCESS_MUTEX);

    return r;
}

// RESET
IC_HEADER_FUNC int ic_broadcast_reset(ic_broadcast* const b)
{
    if (!b)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("ic_broadcast_reset");
        return IC_CONCURRENCY_NULLREF;
    }

    ic_mutex_lock(&b->UNSAFE_PRIVATE_ACCESS_MUTEX);

    b->UNSAFE_PRIVATE_ACCESS_SIGNALED = 0;

    ic_mutex_unlock(&b->UNSAFE_PRIVATE_ACCESS_MUTEX);

    return IC_CONCURRENCY_OK;
}

// DESTROY
IC_HEADER_FUNC int ic_broadcast_destroy(ic_broadcast* const b)
{
    if (!b)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("ic_broadcast_destroy");
        return IC_CONCURRENCY_NULLREF;
    }

    const int r1 = ic_condition_variable_destroy(&b->UNSAFE_PRIVATE_ACCESS_CV);
    const int r2 = ic_mutex_destroy(&b->UNSAFE_PRIVATE_ACCESS_MUTEX);
    return (r1 == IC_CONCURRENCY_OK && r2 == IC_CONCURRENCY_OK) ? IC_CONCURRENCY_OK : IC_CONCURRENCY_FAILURE;
}

#endif // IC_CONCURRENCY_SIGNAL_H