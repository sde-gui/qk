/*
 *   moocpp-exc.h
 *
 *   Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@sourceforge.net>
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

#ifndef MOO_CPP_EXC_H
#define MOO_CPP_EXC_H

#include "moocpp-macros.h"

namespace moo {

class Exception
{
protected:
    NOTHROW Exception(const char *what, const MooCodeLoc *loc)
        : m_what(what ? what : "")
        , m_loc(loc ? *loc : moo_default_code_loc ())
    {
    }

    virtual NOTHROW ~Exception()
    {
    }

public:
    const char * NOTHROW what() const { return m_what; }

private:
    const char *m_what;
    MooCodeLoc m_loc;
};

class ExcUnexpected : public Exception
{
protected:
    NOTHROW ExcUnexpected(const char *msg, const MooCodeLoc &loc)
        : Exception(msg, &loc)
    {
    }

    virtual NOTHROW ~ExcUnexpected()
    {
    }

public:
    NORETURN static void raise(const char *msg, const MooCodeLoc &loc)
    {
#ifdef DEBUG
        _moo_assert_message(loc, msg);
#endif
        throw ExcUnexpected(msg, loc);
    }
};

} // namespace moo

#define mooThrowIfFalse(cond)                                   \
do {                                                            \
    if (cond)                                                   \
        ;                                                       \
    else                                                        \
        moo::ExcUnexpected::raise("condition failed: " #cond,   \
                                  MOO_CODE_LOC);                \
} while(0)

#define mooThrowIfReached()                                     \
do {                                                            \
    moo::ExcUnexpected::raise("should not be reached",          \
                              MOO_CODE_LOC);                    \
} while(0)

#define MOO_BEGIN_NO_EXCEPTIONS                                 \
try {

#define MOO_END_NO_EXCEPTIONS                                   \
} catch (...) {                                                 \
    mooCheckNotReached();                                       \
}

#endif /* MOO_CPP_EXC_H */
/* -%- strip:true -%- */
