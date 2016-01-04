/*
 *   moocpp/strutils.cpp
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

#include "moocpp/strutils.h"
#include <string.h>

using namespace moo;

MOO_DEFINE_STANDARD_PTR_METHODS(gstr, super)

const gstr gstr::null;

//static void compile_errors(const gstr& constref, gstr& ref, gstr val)
//{
//    //ref.borrow(g_strdup("zzz"));
//    //gstr::make_borrowed(g_strdup("zzz"));
//    gstr::make_borrowed(ref);
//    gstr::make_borrowed(constref);
//
//    if (constref)
//        return;
//
//    if (ref)
//        return;
//
//    if (val)
//        return;
//}

static bool str_equal(const char* s1, const char* s2)
{
    if (!s1 || !*s1)
        return !s2 || !*s2;
    else if (!s2)
        return false;
    else
        return strcmp(s1, s2) == 0;
}

bool moo::operator==(const gstr& s1, const char* s2)
{
    return str_equal(s1, s2);
}

bool moo::operator==(const char* s1, const gstr& s2)
{
    return str_equal(s1, s2);
}

bool moo::operator==(const gstr& s1, const gstr& s2)
{
    return str_equal(s1, s2);
}

bool moo::operator!=(const gstr& s1, const gstr& s2)
{
    return !(s1 == s2);
}

bool moo::operator!=(const gstr& s1, const char* s2)
{
    return !(s1 == s2);
}

bool moo::operator!=(const char* s1, const gstr& s2)
{
    return !(s1 == s2);
}
