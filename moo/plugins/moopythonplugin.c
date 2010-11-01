/*
 *   moopythonplugin.c
 *
 *   Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@sourceforge.net>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "mooeditplugins.h"
#include "mooedit/mooplugin-macro.h"
#include "mooedit/mooplugin-loader.h"
#include "mooutils/mooi18n.h"
#include "mooutils/mooutils-messages.h"
#include "mooutils/moolist.h"
#include "mooscript/python/moopython.h"

#define MOO_PYTHON_PLUGIN_ID "MooPython"

typedef struct {
    MooPlugin parent;
} MooPythonPlugin;

static gboolean
moo_python_plugin_init (G_GNUC_UNUSED MooPythonPlugin *plugin)
{
    if (!moo_python_init ())
        return FALSE;

    return TRUE;
}

static void
moo_python_plugin_deinit (G_GNUC_UNUSED MooPythonPlugin *plugin)
{
    moo_python_deinit ();
}


static void
load_python_module (const char *module_file,
                    G_GNUC_UNUSED const char *ini_file,
                    G_GNUC_UNUSED gpointer data)
{
    if (moo_plugin_lookup (MOO_PYTHON_PLUGIN_ID))
        moo_python_run_file (module_file);
}


MOO_PLUGIN_DEFINE_INFO (moo_python,
                        N_("Python"), N_("Python support"),
                        "Yevgen Muntyan <" MOO_EMAIL ">",
                        MOO_VERSION, NULL)
MOO_PLUGIN_DEFINE (MooPython, moo_python,
                   NULL, NULL, NULL, NULL, NULL,
                   0, 0)


gboolean
_moo_python_plugin_init (void)
{
    MooPluginLoader loader = { load_python_module, NULL, NULL };
    moo_plugin_loader_register (&loader, "Python");
    MooPluginParams params = { TRUE, TRUE };
    return moo_plugin_register (MOO_PYTHON_PLUGIN_ID,
                                moo_python_plugin_get_type (),
                                &moo_python_plugin_info,
                                &params);
}
