/*
 *   moocpp/gobjectwrapperclasses.h
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

#include "moocpp/gobjectwrapper.h"

namespace moo {

class Edit : GObjectWrapper
{
public:
    Edit(MooEdit *doc);

    Edit(const Edit&) = delete;
    Edit& operator=(const Edit&) = delete;
};

} // namespace moo
