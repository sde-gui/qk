/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   cproject-plugin.c
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

#include "cproject.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef MOO_VERSION
#define MOO_VERSION NULL
#endif


gboolean    cproject_init           (void);

G_MODULE_EXPORT gboolean
cproject_init (void)
{
    MooPluginParams params = {
        TRUE
    };

    MooPluginPrefsParams prefs_params;

    MooPluginInfo info = {
        MOO_PLUGIN_CURRENT_VERSION,
        CPROJECT_PLUGIN_ID,
        "CProject",
        "CProject",
        "Yevgen Muntyan <muntyan@tamu.edu>",
        MOO_VERSION,
        (MooPluginInitFunc) cproject_plugin_init,
        (MooPluginDeinitFunc) cproject_plugin_deinit,
        (MooPluginWindowAttachFunc) cproject_plugin_attach,
        (MooPluginWindowDetachFunc) cproject_plugin_detach,
        &params,
        &prefs_params
    };

    static CProjectPlugin plugin;

    return moo_plugin_register (&info, &plugin);
}
