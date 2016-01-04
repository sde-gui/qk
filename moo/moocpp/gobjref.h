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
class gobjref;
template<typename Object>
class gobjptr;
template<typename Object>
class gobjptr_impl;
template<typename Object>
class gobj_raw_ptr;

///////////////////////////////////////////////////////////////////////////////////////////
//
// gobjref
//

class gobjref_base
{
protected:
    gobjref_base() : m_gobj(nullptr) {}
    using object_type = GObject;

public:
    gobjref_base(const gobjref_base&) = default;
    gobjref_base& operator=(const gobjref_base&) = default;

    gobjref_base(gobjref_base&& other)
        : m_gobj(other.m_gobj)
    {
        other.m_gobj = nullptr;
    }

    gobjref_base& operator=(gobjref_base&& other)
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
    template<typename Object> friend class gobjptr_impl;
    template<typename Object> friend class gobj_raw_ptr;
    template<typename Object> friend class gobjref;

    void _set_gobj(gpointer gobj) { m_gobj = reinterpret_cast<GObject*>(gobj); }

private:
    GObject* m_gobj;
};

template<>
class gobjref<GObject>; // : public gobjref_base

#define MOO_DEFINE_GOBJREF_METHODS_IMPL(Object, Super)                          \
    using super = Super;                                                        \
    using object_type = Object;                                                 \
    using parent_object_type = typename super::object_type;                     \
    using ptrtype = gobjptr<object_type>;                                       \
                                                                                \
protected:                                                                      \
    friend class gobjptr_impl<object_type>;                                     \
    friend class gobj_raw_ptr<object_type>;                                     \
    friend class gobj_raw_ptr<const object_type>;                               \
                                                                                \
    gobjref() {}                                                                \
                                                                                \
public:                                                                         \
    gobjref(object_type& gobj)                                                  \
    {                                                                           \
        _set_gobj(&gobj);                                                       \
    }                                                                           \
                                                                                \
    object_type* gobj()                                                         \
    {                                                                           \
        return reinterpret_cast<object_type*>(raw_gobj());                      \
    }                                                                           \
                                                                                \
    const object_type* gobj() const                                             \
    {                                                                           \
        return reinterpret_cast<const object_type*>(raw_gobj());                \
    }                                                                           \
                                                                                \
    template<typename X>                                                        \
    X* gobj()                                                                   \
    {                                                                           \
        return g<X>();                                                          \
    }                                                                           \
                                                                                \
    template<typename X>                                                        \
    const X* gobj() const                                                       \
    {                                                                           \
        return g<X>();                                                          \
    }                                                                           \
                                                                                \
    object_type* g() const                                                      \
    {                                                                           \
        return const_cast<object_type*>(gobj());                                \
    }                                                                           \
                                                                                \
    template<typename X>                                                        \
    X* g() const                                                                \
    {                                                                           \
        object_type* o = const_cast<object_type*>(gobj());                      \
        return gobj_is_subclass<Object, X>::down_cast(o);                       \
    }                                                                           \
                                                                                \
    gobjref* self() { return this; }                                            \
    const gobjref* self() const { return this; }                                \
                                                                                \
    operator object_type&() { return *gobj(); }                                 \
    operator const object_type&() const { return *gobj(); }                     \
    gobj_raw_ptr<object_type> operator&() { return g(); }                       \
    gobj_raw_ptr<const object_type> operator&() const { return g(); }           \
                                                                                \
    gobjref(const gobjref&) = default;                                          \
    gobjref& operator=(const gobjref&) = default;                               \
                                                                                \
    gobjref(gobjref&& other)                                                    \
        : super(std::move(static_cast<super&&>(other)))                         \
    {                                                                           \
    }                                                                           \
                                                                                \
    gobjref& operator=(gobjref&& other)                                         \
    {                                                                           \
        super::operator=(std::move(static_cast<super&&>(other)));               \
        return *this;                                                           \
    }

#define MOO_DEFINE_GOBJREF_METHODS(Object)                                      \
    MOO_DEFINE_GOBJREF_METHODS_IMPL(Object, gobjref<gobj_parent_type<Object>>)


template<typename Object>
using gobjref_parent = gobjref<gobj_parent_type<Object>>;

// Generic implementation, falls back to the parent type's gobjref implementation
// if that's known, or to GObject's one, coming from the generic gobjinfo.
template<typename Object>
class gobjref : public gobjref_parent<Object>
{
public:
    MOO_DEFINE_GOBJREF_METHODS(Object)
};

template<typename X>
void g_object_unref(const moo::gobjref<X>*);
template<typename X>
void g_free(const moo::gobjref<X>*);

} // namespace moo
