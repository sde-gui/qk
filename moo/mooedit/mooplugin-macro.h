/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooplugin-macro.h
 *
 *   Copyright (C) 2004-2005 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef __MOO_PLUGIN_MACRO_H__
#define __MOO_PLUGIN_MACRO_H__

#include "mooedit/mooplugin.h"


#define MOO_PLUGIN_DEFINE_PARAMS(info,enabled__,id__,name__,description__,author__,version__) \
static MooPluginParams params__ = {                 \
    enabled__                                       \
};                                                  \
                                                    \
static MooPluginPrefsParams prefs_params__ = {};    \
                                                    \
static MooPluginInfo info = {                       \
    id__,                                           \
    name__,                                         \
    description__,                                  \
    author__,                                       \
    version__,                                      \
    &params__,                                      \
    &prefs_params__                                 \
};


#define MOO_PLUGIN_DEFINE(PluginName,plugin_name,init__,deinit__,attach__,detach__,prefs__,info__,WINDOW_PLUGIN_TYPE) \
                                                                    \
static gpointer plugin_name##_parent_class;                         \
                                                                    \
typedef struct {                                                    \
    MooPluginClass parent_class;                                    \
} PluginName##Class;                                                \
                                                                    \
static void                                                         \
plugin_name##_class_init (MooPluginClass *klass)                    \
{                                                                   \
    plugin_name##_parent_class = g_type_class_peek_parent (klass);  \
                                                                    \
    klass->plugin_system_version = MOO_PLUGIN_CURRENT_VERSION;      \
                                                                    \
    klass->init = (MooPluginInitFunc) init__;                       \
    klass->deinit = (MooPluginDeinitFunc) deinit__;                 \
    klass->attach = (MooPluginAttachFunc) attach__;                 \
    klass->detach = (MooPluginDetachFunc) detach__;                 \
    klass->create_prefs_page = (MooPluginPrefsPageFunc) prefs__;    \
}                                                                   \
                                                                    \
static void                                                         \
plugin_name##_instance_init (MooPlugin *plugin)                     \
{                                                                   \
    plugin->info = &info__;                                         \
    plugin->window_plugin_type = WINDOW_PLUGIN_TYPE;                \
}                                                                   \
                                                                    \
GType plugin_name##_get_type (void) G_GNUC_CONST;                   \
                                                                    \
GType                                                               \
plugin_name##_get_type (void)                                       \
{                                                                   \
    static GType type__ = 0;                                        \
                                                                    \
    if (G_UNLIKELY (type__ == 0))                                   \
    {                                                               \
        static const GTypeInfo info__ = {                           \
            sizeof (PluginName##Class),                             \
            (GBaseInitFunc) NULL,                                   \
            (GBaseFinalizeFunc) NULL,                               \
            (GClassInitFunc) plugin_name##_class_init,              \
            (GClassFinalizeFunc) NULL,                              \
            NULL,   /* class_data */                                \
            sizeof (PluginName),                                    \
            0,      /* n_preallocs */                               \
            (GInstanceInitFunc) plugin_name##_instance_init,        \
            NULL    /* value_table */                               \
        };                                                          \
                                                                    \
        type__ = g_type_register_static (MOO_TYPE_PLUGIN,           \
                                         #PluginName, &info__, 0);  \
    }                                                               \
                                                                    \
    return type__;                                                  \
}


#define MOO_WINDOW_PLUGIN_DEFINE(WindowPluginName,window_plugin_name,create__,destroy__) \
                                                                            \
typedef struct {                                                            \
    MooWindowPluginClass parent_class;                                      \
} WindowPluginName##Class;                                                  \
                                                                            \
GType window_plugin_name##_get_type (void) G_GNUC_CONST;                    \
                                                                            \
static gpointer window_plugin_name##_parent_class = NULL;                   \
                                                                            \
static void                                                                 \
window_plugin_name##_class_init (MooWindowPluginClass *klass)               \
{                                                                           \
    window_plugin_name##_parent_class = g_type_class_peek_parent (klass);   \
    klass->create = (MooWindowPluginCreateFunc) create__;                   \
    klass->destroy = (MooWindowPluginDestroyFunc) destroy__;                \
}                                                                           \
                                                                            \
GType                                                                       \
window_plugin_name##_get_type (void)                                        \
{                                                                           \
    static GType type__ = 0;                                                \
                                                                            \
    if (G_UNLIKELY (type__ == 0))                                           \
    {                                                                       \
        static const GTypeInfo info__ = {                                   \
            sizeof (WindowPluginName##Class),                               \
            (GBaseInitFunc) NULL,                                           \
            (GBaseFinalizeFunc) NULL,                                       \
            (GClassInitFunc) window_plugin_name##_class_init,               \
            (GClassFinalizeFunc) NULL,                                      \
            NULL,   /* class_data */                                        \
            sizeof (WindowPluginName),                                      \
            0,      /* n_preallocs */                                       \
            NULL,                                                           \
            NULL    /* value_table */                                       \
        };                                                                  \
                                                                            \
        type__ = g_type_register_static (MOO_TYPE_WINDOW_PLUGIN,            \
                                         #WindowPluginName, &info__, 0);    \
    }                                                                       \
                                                                            \
    return type__;                                                          \
}


#endif /* __MOO_PLUGIN_MACRO_H__ */
