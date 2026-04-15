#ifndef IC_BOUNDED_LOOP_H
#define IC_BOUNDED_LOOP_H

/*
===============================================================================
IC Bounded Loop Abstractions

C Compatibility:
- C89+ supported (size_t requirement applies externally)
- MSVC / GCC / Clang supported

Provides:
    IC_BOUNDED_WHILE      -> bounded while loop with safety limit
    IC_BOUNDED_DO_WHILE   -> bounded do-while loop with safety limit
    IC_BOUNDED_FOR        -> bounded for loop with safety limit

Purpose:
- Prevent accidental infinite loops
- Provide deterministic maximum iteration limits
- Preserve natural C loop semantics
- Make loop safety explicit and readable in source code
- Treat invalid (<= 0) loop bounds as zero iterations

===============================================================================
*/

#include <stddef.h>

/*
-------------------------------------------------------------------------------
IC_BOUNDED_WHILE

Purpose:
- Executes a loop while:
    1. condition is true
    2. iteration count < max_loops

Safety guarantees:
- Hard upper bound on iterations (prevents infinite loops)
- max_loops is evaluated exactly once
- max_loops <= 0 is treated as 0 (loop does not execute)
- Uses unsigned size_t after validation to avoid overflow
- Loop counter is scoped locally via macro expansion

Implementation notes:
- Expands to a single for-loop
- Uses a per-line unique counter variable
- No hidden state shared between macros

Warnings:
- condition is evaluated every iteration and may have side effects
- __LINE__-based naming may collide if multiple macros are used on same line
- Avoid user variables starting with `_ic_`

Example:
    IC_BOUNDED_WHILE(x != NULL, 1000) {
        x = x->next;
    }
-------------------------------------------------------------------------------
*/
#define IC_BOUNDED_WHILE(condition, max_loops)                          \
    for (size_t _ic_loop_##__LINE__ = 0,                                \
                _ic_max_##__LINE__ =                                    \
                    ((max_loops) <= 0) ? 0u : (size_t)(max_loops);      \
         (_ic_loop_##__LINE__ < _ic_max_##__LINE__) && (condition);     \
         ++_ic_loop_##__LINE__)

/*
-------------------------------------------------------------------------------
IC_BOUNDED_DO_WHILE

Purpose:
- Executes a loop at least once, then continues while:
    1. condition is true
    2. iteration count < max_loops

Safety guarantees:
- Guaranteed single evaluation of max_loops
- Hard upper bound on iterations
- max_loops <= 0 results in zero iterations
- First iteration is always executed unless max_loops is 0
- Fully predictable termination behavior

Implementation notes:
- Uses a first-iteration flag (_ic_first)
- Expands into a single for-loop construct
- No hidden runtime system or shared state

Warnings:
- condition is evaluated after each iteration
- side effects in condition will be executed repeatedly
- avoid macro name collisions with `_ic_` prefix

Example:
    IC_BOUNDED_DO_WHILE(!success, 5) {
        success = try_connect();
        attempts++;
    }
-------------------------------------------------------------------------------
*/
#define IC_BOUNDED_DO_WHILE(condition, max_loops)                       \
    for (size_t _ic_loop_##__LINE__ = 0,                                \
                _ic_max_##__LINE__ =                                    \
                    ((max_loops) <= 0) ? 0u : (size_t)(max_loops),      \
                _ic_first_##__LINE__ = 1;                               \
         _ic_first_##__LINE__ ||                                        \
         ((_ic_loop_##__LINE__ < _ic_max_##__LINE__) && (condition));   \
         _ic_first_##__LINE__ = 0, ++_ic_loop_##__LINE__)

/*
-------------------------------------------------------------------------------
IC_BOUNDED_FOR

Purpose:
- Standard counted loop with explicit safety bound
- Executes:
    initialization -> condition check -> iteration step
- Ensures loop does not exceed max_loops iterations

Safety guarantees:
- Hard upper bound on iterations
- max_loops is evaluated exactly once
- max_loops <= 0 results in zero iterations
- Loop counter is internally tracked and not exposed

Warnings:
- decl must not contain comma expressions
- condition and iter are evaluated exactly as in a normal for-loop
- macro expands directly into a for-loop (no hidden state layer)
- avoid variable name collisions with `_ic_` prefix

Example:
    IC_BOUNDED_FOR(int i, 0, i < 10, i++, 1000) {
        printf("%d\n", i);
    }
-------------------------------------------------------------------------------
*/

#define IC_BOUNDED_FOR(decl, init_val, condition, iter, max_loops)     \
    for (decl = (init_val),                                            \
         size_t _ic_max_##__LINE__ =                                   \
            ((max_loops) <= 0) ? 0u : (size_t)(max_loops),             \
         size_t _ic_loop_##__LINE__ = 0;                               \
         (condition) && (_ic_loop_##__LINE__ < _ic_max_##__LINE__);    \
         (iter), ++_ic_loop_##__LINE__)

#endif // IC_BOUNDED_LOOP_H