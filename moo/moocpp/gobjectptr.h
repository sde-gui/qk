/*
 *   moocpp/gobjectptr.h
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

#include "moocpp/utils.h"
#include <glib-object.h>

namespace moo {

template<typename ObjClass>
class mg_ref_unref;

enum class ref_transfer
{
    take_ownership,
    make_copy,
};

template<typename ObjClass, typename ObjRefUnrefHelper = mg_ref_unref<ObjClass>>
class grefptr
{
public:
    grefptr() : m_obj(nullptr) {}
    grefptr(const nullptr_t) : grefptr() {}

    grefptr(ObjClass* obj, ref_transfer policy)
        : grefptr()
    {
        assign(obj, policy);
    }

    ~grefptr()
    {
        reset();
    }

    void ref(ObjClass* obj)
    {
        assign(obj, ref_transfer::make_copy);
    }

    void take(ObjClass* obj)
    {
        assign(obj, ref_transfer::take_ownership);
    }

    void reset()
    {
        auto* tmp = m_obj;
        m_obj = nullptr;
        if (tmp)
            ObjRefUnrefHelper::unref(tmp);
    }

    // Implicit conversion to ObjClass* is dangerous because there is a lot
    // of code still which frees/steals objects directly. For example:
    // FooObject* tmp = x->s;
    // x->s = NULL;
    // g_object_unref (tmp);
    operator const ObjClass* () const { return m_obj; }
    ObjClass* get() const { return m_obj; }
    ObjClass& operator*() const { return *m_obj; }

    // Explicitly forbid other pointer conversions. This way it's still possible to implement
    // implicit conversions in subclasses, like that to GTypeInstance in gobjptr.
    template<typename T>
    operator T*() const = delete;

    operator bool() const { return m_obj != nullptr; }
    bool operator!() const { return m_obj == nullptr; }

    template<typename X>
    bool operator==(X* other) const
    {
        return get() == other;
    }

    template<typename X, typename Y>
    bool operator==(const grefptr<X, Y>& other) const
    {
        return get() == other.get();
    }

    template<typename X>
    bool operator!=(const X& anything) const
    {
        return !(*this == anything);
    }

    grefptr(const grefptr& other)
        : grefptr(other.get())
    {
    }

    grefptr(grefptr&& other)
        : m_obj(other.m_obj)
    {
        other.m_obj = nullptr;
    }

    grefptr& operator=(const grefptr& other)
    {
        assign(other.m_obj, ref_transfer::make_copy);
        return *this;
    }

    // Note that when T is const Foo, then assign(p) inside will be called with
    // a const Foo, which can't be converted to non-const ObjClass*, so one can't
    // steal a reference to a const object with this method.
    template<typename T>
    grefptr& operator=(T* p)
    {
        assign(p, ref_transfer::take_ownership);
    }

    grefptr& operator=(const nullptr_t&)
    {
        reset();
        return *this;
    }

    grefptr& operator=(grefptr&& other)
    {
        if (m_obj != other.m_obj)
        {
            assign(other.m_obj, ref_transfer::take_ownership);
            other.m_obj = nullptr;
        }
        
        return *this;
    }

private:
    void assign(ObjClass* obj, ref_transfer policy)
    {
        if (m_obj != obj)
        {
            ObjClass* tmp = m_obj;
            m_obj = obj;
            if (m_obj && (policy == ref_transfer::make_copy))
                ObjRefUnrefHelper::ref(m_obj);
            if (tmp)
                ObjRefUnrefHelper::unref(tmp);
        }
    }

private:
    ObjClass* m_obj;
};

class mg_gobj_ref_unref
{
public:
    static void ref(gpointer obj) { g_object_ref(obj); }
    static void unref(gpointer obj) { g_object_unref(obj); }
};

template<typename GObjClass>
class gobjptr : public grefptr<GObjClass, mg_gobj_ref_unref>
{
    using base = grefptr<GObjClass, mg_gobj_ref_unref>;

public:
    gobjptr() {}
    gobjptr(nullptr_t) {}

    gobjptr(GObjClass* obj, ref_transfer policy)
        : base(obj, policy)
    {
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
    operator GTypeInstance* () const { return get() ? &G_OBJECT(get())->g_type_instance : nullptr; }
};

} // namespace moo
