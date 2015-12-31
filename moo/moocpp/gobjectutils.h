/*
 *   moocpp/gobjectutils.h
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

#include <memory>
#include <utility>

namespace moo {

template<typename T, typename TPriv, typename ...Args>
inline void init_private(TPriv*& p, T* owner, GType owner_type, Args&& ...args)
{
    p = G_TYPE_INSTANCE_GET_PRIVATE(owner, owner_type, TPriv);
    new(p) TPriv(std::forward<Args>(args)...);
}

template<typename TPriv>
inline void finalize_private(TPriv*& p)
{
    if (p != nullptr)
    {
        p->~TPriv();
        p = nullptr;
    }
}

} // namespace moo
