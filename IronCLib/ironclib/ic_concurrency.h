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

- ic_atomic_int  (atomic integer type)
- ic_task        (threaded task abstraction)
- ic_lock        (mutex abstraction)

Each struct exists with associated functions below it.

To use C11 atomics and threads, define IC_USE_C11_THREADS_AND_ATOMICS before including this header.
This will still be ignored if the compiler does not support C11.

===============================================================================

ATOMICS
----------------------------------------
struct ic_atomic_int;
int ic_atomic_init(ic_atomic_int* obj, int value);
int ic_atomic_destroy(ic_atomic_int* obj);
int ic_atomic_load(ic_atomic_int* obj);
void ic_atomic_store(ic_atomic_int* obj, int value);
int ic_atomic_fetch_add(ic_atomic_int* obj, int value);

TASKS
----------------------------------------
struct ic_task;
typedef int (*ic_task_function)(void* arg);
int ic_task_init(ic_task* t, ic_task_function func, void* arg);
int ic_task_join(ic_task* t);
int ic_task_is_running(ic_task* t);
int ic_task_get_result(ic_task* t, int* result_out);
int ic_task_destroy(ic_task* t);

THREAD LOCKS
----------------------------------------
struct ic_lock;
int  ic_lock_init(ic_lock* l);
void ic_lock_lock(ic_lock* l);
int  ic_lock_trylock(ic_lock* l);
void ic_lock_unlock(ic_lock* l);
int  ic_lock_destroy(ic_lock* l);

===============================================================================
ERROR CODES

IC_CONCURRENCY_OK               0
IC_CONCURRENCY_NULLREF          1
IC_CONCURRENCY_FAILURE          2
IC_CONCURRENCY_ALREADY_JOINED   3

===============================================================================
*/

#include "ic_inline.h"

/*==============================================================================
  ERROR CODES
==============================================================================*/
#define IC_CONCURRENCY_OK               0
#define IC_CONCURRENCY_NULLREF          1
#define IC_CONCURRENCY_FAILURE          2
#define IC_CONCURRENCY_ALREADY_JOINED   3

/*==============================================================================
  THREAD BACKEND
==============================================================================*/
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

/*##############################################################################
  ATOMICS
##############################################################################*/

#if defined(IC_USE_C11_THREADS_AND_ATOMICS) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) && !defined(_MSC_VER)

    #define IC_ATOMIC_C11
    #include <stdatomic.h>

    typedef struct ic_atomic_int {
        atomic_int UNSAFE_PRIVATE_ACCESS_STATE_VALUE;
    } ic_atomic_int;

#elif defined(__GNUC__) || defined(__clang__)

    #define IC_ATOMIC_GCC

    typedef struct ic_atomic_int {
        int UNSAFE_PRIVATE_ACCESS_STATE_VALUE;
    } ic_atomic_int;

#elif defined(_MSC_VER)

    #define IC_ATOMIC_MSVC
    #include <Windows.h>

    typedef struct ic_atomic_int {
        volatile LONG UNSAFE_PRIVATE_ACCESS_STATE_VALUE;
    } ic_atomic_int;

#else

    #define IC_ATOMIC_MUTEX
    #include <pthread.h>

    typedef struct ic_atomic_int {
        int UNSAFE_PRIVATE_ACCESS_STATE_VALUE;
        pthread_mutex_t UNSAFE_PRIVATE_ACCESS_STATE_MUTEX;
    } ic_atomic_int;

#endif

/*==============================================================================
  ATOMIC API
==============================================================================*/
IC_HEADER_FUNC int ic_atomic_init(ic_atomic_int* obj, int value)
{
    if (!obj)
    {
        return IC_CONCURRENCY_NULLREF;
    }

#if defined(IC_ATOMIC_C11)
    atomic_init(&obj->UNSAFE_PRIVATE_ACCESS_STATE_VALUE, value);

#elif defined(IC_ATOMIC_MSVC)
    obj->UNSAFE_PRIVATE_ACCESS_STATE_VALUE = value;

#elif defined(IC_ATOMIC_GCC)
    obj->UNSAFE_PRIVATE_ACCESS_STATE_VALUE = value;

#elif defined(IC_ATOMIC_MUTEX)
    obj->UNSAFE_PRIVATE_ACCESS_STATE_VALUE = value;

    if (pthread_mutex_init(&obj->UNSAFE_PRIVATE_ACCESS_STATE_MUTEX, NULL) != 0)
    {
        return IC_CONCURRENCY_FAILURE;
    }
#endif

    return IC_CONCURRENCY_OK;
}

IC_HEADER_FUNC int ic_atomic_destroy(ic_atomic_int* obj)
{
    if (!obj)
    {
        return IC_CONCURRENCY_NULLREF;
    }

#if defined(IC_ATOMIC_MUTEX)
    if (pthread_mutex_destroy(&obj->UNSAFE_PRIVATE_ACCESS_STATE_MUTEX) != 0)
    {
        return IC_CONCURRENCY_FAILURE;
    }
#endif

    return IC_CONCURRENCY_OK;
}

IC_HEADER_FUNC int ic_atomic_load(ic_atomic_int* obj)
{
    if (!obj)
    {
        return 0;
    }

#if defined(IC_ATOMIC_C11)
    return atomic_load(&obj->UNSAFE_PRIVATE_ACCESS_STATE_VALUE);

#elif defined(IC_ATOMIC_MSVC)
    return InterlockedCompareExchange(&obj->UNSAFE_PRIVATE_ACCESS_STATE_VALUE, 0, 0);

#elif defined(IC_ATOMIC_GCC)
    return __atomic_load_n(&obj->UNSAFE_PRIVATE_ACCESS_STATE_VALUE, __ATOMIC_SEQ_CST);

#elif defined(IC_ATOMIC_MUTEX)
    int v;

    if (pthread_mutex_lock(&obj->UNSAFE_PRIVATE_ACCESS_STATE_MUTEX) != 0)
    {
        return 0;
    }

    v = obj->UNSAFE_PRIVATE_ACCESS_STATE_VALUE;

    pthread_mutex_unlock(&obj->UNSAFE_PRIVATE_ACCESS_STATE_MUTEX);

    return v;
#endif
}

IC_HEADER_FUNC void ic_atomic_store(ic_atomic_int* obj, int value)
{
    if (!obj)
    {
        return;
    }

#if defined(IC_ATOMIC_C11)
    atomic_store(&obj->UNSAFE_PRIVATE_ACCESS_STATE_VALUE, value);

#elif defined(IC_ATOMIC_MSVC)
    InterlockedExchange(&obj->UNSAFE_PRIVATE_ACCESS_STATE_VALUE, value);

#elif defined(IC_ATOMIC_GCC)
    __atomic_store_n(&obj->UNSAFE_PRIVATE_ACCESS_STATE_VALUE, value, __ATOMIC_SEQ_CST);

#elif defined(IC_ATOMIC_MUTEX)

    if (pthread_mutex_lock(&obj->UNSAFE_PRIVATE_ACCESS_STATE_MUTEX) != 0)
    {
        return;
    }

    obj->UNSAFE_PRIVATE_ACCESS_STATE_VALUE = value;

    pthread_mutex_unlock(&obj->UNSAFE_PRIVATE_ACCESS_STATE_MUTEX);

#endif
}

IC_HEADER_FUNC int ic_atomic_fetch_add(ic_atomic_int* obj, int value)
{
    if (!obj)
    {
        return 0;
    }

#if defined(IC_ATOMIC_C11)
    return atomic_fetch_add(&obj->UNSAFE_PRIVATE_ACCESS_STATE_VALUE, value);

#elif defined(IC_ATOMIC_MSVC)
    return InterlockedExchangeAdd(&obj->UNSAFE_PRIVATE_ACCESS_STATE_VALUE, value);

#elif defined(IC_ATOMIC_GCC)
    return __atomic_fetch_add(&obj->UNSAFE_PRIVATE_ACCESS_STATE_VALUE, value, __ATOMIC_SEQ_CST);

#elif defined(IC_ATOMIC_MUTEX)
    int old_value;

    if (pthread_mutex_lock(&obj->UNSAFE_PRIVATE_ACCESS_STATE_MUTEX) != 0)
    {
        return 0;
    }

    old_value = obj->UNSAFE_PRIVATE_ACCESS_STATE_VALUE;
    obj->UNSAFE_PRIVATE_ACCESS_STATE_VALUE += value;

    pthread_mutex_unlock(&obj->UNSAFE_PRIVATE_ACCESS_STATE_MUTEX);

    return old_value;
#endif
}

/*##############################################################################
  TASK SYSTEM
##############################################################################*/

typedef int (*ic_task_function)(void* arg);

typedef struct ic_task {

    ic_atomic_int UNSAFE_PRIVATE_ACCESS_IS_RUNNING;
    ic_atomic_int UNSAFE_PRIVATE_ACCESS_IS_JOINED;

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

/*==============================================================================
  TASK TRAMPOLINES
==============================================================================*/

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

/*==============================================================================
  TASK API
==============================================================================*/

IC_HEADER_FUNC int ic_task_init(ic_task* t, ic_task_function func, void* arg)
{
    if (!t || !func)
    {
        return IC_CONCURRENCY_NULLREF;
    }

    t->UNSAFE_PRIVATE_ACCESS_FUNCTION = func;
    t->UNSAFE_PRIVATE_ACCESS_ARGUMENT = arg;
    t->UNSAFE_PRIVATE_ACCESS_RESULT = 0;

    ic_atomic_init(&t->UNSAFE_PRIVATE_ACCESS_IS_RUNNING, 0);
    ic_atomic_init(&t->UNSAFE_PRIVATE_ACCESS_IS_JOINED, 0);

#if defined(IC_THREAD_C11)

    if (thrd_create(&t->UNSAFE_PRIVATE_ACCESS_THREAD_HANDLE,
                    ic_task_trampoline, t) != thrd_success)
    {
        ic_atomic_destroy(&t->UNSAFE_PRIVATE_ACCESS_IS_RUNNING);
        ic_atomic_destroy(&t->UNSAFE_PRIVATE_ACCESS_IS_JOINED);
        return IC_CONCURRENCY_FAILURE;
    }

#elif defined(IC_THREAD_PTHREAD)

    if (pthread_create(&t->UNSAFE_PRIVATE_ACCESS_THREAD_HANDLE,
                       NULL, ic_task_trampoline, t) != 0)
    {
        ic_atomic_destroy(&t->UNSAFE_PRIVATE_ACCESS_IS_RUNNING);
        ic_atomic_destroy(&t->UNSAFE_PRIVATE_ACCESS_IS_JOINED);
        return IC_CONCURRENCY_FAILURE;
    }

#elif defined(IC_THREAD_WIN)

    t->UNSAFE_PRIVATE_ACCESS_THREAD_HANDLE =
        CreateThread(NULL, 0, ic_task_trampoline, t, 0,
                     &t->UNSAFE_PRIVATE_ACCESS_THREAD_ID);

    if (!t->UNSAFE_PRIVATE_ACCESS_THREAD_HANDLE)
    {
        ic_atomic_destroy(&t->UNSAFE_PRIVATE_ACCESS_IS_RUNNING);
        ic_atomic_destroy(&t->UNSAFE_PRIVATE_ACCESS_IS_JOINED);
        return IC_CONCURRENCY_FAILURE;
    }

#endif

    return IC_CONCURRENCY_OK;
}

IC_HEADER_FUNC int ic_task_join(ic_task* t)
{
    if (!t)
    {
        return IC_CONCURRENCY_NULLREF;
    }

    if (ic_atomic_load(&t->UNSAFE_PRIVATE_ACCESS_IS_JOINED) == 1)
    {
        return IC_CONCURRENCY_ALREADY_JOINED;
    }

    ic_atomic_store(&t->UNSAFE_PRIVATE_ACCESS_IS_JOINED, 1);

#if defined(IC_THREAD_C11)
    thrd_join(t->UNSAFE_PRIVATE_ACCESS_THREAD_HANDLE, NULL);

#elif defined(IC_THREAD_PTHREAD)
    pthread_join(t->UNSAFE_PRIVATE_ACCESS_THREAD_HANDLE, NULL);

#elif defined(IC_THREAD_WIN)
    WaitForSingleObject(t->UNSAFE_PRIVATE_ACCESS_THREAD_HANDLE, INFINITE);
#endif

    return t->UNSAFE_PRIVATE_ACCESS_RESULT;
}

IC_HEADER_FUNC int ic_task_is_running(ic_task* t)
{
    if (!t)
    {
        return 0;
    }

    return ic_atomic_load(&t->UNSAFE_PRIVATE_ACCESS_IS_RUNNING);
}

IC_HEADER_FUNC int ic_task_get_result(ic_task* t, int* result_out)
{
    if (!t || !result_out)
    {
        return IC_CONCURRENCY_NULLREF;
    }

    if (ic_atomic_load(&t->UNSAFE_PRIVATE_ACCESS_IS_RUNNING))
    {
        return IC_CONCURRENCY_FAILURE;
    }

    *result_out = t->UNSAFE_PRIVATE_ACCESS_RESULT;
    return IC_CONCURRENCY_OK;
}

IC_HEADER_FUNC int ic_task_destroy(ic_task* t)
{
    if (!t)
    {
        return IC_CONCURRENCY_NULLREF;
    }

#if defined(IC_THREAD_WIN)
    CloseHandle(t->UNSAFE_PRIVATE_ACCESS_THREAD_HANDLE);
#endif

    ic_atomic_destroy(&t->UNSAFE_PRIVATE_ACCESS_IS_RUNNING);
    ic_atomic_destroy(&t->UNSAFE_PRIVATE_ACCESS_IS_JOINED);

    return IC_CONCURRENCY_OK;
}

/*##############################################################################
  LOCK SYSTEM
##############################################################################*/

typedef struct ic_lock {

    ic_atomic_int UNSAFE_PRIVATE_ACCESS_IS_LOCKED;

#if defined(IC_THREAD_C11)
    mtx_t UNSAFE_PRIVATE_ACCESS_MUTEX;

#elif defined(IC_THREAD_PTHREAD)
    pthread_mutex_t UNSAFE_PRIVATE_ACCESS_MUTEX;

#elif defined(IC_THREAD_WIN)
    CRITICAL_SECTION UNSAFE_PRIVATE_ACCESS_MUTEX;

#endif

} ic_lock;

/*==============================================================================
  LOCK API
==============================================================================*/

IC_HEADER_FUNC int ic_lock_init(ic_lock* l)
{
    if (!l)
    {
        return IC_CONCURRENCY_NULLREF;
    }

    if (ic_atomic_init(&l->UNSAFE_PRIVATE_ACCESS_IS_LOCKED, 0) != IC_CONCURRENCY_OK)
    {
        return IC_CONCURRENCY_FAILURE;
    }

#if defined(IC_THREAD_C11)

    if (mtx_init(&l->UNSAFE_PRIVATE_ACCESS_MUTEX, mtx_plain) != thrd_success)
    {
        ic_atomic_destroy(&l->UNSAFE_PRIVATE_ACCESS_IS_LOCKED);
        return IC_CONCURRENCY_FAILURE;
    }

#elif defined(IC_THREAD_PTHREAD)

    if (pthread_mutex_init(&l->UNSAFE_PRIVATE_ACCESS_MUTEX, NULL) != 0)
    {
        ic_atomic_destroy(&l->UNSAFE_PRIVATE_ACCESS_IS_LOCKED);
        return IC_CONCURRENCY_FAILURE;
    }

#elif defined(IC_THREAD_WIN)

    InitializeCriticalSection(&l->UNSAFE_PRIVATE_ACCESS_MUTEX);

#endif

    return IC_CONCURRENCY_OK;
}

IC_HEADER_FUNC void ic_lock_lock(ic_lock* l)
{
    if (!l)
    {
        return;
    }

#if defined(IC_THREAD_C11)
    mtx_lock(&l->UNSAFE_PRIVATE_ACCESS_MUTEX);
#elif defined(IC_THREAD_PTHREAD)
    pthread_mutex_lock(&l->UNSAFE_PRIVATE_ACCESS_MUTEX);
#elif defined(IC_THREAD_WIN)
    EnterCriticalSection(&l->UNSAFE_PRIVATE_ACCESS_MUTEX);
#endif

    ic_atomic_store(&l->UNSAFE_PRIVATE_ACCESS_IS_LOCKED, 1);
}

IC_HEADER_FUNC int ic_lock_trylock(ic_lock* l)
{
    if (!l)
    {
        return IC_CONCURRENCY_NULLREF;
    }

#if defined(IC_THREAD_C11)

    if (mtx_trylock(&l->UNSAFE_PRIVATE_ACCESS_MUTEX) != thrd_success)
    {
        return IC_CONCURRENCY_FAILURE;
    }

#elif defined(IC_THREAD_PTHREAD)

    if (pthread_mutex_trylock(&l->UNSAFE_PRIVATE_ACCESS_MUTEX) != 0)
    {
        return IC_CONCURRENCY_FAILURE;
    }

#elif defined(IC_THREAD_WIN)

    if (!TryEnterCriticalSection(&l->UNSAFE_PRIVATE_ACCESS_MUTEX))
    {
        return IC_CONCURRENCY_FAILURE;
    }

#endif

    ic_atomic_store(&l->UNSAFE_PRIVATE_ACCESS_IS_LOCKED, 1);
    return IC_CONCURRENCY_OK;
}

IC_HEADER_FUNC void ic_lock_unlock(ic_lock* l)
{
    if (!l)
    {
        return;
    }

#if defined(IC_THREAD_C11)
    mtx_unlock(&l->UNSAFE_PRIVATE_ACCESS_MUTEX);
#elif defined(IC_THREAD_PTHREAD)
    pthread_mutex_unlock(&l->UNSAFE_PRIVATE_ACCESS_MUTEX);
#elif defined(IC_THREAD_WIN)
    LeaveCriticalSection(&l->UNSAFE_PRIVATE_ACCESS_MUTEX);
#endif

    ic_atomic_store(&l->UNSAFE_PRIVATE_ACCESS_IS_LOCKED, 0);
}

IC_HEADER_FUNC int ic_lock_destroy(ic_lock* l)
{
    if (!l)
    {
        return IC_CONCURRENCY_NULLREF;
    }

    int mutex_result = IC_CONCURRENCY_OK;

#if defined(IC_THREAD_C11)

    mtx_destroy(&l->UNSAFE_PRIVATE_ACCESS_MUTEX);

#elif defined(IC_THREAD_PTHREAD)

    if (pthread_mutex_destroy(&l->UNSAFE_PRIVATE_ACCESS_MUTEX) != 0)
    {
        mutex_result = IC_CONCURRENCY_FAILURE;
    }

#elif defined(IC_THREAD_WIN)
    DeleteCriticalSection(&l->UNSAFE_PRIVATE_ACCESS_MUTEX);
#endif

    ic_atomic_store(&l->UNSAFE_PRIVATE_ACCESS_IS_LOCKED, 0);

    if(ic_atomic_destroy(&l->UNSAFE_PRIVATE_ACCESS_IS_LOCKED) != IC_CONCURRENCY_OK)
    {
        return IC_CONCURRENCY_FAILURE;
    }

    return mutex_result;
}

#endif /* IC_CONCURRENCY_H */