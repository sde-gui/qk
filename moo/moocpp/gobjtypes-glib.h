/*
 *   moocpp/gobjtypes-glib.h
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

#ifdef __cplusplus

#include "moocpp/gobjptr.h"
#include "moocpp/gobjectutils.h"

#include <stdarg.h>

#define MOO_GOBJ_TYPEDEFS(CppObject, CObject)                               \
    using CppObject             = ::moo::gobj_ref<CObject>;                 \
    using CppObject##Ptr        = ::moo::gobj_ptr<CObject>;                 \
    using CppObject##RawPtr     = ::moo::gobj_raw_ptr<CObject>;             \

#define MOO_GIFACE_TYPEDEFS(CppObject, CObject)                             \
    MOO_GOBJ_TYPEDEFS(CppObject, CObject)

#define MOO_DECLARE_CUSTOM_GOBJ_TYPE(CObject)                               \
namespace moo {                                                             \
template<> class gobj_ref<CObject>;                                         \
}

MOO_DEFINE_FLAGS(GSignalFlags);
MOO_DEFINE_FLAGS(GConnectFlags);
MOO_DEFINE_FLAGS(GSpawnFlags);
MOO_DEFINE_FLAGS(GLogLevelFlags);
MOO_DEFINE_FLAGS(GRegexCompileFlags);
MOO_DEFINE_FLAGS(GIOCondition);

namespace moo {

///////////////////////////////////////////////////////////////////////////////////////////
//
// GObject
//

template<>
class gobj_ref<GObject> : public gobj_ref_base
{
public:
    MOO_DEFINE_GOBJREF_METHODS_IMPL(GObject, gobj_ref_base)

    gulong  connect                             (const char* detailed_signal, GCallback c_handler, void* data);
    gulong  connect_swapped                     (const char* detailed_signal, GCallback c_handler, void* data);

    void    signal_emit_by_name                 (const char* detailed_signal, ...);
    void    signal_emit                         (guint signal_id, GQuark detail, ...);

    bool    signal_has_handler_pending          (guint signal_id, GQuark detail, bool may_be_blocked);
    gulong  signal_connect_closure_by_id        (guint signal_id, GQuark detail, GClosure* closure, bool after);
    gulong  signal_connect_closure              (const char* detailed_signal, GClosure* closure, bool after);
    gulong  signal_connect_data                 (const char* detailed_signal, GCallback c_handler, gpointer data, GClosureNotify destroy_data, GConnectFlags connect_flags);
    void    signal_handler_block                (gulong handler_id);
    void    signal_handler_unblock              (gulong handler_id);
    void    signal_handler_disconnect           (gulong handler_id);
    bool    signal_handler_is_connected         (gulong handler_id);
    gulong  signal_handler_find                 (GSignalMatchType mask, guint signal_id, GQuark detail, GClosure* closure, gpointer func, gpointer data);
    guint   signal_handlers_block_matched       (GSignalMatchType mask, guint signal_id, GQuark detail, GClosure* closure, gpointer func, gpointer data);
    guint   signal_handlers_unblock_matched     (GSignalMatchType mask, guint signal_id, GQuark detail, GClosure* closure, gpointer func, gpointer data);
    guint   signal_handlers_disconnect_matched  (GSignalMatchType mask, guint signal_id, GQuark detail, GClosure* closure, gpointer func, gpointer data);
    guint   signal_handlers_disconnect_by_func  (GCallback c_handler, void* data);

    void*   get_data                (const char* key);
    void*   get_data                (GQuark q);
    void    set_data                (const char* key, gpointer value, GDestroyNotify destroy = nullptr);
    void    set_data                (GQuark q, gpointer value, GDestroyNotify destroy = nullptr);

    void    set_property            (const char* property_name, const GValue* value);

    template<typename T>
    void set (const char* prop, const T& value)
    {
        g_object_set (gobj (), prop, cpp_vararg_value_fixer<T>::apply (value), nullptr);
    }

    template<typename T, typename... Args>
    void set (const char* prop, const T& value, Args&&... args)
    {
        set (prop, value);
        set (std::forward<Args> (args)...);
    }

    void get (const char* prop, bool& dest)
    {
        gboolean val;
        g_object_get (gobj (), prop, &val, nullptr);
        dest = val;
    }

    template<typename T>
    void get (const char* prop, T&& dest)
    {
        g_object_get (gobj (), prop, cpp_vararg_dest_fixer<T>::apply (std::forward<T> (dest)), nullptr);
    }

    template<typename T, typename... Args>
    void get (const char* prop, T&& dest, Args&&... args)
    {
        get (prop, std::forward<T> (dest));
        get (std::forward<Args> (args)...);
    }

    void    notify                  (const char* property_name);
    void    freeze_notify           ();
    void    thaw_notify             ();
};

namespace g {

MOO_GOBJ_TYPEDEFS(Object, GObject);

} // namespace g

} // namespace moo

#endif // __cplusplus
