/*
 *   mooapp/mooapp.c
 *
 *   Copyright (C) 2004-2006 by Yevgen Muntyan <muntyan@math.tamu.edu>
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

#define MOO_APP_COMPILATION
#define WANT_MOO_APP_CMD_CHARS
#include "mooapp/mooappinput.h"
#include "mooapp/mooapp-private.h"
#include "mooterm/mootermwindow.h"
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
#include "mooutils/moocompat.h"
#include "mooutils/moodialogs.h"
#include "mooutils/moostock.h"
#include "mooutils/mooutils-fs.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooi18n.h"
#include "mooutils/xdgmime/xdgmime.h"
#include <string.h>
#include <stdio.h>

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

static MooApp *moo_app_instance = NULL;
static MooAppInput *moo_app_input = NULL;


struct _MooAppPrivate {
    char      **argv;
    int         exit_code;
    MooEditor  *editor;
    MooAppInfo *info;
    char       *rc_files[2];
    gboolean    run_input;

    gboolean    running;
    gboolean    in_try_quit;

    GType       term_window_type;
    GSList     *terminals;
    MooTermWindow *term_window;

    MooUIXML   *ui_xml;
    char       *default_ui;
    guint       quit_handler_id;
    gboolean    use_editor;
    gboolean    use_terminal;

    char       *tmpdir;
    guint       sigintr : 1;
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
static GtkWidget *moo_app_create_prefs_dialog (MooApp       *app);

static void     moo_app_set_name        (MooApp             *app,
                                         const char         *short_name,
                                         const char         *full_name);
static void     moo_app_set_description (MooApp             *app,
                                         const char         *description);

static void     start_input             (MooApp             *app);


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
    PROP_RUN_INPUT,
    PROP_USE_EDITOR,
    PROP_USE_TERMINAL,
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
    klass->prefs_dialog = moo_app_create_prefs_dialog;
    klass->exec_cmd = moo_app_exec_cmd_real;

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
                                     PROP_USE_TERMINAL,
                                     g_param_spec_boolean ("use-terminal",
                                             "use-terminal",
                                             "use-terminal",
                                             TRUE,
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
}


static void
moo_app_instance_init (MooApp *app)
{
    g_return_if_fail (moo_app_instance == NULL);

    moo_app_instance = app;

    app->priv = g_new0 (MooAppPrivate, 1);
    app->priv->info = moo_app_info_new ();

    app->priv->info->version = g_strdup (APP_VERSION);
    app->priv->info->website = g_strdup ("http://ggap.sourceforge.net/");
    app->priv->info->website_label = g_strdup ("http://ggap.sourceforge.net");
}


#if defined(HAVE_SIGNAL)
static RETSIGTYPE
sigint_handler (G_GNUC_UNUSED int sig)
{
    if (moo_app_instance && moo_app_instance->priv)
        moo_app_instance->priv->sigintr = TRUE;
    signal (SIGINT, SIG_DFL);
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
    signal (SIGINT, sigint_handler);
#endif

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

    g_free (app->priv->rc_files[0]);
    g_free (app->priv->rc_files[1]);
    moo_app_info_free (app->priv->info);
    g_free (app->priv->default_ui);

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

        case PROP_USE_TERMINAL:
            app->priv->use_terminal = g_value_get_boolean (value);
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


const char *
moo_app_get_rc_file_name (MooApp      *app,
                          MooPrefsType prefs_type)
{
    g_return_val_if_fail (MOO_IS_APP (app), NULL);
    g_return_val_if_fail (prefs_type < 2, NULL);

    if (!app->priv->rc_files[prefs_type])
    {
#ifdef __WIN32__
        const char *templates[] = {"%s.ini", "%s.state"};
        char *basename = g_strdup_printf (templates[prefs_type],
                                          app->priv->info->short_name);
        app->priv->rc_files[prefs_type] =
                g_build_filename (g_get_user_config_dir (),
                                  basename,
                                  NULL);
        g_free (basename);
#else
        const char *templates[] = {".%src", ".%s.state"};
        char *basename = g_strdup_printf (templates[prefs_type],
                                          app->priv->info->short_name);
        app->priv->rc_files[prefs_type] =
                g_build_filename (g_get_home_dir (),
                                  basename,
                                  NULL);
        g_free (basename);
#endif
    }

    return app->priv->rc_files[prefs_type];
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
static void
moo_app_init_editor (MooApp *app)
{
    app->priv->editor = moo_editor_create_instance ();
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
    MooUIXML *xml;
    char **files;
    guint n_files;
    int i;

    xml = moo_app_get_ui_xml (app);
    files = moo_get_data_files (MOO_UI_XML_FILE, MOO_DATA_SHARE, &n_files);

    for (i = n_files - 1; i >= 0; --i)
    {
        GError *error = NULL;
        GMappedFile *file;

        file = g_mapped_file_new (files[i], FALSE, &error);

        if (file)
        {
            moo_ui_xml_add_ui_from_string (xml,
                                           g_mapped_file_get_contents (file),
                                           g_mapped_file_get_length (file));
            g_mapped_file_free (file);
            goto out;
        }

        if (error->domain != G_FILE_ERROR || error->code != G_FILE_ERROR_NOENT)
        {
            g_warning ("%s: could not open file '%s'", G_STRLOC, files[i]);
            g_warning ("%s: %s", G_STRLOC, error->message);
        }

        g_error_free (error);
    }

    if (app->priv->default_ui)
        moo_ui_xml_add_ui_from_string (xml, app->priv->default_ui, -1);

out:
    g_strfreev (files);
}


static gboolean
moo_app_init_real (MooApp *app)
{
    GError *error = NULL;

    gdk_set_program_class (app->priv->info->full_name);
    gtk_window_set_default_icon_name (app->priv->info->short_name);

    if (!moo_prefs_load (moo_app_get_rc_file_name (app, MOO_PREFS_RC),
                         moo_app_get_rc_file_name (app, MOO_PREFS_STATE),
                         &error))
    {
        g_warning ("%s: could not read config file", G_STRLOC);

        if (error)
        {
            g_warning ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }
    }

    moo_app_init_ui (app);

#ifdef MOO_BUILD_EDIT
    if (app->priv->use_editor)
        moo_app_init_editor (app);
#endif

#if defined(__WIN32__) && defined(MOO_BUILD_TERM)
    if (app->priv->use_terminal)
    {
        char *dir = moo_get_app_dir ();
        moo_term_set_helper_directory (dir);
        g_free (dir);
    }
#endif /* __WIN32__ && MOO_BUILD_TERM */

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
    if (moo_app_instance && moo_app_instance->priv->sigintr)
    {
        moo_app_instance->priv->sigintr = FALSE;
        MOO_APP_GET_CLASS(moo_app_instance)->quit (moo_app_instance);
        gtk_main_quit ();
    }

    return TRUE;
}

static int
moo_app_run_real (MooApp *app)
{
    g_return_val_if_fail (!app->priv->running, 0);
    app->priv->running = TRUE;

    app->priv->quit_handler_id =
            gtk_quit_add (1, (GtkFunction) on_gtk_main_quit, app);

    g_timeout_add (100, (GSourceFunc) check_signal, NULL);

    gtk_main ();

    return app->priv->exit_code;
}


static gboolean
moo_app_try_quit_real (MooApp *app)
{
    GSList *l, *list;

    if (!app->priv->running)
        return FALSE;

#ifdef MOO_BUILD_EDIT
    if (!moo_editor_close_all (app->priv->editor, TRUE))
        return TRUE;
#endif /* MOO_BUILD_EDIT */

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


static void
moo_app_save_prefs (MooApp *app)
{
    GError *error = NULL;

    if (!moo_prefs_save (moo_app_get_rc_file_name (app, MOO_PREFS_RC),
                         moo_app_get_rc_file_name (app, MOO_PREFS_STATE),
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


static void
moo_app_quit_real (MooApp *app)
{
    GSList *l, *list;

    if (!app->priv->running)
        return;
    else
        app->priv->running = FALSE;

    if (moo_app_input)
    {
        _moo_app_input_shutdown (moo_app_input);
        _moo_app_input_unref (moo_app_input);
        moo_app_input = NULL;
    }

    list = g_slist_copy (app->priv->terminals);
    for (l = list; l != NULL; l = l->next)
        moo_window_close (MOO_WINDOW (l->data));
    g_slist_free (list);
    g_slist_free (app->priv->terminals);
    app->priv->terminals = NULL;
    app->priv->term_window = NULL;

#ifdef MOO_BUILD_EDIT
    moo_editor_close_all (app->priv->editor, FALSE);

    moo_plugin_shutdown ();

    g_object_unref (app->priv->editor);
    app->priv->editor = NULL;
#endif /* MOO_BUILD_EDIT */

    moo_app_save_prefs (app);

    if (app->priv->quit_handler_id)
        gtk_quit_remove (app->priv->quit_handler_id);

    gtk_main_quit ();

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
install_actions (MooApp *app,
                 GType   type)
{
    MooWindowClass *klass = g_type_class_ref (type);
    char *_about;

    g_return_if_fail (klass != NULL);

    _about = g_strdup_printf (Q_("Menu item label|_About %s"), app->priv->info->full_name);

    moo_window_class_new_action (klass, "Preferences", NULL,
                                 "display-name", GTK_STOCK_PREFERENCES,
                                 "label", GTK_STOCK_PREFERENCES,
                                 "tooltip", GTK_STOCK_PREFERENCES,
                                 "stock-id", GTK_STOCK_PREFERENCES,
                                 "closure-callback", moo_app_prefs_dialog,
                                 NULL);

    moo_window_class_new_action (klass, "About", NULL,
                                 "display-name", GTK_STOCK_ABOUT,
                                 "label", _about,
                                 "no-accel", TRUE,
                                 "stock-id", GTK_STOCK_ABOUT,
                                 "closure-callback", moo_app_about_dialog,
                                 NULL);

    g_type_class_unref (klass);
    g_free (_about);
}


static void
install_editor_actions (MooApp *app)
{
#ifdef MOO_BUILD_EDIT
    MooWindowClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);

    g_return_if_fail (klass != NULL);

    moo_window_class_new_action (klass, "Quit", NULL,
                                 "display-name", GTK_STOCK_QUIT,
                                 "label", GTK_STOCK_QUIT,
                                 "tooltip", GTK_STOCK_QUIT,
                                 "stock-id", GTK_STOCK_QUIT,
                                 "accel", "<ctrl>Q",
                                 "closure-callback", moo_app_quit,
                                 "closure-proxy-func", moo_app_get_instance,
                                 NULL);

    install_actions (app, MOO_TYPE_EDIT_WINDOW);

    g_type_class_unref (klass);
#endif /* MOO_BUILD_EDIT */
}


#if defined(MOO_BUILD_TERM) && defined(MOO_BUILD_EDIT)
static void
new_editor (MooApp *app)
{
    g_return_if_fail (app != NULL);
    gtk_window_present (GTK_WINDOW (moo_editor_new_window (app->priv->editor)));
}

static void
open_in_editor (MooTermWindow *terminal)
{
    MooApp *app = moo_app_get_instance ();
    g_return_if_fail (app != NULL);
    moo_editor_open (app->priv->editor, NULL, GTK_WIDGET (terminal), NULL);
}


static void
install_terminal_actions (MooApp *app)
{
    MooWindowClass *klass = g_type_class_ref (MOO_TYPE_TERM_WINDOW);

    g_return_if_fail (klass != NULL);

    install_actions (app, MOO_TYPE_TERM_WINDOW);

    moo_window_class_new_action (klass, "NewEditor", NULL,
                                 "display-name", "New Editor",
                                 "label", "_New Editor",
                                 "tooltip", "New Editor",
                                 "stock-id", GTK_STOCK_EDIT,
                                 "accel", "<Alt>E",
                                 "closure-callback", new_editor,
                                 "closure-proxy-func", moo_app_get_instance,
                                 NULL);

    moo_window_class_new_action (klass, "OpenInEditor", NULL,
                                 "display-name", "Open In Editor",
                                 "label", "_Open In Editor",
                                 "tooltip", "Open In Editor",
                                 "stock-id", GTK_STOCK_OPEN,
                                 "accel", "<Alt>O",
                                 "closure-callback", open_in_editor,
                                 NULL);

    g_type_class_unref (klass);
}
#else /* !(defined(MOO_BUILD_TERM) && defined(MOO_BUILD_EDIT)) */
static void
install_terminal_actions (G_GNUC_UNUSED MooApp *app)
{
}
#endif /* !(defined(MOO_BUILD_TERM) && defined(MOO_BUILD_EDIT)) */


MooUIXML *
moo_app_get_ui_xml (MooApp *app)
{
    g_return_val_if_fail (MOO_IS_APP (app), NULL);

    if (!app->priv->ui_xml)
        app->priv->ui_xml = moo_ui_xml_new ();

    return app->priv->ui_xml;
}


void
moo_app_set_ui_xml (MooApp     *app,
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

#ifdef MOO_BUILD_EDIT
    if (app->priv->editor)
        moo_editor_set_ui_xml (app->priv->editor, xml);
#endif /* MOO_BUILD_EDIT */

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
    copy->rc_file = g_strdup (info->rc_file);
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
        g_free (info->rc_file);
        g_free (info->logo);
        g_free (info->credits);
        g_free (info);
    }
}


GType
moo_app_info_get_type (void)
{
    static GType type = 0;
    if (!type)
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
        if (line > 0)
            moo_editor_open_file_line (editor, filename, line - 1, NULL);
        else
            moo_editor_new_file (editor, NULL, NULL, filename, NULL);
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
moo_app_present (MooApp *app)
{
    gpointer window = app->priv->term_window;

#ifdef MOO_BUILD_EDIT
    if (!window && app->priv->editor)
        window = moo_editor_get_active_window (app->priv->editor);
#endif /* MOO_BUILD_EDIT */

    g_return_if_fail (window != NULL);

    _moo_window_present (window, 0);
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

#if 0 && defined(MOO_BUILD_TERM)
    moo_prefs_dialog_append_page (dialog, moo_term_prefs_page_new ());
#endif

#ifdef MOO_BUILD_EDIT
    moo_prefs_dialog_append_page (dialog, moo_edit_prefs_page_new (moo_app_get_editor (app)));
    moo_prefs_dialog_append_page (dialog, _moo_user_tools_prefs_page_new ());
    _moo_plugin_attach_prefs (GTK_WIDGET (dialog));
#endif

    g_signal_connect_after (dialog, "apply",
                            G_CALLBACK (prefs_dialog_apply),
                            NULL);

    return GTK_WIDGET (dialog);
}
