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

#include "moocpp/gobjrawptr.h"
#include "moocpp/grefptr.h"
#include "moocpp/gobjectutils.h"

namespace moo {

///////////////////////////////////////////////////////////////////////////////////////////
//
// gobj_ptr
//

template<typename Object>
class gobj_ptr_impl
{
    using ptr_type = gobj_ptr<Object>;
    using ref_type = gobj_ref<Object>;

public:
    gobj_ptr_impl() {}

    gobj_ptr_impl(Object* obj, ref_transfer policy)
    {
        assign(obj, policy);
    }

    static ptr_type wrap_new(Object* obj)
    {
        return ptr_type(obj, ref_transfer::take_ownership);
    }

    static ptr_type wrap(Object* obj)
    {
        return ptr_type(obj, ref_transfer::make_copy);
    }

    ~gobj_ptr_impl()
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
        auto* tmp = gobj();
        m_ref._set_gobj(nullptr);
        return tmp;
    }

    void reset()
    {
        auto* tmp = gobj();
        if (tmp)
        {
            m_ref._set_gobj(nullptr);
            g_object_unref(tmp);
        }
    }

    // Implicit conversion to non-const Object* is dangerous because there is a lot
    // of code still which frees/steals objects directly. For example:
    // FooObject* tmp = x->s;
    // x->s = NULL;
    // g_object_unref (tmp);
    operator const Object* () const { return gobj(); }
    operator ref_type*() const { return m_ref.self(); }
    ref_type* operator->() const { return m_ref.self(); }
    ref_type& operator*() const { return m_ref; }

    // These are nasty. Because of ref_type* conversion this can be converted to void*,
    // which in turn can be passed to g_object_ref or g_free, etc.
    operator void*() const = delete;
    operator const void*() const = delete;

    Object* gobj() const { return m_ref.gobj(); }

    template<typename Super>
    Super* gobj() const
    {
        return gobj_is_subclass<Object, Super>::down_cast(m_ref.gobj());
    }

    template<typename Super>
    operator const Super* () const { return gobj<Super>(); }

    operator bool() const { return gobj() != nullptr; }
    bool operator!() const { return gobj() == nullptr; }

    gobj_ptr_impl(const gobj_ptr_impl& other) = delete;
    gobj_ptr_impl& operator=(const gobj_ptr_impl& other) = delete;

    gobj_ptr_impl(gobj_ptr_impl&& other)
        : gobj_ptr_impl()
    {
        m_ref._set_gobj(other.gobj());
        other.m_ref._set_gobj(nullptr);
    }

    // Note that when T is const Foo, then assign(p) inside will be called with
    // a const Foo, which can't be converted to non-const Object*, so one can't
    // steal a reference to a const object with this method.
    template<typename T>
    gobj_ptr_impl& operator=(T* p)
    {
        assign(p, ref_transfer::take_ownership);
        return *this;
    }

    gobj_ptr_impl& operator=(const nullptr_t&)
    {
        reset();
        return *this;
    }

    gobj_ptr_impl& operator=(gobj_ptr_impl&& other)
    {
        if (gobj() != other.gobj())
        {
            assign(other.gobj(), ref_transfer::take_ownership);
            other.m_ref._set_gobj(nullptr);
        }

        return *this;
    }

private:
    void assign(Object* obj, ref_transfer policy)
    {
        if (gobj() != obj)
        {
            Object* tmp = gobj();
            m_ref._set_gobj(obj);

            if (obj)
            {
                if (policy == ref_transfer::make_copy)
                    g_object_ref(obj);
                else if (g_object_is_floating(obj))
                    g_object_ref_sink(obj);
            }

            if (tmp)
                g_object_unref(tmp);
        }
    }

private:
    mutable gobj_ref<Object> m_ref;
};

template<typename Object>
inline gobj_ptr<Object> wrap_new(Object *obj)
{
    return gobj_ptr<Object>::wrap_new(obj);
}

template<typename Object>
inline gobj_ptr<Object> wrap(Object* obj)
{
    return gobj_ptr<Object>::wrap(obj);
}

template<typename Object>
inline gobj_ptr<Object> wrap(const gobj_raw_ptr<Object>& obj)
{
    return gobj_ptr<Object>::wrap(obj);
}

#define MOO_DEFINE_GOBJPTR_METHODS(Object)                                  \
    using ref_type = gobj_ref<Object>;                                      \
    using impl_type = gobj_ptr_impl<Object>;                                \
                                                                            \
    gobj_ptr() {}                                                           \
    gobj_ptr(const nullptr_t) {}                                            \
                                                                            \
    gobj_ptr(Object* obj, ref_transfer policy)                              \
        : impl_type(obj, policy)                                            \
    {                                                                       \
    }                                                                       \
                                                                            \
    gobj_ptr(const gobj_ptr& other) = delete;                               \
    gobj_ptr& operator=(const gobj_ptr& other) = delete;                    \
                                                                            \
    gobj_ptr(gobj_ptr&& other)                                              \
        : impl_type(std::move(static_cast<impl_type&&>(other)))             \
    {                                                                       \
    }                                                                       \
                                                                            \
    gobj_ptr& operator=(gobj_ptr&& other)                                   \
    {                                                                       \
        impl_type::operator=(std::move(static_cast<impl_type&&>(other)));   \
        return *this;                                                       \
    }                                                                       \
                                                                            \
    gobj_ptr& operator=(const nullptr_t&)                                   \
    {                                                                       \
        reset();                                                            \
        return *this;                                                       \
    }


// Generic implementation.
template<typename Object>
class gobj_ptr : public gobj_ptr_impl<Object>
{
public:
    MOO_DEFINE_GOBJPTR_METHODS(Object)
};

template<> class gobj_ref<GObject>;


template<typename X>
void g_object_unref(const gobj_ptr<X>&) = delete;
template<typename X>
void g_free(const gobj_ptr<X>&) = delete;


template<typename T, typename ...Args>
inline gobj_ptr<T> create_gobj(GType obj_type, Args&& ...args)
{
    return wrap_new(reinterpret_cast<T*>(g_object_new(obj_type, std::forward<Args>(args)...)));
}

template<typename T, typename ...Args>
inline gobj_ptr<T> create_gobj(Args&& ...args)
{
    // object_g_type() will produce a compiler error if the type wasn't registered
    return create_gobj(gobjinfo<T>::object_g_type(), std::forward<Args>(args)...);
}

template<typename T>
inline gobj_ptr<T> create_gobj()
{
    // object_g_type() will produce a compiler error if the type wasn't registered
    return create_gobj<T>(gobjinfo<T>::object_g_type(), nullptr);
}


} // namespace moo

template<typename X>
inline bool operator==(const moo::gobj_ptr<X>& p, const nullptr_t&)
{
    return p.gobj() == nullptr;
}

template<typename X>
inline bool operator==(const nullptr_t&, const moo::gobj_ptr<X>& p)
{
    return p.gobj() == nullptr;
}

template<typename X, typename Y>
inline bool operator==(const moo::gobj_ptr<X>& p1, const moo::gobj_ptr<Y>& p2)
{
    return p1.gobj() == p2.gobj();
}

template<typename X, typename Y>
inline bool operator==(const moo::gobj_ptr<X>& p1, const Y* p2)
{
    return p1.gobj() == p2;
}

template<typename X, typename Y>
inline bool operator==(const X* p1, const moo::gobj_ptr<Y>& p2)
{
    return p1 == p2.gobj();
}

template<typename X, typename Y>
inline bool operator==(const moo::gobj_ptr<X>& p1, const moo::gobj_raw_ptr<Y>& p2)
{
    return p1.gobj() == p2.gobj();
}

template<typename X, typename Y>
inline bool operator==(const moo::gobj_raw_ptr<Y>& p1, const moo::gobj_ptr<X>& p2)
{
    return p1.gobj() == p2.gobj();
}

template<typename X, typename Y>
bool operator!=(const moo::gobj_ptr<X>& p1, const moo::gobj_ptr<Y>& p2)
{
    return !(p1 == p2);
}

template<typename X, typename Y>
bool operator!=(const moo::gobj_ptr<X>& p1, const Y& p2)
{
    return !(p1 == p2);
}

template<typename X, typename Y>
bool operator!=(const X& p1, const moo::gobj_ptr<Y>& p2)
{
    return !(p1 == p2);
}
