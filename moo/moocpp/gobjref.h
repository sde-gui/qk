/*
 *   moocpp/gobjptr.h
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

#include "moocpp/gobjinfo.h"

namespace moo {

template<typename Object>
class gobj_ref;
template<typename Object>
class gobj_ptr;
template<typename Object>
class gobj_ptr_impl;
template<typename Object>
class gobj_raw_ptr;

///////////////////////////////////////////////////////////////////////////////////////////
//
// gobj_ref
//

class gobj_ref_base
{
protected:
    gobj_ref_base() : m_gobj(nullptr) {}
    using object_type = GObject;

public:
    gobj_ref_base(const gobj_ref_base&) = default;
    gobj_ref_base& operator=(const gobj_ref_base&) = default;

    gobj_ref_base(gobj_ref_base&& other)
        : m_gobj(other.m_gobj)
    {
        other.m_gobj = nullptr;
    }

    gobj_ref_base& operator=(gobj_ref_base&& other)
    {
        m_gobj = other.m_gobj;
        other.m_gobj = nullptr;
        return *this;
    }

    GObject* gobj() { return m_gobj; }
    const GObject* gobj() const { return m_gobj; }
    operator GObject&() { return *m_gobj; }
    operator const GObject&() const { return *m_gobj; }
    operator GTypeInstance&() { return *reinterpret_cast<GTypeInstance*>(m_gobj); }
    operator const GTypeInstance&() const { return *reinterpret_cast<const GTypeInstance*>(m_gobj); }

protected:
    GObject* raw_gobj() const { return const_cast<GObject*>(m_gobj); }

private:
    template<typename Object> friend class gobj_ptr_impl;
    template<typename Object> friend class gobj_raw_ptr;
    template<typename Object> friend class gobj_ref;

    void _set_gobj(gpointer gobj) { m_gobj = reinterpret_cast<GObject*>(gobj); }

private:
    GObject* m_gobj;
};

template<>
class gobj_ref<GObject>; // : public gobj_ref_base

#define MOO_DEFINE_GOBJREF_METHODS_IMPL(Object, Super)                                  \
    using super = Super;                                                                \
    using object_type = Object;                                                         \
    using parent_object_type = typename super::object_type;                             \
    using ptrtype = gobj_ptr<object_type>;                                              \
                                                                                        \
protected:                                                                              \
    friend class gobj_ptr_impl<object_type>;                                            \
    friend class gobj_raw_ptr<object_type>;                                             \
    friend class gobj_raw_ptr<const object_type>;                                       \
                                                                                        \
    gobj_ref() {}                                                                       \
                                                                                        \
public:                                                                                 \
    gobj_ref(object_type& gobj)                                                         \
    {                                                                                   \
        _set_gobj(&gobj);                                                               \
    }                                                                                   \
                                                                                        \
    object_type* gobj()                                                                 \
    {                                                                                   \
        return reinterpret_cast<object_type*>(raw_gobj());                              \
    }                                                                                   \
                                                                                        \
    const object_type* gobj() const                                                     \
    {                                                                                   \
        return reinterpret_cast<const object_type*>(raw_gobj());                        \
    }                                                                                   \
                                                                                        \
    template<typename X>                                                                \
    X* gobj()                                                                           \
    {                                                                                   \
        return nc_gobj<X>();                                                            \
    }                                                                                   \
                                                                                        \
    template<typename X>                                                                \
    const X* gobj() const                                                               \
    {                                                                                   \
        return nc_gobj<X>();                                                            \
    }                                                                                   \
                                                                                        \
    object_type* nc_gobj() const                                                        \
    {                                                                                   \
        return const_cast<object_type*>(gobj());                                        \
    }                                                                                   \
                                                                                        \
    template<typename X>                                                                \
    X* nc_gobj() const                                                                  \
    {                                                                                   \
        object_type* o = const_cast<object_type*>(gobj());                              \
        return gobj_is_subclass<Object, X>::down_cast(o);                               \
    }                                                                                   \
                                                                                        \
    gobj_ref* self() { return this; }                                                   \
    const gobj_ref* self() const { return this; }                                       \
                                                                                        \
    operator object_type&() { return *gobj(); }                                         \
    operator const object_type&() const { return *gobj(); }                             \
    gobj_raw_ptr<object_type> operator&() { return nc_gobj(); }                         \
    gobj_raw_ptr<const object_type> operator&() const { return nc_gobj(); }             \
                                                                                        \
    gobj_ref(const gobj_ref&) = default;                                                \
    gobj_ref& operator=(const gobj_ref&) = default;                                     \
                                                                                        \
    gobj_ref(gobj_ref&& other)                                                          \
        : super(std::move(static_cast<super&&>(other)))                                 \
    {                                                                                   \
    }                                                                                   \
                                                                                        \
    gobj_ref& operator=(gobj_ref&& other)                                               \
    {                                                                                   \
        super::operator=(std::move(static_cast<super&&>(other)));                       \
        return *this;                                                                   \
    }

#define MOO_DEFINE_GOBJREF_METHODS(Object)                                      \
    MOO_DEFINE_GOBJREF_METHODS_IMPL(Object, gobj_ref<gobj_parent_type<Object>>)


template<typename Object>
using gobj_ref_parent = gobj_ref<gobj_parent_type<Object>>;

// Generic implementation, falls back to the parent type's gobj_ref implementation
// if that's known, or to GObject's one, coming from the generic gobjinfo.
template<typename Object>
class gobj_ref : public gobj_ref_parent<Object>
{
public:
    MOO_DEFINE_GOBJREF_METHODS(Object)
};

} // namespace moo

// Make sure these aren't called in code ported from pure glib C
template<typename X>
void g_object_unref(const moo::gobj_ref<X>*) = delete;
template<typename X>
void g_free(const moo::gobj_ref<X>*) = delete;

void extern_g_free(gpointer);
void extern_g_object_unref(gpointer);
