/*
 *   mooactionbase-private.h
 *
 *   Copyright (C) 2004-2006 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef __MOO_ACTION_BASE_PRIVATE_H__
#define __MOO_ACTION_BASE_PRIVATE_H__

#include <mooutils/mooactionbase.h>

G_BEGIN_DECLS


#define MOO_ACTION_BASE_PROPS(prefix)               \
    prefix##_PROP_0,                                \
    prefix##_PROP_DISPLAY_NAME,                     \
    prefix##_PROP_ACCEL,                            \
    prefix##_PROP_CONNECT_ACCEL,                    \
    prefix##_PROP_NO_ACCEL,                         \
    prefix##_PROP_ACCEL_EDITABLE,                   \
    prefix##_PROP_FORCE_ACCEL_LABEL,                \
    prefix##_PROP_DEAD,                             \
    prefix##_PROP_ACTIVE,                           \
    prefix##_PROP_HAS_SUBMENU,                      \
    /* these are overridden GtkAction properties */ \
    prefix##_PROP_LABEL,                            \
    prefix##_PROP_TOOLTIP


#define MOO_ACTION_BASE_SET_GET_PROPERTY(prefix,func)   \
    case prefix##_PROP_DISPLAY_NAME:                    \
    case prefix##_PROP_ACCEL:                           \
    case prefix##_PROP_CONNECT_ACCEL:                   \
    case prefix##_PROP_NO_ACCEL:                        \
    case prefix##_PROP_ACCEL_EDITABLE:                  \
    case prefix##_PROP_FORCE_ACCEL_LABEL:               \
    case prefix##_PROP_DEAD:                            \
    case prefix##_PROP_ACTIVE:                          \
    case prefix##_PROP_HAS_SUBMENU:                     \
    case prefix##_PROP_LABEL:                           \
    case prefix##_PROP_TOOLTIP:                         \
        func (object, property_id, value, pspec);       \
        break

#define MOO_ACTION_BASE_SET_PROPERTY(prefix) MOO_ACTION_BASE_SET_GET_PROPERTY(prefix, _moo_action_base_set_property)
#define MOO_ACTION_BASE_GET_PROPERTY(prefix) MOO_ACTION_BASE_SET_GET_PROPERTY(prefix, _moo_action_base_get_property)


void        _moo_action_base_init_class     (GObjectClass   *klass);
void        _moo_action_base_set_property   (GObject        *object,
                                             guint           property_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec);
void        _moo_action_base_get_property   (GObject        *object,
                                             guint           property_id,
                                             GValue         *value,
                                             GParamSpec     *pspec);


G_END_DECLS

#endif /* __MOO_ACTION_BASE_PRIVATE_H__ */
