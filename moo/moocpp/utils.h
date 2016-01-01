/*
 *   moocpp/utils.h
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
#include <mooutils/mooutils-misc.h>
#include <mooglib/moo-glib.h>

namespace moo {

#define MOO_DEFINE_FLAGS(Flags)                                                                                 \
    inline Flags operator | (Flags f1, Flags f2) { return static_cast<Flags>(static_cast<int>(f1) | f2); }      \
    inline Flags operator & (Flags f1, Flags f2) { return static_cast<Flags>(static_cast<int>(f1) & f2); }      \
    inline Flags& operator |= (Flags& f1, Flags f2) { f1 = f1 | f2; return f1; }                                \
    inline Flags& operator &= (Flags& f1, Flags f2) { f1 = f1 & f2; return f1; }                                \
    inline Flags operator ~ (Flags f) { return static_cast<Flags>(~static_cast<int>(f)); }                      \

struct mg_mem_deleter
{
    void operator()(void* mem) { g_free(mem); }
};

template<typename T>
struct mg_slice_deleter
{
    void operator()(T* mem) { g_slice_free (T, mem); }
};

//template<typename T, typename Deleter>
//class mg_mem_holder
//{
//public:
//    // Implicit conversion to and from T* is dangerous because of existing code.
//    explicit mg_mem_holder(T* p = nullptr) : base(p) {}
//
//    // Implicit conversion to T* is dangerous because there is a lot
//    // of code still which frees/steals objects directly. For example:
//    // FooObject* tmp = x->s;
//    // x->s = NULL;
//    // g_object_unref (tmp);
//
//    mg_mem_holder& operator=(const nullptr_t&) { reset(); return *this; }
//
//    template<typename U>
//    void take(U* p) { if (this->get() != p) reset(p); }
//
//    template<typename U, typename D2>
//    mg_mem_holder& operator=(const mg_mem_holder<U, D2>&) = delete;
//    mg_mem_holder& operator=(const mg_mem_holder&) = delete;
//
//private:
//    std::unique_ptr<T, Deleter> m_p;
//};
//
//template<typename T>
//using mg_mem_ptr = mg_mem_holder<T, mg_mem_deleter>;
//
//template<typename T>
//using mg_slice_ptr = mg_mem_holder<T, mg_slice_deleter<T>>;

class mg_str
{
public:
    mg_str() : mg_str(nullptr) {}
    explicit mg_str(const char* s) : m_p(s ? g_strdup(s) : nullptr) {}

    void copy(const char* s) { take(s ? g_strdup(s) : nullptr); }
    void take(char* s) { if (s != m_p.get()) m_p.reset(s); }

    operator const char* () const { return m_p.get(); }
    char* get_mutable() const { return m_p.get(); }

    char* get_copy() const { return m_p ? g_strdup(m_p.get()) : nullptr; }

    mg_str& operator=(const nullptr_t&) { m_p.reset(); return *this; }

    mg_str(const mg_str&) = delete;
    mg_str& operator=(const mg_str&) = delete;
    mg_str(mg_str&& s) : m_p(std::move(s.m_p)) {}
    mg_str& operator=(mg_str&& s) { m_p = std::move(s.m_p); }

private:
    std::unique_ptr<char, mg_mem_deleter> m_p;
};

inline bool operator==(const mg_str& s1, const char* s2) { return moo_str_equal(s1, s2); }
inline bool operator==(const char* s1, const mg_str& s2) { return moo_str_equal(s1, s2); }
inline bool operator==(const mg_str& s1, const mg_str& s2) { return moo_str_equal(s1, s2); }
inline bool operator!=(const mg_str& s1, const mg_str& s2) { return !(s1 == s2); }
template<typename T>
inline bool operator!=(const mg_str& s1, const T& s2) { return !(s1 == s2); }
template<typename T>
inline bool operator!=(const T& s1, const mg_str& s2) { return !(s1 == s2); }

using mg_ustr = mg_str;

template<typename T, typename U>
auto find(const std::vector<T>& vec, const U& elm) -> decltype(vec.begin())
{
    return std::find(vec.begin(), vec.end(), elm);
}

template<typename T, typename U>
auto find(std::vector<T>& vec, const U& elm) -> decltype(vec.begin())
{
    return std::find(vec.begin(), vec.end(), elm);
}

template<typename T, typename U>
bool contains(const std::vector<T>& vec, const U& elm)
{
    return find(vec, elm) != vec.end();
}

template<typename T, typename U>
void remove(std::vector<T>& vec, const U& elm)
{
    auto itr = find(vec, elm);
    g_assert (itr != vec.end());
    vec.erase(itr);
}

} // namespace moo
