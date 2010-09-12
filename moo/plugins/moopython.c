/*
 *   moopython.c
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
#include "mooutils/mooi18n.h"
#include "mooutils/mooutils-messages.h"
#include "plugins/moopython-py.h"

#define MOO_PYTHON_PLUGIN_ID "MooPython"

typedef void (*Fn_Py_InitializeEx) (int initsigs);
typedef void (*Fn_Py_Finalize) (void);
typedef int  (*Fn_PyRun_SimpleString) (const char *command);

typedef struct {
    Fn_Py_InitializeEx pfn_Py_InitializeEx;
    Fn_Py_Finalize pfn_Py_Finalize;
    Fn_PyRun_SimpleString pfn_PyRun_SimpleString;
} MooPythonAPI;

typedef struct {
    MooPlugin parent;
    GModule *module;
    MooPythonAPI api;
} MooPythonPlugin;

static GModule *
find_python_dll (void)
{
    GModule *module = NULL;
    char *path = g_module_build_path (NULL, "python2.6");
    if (path)
        module = g_module_open (path, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);
    g_free (path);
    return module;
}

static gboolean
moo_python_plugin_init (MooPythonPlugin *plugin)
{
    plugin->module = find_python_dll ();

    if (!plugin->module)
    {
        moo_message ("python module not found");
        return FALSE;
    }

    gpointer p;

    if (g_module_symbol (plugin->module, "Py_InitializeEx", &p))
        plugin->api.pfn_Py_InitializeEx = p;
    if (g_module_symbol (plugin->module, "Py_Finalize", &p))
        plugin->api.pfn_Py_Finalize = p;
    if (g_module_symbol (plugin->module, "PyRun_SimpleString", &p))
        plugin->api.pfn_PyRun_SimpleString = p;

    if (!plugin->api.pfn_Py_InitializeEx)
    {
        moo_message ("Py_InitializeEx not found");
        return FALSE;
    }
    if (!plugin->api.pfn_PyRun_SimpleString)
    {
        moo_message ("PyRun_SimpleString not found");
        return FALSE;
    }
    if (!plugin->api.pfn_Py_Finalize)
    {
        moo_message ("PyRun_SimpleString not found");
        return FALSE;
    }

    plugin->api.pfn_Py_InitializeEx (0);
    if (plugin->api.pfn_PyRun_SimpleString (MOO_PYTHON_PY) != 0)
    {
        plugin->api.pfn_Py_Finalize ();
        moo_message ("error in PyRun_SimpleString");
        return FALSE;
    }

    return TRUE;
}

static void
moo_python_plugin_deinit (MooPythonPlugin *plugin)
{
    if (plugin->api.pfn_Py_Finalize)
        plugin->api.pfn_Py_Finalize ();
    if (plugin->module)
        g_module_close (plugin->module);
    plugin->module = NULL;
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
    MooPluginParams params = { TRUE, TRUE };
    return moo_plugin_register (MOO_PYTHON_PLUGIN_ID,
                                moo_python_plugin_get_type (),
                                &moo_python_plugin_info,
                                &params);
}
