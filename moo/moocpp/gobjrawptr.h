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

#include "moocpp/gobjref.h"

namespace moo {

///////////////////////////////////////////////////////////////////////////////////////////
//
// gobj_raw_ptr
//

template<typename Object>
class gobj_raw_ptr
{
    using ref_type = gobj_ref<Object>;

public:
    gobj_raw_ptr(Object* obj = nullptr) { m_ref._set_gobj(obj); }

    operator Object*() const { return gobj(); }
    operator GTypeInstance*() const { return reinterpret_cast<GTypeInstance*>(gobj()); }
    operator gpointer() const { return gobj(); }
    operator gobj_ref<Object>*() const { return m_ref.self(); }

    ref_type* operator->() const { return m_ref.self(); }
    ref_type& operator*() const { return m_ref; }

    Object* gobj() const { return m_ref.gobj(); }
    void set(Object* p) { m_ref._set_gobj(p); }

    template<typename Super>
    Super* gobj() const
    {
        return gobj_is_subclass<Object, Super>::down_cast(m_ref.gobj());
    }

    template<typename Subclass>
    void set(Subclass* p)
    {
        set(gobj_is_subclass<Subclass, Object>::down_cast(p));
    }

    operator bool() const { return gobj() != nullptr; }
    bool operator!() const { return gobj() == nullptr; }

    gobj_raw_ptr(const gobj_raw_ptr& other) = default;
    gobj_raw_ptr& operator=(const gobj_raw_ptr& other) = default;

    gobj_raw_ptr(gobj_raw_ptr&& other)
        : m_ref(std::move(other.m_ref))
    {
        other.set(nullptr);
    }

    gobj_raw_ptr& operator=(gobj_raw_ptr&& other)
    {
        m_ref = std::move(other.m_ref);
        return *this;
    }

    template<typename T>
    gobj_raw_ptr& operator=(T* p)
    {
        set(p);
        return *this;
    }

private:
    mutable gobj_ref<Object> m_ref;
};

template<typename Object>
class gobj_raw_ptr<const Object>
{
    using ref_type = gobj_ref<Object>;

public:
    gobj_raw_ptr(const Object* obj = nullptr) { m_ref._set_gobj(const_cast<Object*>(obj)); }

    operator const Object*() const { return gobj(); }
    operator const GTypeInstance*() const { return reinterpret_cast<GTypeInstance*>(gobj()); }
    operator const void*() const { return gobj(); }
    operator const gobj_ref<Object>*() const { return m_ref.self(); }

    const ref_type* operator->() const { return m_ref.self(); }
    const ref_type& operator*() const { return m_ref; }

    const Object* gobj() const { return m_ref.gobj(); }
    void set(const Object* p) { m_ref._set_gobj(p); }

    template<typename Super>
    const Super* gobj() const
    {
        return gobj_is_subclass<Object, Super>::down_cast(m_ref.gobj());
    }

    template<typename Subclass>
    void set(const Subclass* p)
    {
        set(gobj_is_subclass<Subclass, Object>::down_cast(p));
    }

    operator bool() const { return gobj() != nullptr; }
    bool operator!() const { return gobj() == nullptr; }

    gobj_raw_ptr(const gobj_raw_ptr& other) = default;
    gobj_raw_ptr& operator=(const gobj_raw_ptr& other) = default;

    gobj_raw_ptr(gobj_raw_ptr&& other)
        : m_ref(other.gobj())
    {
        other = nullptr;
    }

    gobj_raw_ptr& operator=(gobj_raw_ptr&& other)
    {
        m_ref._set_gobj(other.gobj());
        other.m_ref._set_gobj(nullptr);
        return *this;
    }

    template<typename T>
    gobj_raw_ptr& operator=(const T* p)
    {
        set(p);
        return *this;
    }

private:
    mutable gobj_ref<Object> m_ref;
};

} // namespace moo

template<typename X>
inline bool operator==(const moo::gobj_raw_ptr<X>& p, const nullptr_t&)
{
    return p.gobj() == nullptr;
}

template<typename X>
inline bool operator==(const nullptr_t&, const moo::gobj_raw_ptr<X>& p)
{
    return p.gobj() == nullptr;
}

template<typename X>
inline bool operator==(const moo::gobj_raw_ptr<X>& p1, const moo::gobj_raw_ptr<X>& p2)
{
    return p1.gobj() == p2.gobj();
}

template<typename X>
inline bool operator==(const moo::gobj_raw_ptr<X>& p1, const X* p2)
{
    return p1.gobj() == p2;
}

template<typename X>
inline bool operator==(const moo::gobj_raw_ptr<X>& p1, X* p2)
{
    return p1.gobj() == p2;
}

template<typename X>
inline bool operator==(const X* p1, const moo::gobj_raw_ptr<X>& p2)
{
    return p1 == p2.gobj();
}

template<typename X>
inline bool operator==(X* p1, const moo::gobj_raw_ptr<X>& p2)
{
    return p1 == p2.gobj();
}

template<typename X>
bool operator!=(const moo::gobj_raw_ptr<X>& p1, const moo::gobj_raw_ptr<X>& p2)
{
    return !(p1 == p2);
}

template<typename X, typename Y>
bool operator!=(const moo::gobj_raw_ptr<X>& p1, const Y& p2)
{
    return !(p1 == p2);
}

template<typename X, typename Y>
bool operator!=(const X& p1, const moo::gobj_raw_ptr<Y>& p2)
{
    return !(p1 == p2);
}
