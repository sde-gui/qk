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

template<typename T>
using mg_mem_ptr = std::unique_ptr<T, mg_mem_deleter>;

template<typename T>
using mg_slice_ptr = std::unique_ptr<T, mg_slice_deleter<T>>;

using mg_str_holder = mg_mem_ptr<char>;

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
