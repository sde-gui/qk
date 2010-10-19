#ifndef MOO_SCRIPT_CLASSES_UTIL_H
#define MOO_SCRIPT_CLASSES_UTIL_H

#include "mooscript.h"

namespace mom {

#define DEFINE_GET_ARG_SIMPLE(Vt, Type, get_arg_func, wrap_func)\
inline Type get_arg_func(const Variant &var, const char *)      \
{                                                               \
    if (var.vt() != Vt)                                         \
        Error::raise("invalid argument value");                 \
    return var.value<Vt>();                                     \
}                                                               \
inline Type get_arg_func##_opt(const Variant &var, const char *)\
{                                                               \
    if (var.vt() == VtVoid)                                     \
        return Type();                                          \
    else if (var.vt() != Vt)                                    \
        Error::raise("invalid argument value");                 \
    else                                                        \
        return var.value<Vt>();                                 \
}                                                               \
inline Variant wrap_func(const Type &val)                       \
{                                                               \
    return Variant(val);                                        \
}

DEFINE_GET_ARG_SIMPLE(VtBool, bool, get_arg_bool, wrap_bool)
DEFINE_GET_ARG_SIMPLE(VtString, String, get_arg_string, wrap_string)
DEFINE_GET_ARG_SIMPLE(VtArray, VariantArray, get_arg_array, wrap_array)

inline gint64 get_arg_int(const Variant &var, const char *)
{
    switch (var.vt())
    {
        case VtInt:
            return var.value<VtInt>();
        case VtIndex:
            return var.value<VtIndex>().get();
        case VtBase1:
            return var.value<VtBase1>().get();
        default:
            Error::raise("invalid argument value");
    }
}

inline Variant wrap_int(gint64 value)
{
    return Variant(value);
}

inline gint64 get_arg_index(const Variant &var, const char *)
{
    switch (var.vt())
    {
        case VtInt:
            return var.value<VtInt>();
        case VtIndex:
            return var.value<VtIndex>().get();
        case VtBase1:
            return var.value<VtBase1>().get_index().get();
        default:
            Error::raise("invalid argument value");
    }
}

inline Variant wrap_index(gint64 val)
{
    return Variant(Index(val));
}

inline Variant get_arg_variant(const Variant &var, const char *)
{
    return var;
}

inline Variant get_arg_variant_opt(const Variant &var, const char *)
{
    return var;
}

template<typename T>
inline T *get_object_arg_impl(const Variant &val, bool null_ok, const char *)
{
    if (null_ok && val.vt() == VtVoid)
        return 0;

    if (val.vt() != VtObject)
        Error::raise("object expected");

    HObject h = val.value<VtObject>();
    if (null_ok && h.id() == 0)
        return 0;

    moo::SharedPtr<Object> obj = Object::lookup_object(h);

    if (!obj)
        Error::raise("bad object");
    if (&obj->meta() != &T::class_meta())
        Error::raise("bad object");

    return static_cast<T*>(obj.get());
}

template<typename T>
inline T &get_object_arg(const Variant &var, const char *argname)
{
    return *get_object_arg_impl<T>(var, false, argname);
}

template<typename T>
inline T *get_object_arg_opt(const Variant &var, const char *argname)
{
    return get_object_arg_impl<T>(var, true, argname);
}

inline Variant wrap_object(Object *obj)
{
    return HObject(obj ? obj->id() : 0);
}

} // namespace mom

#endif /* MOO_SCRIPT_CLASSES_UTIL_H */
