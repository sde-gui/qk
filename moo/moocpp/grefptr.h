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
#include <glib-object.h>

namespace moo {

template<typename ObjClass>
class obj_handle
{
public:
    obj_handle() : m_p(nullptr) {}

    obj_handle(const obj_handle&) = delete;
    obj_handle& operator=(const obj_handle&) = delete;

protected:
    ObjClass* _get_pointer() const      { return m_p; }
    void      _set_pointer(ObjClass* p) { m_p = p; }

private:
    ObjClass* m_p;
};

template<typename ObjClass>
class obj_ref_unref;

template<typename ObjClass>
struct obj_class_ref_unref
{
    static void ref(ObjClass* obj)   { obj->ref(); }
    static void unref(ObjClass* obj) { obj->unref(); }
};

enum class ref_transfer
{
    take_ownership,
    make_copy,
};

template<typename ObjClass,
    typename ObjRefUnref = obj_ref_unref<ObjClass>,
    typename ObjHandle = obj_handle<ObjClass>>
class grefptr
    : private ObjHandle
{
public:
    grefptr() {}
    grefptr(const nullptr_t&) {}

    grefptr(ObjClass* obj, ref_transfer policy)
    {
        assign(obj, policy);
    }

    template<typename ...Args>
    static grefptr create(Args&& ...args)
    {
        return wrap_new(new ObjClass(std::forward<Args>(args)...));
    }

    static grefptr wrap_new(ObjClass* obj)
    {
        return grefptr(obj, ref_transfer::take_ownership);
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

    ObjClass* release()
    {
        auto* tmp = ObjHandle::_get_pointer();
        ObjHandle::_set_pointer(nullptr);
        return tmp;
    }

    void reset()
    {
        auto* tmp = ObjHandle::_get_pointer();
        ObjHandle::_set_pointer(nullptr);
        if (tmp)
            ObjRefUnref::unref(tmp);
    }

    // Implicit conversion to ObjClass* is dangerous because there is a lot
    // of code still which frees/steals objects directly. For example:
    // FooObject* tmp = x->s;
    // x->s = NULL;
    // g_object_unref (tmp);
    operator const ObjClass* () const { return get(); }
    ObjClass* get() const { return ObjHandle::_get_pointer(); }
    ObjClass& operator*() const { return *get(); }
    ObjClass* operator->() const { return get(); }

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
        this->ObjHandle::_set_pointer(other.get());
        (&other)->ObjHandle::_set_pointer(nullptr);
    }

    grefptr& operator=(const grefptr& other)
    {
        assign(other.get(), ref_transfer::make_copy);
        return *this;
    }

    // Note that when T is const Foo, then assign(p) inside will be called with
    // a const Foo, which can't be converted to non-const ObjClass*, so one can't
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
            assign(other.get(), ref_transfer::take_ownership);
            (&other)->ObjHandle::_set_pointer(nullptr);
        }
        
        return *this;
    }

private:
    void assign(ObjClass* obj, ref_transfer policy)
    {
        if (get() != obj)
        {
            ObjClass* tmp = get();
            ObjHandle::_set_pointer(obj);
            if (obj && (policy == ref_transfer::make_copy))
                ObjRefUnref::ref(obj);
            if (tmp)
                ObjRefUnref::unref(tmp);
        }
    }
};


//template<typename ObjClass, typename ObjRefUnrefHelper = mg_ref_unref<ObjClass>>
//class grefptr
//{
//public:
//    grefptr() : m_obj(nullptr) {}
//    grefptr(const nullptr_t) : grefptr() {}
//
//    grefptr(ObjClass* obj, ref_transfer policy)
//        : grefptr()
//    {
//        assign(obj, policy);
//    }
//
//    ~grefptr()
//    {
//        reset();
//    }
//
//    void ref(ObjClass* obj)
//    {
//        assign(obj, ref_transfer::make_copy);
//    }
//
//    void take(ObjClass* obj)
//    {
//        assign(obj, ref_transfer::take_ownership);
//    }
//
//    void reset()
//    {
//        auto* tmp = m_obj;
//        m_obj = nullptr;
//        if (tmp)
//            ObjRefUnrefHelper::unref(tmp);
//    }
//
//    // Implicit conversion to ObjClass* is dangerous because there is a lot
//    // of code still which frees/steals objects directly. For example:
//    // FooObject* tmp = x->s;
//    // x->s = NULL;
//    // g_object_unref (tmp);
//    operator const ObjClass* () const { return m_obj; }
//    ObjClass* get() const { return m_obj; }
//    ObjClass& operator*() const { return *m_obj; }
//
//    // Explicitly forbid other pointer conversions. This way it's still possible to implement
//    // implicit conversions in subclasses, like that to GTypeInstance in gobjptr.
//    template<typename T>
//    operator T*() const = delete;
//
//    operator bool() const { return m_obj != nullptr; }
//    bool operator!() const { return m_obj == nullptr; }
//
//    template<typename X>
//    bool operator==(X* other) const
//    {
//        return get() == other;
//    }
//
//    template<typename X, typename Y>
//    bool operator==(const grefptr<X, Y>& other) const
//    {
//        return get() == other.get();
//    }
//
//    template<typename X>
//    bool operator!=(const X& anything) const
//    {
//        return !(*this == anything);
//    }
//
//    grefptr(const grefptr& other)
//        : grefptr(other.get())
//    {
//    }
//
//    grefptr(grefptr&& other)
//        : m_obj(other.m_obj)
//    {
//        other.m_obj = nullptr;
//    }
//
//    grefptr& operator=(const grefptr& other)
//    {
//        assign(other.m_obj, ref_transfer::make_copy);
//        return *this;
//    }
//
//    // Note that when T is const Foo, then assign(p) inside will be called with
//    // a const Foo, which can't be converted to non-const ObjClass*, so one can't
//    // steal a reference to a const object with this method.
//    template<typename T>
//    grefptr& operator=(T* p)
//    {
//        assign(p, ref_transfer::take_ownership);
//    }
//
//    grefptr& operator=(const nullptr_t&)
//    {
//        reset();
//        return *this;
//    }
//
//    grefptr& operator=(grefptr&& other)
//    {
//        if (m_obj != other.m_obj)
//        {
//            assign(other.m_obj, ref_transfer::take_ownership);
//            other.m_obj = nullptr;
//        }
//        
//        return *this;
//    }
//
//private:
//    void assign(ObjClass* obj, ref_transfer policy)
//    {
//        if (m_obj != obj)
//        {
//            ObjClass* tmp = m_obj;
//            m_obj = obj;
//            if (m_obj && (policy == ref_transfer::make_copy))
//                ObjRefUnrefHelper::ref(m_obj);
//            if (tmp)
//                ObjRefUnrefHelper::unref(tmp);
//        }
//    }
//
//private:
//    ObjClass* m_obj;
//};

} // namespace moo
