#ifndef EC_INLINE_H
#define EC_INLINE_H

/*
===============================================================================
EC Inline Abstraction Layer

C Compatibility:
- C89+ fallback supported
- C99+ inline supported
- MSVC / GCC / Clang supported

Provides:
    EC_INLINE        -> best-effort inline hint
    EC_HEADER_SAFE   -> always safe in headers (never causes linker issues)

===============================================================================
*/

/*
-------------------------------------------------------------------------------
EC_INLINE

Purpose:
- Express intent: "this function should be inlined if possible"
- Does NOT guarantee inlining
- Does NOT guarantee linkage safety in headers

Use for:
- source files (.c)
- optional performance hints
-------------------------------------------------------------------------------
*/

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
    #define EC_INLINE inline
#elif defined(_MSC_VER)
    #define EC_INLINE __inline
#elif defined(__GNUC__) || defined(__clang__)
    #define EC_INLINE __inline__
#else
    #define EC_INLINE
#endif


/*
-------------------------------------------------------------------------------
EC_HEADER_SAFE

Purpose:
- SAFE for header-defined functions
- Prevents multiple definition / linker errors
- Guarantees internal linkage

Rule:
- Always safe to put in headers
- Compiler may inline if it wants

Preferred for:
- small getters
- to_string functions
- tiny utility helpers
-------------------------------------------------------------------------------
*/

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
    #define EC_HEADER_SAFE static inline
#elif defined(_MSC_VER)
    #define EC_HEADER_SAFE static __inline
#elif defined(__GNUC__) || defined(__clang__)
    #define EC_HEADER_SAFE static __inline__
#else
    #define EC_HEADER_SAFE static
#endif

#endif