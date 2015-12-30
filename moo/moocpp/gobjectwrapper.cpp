/*
 *   moocpp/gobjectwrapper.cpp
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

#include "moocpp/gobjectwrapper.h"

using namespace moo;

GObjectWrapper::GObjectWrapper(GObject* obj)
    : GObjectWrapper(obj, true)
{
}

GObjectWrapper::GObjectWrapper(GObject* obj, bool takeReference)
    : m_gobj(obj)
    , m_ownReference(takeReference)
{
    if (takeReference)
        g_object_ref(obj);
}

GObjectWrapper::~GObjectWrapper()
{
    if (m_ownReference)
        g_object_unref(m_gobj);
}
