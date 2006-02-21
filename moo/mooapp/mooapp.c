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
#endif

#define WANT_MOO_APP_CMD_CHARS
#include "mooapp/mooapp-private.h"
#include "mooapp/mooappinput.h"
#include "mooapp/mooappoutput.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mooeditor.h"
#include "mooedit/mooplugin.h"
#include "mooedit/plugins/mooeditplugins.h"
#include "mooutils/moopython.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moocompat.h"
#include "mooutils/moodialogs.h"
#include "mooutils/moostock.h"
#include "mooutils/mooutils-misc.h"
#include <string.h>


#ifdef VERSION
#define APP_VERSION VERSION
#else
#define APP_VERSION "<uknown version>"
#endif


static MooApp *moo_app_instance = NULL;
static MooAppInput *moo_app_input = NULL;
static MooAppOutput *moo_app_output = NULL;


struct _MooAppPrivate {
    char      **argv;
    int         exit_code;
    MooEditor  *editor;
    MooAppInfo *info;
    gboolean    run_python;
    gboolean    run_input;
    gboolean    run_output;

    gboolean    running;
    gboolean    in_try_quit;

    GType       term_window_type;
    GSList     *terminals;
    MooTermWindow *term_window;

    MooUIXML   *ui_xml;
    guint       quit_handler_id;
    gboolean    use_editor;
    gboolean    use_terminal;

    char       *tmpdir;

    gboolean    new_app;
    char      **open_files;
};


static void     moo_app_class_init      (MooAppClass        *klass);
static void     moo_app_instance_init   (MooApp             *app);
static GObject *moo_app_constructor     (GType               type,
                                         guint               n_params,
                                         GObjectConstructParam *params);
static void     moo_app_finalize        (GObject            *object);

static MooAppInfo *moo_app_info_new     (void);
static MooAppInfo *moo_app_info_copy    (const MooAppInfo   *info);
static void     moo_app_info_free       (MooAppInfo         *info);

static void     install_actions         (MooApp             *app,
                                         GType               type);
static void     install_editor_actions  (MooApp             *app);
static void     install_terminal_actions(MooApp             *app);

static void     moo_app_set_property    (GObject            *object,
                                         guint               prop_id,
                                         const GValue       *value,
                                         GParamSpec         *pspec);
static void     moo_app_get_property    (GObject            *object,
                                         guint               prop_id,
                                         GValue             *value,
                                         GParamSpec         *pspec);

static void     moo_app_set_argv        (MooApp             *app,
                                         char              **argv);

static gboolean moo_app_init_real       (MooApp             *app);
static int      moo_app_run_real        (MooApp             *app);
static void     moo_app_quit_real       (MooApp             *app);
static gboolean moo_app_try_quit_real   (MooApp             *app);
static void     moo_app_exec_cmd_real   (MooApp             *app,
                                         char                cmd,
                                         const char         *data,
                                         guint               len);

static void     moo_app_set_name        (MooApp             *app,
                                         const char         *short_name,
                                         const char         *full_name);
static void     moo_app_set_description (MooApp             *app,
                                         const char         *description);

static void     start_io                (MooApp             *app);
static void     execute_selection       (MooEditWindow      *window);


static GObjectClass *moo_app_parent_class;

GType
moo_app_get_type (void)
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
            (GInstanceInitFunc) moo_app_instance_init,
            NULL    /* value_table */
        };

        type = g_type_register_static (G_TYPE_OBJECT, "MooApp",
                                       &type_info, 0);
    }

    return type;
}


enum {
    PROP_0,
    PROP_ARGV,
    PROP_SHORT_NAME,
    PROP_FULL_NAME,
    PROP_DESCRIPTION,
    PROP_RUN_PYTHON,
    PROP_RUN_INPUT,
    PROP_RUN_OUTPUT,
    PROP_USE_EDITOR,
    PROP_USE_TERMINAL,
    PROP_OPEN_FILES,
    PROP_NEW_APP
};

enum {
    INIT,
    RUN,
    QUIT,
    TRY_QUIT,
    PREFS_DIALOG,
    EXEC_CMD,
    LAST_SIGNAL
};


static guint signals[LAST_SIGNAL];


static void
moo_app_class_init (MooAppClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    moo_create_stock_items ();

    moo_app_parent_class = g_type_class_peek_parent (klass);

    gobject_class->constructor = moo_app_constructor;
    gobject_class->finalize = moo_app_finalize;
    gobject_class->set_property = moo_app_set_property;
    gobject_class->get_property = moo_app_get_property;

    klass->init = moo_app_init_real;
    klass->run = moo_app_run_real;
    klass->quit = moo_app_quit_real;
    klass->try_quit = moo_app_try_quit_real;
    klass->prefs_dialog = _moo_app_create_prefs_dialog;
    klass->exec_cmd = moo_app_exec_cmd_real;

    g_object_class_install_property (gobject_class,
                                     PROP_ARGV,
                                     g_param_spec_pointer ("argv",
                                             "argv",
                                             "Null-terminated array of application arguments",
                                             G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_SHORT_NAME,
                                     g_param_spec_string ("short-name",
                                             "short-name",
                                             "short-name",
                                             "ggap",
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_FULL_NAME,
                                     g_param_spec_string ("full-name",
                                             "full-name",
                                             "full-name",
                                             NULL,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_DESCRIPTION,
                                     g_param_spec_string ("description",
                                             "description",
                                             "description",
                                             NULL,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_RUN_PYTHON,
                                     g_param_spec_boolean ("run-python",
                                             "run-python",
                                             "run-python",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_RUN_INPUT,
                                     g_param_spec_boolean ("run-input",
                                             "run-input",
                                             "run-input",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_RUN_OUTPUT,
                                     g_param_spec_boolean ("run-output",
                                             "run-output",
                                             "run-output",
                                             FALSE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_USE_EDITOR,
                                     g_param_spec_boolean ("use-editor",
                                             "use-editor",
                                             "use-editor",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_USE_TERMINAL,
                                     g_param_spec_boolean ("use-terminal",
                                             "use-terminal",
                                             "use-terminal",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_OPEN_FILES,
                                     g_param_spec_pointer ("open-files",
                                             "open-files",
                                             "open-files",
                                             G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_NEW_APP,
                                     g_param_spec_boolean ("new-app",
                                             "new-app",
                                             "new-app",
                                             FALSE,
                                             G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

    signals[INIT] =
            g_signal_new ("init",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooAppClass, init),
                          NULL, NULL,
                          _moo_marshal_BOOLEAN__VOID,
                          G_TYPE_BOOLEAN, 0);

    signals[RUN] =
            g_signal_new ("run",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooAppClass, run),
                          NULL, NULL,
                          _moo_marshal_INT__VOID,
                          G_TYPE_INT, 0);

    signals[QUIT] =
            g_signal_new ("quit",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooAppClass, quit),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[TRY_QUIT] =
            g_signal_new ("try-quit",
                          G_OBJECT_CLASS_TYPE (klass),
                          (GSignalFlags) (G_SIGNAL_ACTION | G_SIGNAL_RUN_LAST),
                          G_STRUCT_OFFSET (MooAppClass, try_quit),
                          g_signal_accumulator_true_handled, NULL,
                          _moo_marshal_BOOLEAN__VOID,
                          G_TYPE_BOOLEAN, 0);

    signals[PREFS_DIALOG] =
            g_signal_new ("prefs-dialog",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooAppClass, quit),
                          NULL, NULL,
                          _moo_marshal_OBJECT__VOID,
                          MOO_TYPE_PREFS_DIALOG, 0);

    signals[EXEC_CMD] =
            g_signal_new ("exec-cmd",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_ACTION | G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooAppClass, exec_cmd),
                          NULL, NULL,
                          _moo_marshal_VOID__CHAR_STRING_UINT,
                          G_TYPE_NONE, 3,
                          G_TYPE_CHAR,
                          G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE,
                          G_TYPE_UINT);
}


static void
moo_app_instance_init (MooApp *app)
{
    g_return_if_fail (moo_app_instance == NULL);

    moo_app_instance = app;

    app->priv = g_new0 (MooAppPrivate, 1);
    app->priv->info = moo_app_info_new ();

    app->priv->info->version = g_strdup (APP_VERSION);
    app->priv->info->website = g_strdup ("http://ggap.berlios.de/");
    app->priv->info->website_label = g_strdup ("http://ggap.berlios.de");
}


static GObject*
moo_app_constructor (GType           type,
                     guint           n_params,
                     GObjectConstructParam *params)
{
    GObject *object;
    MooApp *app;

    if (moo_app_instance != NULL)
    {
        g_critical ("attempt to create second instance of application class");
        g_critical ("crash");
        return NULL;
    }

    object = moo_app_parent_class->constructor (type, n_params, params);
    app = MOO_APP (object);

    if (!app->priv->info->full_name)
        app->priv->info->full_name = g_strdup (app->priv->info->short_name);

    install_editor_actions (app);
    install_terminal_actions (app);

    return object;
}


static void
moo_app_finalize (GObject *object)
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


static void
moo_app_set_property (GObject        *object,
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

        case PROP_DESCRIPTION:
            moo_app_set_description (app, g_value_get_string (value));
            break;

        case PROP_RUN_PYTHON:
            app->priv->run_python = g_value_get_boolean (value);
            break;

        case PROP_RUN_INPUT:
            app->priv->run_input = g_value_get_boolean (value);
            break;

        case PROP_RUN_OUTPUT:
            app->priv->run_output = g_value_get_boolean (value);
            break;

        case PROP_USE_EDITOR:
            app->priv->use_editor = g_value_get_boolean (value);
            break;

        case PROP_USE_TERMINAL:
            app->priv->use_terminal = g_value_get_boolean (value);
            break;

        case PROP_NEW_APP:
            app->priv->new_app = g_value_get_boolean (value);
            break;

        case PROP_OPEN_FILES:
            g_strfreev (app->priv->open_files);
            app->priv->open_files = g_strdupv (g_value_get_pointer (value));
            if (app->priv->open_files && !*app->priv->open_files)
            {
                g_strfreev (app->priv->open_files);
                app->priv->open_files = NULL;
            }
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
moo_app_get_property (GObject        *object,
                      guint           prop_id,
                      GValue         *value,
                      GParamSpec     *pspec)
{
    MooApp *app = MOO_APP (object);

    switch (prop_id)
    {
        case PROP_SHORT_NAME:
            g_value_set_string (value, app->priv->info->short_name);
            break;

        case PROP_FULL_NAME:
            g_value_set_string (value, app->priv->info->full_name);
            break;

        case PROP_DESCRIPTION:
            g_value_set_string (value, app->priv->info->description);
            break;

        case PROP_RUN_PYTHON:
            g_value_set_boolean (value, app->priv->run_python);
            break;

        case PROP_RUN_INPUT:
            g_value_set_boolean (value, app->priv->run_input);
            break;

        case PROP_RUN_OUTPUT:
            g_value_set_boolean (value, app->priv->run_output);
            break;

        case PROP_USE_EDITOR:
            g_value_set_boolean (value, app->priv->use_editor);
            break;

        case PROP_USE_TERMINAL:
            g_value_set_boolean (value, app->priv->use_terminal);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


MooApp*
moo_app_get_instance (void)
{
    return moo_app_instance;
}


static void
moo_app_set_argv (MooApp         *app,
                  char          **argv)
{
    g_strfreev (app->priv->argv);
    app->priv->argv = g_strdupv (argv);
    g_object_notify (G_OBJECT (app), "argv");
}


int
moo_app_get_exit_code (MooApp      *app)
{
    g_return_val_if_fail (MOO_IS_APP (app), -1);
    return app->priv->exit_code;
}

void
moo_app_set_exit_code (MooApp      *app,
                       int          code)
{
    g_return_if_fail (MOO_IS_APP (app));
    app->priv->exit_code = code;
}


const char*
moo_app_get_application_dir (MooApp      *app)
{
    g_return_val_if_fail (MOO_IS_APP (app), ".");

    if (!app->priv->info->app_dir)
    {
        g_return_val_if_fail (app->priv->argv && app->priv->argv[0], ".");
        app->priv->info->app_dir = g_path_get_dirname (app->priv->argv[0]);
    }

    return app->priv->info->app_dir;
}


const char*
moo_app_get_input_pipe_name (G_GNUC_UNUSED MooApp *app)
{
    return moo_app_input ? moo_app_input->pipe_name : NULL;
}


const char*
moo_app_get_output_pipe_name (G_GNUC_UNUSED MooApp *app)
{
    return moo_app_output ? moo_app_output->pipe_name : NULL;
}


const char*
moo_app_get_rc_file_name (MooApp *app)
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


void
moo_app_python_execute_file (G_GNUC_UNUSED GtkWindow *parent_window)
{
    GtkWidget *parent;
    const char *filename = NULL;
    FILE *file;

    g_return_if_fail (moo_python_running ());

    parent = parent_window ? GTK_WIDGET (parent_window) : NULL;
    if (!filename)
        filename = moo_file_dialogp (parent,
                                     MOO_DIALOG_FILE_OPEN_EXISTING,
                                     "Choose Python Script to Execute",
                                     "python_exec_file", NULL);

    if (!filename)
        return;

    file = fopen (filename, "r");

    if (!file)
    {
        moo_error_dialog (parent, "Could not open file", NULL);
    }
    else
    {
        MooPyObject *res = moo_python_run_file (file, filename);

        fclose (file);

        if (res)
            moo_Py_DECREF (res);
        else
            moo_PyErr_Print ();
    }
}


gboolean
moo_app_python_run_file (MooApp      *app,
                         const char  *filename)
{
    FILE *file;
    MooPyObject *res;

    g_return_val_if_fail (MOO_IS_APP (app), FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);
    g_return_val_if_fail (moo_python_running (), FALSE);

    file = fopen (filename, "r");
    g_return_val_if_fail (file != NULL, FALSE);

    res = moo_python_run_file (file, filename);

    fclose (file);

    if (res)
    {
        moo_Py_DECREF (res);
        return TRUE;
    }
    else
    {
        moo_PyErr_Print ();
        return FALSE;
    }
}


gboolean
moo_app_python_run_string (MooApp      *app,
                           const char  *string)
{
    MooPyObject *res;

    g_return_val_if_fail (MOO_IS_APP (app), FALSE);
    g_return_val_if_fail (string != NULL, FALSE);
    g_return_val_if_fail (moo_python_running (), FALSE);

    res = moo_python_run_string (string);

    if (res)
    {
        moo_Py_DECREF (res);
        return TRUE;
    }
    else
    {
        moo_PyErr_Print ();
        return FALSE;
    }
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


static char *
moo_app_get_user_data_dir (MooApp *app)
{
    char *basename = g_strdup_printf (".%s", app->priv->info->short_name);
    char *dir = g_build_filename (g_get_home_dir (), basename, NULL);
    g_free (basename);
    return dir;
}


static char **
moo_app_get_plugin_dirs (MooApp *app)
{
    char **dirs;

#ifdef __WIN32__

    const char *data_dir;
    const char *app_dir = moo_app_get_application_dir (app);

    if (app_dir[0])
        data_dir = app_dir;
    else
        data_dir = ".";

    dirs = g_new0 (char*, 2);
    dirs[0] = g_build_filename (data_dir, MOO_PLUGIN_DIR_BASENAME, NULL);

#else /* !__WIN32__ */

    char *user_data_dir;

    dirs = g_new0 (char*, 3);

#ifdef MOO_PLUGINS_DIR
    dirs[0] = g_strdup (MOO_PLUGINS_DIR);
#else
    dirs[0] = g_build_filename (".", MOO_PLUGIN_DIR_BASENAME, NULL);
#endif

    user_data_dir = moo_app_get_user_data_dir (app);
    dirs[1] = g_build_filename (user_data_dir, MOO_PLUGIN_DIR_BASENAME, NULL);
    g_free (user_data_dir);

#endif /* !__WIN32__ */

    return dirs;
}


static gboolean
moo_app_init_real (MooApp *app)
{
    G_GNUC_UNUSED const char *app_dir;
    const char *rc_file;
    MooLangMgr *lang_mgr;
    MooUIXML *ui_xml;
    GError *error = NULL;

    if (!app->priv->new_app)
    {
        char **p;
        GString *msg = g_string_new (NULL);

        if (!app->priv->open_files || !*(app->priv->open_files))
            g_string_append_len (msg, CMD_PRESENT, strlen (CMD_PRESENT) + 1);

        for (p = app->priv->open_files; p && *p; ++p)
        {
            char *freeme = NULL;
            const char *basename, *filename;

            basename = *p;

            if (g_path_is_absolute (basename))
            {
                filename = basename;
            }
            else
            {
                char *dir = g_get_current_dir ();
                freeme = g_build_filename (dir, basename, NULL);
                filename = freeme;
                g_free (dir);
            }

            g_string_append_len (msg, CMD_OPEN_FILE, strlen (CMD_OPEN_FILE));
            g_string_append_len (msg, filename, strlen (filename) + 1);

            g_free (freeme);
        }

        if (moo_app_send_msg (app, msg->str, msg->len))
        {
            g_string_free (msg, TRUE);
            goto exit;
        }

        g_string_free (msg, TRUE);
    }

    app->priv->new_app = TRUE;
    gdk_set_program_class (app->priv->info->full_name);

#ifdef __WIN32__
    app_dir = moo_app_get_application_dir (app);
#endif

    rc_file = moo_app_get_rc_file_name (app);

    if (!moo_prefs_load (rc_file, &error))
    {
        g_warning ("%s: could not read config file", G_STRLOC);
        if (error)
        {
            g_warning ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }
    }

    ui_xml = moo_app_get_ui_xml (app);

    if (app->priv->use_editor)
    {
        char *lang_dir = NULL, *user_lang_dir = NULL;
        char *user_data_dir = NULL;
        char **plugin_dirs;

#ifdef __WIN32__

        const char *data_dir;

        if (app_dir[0])
            data_dir = app_dir;
        else
            data_dir = ".";

        lang_dir = g_build_filename (data_dir, MOO_LANG_DIR_BASENAME, NULL);

#else /* !__WIN32__ */

#ifdef MOO_TEXT_LANG_FILES_DIR
        lang_dir = g_strdup (MOO_TEXT_LANG_FILES_DIR);
#else
        lang_dir = g_build_filename (".", MOO_LANG_DIR_BASENAME, NULL);
#endif

        user_data_dir = moo_app_get_user_data_dir (app);
        user_lang_dir = g_build_filename (user_data_dir, MOO_LANG_DIR_BASENAME, NULL);

#endif /* !__WIN32__ */

        app->priv->editor = moo_editor_instance ();
        moo_editor_set_ui_xml (app->priv->editor, ui_xml);
        moo_editor_set_app_name (app->priv->editor,
                                 app->priv->info->short_name);

        lang_mgr = moo_editor_get_lang_mgr (app->priv->editor);
        moo_lang_mgr_add_dir (lang_mgr, lang_dir);

        if (user_lang_dir)
            moo_lang_mgr_add_dir (lang_mgr, user_lang_dir);

        moo_lang_mgr_read_dirs (lang_mgr);

        plugin_dirs = moo_app_get_plugin_dirs (app);
        moo_set_plugin_dirs (plugin_dirs);
        moo_plugin_init_builtin ();
        moo_plugin_read_dirs ();

        g_strfreev (plugin_dirs);
        g_free (lang_dir);
        g_free (user_lang_dir);
        g_free (user_data_dir);
    }

#if defined(__WIN32__) && defined(MOO_BUILD_TERM)
    if (app->priv->use_terminal)
    {
        moo_term_set_helper_directory (app_dir);
        g_message ("app dir: %s", app_dir);
    }
#endif /* __WIN32__ && MOO_BUILD_TERM */

    start_io (app);

    return TRUE;

exit:
    app->priv->new_app = FALSE;
    return FALSE;
}


// static void
// add_python_plugin_actions (MooApp *app)
// {
//     MooUIXML *xml;
//     MooWindowClass *klass;
//
//     klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);
//     moo_window_class_new_action (klass, "ReloadPythonPlugins",
//                                  "name", "Reload Python Plugins",
//                                  "label", "Reload Python Plugins",
//                                  "icon-stock-id", GTK_STOCK_REFRESH,
//                                  "closure-callback", reload_python_plugins,
//                                  NULL);
//
//     xml = moo_app_get_ui_xml (app);
//     moo_ui_xml_add_item (xml, moo_ui_xml_new_merge_id (xml),
//                          "ToolsMenu", "ReloadPythonPlugins",
//                          "ReloadPythonPlugins", -1);
//
//     g_type_class_unref (klass);
// }


static void
start_io (MooApp *app)
{
    if (app->priv->run_input)
    {
        moo_app_input = moo_app_input_new (app->priv->info->short_name);
        moo_app_input_start (moo_app_input);
    }

    if (app->priv->run_output)
    {
        moo_app_output = moo_app_output_new (app->priv->info->short_name);
        moo_app_output_start (moo_app_output);
    }
}


gboolean
moo_app_send_msg (MooApp     *app,
                  const char *data,
                  int         len)
{
    g_return_val_if_fail (MOO_IS_APP (app), FALSE);
    g_return_val_if_fail (data != NULL, FALSE);
    return _moo_app_input_send_msg (app->priv->info->short_name, data, len);
}


static gboolean on_gtk_main_quit (MooApp *app)
{
    app->priv->quit_handler_id = 0;

    if (!moo_app_quit (app))
        MOO_APP_GET_CLASS(app)->quit (app);

    return FALSE;
}


static int
moo_app_run_real (MooApp *app)
{
    g_return_val_if_fail (!app->priv->running, 0);
    app->priv->running = TRUE;

    if (!app->priv->new_app)
        return 0;

    app->priv->quit_handler_id =
            gtk_quit_add (0, (GtkFunction) on_gtk_main_quit, app);

    if (app->priv->open_files)
    {
        char **file;
        MooEditor *editor;
        MooEditWindow *window;

        editor = moo_app_get_editor (app);
        window = moo_editor_get_active_window (editor);

        if (!window)
            window = moo_editor_new_window (editor);

        for (file = app->priv->open_files; file && *file; ++file)
            moo_editor_open_file (editor, window, NULL, *file, NULL);

        g_strfreev (app->priv->open_files);
        app->priv->open_files = NULL;
    }

    gtk_main ();

    return app->priv->exit_code;
}


static gboolean
moo_app_try_quit_real (MooApp *app)
{
    GSList *l, *list;

    if (!app->priv->running)
        return FALSE;

    if (!moo_editor_close_all (app->priv->editor, TRUE))
        return TRUE;

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

    return FALSE;
}


static void     moo_app_quit_real       (MooApp         *app)
{
    GSList *l, *list;
    GError *error = NULL;

    if (!app->priv->running)
        return;
    else
        app->priv->running = FALSE;

    if (moo_app_input)
    {
        moo_app_input_shutdown (moo_app_input);
        moo_app_input_unref (moo_app_input);
        moo_app_input = NULL;
    }

    if (moo_app_output)
    {
        moo_app_output_shutdown (moo_app_output);
        moo_app_output_unref (moo_app_output);
        moo_app_output = NULL;
    }

    list = g_slist_copy (app->priv->terminals);
    for (l = list; l != NULL; l = l->next)
        moo_window_close (MOO_WINDOW (l->data));
    g_slist_free (list);
    g_slist_free (app->priv->terminals);
    app->priv->terminals = NULL;
    app->priv->term_window = NULL;

    moo_editor_close_all (app->priv->editor, TRUE);
    g_object_unref (app->priv->editor);
    app->priv->editor = NULL;

    if (!moo_prefs_save (moo_app_get_rc_file_name (app), &error))
    {
        g_warning ("%s: could not save config file", G_STRLOC);
        if (error)
        {
            g_warning ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }
    }

    if (app->priv->quit_handler_id)
        gtk_quit_remove (app->priv->quit_handler_id);

    gtk_main_quit ();

    if (app->priv->tmpdir)
    {
        moo_rmdir (app->priv->tmpdir, TRUE);
        g_free (app->priv->tmpdir);
        app->priv->tmpdir = NULL;
    }
}


gboolean
moo_app_init (MooApp *app)
{
    gboolean retval;
    g_return_val_if_fail (MOO_IS_APP (app), FALSE);
    g_signal_emit (app, signals[INIT], 0, &retval);
    return retval;
}


int
moo_app_run (MooApp *app)
{
    int retval;
    g_return_val_if_fail (MOO_IS_APP (app), -1);
    g_signal_emit (app, signals[RUN], 0, &retval);
    return retval;
}


gboolean         moo_app_quit                   (MooApp     *app)
{
    gboolean stopped = FALSE;

    g_return_val_if_fail (MOO_IS_APP (app), FALSE);

    if (app->priv->in_try_quit || !app->priv->running)
        return TRUE;

    app->priv->in_try_quit = TRUE;
    g_signal_emit (app, signals[TRY_QUIT], 0, &stopped);
    app->priv->in_try_quit = FALSE;

    if (!stopped)
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


static void     moo_app_set_description (MooApp         *app,
                                         const char     *description)
{
    g_free (app->priv->info->description);
    app->priv->info->description = g_strdup (description);
    g_object_notify (G_OBJECT (app), "description");
}


static void install_actions (MooApp *app, GType  type)
{
    MooWindowClass *klass = g_type_class_ref (type);
    char *about, *_about;

    g_return_if_fail (klass != NULL);

    about = g_strdup_printf ("About %s", app->priv->info->full_name);
    _about = g_strdup_printf ("_About %s", app->priv->info->full_name);

    moo_window_class_new_action (klass, "Quit",
                                 "name", "Quit",
                                 "label", "_Quit",
                                 "tooltip", "Quit",
                                 "icon-stock-id", GTK_STOCK_QUIT,
                                 "accel", "<ctrl>Q",
                                 "closure-callback", moo_app_quit,
                                 "closure-proxy-func", moo_app_get_instance,
                                 NULL);

    moo_window_class_new_action (klass, "Preferences",
                                 "name", "Preferences",
                                 "label", "Pre_ferences",
                                 "tooltip", "Preferences",
                                 "icon-stock-id", GTK_STOCK_PREFERENCES,
                                 "accel", "<ctrl>P",
                                 "closure-callback", moo_app_prefs_dialog,
                                 NULL);

    moo_window_class_new_action (klass, "About",
                                 "name", "About",
                                 "label", _about,
                                 "tooltip", about,
                                 "icon-stock-id", GTK_STOCK_ABOUT,
                                 "closure-callback", moo_app_about_dialog,
                                 NULL);

    g_type_class_unref (klass);
    g_free (about);
    g_free (_about);
}


static void install_editor_actions  (MooApp *app)
{
    MooWindowClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);

    g_return_if_fail (klass != NULL);

    install_actions (app, MOO_TYPE_EDIT_WINDOW);

    moo_window_class_new_action (klass, "ExecuteSelection",
                                 "name", "Execute Selection",
                                 "label", "_Execute Selection",
                                 "tooltip", "Execute Selection",
                                 "icon-stock-id", GTK_STOCK_EXECUTE,
                                 "accel", "<shift><alt>Return",
                                 "closure-callback", execute_selection,
                                 NULL);

    g_type_class_unref (klass);
}


#ifdef MOO_BUILD_TERM
static void new_editor (MooApp *app)
{
    g_return_if_fail (app != NULL);
    gtk_window_present (GTK_WINDOW (moo_editor_new_window (app->priv->editor)));
}

static void open_in_editor (MooTermWindow *terminal)
{
    MooApp *app = moo_app_get_instance ();
    g_return_if_fail (app != NULL);
    moo_editor_open (app->priv->editor, NULL, GTK_WIDGET (terminal), NULL);
}


static void install_terminal_actions (MooApp *app)
{
    MooWindowClass *klass = g_type_class_ref (MOO_TYPE_TERM_WINDOW);

    g_return_if_fail (klass != NULL);

    install_actions (app, MOO_TYPE_TERM_WINDOW);

    moo_window_class_new_action (klass, "NewEditor",
                                 "name", "New Editor",
                                 "label", "_New Editor",
                                 "tooltip", "New Editor",
                                 "icon-stock-id", GTK_STOCK_EDIT,
                                 "accel", "<Alt>E",
                                 "closure-callback", new_editor,
                                 "closure-proxy-func", moo_app_get_instance,
                                 NULL);

    moo_window_class_new_action (klass, "OpenInEditor",
                                 "name", "Open In Editor",
                                 "label", "_Open In Editor",
                                 "tooltip", "Open In Editor",
                                 "icon-stock-id", GTK_STOCK_OPEN,
                                 "accel", "<Alt>O",
                                 "closure-callback", open_in_editor,
                                 NULL);

    g_type_class_unref (klass);
}
#else /* !MOO_BUILD_TERM */
static void install_terminal_actions (G_GNUC_UNUSED MooApp *app)
{
}
#endif /* !MOO_BUILD_TERM */


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
        moo_window_set_ui_xml (l->data, xml);
}


static MooAppInfo*
moo_app_info_new (void)
{
    return g_new0 (MooAppInfo, 1);
}


static MooAppInfo*
moo_app_info_copy (const MooAppInfo *info)
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


static void
moo_app_info_free (MooAppInfo *info)
{
    if (info)
    {
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


static void     execute_selection       (MooEditWindow  *window)
{
    MooEdit *edit;
    char *text;

    edit = moo_edit_window_get_active_doc (window);

    g_return_if_fail (edit != NULL);

    text = moo_text_view_get_selection (MOO_TEXT_VIEW (edit));

    if (!text)
        text = moo_text_view_get_text (MOO_TEXT_VIEW (edit));

    if (text)
    {
        moo_app_python_run_string (moo_app_get_instance (), text);
        g_free (text);
    }
}


void
_moo_app_exec_cmd (MooApp     *app,
                   char        cmd,
                   const char *data,
                   guint       len)
{
    g_return_if_fail (MOO_IS_APP (app));
    g_return_if_fail (data != NULL);
    g_signal_emit (app, signals[EXEC_CMD], 0, cmd, data, len);
}


static void
moo_app_open_file (MooApp       *app,
                   const char   *filename)
{
    MooEditor *editor = moo_app_get_editor (app);
    g_return_if_fail (editor != NULL);
    moo_editor_open_file (editor, NULL, NULL, filename, NULL);
}


static void
moo_app_present (MooApp *app)
{
    gpointer window = app->priv->term_window;

    if (!window && app->priv->editor)
        window = moo_editor_get_active_window (app->priv->editor);

    g_return_if_fail (window != NULL);

    moo_window_present (window);
}


static MooAppCmdCode get_cmd_code (char cmd)
{
    guint i;

    for (i = 1; i < MOO_APP_CMD_LAST; ++i)
        if (cmd == moo_app_cmd_chars[i])
            return i;

    return MOO_APP_CMD_ZERO;
}

static void
moo_app_exec_cmd_real (MooApp             *app,
                       char                cmd,
                       const char         *data,
                       G_GNUC_UNUSED guint len)
{
    MooAppCmdCode code;

    g_return_if_fail (MOO_IS_APP (app));

    code = get_cmd_code (cmd);

    switch (code)
    {
        case MOO_APP_CMD_PYTHON_STRING:
            moo_app_python_run_string (app, data);
            break;
        case MOO_APP_CMD_PYTHON_FILE:
            moo_app_python_run_file (app, data);
            break;

        case MOO_APP_CMD_OPEN_FILE:
            moo_app_open_file (app, data);
            break;
        case MOO_APP_CMD_QUIT:
            moo_app_quit (app);
            break;
        case MOO_APP_CMD_DIE:
            MOO_APP_GET_CLASS(app)->quit (app);
            break;

        case MOO_APP_CMD_PRESENT:
            moo_app_present (app);
            break;

        default:
            g_warning ("%s: got unknown command %d", G_STRLOC, cmd);
    }
}


char*
moo_app_tempnam (MooApp     *app)
{
    int i;
    char *basename;
    char *filename;

    g_return_val_if_fail (MOO_IS_APP (app), NULL);

    if (!app->priv->tmpdir)
    {
        char *dirname = NULL;

        for (i = 0; i < 1000; ++i)
        {
            basename = g_strdup_printf ("%s-%08x",
                                        app->priv->info->short_name,
                                        g_random_int ());
            dirname = g_build_filename (g_get_tmp_dir (), basename, NULL);
            g_free (basename);

            if (moo_mkdir (dirname))
            {
                g_free (dirname);
                dirname = NULL;
            }
            else
            {
                break;
            }
        }

        g_return_val_if_fail (dirname != NULL, NULL);
        app->priv->tmpdir = dirname;
    }

    for (i = 0; i < 1000; ++i)
    {
        basename = g_strdup_printf ("%08x.tmp", g_random_int ());
        filename = g_build_filename (app->priv->tmpdir, basename, NULL);
        g_free (basename);

        if (g_file_test (filename, G_FILE_TEST_EXISTS))
            g_free (filename);
        else
            return filename;
    }

    g_warning ("%s: could not generate temp file name", G_STRLOC);
    return NULL;
}
