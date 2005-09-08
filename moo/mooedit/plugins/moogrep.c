/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   moogrep.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef MOO_VERSION
#define MOO_VERSION NULL
#endif

#include "mooedit/mooplugin.h"
#include "mooedit/plugins/mooeditplugins.h"

#define GREP_PLUGIN_ID "grep"


typedef struct {
    GtkWidget *dialog;
    GtkWidget *notebook;
} WindowStuff;

static WindowStuff *window_stuff_new        (void);
static void         window_stuff_free       (WindowStuff    *stuff);

static void         grep_plugin_attach      (MooEditWindow  *window);
static void         grep_plugin_detach      (MooEditWindow  *window);


static void
grep_plugin_attach (MooEditWindow *window)
{
    moo_plugin_set_window_data (GREP_PLUGIN_ID, window,
                                window_stuff_new (),
                                (GDestroyNotify) window_stuff_free);
}


static void
grep_plugin_detach (MooEditWindow *window)
{
    WindowStuff *stuff;

    stuff = moo_plugin_get_window_data (GREP_PLUGIN_ID, window);

    if (stuff->dialog)
        gtk_widget_destroy (stuff->dialog);
}


static WindowStuff*
window_stuff_new (void)
{
    return g_new0 (WindowStuff, 1);
}


static void
window_stuff_free (WindowStuff *stuff)
{
    g_free (stuff);
}


gboolean
moo_grep_init (void)
{
    MooPluginParams params = { TRUE };
    MooPluginPrefsParams prefs_params;

    MooPluginInfo info = {
        MOO_PLUGIN_CURRENT_VERSION,

        GREP_PLUGIN_ID,
        GREP_PLUGIN_ID,
        GREP_PLUGIN_ID,
        "Yevgen Muntyan <muntyan@tamu.edu>",
        MOO_VERSION,

        NULL, /* (MooEditPluginInitFunc) grep_plugin_init, */
        NULL, /* (MooEditPluginDeinitFunc) grep_plugin_deinit, */
        (MooPluginWindowAttachFunc) grep_plugin_attach,
        (MooPluginWindowDetachFunc) grep_plugin_detach,

        &params,
        &prefs_params
    };

    return moo_plugin_register (&info, NULL);
}
