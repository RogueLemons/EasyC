#ifndef EC_STATIC_ASSERT_H
#define EC_STATIC_ASSERT_H

/*
===============================================================================
EC Static Assert

C Version Compatibility:
- C11+: uses _Static_assert
- MSVC: uses static_assert
- Pre-C11: fallback typedef trick

Usage:
    EC_STATIC_ASSERT(sizeof(int) == 4, "int must be 4 bytes");

For compilers without native static assert support, this will cause a compile-time error if the condition is false, but the error message may be less clear.
===============================================================================
*/

// glue helpers
#define EC_INTERNAL_SA_GLUE(a,b) a##b
#define EC_INTERNAL_SA_XGLUE(a,b) EC_INTERNAL_SA_GLUE(a,b)

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L

    #define EC_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)

#elif defined(_MSC_VER) && _MSC_VER >= 1600

    #define EC_STATIC_ASSERT(cond, msg) static_assert(cond, msg)

#else

    #define EC_STATIC_ASSERT(cond, msg) \
        typedef char EC_INTERNAL_SA_XGLUE(static_assert_failed_at_line_, __LINE__) \
        [(cond) ? 1 : -1]

#endif

#endif