/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   mooplugin.h
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

#ifndef __MOO_PLUGIN_H__
#define __MOO_PLUGIN_H__

#include "mooedit/mooeditor.h"
#include "mooedit/moobigpaned.h"
#include "mooutils/mooprefsdialog.h"

G_BEGIN_DECLS

#define MOO_PLUGIN_PREFS_ROOT  "Plugins"
#define MOO_PLUGIN_CURRENT_VERSION 5


typedef struct _MooPluginInfo           MooPluginInfo;
typedef struct _MooPluginParams         MooPluginParams;
typedef struct _MooPluginPrefsParams    MooPluginPrefsParams;


typedef gboolean    (*MooPluginInitFunc)            (gpointer        plugin_data,
                                                     MooPluginInfo  *info);
typedef void        (*MooPluginDeinitFunc)          (gpointer        plugin_data,
                                                     MooPluginInfo  *info);

typedef void        (*MooPluginWindowAttachFunc)    (MooEditWindow  *window,
                                                     gpointer        plugin_data,
                                                     MooPluginInfo  *info);
typedef void        (*MooPluginWindowDetachFunc)    (MooEditWindow  *window,
                                                     gpointer        plugin_data,
                                                     MooPluginInfo  *info);

typedef gboolean    (*MooPluginModuleInitFunc)      (void);


/* keep plugin_new() in sync */
struct _MooPluginInfo
{
    guint plugin_system_version;

    const char *id;

    const char *name;
    const char *description;
    const char *author;
    const char *version;

    MooPluginInitFunc init;
    MooPluginDeinitFunc deinit;
    MooPluginWindowAttachFunc attach;
    MooPluginWindowDetachFunc detach;

    MooPluginParams *params;
    MooPluginPrefsParams *prefs_params;
};


/* keep plugin_params_copy() in sync */
struct _MooPluginParams
{
    gboolean enabled;
};



/* keep plugin_prefs_params_copy() in sync */
struct _MooPluginPrefsParams
{
};


gboolean        moo_plugin_register         (MooPluginInfo  *info,
                                             gpointer        plugin_data);
void            moo_plugin_unregister       (const char     *plugin_id);

MooPluginInfo  *moo_plugin_lookup           (const char     *plugin_id);
/* returns list of strings which should be freed */
GSList         *moo_list_plugins            (void);

gpointer        moo_plugin_get_data         (const char     *plugin_id);

gpointer        moo_plugin_get_window_data  (const char     *plugin_id,
                                             MooEditWindow  *window);
void            moo_plugin_set_window_data  (const char     *plugin_id,
                                             MooEditWindow  *window,
                                             gpointer        data,
                                             GDestroyNotify  free_func);

// void            moo_plugin_attach_prefs     (MooPrefsDialog *dialog);

void            moo_plugin_read_dir         (const char     *dir);


#ifdef MOOEDIT_COMPILATION
void            _moo_plugin_window_attach   (MooEditWindow  *window);
void            _moo_plugin_window_detach   (MooEditWindow  *window);
#endif


G_END_DECLS

#endif /* __MOO_PLUGIN_H__ */
