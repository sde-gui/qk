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

#include "moocpp/grefptr.h"
#include <glib-object.h>

namespace moo {

class mg_gobj_ref_unref
{
public:
    static void ref(gpointer obj) { g_object_ref(obj); }
    static void unref(gpointer obj) { g_object_unref(obj); }
};

template<typename GObjClas>
struct mg_gobjptr_methods;

template<typename GObjClas>
struct gobjref;

template<typename GObjClass>
class gobjptr
    : public grefptr<GObjClass, mg_gobj_ref_unref>
    , private gobjref<GObjClass>
    , public mg_gobjptr_methods<GObjClass>
{
    using base = grefptr<GObjClass, mg_gobj_ref_unref>;
    using objref = gobjref<GObjClass>;

public:
    using object_type = GObjClass;

    gobjptr() {}
    gobjptr(nullptr_t) {}

    gobjptr(GObjClass* obj, ref_transfer policy)
        : base(obj, policy)
    {
    }

    static gobjptr wrap_new(GObjClass* obj)
    {
        return gobjptr(obj, ref_transfer::take_ownership);
    }

    gobjptr(const gobjptr& other)
        : base(other)
    {
    }

    gobjptr(gobjptr&& other)
        : base(std::move(other))
    {
    }

    gobjptr& operator=(const gobjptr& other)
    {
        (static_cast<base&>(*this)) = other;
        return *this;
    }

    template<typename T>
    gobjptr& operator=(T* p)
    {
        (static_cast<base&>(*this)) = p;
        return *this;
    }

    gobjptr& operator=(const nullptr_t&)
    {
        (static_cast<base&>(*this)) = nullptr;
        return *this;
    }

    gobjptr& operator=(gobjptr&& other)
    {
        (static_cast<base&>(*this)) = std::move(other);
        return *this;
    }

    // Returning the GTypeInstance pointer is just as unsafe in general
    // as returning the pointer to the object, but it's unlikely that there
    // is code which gets a GTypeInstance and then calls g_object_unref()
    // on it. Normally GTypeInstance* is used inside FOO_WIDGET() macros,
    // where this conversion is safe.
    operator GTypeInstance* () const { return base::get() ? &G_OBJECT(base::get())->g_type_instance : nullptr; }

    template<typename T>
    T* get() const { return reinterpret_cast<T*>(base::get()); }

    GObjClass* get() const { return base::get(); }

    const objref* operator->() const { return this; }
    const objref& operator*() const { return *static_cast<const objref*>(this); }

    static const gobjptr& from_gobjref(const objref& ac) { return static_cast<const gobjptr&>(ac); }
};

} // namespace moo
