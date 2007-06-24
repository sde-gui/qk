/*
 *   mooapp/mooapp.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define MOO_APP_COMPILATION
#define WANT_MOO_APP_CMD_CHARS
#include "mooapp/mooappinput.h"
#include "mooapp/mooapp-private.h"
#include "mooapp/smclient/eggsmclient.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mooeditor.h"
#include "mooedit/mooplugin.h"
#include "mooedit/mooedit-script.h"
#include "mooedit/moousertools.h"
#include "mooedit/moousertools-prefs.h"
#include "mooedit/plugins/mooeditplugins.h"
#include "mooutils/mooprefsdialog.h"
#include "mooutils/moopython.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moodialogs.h"
#include "mooutils/moostock.h"
#include "mooutils/mooutils-fs.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooi18n.h"
#include "mooutils/xdgmime/xdgmime.h"
#include <glib/gmappedfile.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef VERSION
#define APP_VERSION VERSION
#else
#define APP_VERSION "<uknown version>"
#endif

#define MOO_UI_XML_FILE     "ui.xml"
#ifdef __WIN32__
#define MOO_ACTIONS_FILE    "actions.ini"
#else
#define MOO_ACTIONS_FILE    "actions"
#endif

#define SESSION_VERSION "1.0"

static MooApp *moo_app_instance = NULL;
static MooAppInput *moo_app_input = NULL;

static volatile int signal_received;


struct _MooAppPrivate {
    char      **argv;
    int         exit_code;
    MooEditor  *editor;
    MooAppInfo *info;
    char       *rc_files[2];
    gboolean    run_input;

    gboolean    running;
    gboolean    in_try_quit;

    EggSMClient *sm_client;
    char       *session_file;
    MooMarkupDoc *session;

    MooUIXML   *ui_xml;
    char       *default_ui;
    guint       quit_handler_id;
    gboolean    use_editor;
    gboolean    quit_on_editor_close;

    char       *tmpdir;
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

static void     install_common_actions  (void);
static void     install_editor_actions  (void);

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
static void     moo_app_load_session_real (MooApp           *app,
                                         MooMarkupNode      *xml);
static void     moo_app_save_session_real (MooApp           *app,
                                         MooMarkupNode      *xml);
static GtkWidget *moo_app_create_prefs_dialog (MooApp       *app);

static void     moo_app_load_prefs      (MooApp             *app);
static void     moo_app_save_prefs      (MooApp             *app);

static void     moo_app_set_name        (MooApp             *app,
                                         const char         *short_name,
                                         const char         *full_name);
static void     moo_app_set_description (MooApp             *app,
                                         const char         *description);
static void     moo_app_set_version     (MooApp             *app,
                                         const char         *version);

static void     moo_app_save_session    (MooApp             *app);
static void     moo_app_write_session   (MooApp             *app);

static void     start_input             (MooApp             *app);


static GObjectClass *moo_app_parent_class;

GType
moo_app_get_type (void)
{
    static GType type = 0;

    if (G_UNLIKELY (!type))
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
    PROP_VERSION,
    PROP_DESCRIPTION,
    PROP_RUN_INPUT,
    PROP_USE_EDITOR,
    PROP_QUIT_ON_EDITOR_CLOSE,
    PROP_DEFAULT_UI,
    PROP_LOGO,
    PROP_WEBSITE,
    PROP_WEBSITE_LABEL,
    PROP_CREDITS
};

enum {
    INIT,
    RUN,
    QUIT,
    TRY_QUIT,
    PREFS_DIALOG,
    EXEC_CMD,
    LOAD_SESSION,
    SAVE_SESSION,
    LAST_SIGNAL
};


static guint signals[LAST_SIGNAL];


static void
moo_app_class_init (MooAppClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    moo_app_parent_class = g_type_class_peek_parent (klass);

    gobject_class->constructor = moo_app_constructor;
    gobject_class->finalize = moo_app_finalize;
    gobject_class->set_property = moo_app_set_property;
    gobject_class->get_property = moo_app_get_property;

    klass->init = moo_app_init_real;
    klass->run = moo_app_run_real;
    klass->quit = moo_app_quit_real;
    klass->try_quit = moo_app_try_quit_real;
    klass->prefs_dialog = moo_app_create_prefs_dialog;
    klass->exec_cmd = moo_app_exec_cmd_real;
    klass->load_session = moo_app_load_session_real;
    klass->save_session = moo_app_save_session_real;

    g_object_class_install_property (gobject_class,
                                     PROP_ARGV,
                                     g_param_spec_boxed ("argv",
                                             "argv",
                                             "Null-terminated array of application arguments",
                                             G_TYPE_STRV,
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
                                     PROP_VERSION,
                                     g_param_spec_string ("version",
                                             "version",
                                             "version",
                                             NULL,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_WEBSITE,
                                     g_param_spec_string ("website",
                                             "website",
                                             "website",
                                             NULL,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_WEBSITE_LABEL,
                                     g_param_spec_string ("website-label",
                                             "website-label",
                                             "website-label",
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
                                     PROP_LOGO,
                                     g_param_spec_string ("logo",
                                             "logo",
                                             "logo",
                                             NULL,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_RUN_INPUT,
                                     g_param_spec_boolean ("run-input",
                                             "run-input",
                                             "run-input",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_USE_EDITOR,
                                     g_param_spec_boolean ("use-editor",
                                             "use-editor",
                                             "use-editor",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_QUIT_ON_EDITOR_CLOSE,
                                     g_param_spec_boolean ("quit-on-editor-close",
                                             "quit-on-editor-close",
                                             "quit-on-editor-close",
                                             FALSE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_DEFAULT_UI,
                                     g_param_spec_string ("default-ui",
                                             "default-ui",
                                             "default-ui",
                                             NULL,
                                             G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_CREDITS,
                                     g_param_spec_string ("credits",
                                             "credits",
                                             "credits",
                                             NULL,
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

    signals[LOAD_SESSION] =
            g_signal_new ("load-session",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooAppClass, load_session),
                          NULL, NULL,
                          _moo_marshal_VOID__POINTER,
                          G_TYPE_NONE, 1,
                          G_TYPE_POINTER);

    signals[SAVE_SESSION] =
            g_signal_new ("save-session",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooAppClass, save_session),
                          NULL, NULL,
                          _moo_marshal_VOID__POINTER,
                          G_TYPE_NONE, 1,
                          G_TYPE_POINTER);
}


static void
moo_app_instance_init (MooApp *app)
{
    g_return_if_fail (moo_app_instance == NULL);

    _moo_stock_init ();

    moo_app_instance = app;

    app->priv = g_new0 (MooAppPrivate, 1);
    app->priv->info = moo_app_info_new ();

    app->priv->info->version = g_strdup (APP_VERSION);
    app->priv->info->website = g_strdup ("http://ggap.sourceforge.net/");
    app->priv->info->website_label = g_strdup ("http://ggap.sourceforge.net");
}


#if defined(HAVE_SIGNAL)
static void
setup_signals (void(*handler)(int))
{
    signal (SIGINT, handler);
#ifdef SIGHUP
    /* TODO: maybe detach from terminal in this case? */
    signal (SIGHUP, handler);
#endif
}

static void
sigint_handler (int sig)
{
    signal_received = sig;
    setup_signals (SIG_DFL);
}
#endif

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
        g_critical ("going to crash now");
        return NULL;
    }

    object = moo_app_parent_class->constructor (type, n_params, params);
    app = MOO_APP (object);

    g_set_prgname (app->priv->info->short_name);

    if (!app->priv->info->full_name)
        app->priv->info->full_name = g_strdup (app->priv->info->short_name);

#if defined(HAVE_SIGNAL) && defined(SIGINT)
    setup_signals (sigint_handler);
#endif

    install_common_actions ();
    install_editor_actions ();

    return object;
}


static void
moo_app_finalize (GObject *object)
{
    MooApp *app = MOO_APP(object);

    moo_app_quit_real (app);

    moo_app_instance = NULL;

    g_free (app->priv->rc_files[0]);
    g_free (app->priv->rc_files[1]);
    moo_app_info_free (app->priv->info);
    g_free (app->priv->default_ui);

    g_free (app->priv->session_file);
    if (app->priv->session)
        moo_markup_doc_unref (app->priv->session);

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
            moo_app_set_argv (app, (char**) g_value_get_boxed (value));
            break;

        case PROP_SHORT_NAME:
            moo_app_set_name (app, g_value_get_string (value), NULL);
            break;

        case PROP_FULL_NAME:
            moo_app_set_name (app, NULL, g_value_get_string (value));
            break;

        case PROP_VERSION:
            moo_app_set_version (app, g_value_get_string (value));
            break;

        case PROP_WEBSITE:
            g_free (app->priv->info->website);
            app->priv->info->website = g_strdup (g_value_get_string (value));
            g_object_notify (G_OBJECT (app), "website");
            break;
        case PROP_WEBSITE_LABEL:
            g_free (app->priv->info->website_label);
            app->priv->info->website_label = g_strdup (g_value_get_string (value));
            g_object_notify (G_OBJECT (app), "website_label");
            break;

        case PROP_DESCRIPTION:
            moo_app_set_description (app, g_value_get_string (value));
            break;

        case PROP_RUN_INPUT:
            app->priv->run_input = g_value_get_boolean (value);
            break;

        case PROP_USE_EDITOR:
            app->priv->use_editor = g_value_get_boolean (value);
            break;

        case PROP_QUIT_ON_EDITOR_CLOSE:
            app->priv->quit_on_editor_close = g_value_get_boolean (value);
            break;

        case PROP_DEFAULT_UI:
            g_free (app->priv->default_ui);
            app->priv->default_ui = g_strdup (g_value_get_string (value));
            break;

        case PROP_LOGO:
            g_free (app->priv->info->logo);
            app->priv->info->logo = g_strdup (g_value_get_string (value));
            g_object_notify (G_OBJECT (app), "logo");
            break;

        case PROP_CREDITS:
            g_free (app->priv->info->credits);
            app->priv->info->credits = g_strdup (g_value_get_string (value));
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

        case PROP_VERSION:
            g_value_set_string (value, app->priv->info->version);
            break;

        case PROP_DESCRIPTION:
            g_value_set_string (value, app->priv->info->description);
            break;

        case PROP_WEBSITE:
            g_value_set_string (value, app->priv->info->website);
            break;
        case PROP_WEBSITE_LABEL:
            g_value_set_string (value, app->priv->info->website_label);
            break;

        case PROP_RUN_INPUT:
            g_value_set_boolean (value, app->priv->run_input);
            break;

        case PROP_USE_EDITOR:
            g_value_set_boolean (value, app->priv->use_editor);
            break;

        case PROP_QUIT_ON_EDITOR_CLOSE:
            g_value_set_boolean (value, app->priv->quit_on_editor_close);
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


void
moo_app_set_exit_code (MooApp      *app,
                       int          code)
{
    g_return_if_fail (MOO_IS_APP (app));
    app->priv->exit_code = code;
}


const char *
moo_app_get_input_pipe_name (G_GNUC_UNUSED MooApp *app)
{
    return moo_app_input ? _moo_app_input_get_name (moo_app_input) : NULL;
}


char *
moo_app_create_user_data_dir (MooApp *app)
{
    char *dir;
    GError *error = NULL;

    g_return_val_if_fail (MOO_IS_APP (app), NULL);

    dir = moo_get_user_data_dir ();
    g_return_val_if_fail (dir != NULL, NULL);

    if (g_file_test (dir, G_FILE_TEST_IS_DIR))
        return dir;

    if (!_moo_create_dir (dir, &error))
    {
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
        g_free (dir);
        dir = NULL;
    }

    return dir;
}


static gboolean
moo_app_python_run_file (MooApp      *app,
                         const char  *filename)
{
    FILE *file;
    MooPyObject *res;

    g_return_val_if_fail (MOO_IS_APP (app), FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);
    g_return_val_if_fail (moo_python_running (), FALSE);

    file = _moo_fopen (filename, "r");
    g_return_val_if_fail (file != NULL, FALSE);

    res = moo_python_run_file (file, filename, NULL, NULL);

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


static gboolean
run_python_string (const char *string)
{
    MooPyObject *res;

    g_return_val_if_fail (string != NULL, FALSE);
    g_return_val_if_fail (moo_python_running (), FALSE);

    res = moo_python_run_simple_string (string);

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


MooEditor *
moo_app_get_editor (MooApp *app)
{
    g_return_val_if_fail (MOO_IS_APP (app), NULL);
    return app->priv->editor;
}


const MooAppInfo *
moo_app_get_info (MooApp     *app)
{
    g_return_val_if_fail (MOO_IS_APP (app), NULL);
    return app->priv->info;
}


#ifdef MOO_BUILD_EDIT
static gboolean
close_editor_window (MooApp *app)
{
    GSList *windows;
    gboolean ret = FALSE;

    if (!app->priv->running || app->priv->in_try_quit ||
        !app->priv->quit_on_editor_close)
        return FALSE;

    windows = moo_editor_list_windows (app->priv->editor);

    if (windows && !windows->next)
    {
        moo_app_quit (app);
        ret = TRUE;
    }

    g_slist_free (windows);
    return ret;
}

static void
moo_app_init_editor (MooApp *app)
{
    app->priv->editor = moo_editor_create_instance ();

    g_signal_connect_swapped (app->priv->editor, "close-window",
                              G_CALLBACK (close_editor_window), app);

    /* if ui_xml wasn't set yet, then moo_app_get_ui_xml()
       will get editor's xml */
    moo_editor_set_ui_xml (app->priv->editor,
                           moo_app_get_ui_xml (app));
    moo_editor_set_app_name (app->priv->editor,
                             app->priv->info->short_name);

    moo_plugin_read_dirs ();

    _moo_edit_load_user_tools (MOO_USER_TOOL_MENU);
    _moo_edit_load_user_tools (MOO_USER_TOOL_CONTEXT);
}
#endif /* MOO_BUILD_EDIT */


static void
moo_app_init_ui (MooApp *app)
{
    MooUIXML *xml = NULL;
    char **files;
    guint n_files, i;

    files = moo_get_data_files (MOO_UI_XML_FILE, MOO_DATA_SHARE, &n_files);

    for (i = 0; i < n_files; ++i)
    {
        GError *error = NULL;
        GMappedFile *file;

        file = g_mapped_file_new (files[i], FALSE, &error);

        if (file)
        {
            xml = moo_ui_xml_new ();
            moo_ui_xml_add_ui_from_string (xml,
                                           g_mapped_file_get_contents (file),
                                           g_mapped_file_get_length (file));
            g_mapped_file_free (file);
            break;
        }

        if (error->domain != G_FILE_ERROR || error->code != G_FILE_ERROR_NOENT)
        {
            g_warning ("%s: could not open file '%s'", G_STRLOC, files[i]);
            g_warning ("%s: %s", G_STRLOC, error->message);
        }

        g_error_free (error);
    }

    if (!xml && app->priv->default_ui)
    {
        xml = moo_ui_xml_new ();
        moo_ui_xml_add_ui_from_string (xml, app->priv->default_ui, -1);
    }

    if (xml)
    {
        if (app->priv->ui_xml)
            g_object_unref (app->priv->ui_xml);
        app->priv->ui_xml = xml;
    }

    g_strfreev (files);
}


static gboolean
moo_app_init_real (MooApp *app)
{
    gdk_set_program_class (app->priv->info->full_name);
    gtk_window_set_default_icon_name (app->priv->info->short_name);

    moo_app_load_prefs (app);
    moo_app_init_ui (app);

#ifdef MOO_BUILD_EDIT
    if (app->priv->use_editor)
        moo_app_init_editor (app);
#endif

    start_input (app);

    return TRUE;
}


static void
start_input (MooApp *app)
{
    if (app->priv->run_input)
    {
        moo_app_input = _moo_app_input_new (app->priv->info->short_name);

        if (!_moo_app_input_start (moo_app_input))
        {
            g_critical ("%s: oops", G_STRLOC);
            _moo_app_input_unref (moo_app_input);
            moo_app_input = NULL;
        }
    }
}


gboolean
moo_app_send_msg (MooApp     *app,
                  const char *pid,
                  const char *data,
                  int         len)
{
    g_return_val_if_fail (MOO_IS_APP (app), FALSE);
    g_return_val_if_fail (data != NULL, FALSE);
    return _moo_app_input_send_msg (app->priv->info->short_name, pid, data, len);
}


gboolean
moo_app_send_files (MooApp     *app,
                    char      **files,
                    guint32     line,
                    guint32     stamp,
                    const char *pid)
{
    gboolean result;
    GString *msg;

    g_return_val_if_fail (MOO_IS_APP (app), FALSE);

    _moo_message ("moo_app_send_files: got %d files to pid %s",
                  files ? g_strv_length (files) : 0,
                  pid ? pid : "NONE");

    msg = g_string_new (NULL);

    g_string_append_printf (msg, "%s%08x%08x", CMD_OPEN_URIS, stamp, line);

    while (files && *files)
    {
        char *freeme = NULL, *uri;
        const char *basename, *filename;

        basename = *files++;

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

        uri = g_filename_to_uri (filename, NULL, NULL);

        if (uri)
        {
            g_string_append (msg, uri);
            g_string_append (msg, "\r\n");
        }

        g_free (freeme);
        g_free (uri);
    }

    result = moo_app_send_msg (app, pid, msg->str, msg->len + 1);

    g_string_free (msg, TRUE);
    return result;
}


static gboolean
on_gtk_main_quit (MooApp *app)
{
    app->priv->quit_handler_id = 0;

    if (!moo_app_quit (app))
        MOO_APP_GET_CLASS(app)->quit (app);

    return FALSE;
}


static gboolean
check_signal (void)
{
    if (signal_received)
    {
        printf ("%s\n", g_strsignal (signal_received));
        MOO_APP_GET_CLASS(moo_app_instance)->quit (moo_app_instance);
        exit (0);
    }

    return TRUE;
}


static gboolean
moo_app_try_quit (MooApp *app)
{
    gboolean stopped = FALSE;

    g_return_val_if_fail (MOO_IS_APP (app), FALSE);

    if (!app->priv->running)
        return TRUE;

    app->priv->in_try_quit = TRUE;
    g_signal_emit (app, signals[TRY_QUIT], 0, &stopped);
    app->priv->in_try_quit = FALSE;

    return !stopped;
}


static void
sm_quit_requested (MooApp *app)
{
    EggSMClient *sm_client;

    sm_client = app->priv->sm_client;
    g_return_if_fail (sm_client != NULL);

    g_object_ref (sm_client);
    egg_sm_client_will_quit (sm_client, moo_app_try_quit (app));
    g_object_unref (sm_client);
}

static void
sm_quit (MooApp *app)
{
    if (!moo_app_quit (app))
        MOO_APP_GET_CLASS(app)->quit (app);
}

static int
moo_app_run_real (MooApp *app)
{
    g_return_val_if_fail (!app->priv->running, 0);
    app->priv->running = TRUE;

    app->priv->quit_handler_id =
            gtk_quit_add (1, (GtkFunction) on_gtk_main_quit, app);

    _moo_timeout_add (100, (GSourceFunc) check_signal, NULL);

    app->priv->sm_client = egg_sm_client_get ();
    /* make it install log handler */
    g_option_group_free (egg_sm_client_get_option_group ());
    g_signal_connect_swapped (app->priv->sm_client, "quit-requested",
                              G_CALLBACK (sm_quit_requested), app);
    g_signal_connect_swapped (app->priv->sm_client, "quit",
                              G_CALLBACK (sm_quit), app);
    if (EGG_SM_CLIENT_GET_CLASS (app->priv->sm_client)->startup)
        EGG_SM_CLIENT_GET_CLASS (app->priv->sm_client)->startup (app->priv->sm_client, NULL);

    gtk_main ();

    return app->priv->exit_code;
}


static gboolean
moo_app_try_quit_real (MooApp *app)
{
    if (!app->priv->running)
        return FALSE;

    moo_app_save_session (app);

#ifdef MOO_BUILD_EDIT
    if (!moo_editor_close_all (app->priv->editor, TRUE, TRUE))
        return TRUE;
#endif /* MOO_BUILD_EDIT */

    return FALSE;
}


static void
moo_app_quit_real (MooApp *app)
{
    guint i;

    if (!app->priv->running)
        return;
    else
        app->priv->running = FALSE;

    g_object_unref (app->priv->sm_client);
    app->priv->sm_client = NULL;

    if (moo_app_input)
    {
        _moo_app_input_shutdown (moo_app_input);
        _moo_app_input_unref (moo_app_input);
        moo_app_input = NULL;
    }

#ifdef MOO_BUILD_EDIT
    moo_editor_close_all (app->priv->editor, FALSE, FALSE);

    moo_plugin_shutdown ();

    g_object_unref (app->priv->editor);
    app->priv->editor = NULL;
#endif /* MOO_BUILD_EDIT */

    moo_app_write_session (app);
    moo_app_save_prefs (app);

    if (app->priv->quit_handler_id)
        gtk_quit_remove (app->priv->quit_handler_id);

    i = 0;
    while (gtk_main_level () && i < 1000)
    {
        gtk_main_quit ();
        i++;
    }

    if (app->priv->tmpdir)
    {
        GError *error = NULL;
        _moo_remove_dir (app->priv->tmpdir, TRUE, &error);

        if (error)
        {
            g_warning ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }

        g_free (app->priv->tmpdir);
        app->priv->tmpdir = NULL;
    }

#ifdef MOO_USE_XDGMIME
    xdg_mime_shutdown ();
#endif
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


gboolean
moo_app_quit (MooApp *app)
{
    g_return_val_if_fail (MOO_IS_APP (app), FALSE);

    if (app->priv->in_try_quit || !app->priv->running)
        return TRUE;

    if (moo_app_try_quit (app))
    {
        MOO_APP_GET_CLASS(app)->quit (app);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


static void
moo_app_set_name (MooApp     *app,
                  const char *short_name,
                  const char *full_name)
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


static void
moo_app_set_description (MooApp     *app,
                         const char *description)
{
    g_free (app->priv->info->description);
    app->priv->info->description = g_strdup (description);
    g_object_notify (G_OBJECT (app), "description");
}

static void
moo_app_set_version (MooApp     *app,
                     const char *version)
{
    g_free (app->priv->info->version);
    app->priv->info->version = g_strdup (version);
    g_object_notify (G_OBJECT (app), "version");
}


static void
install_common_actions (void)
{
    MooWindowClass *klass = g_type_class_ref (MOO_TYPE_WINDOW);

    g_return_if_fail (klass != NULL);

    moo_window_class_new_action (klass, "Preferences", NULL,
                                 "display-name", GTK_STOCK_PREFERENCES,
                                 "label", GTK_STOCK_PREFERENCES,
                                 "tooltip", GTK_STOCK_PREFERENCES,
                                 "stock-id", GTK_STOCK_PREFERENCES,
                                 "closure-callback", moo_app_prefs_dialog,
                                 NULL);

    moo_window_class_new_action (klass, "About", NULL,
                                 "label", GTK_STOCK_ABOUT,
                                 "no-accel", TRUE,
                                 "stock-id", GTK_STOCK_ABOUT,
                                 "closure-callback", moo_app_about_dialog,
                                 NULL);

    moo_window_class_new_action (klass, "SystemInfo", NULL,
                                 /* menu item label */
                                 "label", _("System Info"),
                                 "no-accel", TRUE,
                                 "closure-callback", moo_app_system_info_dialog,
                                 NULL);

    moo_window_class_new_action (klass, "Quit", NULL,
                                 "display-name", GTK_STOCK_QUIT,
                                 "label", GTK_STOCK_QUIT,
                                 "tooltip", GTK_STOCK_QUIT,
                                 "stock-id", GTK_STOCK_QUIT,
                                 "accel", "<ctrl>Q",
                                 "closure-callback", moo_app_quit,
                                 "closure-proxy-func", moo_app_get_instance,
                                 NULL);

    g_type_class_unref (klass);
}


static void
install_editor_actions (void)
{
#ifdef MOO_BUILD_EDIT
    MooWindowClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);
    g_return_if_fail (klass != NULL);
    g_type_class_unref (klass);
#endif /* MOO_BUILD_EDIT */
}


MooUIXML *
moo_app_get_ui_xml (MooApp *app)
{
    g_return_val_if_fail (MOO_IS_APP (app), NULL);

    if (!app->priv->ui_xml)
    {
#ifdef MOO_BUILD_EDIT
        if (app->priv->editor)
        {
            app->priv->ui_xml = moo_editor_get_ui_xml (app->priv->editor);
            g_object_ref (app->priv->ui_xml);
        }
#endif
        if (!app->priv->ui_xml)
            app->priv->ui_xml = moo_ui_xml_new ();
    }

    return app->priv->ui_xml;
}


void
moo_app_set_ui_xml (MooApp     *app,
                    MooUIXML   *xml)
{
    g_return_if_fail (MOO_IS_APP (app));

    if (app->priv->ui_xml == xml)
        return;

    if (app->priv->ui_xml)
        g_object_unref (app->priv->ui_xml);

    app->priv->ui_xml = xml;

    if (xml)
        g_object_ref (app->priv->ui_xml);

#ifdef MOO_BUILD_EDIT
    if (app->priv->editor)
        moo_editor_set_ui_xml (app->priv->editor, xml);
#endif /* MOO_BUILD_EDIT */
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
    copy->logo = g_strdup (info->logo);
    copy->credits = g_strdup (info->credits);

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
        g_free (info->logo);
        g_free (info->credits);
        g_free (info);
    }
}


GType
moo_app_info_get_type (void)
{
    static GType type = 0;
    if (G_UNLIKELY (!type))
        g_boxed_type_register_static ("MooAppInfo",
                                      (GBoxedCopyFunc) moo_app_info_copy,
                                      (GBoxedFreeFunc) moo_app_info_free);
    return type;
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
moo_app_new_file (MooApp       *app,
                  const char   *filename,
                  guint32       line)
{
#ifdef MOO_BUILD_EDIT
    MooEditor *editor = moo_app_get_editor (app);

    g_return_if_fail (editor != NULL);

    if (filename)
    {
        char *norm_name = _moo_normalize_file_path (filename);

        /* Normal case, like 'medit /foo/ *', ignore directories here;
         * usual errors will be handled in editor code. */
        if (g_str_has_suffix (filename, "/") ||
            g_file_test (norm_name, G_FILE_TEST_IS_DIR))
        {
            _moo_message ("%s: %s is a directory", G_STRLOC, norm_name);
        }
        else
        {
            char *colon;

            if ((colon = strrchr (norm_name, ':')) &&
                colon != norm_name &&
                strspn (colon + 1, "0123456789") == strlen (colon + 1) &&
                !g_file_test (norm_name, G_FILE_TEST_EXISTS))
            {
                if (colon[1])
                {
                    errno = 0;
                    line = strtol (colon + 1, NULL, 10);
                    if (errno)
                        line = 0;
                }

                *colon = 0;
            }

            if (line > 0)
                moo_editor_open_file_line (editor, norm_name, line - 1, NULL);
            else
                moo_editor_new_file (editor, NULL, NULL, norm_name, NULL);
        }

        g_free (norm_name);
    }
    else
    {
        MooEdit *doc;

        doc = moo_editor_get_active_doc (editor);

        if (!doc || !moo_edit_is_empty (doc))
            moo_editor_new_doc (editor, NULL);
    }
#endif /* MOO_BUILD_EDIT */
}


static void
moo_app_load_session_real (MooApp        *app,
                           MooMarkupNode *xml)
{
#ifdef MOO_BUILD_EDIT
    MooEditor *editor;
    editor = moo_app_get_editor (app);
    g_return_if_fail (editor != NULL);
    _moo_editor_load_session (editor, xml);
#endif /* MOO_BUILD_EDIT */
}

static void
moo_app_save_session_real (MooApp        *app,
                           MooMarkupNode *xml)
{
#ifdef MOO_BUILD_EDIT
    MooEditor *editor;
    editor = moo_app_get_editor (app);
    g_return_if_fail (editor != NULL);
    _moo_editor_save_session (editor, xml);
#endif /* MOO_BUILD_EDIT */
}

static void
moo_app_save_session (MooApp *app)
{
    MooMarkupNode *root;

    if (!app->priv->session_file)
        return;

    if (app->priv->session)
        moo_markup_doc_unref (app->priv->session);

    app->priv->session = moo_markup_doc_new ("session");
    root = moo_markup_create_root_element (app->priv->session, "session");
    moo_markup_set_prop (root, "version", SESSION_VERSION);

    g_signal_emit (app, signals[SAVE_SESSION], 0, root);
}

static void
moo_app_write_session (MooApp *app)
{
    char *string;
    GError *error = NULL;

    if (!app->priv->session_file)
        return;

    if (!app->priv->session)
    {
        char *file = moo_get_user_cache_file (app->priv->session_file);
        _moo_unlink (file);
        g_free (file);
        return;
    }

    string = moo_markup_format_pretty (app->priv->session, 1);
    moo_save_user_cache_file (app->priv->session_file, string, -1, &error);

    if (error)
    {
        char *file = moo_get_user_cache_file (app->priv->session_file);
        g_critical ("could not save session file %s: %s", file, error->message);
        g_free (file);
        g_error_free (error);
    }
}

void
moo_app_load_session (MooApp *app)
{
    MooMarkupDoc *doc;
    MooMarkupNode *root;
    GError *error = NULL;
    const char *version;
    char *session_file;

    g_return_if_fail (MOO_IS_APP (app));

    if (!app->priv->session_file)
        app->priv->session_file = g_strdup_printf ("%s.session", g_get_prgname ());

    session_file = moo_get_user_cache_file (app->priv->session_file);

    if (!g_file_test (session_file, G_FILE_TEST_EXISTS) ||
        !(doc = moo_markup_parse_file (session_file, &error)))
    {
        if (error)
        {
            g_warning ("could not open session file %s: %s",
                       session_file, error->message);
            g_error_free (error);
        }

        g_free (session_file);
        return;
    }

    if (!(root = moo_markup_get_root_element (doc, "session")) ||
        !(version = moo_markup_get_prop (root, "version")))
        g_warning ("malformed session file %s, ignoring", session_file);
    else if (strcmp (version, SESSION_VERSION) != 0)
        g_warning ("invalid session file version %s in %s, ignoring",
                   version, session_file);
    else
        g_signal_emit (app, signals[LOAD_SESSION], 0, root);

    moo_markup_doc_unref (doc);
    g_free (session_file);
}


static void
moo_app_present (MooApp *app)
{
    gpointer window = NULL;

#ifdef MOO_BUILD_EDIT
    if (!window && app->priv->editor)
        window = moo_editor_get_active_window (app->priv->editor);
#endif /* MOO_BUILD_EDIT */

    if (window)
        moo_window_present (window, 0);
}


static void
moo_app_open_uris (MooApp     *app,
                   const char *data)
{
#ifdef MOO_BUILD_EDIT
    char **uris;
    guint32 stamp;
    char *stamp_string;
    char *line_string;
    guint32 line;

    stamp_string = g_strndup (data, 8);
    stamp = strtoul (stamp_string, NULL, 16);
    line_string = g_strndup (data + 8, 8);
    line = strtoul (line_string, NULL, 16);

    if (line > G_MAXINT)
        line = 0;

    data += 16;
    uris = g_strsplit (data, "\r\n", 0);

    if (uris && *uris)
    {
        char **p;

        for (p = uris; p && *p; ++p)
        {
            char *filename = g_filename_from_uri (*p, NULL, NULL);

            if (filename)
            {
                if (p == uris && line > 0)
                    moo_app_new_file (app, filename, line);
                else
                    moo_app_new_file (app, filename, 0);
            }

            g_free (filename);
        }
    }
    else
    {
        moo_app_new_file (app, NULL, 0);
    }

    moo_editor_present (app->priv->editor, stamp);

    g_strfreev (uris);
    g_free (stamp_string);
#endif /* MOO_BUILD_EDIT */
}

void
moo_app_open_files (MooApp     *app,
                    char      **files,
                    guint32     line,
                    guint32     stamp)
{
#ifdef MOO_BUILD_EDIT
    char **p;

    if (line > G_MAXINT)
        line = 0;

    for (p = files; p && *p; ++p)
    {
        if (p == files && line > 0)
            moo_app_new_file (app, *p, line);
        else
            moo_app_new_file (app, *p, 0);
    }

    moo_editor_present (app->priv->editor, stamp);
#endif /* MOO_BUILD_EDIT */
}


static MooAppCmdCode
get_cmd_code (char cmd)
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
            run_python_string (data);
            break;
        case MOO_APP_CMD_PYTHON_FILE:
            moo_app_python_run_file (app, data);
            break;

        case MOO_APP_CMD_OPEN_FILE:
            moo_app_new_file (app, data, 0);
            break;
        case MOO_APP_CMD_OPEN_URIS:
            moo_app_open_uris (app, data);
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
            g_warning ("%s: got unknown command %c", G_STRLOC, cmd);
    }
}


char*
moo_app_tempnam (MooApp *app)
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

            if (_moo_mkdir (dirname))
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


void
moo_app_prefs_dialog (GtkWidget *parent)
{
    MooApp *app;
    GtkWidget *dialog;

    app = moo_app_get_instance ();
    dialog = MOO_APP_GET_CLASS(app)->prefs_dialog (app);
    g_return_if_fail (MOO_IS_PREFS_DIALOG (dialog));

    moo_prefs_dialog_run (MOO_PREFS_DIALOG (dialog), parent);
}


static void
prefs_dialog_apply (void)
{
    moo_app_save_prefs (moo_app_get_instance ());
}


static GtkWidget *
moo_app_create_prefs_dialog (MooApp *app)
{
    char *title;
    const MooAppInfo *info;
    MooPrefsDialog *dialog;

    info = moo_app_get_info (app);
    title = g_strdup_printf ("%s Preferences", info->full_name);
    dialog = MOO_PREFS_DIALOG (moo_prefs_dialog_new (title));
    g_free (title);

#ifdef MOO_BUILD_EDIT
    moo_prefs_dialog_append_page (dialog, moo_edit_prefs_page_new (moo_app_get_editor (app)));
    moo_prefs_dialog_append_page (dialog, _moo_user_tools_prefs_page_new ());
    moo_plugin_attach_prefs (GTK_WIDGET (dialog));
#endif

    g_signal_connect_after (dialog, "apply",
                            G_CALLBACK (prefs_dialog_apply),
                            NULL);

    return GTK_WIDGET (dialog);
}


#ifndef __WIN32__
static void
move_rc_files (MooApp *app)
{
    char *old_dir;
    char *new_dir;
    char *cache_dir;

    old_dir = g_strdup_printf ("%s/.%s", g_get_home_dir (), g_get_prgname ());
    new_dir = g_strdup_printf ("%s/%s", g_get_user_data_dir (), g_get_prgname ());
    cache_dir = g_strdup_printf ("%s/%s", g_get_user_cache_dir (), g_get_prgname ());

    /* do not be too clever here, there are way too many possible errors */

    if (!g_file_test (new_dir, G_FILE_TEST_EXISTS) &&
        g_file_test (old_dir, G_FILE_TEST_EXISTS) &&
        _moo_rename (old_dir, new_dir) != 0)
    {
        _moo_set_user_data_dir (old_dir);
    }

    {
        char *new_file;
        char *old_file;

        new_file = g_strdup_printf ("%s/%src", g_get_user_config_dir (), g_get_prgname ());
        old_file = g_strdup_printf ("%s/.%src", g_get_home_dir (), g_get_prgname ());

        if (!g_file_test (new_file, G_FILE_TEST_EXISTS) &&
            g_file_test (old_file, G_FILE_TEST_EXISTS) &&
            _moo_rename (old_file, new_file) != 0)
        {
            app->priv->rc_files[MOO_PREFS_RC] = old_file;
            old_file = NULL;
        }
        else
        {
            app->priv->rc_files[MOO_PREFS_RC] = new_file;
            new_file = NULL;

            if (!g_file_test (g_get_user_config_dir (), G_FILE_TEST_EXISTS))
                _moo_mkdir_with_parents (g_get_user_config_dir ());
        }

        g_free (old_file);
        g_free (new_file);
    }

    if (!g_file_test (cache_dir, G_FILE_TEST_EXISTS))
        _moo_mkdir_with_parents (cache_dir);

    {
        const char *new_file;
        char *old_file = g_strdup_printf ("%s/.%s.state", g_get_home_dir (), g_get_prgname ());

        app->priv->rc_files[MOO_PREFS_STATE] =
            g_strdup_printf ("%s/%s.state", cache_dir, g_get_prgname ());
        new_file = app->priv->rc_files[MOO_PREFS_STATE];

        if (!g_file_test (new_file, G_FILE_TEST_EXISTS) &&
            g_file_test (old_file, G_FILE_TEST_EXISTS))
        {
            _moo_rename (old_file, new_file);
        }

        g_free (old_file);
    }

    g_free (cache_dir);
    g_free (new_dir);
    g_free (old_dir);
}
#endif


static void
moo_app_load_prefs (MooApp *app)
{
    GError *error = NULL;

#ifndef __WIN32__
    move_rc_files (app);
#else
    app->priv->rc_files[MOO_PREFS_RC] =
        g_strdup_printf ("%s/%s.ini", g_get_user_config_dir (), g_get_prgname ());
    app->priv->rc_files[MOO_PREFS_STATE] =
        g_strdup_printf ("%s/%s.state", g_get_user_config_dir (), g_get_prgname ());
#endif

    if (!moo_prefs_load (app->priv->rc_files[MOO_PREFS_RC],
                         app->priv->rc_files[MOO_PREFS_STATE],
                         &error))
    {
        g_warning ("%s: could not read config file", G_STRLOC);

        if (error)
        {
            g_warning ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }
    }
}


static void
moo_app_save_prefs (MooApp *app)
{
    GError *error = NULL;

    if (!moo_prefs_save (app->priv->rc_files[MOO_PREFS_RC],
                         app->priv->rc_files[MOO_PREFS_STATE],
                         &error))
    {
        g_warning ("%s: could not save config file", G_STRLOC);

        if (error)
        {
            g_warning ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }
    }
}
