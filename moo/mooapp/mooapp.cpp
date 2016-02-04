/*
 *   mooapp/mooapp.c
 *
 *   Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
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

/**
 * class:MooApp: (parent GObject): application object
 */

#include "config.h"

#include "mooapp-private.h"
#include "eggsmclient/eggsmclient.h"
#include "mooapp-accels.h"
#include "mooapp-info.h"
#include "mooappabout.h"
#include "moolua/medit-lua.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mooeditor.h"
#include "mooedit/mooplugin.h"
#include "mooedit/mooeditfileinfo.h"
#include "mooedit/mooedit-enums.h"
#include "mooutils/mooprefsdialog.h"
#include "marshals.h"
#include "mooutils/mooappinput.h"
#include "mooutils/moodialogs.h"
#include "mooutils/moostock.h"
#include "mooutils/mooutils-fs.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooutils-debug.h"
#include "mooutils/mooi18n.h"
#include "mooutils/moo-mime.h"
#include "mooutils/moohelp.h"
#include "mooutils/moocompat.h"
#include "mooutils/mooutils-script.h"
#include <mooglib/moo-glib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef MOO_USE_QUARTZ
#include <ige-mac-dock.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

using namespace moo;

#define MOO_UI_XML_FILE     "ui.xml"
#ifdef __WIN32__
#define MOO_ACTIONS_FILE    "actions.ini"
#else
#define MOO_ACTIONS_FILE    "actions"
#endif

#define SESSION_VERSION "1.0"

#define ASK_OPEN_BUG_URL_KEY "Application/ask_open_bug_url"

struct App::Private
{
    static App *instance;
    static bool atexit_installed;
    static volatile int signal_received;

    Private(App& app) : app(app) {}
    Private(const Private&) = delete;
    Private& operator=(const Private&) = delete;

    App&                    app;

    gobj_ptr<MooEditor>     editor;
    gstr                    rc_files[2];

    bool                    run_input = false;
    gstr                    instance_name;

    bool                    running = false;
    bool                    in_try_quit = false;
    bool                    saved_session_in_try_quit = false;
    bool                    in_after_close_window = false;
    int                     exit_status = 0;

#ifndef __WIN32__
    EggSMClient*            sm_client = nullptr;
#endif

    int                     use_session = 0;
    gstr                    session_file;
    gref_ptr<MooMarkupDoc>  session;

    gobj_ptr<MooUiXml>      ui_xml;

    guint                   quit_handler_id = 0;

#ifdef MOO_USE_QUARTZ
    IgeMacDock *dock = nullptr;
#endif

    MooUiXml*       get_ui_xml                  ();

    bool            try_quit                    ();
    void            do_quit                     ();
    static gboolean on_gtk_main_quit            (Private* self);
    static gboolean check_signal                ();
#ifndef __WIN32__
    static void     sm_quit_requested           (Private* self);
    static void     sm_quit                     (Private* self);
#endif // __WIN32__

    static void     install_common_actions      ();
    static void     install_editor_actions      ();

    void            exec_cmd                    (char cmd, const char* data, guint len);
    void            do_load_session             (MooMarkupNode* xml);

    void            load_prefs                  ();
    void            save_prefs                  ();

    void            save_session                ();
    void            write_session               ();

    static void     install_cleanup             ();
    static void     cleanup                     ();

    void            start_input                 ();
    static void     input_callback              (char cmd, const char *data, gsize len, gpointer cb_data);

    void            cmd_open_files              (const char* data);

    void            init_ui                     ();
    void            init_mac                    ();
    void            init_editor                 ();

    static void     prefs_dialog                (GtkWidget* parent);
    GtkWidget*      create_prefs_dialog         ();

    static void     open_help                   (GtkWidget* window);
    static void     report_bug                  (GtkWidget* window);
    static void     prefs_dialog_apply          ();

    static void     editor_will_close_window    (Private* self);
    static void     editor_after_close_window   (Private* self);
};


App* App::Private::instance;
bool App::Private::atexit_installed;
volatile int App::Private::signal_received;


G_DEFINE_TYPE (MooApp, moo_app, G_TYPE_OBJECT);


enum {
    STARTED,
    QUIT,
    LOAD_SESSION,
    SAVE_SESSION,
    LAST_SIGNAL
};


static guint signals[LAST_SIGNAL];


static void
moo_app_class_init (MooAppClass *klass)
{
    moo::init_gobj_system ();

    /**
     * MooApp::started:
     *
     * @app: the object which received the signal
     *
     * This signal is emitted after application loaded session,
     * started main loop, and hit idle for the first time.
     **/
    signals[STARTED] =
            g_signal_new ("started",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooAppClass, started),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    /**
     * MooApp::quit:
     *
     * @app: the object which received the signal
     *
     * This signal is emitted when application quits,
     * after session has been saved.
     **/
    signals[QUIT] =
            g_signal_new ("quit",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooAppClass, quit),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    /**
     * MooApp::load-session:
     *
     * @app: the object which received the signal
     *
     * This signal is emitted when application is loading session,
     * after editor session has been loaded.
     **/
    signals[LOAD_SESSION] =
            g_signal_new ("load-session",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooAppClass, load_session),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    /**
     * MooApp::save-session:
     *
     * @app: the object which received the signal
     *
     * This signal is emitted when application is saving session,
     * before saving editor session.
     **/
    signals[SAVE_SESSION] =
            g_signal_new ("save-session",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooAppClass, save_session),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);
}


App::App(gobj_wrapper_data& d, const StartupOptions& opts)
    : Super(d)
    , p(nullptr)
{
    g_return_if_fail (Private::instance == nullptr);
    Private::instance = this;

    _moo_stock_init ();

    p = new Private(*this);
    p->run_input = opts.run_input;
    p->use_session = opts.use_session;
    p->instance_name = opts.instance_name;

#if defined(HAVE_SIGNAL) && defined(SIGINT)
    setup_signals (sigint_handler);
#endif
    Private::install_cleanup ();

    Private::install_common_actions ();
    Private::install_editor_actions ();
}

App::~App()
{
    p->do_quit ();
    Private::instance = nullptr;
    delete p;
}

static void
moo_app_init (MooApp *app)
{
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


App& App::instance()
{
    return *Private::instance;
}


/**
 * moo_app_instance: (static-method-of MooApp)
 **/
MooApp *
moo_app_instance (void)
{
    return App::instance().gobj();
}


#define SCRIPT_PREFIX_LUA "lua:"
#define SCRIPT_PREFIX_LUA_FILE "luaf:"
#define SCRIPT_PREFIX_PYTHON "py:"
#define SCRIPT_PREFIX_PYTHON_FILE "pyf:"

void App::run_script (const char* script)
{
    g_return_if_fail (script != NULL);

    if (g_str_has_prefix (script, SCRIPT_PREFIX_LUA))
        medit_lua_run_string (script + strlen (SCRIPT_PREFIX_LUA));
    else if (g_str_has_prefix (script, SCRIPT_PREFIX_LUA_FILE))
        medit_lua_run_file (script + strlen (SCRIPT_PREFIX_LUA_FILE));
//     else if (g_str_has_prefix (script, SCRIPT_PREFIX_PYTHON))
//         moo_python_run_string (script + strlen (SCRIPT_PREFIX_PYTHON));
//     else if (g_str_has_prefix (script, SCRIPT_PREFIX_PYTHON_FILE))
//         moo_python_run_file (script + strlen (SCRIPT_PREFIX_PYTHON_FILE));
    else
        medit_lua_run_string (script);
}

// static void
// run_python_file (MooApp     *app,
//                  const char *filename)
// {
//     FILE *file;
//     MooPyObject *res;
//
//     g_return_if_fail (MOO_IS_APP (app));
//     g_return_if_fail (filename != NULL);
//     g_return_if_fail (moo_python_running ());
//
//     file = _moo_fopen (filename, "rb");
//     g_return_if_fail (file != NULL);
//
//     res = moo_python_run_file (file, filename, NULL, NULL);
//
//     fclose (file);
//
//     if (res)
//         moo_Py_DECREF (res);
//     else
//         moo_PyErr_Print ();
// }
//
// static void
// run_python_script (const char *string)
// {
//     MooPyObject *res;
//
//     g_return_if_fail (string != NULL);
//     g_return_if_fail (moo_python_running ());
//
//     res = moo_python_run_simple_string (string);
//
//     if (res)
//         moo_Py_DECREF (res);
//     else
//         moo_PyErr_Print ();
// }


/**
 * moo_app_get_editor:
 */
MooEditor *
moo_app_get_editor (MooApp *app)
{
    g_return_val_if_fail(MOO_IS_APP(app), nullptr);
    return App::get(*app).get_editor();
}


MooEditor* App::get_editor()
{
    return p->editor.gobj();
}


void App::Private::editor_will_close_window (App::Private* self)
{
    MooEditWindowArray *windows;

    if (!self->running || self->saved_session_in_try_quit)
        return;

    windows = moo_editor_get_windows (self->editor.gobj());

    if (moo_edit_window_array_get_size (windows) == 1)
        self->save_session ();

    moo_edit_window_array_free (windows);
}

void App::Private::editor_after_close_window (App::Private* self)
{
    MooEditWindowArray *windows;

    if (!self->running || self->in_try_quit)
        return;

    windows = moo_editor_get_windows (self->editor.gobj());

    if (moo_edit_window_array_get_size (windows) == 0)
    {
        self->in_after_close_window = TRUE;
        self->app.quit ();
        self->in_after_close_window = FALSE;
    }

    moo_edit_window_array_free (windows);
}

void App::Private::init_editor ()
{
    editor.set_new (moo_editor_create_instance ());

    editor->connect_swapped ("will-close-window",
                             G_CALLBACK(editor_will_close_window),
                             this);
    editor->connect_swapped ("after-close-window",
                             G_CALLBACK(editor_after_close_window),
                             this);

    moo_editor_set_ui_xml (editor.gobj(), get_ui_xml ());

    app.init_plugins ();
}


void App::Private::init_ui ()
{
    gobj_ptr<MooUiXml> xml;
    char **files, **p;

    files = moo_get_data_files (MOO_UI_XML_FILE);

    for (p = files; p && *p; ++p)
    {
        GError *error = NULL;
        GMappedFile *file;

        file = g_mapped_file_new (*p, FALSE, &error);

        if (file)
        {
            xml.set_new (moo_ui_xml_new ());
            moo_ui_xml_add_ui_from_string (xml.gobj(),
                                           g_mapped_file_get_contents (file),
                                           g_mapped_file_get_length (file));
            g_mapped_file_unref (file);
            break;
        }

        if (!(error && error->domain == G_FILE_ERROR && error->code == G_FILE_ERROR_NOENT))
            g_warning ("could not open file '%s': %s", *p, moo_error_message (error));

        g_error_free (error);
    }

    if (xml)
        ui_xml = xml;

    g_strfreev (files);
}


#ifdef MOO_USE_QUARTZ

static void
dock_open_documents (App* app, char** files)
{
    app->open_files (files, 0, 0, 0);
}

static void
dock_quit_activate (App *app)
{
    app->quit ();
}

void App::Private::init_mac ()
{
    dock = ige_mac_dock_get_default ();
    g_signal_connect_swapped (dock, "open-documents",
                              G_CALLBACK (dock_open_documents), &app);
    g_signal_connect_swapped (dock, "quit-activate",
                              G_CALLBACK (dock_quit_activate), &app);
}

#else /* !MOO_USE_QUARTZ */
void App::Private::init_mac ()
{
}
#endif /* !MOO_USE_QUARTZ */


void App::Private::input_callback (char        cmd,
                                   const char *data,
                                   gsize       len,
                                   gpointer    cb_data)
{
    App::Private* self = reinterpret_cast<App::Private*> (cb_data);

    g_return_if_fail (self != nullptr);
    g_return_if_fail (data != nullptr);

    self->exec_cmd (cmd, data, len);
}

void App::Private::start_input ()
{
    if (run_input)
        _moo_app_input_start (instance_name, TRUE, input_callback, this);
}


gboolean App::Private::on_gtk_main_quit (App::Private* self)
{
    self->quit_handler_id = 0;

    if (!self->app.quit())
        self->do_quit ();

    return FALSE;
}


gboolean App::Private::check_signal ()
{
    if (signal_received)
    {
        g_print ("%s\n", g_strsignal (signal_received));
        if (instance)
            instance->p->do_quit();
        exit (EXIT_FAILURE);
    }

    return TRUE;
}


static gboolean
emit_started (App *app)
{
    app->signal_emit_by_name ("started");
    return FALSE;
}

#ifndef __WIN32__

void App::Private::sm_quit_requested (App::Private* self)
{
    EggSMClient *sm_client;

    sm_client = self->sm_client;
    g_return_if_fail (sm_client != NULL);

    g_object_ref (sm_client);
    egg_sm_client_will_quit (sm_client, self->app.quit());
    g_object_unref (sm_client);
}

void App::Private::sm_quit (App::Private* self)
{
    if (!self->app.quit())
        self->do_quit (app);
}

#endif // __WIN32__


void App::set_exit_status (int value)
{
    p->exit_status = value;
}


void App::Private::install_cleanup ()
{
    if (!atexit_installed)
    {
        atexit_installed = TRUE;
        atexit (cleanup);
    }
}

void App::Private::cleanup ()
{
    _moo_app_input_shutdown ();
    moo_mime_shutdown ();
    moo_cleanup ();
}


void App::Private::do_quit ()
{
    guint i;

    if (!running)
        return;

    running = FALSE;

    app.signal_emit (signals[QUIT], 0);

#ifndef __WIN32__
    g_object_unref (sm_client);
    sm_client = NULL;
#endif

    _moo_editor_close_all (editor.gobj());

    moo_plugin_shutdown ();

    editor.reset ();

    write_session ();
    save_prefs ();

    if (quit_handler_id)
        gtk_quit_remove (quit_handler_id);

    i = 0;
    while (gtk_main_level () && i < 1000)
    {
        gtk_main_quit ();
        i++;
    }

    cleanup ();
}


bool App::init()
{
    gdk_set_program_class (MOO_APP_FULL_NAME);
    gtk_window_set_default_icon_name (MOO_APP_SHORT_NAME);

    moo_set_display_app_name (MOO_APP_SHORT_NAME);
    _moo_set_app_instance_name (p->instance_name);

    p->load_prefs ();
    p->init_ui ();
    p->init_mac ();

    p->init_editor ();

    if (p->use_session == -1)
        p->use_session = moo_prefs_get_bool (moo_edit_setting (MOO_EDIT_PREFS_SAVE_SESSION));

    if (p->use_session)
        p->run_input = true;

    p->start_input ();

    return TRUE;
}


int App::run()
{
    g_return_val_if_fail (!p->running, 0);

    p->running = TRUE;

    p->quit_handler_id = gtk_quit_add (1, (GtkFunction) Private::on_gtk_main_quit, p);

    g_timeout_add (100, (GSourceFunc) App::Private::check_signal, NULL);

#ifndef __WIN32__
    p->sm_client = egg_sm_client_get ();
    /* make it install log handler */
    g_option_group_free (egg_sm_client_get_option_group ());
    g_signal_connect_swapped (p->sm_client, "quit-requested",
                              G_CALLBACK (sm_quit_requested), p);
    g_signal_connect_swapped (p->sm_client, "quit",
                              G_CALLBACK (sm_quit), p);

    if (EGG_SM_CLIENT_GET_CLASS (p->sm_client)->startup)
        EGG_SM_CLIENT_GET_CLASS (p->sm_client)->startup (p->sm_client, NULL);
#endif // __WIN32__

    g_idle_add_full (G_PRIORITY_DEFAULT_IDLE + 1, (GSourceFunc) emit_started, this, NULL);

    gtk_main ();

    return p->exit_status;
}


bool App::Private::try_quit()
{
    gboolean closed;

    if (!running)
        return TRUE;

    in_try_quit = TRUE;

    if (!in_after_close_window)
    {
        saved_session_in_try_quit = TRUE;
        save_session ();
    }

    closed = _moo_editor_close_all (editor.gobj());

    saved_session_in_try_quit = FALSE;
    in_try_quit = FALSE;

    return closed;
}

bool App::quit()
{
    if (p->in_try_quit || !p->running)
        return TRUE;

    if (p->try_quit())
    {
        p->do_quit();
        return TRUE;
    }

    return FALSE;
}

/**
 * moo_app_quit:
 **/
gboolean
moo_app_quit (MooApp *app)
{
    g_return_val_if_fail (MOO_IS_APP (app), FALSE);
    return App::get(*app).quit();
}


void App::Private::install_common_actions()
{
    MooWindowClass *klass = MOO_WINDOW_CLASS (g_type_class_ref (MOO_TYPE_WINDOW));

    g_return_if_fail (klass != NULL);

    moo_window_class_new_action (klass, "Preferences", NULL,
                                 "display-name", GTK_STOCK_PREFERENCES,
                                 "label", GTK_STOCK_PREFERENCES,
                                 "tooltip", GTK_STOCK_PREFERENCES,
                                 "stock-id", GTK_STOCK_PREFERENCES,
                                 "closure-callback", prefs_dialog,
                                 NULL);

    moo_window_class_new_action (klass, "About", NULL,
                                 "label", GTK_STOCK_ABOUT,
                                 "no-accel", TRUE,
                                 "stock-id", GTK_STOCK_ABOUT,
                                 "closure-callback", App::about_dialog,
                                 NULL);

    moo_window_class_new_action (klass, "Help", NULL,
                                 "label", GTK_STOCK_HELP,
                                 "default-accel", MOO_APP_ACCEL_HELP,
                                 "stock-id", GTK_STOCK_HELP,
                                 "closure-callback", open_help,
                                 NULL);

    moo_window_class_new_action (klass, "ReportBug", NULL,
                                 "label", _("Report a Bug..."),
                                 "closure-callback", report_bug,
                                 NULL);

    moo_window_class_new_action (klass, "Quit", NULL,
                                 "display-name", GTK_STOCK_QUIT,
                                 "label", GTK_STOCK_QUIT,
                                 "tooltip", GTK_STOCK_QUIT,
                                 "stock-id", GTK_STOCK_QUIT,
                                 "default-accel", MOO_APP_ACCEL_QUIT,
                                 "closure-callback", moo_app_quit,
                                 "closure-proxy-func", moo_app_instance,
                                 NULL);

    g_type_class_unref (klass);
}


void App::Private::install_editor_actions ()
{
}


MooUiXml* App::Private::get_ui_xml ()
{
    if (!ui_xml)
    {
        if (editor)
            ui_xml.ref(moo_editor_get_ui_xml(editor.gobj()));

        if (!ui_xml)
            ui_xml.set_new(moo_ui_xml_new());
    }

    return ui_xml.gobj();
}


void App::Private::do_load_session (MooMarkupNode* xml)
{
    _moo_editor_load_session (editor.gobj(), xml);
    app.signal_emit (signals[LOAD_SESSION], 0);
}


void App::Private::save_session ()
{
    MooMarkupNode *root;

    if (session_file.empty())
        return;

    session.set_new (moo_markup_doc_new ("session"));
    root = moo_markup_create_root_element (session.gobj(), "session");
    moo_markup_set_prop (root, "version", SESSION_VERSION);

    app.signal_emit (signals[SAVE_SESSION], 0);
    _moo_editor_save_session (editor.gobj(), root);
}

void App::Private::write_session ()
{
    MooFileWriter *writer;

    if (session_file.empty())
        return;

    gstr filename = moo_get_user_cache_file (session_file);

    if (!session)
    {
        mgw_errno_t err;
        mgw_unlink (filename, &err);
        return;
    }

    gerrp error;

    if ((writer = moo_config_writer_new (filename, FALSE, error)))
    {
        moo_markup_write_pretty (session.gobj(), writer, 1);
        moo_file_writer_close (writer, error);
    }

    if (error)
        g_critical ("could not save session file %s: %s", filename, error->message);
}

void App::load_session ()
{
    MooMarkupNode *root;
    const char *version;

    if (!p->use_session)
        return;

    if (p->session_file.empty())
    {
        if (!p->instance_name.empty())
            p->session_file.set_new(g_strdup_printf(MOO_NAMED_SESSION_XML_FILE_NAME,
                                                    p->instance_name.get()));
        else
            p->session_file.set_const(MOO_SESSION_XML_FILE_NAME);
    }

    gstr session_file = moo_get_user_cache_file (p->session_file);

    gref_ptr<MooMarkupDoc> doc;
    gerrp error;

    if (!g_file_test (session_file, G_FILE_TEST_EXISTS) ||
        !(doc = gref_ptr<MooMarkupDoc>::wrap_new (moo_markup_parse_file (session_file, &error))))
    {
        if (error)
            g_warning ("could not open session file %s: %s",
                       session_file, error->message);

        return;
    }

    if (!(root = moo_markup_get_root_element (doc.gobj(), "session")) ||
        !(version = moo_markup_get_prop (root, "version")))
        g_warning ("malformed session file %s, ignoring", session_file);
    else if (strcmp (version, SESSION_VERSION) != 0)
        g_warning ("invalid session file version %s in %s, ignoring",
                   version, session_file);
    else
    {
        p->session = doc;
        p->do_load_session (root);
        p->session = nullptr;
    }
}


// static void
// moo_app_present (MooApp *app)
// {
//     gpointer window = NULL;
//
//     if (!window && app->priv->editor)
//         window = moo_editor_get_active_window (app->priv->editor);
//
//     if (window)
//         moo_window_present (window, 0);
// }


// static void
// moo_app_open_uris (MooApp     *app,
//                    const char *data,
//                    gboolean    has_encoding)
// {
//     char **uris;
//     guint32 stamp;
//     char *stamp_string;
//     char *line_string;
//     guint32 line;
//     char *encoding = NULL;
//
//     g_return_if_fail (strlen (data) > (has_encoding ? 32 : 16));
//
//     stamp_string = g_strndup (data, 8);
//     stamp = strtoul (stamp_string, NULL, 16);
//     line_string = g_strndup (data + 8, 8);
//     line = strtoul (line_string, NULL, 16);
//
//     if (line > G_MAXINT)
//         line = 0;
//
//     data += 16;
//
//     if (has_encoding)
//     {
//         char *p;
//         encoding = g_strndup (data, 16);
//         p = strchr (encoding, ' ');
//         if (p)
//             *p = 0;
//         data += 16;
//     }
//
//     uris = g_strsplit (data, "\r\n", 0);
//
//     if (uris && *uris)
//     {
//         char **p;
//
//         for (p = uris; p && *p && **p; ++p)
//         {
//             guint line_here = 0;
//             guint options = 0;
//             char *filename;
//
//             filename = _moo_edit_uri_to_filename (*p, &line_here, &options);
//
//             if (p != uris)
//                 line = 0;
//             if (line_here)
//                 line = line_here;
//
//             if (filename)
//                 moo_app_new_file (app, filename, encoding, line, options);
//
//             g_free (filename);
//         }
//     }
//     else
//     {
//         moo_app_new_file (app, NULL, encoding, 0, 0);
//     }
//
//     moo_editor_present (app->priv->editor, stamp);
//
//     g_free (encoding);
//     g_strfreev (uris);
//     g_free (stamp_string);
// }

void App::open_files (MooOpenInfoArray *files, guint32 stamp)
{
    if (!moo_open_info_array_is_empty (files))
    {
        guint i;
        MooOpenInfoArray *tmp = moo_open_info_array_copy (files);
        for (i = 0; i < tmp->n_elms; ++i)
            moo_open_info_add_flags (tmp->elms[i], MOO_OPEN_FLAG_CREATE_NEW);
        moo_editor_open_files (p->editor.gobj(), tmp, NULL, NULL);
        moo_open_info_array_free (tmp);
    }

    moo_editor_present (p->editor.gobj(), stamp);
}


static MooAppCmdCode
get_cmd_code (char cmd)
{
    guint i;

    for (i = 1; i < CMD_LAST; ++i)
        if (cmd == moo_app_cmd_chars[i])
            return MooAppCmdCode (i);

    g_return_val_if_reached (MooAppCmdCode (0));
}

void App::Private::exec_cmd (char cmd,
                             const char* data,
                             G_GNUC_UNUSED guint len)
{
    MooAppCmdCode code = get_cmd_code (cmd);

    switch (code)
    {
        case CMD_SCRIPT:
            app.run_script (data);
            break;

        case CMD_OPEN_FILES:
            cmd_open_files (data);
            break;

        default:
            g_warning ("got unknown command %c %d", cmd, code);
    }
}


void App::Private::open_help (GtkWidget *window)
{
    GtkWidget *focus = gtk_window_get_focus (GTK_WINDOW (window));
    moo_help_open_any (focus ? focus : window);
}


void App::Private::report_bug (GtkWidget *window)
{
    gboolean do_open = TRUE;

    moo_prefs_create_key (ASK_OPEN_BUG_URL_KEY, MOO_PREFS_STATE, G_TYPE_STRING, NULL);

    gstr version_escaped = g::uri_escape_string (MOO_DISPLAY_VERSION);

    gstr os = get_system_name ();
    if (!os.empty())
        os = g::uri_escape_string (os);

    gstr url = gstr::printf ("http://mooedit.sourceforge.net/cgi-bin/report_bug.cgi?version=%s&os=%s",
                             version_escaped.get(), os.get());
    gstr message = gstr::printf (_("The following URL will be opened:\n\n%s\n\n"
                                 "It contains medit version and your operating system name (%s)"),
                                 url.get(), os.get());

    const char *prefs_val = moo_prefs_get_string (ASK_OPEN_BUG_URL_KEY);
    if (!prefs_val || strcmp (prefs_val, url) != 0)
    {
        do_open = moo_question_dialog (_("Open URL?"), message, window, GTK_RESPONSE_OK);
        if (do_open)
            moo_prefs_set_string (ASK_OPEN_BUG_URL_KEY, url);
    }

    if (do_open)
        moo_open_url (url);
}


void App::Private::save_prefs ()
{
    gerrp error;

    if (!moo_prefs_save (rc_files[MOO_PREFS_RC],
                         rc_files[MOO_PREFS_STATE],
                         error))
    {
        g_warning ("could not save config files: %s", moo_error_message (error));
    }
}

void App::Private::prefs_dialog_apply ()
{
    instance->p->save_prefs();
}


GtkWidget* App::Private::create_prefs_dialog ()
{
    MooPrefsDialog *dialog;

    /* Prefs dialog title */
    dialog = MOO_PREFS_DIALOG (moo_prefs_dialog_new (_("Preferences")));

    moo_prefs_dialog_append_page (dialog, moo_edit_prefs_page_new_1 (editor.gobj()));
    moo_prefs_dialog_append_page (dialog, moo_edit_prefs_page_new_2 (editor.gobj()));
    moo_prefs_dialog_append_page (dialog, moo_edit_prefs_page_new_3 (editor.gobj()));
    moo_prefs_dialog_append_page (dialog, moo_edit_prefs_page_new_4 (editor.gobj()));
    moo_prefs_dialog_append_page (dialog, moo_edit_prefs_page_new_5 (editor.gobj()));
    moo_plugin_attach_prefs (GTK_WIDGET (dialog));

    g_signal_connect_after (dialog, "apply",
                            G_CALLBACK (prefs_dialog_apply),
                            NULL);

    return GTK_WIDGET (dialog);
}


void App::Private::prefs_dialog (GtkWidget *parent)
{
    g_return_if_fail (instance != nullptr);
    GtkWidget *dialog = instance->p->create_prefs_dialog ();
    g_return_if_fail (MOO_IS_PREFS_DIALOG (dialog));
    moo_prefs_dialog_run (MOO_PREFS_DIALOG (dialog), parent);
}


void App::Private::load_prefs ()
{
    gerrp error;
    char **sys_files;

    rc_files[MOO_PREFS_RC].set_new(moo_get_user_data_file (MOO_PREFS_XML_FILE_NAME));
    rc_files[MOO_PREFS_STATE].set_new(moo_get_user_cache_file (MOO_STATE_XML_FILE_NAME));

    sys_files = moo_get_sys_data_files (MOO_PREFS_XML_FILE_NAME);

    if (!moo_prefs_load (sys_files,
                         rc_files[MOO_PREFS_RC],
                         rc_files[MOO_PREFS_STATE],
                         error))
    {
        g_warning ("could not read config files: %s", moo_error_message (error));
    }

    g_strfreev (sys_files);
}


#define MOO_APP_CMD_VERSION "1.0"

static MooOpenInfoArray *
moo_app_parse_files (const char      *data,
                     guint32         *stamp)
{
    MooMarkupDoc *xml;
    MooMarkupNode *root;
    MooMarkupNode *node;
    const char *version;
    MooOpenInfoArray *files;

    *stamp = 0;

    xml = moo_markup_parse_memory (data, -1, NULL);
    g_return_val_if_fail (xml != NULL, FALSE);

    if (!(root = moo_markup_get_root_element (xml, "moo-app-open-files")) ||
        !(version = moo_markup_get_prop (root, "version")) ||
        strcmp (version, MOO_APP_CMD_VERSION) != 0)
    {
        g_warning ("%s: invalid markup", G_STRFUNC);
        moo_markup_doc_unref (xml);
        return NULL;
    }

    *stamp = moo_markup_uint_prop (root, "stamp", 0);
    files = moo_open_info_array_new ();

    for (node = root->children; node != NULL; node = node->next)
    {
        const char *uri;
        const char *encoding;
        MooOpenInfo *info;
        int line;

        if (!MOO_MARKUP_IS_ELEMENT (node))
            continue;

        if (strcmp (node->name, "file") != 0 ||
            !(uri = moo_markup_get_content (node)) ||
            !uri[0])
        {
            g_critical ("%s: oops", G_STRFUNC);
            continue;
        }

        encoding = moo_markup_get_prop (node, "encoding");
        if (!encoding || !encoding[0])
            encoding = NULL;

        info = moo_open_info_new_uri (uri, encoding, -1, MOO_OPEN_FLAG_CREATE_NEW);

        line = moo_markup_int_prop (node, "line", 0);
        if (line > 0)
            moo_open_info_set_line (info, line - 1);

        if (moo_markup_bool_prop (node, "new-window", FALSE))
            moo_open_info_add_flags (info, MOO_OPEN_FLAG_NEW_WINDOW);
        if (moo_markup_bool_prop (node, "new-tab", FALSE))
            moo_open_info_add_flags (info, MOO_OPEN_FLAG_NEW_TAB);
        if (moo_markup_bool_prop (node, "reload", FALSE))
            moo_open_info_add_flags (info, MOO_OPEN_FLAG_RELOAD);

        moo_open_info_array_take (files, info);
    }

    moo_markup_doc_unref (xml);
    return files;
}

void App::Private::cmd_open_files (const char *data)
{
    MooOpenInfoArray *files;
    guint32 stamp;
    files = moo_app_parse_files (data, &stamp);
    app.open_files (files, stamp);
    moo_open_info_array_free (files);
}

G_GNUC_PRINTF(2, 3) static void
append_escaped (GString *str, const char *format, ...)
{
    va_list args;
    char *escaped;

    va_start (args, format);

    escaped = g_markup_vprintf_escaped (format, args);
    g_string_append (str, escaped);
    g_free (escaped);

    va_end (args);
}

bool App::send_files (MooOpenInfoArray *files,
                      guint32           stamp,
                      const char       *pid)
{
    gboolean result;
    GString *msg;
    int i, c;

#if 0
    _moo_message ("moo_app_send_files: got %d files to pid %s",
                  n_files, pid ? pid : "NONE");
#endif

    msg = g_string_new (NULL);
    g_string_append_printf (msg, "%s<moo-app-open-files version=\"%s\" stamp=\"%u\">",
                            CMD_OPEN_FILES_S, MOO_APP_CMD_VERSION, stamp);

    for (i = 0, c = moo_open_info_array_get_size (files); i < c; ++i)
    {
        MooOpenInfo *info = files->elms[i];
        const char *encoding = moo_open_info_get_encoding (info);
        int line = moo_open_info_get_line (info);
        MooOpenFlags flags = moo_open_info_get_flags (info);
        char *uri;

        g_string_append (msg, "<file");

        if (encoding)
            g_string_append_printf (msg, " encoding=\"%s\"", encoding);
        if (line >= 0)
            g_string_append_printf (msg, " line=\"%u\"", (guint) line + 1);
        if (flags & MOO_OPEN_FLAG_NEW_WINDOW)
            g_string_append_printf (msg, " new-window=\"true\"");
        if (flags & MOO_OPEN_FLAG_NEW_TAB)
            g_string_append_printf (msg, " new-tab=\"true\"");
        if (flags & MOO_OPEN_FLAG_RELOAD)
            g_string_append_printf (msg, " reload=\"true\"");

        uri = moo_open_info_get_uri (info);
        append_escaped (msg, ">%s</file>", uri);
        g_free (uri);
    }

    g_string_append (msg, "</moo-app-open-files>");

    result = _moo_app_input_send_msg (pid, msg->str, msg->len);

    g_string_free (msg, TRUE);
    return result;
}

bool App::send_msg(const char* pid, const char* data, gssize len)
{
    return _moo_app_input_send_msg (pid, data, len);
}
