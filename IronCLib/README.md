# IronCLib
A small C library for writing safer, more consistent C code.

It is a header-only utility library and a code analysis parser is planned for additional warnings and safety checks.

Includes:
* Instant features
  * Compile-time assertions (`ic_static_assert.h`)
  * Inline and header safety abstractions (`ic_inline.h`)
  * Memory allocation safety checks (`ic_memory.h`)
  * Bounded loop safety utilities (`ic_bounded_loop.h`)
* Drop-in code generators
  * Type-safe enum replacement (`ic_typenum.h`)
  * Opaque struct storage for encapsulation (`ic_opaque_storage.h`)
* Monolithic code generators
  * Result-based error handling (`ic_result.h`)
  * Safer numeric casting function generators (`ic_num_cast.h`)

## Table of Contents (TODO)
* [Overview](#ironclib)
* [Quick Intro](#quick-intro)
  * [Example 1](#example-1)
  * [Example 2](#example-2)
  * [Example 3](#example-3)
* [Header Library](#header-library)
  * [ic.h](#ich)
  * [ic_static_assert.h](#ic_static_asserth)
  * [ic_inline.h](#ic_inlineh)
  * [ic_typenum.h](#ic_typenumh)
  * [ic_opaque_storage.h](#ic_opaque_storageh)
  * [ic_result.h](#ic_resulth)
  * [ic_memory.h](#ic_memoryh)
  * [ic_bounded_loop.h](#ic_bounded_looph)
  * [ic_num_cast.h](#ic_num_casth)
* [Using in your system](#using-in-your-system)
  * [Create a standardized, type-safe error system](#create-a-standardized-type-safe-error-system)
  * [Create one entrypoint for memory allocation](#create-one-entrypoint-for-memory-allocation)
  * [Write with rules for structs and opaque storage](#write-with-rules-for-structs-and-opaque-storage)
* [TODO](#todo)

## Quick Intro
A fast showcase of how the library is used, before more in-depth descriptions and examples. 

### Example 1
This example shows how you can combine a type-safe enum replacement with result types static assert in a header implemented function.

```c
#include "ic.h"
#include <stdint.h>

#define MATH_ERROR_LIST(X, Type) \
    X(Type, Generic, 0, "An error occurred") \
    X(Type, DivideByZero, 1, "Division by zero") 

IC_TYPENUM_FULL(MathError, uint8_t, MATH_ERROR_LIST)
IC_RESULT_TYPE(IntResult, int, MathError)

IC_HEADER_FUNC IntResult safe_divide(int a, int b) {
    IC_STATIC_ASSERT(sizeof(int) >= 4, "int too small");

    if (b == 0) {
        return IntResult_err(Error_DivideByZero);
    }

    return IntResult_ok(a / b);
}
```

### Example 2
This example shows how to create opaque structs allocated on the stack, where users can only access struct implementation members via functions. *Note: This example does not conform with strict aliasing.*

```c
// ------- Matrix4x4.h -------
#include "ic.h"

#define MAT4_SIZE  (sizeof(float) * 16)
#define MAT4_ALIGN (IC_ALIGNOF(float))
IC_OPAQUE_STORAGE(Matrix4x4, MAT4_ALIGN, MAT4_SIZE)

// API
void mat4_set_identity(Matrix4x4* const m);
float mat4_get(const Matrix4x4* const m, const int row, const int col);

// ------- Matrix4x4.c -------
#include "Matrix4x4.h"

struct Matrix4x4Impl {
    float data[16];
};
typedef struct Matrix4x4Impl Matrix4x4Impl;
IC_OPAQUE_IMPL_ASSERT(Matrix4x4Impl, MAT4_ALIGN, MAT4_SIZE)

void mat4_set_identity(Matrix4x4* const m) {
    Matrix4x4Impl* const real = (Matrix4x4Impl*)m;

    for (int i = 0; i < 16; ++i)
    {
        real->data[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
}

float mat4_get(const Matrix4x4* const m, const int row, const int col) {
    const Matrix4x4Impl* const real = (const Matrix4x4Impl*)m;
    return real->data[row * 4 + col];
}
```

### Example 3
This examples shows using a slightly safer memory allocator, bounded while loop, and usage of safer casting functions. 

```c
#include "ic.h"
#include <stdint.h>

#define MY_CASTING_MATRIX(X) \
    X(int32_t, IC_MOLD_SIGNED_INT, INT32_MIN, INT32_MAX, uint16_t, IC_MOLD_UNSIGNED_INT, 0, UINT16_MAX)
    // Add more entries here
IC_CASTING_FUNCTIONS(MY_CASTING_MATRIX)

int main(void)
{
    int32_t count = 10;
    uint16_t* values = IC_MALLOC_ARRAY(uint16_t, count);
    if (!values) return 1;

    int32_t i = 0;
    IC_BOUNDED_WHILE(i < count, 1000) {
        values[i] = cast_int32_t_to_uint16_t(i);
        i++;
    }

    free(values);
    return 0;
}
```

## Read more?

# TODO
- Make typenum generated functions use pointers (only if starting to allow non-integer internal types, maybe for SteelC)?
- Add tests that can be verified on multiple compilers
- Add IronHammerC testing system
- link to other doc files in Read more
- Update table of contents
