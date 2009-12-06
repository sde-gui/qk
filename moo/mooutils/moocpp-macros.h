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

#include <mooutils/mooutils-messages.h>

#define mooAssert _MOO_DEBUG_ASSERT
#define mooCheck  _MOO_RELEASE_ASSERT

#define mooAssertNotReached _MOO_DEBUG_ASSERT_NOT_REACHED
#define mooCheckNotReached  _MOO_RELEASE_ASSERT_NOT_REACHED

#define mooStaticAssert MOO_STATIC_ASSERT

namespace moo {

template<typename FromClass, typename ToClass>
inline void MOO_FUNC_DEV_MODE checkCanCast()
{
    FromClass *p = 0;
    ToClass &q = *p;
    (void) q;
}

#ifdef MOO_DEV_MODE
#define mooCheckCanCast(FromClass, ToClass) moo::checkCanCast<FromClass, ToClass>()
#else // !DEBUG
#define mooCheckCanCast(FromClass, ToClass) MOO_VOID_STMT
#endif // !DEBUG

#define MOO_DISABLE_COPY_AND_ASSIGN(Class)          \
private:                                            \
    Class(const Class&) MOO_FA_MISSING;             \
    Class &operator=(const Class&) MOO_FA_MISSING;

#ifdef MOO_DEV_MODE

namespace _test {

mooStaticAssert(sizeof(char) == 1, "test");

inline void __moo_test_func()
{
    mooCheck(false);
    mooAssert(false);
    mooAssertNotReached();
    mooCheckNotReached();
}

class Foo1 {
public:
    Foo1() {}

    void meth1() NOTHROW;
    void NOTHROW meth2() {}

    MOO_DISABLE_COPY_AND_ASSIGN(Foo1)
};

inline void NOTHROW Foo1::meth1()
{
}

inline void __moo_test_func()
{
    Foo1 f;
    f.meth1();
}

} // namespace _test

#endif // MOO_DEV_MODE

} // namespace moo

#endif /* MOO_CPP_MACROS_H */
/* -%- strip:true -%- */
