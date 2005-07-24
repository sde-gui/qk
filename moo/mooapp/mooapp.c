/*
 *   mooapp/mooapp.c
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
#endif /* HAVE_CONFIG_H */

#ifdef USE_PYTHON
#include <Python.h>
#include "mooapp/moopythonconsole.h"
#endif

#include "mooapp/mooapp-private.h"
#include "mooapp/moopython.h"
#include "mooapp/mooappinput.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mooeditor.h"
#include "mooui/moouiobject.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moocompat.h"
#include "mooutils/moodialogs.h"
#include "mooutils/moostock.h"


/* moo-pygtk.c */
void initmoo (void);


static MooApp *moo_app_instance = NULL;
static MooPython *moo_app_python = NULL;
static MooAppInput *moo_app_input = NULL;


struct _MooAppPrivate {
    char      **argv;
    int         exit_code;
    MooEditor  *editor;
    MooAppInfo *info;
    gboolean    running;
    gboolean    run_python;
    MooAppWindowPolicy window_policy;
    GSList     *terminals;
    MooTermWindow *term_window;
    MooUIXML   *ui_xml;
};


static void     moo_app_class_init      (MooAppClass    *klass);
static void     moo_app_gobj_init       (MooApp         *app);
static GObject *moo_edit_constructor    (GType           type,
                                         guint           n_params,
                                         GObjectConstructParam *params);
static void     moo_app_finalize        (GObject        *object);

static void     moo_app_install_actions (MooApp         *app,
                                         GType           type);

static void     moo_app_set_property    (GObject        *object,
                                         guint           prop_id,
                                         const GValue   *value,
                                         GParamSpec     *pspec);
static void     moo_app_get_property    (GObject        *object,
                                         guint           prop_id,
                                         GValue         *value,
                                         GParamSpec     *pspec);

static void     moo_app_set_argv        (MooApp         *app,
                                         char          **argv);
static char   **moo_app_get_argv        (MooApp         *app);

static gboolean moo_app_init_real       (MooApp         *app);
static int      moo_app_run_real        (MooApp         *app);
static void     moo_app_quit_real       (MooApp         *app);
static gboolean moo_app_try_quit_real   (MooApp         *app);

static void     moo_app_set_name        (MooApp         *app,
                                         const char     *short_name,
                                         const char     *full_name);

static void     all_editors_closed      (MooApp         *app);
static void     start_python            (MooApp         *app);


static GObjectClass *moo_app_parent_class;
GType moo_app_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GTypeInfo type_info = {
            sizeof (MooAppClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) moo_app_class_init,
            (GClassFinalizeFunc) NULL,
            NULL,   /* class_data */
            sizeof (MooApp),
            0,      /* n_preallocs */
            (GInstanceInitFunc) moo_app_gobj_init,
            NULL    /* value_table */
        };

        type = g_type_register_static (G_TYPE_OBJECT,
                                       "MooApp",
                                       &type_info,
                                       0);
    }

    return type;
}


enum {
    PROP_0,
    PROP_ARGV,
    PROP_SHORT_NAME,
    PROP_FULL_NAME,
    PROP_WINDOW_POLICY,
    PROP_RUN_PYTHON
};

enum {
    INIT,
    RUN,
    QUIT,
    TRY_QUIT,
    PREFS_DIALOG,
    LAST_SIGNAL
};


static guint signals[LAST_SIGNAL];


static void moo_app_class_init (MooAppClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    moo_create_stock_items ();

    moo_app_parent_class = g_type_class_peek_parent (klass);

    gobject_class->constructor = moo_edit_constructor;
    gobject_class->finalize = moo_app_finalize;
    gobject_class->set_property = moo_app_set_property;
    gobject_class->get_property = moo_app_get_property;

    klass->init = moo_app_init_real;
    klass->run = moo_app_run_real;
    klass->quit = moo_app_quit_real;
    klass->try_quit = moo_app_try_quit_real;
    klass->prefs_dialog = _moo_app_create_prefs_dialog;

    g_object_class_install_property (gobject_class,
                                     PROP_ARGV,
                                     g_param_spec_pointer
                                             ("argv",
                                              "argv",
                                              "Null-terminated array of application arguments",
                                              G_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_SHORT_NAME,
                                     g_param_spec_string
                                             ("short-name",
                                              "short-name",
                                              "short-name",
                                              "ggap",
                                              G_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_FULL_NAME,
                                     g_param_spec_string
                                             ("full-name",
                                              "full-name",
                                              "full-name",
                                              NULL,
                                              G_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_WINDOW_POLICY,
                                     g_param_spec_flags
                                             ("window-policy",
                                              "window-policy",
                                              "window-policy",
                                              MOO_TYPE_APP_WINDOW_POLICY,
                                              MOO_APP_ONE_EDITOR | MOO_APP_ONE_TERMINAL |
                                                      MOO_APP_QUIT_ON_CLOSE_ALL_WINDOWS,
                                              G_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_RUN_PYTHON,
                                     g_param_spec_boolean
                                             ("run-python",
                                              "run-python",
                                              "run-python",
                                              TRUE,
                                              G_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT));

    signals[INIT] = g_signal_new ("init",
                                  G_OBJECT_CLASS_TYPE (klass),
                                  0,
                                  G_STRUCT_OFFSET (MooAppClass, init),
                                  NULL, NULL,
                                  _moo_marshal_BOOLEAN__VOID,
                                  G_TYPE_STRING, 0);

    signals[RUN] = g_signal_new ("run",
                                 G_OBJECT_CLASS_TYPE (klass),
                                 0,
                                 G_STRUCT_OFFSET (MooAppClass, run),
                                 NULL, NULL,
                                 _moo_marshal_INT__VOID,
                                 G_TYPE_INT, 0);

    signals[QUIT] = g_signal_new ("quit",
                                  G_OBJECT_CLASS_TYPE (klass),
                                  0,
                                  G_STRUCT_OFFSET (MooAppClass, quit),
                                  NULL, NULL,
                                  _moo_marshal_VOID__VOID,
                                  G_TYPE_NONE, 0);

    signals[TRY_QUIT] = g_signal_new ("try-quit",
                                  G_OBJECT_CLASS_TYPE (klass),
                                  (GSignalFlags) (G_SIGNAL_ACTION | G_SIGNAL_RUN_LAST),
                                  G_STRUCT_OFFSET (MooAppClass, try_quit),
                                  g_signal_accumulator_true_handled, NULL,
                                  _moo_marshal_BOOLEAN__VOID,
                                  G_TYPE_BOOLEAN, 0);

    signals[PREFS_DIALOG] = g_signal_new ("prefs-dialog",
                                          G_OBJECT_CLASS_TYPE (klass),
                                          0,
                                          G_STRUCT_OFFSET (MooAppClass, quit),
                                          NULL, NULL,
                                          _moo_marshal_OBJECT__VOID,
                                          MOO_TYPE_PREFS_DIALOG, 0);
}


static void moo_app_gobj_init (MooApp *app)
{
    g_assert (moo_app_instance == NULL);

    moo_app_instance = app;

    app->priv = g_new0 (MooAppPrivate, 1);
    app->priv->info = g_new0 (MooAppInfo, 1);

#ifdef VERSION
    app->priv->info->version = g_strdup (VERSION);
#else
    app->priv->info->version = g_strdup ("<unknown version>");
#endif
    app->priv->info->website = g_strdup ("http://ggap.sourceforge.net/");
    app->priv->info->website_label = g_strdup ("ggap.sourceforge.net");
}


static GObject *moo_edit_constructor    (GType           type,
                                         guint           n_params,
                                         GObjectConstructParam *params)
{
    GObject *object = moo_app_parent_class->constructor (type, n_params, params);
    MooApp *app = MOO_APP (object);

    if (!app->priv->info->full_name)
        app->priv->info->full_name = g_strdup (app->priv->info->short_name);

    moo_app_install_actions (app, MOO_TYPE_EDIT_WINDOW);
    moo_app_install_actions (app, MOO_TYPE_TERM_WINDOW);

    return object;
}


static void moo_app_finalize       (GObject      *object)
{
    MooApp *app = MOO_APP(object);

    moo_app_quit_real (app);

    moo_app_instance = NULL;

    g_free (app->priv->info->short_name);
    g_free (app->priv->info->full_name);
    g_free (app->priv->info->app_dir);
    g_free (app->priv->info->rc_file);
    g_free (app->priv->info);

    if (app->priv->argv)
        g_strfreev (app->priv->argv);
    if (app->priv->editor)
        g_object_unref (app->priv->editor);
    if (app->priv->ui_xml)
        g_object_unref (app->priv->ui_xml);

    g_free (app->priv);

    G_OBJECT_CLASS (moo_app_parent_class)->finalize (object);
}


static void moo_app_set_property    (GObject        *object,
                                     guint           prop_id,
                                     const GValue   *value,
                                     GParamSpec     *pspec)
{
    MooApp *app = MOO_APP (object);
    switch (prop_id)
    {
        case PROP_ARGV:
            moo_app_set_argv (app, (char**)g_value_get_pointer (value));
            break;

        case PROP_SHORT_NAME:
            moo_app_set_name (app, g_value_get_string (value), NULL);
            break;

        case PROP_FULL_NAME:
            moo_app_set_name (app, NULL, g_value_get_string (value));
            break;

        case PROP_WINDOW_POLICY:
            app->priv->window_policy = g_value_get_flags (value);
            g_object_notify (object, "window-policy");
            break;

        case PROP_RUN_PYTHON:
            app->priv->run_python = g_value_get_boolean (value);
            g_object_notify (object, "run-python");
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void moo_app_get_property    (GObject        *object,
                                     guint           prop_id,
                                     GValue         *value,
                                     GParamSpec     *pspec)
{
    MooApp *app = MOO_APP (object);
    switch (prop_id)
    {
        case PROP_ARGV:
            g_value_set_pointer (value, moo_app_get_argv (app));
            break;

        case PROP_SHORT_NAME:
            g_value_set_string (value, app->priv->info->short_name);
            break;

        case PROP_FULL_NAME:
            g_value_set_string (value, app->priv->info->full_name);
            break;

        case PROP_WINDOW_POLICY:
            g_value_set_flags (value, app->priv->window_policy);
            break;

        case PROP_RUN_PYTHON:
            g_value_set_boolean (value, app->priv->run_python);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


MooApp          *moo_app_get_instance          (void)
{
    return moo_app_instance;
}


static void     moo_app_set_argv        (MooApp         *app,
                                         char          **argv)
{
    g_strfreev (app->priv->argv);
    app->priv->argv = g_strdupv (argv);
    g_object_notify (G_OBJECT (app), "argv");
}


static char   **moo_app_get_argv        (MooApp         *app)
{
    return g_strdupv (app->priv->argv);
}


int              moo_app_get_exit_code         (MooApp      *app)
{
    g_return_val_if_fail (MOO_IS_APP (app), 0);
    return app->priv->exit_code;
}

void             moo_app_set_exit_code         (MooApp      *app,
                                                int          code)
{
    g_return_if_fail (MOO_IS_APP (app));
    app->priv->exit_code = code;
}


const char      *moo_app_get_application_dir   (MooApp      *app)
{
    g_return_val_if_fail (MOO_IS_APP (app), ".");

    if (!app->priv->info->app_dir)
    {
        g_return_val_if_fail (app->priv->argv && app->priv->argv[0], ".");
        app->priv->info->app_dir = g_path_get_dirname (app->priv->argv[0]);
    }

    return app->priv->info->app_dir;
}


const char      *moo_app_get_input_pipe_name   (G_GNUC_UNUSED MooApp *app)
{
#ifdef USE_PYTHON
    g_return_val_if_fail (moo_app_input != NULL, NULL);
    return moo_app_input->pipe_name;
#else /* !USE_PYTHON */
    g_warning ("%s: python support is not compiled in", G_STRLOC);
    return NULL;
#endif /* !USE_PYTHON */
}


const char      *moo_app_get_rc_file_name       (MooApp *app)
{
    g_return_val_if_fail (MOO_IS_APP (app), NULL);

    if (!app->priv->info->rc_file)
    {
#ifdef __WIN32__
        char *basename = g_strdup_printf ("%s.ini",
                                          app->priv->info->short_name);
        app->priv->info->rc_file =
                g_build_filename (g_get_user_config_dir (),
                                  basename,
                                  NULL);
        g_free (basename);
#else
        char *basename = g_strdup_printf (".%src",
                                          app->priv->info->short_name);
        app->priv->info->rc_file =
                g_build_filename (g_get_home_dir (),
                                  basename,
                                  NULL);
        g_free (basename);
#endif
    }

    return app->priv->info->rc_file;
}


void             moo_app_python_execute_file   (GtkWindow *parent_window)
{
#ifdef USE_PYTHON
    GtkWidget *parent;
    const char *filename = NULL;
    FILE *file;

    g_return_if_fail (moo_app_python != NULL);

    parent = parent_window ? GTK_WIDGET (parent_window) : NULL;
    if (!filename)
        filename = moo_file_dialogp (parent,
                                     MOO_DIALOG_FILE_OPEN_EXISTING,
                                     "Choose Python Script to Execute",
                                     "dialogs::python_exec_file", NULL);

    if (!filename) return;

    file = fopen (filename, "r");
    if (!file)
    {
        moo_error_dialog (parent, "Could not open file", NULL);
    }
    else
    {
        PyObject *res = (PyObject*)moo_python_run_file (moo_app_python, file, filename);
        fclose (file);
        if (res)
            Py_XDECREF (res);
        else
            PyErr_Print ();
    }
#else /* !USE_PYTHON */
    g_warning ("%s: python support is not compiled in", G_STRLOC);
#endif /* !USE_PYTHON */
}


gboolean         moo_app_python_run_file       (G_GNUC_UNUSED MooApp      *app,
                                                G_GNUC_UNUSED const char  *filename)
{
#ifdef USE_PYTHON
    FILE *file;
    PyObject *res;

    g_return_val_if_fail (filename != NULL, FALSE);
    g_return_val_if_fail (moo_app_python != NULL, FALSE);

    file = fopen (filename, "r");
    g_return_val_if_fail (file != NULL, FALSE);

    res = (PyObject*)moo_python_run_file (moo_app_python, file, filename);
    fclose (file);
    if (res) {
        Py_XDECREF (res);
        return TRUE;
    }
    else {
        PyErr_Print ();
        return FALSE;
    }
#else /* !USE_PYTHON */
    g_warning ("%s: python support is not compiled in", G_STRLOC);
    return FALSE;
#endif /* !USE_PYTHON */
}

gboolean         moo_app_python_run_string     (G_GNUC_UNUSED MooApp      *app,
                                                G_GNUC_UNUSED const char  *string)
{
#ifdef USE_PYTHON
    PyObject *res;
    g_return_val_if_fail (string != NULL, FALSE);
    g_return_val_if_fail (moo_app_python != NULL, FALSE);
    res = (PyObject*) moo_python_run_string (moo_app_python, string, FALSE);
    if (res) {
        Py_XDECREF (res);
        return TRUE;
    }
    else {
        PyErr_Print ();
        return FALSE;
    }
#else /* !USE_PYTHON */
    g_warning ("%s: python support is not compiled in", G_STRLOC);
    return FALSE;
#endif /* !USE_PYTHON */
}


GtkWidget       *moo_app_get_python_console    (G_GNUC_UNUSED MooApp *app)
{
#ifdef USE_PYTHON
    g_return_val_if_fail (MOO_IS_APP (app), NULL);
    g_return_val_if_fail (moo_app_python != NULL, NULL);
    return GTK_WIDGET (moo_app_python->console);
#else /* !USE_PYTHON */
    g_warning ("%s: python support is not compiled in", G_STRLOC);
    return NULL;
#endif /* !USE_PYTHON */
}

void             moo_app_show_python_console   (G_GNUC_UNUSED MooApp *app)
{
#ifdef USE_PYTHON
    g_return_if_fail (MOO_IS_APP (app));
    g_return_if_fail (moo_app_python != NULL);
    gtk_window_present (GTK_WINDOW (moo_app_python->console));
#else /* !USE_PYTHON */
    g_warning ("%s: python support is not compiled in", G_STRLOC);
#endif /* !USE_PYTHON */
}

void             moo_app_hide_python_console   (G_GNUC_UNUSED MooApp *app)
{
#ifdef USE_PYTHON
    g_return_if_fail (MOO_IS_APP (app));
    g_return_if_fail (moo_app_python != NULL);
    gtk_widget_hide (GTK_WIDGET (moo_app_python->console));
#else /* !USE_PYTHON */
    g_warning ("%s: python support is not compiled in", G_STRLOC);
#endif /* !USE_PYTHON */
}


MooEditor       *moo_app_get_editor            (MooApp          *app)
{
    g_return_val_if_fail (MOO_IS_APP (app), NULL);
    return app->priv->editor;
}


const MooAppInfo*moo_app_get_info               (MooApp     *app)
{
    g_return_val_if_fail (MOO_IS_APP (app), NULL);
    return app->priv->info;
}


static guint strv_length (char **argv)
{
    guint len = 0;
    char **c;

    if (!argv)
        return 0;

    for (c = argv; *c != NULL; ++c)
        ++len;

    return len;
}


static gboolean moo_app_init_real       (MooApp         *app)
{
    G_GNUC_UNUSED const char *app_dir;
    const char *rc_file;
    char *lang_files_dir;
    MooEditLangMgr *mgr;
    MooUIXML *ui_xml;
    MooAppWindowPolicy policy = app->priv->window_policy;

#ifdef __WIN32__
    app_dir = moo_app_get_application_dir (app);
#endif

    rc_file = moo_app_get_rc_file_name (app);
    moo_prefs_load (rc_file);

    ui_xml = moo_app_get_ui_xml (app);

    if (policy & MOO_APP_USE_EDITOR)
    {
#ifdef __WIN32__
        if (app_dir[0])
            lang_files_dir = g_strdup_printf ("%s\\language-specs", app_dir);
        else
            lang_files_dir = g_strdup ("./language-specs");
#else /* !__WIN32__ */
# ifdef MOO_EDIT_LANG_FILES_DIR
        lang_files_dir = g_strdup (MOO_EDIT_LANG_FILES_DIR);
# else /* !MOO_EDIT_LANG_FILES_DIR */
        lang_files_dir = g_strdup (".");
# endif /* !MOO_EDIT_LANG_FILES_DIR */
#endif /* !__WIN32__ */

        app->priv->editor = moo_editor_new ();
        moo_editor_set_ui_xml (app->priv->editor, ui_xml);

        mgr = moo_editor_get_lang_mgr (app->priv->editor);
        moo_edit_lang_mgr_add_lang_files_dir (mgr, lang_files_dir);
        g_free (lang_files_dir);

        g_signal_connect_swapped (app->priv->editor,
                                  "all-windows-closed",
                                  G_CALLBACK (all_editors_closed),
                                  app);
    }

#ifdef __WIN32__
    moo_term_set_helper_directory (app_dir);
    g_message ("app dir: %s", app_dir);
#endif /* __WIN32__ */

    start_python (app);

    return TRUE;
}


static void     start_python            (MooApp         *app)
{
#ifdef USE_PYTHON
    if (app->priv->run_python)
    {
        moo_app_python = moo_python_new ();
        moo_python_start (moo_app_python,
                          strv_length (app->priv->argv),
                          app->priv->argv);
#ifdef USE_PYGTK
        initmoo ();
#endif

        moo_app_input = moo_app_input_new (moo_app_python,
                                           app->priv->info->short_name);
        moo_app_input_start (moo_app_input);
    }
    else
    {
        moo_app_python = moo_python_get_instance ();
        if (moo_app_python)
            g_object_ref (moo_app_python);
    }
#endif /* !USE_PYTHON */
}


static int      moo_app_run_real        (MooApp         *app)
{
    g_return_val_if_fail (!app->priv->running, 0);
    app->priv->running = TRUE;

    gtk_main ();

    return app->priv->exit_code;
}


static gboolean moo_app_try_quit_real   (MooApp         *app)
{
    GSList *l, *list;

    if (!app->priv->running)
        return FALSE;

    list = g_slist_copy (app->priv->terminals);
    for (l = list; l != NULL; l = l->next)
    {
        if (!moo_window_close (MOO_WINDOW (l->data)))
        {
            g_slist_free (list);
            return TRUE;
        }
    }
    g_slist_free (list);

    if (!moo_editor_close_all (app->priv->editor))
        return TRUE;

    return FALSE;
}


static void     moo_app_quit_real       (MooApp         *app)
{
    GSList *l, *list;

    if (!app->priv->running)
        return;
    else
        app->priv->running = FALSE;

#ifdef USE_PYTHON
    if (moo_app_input)
    {
        moo_app_input_shutdown (moo_app_input);
        moo_app_input_unref (moo_app_input);
        moo_app_input = NULL;
    }

    if (moo_app_python)
    {
        moo_python_shutdown (moo_app_python);
        g_object_unref (moo_app_python);
        moo_app_python = NULL;
    }
#endif

    list = g_slist_copy (app->priv->terminals);
    for (l = list; l != NULL; l = l->next)
        moo_window_close (MOO_WINDOW (l->data));
    g_slist_free (list);
    g_slist_free (app->priv->terminals);
    app->priv->terminals = NULL;
    app->priv->term_window = NULL;

    moo_editor_close_all (app->priv->editor);
    g_object_unref (app->priv->editor);
    app->priv->editor = NULL;

    moo_prefs_save (moo_app_get_rc_file_name (app));

    gtk_main_quit ();
}


void             moo_app_init                   (MooApp     *app)
{
    g_return_if_fail (MOO_IS_APP (app));

    MOO_APP_GET_CLASS(app)->init (app);
}


int              moo_app_run                    (MooApp     *app)
{
    g_return_val_if_fail (MOO_IS_APP (app), -1);

    return MOO_APP_GET_CLASS(app)->run (app);
}


gboolean         moo_app_quit                   (MooApp     *app)
{
    gboolean quit;

    g_return_val_if_fail (MOO_IS_APP (app), FALSE);

    g_signal_emit (app, signals[TRY_QUIT], 0, &quit);

    if (!quit)
    {
        MOO_APP_GET_CLASS(app)->quit (app);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


static void     moo_app_set_name        (MooApp         *app,
                                         const char     *short_name,
                                         const char     *full_name)
{
    if (short_name)
    {
        g_free (app->priv->info->short_name);
        app->priv->info->short_name = g_strdup (short_name);
        g_object_notify (G_OBJECT (app), "short_name");
    }

    if (full_name)
    {
        g_free (app->priv->info->full_name);
        app->priv->info->full_name = g_strdup (full_name);
        g_object_notify (G_OBJECT (app), "full_name");
    }
}


static void     moo_app_install_actions (G_GNUC_UNUSED MooApp         *app,
                                         GType           type)
{
    GObjectClass *klass = g_type_class_ref (type);
    char *about, *_about;

    g_return_if_fail (klass != NULL);

    about = g_strdup_printf ("About %s", app->priv->info->full_name);
    _about = g_strdup_printf ("_About %s", app->priv->info->full_name);

    moo_ui_object_class_new_action (klass,
                                    "id", "Quit",
                                    "name", "Quit",
                                    "label", "_Quit",
                                    "tooltip", "Quit",
                                    "icon-stock-id", GTK_STOCK_QUIT,
                                    "accel", "<ctrl>Q",
                                    "closure::callback", moo_app_quit,
                                    "closure::proxy-func", moo_app_get_instance,
                                    NULL);

    moo_ui_object_class_new_action (klass,
                                    "id", "Preferences",
                                    "name", "Preferences",
                                    "label", "Pre_ferences",
                                    "tooltip", "Preferences",
                                    "icon-stock-id", GTK_STOCK_PREFERENCES,
                                    "accel", "<ctrl>P",
                                    "closure::callback", moo_app_prefs_dialog,
                                    NULL);

    moo_ui_object_class_new_action (klass,
                                    "id", "About",
                                    "name", "About",
                                    "label", _about,
                                    "tooltip", about,
                                    "icon-stock-id", GTK_STOCK_ABOUT,
                                    "accel", "",
                                    "closure::callback", moo_app_about_dialog,
                                    NULL);

#ifdef USE_PYTHON
    moo_ui_object_class_new_action (klass,
                                    "id", "PythonMenu",
                                    "name", "Python Menu",
                                    "label", "P_ython",
                                    "visible", TRUE,
                                    "no-accel", TRUE,
                                    NULL);
#else /* !USE_PYTHON */
    moo_ui_object_class_new_action (klass,
                                    "id", "PythonMenu",
                                    "dead", TRUE,
                                    NULL);
#endif /* !USE_PYTHON */

    /* this one can be compiled in since it defined and does nothing when !USE_PYTHON */
    moo_ui_object_class_new_action (klass,
                                    "id", "ExecuteScript",
                                    "name", "Execute Script",
                                    "label", "_Execute Script",
                                    "tooltip", "Execute Script",
                                    "accel", "",
                                    "icon-stock-id", GTK_STOCK_EXECUTE,
                                    "closure::callback", moo_app_python_execute_file,
                                    NULL);

#ifdef USE_PYTHON
    moo_ui_object_class_new_action (klass,
                                    "id", "ShowConsole",
                                    "name", "Show Console",
                                    "label", "Show Conso_le",
                                    "tooltip", "Show Console",
                                    "accel", "<alt>L",
                                    "closure::callback", moo_app_show_python_console,
                                    "closure::proxy-func", moo_app_get_instance,
                                    NULL);
#endif /* USE_PYTHON */

    g_type_class_unref (klass);
    g_free (about);
    g_free (_about);
}


static void     all_editors_closed      (MooApp         *app)
{
    MooAppWindowPolicy policy = app->priv->window_policy;
    gboolean quit = (policy & MOO_APP_QUIT_ON_CLOSE_ALL_EDITORS) ||
            ((policy & MOO_APP_QUIT_ON_CLOSE_ALL_WINDOWS) && !app->priv->terminals);

    if (quit)
        moo_app_quit (app);
}


MooUIXML        *moo_app_get_ui_xml             (MooApp     *app)
{
    g_return_val_if_fail (MOO_IS_APP (app), NULL);

    if (!app->priv->ui_xml)
        app->priv->ui_xml = moo_ui_xml_new ();

    return app->priv->ui_xml;
}


void             moo_app_set_ui_xml             (MooApp     *app,
                                                 MooUIXML   *xml)
{
    GSList *l;

    g_return_if_fail (MOO_IS_APP (app));

    if (app->priv->ui_xml == xml)
        return;

    if (app->priv->ui_xml)
        g_object_unref (app->priv->ui_xml);
    app->priv->ui_xml = xml;
    if (xml)
        g_object_ref (app->priv->ui_xml);

    if (app->priv->editor)
        moo_editor_set_ui_xml (app->priv->editor, xml);
    for (l = app->priv->terminals; l != NULL; l = l->next)
        moo_ui_object_set_ui_xml (MOO_UI_OBJECT (l->data), xml);
}


GType            moo_app_window_policy_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GFlagsValue values[] = {
            { MOO_APP_ONE_EDITOR, (char*)"MOO_APP_ONE_EDITOR", (char*)"one-editor" },
            { MOO_APP_MANY_EDITORS, (char*)"MOO_APP_MANY_EDITORS", (char*)"many-editors" },
            { MOO_APP_USE_EDITOR, (char*)"MOO_APP_USE_EDITOR", (char*)"use-editor" },
            { MOO_APP_ONE_TERMINAL, (char*)"MOO_APP_ONE_TERMINAL", (char*)"one-terminal" },
            { MOO_APP_MANY_TERMINALS, (char*)"MOO_APP_MANY_TERMINALS", (char*)"many-terminals" },
            { MOO_APP_USE_TERMINAL, (char*)"MOO_APP_USE_TERMINAL", (char*)"use-terminal" },
            { MOO_APP_QUIT_ON_CLOSE_ALL_EDITORS, (char*)"MOO_APP_QUIT_ON_CLOSE_ALL_EDITORS", (char*)"quit-on-close-all-editors" },
            { MOO_APP_QUIT_ON_CLOSE_ALL_TERMINALS, (char*)"MOO_APP_QUIT_ON_CLOSE_ALL_TERMINALS", (char*)"quit-on-close-all-terminals" },
            { MOO_APP_QUIT_ON_CLOSE_ALL_WINDOWS, (char*)"MOO_APP_QUIT_ON_CLOSE_ALL_WINDOWS", (char*)"quit-on-close-all-windows" },
            { 0, NULL, NULL }
        };
        type = g_flags_register_static ("MooAppWindowPolicy", values);
    }

    return type;
}


static MooAppInfo  *moo_app_info_copy   (MooAppInfo *info)
{
    MooAppInfo *copy;

    g_return_val_if_fail (info != NULL, NULL);

    copy = g_new (MooAppInfo, 1);

    copy->short_name = g_strdup (info->short_name);
    copy->full_name = g_strdup (info->full_name);
    copy->description = g_strdup (info->description);
    copy->version = g_strdup (info->version);
    copy->website = g_strdup (info->website);
    copy->website_label = g_strdup (info->website_label);
    copy->app_dir = g_strdup (info->app_dir);
    copy->rc_file = g_strdup (info->rc_file);

    return copy;
}


static void         moo_app_info_free   (MooAppInfo *info)
{
    if (!info)
        return;

    g_free (info->short_name);
    g_free (info->full_name);
    g_free (info->description);
    g_free (info->version);
    g_free (info->website);
    g_free (info->website_label);
    g_free (info->app_dir);
    g_free (info->rc_file);

    g_free (info);
}


GType            moo_app_info_get_type          (void)
{
    static GType type = 0;
    if (!type)
        g_boxed_type_register_static ("MooAppInfo",
                                      (GBoxedCopyFunc) moo_app_info_copy,
                                      (GBoxedFreeFunc) moo_app_info_free);
    return type;
}
