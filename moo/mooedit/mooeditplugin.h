/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   mooeditplugin.h
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

#ifndef __MOO_EDIT_PLUGIN_H__
#define __MOO_EDIT_PLUGIN_H__

#include "mooedit/mooeditor.h"
#include "mooedit/moobigpaned.h"
#include "mooutils/mooprefsdialog.h"

G_BEGIN_DECLS

#define MOO_EDIT_PLUGIN_PREFS_ROOT  "Plugins"
#define MOO_EDIT_PLUGIN_CURRENT_VERSION 1


typedef struct _MooEditPluginInfo           MooEditPluginInfo;
typedef struct _MooEditPluginWindowData     MooEditPluginWindowData;
typedef struct _MooEditPluginParams         MooEditPluginParams;
typedef struct _MooEditPluginPrefsParams    MooEditPluginPrefsParams;


typedef gboolean    (*MooEditPluginInitFunc)            (MooEditPluginInfo          *info,
                                                         gpointer                    plugin_data);
typedef void        (*MooEditPluginDeinitFunc)          (MooEditPluginInfo          *info,
                                                         gpointer                    plugin_data);

typedef void        (*MooEditPluginWindowAttachFunc)    (MooEditPluginInfo          *info,
                                                         MooEditPluginWindowData    *plugin_window_data,
                                                         gpointer                    plugin_data);
typedef void        (*MooEditPluginWindowDetachFunc)    (MooEditPluginInfo          *info,
                                                         MooEditPluginWindowData    *plugin_window_data,
                                                         gpointer                    plugin_data);

typedef gboolean    (*MooEditPluginPaneCreateFunc)      (MooEditPluginInfo          *info,
                                                         MooEditPluginWindowData    *plugin_window_data,
                                                         MooPaneLabel              **label,
                                                         GtkWidget                 **widget,
                                                         gpointer                    plugin_data);
typedef void        (*MooEditPluginPaneDestroyFunc)     (MooEditPluginInfo          *info,
                                                         MooEditPluginWindowData    *plugin_window_data,
                                                         gpointer                    plugin_data);

typedef GtkWidget*  (*MooEditPluginPrefsPageCreateFunc) (MooEditPluginInfo          *info,
                                                         gpointer                    plugin_data);


typedef gboolean    (*MooEditPluginModuleInitFunc)      (void);


/* keep plugin_new() in sync */
struct _MooEditPluginInfo
{
    guint plugin_system_version;

    const char *id;
    const char *name;
    const char *description;

    MooEditPluginInitFunc init;
    MooEditPluginDeinitFunc deinit;
    MooEditPluginWindowAttachFunc attach;
    MooEditPluginWindowDetachFunc detach;
    MooEditPluginPaneCreateFunc pane_create;
    MooEditPluginPaneDestroyFunc pane_destroy;

    MooEditPluginParams *params;
    MooEditPluginPrefsParams *prefs_params;
};


struct _MooEditPluginWindowData
{
    MooEditWindow *window;
    gpointer data;
};


struct _MooEditPluginParams
{
    gboolean enabled;
    gboolean want_pane;
    MooPanePosition pane_position;
};


struct _MooEditPluginPrefsParams
{
    MooEditPluginPrefsPageCreateFunc prefs_page_create;
};


gboolean                 moo_edit_plugin_register           (MooEditPluginInfo  *info,
                                                             gpointer            plugin_data);
void                     moo_edit_plugin_unregister         (const char         *id);

MooEditPluginInfo       *moo_edit_plugin_lookup             (const char         *id);
/* returns list of strings which should be freed */
GSList                  *moo_edit_list_plugins              (void);

MooEditPluginWindowData *moo_edit_plugin_get_window_data    (MooEditWindow      *window,
                                                             const char         *id);

void                     moo_edit_plugin_attach_prefs       (MooPrefsDialog     *dialog);

void                     moo_edit_plugin_read_dir           (const char         *dir);
gboolean                 moo_edit_plugin_read_module        (GModule            *module,
                                                             const char         *name);


G_END_DECLS

#endif /* __MOO_EDIT_PLUGIN_H__ */
