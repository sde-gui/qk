/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooplugin-python.c
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

#include <Python.h>
#define NO_IMPORT_PYGOBJECT
#include "pygobject.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "moopython/mooplugin-python.h"
#include "mooedit/mooplugin-macro.h"
#include "mooutils/mooutils-python.h"

#ifndef MOO_VERSION
#define MOO_VERSION NULL
#endif

#define MOO_PYTHON_PLUGIN_ID "moo-python"

#define MOO_TYPE_PYTHON_PLUGIN      (moo_python_plugin_get_type())
#define MOO_IS_PYTHON_PLUGIN(obj_)  (G_TYPE_CHECK_INSTANCE_TYPE (obj_, MOO_TYPE_PYTHON_PLUGIN))

#define PLUGIN_SUFFIX ".py"

typedef enum {
    HOOK_NEW_WINDOW = 0,
    HOOK_CLOSE_WINDOW,
    HOOK_NEW_DOC,
    HOOK_CLOSE_DOC,
    HOOK_LAST
} HookType;

typedef struct {
    HookType type;
    PyObject *callback;
    PyObject *data;
} Hook;

typedef struct {
    MooPlugin parent;

    GHashTable *hook_ids;
    GSList *hooks[HOOK_LAST];
    int last_id;
} MooPythonPlugin;


static Hook     *hook_new                       (HookType            type,
                                                 PyObject           *callback,
                                                 PyObject           *data);
static void      hook_free                      (Hook               *hook);

static GType     moo_python_plugin_get_type     (void);

static PyObject *moo_python_plugin_add_hook     (MooPythonPlugin    *plugin,
                                                 HookType            type,
                                                 PyObject           *callback,
                                                 PyObject           *data);
static void      moo_python_plugin_remove_hook  (MooPythonPlugin    *plugin,
                                                 int                 id);


static gboolean
moo_python_plugin_init (MooPythonPlugin *plugin)
{
    g_return_val_if_fail (MOO_IS_PYTHON_PLUGIN (plugin), FALSE);
    g_return_val_if_fail (plugin->hook_ids == NULL, FALSE);

    plugin->hook_ids = g_hash_table_new (g_direct_hash, g_direct_equal);

    return TRUE;
}


static void
prepend_id (gpointer id,
            G_GNUC_UNUSED gpointer hook,
            GSList **list)
{
    *list = g_slist_prepend (*list, id);
}

static void
moo_python_plugin_deinit (MooPythonPlugin *plugin)
{
    GSList *ids = NULL, *l;

    g_hash_table_foreach (plugin->hook_ids, (GHFunc) prepend_id, &ids);

    for (l = ids; l != NULL; l = l->next)
        moo_python_plugin_remove_hook (plugin, GPOINTER_TO_INT (l->data));

    g_hash_table_destroy (plugin->hook_ids);
    plugin->hook_ids = NULL;
    g_slist_free (ids);
}


static void
call_hooks (MooPythonPlugin *plugin,
            MooEditWindow   *window,
            MooEdit         *doc,
            HookType         type)
{
    GSList *l;
    PyObject *py_win, *py_doc;

    g_return_if_fail (MOO_IS_PYTHON_PLUGIN (plugin));
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (!doc || MOO_IS_EDIT (doc));
    g_return_if_fail (type < HOOK_LAST);

    py_win = pygobject_new (G_OBJECT (window));
    py_doc = doc ? pygobject_new (G_OBJECT (doc)) : NULL;

    for (l = plugin->hooks[type]; l != NULL; l = l->next)
    {
        PyObject *result;
        Hook *hook = l->data;

        PyTuple_SET_ITEM (hook->data, 0, py_win);
        if (py_doc)
            PyTuple_SET_ITEM (hook->data, 1, py_doc);

        result = PyObject_Call (hook->callback, hook->data, NULL);

        PyTuple_SET_ITEM (hook->data, 0, NULL);
        if (py_doc)
            PyTuple_SET_ITEM (hook->data, 1, NULL);

        if (result)
        {
            Py_DECREF (result);
        }
        else
        {
            PyErr_Print ();
        }
    }

    Py_XDECREF (py_win);
    Py_XDECREF (py_doc);
}


static void
moo_python_plugin_attach_win (MooPythonPlugin *plugin,
                              MooEditWindow   *window)
{
    g_return_if_fail (MOO_IS_PYTHON_PLUGIN (plugin));
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    call_hooks (plugin, window, NULL, HOOK_NEW_WINDOW);
}


static void
moo_python_plugin_detach_win (MooPythonPlugin *plugin,
                              MooEditWindow   *window)
{
    g_return_if_fail (MOO_IS_PYTHON_PLUGIN (plugin));
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    call_hooks (plugin, window, NULL, HOOK_CLOSE_WINDOW);
}


static void
moo_python_plugin_attach_doc (MooPythonPlugin *plugin,
                              MooEdit         *doc,
                              MooEditWindow   *window)
{
    g_return_if_fail (MOO_IS_PYTHON_PLUGIN (plugin));
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDIT (doc));
    call_hooks (plugin, window, doc, HOOK_NEW_DOC);
}


static void
moo_python_plugin_detach_doc (MooPythonPlugin *plugin,
                              MooEdit         *doc,
                              MooEditWindow   *window)
{
    g_return_if_fail (MOO_IS_PYTHON_PLUGIN (plugin));
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDIT (doc));
    call_hooks (plugin, window, doc, HOOK_CLOSE_DOC);
}


static void
moo_python_plugin_read_file (G_GNUC_UNUSED MooPythonPlugin *plugin,
                             const char      *path)
{
    FILE *file;
    PyObject *result, *main_dict, *main_mod;

    g_return_if_fail (path != NULL);

    file = fopen (path, "r");

    if (!file)
    {
        perror ("fopen");
        return;
    }

    main_mod = PyImport_AddModule ((char*) "__main__");
    g_return_if_fail (main_mod != NULL);
    main_dict = PyModule_GetDict (main_mod);

    result = PyRun_File (file, path, Py_file_input, main_dict, main_dict);
    fclose (file);

    if (result)
    {
        Py_XDECREF (result);
    }
    else
    {
        PyErr_Print ();
    }
}


static void
moo_python_plugin_read_dir (MooPythonPlugin *plugin,
                            const char      *path)
{
    GDir *dir;
    const char *name;

    g_return_if_fail (path != NULL);

    dir = g_dir_open (path, 0, NULL);

    if (!dir)
        return;

    while ((name = g_dir_read_name (dir)))
    {
        char *file_path, *prefix, *suffix;

        if (!g_str_has_suffix (name, PLUGIN_SUFFIX))
            continue;

        suffix = g_strrstr (name, PLUGIN_SUFFIX);
        prefix = g_strndup (name, suffix - name);

        file_path = g_build_filename (path, name, NULL);
        moo_python_plugin_read_file (plugin, file_path);

        g_free (prefix);
        g_free (file_path);
    }

    g_dir_close (dir);
}


static void
moo_python_plugin_read_dirs (MooPythonPlugin *plugin,
                             char           **dirs)
{
    for ( ; dirs && *dirs; ++dirs)
        moo_python_plugin_read_dir (plugin, *dirs);
}


MOO_PLUGIN_DEFINE_INFO (moo_python, MOO_PYTHON_PLUGIN_ID,
                        "Python plugin loader", "A snake",
                        "Yevgen Muntyan <muntyan@tamu.edu>",
                        MOO_VERSION);
MOO_PLUGIN_DEFINE_FULL (MooPython, moo_python,
                        moo_python_plugin_init, moo_python_plugin_deinit,
                        moo_python_plugin_attach_win, moo_python_plugin_detach_win,
                        moo_python_plugin_attach_doc, moo_python_plugin_detach_doc,
                        NULL, 0, 0);


gboolean
_moo_python_plugin_init (char **dirs)
{
    gboolean result;

    moo_python_plugin_params.visible = FALSE;
    result = moo_plugin_register (MOO_TYPE_PYTHON_PLUGIN);

    if (result)
    {
        MooPythonPlugin *plugin = moo_plugin_lookup (MOO_PYTHON_PLUGIN_ID);
        g_return_val_if_fail (MOO_IS_PYTHON_PLUGIN (plugin), FALSE);
        moo_python_plugin_read_dirs (plugin, dirs);
    }

    return result;
}


static PyObject*
moo_python_plugin_add_hook (MooPythonPlugin *plugin,
                            HookType         type,
                            PyObject        *callback,
                            PyObject        *data)
{
    int id = ++plugin->last_id;
    Hook *hook = hook_new (type, callback, data);
    plugin->hooks[type] = g_slist_prepend (plugin->hooks[type], hook);
    g_hash_table_insert (plugin->hook_ids, GINT_TO_POINTER (id), hook);
    return_Int (id);
}


PyObject*
_moo_python_plugin_hook (const char *event,
                         PyObject   *callback,
                         PyObject   *data)
{
    MooPythonPlugin *plugin;
    PyObject *result;

    plugin = moo_plugin_lookup (MOO_PYTHON_PLUGIN_ID);
    g_return_val_if_fail (MOO_IS_PYTHON_PLUGIN (plugin), NULL);

    if (!strcmp (event, "new-window"))
        result = moo_python_plugin_add_hook (plugin, HOOK_NEW_WINDOW, callback, data);
    else if (!strcmp (event, "close-window"))
        result = moo_python_plugin_add_hook (plugin, HOOK_CLOSE_WINDOW, callback, data);
    else if (!strcmp (event, "new-doc"))
        result = moo_python_plugin_add_hook (plugin, HOOK_NEW_DOC, callback, data);
    else if (!strcmp (event, "close-doc"))
        result = moo_python_plugin_add_hook (plugin, HOOK_CLOSE_DOC, callback, data);
    else
        return_TypeErr ("invalid event type");

    return result;
}


static Hook*
hook_new (HookType    type,
          PyObject   *callback,
          PyObject   *data)
{
    int data_len, extra, i;
    Hook *hook = g_new0 (Hook, 1);

    hook->type = type;
    hook->callback = callback;
    Py_INCREF (callback);

    data_len = data ? PyTuple_GET_SIZE (data) : 0;

    switch (type)
    {
        case HOOK_NEW_WINDOW:
        case HOOK_CLOSE_WINDOW:
            extra = 1;
            break;
        case HOOK_NEW_DOC:
        case HOOK_CLOSE_DOC:
            extra = 2;
            break;
        case HOOK_LAST:
            g_return_val_if_reached (NULL);
    }

    hook->data = PyTuple_New (data_len + extra);

    for (i = 0; i < data_len; ++i)
    {
        PyTuple_SET_ITEM (hook->data, i + extra,
                          PyTuple_GET_ITEM (data, i));
        Py_INCREF (PyTuple_GET_ITEM (data, i));
    }

    return hook;
}


static void
moo_python_plugin_remove_hook (MooPythonPlugin    *plugin,
                               int                 id)
{
    Hook *hook;

    g_return_if_fail (MOO_IS_PYTHON_PLUGIN (plugin));
    g_return_if_fail (id > 0);

    hook = g_hash_table_lookup (plugin->hook_ids, GINT_TO_POINTER (id));
    g_return_if_fail (hook != NULL);

    plugin->hooks[hook->type] = g_slist_remove (plugin->hooks[hook->type], hook);
    g_hash_table_remove (plugin->hook_ids, GINT_TO_POINTER (id));

    hook_free (hook);
}


static void
hook_free (Hook *hook)
{
    if (hook)
    {
        int i, extra = 0;

        switch (hook->type)
        {
            case HOOK_NEW_WINDOW:
            case HOOK_CLOSE_WINDOW:
                extra = 1;
                break;
            case HOOK_NEW_DOC:
            case HOOK_CLOSE_DOC:
                extra = 2;
                break;
            case HOOK_LAST:
                g_critical ("%s: oops", G_STRLOC);
                extra = 0;
                break;
        }

        for (i = 0; i < extra; ++i)
        {
            PyTuple_SET_ITEM (hook->data, i, Py_None);
            Py_INCREF (Py_None);
        }

        Py_XDECREF (hook->callback);
        Py_XDECREF (hook->data);
        g_free (hook);
    }
}
