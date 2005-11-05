/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   moofind.c
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

#include "mooedit/mooplugin-macro.h"

#define PYTHON_PLUGIN_ID "python"


gboolean _moo_python_plugin_init (void);


typedef struct {
    MooPlugin parent;
} PythonPlugin;


static gboolean
python_plugin_init (G_GNUC_UNUSED PythonPlugin *plugin)
{
    return TRUE;
}


static void
python_plugin_deinit (G_GNUC_UNUSED PythonPlugin *plugin)
{
}


static void
python_plugin_attach_win (G_GNUC_UNUSED PythonPlugin   *plugin,
                          G_GNUC_UNUSED MooEditWindow  *window)
{
}

static void
python_plugin_detach_win (G_GNUC_UNUSED PythonPlugin   *plugin,
                          G_GNUC_UNUSED MooEditWindow  *window)
{
}


static void
python_plugin_attach_doc (G_GNUC_UNUSED PythonPlugin   *plugin,
                          G_GNUC_UNUSED MooEdit        *doc,
                          G_GNUC_UNUSED MooEditWindow  *window)
{
}

static void
python_plugin_detach_doc (G_GNUC_UNUSED PythonPlugin   *plugin,
                          G_GNUC_UNUSED MooEdit        *doc,
                          G_GNUC_UNUSED MooEditWindow  *window)
{
}


MOO_PLUGIN_DEFINE_INFO (python, PYTHON_PLUGIN_ID,
                        "Python", "A snake",
                        "Yevgen Muntyan <muntyan@tamu.edu>",
                        MOO_VERSION);
MOO_PLUGIN_DEFINE_FULL (Python, python,
                        python_plugin_init, python_plugin_deinit,
                        python_plugin_attach_win, python_plugin_detach_win,
                        python_plugin_attach_doc, python_plugin_detach_doc,
                        NULL, 0, 0);


gboolean
_moo_python_plugin_init (void)
{
    python_plugin_params.visible = FALSE;
    return moo_plugin_register (python_plugin_get_type ());
}
