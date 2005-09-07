/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   cproject.c
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

#include <gmodule.h>
#include "mooedit/mooeditplugin.h"
#include "cproject.h"

static gboolean cproject_plugin_init (void)
{
    return TRUE;
}


G_MODULE_EXPORT gboolean
cproject_init (void)
{
    MooEditPluginParams params = {
        TRUE, FALSE, 0
    };

    MooEditPluginPrefsParams prefs_params = {
        NULL /* (MooEditPluginPrefsPageCreateFunc) */
    };

    MooEditPluginInfo info = {
        MOO_EDIT_PLUGIN_CURRENT_VERSION,
        "CProject",
        "CProject",
        "CProject",
        (MooEditPluginInitFunc) cproject_plugin_init,
        NULL, /* MooEditPluginDeinitFunc */
        NULL, /* MooEditPluginWindowAttachFunc */
        NULL, /* MooEditPluginWindowDetachFunc */
        NULL, /* MooEditPluginPaneCreateFunc */
        NULL, /* MooEditPluginPaneDestroyFunc */
        &params,
        &prefs_params
    };

    return moo_edit_plugin_register (&info, NULL);
}
