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

//struct mg_mem_deleter
//{
//    void operator()(void* mem) { g_free(mem); }
//};
//
//template<typename T>
//struct mg_slice_deleter
//{
//    void operator()(T* mem) { g_slice_free (T, mem); }
//};

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

//template<typename T, typename U>
//auto find(const std::vector<T>& vec, const U& elm) -> decltype(vec.begin())
//{
//    return std::find(vec.begin(), vec.end(), elm);
//}
//
//template<typename T, typename U>
//auto find(std::vector<T>& vec, const U& elm) -> decltype(vec.begin())
//{
//    return std::find(vec.begin(), vec.end(), elm);
//}

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
