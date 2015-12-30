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

namespace moo {

#define MOO_INITIALIZE_PRIVATE(_priv_, _owner_, _owner_g_type_, _priv_type_)    \
    (_priv_) = G_TYPE_INSTANCE_GET_PRIVATE ((_owner_), (_owner_g_type_), _priv_type_); \
    new(_priv_) (_priv_type_)

#define MOO_FINALIZE_PRIVATE(_priv_, _priv_type_) \
    _priv_->~_priv_type_()

} // namespace moo
