#ifndef IC_CONCURRENCY_H
#define IC_CONCURRENCY_H

/*
===============================================================================
IC CONCURRENCY HEADER

C Compatibility:
- C99+ supported
- GCC / Clang / MSVC supported

Dependencies:
- ic_inline.h        -> IC_HEADER_FUNC
- threads.h/stdatomic.h/pthread.h/Windows.h 
  depending on backend

Overview
--------
This header provides a portable concurrency layer consisting of:

- ic_atomic_i32  (atomic integer type)
- ic_task        (threaded task abstraction)
- ic_mutex       (mutex abstraction)
- ic_thread_sleep (cross-platform thread sleep function)

The structs exist with associated functions below it.

===============================================================================

TAGS
----------------------------------------
IC_USE_C11_THREADS_AND_ATOMICS
Define this before including the header to use C11 threads and atomics if available. 
The library will fall back to other implementations if C11 is not supported, but if C11 is used
it will assume a fully implemented C11 threads and atomics library is available and will not compile if it is not.

IC_CONCURRENCY_NULLPTR_PANIC(msg)
Define this macro to customize the behavior when a null pointer is passed to a concurrency function. 
By default, it does nothing, but you can set it to log an error, abort the program, or any other behavior you prefer.

ATOMICS
----------------------------------------
struct ic_atomic_i32;
ic_atomic_i32 ic_make_atomic(const int32_t value);
    Theoretically safer that ic_make_atomic: int ic_atomic_init(ic_atomic_i32* const out_atom, const int32_t value);
int32_t ic_atomic_load(const ic_atomic_i32* const atom);
void ic_atomic_store(ic_atomic_i32* const atom, const int32_t value);
int32_t ic_atomic_fetch_add(ic_atomic_i32* const atom, const int32_t value);
int32_t ic_atomic_exchange(ic_atomic_i32* const atom, const int32_t value);

TASKS
----------------------------------------
struct ic_task;
typedef int (*ic_task_function)(void* arg);
int ic_task_init(ic_task* const out_t, const ic_task_function func, void* const arg);
int ic_task_is_running(const ic_task* const t);
int ic_task_get_result(const ic_task* const t, int* const out_result);
int ic_task_join(ic_task* const t);

THREAD LOCKS (MUTEXES)
----------------------------------------
struct ic_mutex;
int  ic_mutex_init(ic_mutex* const out_m);
void ic_mutex_lock(ic_mutex* const m);
int  ic_mutex_trylock(ic_mutex* const m);
void ic_mutex_unlock(ic_mutex* const m);
int  ic_mutex_destroy(ic_mutex* const m);

SLEEP
----------------------------------------
void ic_thread_sleep(const int32_t milliseconds);

===============================================================================
ERROR CODES

IC_CONCURRENCY_OK               0
IC_CONCURRENCY_NULLREF          1
IC_CONCURRENCY_FAILURE          2
IC_CONCURRENCY_ALREADY_JOINED   3
IC_CONCURRENCY_ALREADY_LOCKED   4

===============================================================================
*/

#include "ic_inline.h"
#include "ic_static_assert.h"
#include <stdint.h>

#ifndef IC_CONCURRENCY_NULLPTR_PANIC
    #define IC_CONCURRENCY_NULLPTR_PANIC(msg) ((void)0)
#endif

// ==============================================================================
//  ERROR CODES
// ==============================================================================
#define IC_CONCURRENCY_OK               0
#define IC_CONCURRENCY_NULLREF          1
#define IC_CONCURRENCY_FAILURE          2
#define IC_CONCURRENCY_ALREADY_JOINED   3
#define IC_CONCURRENCY_ALREADY_LOCKED   4
// ==============================================================================
//  THREAD BACKEND
// ==============================================================================
#if defined(IC_USE_C11_THREADS_AND_ATOMICS)

    #if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
        #include <threads.h>
        #define IC_THREAD_C11
    #else
        #error "IC_CONCURRENCY ERROR: IC_USE_C11_THREADS_AND_ATOMICS requires a C11 compiler."
    #endif


#elif defined(_WIN32)

    #define IC_THREAD_WIN
    #include <Windows.h>


#elif defined(__unix__) || defined(__APPLE__)

    #define IC_THREAD_PTHREAD
    #include <pthread.h>


#else

    #error "IC_CONCURRENCY ERROR: No supported threading backend (Win / pthread / optional C11)."

#endif

// ##############################################################################
//  ATOMICS
// ##############################################################################

#if defined(IC_USE_C11_THREADS_AND_ATOMICS) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) && !defined(_MSC_VER)

    #define IC_ATOMIC_C11
    #include <stdatomic.h>

    typedef struct ic_atomic_i32 {
        _Atomic int32_t UNSAFE_PRIVATE_ACCESS_STATE_VALUE;
    } ic_atomic_i32;

#elif defined(__GNUC__) || defined(__clang__)

    #define IC_ATOMIC_GCC

    typedef struct ic_atomic_i32 {
        int32_t UNSAFE_PRIVATE_ACCESS_STATE_VALUE;
    } ic_atomic_i32;

#elif defined(_MSC_VER)

    #define IC_ATOMIC_MSVC
    #include <Windows.h>

    typedef struct ic_atomic_i32 {
        volatile LONG UNSAFE_PRIVATE_ACCESS_STATE_VALUE;
    } ic_atomic_i32;

#else

    #error "IC_CONCURRENCY ERROR: No supported atomic backend (C11 atomics / GCC built-ins / MSVC Interlocked)."

#endif

IC_STATIC_ASSERT(sizeof(ic_atomic_i32) == sizeof(int32_t), "ic_atomic_i32 must be the same size as int32_t");

// ==============================================================================
//  ATOMIC API
// ==============================================================================
IC_HEADER_FUNC int ic_atomic_init(ic_atomic_i32* const out_atom, const int32_t value)
{
    if (!out_atom)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("ic_atomic_init: out_atom is null");
        return IC_CONCURRENCY_NULLREF;
    }

#if defined(IC_ATOMIC_C11)
    atomic_init(&out_atom->UNSAFE_PRIVATE_ACCESS_STATE_VALUE, value);

#elif defined(IC_ATOMIC_MSVC)
    out_atom->UNSAFE_PRIVATE_ACCESS_STATE_VALUE = value;

#elif defined(IC_ATOMIC_GCC)
    out_atom->UNSAFE_PRIVATE_ACCESS_STATE_VALUE = value;

#endif

    return IC_CONCURRENCY_OK;
}

IC_HEADER_FUNC ic_atomic_i32 ic_make_atomic(const int32_t value)
{
    ic_atomic_i32 atomic;
    (void)ic_atomic_init(&atomic, value);
    return atomic;
}

IC_HEADER_FUNC int32_t ic_atomic_load(const ic_atomic_i32* const atom)
{
    if (!atom)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("ic_atomic_load: atom is null");
        return 0;
    }

#if defined(IC_ATOMIC_C11)
    return atomic_load(&atom->UNSAFE_PRIVATE_ACCESS_STATE_VALUE);

#elif defined(IC_ATOMIC_MSVC)
    // InterlockedCompareExchange is not const-correct.
    // Casts away const, but does NOT modify the value (safe).
    return InterlockedCompareExchange((volatile LONG*)(void*)(&atom->UNSAFE_PRIVATE_ACCESS_STATE_VALUE), 0, 0);

#elif defined(IC_ATOMIC_GCC)
    return __atomic_load_n(&atom->UNSAFE_PRIVATE_ACCESS_STATE_VALUE, __ATOMIC_SEQ_CST);

#endif
}

IC_HEADER_FUNC void ic_atomic_store(ic_atomic_i32* const atom, const int32_t value)
{
    if (!atom)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("ic_atomic_store: atom is null");
        return;
    }

#if defined(IC_ATOMIC_C11)
    atomic_store(&atom->UNSAFE_PRIVATE_ACCESS_STATE_VALUE, value);

#elif defined(IC_ATOMIC_MSVC)
    InterlockedExchange(&atom->UNSAFE_PRIVATE_ACCESS_STATE_VALUE, value);

#elif defined(IC_ATOMIC_GCC)
    __atomic_store_n(&atom->UNSAFE_PRIVATE_ACCESS_STATE_VALUE, value, __ATOMIC_SEQ_CST);

#endif
}

IC_HEADER_FUNC int32_t ic_atomic_fetch_add(ic_atomic_i32* const atom, const int32_t value)
{
    if (!atom)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("ic_atomic_fetch_add: atom is null");
        return 0;
    }

#if defined(IC_ATOMIC_C11)
    return atomic_fetch_add(&atom->UNSAFE_PRIVATE_ACCESS_STATE_VALUE, value);

#elif defined(IC_ATOMIC_MSVC)
    return InterlockedExchangeAdd(&atom->UNSAFE_PRIVATE_ACCESS_STATE_VALUE, value);

#elif defined(IC_ATOMIC_GCC)
    return __atomic_fetch_add(&atom->UNSAFE_PRIVATE_ACCESS_STATE_VALUE, value, __ATOMIC_SEQ_CST);

#endif
}

IC_HEADER_FUNC int32_t ic_atomic_exchange(ic_atomic_i32* const atom, const int32_t value)
{
    if (!atom)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("ic_atomic_exchange: atom is null");
        return 0;
    }

#if defined(IC_ATOMIC_C11)

    return atomic_exchange(&atom->UNSAFE_PRIVATE_ACCESS_STATE_VALUE, value);

#elif defined(IC_ATOMIC_MSVC)

    return InterlockedExchange(&atom->UNSAFE_PRIVATE_ACCESS_STATE_VALUE, value);

#elif defined(IC_ATOMIC_GCC)

    return __atomic_exchange_n(&atom->UNSAFE_PRIVATE_ACCESS_STATE_VALUE, value, __ATOMIC_SEQ_CST);

#endif
}

// ==============================================================================
//  TASK SYSTEM
// ==============================================================================

typedef int (*ic_task_function)(void* arg);

typedef struct ic_task {

    ic_atomic_i32 UNSAFE_PRIVATE_ACCESS_IS_RUNNING;
    ic_atomic_i32 UNSAFE_PRIVATE_ACCESS_IS_JOINED;

    ic_task_function UNSAFE_PRIVATE_ACCESS_FUNCTION;
    void* UNSAFE_PRIVATE_ACCESS_ARGUMENT;

    int UNSAFE_PRIVATE_ACCESS_RESULT;

#if defined(IC_THREAD_C11)
    thrd_t UNSAFE_PRIVATE_ACCESS_THREAD_HANDLE;

#elif defined(IC_THREAD_PTHREAD)
    pthread_t UNSAFE_PRIVATE_ACCESS_THREAD_HANDLE;

#elif defined(IC_THREAD_WIN)
    HANDLE UNSAFE_PRIVATE_ACCESS_THREAD_HANDLE;
    DWORD UNSAFE_PRIVATE_ACCESS_THREAD_ID;
#endif

} ic_task;

// ==============================================================================
//  TASK TRAMPOLINES
// ==============================================================================

#if defined(IC_THREAD_C11)
IC_HEADER_FUNC int ic_task_trampoline(void* arg)
{
    ic_task* t = (ic_task*)arg;

    ic_atomic_store(&t->UNSAFE_PRIVATE_ACCESS_IS_RUNNING, 1);

    t->UNSAFE_PRIVATE_ACCESS_RESULT =
        t->UNSAFE_PRIVATE_ACCESS_FUNCTION(t->UNSAFE_PRIVATE_ACCESS_ARGUMENT);

    ic_atomic_store(&t->UNSAFE_PRIVATE_ACCESS_IS_RUNNING, 0);

    return t->UNSAFE_PRIVATE_ACCESS_RESULT;
}
#endif

#if defined(IC_THREAD_PTHREAD)
IC_HEADER_FUNC void* ic_task_trampoline(void* arg)
{
    ic_task* t = (ic_task*)arg;

    ic_atomic_store(&t->UNSAFE_PRIVATE_ACCESS_IS_RUNNING, 1);

    t->UNSAFE_PRIVATE_ACCESS_RESULT =
        t->UNSAFE_PRIVATE_ACCESS_FUNCTION(t->UNSAFE_PRIVATE_ACCESS_ARGUMENT);

    ic_atomic_store(&t->UNSAFE_PRIVATE_ACCESS_IS_RUNNING, 0);

    return NULL;
}
#endif

#if defined(IC_THREAD_WIN)
IC_HEADER_FUNC DWORD WINAPI ic_task_trampoline(LPVOID arg)
{
    ic_task* t = (ic_task*)arg;

    ic_atomic_store(&t->UNSAFE_PRIVATE_ACCESS_IS_RUNNING, 1);

    t->UNSAFE_PRIVATE_ACCESS_RESULT =
        t->UNSAFE_PRIVATE_ACCESS_FUNCTION(t->UNSAFE_PRIVATE_ACCESS_ARGUMENT);

    ic_atomic_store(&t->UNSAFE_PRIVATE_ACCESS_IS_RUNNING, 0);

    return 0;
}
#endif

// ==============================================================================
//  TASK API
// ==============================================================================

IC_HEADER_FUNC int ic_task_init(ic_task* const out_t, const ic_task_function func, void* const arg)
{
    if (!out_t || !func)
    {
        return IC_CONCURRENCY_NULLREF;
    }

    out_t->UNSAFE_PRIVATE_ACCESS_FUNCTION = func;
    out_t->UNSAFE_PRIVATE_ACCESS_ARGUMENT = arg;
    out_t->UNSAFE_PRIVATE_ACCESS_RESULT = 0;

    ic_atomic_init(&out_t->UNSAFE_PRIVATE_ACCESS_IS_RUNNING, 0);
    ic_atomic_init(&out_t->UNSAFE_PRIVATE_ACCESS_IS_JOINED, 0);

#if defined(IC_THREAD_C11)

    if (thrd_create(&out_t->UNSAFE_PRIVATE_ACCESS_THREAD_HANDLE,
                    ic_task_trampoline, out_t) != thrd_success)
    {
        return IC_CONCURRENCY_FAILURE;
    }

#elif defined(IC_THREAD_PTHREAD)

    if (pthread_create(&out_t->UNSAFE_PRIVATE_ACCESS_THREAD_HANDLE,
                       NULL, ic_task_trampoline, out_t) != 0)
    {
        return IC_CONCURRENCY_FAILURE;
    }

#elif defined(IC_THREAD_WIN)

    out_t->UNSAFE_PRIVATE_ACCESS_THREAD_HANDLE =
        CreateThread(NULL, 0, ic_task_trampoline, out_t, 0,
                     &out_t->UNSAFE_PRIVATE_ACCESS_THREAD_ID);

    if (!out_t->UNSAFE_PRIVATE_ACCESS_THREAD_HANDLE)
    {
        return IC_CONCURRENCY_FAILURE;
    }

#endif

    return IC_CONCURRENCY_OK;
}

IC_HEADER_FUNC int ic_task_join(ic_task* const t)
{
    if (!t)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("ic_task_join: t is null");
        return IC_CONCURRENCY_NULLREF;
    }

    if (ic_atomic_load(&t->UNSAFE_PRIVATE_ACCESS_IS_JOINED) == 1)
    {
        return IC_CONCURRENCY_ALREADY_JOINED;
    }

    ic_atomic_store(&t->UNSAFE_PRIVATE_ACCESS_IS_JOINED, 1);

#if defined(IC_THREAD_C11)
    if (thrd_join(t->UNSAFE_PRIVATE_ACCESS_THREAD_HANDLE, NULL) != thrd_success)
    {
        return IC_CONCURRENCY_FAILURE;
    }

#elif defined(IC_THREAD_PTHREAD)
    if (pthread_join(t->UNSAFE_PRIVATE_ACCESS_THREAD_HANDLE, NULL) != 0)
    {
        return IC_CONCURRENCY_FAILURE;
    }

#elif defined(IC_THREAD_WIN)
    DWORD wait_result = WaitForSingleObject(t->UNSAFE_PRIVATE_ACCESS_THREAD_HANDLE, INFINITE);
    CloseHandle(t->UNSAFE_PRIVATE_ACCESS_THREAD_HANDLE);
    if (wait_result != WAIT_OBJECT_0)
    {
        return IC_CONCURRENCY_FAILURE;
    }

#endif

    return IC_CONCURRENCY_OK;
}

IC_HEADER_FUNC int ic_task_is_running(const ic_task* const t)
{
    if (!t)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("ic_task_is_running: t is null");
        return 0;
    }

    return ic_atomic_load(&t->UNSAFE_PRIVATE_ACCESS_IS_RUNNING);
}

IC_HEADER_FUNC int ic_task_get_result(const ic_task* const t, int* const out_result)
{
    if (!t || !out_result)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("ic_task_get_result: t or out_result is null");
        return IC_CONCURRENCY_NULLREF;
    }

    if (ic_atomic_load(&t->UNSAFE_PRIVATE_ACCESS_IS_RUNNING))
    {
        return IC_CONCURRENCY_FAILURE;
    }

    *out_result = t->UNSAFE_PRIVATE_ACCESS_RESULT;
    return IC_CONCURRENCY_OK;
}

// ==============================================================================
//  LOCK SYSTEM
// ==============================================================================

typedef struct ic_mutex {

#if defined(IC_THREAD_C11)
    mtx_t UNSAFE_PRIVATE_ACCESS_MUTEX;

#elif defined(IC_THREAD_PTHREAD)
    pthread_mutex_t UNSAFE_PRIVATE_ACCESS_MUTEX;

#elif defined(IC_THREAD_WIN)
    SRWLOCK UNSAFE_PRIVATE_ACCESS_MUTEX;

#endif

} ic_mutex;

// ==============================================================================
//  MUTEX API
// ==============================================================================

IC_HEADER_FUNC int ic_mutex_init(ic_mutex* const out_m)
{
    if (!out_m)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("ic_mutex_init: out_m is null");
        return IC_CONCURRENCY_NULLREF;
    }

#if defined(IC_THREAD_C11)

    if (mtx_init(&out_m->UNSAFE_PRIVATE_ACCESS_MUTEX, mtx_plain) != thrd_success)
    {
        return IC_CONCURRENCY_FAILURE;
    }

#elif defined(IC_THREAD_PTHREAD)

    if (pthread_mutex_init(&out_m->UNSAFE_PRIVATE_ACCESS_MUTEX, NULL) != 0)
    {
        return IC_CONCURRENCY_FAILURE;
    }

#elif defined(IC_THREAD_WIN)

    InitializeSRWLock(&out_m->UNSAFE_PRIVATE_ACCESS_MUTEX);

#endif

    return IC_CONCURRENCY_OK;
}

IC_HEADER_FUNC void ic_mutex_lock(ic_mutex* const m)
{
    if (!m)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("ic_mutex_lock: m is null");
        return;
    }

#if defined(IC_THREAD_C11)
    if (mtx_lock(&m->UNSAFE_PRIVATE_ACCESS_MUTEX) != thrd_success)
    {
        return;
    }
#elif defined(IC_THREAD_PTHREAD)
    if (pthread_mutex_lock(&m->UNSAFE_PRIVATE_ACCESS_MUTEX) != 0)
    {
        return;
    }
#elif defined(IC_THREAD_WIN)
    AcquireSRWLockExclusive(&m->UNSAFE_PRIVATE_ACCESS_MUTEX);
#endif
}

IC_HEADER_FUNC int ic_mutex_trylock(ic_mutex* const m)
{
    if (!m)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("ic_mutex_trylock: m is null");
        return IC_CONCURRENCY_NULLREF;
    }

#if defined(IC_THREAD_C11)

    if (mtx_trylock(&m->UNSAFE_PRIVATE_ACCESS_MUTEX) != thrd_success)
    {
        return IC_CONCURRENCY_ALREADY_LOCKED;
    }

#elif defined(IC_THREAD_PTHREAD)

    if (pthread_mutex_trylock(&m->UNSAFE_PRIVATE_ACCESS_MUTEX) != 0)
    {
        return IC_CONCURRENCY_ALREADY_LOCKED;
    }

#elif defined(IC_THREAD_WIN)

    if (!TryAcquireSRWLockExclusive(&m->UNSAFE_PRIVATE_ACCESS_MUTEX))
    {
        return IC_CONCURRENCY_ALREADY_LOCKED;
    }

#endif

    return IC_CONCURRENCY_OK;
}

IC_HEADER_FUNC void ic_mutex_unlock(ic_mutex* const m)
{
    if (!m)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("ic_mutex_unlock: m is null");
        return;
    }

#if defined(IC_THREAD_C11)
    mtx_unlock(&m->UNSAFE_PRIVATE_ACCESS_MUTEX);
#elif defined(IC_THREAD_PTHREAD)
    pthread_mutex_unlock(&m->UNSAFE_PRIVATE_ACCESS_MUTEX);
#elif defined(IC_THREAD_WIN)
    ReleaseSRWLockExclusive(&m->UNSAFE_PRIVATE_ACCESS_MUTEX);
#endif
}

IC_HEADER_FUNC int ic_mutex_destroy(ic_mutex* const m)
{
    if (!m)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("ic_mutex_destroy: m is null");
        return IC_CONCURRENCY_NULLREF;
    }

    int mutex_result = IC_CONCURRENCY_OK;

#if defined(IC_THREAD_C11)

    mtx_destroy(&m->UNSAFE_PRIVATE_ACCESS_MUTEX);

#elif defined(IC_THREAD_PTHREAD)

    if (pthread_mutex_destroy(&m->UNSAFE_PRIVATE_ACCESS_MUTEX) != 0)
    {
        mutex_result = IC_CONCURRENCY_FAILURE;
    }

#elif defined(IC_THREAD_WIN)
    // No destroy required for SRWLOCK (kernel-managed, no state teardown)

#endif

    return mutex_result;
}

// ==============================================================================
//  THREAD SLEEP
// ==============================================================================

IC_HEADER_FUNC void ic_thread_sleep(const int32_t milliseconds)
{
    if (milliseconds <= 0)
    {
        return; // safe no-op
    }

#if defined(IC_THREAD_C11)

    struct timespec ts;
    ts.tv_sec  = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    thrd_sleep(&ts, NULL);

#elif defined(IC_THREAD_PTHREAD)

    struct timespec ts;
    ts.tv_sec  = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);

#elif defined(IC_THREAD_WIN)

    Sleep((DWORD)milliseconds);

#endif
}

#endif // IC_CONCURRENCY_H