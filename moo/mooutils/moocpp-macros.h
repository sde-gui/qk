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

#define mooAssert _MOO_DEBUG_ASSERT
#define mooCheck  _MOO_RELEASE_ASSERT

#define mooAssertNotReached _MOO_DEBUG_ASSERT_NOT_REACHED
#define mooCheckNotReached  _MOO_RELEASE_ASSERT_NOT_REACHED

#define mooStaticAssert MOO_STATIC_ASSERT

#ifdef DEBUG
mooStaticAssert(sizeof(char) == 1, "test");
inline void _moo_dummy_test_func()
{
    mooCheck(false);
    mooAssert(false);
    mooAssertNotReached();
    mooCheckNotReached();
}
#endif

namespace moo {

template<typename FromClass, typename ToClass>
inline void checkCanCast()
{
    FromClass *p = 0;
    ToClass &q = *p;
    (void) q;
}

#ifdef DEBUG
#define mooCheckCanCast(FromClass, ToClass) moo::checkCanCast<FromClass, ToClass>()
#else // !DEBUG
#define mooCheckCanCast(FromClass, ToClass) MOO_VOID_STMT
#endif // !DEBUG

} // namespace moo

#define MOO_DISABLE_COPY_AND_ASSIGN(Class)      \
private:                                        \
    Class(const Class&);                        \
    Class &operator=(const Class&);

#define MOO_DO_ONCE_BEGIN                       \
do {                                            \
    static bool _moo_beenHere = false;          \
    if (!_moo_beenHere)                         \
    {                                           \
        _moo_beenHere = true;

#define MOO_DO_ONCE_END                         \
    }                                           \
} while (0);

#endif /* MOO_CPP_MACROS_H */
/* -%- strip:true -%- */
