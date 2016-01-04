/*
 *   moocpp/strutils.h
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

#include <algorithm>
#include <memory>
#include <vector>
#include <utility>
#include <mooglib/moo-glib.h>

namespace moo {

enum class mem_transfer
{
    take_ownership,
    borrow,
    make_copy
};

template<typename Buf, typename MemHandler, typename Self>
class mg_mem_holder
{
public:
    enum class memory_type
    {
        literal,
        allocated,
    };

    mg_mem_holder() : m_p(nullptr), m_ot(ownership_type::literal) {}
    mg_mem_holder(const nullptr_t&) : mg_mem_holder() {}

    ~mg_mem_holder()
    {
        reset();
    }

    mg_mem_holder(const mg_mem_holder& other) = delete;
    mg_mem_holder& operator=(const mg_mem_holder& other) = delete;

    mg_mem_holder(mg_mem_holder&& other)
        : mg_mem_holder()
    {
        *this = std::move(other);
    }

    mg_mem_holder& operator=(mg_mem_holder&& other)
    {
        swap(*this, other);
        return *this;
    }

    mg_mem_holder(const Buf* p, memory_type mt, mem_transfer tt)
        : mg_mem_holder()
    {
        assign(p, mt, tt);
    }

    void assign(const Buf* p, memory_type mt, mem_transfer tt)
    {
        // Make sure old data is freed only after we are done, to handle self-assignment and self-mutation
        mg_mem_holder tmp;
        swap(*this, tmp);

        if (p == nullptr)
            return;

        switch (tt)
        {
        case mem_transfer::take_ownership:
            {
                switch (mt)
                {
                case memory_type::literal:
                    m_p = const_cast<Buf*>(p);
                    m_ot = ownership_type::literal;
                    break;
                case memory_type::allocated:
                    m_p = const_cast<Buf*>(p);
                    m_ot = ownership_type::owned;
                    break;
                default:
                    g_assert_not_reached();
                    break;
                }
            }
            break;

        case mem_transfer::borrow:
            {
                switch (mt)
                {
                case memory_type::literal:
                    m_p = const_cast<Buf*>(p);
                    m_ot = ownership_type::literal;
                    break;
                case memory_type::allocated:
                    m_p = const_cast<Buf*>(p);
                    m_ot = ownership_type::borrowed;
                    break;
                default:
                    g_assert_not_reached();
                    break;
                }
            }
            break;

        case mem_transfer::make_copy:
            {
                switch (mt)
                {
                case memory_type::literal:
                    m_p = const_cast<Buf*>(p);
                    m_ot = ownership_type::literal;
                    break;
                case memory_type::allocated:
                    m_p = MemHandler::dup(p);
                    m_ot = ownership_type::owned;
                    break;
                default:
                    g_assert_not_reached();
                    break;
                }
            }
            break;

        default:
            g_assert_not_reached();
            break;
        }
    }

    void reset()
    {
        if (m_p != nullptr && m_ot == ownership_type::owned)
        {
            MemHandler::free(m_p);
        }
    }

    mg_mem_holder& operator=(const nullptr_t&)
    {
        reset();
        return *this;
    }

    operator const Buf* () const { return m_p; }
    const Buf* get() const { return m_p; }

    Buf* get_mutable()
    {
        ensure_owned();
        return m_p;
    }

    bool is_null() const { return m_p == nullptr; }

    Self& ensure_not_borrowed()
    {
        if (m_p != nullptr && m_ot == ownership_type::borrowed)
            assign(m_p, memory_type::allocated, mem_transfer::make_copy);
        return static_cast<Self&>(*this);
    }

    void ensure_owned()
    {
        if (m_p != nullptr && m_ot != ownership_type::owned)
            assign(m_p, memory_type::allocated, mem_transfer::make_copy);
    }

    Buf* release_owned()
    {
        ensure_owned();
        Buf* ret = m_p;
        m_p = nullptr;
        m_ot = ownership_type::literal;
        return ret;
    }

    Self borrow() const
    {
        return make_borrowed(*this);
    }

    void borrow(const Buf* p)
    {
        assign(p, memory_type::allocated, mem_transfer::borrow);
    }

    // Borrowing a temporary is not a good idea
    void borrow(Buf*&& p) = delete;

    void borrow(const mg_mem_holder& other)
    {
        if (other.m_ot == ownership_type::literal)
            literal(static_cast<const Buf*>(other));
        else
            borrow(static_cast<const Buf*>(other));
    }

    void literal(const Buf* p)
    {
        assign(p, memory_type::literal, mem_transfer::borrow);
    }

    Self copy() const
    {
        return make_copy(*this);
    }

    void copy(const Buf* p)
    {
        assign(p, memory_type::allocated, mem_transfer::make_copy);
    }

    void copy(const mg_mem_holder& other)
    {
        if (other.m_ot == ownership_type::literal)
            literal(static_cast<const Buf*>(other));
        else
            copy(static_cast<const Buf*>(other));
    }

    void take(Buf* p)
    {
        assign(p, memory_type::allocated, mem_transfer::take_ownership);
    }

    void take(mg_mem_holder&& other)
    {
        *this = std::move(other);
    }

    template<typename Arg>
    static Self make_borrowed(Self&& arg) = delete;

    static Self make_borrowed(Buf*&& p) = delete;

    template<typename Arg>
    static Self make_borrowed(const Arg& arg)
    {
        Self s;
        s.borrow(arg);
        return std::move(s);
    }

    template<typename Arg>
    static Self wrap_literal(Arg&& arg)
    {
        Self s;
        s.literal(std::forward<Arg>(arg));
        return std::move(s);
    }

    template<typename Arg>
    static Self make_copy(Arg&& arg)
    {
        Self s;
        s.copy(std::forward<Arg>(arg));
        return std::move(s);
    }

    template<typename Arg>
    static Self wrap_new(Arg&& arg)
    {
        Self s;
        s.take(std::forward<Arg>(arg));
        return std::move(s);
    }

    static void swap(mg_mem_holder& p1, mg_mem_holder& p2)
    {
        std::swap(p1.m_p, p2.m_p);
        std::swap(p1.m_ot, p2.m_ot);
    }

private:
    enum class ownership_type
    {
        borrowed,
        owned,
        literal,
    };

    Buf*           m_p;
    ownership_type m_ot;
};

template<typename Buf, typename MemHandler, typename Self>
inline void swap(mg_mem_holder<Buf, MemHandler, Self>& p1, mg_mem_holder<Buf, MemHandler, Self>& p2)
{
    mg_mem_holder<Buf, MemHandler, Self>::swap(p1, p2);
}

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

} // namespace moo
