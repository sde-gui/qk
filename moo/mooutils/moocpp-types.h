/*
 *   moocpp-types.h
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

#ifndef MOO_CPP_TYPES_H
#define MOO_CPP_TYPES_H

#include <mooutils/moocpp-macros.h>

namespace moo {

class RefCount
{
public:
    RefCount(int count) : m_count(count) { mCheck(count >= 0); }

    operator int() const;

    void ref();
    bool unref();

private:
    MOO_DISABLE_COPY_AND_ASSIGN(RefCount)

private:
    int m_count;
};

} // namespace moo

#endif /* MOO_CPP_TYPES_H */
/* -%- strip:true -%- */
