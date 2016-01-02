/*
 *   moocpp/gobjptrtypes.h
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

#include <glib-object.h>
#include <gtk/gtk.h>
#include <mooglib/moo-glib.h>

#include "moocpp/gobjptr.h"

namespace moo {

///////////////////////////////////////////////////////////////////////////////////////////
//
// mg_gobj_accessor
//

template<typename Self, typename GObjClas>
struct mg_gobj_accessor
{
protected:
    GObject* g() const { void *o = static_cast<const Self&>(*this).get(); return o ? G_OBJECT(o) : nullptr; }
};

template<typename Self>
struct mg_gobj_accessor<Self, GtkObject> : public mg_gobj_accessor<Self, GObject>
{
};

template<typename Self>
struct mg_gobj_accessor<Self, GtkWidget> : public mg_gobj_accessor<Self, GtkObject>
{
};

///////////////////////////////////////////////////////////////////////////////////////////
//
// mg_gobjptr_methods
//

template<typename Self, typename GObjClas>
struct mg_gobjptr_methods
{
    GObject* g_object() const { return g(); }

protected:
    GObject* g() const { void *o = static_cast<const Self&>(*this).get(); return o ? G_OBJECT(o) : nullptr; }
};

template<typename Self>
struct mg_gobjptr_methods<Self, GtkObject> : public mg_gobjptr_methods<Self, GObject>
{
    GtkObject* gtk_object() const { return g() ? GTK_OBJECT(g()) : nullptr; }
};

template<typename Self>
struct mg_gobjptr_methods<Self, GtkWidget> : public mg_gobjptr_methods<Self, GtkObject>
{
    GtkWidget* gtk_widget() const { return g() ? GTK_WIDGET(g()) : nullptr; }
};

} // namespace moo
