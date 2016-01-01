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
class ObjRefUnref;

template<typename ObjClass, typename ObjRefUnrefHelper = ObjRefUnref<ObjClass>>
class RefPtr
{
public:
    enum AssignmentPolicy
    {
        get_reference,
        steal_reference,
    };

    RefPtr() : RefPtr(nullptr) {}

    explicit RefPtr(ObjClass* obj, AssignmentPolicy policy = get_reference)
        : m_obj(nullptr)
    {
        assign(obj, policy);
    }

    ~RefPtr()
    {
        reset();
    }

    void ref(ObjClass* obj)
    {
        assign(obj, get_reference);
    }

    void take(ObjClass* obj)
    {
        assign(obj, steal_reference);
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

    operator bool() const { return m_obj != nullptr; }
    bool operator!() const { return m_obj == nullptr; }

    template<typename X>
    bool operator==(X* other) const
    {
        return get() == other;
    }

    template<typename X, typename Y>
    bool operator==(const RefPtr<X, Y>& other) const
    {
        return get() == other.get();
    }

    template<typename X>
    bool operator!=(const X& anything) const
    {
        return !(*this == anything);
    }

    RefPtr(const RefPtr& other)
        : RefPtr(other.get())
    {
    }

    RefPtr(RefPtr&& other)
        : m_obj(other.m_obj)
    {
        other.m_obj = nullptr;
    }

    RefPtr& operator=(const RefPtr& other)
    {
        assign(other.m_obj, get_reference);
        return *this;
    }

    // Note that when T is const Foo, then assign(p) inside will be called with
    // a const Foo, which can't be converted to non-const ObjClass*, so one can't
    // steal a reference to a const object with this method.
    template<typename T>
    RefPtr& operator=(T* p)
    {
        assign(p, steal_reference);
    }

    RefPtr& operator=(const nullptr_t&)
    {
        reset();
        return *this;
    }

    RefPtr& operator=(RefPtr&& other)
    {
        if (m_obj != other.m_obj)
        {
            assign(other.m_obj, steal_reference);
            other.m_obj = nullptr;
        }
        
        return *this;
    }

private:
    void assign(ObjClass* obj, AssignmentPolicy policy)
    {
        if (m_obj != obj)
        {
            ObjClass* tmp = m_obj;
            m_obj = obj;
            if (m_obj && (policy == get_reference))
                ObjRefUnrefHelper::ref(m_obj);
            if (tmp)
                ObjRefUnrefHelper::unref(tmp);
        }
    }

private:
    ObjClass* m_obj;
};

class GObjRefUnref
{
public:
    static void ref(gpointer obj) { g_object_ref(obj); }
    static void unref(gpointer obj) { g_object_unref(obj); }
};

template<typename GObjClass>
class GObjRefPtr : public RefPtr<GObjClass, GObjRefUnref>
{
    using base = RefPtr<GObjClass, GObjRefUnref>;

public:
    GObjRefPtr() {}

    explicit GObjRefPtr(GObjClass* obj, AssignmentPolicy policy = get_reference)
        : base(obj, policy)
    {
    }

    GObjRefPtr(const GObjRefPtr& other)
        : base(other)
    {
    }

    GObjRefPtr(GObjRefPtr&& other)
        : base(std::move(other))
    {
    }

    GObjRefPtr& operator=(const GObjRefPtr& other)
    {
        (static_cast<base&>(*this)) = other;
        return *this;
    }

    template<typename T>
    GObjRefPtr& operator=(T* p)
    {
        (static_cast<base&>(*this)) = p;
        return *this;
    }

    GObjRefPtr& operator=(const nullptr_t&)
    {
        (static_cast<base&>(*this)) = nullptr;
        return *this;
    }

    GObjRefPtr& operator=(GObjRefPtr&& other)
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
