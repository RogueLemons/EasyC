#include <stdio.h>

#include <stdint.h>
#include <stddef.h>
#include <float.h>
#include <limits.h>

#include "ironclib/ic_static_assert.h"
#include "ironclib/ic_num_cast.h"

IC_STATIC_ASSERT(sizeof(float) == 4, "Expected float to be 4 bytes");

// Expansion helper macros
#define DATA_PAIR_IMPL(X, a1,a2,a3,a4,b1,b2,b3,b4) X(a1,a2,a3,a4,b1,b2,b3,b4)
#define DATA_PAIR(X, A, B) DATA_PAIR_IMPL(X, A, B)

// Make cast functions for standard types
typedef unsigned int unsigned_int;
#define UINT_DATA       unsigned_int, IC_MOLD_UNSIGNED_INT, 0,         UINT_MAX
#define INT_DATA        int,          IC_MOLD_SIGNED_INT,   INT_MIN,   INT_MAX
#define FLOAT_DATA      float,        IC_MOLD_FLOAT,        -FLT_MAX,  FLT_MAX
#define DOUBLE_DATA     double,       IC_MOLD_FLOAT,        -DBL_MAX,  DBL_MAX

#define STANDARD_CAST_CONVERSION_MATRIX(X)  \
    DATA_PAIR(X, INT_DATA,   INT_DATA)      \
    DATA_PAIR(X, INT_DATA,   UINT_DATA)     \
    DATA_PAIR(X, INT_DATA,   FLOAT_DATA)    \
    DATA_PAIR(X, INT_DATA,   DOUBLE_DATA)   \
    DATA_PAIR(X, UINT_DATA,  INT_DATA)      \
    DATA_PAIR(X, UINT_DATA,  UINT_DATA)     \
    DATA_PAIR(X, UINT_DATA,  FLOAT_DATA)    \
    DATA_PAIR(X, UINT_DATA,  DOUBLE_DATA)   \
    DATA_PAIR(X, FLOAT_DATA, INT_DATA)      \
    DATA_PAIR(X, FLOAT_DATA, UINT_DATA)     \
    DATA_PAIR(X, FLOAT_DATA, FLOAT_DATA)    \
    DATA_PAIR(X, FLOAT_DATA, DOUBLE_DATA)   \
    DATA_PAIR(X, DOUBLE_DATA, INT_DATA)     \
    DATA_PAIR(X, DOUBLE_DATA, UINT_DATA)    \
    DATA_PAIR(X, DOUBLE_DATA, FLOAT_DATA)   \
    DATA_PAIR(X, DOUBLE_DATA, DOUBLE_DATA)

// Generate casting functions
IC_CASTING_FUNCTIONS(STANDARD_CAST_CONVERSION_MATRIX)

int main(void) 
{
    printf("The value of PI is: %f\n", 3.14f);
    return 0;
}