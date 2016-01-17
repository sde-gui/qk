/*
 *   moocpp/gobjinfo.h
 *
 *   Copyright (C) 2004-2016 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
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

#include <glib-object.h>
#include <type_traits>

namespace moo {

void init_gobj_system ();

///////////////////////////////////////////////////////////////////////////////////////////
//
// gobjinfo
//

template<typename Object, typename Super>
struct gobj_is_subclass
{
    static const bool value = false;
};

// Generic implementation, all we know it's a subclass of GObject; we don't
// even know its GType. This implementation is needed so that it's possible
// to have gobj_ptr<GAnything> without having to define gobjinfo for it.
template<typename Object>
struct gobjinfo
{
    static const bool is_gobject = true;
    using object_type = Object;
    using parent_type = GObject;
    // object_g_type() is not defined
};

template<>
struct gobjinfo<GObject>
{
    static const bool is_gobject = true;
    using object_type = GObject;
    static GType object_g_type() { return G_TYPE_OBJECT; }
};

template<typename T>
inline GType get_g_type()
{
    return gobjinfo<T>::object_g_type();
}

template<>
struct gobj_is_subclass<GObject, GObject>
{
    static const bool value = true;
    static GObject* down_cast(GObject* o) { return o; }
};

#define MOO_DEFINE_GOBJ_TYPE(Object, Parent, g_type)                                            \
namespace moo {                                                                                 \
                                                                                                \
    template<>                                                                                  \
    struct gobjinfo<Object>                                                                     \
    {                                                                                           \
        static const bool is_gobject = true;                                                    \
        using object_type = Object;                                                             \
        using parent_type = Parent;                                                             \
        static GType object_g_type() { return g_type; }                                         \
    };                                                                                          \
                                                                                                \
    template<>                                                                                  \
    struct gobj_is_subclass<Object, Object>                                                     \
    {                                                                                           \
        static const bool value = true;                                                         \
        static Object* down_cast(Object* o) { return o; }                                       \
    };                                                                                          \
                                                                                                \
    template<typename Super>                                                                    \
    struct gobj_is_subclass<Object, Super>                                                      \
    {                                                                                           \
        static const bool value = true;                                                         \
        static Super* down_cast(Object *o)                                                      \
        {                                                                                       \
            static_assert(gobj_is_subclass<Object, Super>::value,                               \
                          "In " __FUNCTION__ ": Super is not a superclass of " #Object);        \
            Parent* p = reinterpret_cast<Parent*>(o);                                           \
            Super* s = gobj_is_subclass<Parent, Super>::down_cast(p);                           \
            return s;                                                                           \
        }                                                                                       \
    };                                                                                          \
}                                                                                               \

#define MOO_DEFINE_NON_GOBJ_TYPE(Object)                                                        \
namespace moo {                                                                                 \
    template<> struct gobjinfo<Object> { static const bool is_gobject = false; };               \
}

template<typename Object>
using gobj_parent_type = typename gobjinfo<Object>::parent_type;

} // namespace moo

#endif // __cplusplus
