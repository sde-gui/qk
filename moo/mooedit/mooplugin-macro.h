/*
 *   mooplugin-macro.h
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

#ifndef __MOO_PLUGIN_MACRO_H__
#define __MOO_PLUGIN_MACRO_H__

#include <mooedit/mooplugin.h>


#define MOO_PLUGIN_DEFINE_INFO(plugin_name__,id__,name__,                   \
                               description__,author__,version__,            \
                               langs__)                                     \
                                                                            \
static struct {                                                             \
    const char *id;                                                         \
    const char *name;                                                       \
    const char *description;                                                \
    const char *author;                                                     \
    const char *version;                                                    \
    const char *langs;                                                      \
} plugin_name__##_plugin_info = {id__, name__, description__,               \
                                 author__, version__, langs__};


#define MOO_PLUGIN_DEFINE_FULL(Name__,name__,                               \
                               init__,deinit__,                             \
                               attach_win__,detach_win__,                   \
                               attach_doc__,detach_doc__,                   \
                               prefs_page_func__,                           \
                               WIN_PLUGIN_TYPE__,DOC_PLUGIN_TYPE__)         \
                                                                            \
static gpointer name__##_plugin_parent_class;                               \
                                                                            \
typedef struct {                                                            \
    MooPluginClass parent_class;                                            \
} Name__##PluginClass;                                                      \
                                                                            \
static void                                                                 \
name__##_plugin_class_init (MooPluginClass *klass)                          \
{                                                                           \
    name__##_plugin_parent_class = g_type_class_peek_parent (klass);        \
                                                                            \
    /* klass->plugin_system_version = MOO_PLUGIN_CURRENT_VERSION; */        \
                                                                            \
    klass->init = (MooPluginInitFunc) init__;                               \
    klass->deinit = (MooPluginDeinitFunc) deinit__;                         \
    klass->attach_win = (MooPluginAttachWinFunc) attach_win__;              \
    klass->detach_win = (MooPluginDetachWinFunc) detach_win__;              \
    klass->attach_doc = (MooPluginAttachDocFunc) attach_doc__;              \
    klass->detach_doc = (MooPluginDetachDocFunc) detach_doc__;              \
    klass->create_prefs_page = (MooPluginPrefsPageFunc) prefs_page_func__;  \
}                                                                           \
                                                                            \
static void                                                                 \
name__##_plugin_instance_init (MooPlugin *plugin)                           \
{                                                                           \
    plugin->info =                                                          \
        moo_plugin_info_new (name__##_plugin_info.id,                       \
                             name__##_plugin_info.name,                     \
                             name__##_plugin_info.description,              \
                             name__##_plugin_info.author,                   \
                             name__##_plugin_info.version,                  \
                             name__##_plugin_info.langs,                    \
                             TRUE, TRUE);                                   \
                                                                            \
    plugin->win_plugin_type = WIN_PLUGIN_TYPE__;                            \
    plugin->doc_plugin_type = DOC_PLUGIN_TYPE__;                            \
}                                                                           \
                                                                            \
static GType name__##_plugin_get_type (void) G_GNUC_CONST;                  \
                                                                            \
static GType                                                                \
name__##_plugin_get_type (void)                                             \
{                                                                           \
    static GType type__ = 0;                                                \
                                                                            \
    if (G_UNLIKELY (type__ == 0))                                           \
    {                                                                       \
        static const GTypeInfo info__ = {                                   \
            sizeof (Name__##PluginClass),                                   \
            (GBaseInitFunc) NULL,                                           \
            (GBaseFinalizeFunc) NULL,                                       \
            (GClassInitFunc) name__##_plugin_class_init,                    \
            (GClassFinalizeFunc) NULL,                                      \
            NULL,   /* class_data */                                        \
            sizeof (Name__##Plugin),                                        \
            0,      /* n_preallocs */                                       \
            (GInstanceInitFunc) name__##_plugin_instance_init,              \
            NULL    /* value_table */                                       \
        };                                                                  \
                                                                            \
        type__ = g_type_register_static (MOO_TYPE_PLUGIN,                   \
                                         #Name__ "Plugin", &info__, 0);     \
    }                                                                       \
                                                                            \
    return type__;                                                          \
}


#define MOO_WIN_PLUGIN_DEFINE(Name__,name__,create__,destroy__)             \
                                                                            \
typedef struct {                                                            \
    MooWinPluginClass parent_class;                                         \
} Name__##WindowPluginClass;                                                \
                                                                            \
static GType name__##_window_plugin_get_type (void) G_GNUC_CONST;           \
                                                                            \
static gpointer name__##_window_plugin_parent_class = NULL;                 \
                                                                            \
static void                                                                 \
name__##_window_plugin_class_init (MooWinPluginClass *klass)                \
{                                                                           \
    name__##_window_plugin_parent_class = g_type_class_peek_parent (klass); \
    klass->create = (MooWinPluginCreateFunc) create__;                      \
    klass->destroy = (MooWinPluginDestroyFunc) destroy__;                   \
}                                                                           \
                                                                            \
static GType                                                                \
name__##_window_plugin_get_type (void)                                      \
{                                                                           \
    static GType type__ = 0;                                                \
                                                                            \
    if (G_UNLIKELY (type__ == 0))                                           \
    {                                                                       \
        static const GTypeInfo info__ = {                                   \
            sizeof (Name__##WindowPluginClass),                             \
            (GBaseInitFunc) NULL,                                           \
            (GBaseFinalizeFunc) NULL,                                       \
            (GClassInitFunc) name__##_window_plugin_class_init,             \
            (GClassFinalizeFunc) NULL,                                      \
            NULL,   /* class_data */                                        \
            sizeof (Name__##WindowPlugin),                                  \
            0,      /* n_preallocs */                                       \
            NULL,                                                           \
            NULL    /* value_table */                                       \
        };                                                                  \
                                                                            \
        type__ = g_type_register_static (MOO_TYPE_WIN_PLUGIN,               \
                                         #Name__ "WindowPlugin",&info__, 0);\
    }                                                                       \
                                                                            \
    return type__;                                                          \
}


#define MOO_DOC_PLUGIN_DEFINE(Name__,name__,create__,destroy__)             \
                                                                            \
typedef struct {                                                            \
    MooDocPluginClass parent_class;                                         \
} Name__##DocPluginClass;                                                   \
                                                                            \
static GType name__##_doc_plugin_get_type (void) G_GNUC_CONST;              \
                                                                            \
static gpointer name__##_doc_plugin_parent_class = NULL;                    \
                                                                            \
static void                                                                 \
name__##_doc_plugin_class_init (MooDocPluginClass *klass)                   \
{                                                                           \
    name__##_doc_plugin_parent_class = g_type_class_peek_parent (klass);    \
    klass->create = (MooDocPluginCreateFunc) create__;                      \
    klass->destroy = (MooDocPluginDestroyFunc) destroy__;                   \
}                                                                           \
                                                                            \
static GType                                                                \
name__##_doc_plugin_get_type (void)                                         \
{                                                                           \
    static GType type__ = 0;                                                \
                                                                            \
    if (G_UNLIKELY (type__ == 0))                                           \
    {                                                                       \
        static const GTypeInfo info__ = {                                   \
            sizeof (Name__##DocPluginClass),                                \
            (GBaseInitFunc) NULL,                                           \
            (GBaseFinalizeFunc) NULL,                                       \
            (GClassInitFunc) name__##_doc_plugin_class_init,                \
            (GClassFinalizeFunc) NULL,                                      \
            NULL,   /* class_data */                                        \
            sizeof (Name__##DocPlugin),                                     \
            0,      /* n_preallocs */                                       \
            NULL,                                                           \
            NULL    /* value_table */                                       \
        };                                                                  \
                                                                            \
        type__ = g_type_register_static (MOO_TYPE_DOC_PLUGIN,               \
                                         #Name__ "DocPlugin", &info__, 0);  \
    }                                                                       \
                                                                            \
    return type__;                                                          \
}


#endif /* __MOO_PLUGIN_MACRO_H__ */
