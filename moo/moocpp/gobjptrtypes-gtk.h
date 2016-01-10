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

MOO_DEFINE_GOBJ_TYPE(GtkObject, GObject, GTK_TYPE_OBJECT)
MOO_DEFINE_GOBJ_TYPE(GtkWidget, GtkObject, GTK_TYPE_WIDGET)
MOO_DEFINE_GOBJ_TYPE(GtkTextView, GtkWidget, GTK_TYPE_TEXT_VIEW)
MOO_DEFINE_GOBJ_TYPE(GtkEntry, GtkWidget, GTK_TYPE_ENTRY)

namespace moo {

namespace gtk {

MOO_GOBJ_TYPEDEFS(Object, GtkObject);
MOO_GOBJ_TYPEDEFS(Widget, GtkWidget);
MOO_GOBJ_TYPEDEFS(TextView, GtkTextView);

} // namespace g

} // namespace moo

#endif // __cplusplus
