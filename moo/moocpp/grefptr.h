/*
 *   moocpp/grefptr.h
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

#include "moocpp/utils.h"

namespace moo {

template<typename Object>
class obj_ref_unref;

template<typename Object>
struct obj_class_ref_unref
{
    static void ref(Object* obj)   { obj->ref(); }
    static void unref(Object* obj) { obj->unref(); }
};

enum class ref_transfer
{
    take_ownership,
    make_copy,
};

template<typename Object,
    typename ObjRefUnref = obj_ref_unref<Object>>
class grefptr
{
public:
    grefptr() : m_p(nullptr) {}
    grefptr(const nullptr_t&) : grefptr() {}

    grefptr(Object* obj, ref_transfer policy)
        : grefptr()
    {
        assign(obj, policy);
    }

    template<typename ...Args>
    static grefptr create(Args&& ...args)
    {
        return wrap_new(new Object(std::forward<Args>(args)...));
    }

    static grefptr wrap_new(Object* obj)
    {
        return grefptr(obj, ref_transfer::take_ownership);
    }

    ~grefptr()
    {
        reset();
    }

    void ref(Object* obj)
    {
        assign(obj, ref_transfer::make_copy);
    }

    void take(Object* obj)
    {
        assign(obj, ref_transfer::take_ownership);
    }

    Object* release()
    {
        auto* tmp = m_p;
        m_p = nullptr;
        return tmp;
    }

    void reset()
    {
        auto* tmp = m_p;
        m_p = nullptr;
        if (tmp)
            ObjRefUnref::unref(tmp);
    }

    // Implicit conversion to Object* is dangerous because there is a lot
    // of code still which frees/steals objects directly. For example:
    // FooObject* tmp = x->s;
    // x->s = NULL;
    // g_object_unref (tmp);
    operator const Object* () const { return get(); }
    Object* get() const { return m_p; }
    Object& operator*() const { return *get(); }
    Object* operator->() const { return get(); }

    // Explicitly forbid other pointer conversions. This way it's still possible to implement
    // implicit conversions in subclasses, like that to GTypeInstance in gobjptr.
    template<typename T>
    operator T*() const = delete;

    operator bool() const { return get() != nullptr; }
    bool operator!() const { return get() == nullptr; }

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
        : grefptr(other.get(), ref_transfer::make_copy)
    {
    }

    grefptr(grefptr&& other)
        : grefptr()
    {
        this->m_p = other.m_p;
        other.m_p = nullptr;
    }

    grefptr& operator=(const grefptr& other)
    {
        assign(other.get(), ref_transfer::make_copy);
        return *this;
    }

    // Note that when T is const Foo, then assign(p) inside will be called with
    // a const Foo, which can't be converted to non-const Object*, so one can't
    // steal a reference to a const object with this method.
    template<typename T>
    grefptr& operator=(T* p)
    {
        assign(p, ref_transfer::take_ownership);
        return *this;
    }

    grefptr& operator=(const nullptr_t&)
    {
        reset();
        return *this;
    }

    grefptr& operator=(grefptr&& other)
    {
        if (get() != other.get())
        {
            assign(other.m_p, ref_transfer::take_ownership);
            other.m_p = nullptr;
        }
        
        return *this;
    }

private:
    void assign(Object* obj, ref_transfer policy)
    {
        if (get() != obj)
        {
            Object* tmp = get();
            m_p = obj;
            if (obj && (policy == ref_transfer::make_copy))
                ObjRefUnref::ref(obj);
            if (tmp)
                ObjRefUnref::unref(tmp);
        }
    }

private:
    Object* m_p;
};

} // namespace moo
