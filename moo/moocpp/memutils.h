/*
 *   moocpp/memutils.h
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

#ifndef __cplusplus
#error "This is a C++ file"
#endif

#include <algorithm>
#include <memory>
#include <vector>
#include <utility>
#include <moocpp/utils.h>
#include <mooglib/moo-glib.h>

void extern_g_free(gpointer);
void extern_g_object_unref(gpointer);
void extern_g_strfreev(char**);

namespace moo {

enum class mem_transfer
{
    take_ownership,
    borrow,
    make_copy
};

template<typename T>
class gbuf
{
public:
    explicit gbuf(T* p = nullptr) : m_p(p) {}
    ~gbuf() { ::g_free(m_p); }

    void set_new(T* p) { if (m_p != p) { ::g_free(m_p); m_p = p; } }
    operator const T*() const { return m_p; }
    T* get() const { return m_p; }
    T*& _get() { return m_p; }
    T* operator->() const { return m_p; }

    operator T*() const = delete;
    T** operator&() = delete;

    T* release() { T* p = m_p; m_p = nullptr; return p; }

    MOO_DISABLE_COPY_OPS(gbuf);

    gbuf(gbuf&& other) : gbuf() { std::swap(m_p, other.m_p); }
    gbuf& operator=(gbuf&& other) { std::swap(m_p, other.m_p); return *this; }

    operator bool() const { return m_p != nullptr; }
    bool operator !() const { return m_p == nullptr; }

private:
    T* m_p;
};

#define MOO_DEFINE_STANDARD_PTR_METHODS_INLINE(Self, Super)                     \
    Self() : Super() {}                                                         \
    Self(const nullptr_t&) : Super(nullptr) {}                                  \
    Self(const Self& other) = delete;                                           \
    Self(Self&& other) : Super(std::move(other)) {}                             \
                                                                                \
    Self& operator=(const Self& other) = delete;                                \
                                                                                \
    Self& operator=(Self&& other)                                               \
    {                                                                           \
        static_cast<Super&>(*this) = std::move(static_cast<Super&&>(other));    \
        return *this;                                                           \
    }                                                                           \
                                                                                \
    Self& operator=(const nullptr_t&)                                           \
    {                                                                           \
        static_cast<Super&>(*this) = nullptr;                                   \
        return *this;                                                           \
    }

#define MOO_DECLARE_STANDARD_PTR_METHODS(Self, Super)                           \
    Self();                                                                     \
    Self(const nullptr_t&);                                                     \
    Self(const Self& other) = delete;                                           \
    Self(Self&& other);                                                         \
    Self& operator=(const Self& other) = delete;                                \
    Self& operator=(Self&& other);                                              \
    Self& operator=(const nullptr_t&);

#define MOO_DEFINE_STANDARD_PTR_METHODS(Self, Super)                            \
    Self::Self() : Super() {}                                                   \
    Self::Self(const nullptr_t&) : Super(nullptr) {}                            \
    Self::Self(Self&& other) : Super(std::move(other)) {}                       \
                                                                                \
    Self& Self::operator=(Self&& other)                                         \
    {                                                                           \
        static_cast<Super&>(*this) = std::move(static_cast<Super&&>(other));    \
        return *this;                                                           \
    }                                                                           \
                                                                                \
    Self& Self::operator=(const nullptr_t&)                                     \
    {                                                                           \
        static_cast<Super&>(*this) = nullptr;                                   \
        return *this;                                                           \
    }


template<typename T>
class objp
{
public:
    objp (T* p = nullptr) : m_p(p) {}
    ~objp () { delete m_p; }

    template<class... Args>
    static objp make (Args&&... args)
    {
        return objp (new T (std::forward<Args> (args)...));
    }

    objp (objp&& other) : objp () { *this = std::move (other); }
    objp& operator=(objp&& other) { std::swap (m_p, other.m_p); return *this; }

    T* get() const { return m_p; }
    T** pp () { g_return_val_if_fail (m_p == nullptr, nullptr); return &m_p; }
    T* operator->() const { return m_p; }
    operator const T*() const { return m_p; }

    operator T*() const = delete;
    T** operator&() = delete;

    MOO_DISABLE_COPY_OPS (objp);

    void set (T* p) { if (m_p != p) { delete m_p; m_p = p; } }
    void reset (T* p = nullptr) { set (p); }
    T* release () { T* p = m_p; m_p = nullptr; return p; }

    bool operator==(const T* p) const { return m_p == p; }
    bool operator!=(const T* p) const { return m_p != p; }

    operator bool () const { return m_p != nullptr; }
    bool operator !() const { return m_p == nullptr; }

private:
    T* m_p;
};


} // namespace moo

template<typename T>
void g_free(const moo::gbuf<T>&) = delete;
template<typename T>
void g_free (const moo::objp<T>&) = delete;
