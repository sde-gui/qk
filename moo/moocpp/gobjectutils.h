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

#include <memory>
#include <utility>
#include "moocpp/gobjinfo.h"

namespace moo {

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

template<typename T>
inline T* object_ref(T *obj)
{
    return static_cast<T*>(g_object_ref(obj));
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

} // namespace moo
