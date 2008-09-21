/*
 *   medit-app.c
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
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

#include <config.h>
#include "mooapp/mooapp.h"
#include "mooedit/mooplugin.h"
#include "moopython/moopython-builtin.h"
#include "mooutils/mooi18n.h"
#include "mooutils/mooutils-fs.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mootype-macros.h"
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "mem-debug.h"
#include "run-tests.h"

static struct MeditOpts {
    int use_session;
    int pid;
    gboolean new_app;
    const char *app_name;
    gboolean new_window;
    gboolean new_tab;
    gboolean reload;
    const char *project;
    gboolean project_mode;
    int line;
    const char *encoding;
    const char *log_file;
    gboolean log_window;
    const char *exec_string;
    const char *exec_file;
    char **files;
    gboolean show_version;
    const char *debug;
} medit_opts = { -1, -1 };

#include "parse.h"

typedef MooApp MeditApp;
typedef MooAppClass MeditAppClass;
MOO_DEFINE_TYPE_STATIC (MeditApp, medit_app, MOO_TYPE_APP)

static void
medit_app_init_plugins (G_GNUC_UNUSED MooApp *app)
{
#ifdef MOO_PYTHON_BUILTIN
    _moo_python_builtin_init ();
#endif
}

static void
medit_app_class_init (MooAppClass *klass)
{
    klass->init_plugins = medit_app_init_plugins;
}

static void
medit_app_init (G_GNUC_UNUSED MooApp *app)
{
}

static gboolean
parse_use_session (const char *option_name,
                   const char *value,
                   G_GNUC_UNUSED gpointer data,
                   GError **error)
{
    if (!value || strcmp (value, "yes") == 0)
    {
        medit_opts.use_session = TRUE;
        return TRUE;
    }
    else if (strcmp (value, "no") == 0)
    {
        medit_opts.use_session = FALSE;
        return TRUE;
    }
    else
    {
        g_set_error (error, G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE,
                     /* error message for wrong commmand line */
                     _("Invalid value '%s' for option %s"), value, option_name);
        return FALSE;
    }
}

static GOptionEntry medit_options[] = {
    { "new-app", 'n', 0, G_OPTION_ARG_NONE, &medit_opts.new_app,
            /* help message for command line option --new-app */ N_("Run new instance of application"), NULL },

#if !GLIB_CHECK_VERSION(2,8,0)
#define G_OPTION_FLAG_OPTIONAL_ARG 0
#endif
    { "use-session", 's', G_OPTION_FLAG_OPTIONAL_ARG, G_OPTION_ARG_CALLBACK, (void*) parse_use_session,
            /* help message for command line option --use-session */ N_("Load and save session"), "yes|no" },
#if !GLIB_CHECK_VERSION(2,8,0)
#undef G_OPTION_FLAG_OPTIONAL_ARG
#endif

    { "pid", 0, 0, G_OPTION_ARG_INT, &medit_opts.pid,
            /* help message for command line option --pid=PID */ N_("Use existing instance with process id PID"),
            /* "PID" part in "--pid=PID" */ N_("PID") },
    { "app-name", 0, 0, G_OPTION_ARG_STRING, &medit_opts.app_name,
            /* help message for command line option --app-name=NAME */ N_("Set instance name to NAME if it's not already running"),
            /* "NAME" part in "--app-name=NAME" */ N_("NAME") },
    { "new-window", 'w', 0, G_OPTION_ARG_NONE, &medit_opts.new_window,
            /* help message for command line option --new-window */ N_("Open file(s) in a new window"), NULL },
    { "new-tab", 't', 0, G_OPTION_ARG_NONE, &medit_opts.new_tab,
            /* help message for command line option --new-tab */ N_("Open file(s) in a new tab"), NULL },
    { "project", 0, G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_FILENAME, &medit_opts.project,
            "Open project file FILE", "FILE" },
    { "project-mode", 0, G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_NONE, &medit_opts.project_mode,
            "IDE mode", NULL },
    { "line", 'l', 0, G_OPTION_ARG_INT, &medit_opts.line,
            /* help message for command line option --line=LINE */ N_("Open file and position cursor on line LINE"),
            /* "LINE" part in --line=LINE */ N_("LINE") },
    { "encoding", 'e', 0, G_OPTION_ARG_STRING, &medit_opts.encoding,
            /* help message for command line option --encoding=ENCODING */ N_("Use provided character encoding"),
            /* "ENCODING" part in --encoding=ENCODING */ N_("ENCODING") },
    { "reload", 'r', 0, G_OPTION_ARG_NONE, &medit_opts.reload,
            /* help message for command line option --reload */ N_("Automatically reload file if it was modified on disk"), NULL },
    { "log-window", 0, G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_NONE, &medit_opts.log_window,
            "Show debug output", NULL },
    { "log-file", 0, G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_FILENAME, &medit_opts.log_file,
            "Write debug output to FILE", "FILE" },
    { "debug", 0, G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_STRING, &medit_opts.debug,
            "Run in debug mode", NULL },
    { "exec", 0, 0, G_OPTION_ARG_STRING, &medit_opts.exec_string,
            /* help message for command line option --exec CODE */ N_("Execute python code in an existing instance"),
            /* "CODE" part in --exec CODE */ N_("CODE") },
    { "exec-file", 0, 0, G_OPTION_ARG_FILENAME, &medit_opts.exec_file,
            /* help message for command line option --exec-file FILE */ N_("Execute python file in an existing instance"),
            /* "FILE" part in --exec-file FILE */ N_("FILE") },
    { "version", 0, 0, G_OPTION_ARG_NONE, &medit_opts.show_version,
            /* help message for command line option --version */ N_("Show version information and exit"), NULL },
    { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &medit_opts.files,
            NULL, /* "FILES" part in "medit [OPTION...] [FILES]" */ N_("FILES") },
    { NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL }
};

static gboolean
post_parse_func (void)
{
    if (medit_opts.show_version)
    {
        g_print ("medit " MOO_VERSION "\n");
        exit (0);
    }

    if (medit_opts.pid > 0 && medit_opts.app_name)
    {
        /* error message for wrong commmand line */
        g_printerr (_("%s and %s options may not be used simultaneously\n"),
                    "--app-name", "--pid");
        exit (EXIT_FAILURE);
    }

    if (medit_opts.exec_string && medit_opts.exec_file)
    {
        /* error message for wrong commmand line */
        g_printerr (_("%s and %s options may not be used simultaneously\n"),
                    "--exec", "--exec-file");
        exit (EXIT_FAILURE);
    }

    if (medit_opts.debug)
        g_setenv ("MOO_DEBUG", medit_opts.debug, FALSE);

    if (medit_opts.project)
        medit_opts.project_mode = TRUE;

    return TRUE;
}

static GOptionContext *
parse_args (int argc, char *argv[])
{
    GOptionContext *ctx;
    GOptionGroup *grp;
    GError *error = NULL;

    grp = g_option_group_new ("medit", "medit", "medit", NULL, NULL);
    g_option_group_add_entries (grp, medit_options);
    g_option_group_set_parse_hooks (grp, NULL, (GOptionParseFunc) post_parse_func);
    g_option_group_set_translation_domain (grp, GETTEXT_PACKAGE);

    ctx = g_option_context_new (NULL);
    g_option_context_set_main_group (ctx, grp);
    g_option_context_add_group (ctx, gtk_get_option_group (FALSE));

    if (!g_option_context_parse (ctx, &argc, &argv, &error))
    {
        g_printerr ("%s\n", error->message);
        exit (EXIT_FAILURE);
    }

    return ctx;
}

static void
notify_startup_complete (void)
{
#ifdef GDK_WINDOWING_X11
    const char *v = g_getenv ("DESKTOP_STARTUP_ID");

    if (v && *v)
    {
        gtk_init (NULL, NULL);
        gdk_notify_startup_complete ();
    }
#endif
}

static guint32
get_time_stamp (void)
{
#ifdef GDK_WINDOWING_X11
    const char *startup_id;
    char *time_str, *end;
    gulong stamp;

    startup_id = g_getenv ("DESKTOP_STARTUP_ID");
    if (!startup_id || !startup_id[0])
        return 0;

    if (!(time_str = g_strrstr (startup_id, "_TIME")))
        return 0;

    errno = 0;
    time_str += 5;
    stamp = strtoul (time_str, &end, 0);

    return !*end && !errno ? stamp : 0;
#else
    return 0;
#endif
}

static void
push_appdir_to_path (void)
{
#ifdef __WIN32__
    char *appdir;
    const char *path;
    char *new_path;

    appdir = moo_win32_get_app_dir ();
    g_return_if_fail (appdir != NULL);

    path = g_getenv ("Path");

    if (path)
        new_path = g_strdup_printf ("%s;%s", appdir, path);
    else
        new_path = g_strdup (appdir);

    g_setenv ("Path", new_path, TRUE);

    g_free (new_path);
    g_free (appdir);
#endif
}

static void
project_mode (const char *file)
{
    MooPlugin *plugin;

    plugin = (MooPlugin*) moo_plugin_lookup ("ProjectManager");

    if (!plugin)
    {
        fputs ("Could not initialize project manager plugin\n", stderr);
        exit (EXIT_FAILURE);
    }

    if (file)
    {
        char *project = moo_filename_from_locale (file);
        g_object_set (plugin, "project", project, NULL);
        g_free (project);
    }

    moo_plugin_set_enabled (plugin, TRUE);
}

static int
medit_main (int argc, char *argv[])
{
    MooApp *app = NULL;
    MooEditor *editor;
    int retval;
    gboolean new_instance = FALSE;
    gboolean run_input = TRUE;
    guint32 stamp;
    const char *name = NULL;
    char pid_buf[32];
    GOptionContext *ctx;
    MooAppFileInfo *files;
    int n_files;

    init_mem_stuff ();
    g_thread_init (NULL);
    g_set_prgname ("medit");

    ctx = parse_args (argc, argv);

#if !GLIB_CHECK_VERSION(2,8,0)
    g_set_prgname ("medit");
#endif

    stamp = get_time_stamp ();

#if 0
    gdk_window_set_debug_updates (TRUE);
#endif

#if 0
    g_idle_add_full (G_PRIORITY_LOW, (GSourceFunc) exit, NULL, NULL);
#endif

    if (medit_opts.new_app || medit_opts.project_mode)
        new_instance = TRUE;

    run_input = !medit_opts.new_app || medit_opts.app_name ||
                 medit_opts.use_session == 1 || medit_opts.project_mode;

    if (medit_opts.pid > 0)
    {
        sprintf (pid_buf, "%d", medit_opts.pid);
        name = pid_buf;
    }
    else if (medit_opts.app_name)
        name = medit_opts.app_name;
    else if (!medit_opts.new_app)
        name = g_getenv ("MEDIT_PID");

    if (name && !name[0])
        name = NULL;

    if (medit_opts.exec_string || medit_opts.exec_file)
    {
        GString *msg;
        msg = g_string_new (medit_opts.exec_string ? "p" : "P");
        g_string_append (msg, medit_opts.exec_string ? medit_opts.exec_string : medit_opts.exec_file);
        moo_app_send_msg (name, msg->str, msg->len + 1);
        notify_startup_complete ();
        exit (0);
    }

    parse_files (&files, &n_files);

    if (name)
    {
        if (moo_app_send_files (files, n_files, stamp, name))
            exit (0);

        if (!medit_opts.app_name)
        {
            g_printerr ("Could not send files to instance '%s'\n", name);
            exit (EXIT_FAILURE);
        }
    }

    if (!new_instance && !medit_opts.app_name &&
         moo_app_send_files (files, n_files, stamp, NULL))
    {
        notify_startup_complete ();
        exit (0);
    }

    push_appdir_to_path ();

    gtk_init (NULL, NULL);
    gdk_threads_init ();
    gdk_threads_enter ();

    if (medit_opts.log_file || medit_opts.log_window)
    {
        if (medit_opts.log_file)
            moo_set_log_func_file (medit_opts.log_file);
        else
            moo_set_log_func_window (TRUE);
    }

    app = MOO_APP (g_object_new (medit_app_get_type (),
                                 "run-input", run_input,
                                 "use-session", medit_opts.use_session,
                                 "instance-name", medit_opts.app_name,
                                 NULL));

    if (!moo_app_init (app))
    {
        gdk_notify_startup_complete ();
        g_object_unref (app);
        exit (EXIT_FAILURE);
    }

    if (medit_opts.project_mode)
#ifdef MOO_ENABLE_PROJECT
        project_mode (medit_opts.project);
#else
    {
        fputs ("medit was built without project support\n", stderr);
	exit (EXIT_FAILURE);
    }
#endif
    else
        moo_app_load_session (app);

    editor = moo_app_get_editor (app);
    if (!moo_editor_get_active_window (editor))
        moo_editor_new_window (editor);

    if (files)
        moo_app_open_files (app, files, n_files, stamp);

    free_files (files, n_files);
    g_option_context_free (ctx);

    retval = moo_app_run (app);
    gdk_threads_leave ();

    g_object_unref (app);

    return retval;
}

int
main (int argc, char *argv[])
{
#ifdef MOO_ENABLE_UNIT_TESTS
    if (argc > 1 && strcmp (argv[1], "--ut") == 0)
    {
        memmove (argv + 1, argv + 2, (argc - 1) * sizeof (char*));
        argc -= 1;
        return unit_tests_main (argc, argv);
    }
    else
#endif
    return medit_main (argc, argv);
}

#if defined(__WIN32__) && !defined(__GNUC__)

#include <windows.h>

int WINAPI
WinMain (HINSTANCE hInstance,
         HINSTANCE hPrevInstance,
         char     *lpszCmdLine,
         int       nCmdShow)
{
	return main (__argc, __argv);
}

#endif
