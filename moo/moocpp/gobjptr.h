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

namespace moo {

///////////////////////////////////////////////////////////////////////////////////////////
//
// gobjptr
//

template<typename Object>
class gobjptr_impl
{
    using ptr_type = gobjptr<Object>;
    using ref_type = gobjref<Object>;

public:
    gobjptr_impl() {}

    gobjptr_impl(Object* obj, ref_transfer policy)
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

    ~gobjptr_impl()
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
        auto* tmp = get();
        m_ref._set_gobj(nullptr);
        return tmp;
    }

    void reset()
    {
        auto* tmp = get();
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
    operator const Object* () const { return get(); }
    operator ref_type*() const { return m_ref.self(); }
    ref_type* operator->() const { return m_ref.self(); }
    ref_type& operator*() const { return m_ref; }

    Object* get() const { return m_ref.gobj(); }

    template<typename Super>
    Super* get() const
    {
        return gobj_is_subclass<Object, Super>::down_cast(m_ref.gobj());
    }

    template<typename Super>
    operator const Super* () const { return get<Super>(); }

    operator bool() const { return get() != nullptr; }
    bool operator!() const { return get() == nullptr; }

    gobjptr_impl(const gobjptr_impl& other) = delete;
    gobjptr_impl& operator=(const gobjptr_impl& other) = delete;

    gobjptr_impl(gobjptr_impl&& other)
        : gobjptr_impl()
    {
        m_ref._set_gobj(other.get());
        other.m_ref._set_gobj(nullptr);
    }

    // Note that when T is const Foo, then assign(p) inside will be called with
    // a const Foo, which can't be converted to non-const Object*, so one can't
    // steal a reference to a const object with this method.
    template<typename T>
    gobjptr_impl& operator=(T* p)
    {
        assign(p, ref_transfer::take_ownership);
        return *this;
    }

    gobjptr_impl& operator=(const nullptr_t&)
    {
        reset();
        return *this;
    }

    gobjptr_impl& operator=(gobjptr_impl&& other)
    {
        if (get() != other.get())
        {
            assign(other.get(), ref_transfer::take_ownership);
            other.m_ref._set_gobj(nullptr);
        }

        return *this;
    }

private:
    void assign(Object* obj, ref_transfer policy)
    {
        if (get() != obj)
        {
            Object* tmp = get();
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
    mutable gobjref<Object> m_ref;
};

template<typename Object>
inline gobjptr<Object> wrap_new(Object *obj)
{
    return gobjptr<Object>::wrap_new(obj);
}

template<typename Object>
inline gobjptr<Object> wrap(Object* obj)
{
    return gobjptr<Object>::wrap(obj);
}

template<typename Object>
inline gobjptr<Object> wrap(gobjref<Object>& obj)
{
    return gobjptr<Object>::wrap(obj.gobj());
}

#define MOO_DEFINE_GOBJPTR_METHODS(Object)                                  \
    using ref_type = gobjref<Object>;                                       \
    using impl_type = gobjptr_impl<Object>;                                 \
                                                                            \
    gobjptr() {}                                                            \
    gobjptr(const nullptr_t) {}                                             \
                                                                            \
    gobjptr(Object* obj, ref_transfer policy)                               \
        : impl_type(obj, policy)                                            \
    {                                                                       \
    }                                                                       \
                                                                            \
    gobjptr(const gobjptr& other) = delete;                                 \
    gobjptr& operator=(const gobjptr& other) = delete;                      \
                                                                            \
    gobjptr(gobjptr&& other)                                                \
        : impl_type(std::move(static_cast<impl_type&&>(other)))             \
    {                                                                       \
    }                                                                       \
                                                                            \
    gobjptr& operator=(gobjptr&& other)                                     \
    {                                                                       \
        impl_type::operator=(std::move(static_cast<impl_type&&>(other)));   \
        return *this;                                                       \
    }                                                                       \
                                                                            \
    gobjptr& operator=(const nullptr_t&)                                    \
    {                                                                       \
        reset();                                                            \
        return *this;                                                       \
    }


// Generic implementation.
template<typename Object>
class gobjptr : public gobjptr_impl<Object>
{
public:
    MOO_DEFINE_GOBJPTR_METHODS(Object)
};

template<> class gobjref<GObject>;


template<typename X>
void g_object_unref(const moo::gobjptr<X>&);
template<typename X>
void g_free(const moo::gobjptr<X>&);

} // namespace moo

template<typename X>
inline bool operator==(const moo::gobjptr<X>& p, const nullptr_t&)
{
    return p.get() == nullptr;
}

template<typename X>
inline bool operator==(const nullptr_t&, const moo::gobjptr<X>& p)
{
    return p.get() == nullptr;
}

template<typename X, typename Y>
inline bool operator==(const moo::gobjptr<X>& p1, const moo::gobjptr<Y>& p2)
{
    return p1.get() == p2.get();
}

template<typename X, typename Y>
inline bool operator==(const moo::gobjptr<X>& p1, const Y* p2)
{
    return p1.get() == p2;
}

template<typename X, typename Y>
inline bool operator==(const X* p1, const moo::gobjptr<Y>& p2)
{
    return p1 == p2.get();
}

template<typename X, typename Y>
inline bool operator==(const moo::gobjptr<X>& p1, const moo::gobj_raw_ptr<Y>& p2)
{
    return p1.get() == p2.get();
}

template<typename X, typename Y>
inline bool operator==(const moo::gobj_raw_ptr<Y>& p1, const moo::gobjptr<X>& p2)
{
    return p1.get() == p2.get();
}

template<typename X, typename Y>
bool operator!=(const moo::gobjptr<X>& p1, const moo::gobjptr<Y>& p2)
{
    return !(p1 == p2);
}

template<typename X, typename Y>
bool operator!=(const moo::gobjptr<X>& p1, const Y& p2)
{
    return !(p1 == p2);
}

template<typename X, typename Y>
bool operator!=(const X& p1, const moo::gobjptr<Y>& p2)
{
    return !(p1 == p2);
}
