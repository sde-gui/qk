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

#include <type_traits>

namespace moo {

///////////////////////////////////////////////////////////////////////////////////////////
//
// gobjinfo
//

template<typename Object, typename Super>
struct gobj_is_subclass
{
    static const bool value = false;
};

// Generic implementation, all we know it's a subclass of GObject; we don't
// even know its GType. This implementation is needed so that it's possible
// to have gobjptr<GAnything> without having to define gobjinfo for it.
template<typename Object>
struct gobjinfo
{
    using object_type = Object;
    using parent_type = GObject;
    static GType object_g_type() { return G_TYPE_OBJECT; }
    static GType parent_g_type() { return G_TYPE_OBJECT; }
};

template<>
struct gobjinfo<GObject>
{
    using object_type = GObject;
    static GType object_g_type() { return G_TYPE_OBJECT; }
    static GType parent_g_type() { return G_TYPE_NONE; }
};

template<>
struct gobj_is_subclass<GObject, GObject>
{
    static const bool value = true;
    static GObject* down_cast(GObject* o) { return o; }
};

#define MOO_DEFINE_GOBJ_TYPE_COMMON(Object, Parent, g_type)                                     \
    template<>                                                                                  \
    struct gobjinfo<Object>                                                                     \
    {                                                                                           \
        using object_type = Object;                                                             \
        using parent_type = Parent;                                                             \
        static GType object_g_type() { return g_type; }                                         \
        static GType parent_g_type() { return gobjinfo<Parent>::object_g_type(); }              \
    };                                                                                          \
                                                                                                \
    template<>                                                                                  \
    struct gobj_is_subclass<Object, Object>                                                     \
    {                                                                                           \
        static const bool value = true;                                                         \
        static Object* down_cast(Object* o) { return o; }                                       \
    };


#define MOO_DEFINE_GOBJ_CHILD_TYPE(Object, g_type)                                              \
    MOO_DEFINE_GOBJ_TYPE_COMMON(Object, GObject, g_type)                                        \
                                                                                                \
    template<typename Super>                                                                    \
    struct gobj_is_subclass<Object, Super>                                                      \
    {                                                                                           \
        static const bool value = true;                                                         \
        static Super* down_cast(Object *o)                                                      \
        {                                                                                       \
            static_assert(std::is_same<Super, GObject>::value,                                  \
                "In " __FUNCTION__ ": Super is not a superclass of " #Object);                  \
            return reinterpret_cast<Super*>(o);                                                 \
        }                                                                                       \
    };


#define MOO_DEFINE_GOBJ_TYPE(Object, Parent, g_type)                                            \
    static_assert(!std::is_same<Parent, GObject>::value,                                        \
                  "Use MOO_DEFINE_GOBJ_CHILD_TYPE for child classes of GObject");               \
                                                                                                \
    MOO_DEFINE_GOBJ_TYPE_COMMON(Object, GObject, g_type)                                        \
                                                                                                \
    template<typename Super>                                                                    \
    struct gobj_is_subclass<Object, Super>                                                      \
    {                                                                                           \
        static const bool value = true;                                                         \
        static Super* down_cast(Object *o)                                                      \
        {                                                                                       \
            static_assert(gobj_is_subclass<Parent, Super>::value,                               \
                          "In " __FUNCTION__ ": Super is not a superclass of " #Object);        \
            return gobj_is_subclass<Parent, Super>::down_cast(reinterpret_cast<Parent*>(o));    \
        }                                                                                       \
    };


template<typename Object>
class gobjref;
template<typename Object>
class gobjptr;
template<typename Object>
class gobjptr_impl;

///////////////////////////////////////////////////////////////////////////////////////////
//
// gobjref
//

class gobjref_base
{
protected:
    gobjref_base(GObject* gobj = nullptr) : m_gobj(gobj) {}
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

    GObject* gobj() const { return m_gobj; }
    operator GObject*() const { return m_gobj; }
    operator GTypeInstance*() const { return reinterpret_cast<GTypeInstance*>(m_gobj); }
    operator gpointer() const { return m_gobj; }

    operator bool() const { return m_gobj != nullptr; }
    bool operator!() const { return m_gobj == nullptr; }

private:
    template<typename Object>
    friend class gobjptr_impl;

    void    set_gobj    (GObject* gobj) { m_gobj = gobj; }

private:
    GObject* m_gobj;
};

template<>
class gobjref<GObject>; // : public gobjref_base

#define MOO_DEFINE_GOBJREF_METHODS_COMMON(Object, Super)                        \
    using super = Super;                                                        \
    using object_type = Object;                                                 \
    using parent_object_type = typename super::object_type;                     \
    using ptrtype = gobjptr<object_type>;                                       \
                                                                                \
    gobjref(object_type* gobj = nullptr)                                        \
        : super(reinterpret_cast<parent_object_type*>(gobj))                    \
    {                                                                           \
    }                                                                           \
                                                                                \
    object_type* gobj() const                                                   \
    {                                                                           \
        const auto* p = static_cast<const gobjref_base*>(this);                 \
        return reinterpret_cast<object_type*>(p->gobj());                       \
    }                                                                           \
                                                                                \
    operator object_type*() const { return gobj(); }                            \
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

#define MOO_DEFINE_GOBJREF_METHODS(Object, Super)                               \
    MOO_DEFINE_GOBJREF_METHODS_COMMON(Object, Super)

// Generic implementation, falls back to the parent type's gobjref implementation
// if that's known, or to GObject's one, coming from the generic gobjinfo.
template<typename Object>
class gobjref : public gobjref<typename gobjinfo<Object>::parent_type>
{
public:
    MOO_DEFINE_GOBJREF_METHODS(Object, gobjref<typename gobjinfo<Object>::parent_type>)
};

template<typename Object>
bool operator==(const gobjref<Object>& p1, const nullptr_t&) { return p1.gobj() == nullptr; }
template<typename Object>
bool operator==(const nullptr_t&, const gobjref<Object>& p2) { return p2.gobj() == nullptr; }
template<typename Object>
bool operator!=(const gobjref<Object>& p1, const nullptr_t&) { return p1.gobj() != nullptr; }
template<typename Object>
bool operator!=(const nullptr_t&, const gobjref<Object>& p2) { return p2.gobj() != nullptr; }

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
        m_ref.set_gobj(nullptr);
        return tmp;
    }

    void reset()
    {
        auto* tmp = get();
        if (tmp)
        {
            m_ref.set_gobj(nullptr);
            g_object_unref(tmp);
        }
    }

    // Implicit conversion to non-const Object* is dangerous because there is a lot
    // of code still which frees/steals objects directly. For example:
    // FooObject* tmp = x->s;
    // x->s = NULL;
    // g_object_unref (tmp);
    operator const Object* () const { return get(); }
    const ref_type* operator->() const { return &m_ref; }
    const ref_type& operator*() const { return m_ref; }

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

    template<typename X>
    bool operator==(X* other) const
    {
        return get() == other;
    }

    template<typename X>
    bool operator==(const gobjptr_impl<X>& other) const
    {
        return get() == other.get();
    }

    template<typename X>
    bool operator!=(const X& anything) const
    {
        return !(*this == anything);
    }

    gobjptr_impl(const gobjptr_impl& other)
        : gobjptr_impl(other.get(), ref_transfer::make_copy)
    {
    }

    gobjptr_impl(gobjptr_impl&& other)
        : gobjptr_impl()
    {
        m_ref.set_gobj(other.get() ? G_OBJECT(other.get()) : nullptr);
        other.m_ref.set_gobj(nullptr);
    }

    gobjptr_impl& operator=(const gobjptr_impl& other)
    {
        assign(other.get(), ref_transfer::make_copy);
        return *this;
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
            other.m_ref.set_gobj(nullptr);
        }

        return *this;
    }

private:
    void assign(Object* obj, ref_transfer policy)
    {
        if (get() != obj)
        {
            Object* tmp = get();
            m_ref.set_gobj(obj ? G_OBJECT(obj) : nullptr);
            if (obj && (policy == ref_transfer::make_copy))
                g_object_ref(obj);
            if (tmp)
                g_object_unref(tmp);
        }
    }

private:
    gobjref<Object> m_ref;
};

template<typename Object>
inline gobjptr<Object> wrap_new(Object *obj)
{
    return gobjptr<Object>::wrap_new(obj);
}

template<typename Object>
inline gobjptr<Object> wrap(Object *obj)
{
    return gobjptr<Object>::wrap(obj);
}

#define MOO_DEFINE_GOBJPTR_METHODS(Object)                                  \
    using ref_type = gobjref<Object>;                                       \
    using impl_type = gobjptr_impl<Object>;                                 \
                                                                            \
    gobjptr() {}                                                            \
                                                                            \
    gobjptr(Object* obj, ref_transfer policy)                               \
        : impl_type(obj, policy)                                            \
    {                                                                       \
    }                                                                       \
                                                                            \
    gobjptr(const gobjptr& other) = default;                                \
    gobjptr& operator=(const gobjptr& other) = default;                     \
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


///////////////////////////////////////////////////////////////////////////////////////////
//
// GObject
//

template<>
class gobjref<GObject> : public gobjref_base
{
public:
    MOO_DEFINE_GOBJREF_METHODS_COMMON(GObject, gobjref_base)

    gulong  signal_connect          (const char *detailed_signal, GCallback c_handler, void *data) const;
    gulong  signal_connect_swapped  (const char *detailed_signal, GCallback c_handler, void *data) const;
    
    void    set_data                (const char* key, gpointer value) const;
    
    void    set                     (const gchar *first_prop, ...) const G_GNUC_NULL_TERMINATED;
    void    set_property            (const gchar *property_name, const GValue *value) const;
    
    void    freeze_notify           () const;
    void    thaw_notify             () const;
};

template<>
class gobjptr<GObject> : public gobjptr_impl<GObject>
{
    static gobjptr<GObject> wrap_new(GObject*);
};

} // namespace moo
