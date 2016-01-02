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
#include <list>
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

template<typename Container, typename U>
auto find(const Container& cont, const U& elm) -> decltype(cont.begin())
{
    return std::find(cont.begin(), cont.end(), elm);
}

template<typename Container, typename U>
auto find(Container& cont, const U& elm) -> decltype(cont.begin())
{
    return std::find(cont.begin(), cont.end(), elm);
}

template<typename Container, typename UnPr>
auto find_if(const Container& cont, const UnPr& pred) -> decltype(cont.begin())
{
    return std::find_if(cont.begin(), cont.end(), pred);
}

template<typename Container, typename U>
bool contains(const Container& vec, const U& elm)
{
    return find(vec, elm) != vec.end();
}

template<typename Container, typename UnPr>
bool any_of(const Container& cont, const UnPr& pred)
{
    return std::any_of(cont.begin(), cont.end(), pred);
}

template<typename Container, typename U>
void remove(Container& vec, const U& elm)
{
    auto itr = find(vec, elm);
    g_assert (itr != vec.end());
    vec.erase(itr);
}

} // namespace moo
