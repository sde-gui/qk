/*
 *   moocpp/gobjectutils.h
 *
 *   Copyright (C) 2004-2015 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#ifdef __cplusplus

#include <memory>
#include <utility>
#include <moocpp/gobjinfo.h>
#include <moocpp/utils.h>

namespace moo {

template<typename T>
inline GType get_g_type();

template<typename T>
inline T* object_ref(T *obj)
{
    return static_cast<T*>(g_object_ref(obj));
}

template<typename T>
inline T* object_cast(gpointer obj)
{
    return obj ? reinterpret_cast<T*>(G_TYPE_CHECK_INSTANCE_CAST(g_object_ref(obj), get_g_type<T>(), T)) : nullptr;
}

template<typename T>
inline T* object_ref_opt(T *obj)
{
    return obj ? object_cast<T>(g_object_ref(obj)) : nullptr;
}

template<typename T>
inline T* object_cast_opt(gpointer obj)
{
    return obj ? object_cast<T>(obj) : nullptr;
}

template<typename T>
inline T* object_ref_cast_opt(gpointer obj)
{
    return obj ? object_ref(object_cast<T>(obj)) : nullptr;
}

template<typename T>
inline T* new_object()
{
    return object_cast<T>(g_object_new(get_g_type<T>(), nullptr));
}

template<typename T, typename ...Args>
inline void init_cpp_gobj(T* o, Args&& ...args)
{
#ifdef MOO_DEBUG
    g_assert(g_object_get_data(G_OBJECT(o), "__moo_cpp_object_init__") == nullptr);
#endif

    new(o) T(std::forward<Args>(args)...);

#ifdef MOO_DEBUG
    g_object_set_data(G_OBJECT(o), "__moo_cpp_object_init__", GINT_TO_POINTER(true));
#endif
}

template<typename T>
inline void finalize_cpp_gobj(T* o)
{
#ifdef MOO_DEBUG
    g_assert(g_object_get_data(G_OBJECT(o), "__moo_cpp_object_init__") == GINT_TO_POINTER(true));
#endif

    o->~T();

#ifdef MOO_DEBUG
    g_object_set_data(G_OBJECT(o), "__moo_cpp_object_init__", nullptr);
#endif
}


template<typename T, typename TPriv, typename ...Args>
inline void init_cpp_private(T* owner, TPriv*& p, GType owner_type, Args&& ...args)
{
#ifdef MOO_DEBUG
    g_assert(g_object_get_data(G_OBJECT(owner), "__moo_cpp_private_init__") == nullptr);
#endif

    p = G_TYPE_INSTANCE_GET_PRIVATE(owner, owner_type, TPriv);
    new(p) TPriv(std::forward<Args>(args)...);

#ifdef MOO_DEBUG
    g_object_set_data(G_OBJECT(owner), "__moo_cpp_private_init__", GINT_TO_POINTER(true));
#endif
}

template<typename T, typename TPriv, typename ...Args>
inline void init_cpp_private(T* owner, TPriv*& p, Args&& ...args)
{
#ifdef MOO_DEBUG
    g_assert(g_object_get_data(G_OBJECT(owner), "__moo_cpp_private_init__") == nullptr);
#endif

    // object_g_type() will produce a compiler error if the type wasn't registered
    p = G_TYPE_INSTANCE_GET_PRIVATE(owner, gobjinfo<T>::object_g_type(), TPriv);
    new(p) TPriv(std::forward<Args>(args)...);

#ifdef MOO_DEBUG
    g_object_set_data(G_OBJECT(owner), "__moo_cpp_private_init__", GINT_TO_POINTER(true));
#endif
}

template<typename T, typename TPriv>
inline void finalize_cpp_private(T* owner, TPriv*& p)
{
#ifdef MOO_DEBUG
    g_assert(g_object_get_data(G_OBJECT(owner), "__moo_cpp_private_init__") == GINT_TO_POINTER(true));
#endif

    if (p != nullptr)
    {
        p->~TPriv();
        p = nullptr;
    }

#ifdef MOO_DEBUG
    g_object_set_data(G_OBJECT(owner), "__moo_cpp_private_init__", nullptr);
#endif
}

struct class_helper
{
    template<typename X>
    static size_t address(X* x)
    {
        return reinterpret_cast<size_t>(reinterpret_cast<const volatile char*>(x));
    }

    template<typename Sup, typename Sub>
    static void verify_g_object_subclass_alignment()
    {
        Sup* x = nullptr;
        Sub* y = static_cast<Sub*>(x);
        moo_release_assert(class_helper::address(x) == class_helper::address(y));
    }
};

template<typename CObject>
std::vector<gobj_ptr<CObject>> object_list_to_vector (GList* list)
{
    std::vector<gobj_ptr<CObject>> ret;

    for (GList* l = list; l != nullptr; l = l->next)
    {
        CObject* o = reinterpret_cast<CObject*>(l->data);
        g_assert (!o || G_TYPE_CHECK_INSTANCE_TYPE ((o), gobjinfo<CObject>::object_g_type ()));
        ret.emplace_back (wrap (o));
    }

    return ret;
}

template<typename T>
class cpp_vararg_value_fixer;

template<typename T>
class cpp_vararg_value_fixer
{
public:
    static const T& apply (const T& val) { return val; }
};

template<typename TSrc, typename TDest>
class cpp_vararg_value_fixer_convert
{
public:
    static TDest apply (const TSrc& val) { return val; }
};

template<> class cpp_vararg_value_fixer<bool> : public cpp_vararg_value_fixer_convert<bool, gboolean>{};
template<> class cpp_vararg_value_fixer<gstr> : public cpp_vararg_value_fixer_convert<gstr, const char*>{};
template<> class cpp_vararg_value_fixer<gstrp> : public cpp_vararg_value_fixer_convert<gstrp, const char*>{};

template<typename CObject>
class cpp_vararg_value_fixer<gobj_ptr<CObject>>
{
public:
    static CObject* apply (const gobj_ptr<CObject>& val) { return val.gobj(); }
};

template<typename CObject>
class cpp_vararg_value_fixer<gobj_raw_ptr<CObject>>
{
public:
    static CObject* apply (const gobj_raw_ptr<CObject>& val) { return val.gobj (); }
};

template<typename T>
class cpp_vararg_value_fixer<objp<T>>
{
public:
    static T* apply (objp<T> val) { return val.release (); }
};


template<typename T>
struct cpp_vararg_dest_fixer
{
    template<typename T>
    static void* apply (T) = delete;
};

template<typename T>
struct cpp_vararg_dest_fixer_passthrough
{
    static T* apply (T* p) { return p; }
};

template<typename T>
struct cpp_vararg_dest_fixer_passthrough_ref
{
    static T* apply (T& p) { return &p; }
};

template<> struct cpp_vararg_dest_fixer<int&> : public cpp_vararg_dest_fixer_passthrough_ref<int>{};
template<> struct cpp_vararg_dest_fixer<guint&> : public cpp_vararg_dest_fixer_passthrough_ref<guint>{};

template<>
struct cpp_vararg_dest_fixer<gstrp&>
{
    static char** apply (gstrp& s) { return s.pp (); }
};

template<typename T>
struct cpp_vararg_dest_fixer<objp<T>&>
{
    static void** apply (objp<T>& o) { return reinterpret_cast<void**>(o.pp ()); }
};

template<typename T>
struct cpp_vararg_dest_fixer<gobj_ptr<T>&>
{
    static void** apply (gobj_ptr<T>& o) { return reinterpret_cast<void**>(o.pp ()); }
};

} // namespace moo

#endif // __cplusplus
