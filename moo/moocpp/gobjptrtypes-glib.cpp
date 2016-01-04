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
using namespace g;

gulong Object::signal_connect(const char *detailed_signal, GCallback c_handler, void *data)
{
    return g_signal_connect(gobj(), detailed_signal, c_handler, data);
}

gulong Object::signal_connect_swapped(const char *detailed_signal, GCallback c_handler, void *data)
{
    return g_signal_connect_swapped(gobj(), detailed_signal, c_handler, data);
}

void Object::signal_emit_by_name(const char* detailed_signal, ...)
{
    guint signal_id;
    GQuark detail;
    g_return_if_fail(g_signal_parse_name(detailed_signal,
                                         G_OBJECT_TYPE(gobj()),
                                         &signal_id, &detail,
                                         true));

    va_list args;
    va_start(args, detailed_signal);
    g_signal_emit_valist(gobj(), signal_id, detail, args);
    va_end(args);
}

void Object::signal_emit(guint signal_id, GQuark detail, ...)
{
    va_list args;
    va_start(args, detail);
    g_signal_emit_valist(gobj(), signal_id, detail, args);
    va_end(args);
}

void Object::set_data(const char* key, gpointer value)
{
    g_object_set_data(gobj(), key, value);
}

void Object::set(const gchar *first_prop, ...)
{
    va_list args;
    va_start(args, first_prop);
    g_object_set_valist(gobj(), first_prop, args);
    va_end(args);
}

void Object::set_property(const gchar *property_name, const GValue *value)
{
    g_object_set_property(gobj(), property_name, value);
}

void Object::notify(const char* property_name)
{
    g_object_notify(gobj(), property_name);
}

void Object::freeze_notify()
{
    g_object_freeze_notify(gobj());
}

void Object::thaw_notify()
{
    g_object_thaw_notify(gobj());
}
