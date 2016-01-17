/*
 *   moocpp/gobjptrtypes-gtk.h
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

#ifdef __cplusplus

#include <gtk/gtk.h>
#include <mooglib/moo-glib.h>

#include "moocpp/gobjptrtypes-glib.h"

#define MOO_DEFINE_GTK_TYPE(Object, Parent, obj_g_type)         \
    MOO_DEFINE_GOBJ_TYPE(Gtk##Object, Parent, obj_g_type)       \
    namespace moo {                                             \
    namespace gtk {                                             \
    MOO_GOBJ_TYPEDEFS(Object, Gtk##Object)                      \
    }                                                           \
    }

MOO_DEFINE_GTK_TYPE(Object, GObject, GTK_TYPE_OBJECT)
MOO_DEFINE_GTK_TYPE(Widget, GtkObject, GTK_TYPE_WIDGET)
MOO_DEFINE_GTK_TYPE(TextView, GtkWidget, GTK_TYPE_TEXT_VIEW)
MOO_DEFINE_GTK_TYPE(Entry, GtkWidget, GTK_TYPE_ENTRY)
MOO_DEFINE_GTK_TYPE(Action, GObject, GTK_TYPE_ACTION)

#endif // __cplusplus
