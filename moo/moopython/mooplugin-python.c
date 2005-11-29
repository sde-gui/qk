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

typedef struct _MooPyPluginData MooPyPluginData;

typedef struct {
    char **dirs;
    GHashTable *hook_ids;
    GSList *hooks[HOOK_LAST];
    int last_id;
    GSList *plugins; /* MooPyPluginInfo* */
} MooPythonPlugin;

static MooPythonPlugin python_plugin;


static Hook     *hook_new                       (HookType            type,
                                                 PyObject           *callback,
                                                 PyObject           *data);
static void      hook_free                      (Hook               *hook);

static PyObject *moo_python_plugin_add_hook     (MooPythonPlugin    *plugin,
                                                 HookType            type,
                                                 PyObject           *callback,
                                                 PyObject           *data);
static void      moo_python_plugin_remove_hook  (MooPythonPlugin    *plugin,
                                                 int                 id);

static void      moo_py_plugin_delete           (MooPyPluginData    *data);


static void
call_hooks (MooPythonPlugin *plugin,
            MooEditWindow   *window,
            MooEdit         *doc,
            HookType         type)
{
    GSList *l;
    PyObject *py_win, *py_doc;

    g_return_if_fail (!window || MOO_IS_EDIT_WINDOW (window));
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


void
_moo_python_attach_win (MooEditWindow *window)
{
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    call_hooks (&python_plugin, window, NULL, HOOK_NEW_WINDOW);
}


void
_moo_python_detach_win (MooEditWindow *window)
{
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    call_hooks (&python_plugin, window, NULL, HOOK_CLOSE_WINDOW);
}


void
_moo_python_attach_doc (MooEditWindow   *window,
                        MooEdit         *doc)
{
    g_return_if_fail (!window || MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDIT (doc));
    call_hooks (&python_plugin, window, doc, HOOK_NEW_DOC);
}


void
_moo_python_detach_doc (MooEditWindow   *window,
                        MooEdit         *doc)
{
    g_return_if_fail (!window || MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDIT (doc));
    call_hooks (&python_plugin, window, doc, HOOK_CLOSE_DOC);
}


static void
moo_python_plugin_read_file (const char *path)
{
    PyObject *mod, *code;
    char *modname = NULL, *content = NULL;
    GError *error = NULL;

    g_return_if_fail (path != NULL);

    if (!g_file_get_contents (path, &content, NULL, &error))
    {
        g_warning ("%s: could not read plugin file", G_STRLOC);
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
        return;
    }

    modname = g_strdup_printf ("moo_plugin_%08x%08x", g_random_int (), g_random_int ());
    code = Py_CompileString (content, path, Py_file_input);

    if (!code)
    {
        PyErr_Print ();
        goto out;
    }

    mod = PyImport_ExecCodeModule (modname, code);
    Py_DECREF (code);

    if (!mod)
    {
        PyErr_Print ();
        goto out;
    }

    Py_DECREF (mod);

out:
    g_free (content);
    g_free (modname);
}


static void
moo_python_plugin_read_dir (const char      *path)
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
        moo_python_plugin_read_file (file_path);

        g_free (prefix);
        g_free (file_path);
    }

    g_dir_close (dir);
}


static void
moo_python_plugin_read_dirs (char **dirs)
{
    for ( ; dirs && *dirs; ++dirs)
        moo_python_plugin_read_dir (*dirs);
}


void
_moo_python_plugin_init (char **dirs)
{
    if (!python_plugin.hook_ids)
        python_plugin.hook_ids = g_hash_table_new (g_direct_hash, g_direct_equal);
    if (python_plugin.dirs)
        g_strfreev (python_plugin.dirs);
    python_plugin.dirs = g_strdupv (dirs);
    moo_python_plugin_read_dirs (dirs);
}


void
_moo_python_plugin_reload (void)
{
    char **dirs = python_plugin.dirs;
    python_plugin.dirs = NULL;
    _moo_python_plugin_deinit ();
    _moo_python_plugin_init (dirs);
    g_strfreev (dirs);
}


static void
prepend_id (gpointer id,
            G_GNUC_UNUSED gpointer hook,
            GSList **list)
{
    *list = g_slist_prepend (*list, id);
}

void
_moo_python_plugin_deinit (void)
{
    GSList *ids = NULL, *l;

    g_return_if_fail (python_plugin.hook_ids != NULL);
    g_hash_table_foreach (python_plugin.hook_ids, (GHFunc) prepend_id, &ids);

    for (l = ids; l != NULL; l = l->next)
        moo_python_plugin_remove_hook (&python_plugin, GPOINTER_TO_INT (l->data));

    g_hash_table_destroy (python_plugin.hook_ids);
    python_plugin.hook_ids = NULL;
    g_slist_free (ids);

    g_slist_foreach (python_plugin.plugins, (GFunc) moo_py_plugin_delete, NULL);
    g_slist_free (python_plugin.plugins);
    python_plugin.plugins = NULL;
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
    PyObject *result;

    if (!strcmp (event, "new-window"))
        result = moo_python_plugin_add_hook (&python_plugin, HOOK_NEW_WINDOW, callback, data);
    else if (!strcmp (event, "close-window"))
        result = moo_python_plugin_add_hook (&python_plugin, HOOK_CLOSE_WINDOW, callback, data);
    else if (!strcmp (event, "new-doc"))
        result = moo_python_plugin_add_hook (&python_plugin, HOOK_NEW_DOC, callback, data);
    else if (!strcmp (event, "close-doc"))
        result = moo_python_plugin_add_hook (&python_plugin, HOOK_CLOSE_DOC, callback, data);
    else
        return_TypeErr ("invalid event type");

    return result;
}


static Hook*
hook_new (HookType    type,
          PyObject   *callback,
          PyObject   *data)
{
    int data_len, extra = 0, i;
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
#define MOO_PY_WIN_PLUGIN(obj_) ((MooPyWinPlugin*)obj_)
#define MOO_PY_DOC_PLUGIN(obj_) ((MooPyDocPlugin*)obj_)

typedef struct _MooPyPlugin             MooPyPlugin;
typedef struct _MooPyPluginClass        MooPyPluginClass;
typedef struct _MooPyWinPlugin          MooPyWinPlugin;
typedef struct _MooPyWinPluginClass     MooPyWinPluginClass;
typedef struct _MooPyDocPlugin          MooPyDocPlugin;
typedef struct _MooPyDocPluginClass     MooPyDocPluginClass;

struct _MooPyPluginData {
    PyObject   *py_plugin_type;
    PyObject   *py_win_plugin_type;
    PyObject   *py_doc_plugin_type;
    GType plugin_type;
    GType win_plugin_type;
    GType doc_plugin_type;
};

struct _MooPyPlugin {
    MooPlugin base;
    MooPyPluginData *class_data;
    PyObject *instance;
};

struct _MooPyPluginClass {
    MooPluginClass base_class;
    MooPyPluginData *data;
};

struct _MooPyWinPlugin {
    MooWinPlugin base;
    MooPyPluginData *class_data;
    PyObject *instance;
};

struct _MooPyWinPluginClass {
    MooWinPluginClass base_class;
    MooPyPluginData *data;
};

struct _MooPyDocPlugin {
    MooDocPlugin base;
    MooPyPluginData *class_data;
    PyObject *instance;
};

struct _MooPyDocPluginClass {
    MooDocPluginClass base_class;
    MooPyPluginData *data;
};


static gpointer moo_py_plugin_parent_class;
static gpointer moo_py_win_plugin_parent_class;
static gpointer moo_py_doc_plugin_parent_class;


static void     moo_py_plugin_class_init        (MooPyPluginClass       *klass,
                                                 MooPyPluginData        *data);
static void     moo_py_plugin_instance_init     (MooPyPlugin            *plugin,
                                                 MooPyPluginClass       *klass);
static void     moo_py_plugin_finalize          (GObject                *object);

static void     moo_py_win_plugin_class_init    (MooPyWinPluginClass    *klass,
                                                 MooPyPluginData        *data);
static void     moo_py_win_plugin_instance_init (MooPyWinPlugin         *plugin,
                                                 MooPyWinPluginClass    *klass);
static void     moo_py_win_plugin_finalize      (GObject                *object);

static void     moo_py_doc_plugin_class_init    (MooPyDocPluginClass    *klass,
                                                 MooPyPluginData        *data);
static void     moo_py_doc_plugin_instance_init (MooPyDocPlugin         *plugin,
                                                 MooPyDocPluginClass    *klass);
static void     moo_py_doc_plugin_finalize      (GObject                *object);

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

static gboolean moo_py_win_plugin_create    (MooWinPlugin           *plugin);
static void     moo_py_win_plugin_destroy   (MooWinPlugin           *plugin);

static gboolean moo_py_doc_plugin_create    (MooDocPlugin           *plugin);
static void     moo_py_doc_plugin_destroy   (MooDocPlugin           *plugin);

static MooPluginInfo *get_plugin_info       (PyObject               *object);
static GType    generate_win_plugin_type    (PyObject               *py_type,
                                             MooPyPluginData        *data);
static GType    generate_doc_plugin_type    (PyObject               *py_type,
                                             MooPyPluginData        *data);

static GtkWidget *moo_py_plugin_create_prefs_page (MooPlugin        *plugin);

static void     plugin_info_free            (MooPluginInfo          *info);


static void
moo_py_plugin_class_init (MooPyPluginClass       *klass,
                          MooPyPluginData        *data)
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
    MooPlugin *moo_plugin = MOO_PLUGIN (plugin);

    plugin->class_data = klass->data;

    plugin->instance = PyObject_CallObject (klass->data->py_plugin_type, NULL);

    if (!plugin->instance)
    {
        PyErr_Print ();
        g_warning ("%s: could not create plugin instance", G_STRLOC);
        g_return_if_reached ();
    }

    moo_plugin->info = get_plugin_info (plugin->instance);

    if (klass->data->py_win_plugin_type && !klass->data->win_plugin_type)
        klass->data->win_plugin_type = generate_win_plugin_type (klass->data->py_win_plugin_type, klass->data);

    if (klass->data->py_doc_plugin_type && !klass->data->doc_plugin_type)
        klass->data->doc_plugin_type = generate_doc_plugin_type (klass->data->py_doc_plugin_type, klass->data);

    moo_plugin->win_plugin_type = klass->data->win_plugin_type;
    moo_plugin->doc_plugin_type = klass->data->doc_plugin_type;
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


PyObject*
_moo_python_plugin_register (PyObject   *py_plugin_type,
                             PyObject   *py_win_plugin_type,
                             PyObject   *py_doc_plugin_type)
{
    char *plugin_type_name = NULL;
    static GTypeInfo plugin_type_info;
    MooPyPluginData *class_data;
    int i;

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

    class_data = g_new0 (MooPyPluginData, 1);
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

    class_data->plugin_type = g_type_register_static (MOO_TYPE_PLUGIN, plugin_type_name,
                                                      &plugin_type_info, 0);
    g_free (plugin_type_name);

    if (!moo_plugin_register (class_data->plugin_type))
    {
        Py_XDECREF (class_data->py_plugin_type);
        Py_XDECREF (class_data->py_win_plugin_type);
        Py_XDECREF (class_data->py_doc_plugin_type);
        g_free (class_data);
        return_RuntimeErr ("could not register plugin");
    }

    python_plugin.plugins = g_slist_prepend (python_plugin.plugins, class_data);

    return_None;
}


static void
moo_py_plugin_delete (MooPyPluginData *data)
{
    g_return_if_fail (data != NULL);
    moo_plugin_unregister (data->plugin_type);
    Py_XDECREF (data->py_plugin_type);
    Py_XDECREF (data->py_win_plugin_type);
    Py_XDECREF (data->py_doc_plugin_type);
    g_free (data);
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
generate_win_plugin_type (PyObject             *py_type,
                          MooPyPluginData      *class_data)
{
    GType type;
    char *type_name = NULL;
    static GTypeInfo type_info;
    int i;

    g_return_val_if_fail (py_type != NULL, 0);
    g_return_val_if_fail (PyType_Check (py_type), 0);

    for (i = 0; i < 1000; ++i)
    {
        type_name = g_strdup_printf ("MooPyWinPlugin-%08x", g_random_int ());
        if (!g_type_from_name (type_name))
            break;
        g_free (type_name);
        type_name = NULL;
    }

    if (!type_name)
    {
        g_critical ("%s: could not find name for win plugin class", G_STRLOC);
        return 0;
    }

    type_info.class_size = sizeof (MooPyWinPluginClass);
    type_info.class_init = (GClassInitFunc) moo_py_win_plugin_class_init;
    type_info.class_data = class_data;
    type_info.instance_size = sizeof (MooPyWinPlugin);
    type_info.instance_init = (GInstanceInitFunc) moo_py_win_plugin_instance_init;

    type = g_type_register_static (MOO_TYPE_WIN_PLUGIN, type_name, &type_info, 0);

    g_free (type_name);
    return type;
}

static GType
generate_doc_plugin_type (PyObject               *py_type,
                          MooPyPluginData        *class_data)
{
    char *type_name = NULL;
    static GTypeInfo type_info;
    int i;
    GType type;

    g_return_val_if_fail (py_type != NULL, 0);
    g_return_val_if_fail (PyType_Check (py_type), 0);

    for (i = 0; i < 1000; ++i)
    {
        type_name = g_strdup_printf ("MooPyDocPlugin-%08x", g_random_int ());
        if (!g_type_from_name (type_name))
            break;
        g_free (type_name);
        type_name = NULL;
    }

    if (!type_name)
    {
        g_critical ("%s: could not find name for doc plugin class", G_STRLOC);
        return 0;
    }

    type_info.class_size = sizeof (MooPyDocPluginClass);
    type_info.class_init = (GClassInitFunc) moo_py_doc_plugin_class_init;
    type_info.class_data = class_data;
    type_info.instance_size = sizeof (MooPyDocPlugin);
    type_info.instance_init = (GInstanceInitFunc) moo_py_doc_plugin_instance_init;

    type = g_type_register_static (MOO_TYPE_DOC_PLUGIN, type_name, &type_info, 0);

    g_free (type_name);
    return type;
}


static void
moo_py_win_plugin_class_init (MooPyWinPluginClass    *klass,
                              MooPyPluginData        *data)
{
    MooWinPluginClass *plugin_class = MOO_WIN_PLUGIN_CLASS (klass);
    GObjectClass *gobj_class = G_OBJECT_CLASS (klass);

    moo_py_win_plugin_parent_class = g_type_class_peek_parent (klass);

    klass->data = data;
    gobj_class->finalize = moo_py_win_plugin_finalize;
    plugin_class->create = moo_py_win_plugin_create;
    plugin_class->destroy = moo_py_win_plugin_destroy;
}

static void
moo_py_win_plugin_instance_init (MooPyWinPlugin         *plugin,
                                 MooPyWinPluginClass    *klass)
{
    PyObject *args;

    plugin->class_data = klass->data;

    args = PyTuple_New (0);
    plugin->instance = PyType_GenericNew ((PyTypeObject*) klass->data->py_win_plugin_type, args, NULL);
    Py_DECREF (args);
}

static void
moo_py_win_plugin_finalize (GObject *object)
{
    MooPyWinPlugin *plugin = MOO_PY_WIN_PLUGIN (object);
    Py_XDECREF (plugin->instance);
    G_OBJECT_CLASS(moo_py_win_plugin_parent_class)->finalize (object);
}


static void
moo_py_doc_plugin_class_init (MooPyDocPluginClass    *klass,
                              MooPyPluginData        *data)
{
    MooDocPluginClass *plugin_class = MOO_DOC_PLUGIN_CLASS (klass);
    GObjectClass *gobj_class = G_OBJECT_CLASS (klass);

    moo_py_doc_plugin_parent_class = g_type_class_peek_parent (klass);

    klass->data = data;
    gobj_class->finalize = moo_py_doc_plugin_finalize;
    plugin_class->create = moo_py_doc_plugin_create;
    plugin_class->destroy = moo_py_doc_plugin_destroy;
}

static void
moo_py_doc_plugin_instance_init (MooPyDocPlugin         *plugin,
                                 MooPyDocPluginClass    *klass)
{
    PyObject *args;

    plugin->class_data = klass->data;

    args = PyTuple_New (0);
    plugin->instance = PyType_GenericNew ((PyTypeObject*) klass->data->py_doc_plugin_type, args, NULL);
    Py_DECREF (args);
}

static void
moo_py_doc_plugin_finalize (GObject *object)
{
    MooPyDocPlugin *plugin = MOO_PY_DOC_PLUGIN (object);
    Py_XDECREF (plugin->instance);
    G_OBJECT_CLASS(moo_py_doc_plugin_parent_class)->finalize (object);
}


/**************************************************************************/
/* Plugin methods
 */

static gboolean
call_any_meth__ (PyObject               *instance,
                 MooEdit                *doc,
                 MooEditWindow          *window,
                 const char             *meth_name)
{
    PyObject *result, *meth, *py_window, *py_doc;

    if (!instance)
    {
        g_critical ("%s: oops", G_STRLOC);
        return FALSE;
    }

    if (!PyObject_HasAttrString (instance, (char*) meth_name))
        return TRUE;

    meth = PyObject_GetAttrString (instance, (char*) meth_name);

    if (!meth)
    {
        PyErr_Print ();
        return FALSE;
    }

    if (!PyCallable_Check (meth))
    {
        g_critical ("%s: %s is not callable", G_STRLOC, meth_name);
        Py_DECREF (meth);
        return FALSE;
    }

    py_window = window ? pygobject_new (G_OBJECT (window)) : NULL;
    py_doc = doc ? pygobject_new (G_OBJECT (doc)) : NULL;

    if (window)
    {
        if (doc)
            result = PyObject_CallFunction (meth, (char*) "(OO)", py_doc, py_window);
        else
            result = PyObject_CallFunction (meth, (char*) "(O)", py_window);
    }
    else
    {
        result = PyObject_CallObject (meth, NULL);
    }

    Py_XDECREF (meth);
    Py_XDECREF (py_window);
    Py_XDECREF (py_doc);

    if (result)
    {
        gboolean bool_result = PyObject_IsTrue (result);
        Py_DECREF (result);
        return bool_result;
    }
    else
    {
        PyErr_Print ();
        return FALSE;
    }
}

static gboolean
call_bool_meth (PyObject               *instance,
                MooEdit                *doc,
                MooEditWindow          *window,
                const char             *meth_name)
{
    return call_any_meth__ (instance, doc, window, meth_name);
}

static void
call_meth (PyObject               *instance,
           MooEdit                *doc,
           MooEditWindow          *window,
           const char             *meth_name)
{
    call_any_meth__ (instance, doc, window, meth_name);
}


static gboolean
moo_py_plugin_init (MooPlugin *plugin)
{
    return call_bool_meth (MOO_PY_PLUGIN(plugin)->instance, NULL, NULL, "init");
}

static void
moo_py_plugin_deinit (MooPlugin *plugin)
{
    call_meth (MOO_PY_PLUGIN(plugin)->instance, NULL, NULL, "deinit");
}

static void
moo_py_plugin_attach_win (MooPlugin              *plugin,
                          MooEditWindow          *window)
{
    call_meth (MOO_PY_PLUGIN(plugin)->instance, NULL, window, "attach_win");
}

static void
moo_py_plugin_detach_win (MooPlugin              *plugin,
                          MooEditWindow          *window)
{
    call_meth (MOO_PY_PLUGIN(plugin)->instance, NULL, window, "detach_win");
}

static void
moo_py_plugin_attach_doc (MooPlugin              *plugin,
                          MooEdit                *doc,
                          MooEditWindow          *window)
{
    call_meth (MOO_PY_PLUGIN(plugin)->instance, doc, window, "attach_doc");
}

static void
moo_py_plugin_detach_doc (MooPlugin              *plugin,
                          MooEdit                *doc,
                          MooEditWindow          *window)
{
    call_meth (MOO_PY_PLUGIN(plugin)->instance, doc, window, "detach_doc");
}

static gboolean
moo_py_win_plugin_create (MooWinPlugin *plugin)
{
    return call_bool_meth (MOO_PY_WIN_PLUGIN(plugin)->instance,
                           NULL, plugin->window, "create");
}

static void
moo_py_win_plugin_destroy (MooWinPlugin *plugin)
{
    call_meth (MOO_PY_WIN_PLUGIN(plugin)->instance,
               NULL, plugin->window, "destroy");
}

static gboolean
moo_py_doc_plugin_create (MooDocPlugin *plugin)
{
    return call_bool_meth (MOO_PY_DOC_PLUGIN(plugin)->instance,
                           plugin->doc, plugin->window, "create");
}

static void
moo_py_doc_plugin_destroy (MooDocPlugin *plugin)
{
    call_meth (MOO_PY_DOC_PLUGIN(plugin)->instance,
               plugin->doc, plugin->window, "destroy");
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
