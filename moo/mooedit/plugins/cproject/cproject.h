/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   cproject.h
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

#ifndef __C_PROJECT_H__
#define __C_PROJECT_H__

#include "mooedit/mooplugin.h"
#include "mooedit/moocmdview.h"
#include "cproject-project.h"

G_BEGIN_DECLS

#define CPROJECT_PLUGIN_ID   "CProject"
#define CPROJECT_PLUGIN_TYPE (cproject_plugin_get_type ())

typedef struct _CProjectPlugin CProjectPlugin;


struct _CProjectPlugin
{
    MooPlugin parent;

    Project *project;
    MooEditWindow *window;
    GtkWidget *build_configuration_menu;
    MooRecentMgr *recent_mgr;
    MooCmdView *output;

    char *running;
};


GType       cproject_plugin_get_type    (void);

gboolean    cproject_plugin_init        (CProjectPlugin *plugin);
void        cproject_plugin_deinit      (CProjectPlugin *plugin);
void        cproject_plugin_attach      (CProjectPlugin *plugin,
                                         MooEditWindow  *window);
void        cproject_plugin_detach      (CProjectPlugin *plugin,
                                         MooEditWindow  *window);

void        cproject_load_prefs         (CProjectPlugin *plugin);
void        cproject_save_prefs         (CProjectPlugin *plugin);
void        cproject_update_project_ui  (CProjectPlugin *plugin);
void        cproject_update_file_ui     (CProjectPlugin *plugin);

gboolean    cproject_close_project      (CProjectPlugin *plugin);
void        cproject_new_project        (CProjectPlugin *plugin);
void        cproject_open_project       (CProjectPlugin *plugin,
                                         const char     *path);
void        cproject_project_options    (CProjectPlugin *plugin);
void        cproject_build_project      (CProjectPlugin *plugin);
void        cproject_compile_file       (CProjectPlugin *plugin);
void        cproject_execute            (CProjectPlugin *plugin);


G_END_DECLS

#endif /* __C_PROJECT_H__ */
