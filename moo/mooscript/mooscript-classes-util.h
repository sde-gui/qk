#ifndef MOO_SCRIPT_CLASSES_UTIL_H
#define MOO_SCRIPT_CLASSES_UTIL_H

#include "mooscript.h"

namespace mom {

struct FunctionCallInfo
{
    String name;

    explicit FunctionCallInfo(const char *name)
        : name(name)
    {
    }
};

FunctionCallInfo current_func();

NORETURN inline void invalid_argument(const char *arg, const char *why = 0)
{
    Error::raisef("in function '%s', invalid value for argument '%s'%s%s",
                  (const char*) current_func().name, arg, why ? ": " : "", why ? why : "");
}

inline const char *get_argument_type_name(VariantType vt)
{
    switch (vt)
    {
        case VtVoid:
            moo_return_val_if_reached("none");
        case VtBool:
            return "bool";
        case VtIndex:
            return "index";
        case VtBase1:
            return "base1";
        case VtInt:
            return "int";
        case VtDouble:
            return "double";
        case VtString:
            return "string";
        case VtArray:
            return "list";
        case VtArgs:
            return "arglist";
        case VtDict:
            return "dict";
        case VtObject:
            return "object";
        default:
            moo_return_val_if_reached("unknown");
    }
}

NORETURN inline void invalid_argument_type(const char *arg, VariantType vt_expected, VariantType vt_actual)
{
    const char *type_expected = get_argument_type_name(vt_expected);
    const char *type_actual = get_argument_type_name(vt_actual);
    Error::raisef("in function %s, invalid value for argument '%s': expected %s, got %s",
                  (const char*) current_func().name, arg, type_expected, type_actual);
}

#define DEFINE_GET_ARG_SIMPLE(Vt, Type, get_arg_func, wrap_func)    \
inline Type get_arg_func(const Variant &var, const char *arg)       \
{                                                                   \
    if (var.vt() != Vt)                                             \
        invalid_argument_type(arg, Vt, var.vt());                   \
    return var.value<Vt>();                                         \
}                                                                   \
inline Type get_arg_func##_opt(const Variant &var, const char *arg) \
{                                                                   \
    if (var.vt() == VtVoid)                                         \
        return Type();                                              \
    else if (var.vt() != Vt)                                        \
        invalid_argument_type(arg, Vt, var.vt());                   \
    else                                                            \
        return var.value<Vt>();                                     \
}                                                                   \
inline Variant wrap_func(const Type &val)                           \
{                                                                   \
    return Variant(val);                                            \
}

DEFINE_GET_ARG_SIMPLE(VtBool, bool, get_arg_bool, wrap_bool)
DEFINE_GET_ARG_SIMPLE(VtString, String, get_arg_string, wrap_string)
DEFINE_GET_ARG_SIMPLE(VtArray, VariantArray, get_arg_array, wrap_array)

inline gint64 get_arg_int(const Variant &var, const char *arg)
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
            Error::raisef("in function %s, invalid value for argument '%s': expected an int, got %s",
                          (const char*) current_func().name, arg, get_argument_type_name(var.vt()));
    }
}

inline Variant wrap_int(gint64 value)
{
    return Variant(value);
}

inline gint64 get_arg_index(const Variant &var, const char *arg)
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
            Error::raisef("in function %s, invalid value for argument '%s': expected an index, got %s",
                          (const char*) current_func().name, arg, get_argument_type_name(var.vt()));
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
inline T *get_object_arg_impl(const Variant &val, bool null_ok, const char *arg)
{
    if (null_ok && val.vt() == VtVoid)
        return 0;

    if (val.vt() != VtObject)
        invalid_argument(arg, "object expected");

    HObject h = val.value<VtObject>();
    if (null_ok && h.id() == 0)
        return 0;

    moo::SharedPtr<Object> obj = Object::lookup_object(h);

    if (!obj)
        invalid_argument(arg, "bad object");
    if (&obj->meta() != &T::class_meta())
        invalid_argument(arg, "bad object");

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
