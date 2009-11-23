/*
 *   moocpp-macros.h
 *
 *   Copyright (C) 2004-2009 by Yevgen Muntyan <muntyan@tamu.edu>
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

#ifndef MOO_CPP_MACROS_H
#define MOO_CPP_MACROS_H

#include <mooutils/mooutils-macros.h>

#define mAssert _MOO_DEBUG_ASSERT
#define mCheck  _MOO_RELEASE_ASSERT

#define mStaticAssert MOO_STATIC_ASSERT
mStaticAssert(sizeof(char) == 1, "test");

namespace moo {
namespace impl {

template<typename FromClass, typename ToClass>
struct _mCanCast
{
    static void check()
    {
        FromClass *p = 0;
        ToClass &q = *p;
        (void) q;
    }
};

} // namespace impl
} // namespace moo

#define mCanCast(FromClass, ToClass) moo::impl::_mCanCast<FromClass, ToClass>::check()

#define MOO_DISABLE_COPY_AND_ASSIGN(Class)                      \
private:                                                        \
    Class(const Class&);                                        \
    Class &operator=(const Class&);

#define MOO_IMPLEMENT_POINTER(Class, get_ptr_expr)              \
    operator Class*() const { return get_ptr_expr; }            \
    Class &operator*() const { return *(get_ptr_expr); }        \
    Class *operator->() const { return get_ptr_expr; }

#define MOO_IMPLEMENT_POINTER_TO_MEM(Class, get_ptr_expr)       \
    operator Class*() { return get_ptr_expr; }                  \
    operator const Class*() const { return get_ptr_expr; }      \
    Class &operator*() { return *(get_ptr_expr); }              \
    const Class &operator*() const { return *(get_ptr_expr); }  \
    Class *operator->() { return get_ptr_expr; }                \
    const Class *operator->() const { return get_ptr_expr; }

#define MOO_IMPLEMENT_BOOL(get_bool_expr)                       \
    operator bool() const { return get_bool_expr; }             \
    bool operator !() const { return !(get_bool_expr); }

#endif /* MOO_CPP_MACROS_H */
/* -%- strip:true -%- */
