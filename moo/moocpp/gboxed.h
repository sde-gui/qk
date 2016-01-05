/*
 *   moocpp/gboxed.h
 *
 *   Copyright (C) 2004-2016 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
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

#include <memory>
#include <utility>
#include <mooutils/mootype-macros.h>

namespace moo {

template<typename T>
class gboxed_helper
{
public:
    static T* array_elm_copy(T* obj)
    {
        return new T(*obj);
    }

    static gpointer copy(gpointer p)
    {
        return array_elm_copy(reinterpret_cast<T*>(p));
    }

    static void array_elm_free(T* obj)
    {
        delete obj;
    }

    static void free(gpointer p)
    {
        array_elm_free(reinterpret_cast<T*>(p));
    }
};

#define MOO_DEFINE_BOXED_CPP_TYPE(Object, object)       \
    MOO_DEFINE_BOXED_TYPE(Object, object,               \
                          gboxed_helper<Object>::copy,  \
                          gboxed_helper<Object>::free)

} // namespace moo
