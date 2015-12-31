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

class GMemDeleter
{
public:
    void operator()(void* mem) { g_free(mem); }
};

template<typename T>
class GSliceDeleter
{
public:
    void operator()(T* mem) { g_slice_free (T, mem); }
};

template<typename T, typename Deleter>
class GMemHolderBase : public std::unique_ptr<T, Deleter>
{
    typedef std::unique_ptr<T, Deleter> unique_ptr;

public:
    GMemHolderBase(T *mem)
        : unique_ptr(mem)
    {
    }

    GMemHolderBase& operator=(T* mem) { if (mem != this->get()) *this = unique_ptr(mem); return *this; }
};

template<typename T>
using GMemHolder = GMemHolderBase<T, GMemDeleter>;

template<typename T>
using GSliceHolder = GMemHolderBase<T, GSliceDeleter<T>>;

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
