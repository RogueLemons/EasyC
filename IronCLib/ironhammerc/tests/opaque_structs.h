#ifndef IRON_HAMMER_C_TESTS_OPAQUE_STRUCTS_H
#define IRON_HAMMER_C_TESTS_OPAQUE_STRUCTS_H

#include "ironclib/ic_opaque_storage.h"

#define STRICT_OBJECT_SIZE  (8 * sizeof(float))
#define STRICT_OBJECT_ALIGN (IC_ALIGNOF(double))
IC_OPAQUE_STORAGE(StrictObject, STRICT_OBJECT_ALIGN, STRICT_OBJECT_SIZE)

void StrictObject_init(StrictObject* const out_obj);
void StrictObject_set_floats(StrictObject* const mod_obj, const float a, const float b, const float c);
void StrictObject_set_doubles(StrictObject* const mod_obj, const double a, const double b);
float StrictObject_get_float_a(const StrictObject* const obj);
float StrictObject_get_float_b(const StrictObject* const obj);
float StrictObject_get_float_c(const StrictObject* const obj);
double StrictObject_get_double_a(const StrictObject* const obj);
double StrictObject_get_double_b(const StrictObject* const obj);

#define FAST_OBJECT_SIZE  (8 * sizeof(float))
#define FAST_OBJECT_ALIGN (IC_ALIGNOF(double))
IC_OPAQUE_STORAGE(FastObject, FAST_OBJECT_ALIGN, FAST_OBJECT_SIZE)

void FastObject_init(FastObject* const out_obj);
void FastObject_set_floats(FastObject* const mod_obj, const float a, const float b, const float c);
void FastObject_set_doubles(FastObject* const mod_obj, const double a, const double b);
float FastObject_get_float_a(const FastObject* const obj);
float FastObject_get_float_b(const FastObject* const obj);
float FastObject_get_float_c(const FastObject* const obj);
double FastObject_get_double_a(const FastObject* const obj);
double FastObject_get_double_b(const FastObject* const obj);

#endif // IRON_HAMMER_C_TESTS_OPAQUE_STRUCTS_H