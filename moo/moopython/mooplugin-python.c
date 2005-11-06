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


/***************************************************************************/
/* Python plugins
 */

#define MOO_PY_PLUGIN(obj_) ((MooPyPlugin*)obj_)

typedef struct _MooPyPlugin             MooPyPlugin;
typedef struct _MooPyPluginClass        MooPyPluginClass;
typedef struct _MooPyPluginClassData    MooPyPluginClassData;

struct _MooPyPluginClassData {
    PyObject   *py_plugin_type;
    PyObject   *py_win_plugin_type;
    PyObject   *py_doc_plugin_type;
    GType win_plugin_type;
    GType doc_plugin_type;
};

struct _MooPyPlugin {
    MooPlugin base;
    MooPyPluginClassData *class_data;
    PyObject *instance;
};

struct _MooPyPluginClass {
    MooPluginClass base_class;
    MooPyPluginClassData *data;
};


static gpointer moo_py_plugin_parent_class;


static void     moo_py_plugin_class_init    (MooPyPluginClass       *klass,
                                             MooPyPluginClassData   *data);
static void     moo_py_plugin_instance_init (MooPyPlugin            *plugin,
                                             MooPyPluginClass       *klass);
static void     moo_py_plugin_finalize      (GObject                *object);

static gboolean moo_py_plugin_init          (MooPlugin              *plugin);
static void     moo_py_plugin_deinit        (MooPlugin              *plugin);
static void     moo_py_plugin_attach_win    (MooPlugin              *plugin,
                                             MooEditWindow          *window);
static void     moo_py_plugin_detach_win    (MooPlugin              *plugin,
                                             MooEditWindow          *window);
static void     moo_py_plugin_attach_doc    (MooPlugin              *plugin,
                                             MooEdit                *doc,
                                             MooEditWindow          *window);
static void     moo_py_plugin_detach_doc    (MooPlugin              *plugin,
                                             MooEdit                *doc,
                                             MooEditWindow          *window);

static MooPluginInfo *get_plugin_info       (PyObject               *object);
static GType    generate_win_plugin_type    (PyObject               *py_type);
static GType    generate_doc_plugin_type    (PyObject               *py_type);

static GtkWidget *moo_py_plugin_create_prefs_page (MooPlugin        *plugin);

static void     plugin_info_free            (MooPluginInfo          *info);


static void
moo_py_plugin_class_init (MooPyPluginClass       *klass,
                          MooPyPluginClassData   *data)
{
    MooPluginClass *plugin_class = MOO_PLUGIN_CLASS (klass);
    GObjectClass *gobj_class = G_OBJECT_CLASS (klass);

    moo_py_plugin_parent_class = g_type_class_peek_parent (klass);

    klass->data = data;
    gobj_class->finalize = moo_py_plugin_finalize;
    plugin_class->plugin_system_version = MOO_PLUGIN_CURRENT_VERSION;

    if (data->py_plugin_type)
    {
        plugin_class->init = moo_py_plugin_init;
        plugin_class->deinit = moo_py_plugin_deinit;
        plugin_class->attach_win = moo_py_plugin_attach_win;
        plugin_class->detach_win = moo_py_plugin_detach_win;
        plugin_class->attach_doc = moo_py_plugin_attach_doc;
        plugin_class->detach_doc = moo_py_plugin_detach_doc;
        plugin_class->create_prefs_page = moo_py_plugin_create_prefs_page;
    }
}


static void
moo_py_plugin_instance_init (MooPyPlugin            *plugin,
                             MooPyPluginClass       *klass)
{
    PyObject *args;
    MooPlugin *moo_plugin = MOO_PLUGIN (plugin);

    plugin->class_data = klass->data;

    args = PyTuple_New (0);
    plugin->instance = PyType_GenericNew ((PyTypeObject*) klass->data->py_plugin_type, args, NULL);
    Py_DECREF (args);

    g_return_if_fail (plugin->instance != NULL);

    moo_plugin->info = get_plugin_info (plugin->instance);

    if (klass->data->py_win_plugin_type && !klass->data->win_plugin_type)
        klass->data->win_plugin_type = generate_win_plugin_type (klass->data->py_win_plugin_type);
    if (klass->data->py_win_plugin_type && !klass->data->doc_plugin_type)
        klass->data->doc_plugin_type = generate_doc_plugin_type (klass->data->py_doc_plugin_type);

    moo_plugin->win_plugin_type = klass->data->win_plugin_type;
    moo_plugin->doc_plugin_type = klass->data->doc_plugin_type;
}


static gboolean
moo_py_plugin_init (MooPlugin *plugin)
{
    PyObject *result, *meth;
    MooPyPlugin *py_plugin = MOO_PY_PLUGIN (plugin);
    gboolean bool_result;

    g_return_val_if_fail (py_plugin->instance != NULL, FALSE);

    if (!PyObject_HasAttrString (py_plugin->instance, (char*) "init"))
         return TRUE;

    meth = PyObject_GetAttrString (py_plugin->instance, (char*) "init");

    if (!meth)
    {
        PyErr_Print ();
        return FALSE;
    }

    if (!PyCallable_Check (meth))
    {
        g_critical ("%s: init is not callable", G_STRLOC);
        Py_DECREF (meth);
        return FALSE;
    }

    result = PyObject_CallObject (meth, NULL);
    Py_DECREF (meth);

    if (!result)
    {
        PyErr_Print ();
        return FALSE;
    }

    bool_result = PyObject_IsTrue (result);

    Py_DECREF (result);
    return bool_result;
}


static void
moo_py_plugin_deinit (MooPlugin *plugin)
{
    PyObject *result, *meth;
    MooPyPlugin *py_plugin = MOO_PY_PLUGIN (plugin);

    g_return_if_fail (py_plugin->instance != NULL);

    if (!PyObject_HasAttrString (py_plugin->instance, (char*) "deinit"))
         return;

    meth = PyObject_GetAttrString (py_plugin->instance, (char*) "deinit");

    if (!meth)
    {
        PyErr_Print ();
        return;
    }

    if (!PyCallable_Check (meth))
    {
        g_critical ("%s: deinit is not callable", G_STRLOC);
        Py_DECREF (meth);
        return;
    }

    result = PyObject_CallObject (meth, NULL);
    Py_DECREF (meth);

    if (!result)
        PyErr_Print ();

    Py_XDECREF (result);
}


static void
moo_py_plugin_call_meth (MooPlugin              *plugin,
                         MooEdit                *doc,
                         MooEditWindow          *window,
                         const char             *meth_name)
{
    PyObject *result, *meth, *py_window, *py_doc;
    MooPyPlugin *py_plugin = MOO_PY_PLUGIN (plugin);

    g_return_if_fail (py_plugin->instance != NULL);

    if (!PyObject_HasAttrString (py_plugin->instance, (char*) meth_name))
         return;

    meth = PyObject_GetAttrString (py_plugin->instance, (char*) meth_name);

    if (!meth)
    {
        PyErr_Print ();
        return;
    }

    if (!PyCallable_Check (meth))
    {
        g_critical ("%s: %s is not callable", G_STRLOC, meth_name);
        Py_DECREF (meth);
        return;
    }

    py_window = pygobject_new (G_OBJECT (window));
    py_doc = doc ? pygobject_new (G_OBJECT (doc)) : NULL;

    if (doc)
        result = PyObject_CallFunction (meth, (char*) "(OO)", py_doc, py_window);
    else
        result = PyObject_CallFunction (meth, (char*) "(O)", py_window);

    Py_XDECREF (meth);
    Py_XDECREF (py_window);
    Py_XDECREF (py_doc);

    if (!result)
        PyErr_Print ();

    Py_XDECREF (result);
}


static void
moo_py_plugin_attach_win (MooPlugin              *plugin,
                          MooEditWindow          *window)
{
    moo_py_plugin_call_meth (plugin, NULL, window, "attach_win");
}

static void
moo_py_plugin_detach_win (MooPlugin              *plugin,
                          MooEditWindow          *window)
{
    moo_py_plugin_call_meth (plugin, NULL, window, "detach_win");
}

static void
moo_py_plugin_attach_doc (MooPlugin              *plugin,
                          MooEdit                *doc,
                          MooEditWindow          *window)
{
    moo_py_plugin_call_meth (plugin, doc, window, "attach_doc");
}

static void
moo_py_plugin_detach_doc (MooPlugin              *plugin,
                          MooEdit                *doc,
                          MooEditWindow          *window)
{
    moo_py_plugin_call_meth (plugin, doc, window, "detach_doc");
}


static void
moo_py_plugin_finalize (GObject *object)
{
    MooPyPlugin *py_plugin = MOO_PY_PLUGIN (object);
    MooPlugin *plugin = MOO_PLUGIN (object);
    Py_XDECREF (py_plugin->instance);
    plugin_info_free (plugin->info);
    G_OBJECT_CLASS(moo_py_plugin_parent_class)->finalize (object);
}


static GtkWidget*
moo_py_plugin_create_prefs_page (MooPlugin *plugin)
{
    PyObject *result, *meth;
    MooPyPlugin *py_plugin = MOO_PY_PLUGIN (plugin);

    g_return_val_if_fail (py_plugin->instance != NULL, NULL);

    if (!PyObject_HasAttrString (py_plugin->instance, (char*) "create_prefs_page"))
         return NULL;

    meth = PyObject_GetAttrString (py_plugin->instance, (char*) "create_prefs_page");

    if (!meth)
    {
        PyErr_Print ();
        return NULL;
    }

    if (!PyCallable_Check (meth))
    {
        g_critical ("%s: create_prefs_page is not callable", G_STRLOC);
        Py_DECREF (meth);
        return NULL;
    }

    result = PyObject_CallObject (meth, NULL);
    Py_DECREF (meth);

    if (!result)
    {
        PyErr_Print ();
        return NULL;
    }
    else
    {
        gpointer page = pygobject_get (result);
        g_object_ref (page);
        Py_DECREF (result);
        return page;
    }
}


PyObject*
_moo_python_plugin_register (const char *id,
                             PyObject   *py_plugin_type,
                             PyObject   *py_win_plugin_type,
                             PyObject   *py_doc_plugin_type)
{
    GType plugin_type;
    char *plugin_type_name = NULL;
    static GTypeInfo plugin_type_info;
    MooPyPluginClassData *class_data;
    int i;

    g_return_val_if_fail (id != NULL, NULL);
    g_return_val_if_fail (py_plugin_type != NULL, NULL);
    g_return_val_if_fail (PyType_Check (py_plugin_type), NULL);
    g_return_val_if_fail (!py_win_plugin_type || PyType_Check (py_win_plugin_type), NULL);
    g_return_val_if_fail (!py_doc_plugin_type || PyType_Check (py_doc_plugin_type), NULL);

    for (i = 0; i < 1000; ++i)
    {
        plugin_type_name = g_strdup_printf ("MooPyPlugin-%08x", g_random_int ());
        if (!g_type_from_name (plugin_type_name))
            break;
        g_free (plugin_type_name);
        plugin_type_name = NULL;
    }

    if (!plugin_type_name)
        return_RuntimeErr ("could not find name for plugin class");

    class_data = g_new0 (MooPyPluginClassData, 1);
    class_data->py_plugin_type = py_plugin_type;
    class_data->py_win_plugin_type = py_win_plugin_type;
    class_data->py_doc_plugin_type = py_doc_plugin_type;
    Py_XINCREF (py_plugin_type);
    Py_XINCREF (py_win_plugin_type);
    Py_XINCREF (py_doc_plugin_type);

    plugin_type_info.class_size = sizeof (MooPyPluginClass);
    plugin_type_info.class_init = (GClassInitFunc) moo_py_plugin_class_init;
    plugin_type_info.class_data = class_data;
    plugin_type_info.instance_size = sizeof (MooPyPlugin);
    plugin_type_info.instance_init = (GInstanceInitFunc) moo_py_plugin_instance_init;

    plugin_type = g_type_register_static (MOO_TYPE_PLUGIN, plugin_type_name,
                                          &plugin_type_info, 0);

    if (!moo_plugin_register (plugin_type))
    {
        Py_XDECREF (class_data->py_plugin_type);
        Py_XINCREF (class_data->py_win_plugin_type);
        Py_XINCREF (class_data->py_doc_plugin_type);
        g_free (class_data);
        return_RuntimeErr ("could not register plugin");
    }

    return_None;
}


static void
plugin_info_free (MooPluginInfo *info)
{
    if (info)
    {
        g_free ((char*) info->id);
        g_free ((char*) info->name);
        g_free ((char*) info->description);
        g_free ((char*) info->author);
        g_free ((char*) info->version);
        g_free (info->params);
        g_free (info->prefs_params);
        g_free (info);
    }
}


static char *
dict_get_string (PyObject   *dict,
                 const char *key)
{
    PyObject *val, *strval;
    char *result;

    val = PyDict_GetItemString (dict, (char*) key);

    if (!val)
        return NULL;

    strval = PyObject_Str (val);
    Py_DECREF (val);

    if (!strval)
    {
        PyErr_Print ();
        return NULL;
    }

    result = g_strdup (PyString_AS_STRING (strval));

    Py_DECREF (strval);
    return result;
}


static gboolean
dict_get_bool (PyObject   *dict,
               const char *key,
               gboolean    default_val)
{
    PyObject *val;
    gboolean result;

    val = PyDict_GetItemString (dict, (char*) key);

    if (!val)
        return default_val;

    result = PyObject_IsTrue (val);

    Py_DECREF (val);
    return result;
}


static MooPluginInfo*
get_plugin_info (PyObject *object)
{
    MooPluginInfo *info;
    PyObject *result, *meth;

    if (!PyObject_HasAttrString (object, (char*) "get_info"))
    {
        g_critical ("%s: no get_info attribute", G_STRLOC);
        return NULL;
    }

    meth = PyObject_GetAttrString (object, (char*) "get_info");

    if (!meth)
    {
        PyErr_Print ();
        return NULL;
    }

    if (!PyCallable_Check (meth))
    {
        g_critical ("%s: get_info is not callable", G_STRLOC);
        Py_DECREF (meth);
        return NULL;
    }

    result = PyObject_CallObject (meth, NULL);
    Py_DECREF (meth);

    if (!result)
    {
        PyErr_Print ();
        return NULL;
    }

    if (!PyDict_Check (result))
    {
        g_critical ("%s: not a dict", G_STRLOC);
        Py_DECREF (result);
        return NULL;
    }

    info = g_new0 (MooPluginInfo, 1);
    info->params = g_new0 (MooPluginParams, 1);
    info->prefs_params = g_new0 (MooPluginPrefsParams, 1);

    info->id = dict_get_string (result, "id");
    info->name = dict_get_string (result, "name");
    info->description = dict_get_string (result, "description");
    info->author = dict_get_string (result, "author");
    info->version = dict_get_string (result, "version");

    info->params->enabled = dict_get_bool (result, "enabled", TRUE);
    info->params->visible = dict_get_bool (result, "visible", TRUE);

    return info;
}


static GType
generate_win_plugin_type (PyObject *py_type)
{
    return 0;
}

static GType
generate_doc_plugin_type (PyObject *py_type)
{
    return 0;
}
