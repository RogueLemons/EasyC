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
    EC_STATIC_ASSERT(sizeof(int) == 4, int_must_be_4_bytes);

The message must be a valid identifier (no spaces).
===============================================================================
*/

// glue helpers
#define EC_SA_GLUE(a,b) a##b
#define EC_SA_XGLUE(a,b) EC_SA_GLUE(a,b)

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L

    #define EC_STATIC_ASSERT(cond, msg) _Static_assert(cond, #msg)

#elif defined(_MSC_VER)

    #define EC_STATIC_ASSERT(cond, msg) static_assert(cond, #msg)

#else

    #define EC_STATIC_ASSERT(cond, msg) \
        typedef char EC_SA_XGLUE(static_assert_failed_##msg##_at_line_, __LINE__) \
        [(cond) ? 1 : -1]

#endif

#endif