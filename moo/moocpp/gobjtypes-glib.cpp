/*
 *   moocpp/gobjtypes-glib.cpp
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

#include "moocpp/moocpp.h"

using namespace moo;
using namespace g;

void ::extern_g_free(gpointer p)
{
    g_free(p);
}

void ::extern_g_object_unref(gpointer o)
{
    g_object_unref(o);
}

void extern_g_strfreev(char** p)
{
    if (p)
        g_strfreev (p);
}


void moo::init_gobj_system ()
{
}


GQuark gobj_wrapper_base::qdata_key = g_quark_from_static_string ("__moo_gobj_wrapper__");

gobj_wrapper_base& gobj_wrapper_base::get(Object g)
{
    void* o = g.get_data(qdata_key);
    g_assert(o != nullptr);
    return *reinterpret_cast<gobj_wrapper_base*>(o);
}

gobj_wrapper_base::gobj_wrapper_base(gobj_ref<GObject> g)
{
    g.set_data(qdata_key, this, free_qdata);
}

gobj_wrapper_base::~gobj_wrapper_base()
{
}

void gobj_wrapper_base::free_qdata(gpointer d)
{
    gobj_wrapper_base* self = reinterpret_cast<gobj_wrapper_base*>(d);
    delete self;
}


gulong Object::connect(const char *detailed_signal, GCallback c_handler, void *data)
{
    return g_signal_connect(gobj(), detailed_signal, c_handler, data);
}

gulong Object::connect_swapped(const char *detailed_signal, GCallback c_handler, void *data)
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

bool Object::signal_has_handler_pending(guint signal_id, GQuark detail, bool may_be_blocked)
{
    return g_signal_has_handler_pending(gobj(), signal_id, detail, may_be_blocked);
}

gulong Object::signal_connect_closure_by_id(guint signal_id, GQuark detail, GClosure* closure, bool after)
{
    return g_signal_connect_closure_by_id(gobj(), signal_id, detail, closure, after);
}

gulong Object::signal_connect_closure(const char* detailed_signal, GClosure* closure, bool after)
{
    return g_signal_connect_closure(gobj(), detailed_signal, closure, after);
}

gulong Object::signal_connect_data(const char* detailed_signal, GCallback c_handler, gpointer data, GClosureNotify destroy_data, GConnectFlags connect_flags)
{
    return g_signal_connect_data(gobj(), detailed_signal, c_handler, data, destroy_data, connect_flags);
}

void Object::signal_handler_block(gulong handler_id)
{
    g_signal_handler_block(gobj(), handler_id);
}

void Object::signal_handler_unblock(gulong handler_id)
{
    g_signal_handler_unblock(gobj(), handler_id);
}

void Object::signal_handler_disconnect(gulong handler_id)
{
    g_signal_handler_disconnect(gobj(), handler_id);
}

gulong Object::signal_handler_find(GSignalMatchType mask, guint signal_id, GQuark detail, GClosure* closure, gpointer func, gpointer data)
{
    return g_signal_handler_find(gobj(), mask, signal_id, detail, closure, func, data);
}

guint Object::signal_handlers_block_matched(GSignalMatchType mask, guint signal_id, GQuark detail, GClosure* closure, gpointer func, gpointer data)
{
    return g_signal_handlers_block_matched(gobj(), mask, signal_id, detail, closure, func, data);
}

guint Object::signal_handlers_unblock_matched(GSignalMatchType mask, guint signal_id, GQuark detail, GClosure* closure, gpointer func, gpointer data)
{
    return g_signal_handlers_unblock_matched(gobj(), mask, signal_id, detail, closure, func, data);
}

guint Object::signal_handlers_disconnect_matched(GSignalMatchType mask, guint signal_id, GQuark detail, GClosure* closure, gpointer func, gpointer data)
{
    return g_signal_handlers_disconnect_matched(gobj(), mask, signal_id, detail, closure, func, data);
}

void Object::set_data(const char* key, gpointer value, GDestroyNotify destroy)
{
    g_object_set_data(gobj(), key, value);
}

void Object::set_data(GQuark q, gpointer data, GDestroyNotify destroy)
{
    g_object_set_qdata_full(gobj(), q, data, destroy);
}

void* Object::get_data(const char* key)
{
    return g_object_get_data(gobj(), key);
}

void* Object::get_data(GQuark q)
{
    return g_object_get_qdata(gobj(), q);
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
