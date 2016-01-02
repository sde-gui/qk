/*
 *   moocpp/gobjptrtypes-glib.h
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
#include <mooglib/moo-glib.h>

#include "moocpp/gobjptr.h"

#include <stdarg.h>

namespace moo {

///////////////////////////////////////////////////////////////////////////////////////////
//
//
//

struct gobjref_base
{
    gulong      signal_connect          (const char *detailed_signal, GCallback c_handler, void *data) const;
    gulong      signal_connect_swapped  (const char *detailed_signal, GCallback c_handler, void *data) const;

    void        set_data                (const char* key, gpointer value) const;

    void        set                     (const gchar *first_prop, ...) G_GNUC_NULL_TERMINATED const;
    void        set_property            (const gchar *property_name, const GValue *value) const;

    GObject*                g           () const;
    const gobjptr<GObject>& self        () const;
};

template<typename GObjClass>
struct gobjref : public gobjref_base
{
};

///////////////////////////////////////////////////////////////////////////////////////////
//
//
//

struct mg_gobjptr_methods_base
{
    GObject*                g_object() const { return g(); }

protected:
    const gobjptr<GObject>& self() const;//    { return static_cast<const gobjptr<GObject>&>(*this); }
    GObject*                g() const;//       { return self().get(); }
};

template<typename GObjClass>
struct mg_gobjptr_methods : public mg_gobjptr_methods_base
{
};

} // namespace moo
