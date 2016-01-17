/*
 *   moocpp/gobjtypes-gtk.h
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

#include "moocpp/gobjtypes-glib.h"

#define MOO_DEFINE_GTK_TYPE(Object, Parent, obj_g_type)         \
    MOO_DEFINE_GOBJ_TYPE(Gtk##Object, Parent, obj_g_type)       \
    namespace moo {                                             \
    namespace gtk {                                             \
    MOO_GOBJ_TYPEDEFS(Object, Gtk##Object)                      \
    }                                                           \
    }

MOO_DECLARE_CUSTOM_GOBJ_TYPE(GtkTextView);

MOO_DEFINE_GTK_TYPE(Object, GObject, GTK_TYPE_OBJECT)
MOO_DEFINE_GTK_TYPE(Widget, GtkObject, GTK_TYPE_WIDGET)
MOO_DEFINE_GTK_TYPE(Entry, GtkWidget, GTK_TYPE_ENTRY)
MOO_DEFINE_GTK_TYPE(Action, GObject, GTK_TYPE_ACTION)
MOO_DEFINE_GTK_TYPE(TextView, GtkWidget, GTK_TYPE_TEXT_VIEW)
MOO_DEFINE_GTK_TYPE(TextBuffer, GObject, GTK_TYPE_TEXT_BUFFER)
MOO_DEFINE_GTK_TYPE(TextMark, GObject, GTK_TYPE_TEXT_MARK)
MOO_DEFINE_GTK_TYPE(MenuShell, GtkWidget, GTK_TYPE_MENU_SHELL)
MOO_DEFINE_GTK_TYPE(Menu, GtkMenuShell, GTK_TYPE_MENU)

template<>
class moo::gobj_ref<GtkTextView> : public moo::gobj_ref_parent<GtkTextView>
{
public:
    MOO_DEFINE_GOBJREF_METHODS(GtkTextView);

    gtk::TextBuffer get_buffer  ();
};

MOO_REGISTER_CUSTOM_GOBJ_TYPE(GtkTextView);

#endif // __cplusplus
