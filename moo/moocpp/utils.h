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

template<typename T, typename Deleter>
struct mg_mem_holder : public std::unique_ptr<T, Deleter>
{
    typedef std::unique_ptr<T, Deleter> base;

    mg_mem_holder(T* p = nullptr) : base(p) {}

    operator T*() const { return this->get(); }

    mg_mem_holder& operator=(const nullptr_t&) { reset(); return *this; }

    //template<typename U>
    //mg_mem_holder& operator=(U* p) { if (this->get() != p) reset(p); return *this; }
    template<typename U>
    void take(U* p) { if (this->get() != p) reset(p); }

    template<typename U, typename D2>
    mg_mem_holder& operator=(const mg_mem_holder<U, D2>&) = delete;
    mg_mem_holder& operator=(const mg_mem_holder&) = delete;
};

template<typename T>
using mg_mem_ptr = mg_mem_holder<T, mg_mem_deleter>;

template<typename T>
using mg_slice_ptr = mg_mem_holder<T, mg_slice_deleter<T>>;

using mg_char_buf = mg_mem_ptr<char>;

struct mg_str : public mg_char_buf
{
    mg_str(const char* s = nullptr) : mg_char_buf(s ? g_strdup(s) : nullptr) {}

    void copy(const char* s) { take(s ? g_strdup(s) : nullptr); }

    mg_str& operator=(const nullptr_t&) { reset(); return *this; }

    mg_str& operator=(const mg_str&) = delete;
};

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

// TODO: remove these

template<typename T, typename D>
inline void g_free(const moo::mg_mem_holder<T, D>&)
{
    static_assert(false, "g_free applied to a mg_mem_holder");
}

template<typename T, typename D>
inline void g_slice_free1(gsize, const moo::mg_mem_holder<T, D>&)
{
    static_assert(false, "g_slice_free applied to a mg_mem_holder");
}

inline void glib_g_free(gpointer p) { g_free(p); }
