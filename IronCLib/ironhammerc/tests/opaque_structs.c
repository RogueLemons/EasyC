#include "opaque_structs.h"
#include "ironclib/ic_inline.h"
#include <string.h> // memset, memcpy

// ================= Strict aliasing rules with memcpy ================= 


typedef struct StrictObjectImpl 
{
    float float_data[3];
    double double_data[2];
} StrictObjectImpl;
IC_OPAQUE_IMPL_ASSERT(StrictObjectImpl, STRICT_OBJECT_ALIGN, STRICT_OBJECT_SIZE)

void StrictObject_init(StrictObject* const out_obj)
{
    StrictObjectImpl impl;
    memset(&impl, 0, sizeof(impl));
    memcpy(out_obj->data, &impl, sizeof(impl));
}

void StrictObject_set_floats(StrictObject* const mod_obj, const float a, const float b, const float c)
{
    StrictObjectImpl impl;
    memcpy(&impl, mod_obj->data, sizeof(impl));

    impl.float_data[0] = a;
    impl.float_data[1] = b;
    impl.float_data[2] = c;

    memcpy(mod_obj->data, &impl, sizeof(impl));
}

void StrictObject_set_doubles(StrictObject* const mod_obj, const double a, const double b)
{
    StrictObjectImpl impl;
    memcpy(&impl, mod_obj->data, sizeof(impl));

    impl.double_data[0] = a;
    impl.double_data[1] = b;

    memcpy(mod_obj->data, &impl, sizeof(impl));
}

float StrictObject_get_float_a(const StrictObject* const obj)
{
    StrictObjectImpl impl;
    memcpy(&impl, obj->data, sizeof(impl));
    return impl.float_data[0];
}

float StrictObject_get_float_b(const StrictObject* const obj)
{
    StrictObjectImpl impl;
    memcpy(&impl, obj->data, sizeof(impl));
    return impl.float_data[1];
}

float StrictObject_get_float_c(const StrictObject* const obj)
{
    StrictObjectImpl impl;
    memcpy(&impl, obj->data, sizeof(impl));
    return impl.float_data[2];
}

double StrictObject_get_double_a(const StrictObject* const obj)
{
    StrictObjectImpl impl;
    memcpy(&impl, obj->data, sizeof(impl));
    return impl.double_data[0];
}

double StrictObject_get_double_b(const StrictObject* const obj)
{
    StrictObjectImpl impl;
    memcpy(&impl, obj->data, sizeof(impl));
    return impl.double_data[1];
}

// ================= Fast version for aligned data ================= 

typedef struct FastObjectImpl 
{
    float float_data[3];
    double double_data[2];
}FastObjectImpl;
IC_OPAQUE_IMPL_ASSERT(FastObjectImpl, FAST_OBJECT_ALIGN, FAST_OBJECT_SIZE)

static IC_INLINE FastObjectImpl* fast_impl(FastObject* const obj)
{
    return (FastObjectImpl*)(obj->data);
}

static IC_INLINE const FastObjectImpl* fast_impl_const(const FastObject* const obj)
{
    return (const FastObjectImpl*)(obj->data);
}

void FastObject_init(FastObject* const out_obj)
{
    memset(out_obj->data, 0, FAST_OBJECT_SIZE);
}

void FastObject_set_floats(FastObject* const mod_obj, const float a, const float b, const float c)
{
    FastObjectImpl* o = fast_impl(mod_obj);
    o->float_data[0] = a;
    o->float_data[1] = b;
    o->float_data[2] = c;
}

void FastObject_set_doubles(FastObject* const mod_obj, const double a, const double b)
{
    FastObjectImpl* o = fast_impl(mod_obj);
    o->double_data[0] = a;
    o->double_data[1] = b;
}

float FastObject_get_float_a(const FastObject* const obj)
{
    return fast_impl_const(obj)->float_data[0];
}

float FastObject_get_float_b(const FastObject* const obj)
{
    return fast_impl_const(obj)->float_data[1];
}

float FastObject_get_float_c(const FastObject* const obj)
{
    return fast_impl_const(obj)->float_data[2];
}

double FastObject_get_double_a(const FastObject* const obj)
{
    return fast_impl_const(obj)->double_data[0];
}

double FastObject_get_double_b(const FastObject* const obj)
{
    return fast_impl_const(obj)->double_data[1];
}