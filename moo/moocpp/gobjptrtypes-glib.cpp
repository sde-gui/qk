/*
 *   moocpp/gobjptrtypes-glib.cpp
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

#include "moocpp/gobjptrtypes-glib.h"

using namespace moo;

const gobjptr<GObject>& gobjref_base::self() const
{
    return gobjptr<GObject>::from_gobjref(static_cast<const gobjref<GObject>&>(*this));
}

GObject* gobjref_base::g() const
{
    return self().get();
}

gulong gobjref_base::signal_connect(const char *detailed_signal, GCallback c_handler, void *data) const
{
    return g_signal_connect(g(), detailed_signal, c_handler, data);
}

gulong gobjref_base::signal_connect_swapped(const char *detailed_signal, GCallback c_handler, void *data) const
{
    return g_signal_connect_swapped(g(), detailed_signal, c_handler, data);
}

void gobjref_base::set_data(const char* key, gpointer value) const
{
    g_object_set_data(g(), key, value);
}

void gobjref_base::set(const gchar *first_prop, ...) const
{
    va_list args;
    va_start(args, first_prop);
    g_object_set_valist(g(), first_prop, args);
    va_end(args);
}

void gobjref_base::set_property(const gchar *property_name, const GValue *value) const
{
    g_object_set_property(g(), property_name, value);
}
