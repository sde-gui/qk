/*
 *   main.cpp
 *
 *   Copyright (C) 2004-2016 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
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
#include "mooutils/mooi18n.h"
#include "mooutils/mooutils-fs.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mootype-macros.h"
#include "plugins/mooplugin-builtin.h"
#include "moocpp/moocpp.h"
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "mem-debug.h"
#include "run-tests.h"
#ifdef GDK_WINDOWING_WIN32
#include <gdk/gdkwin32.h>
#include <windows.h>
#include <windowsx.h>
#endif

using namespace moo;

static struct MeditOpts {
    int use_session;
    int pid;
    gboolean new_app;
    const char *instance_name;
    gboolean new_window;
    gboolean new_tab;
    gboolean reload;
    int line;
    const char *encoding;
    const char *log_file;
    gboolean log_window;
    const char *exec_string;
    const char *exec_file;
    char **files;
    const char *geometry;
    gboolean show_version;
    const char *debug;
    gboolean ut;
    gboolean ut_uninstalled;
    gboolean ut_list;
    char *ut_dir;
    char *ut_coverage_file;
    char **ut_tests;
    char **run_script;
    char **send_script;
    gboolean portable;
} medit_opts = { -1, -1 };

#include "parse.h"

class MeditApp : public App
{
public:
    MeditApp(gobj_wrapper_data& d, const StartupOptions& opts)
        : App(d, opts)
    {
    }

protected:
    void init_plugins() override
    {
        moo_plugin_init ();
    }
};

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
    { "use-session", 's', G_OPTION_FLAG_OPTIONAL_ARG, G_OPTION_ARG_CALLBACK, (void*) parse_use_session,
            /* help message for command line option --use-session */ N_("Load and save session"), "yes|no" },
    { "pid", 0, 0, G_OPTION_ARG_INT, &medit_opts.pid,
            /* help message for command line option --pid=PID */ N_("Use existing instance with process id PID"),
            /* "PID" part in "--pid=PID" */ N_("PID") },
    { "app-name", 0, 0, G_OPTION_ARG_STRING, (gpointer) &medit_opts.instance_name,
            /* help message for command line option --app-name=NAME */ N_("Set instance name to NAME if it's not already running"),
            /* "NAME" part in "--app-name=NAME" */ N_("NAME") },
    { "new-window", 'w', 0, G_OPTION_ARG_NONE, &medit_opts.new_window,
            /* help message for command line option --new-window */ N_("Open file(s) in a new window"), NULL },
    { "new-tab", 't', 0, G_OPTION_ARG_NONE, &medit_opts.new_tab,
            /* help message for command line option --new-tab */ N_("Open file(s) in a new tab"), NULL },
    { "line", 'l', 0, G_OPTION_ARG_INT, &medit_opts.line,
            /* help message for command line option --line=LINE */ N_("Open file and position cursor on line LINE"),
            /* "LINE" part in --line=LINE */ N_("LINE") },
    { "encoding", 'e', 0, G_OPTION_ARG_STRING, (gpointer) &medit_opts.encoding,
            /* help message for command line option --encoding=ENCODING */ N_("Use character encoding ENCODING"),
            /* "ENCODING" part in --encoding=ENCODING */ N_("ENCODING") },
    { "reload", 'r', 0, G_OPTION_ARG_NONE, &medit_opts.reload,
            /* help message for command line option --reload */ N_("Automatically reload file if it was modified on disk"), NULL },
    { "run-script", 0, G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_STRING_ARRAY, (gpointer) &medit_opts.run_script,
            "Run SCRIPT", "SCRIPT" },
    { "send-script", 0, G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_STRING_ARRAY, (gpointer) &medit_opts.send_script,
            "Send SCRIPT to existing instance", "SCRIPT" },
    { "log-window", 0, G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_NONE, &medit_opts.log_window,
            "Show debug output", NULL },
    { "log-file", 0, G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_FILENAME, (gpointer) &medit_opts.log_file,
            "Write debug output to FILE", "FILE" },
    { "debug", 0, G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_STRING, (gpointer) &medit_opts.debug,
            "Run in debug mode", NULL },
    { "geometry", 0, 0, G_OPTION_ARG_STRING, (gpointer) &medit_opts.geometry,
            /* help message for command line option --geometry=WIDTHxHEIGHT[+X+Y] */ N_("Default window size and position"),
            /* "WIDTHxHEIGHT[+X+Y]" part in --geometry=WIDTHxHEIGHT[+X+Y] */ N_("WIDTHxHEIGHT[+X+Y]") },
    { "version", 0, 0, G_OPTION_ARG_NONE, &medit_opts.show_version,
            /* help message for command line option --version */ N_("Show version information and exit"), NULL },
    { "ut", 0, G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_NONE, &medit_opts.ut,
            "Run unit tests", NULL },
    { "ut-uninstalled", 0, G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_NONE, &medit_opts.ut_uninstalled,
            "Run unit tests in uninstalled medit", NULL },
    { "ut-dir", 0, G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_STRING, &medit_opts.ut_dir,
            "Data dir for unit tests", NULL },
    { "ut-coverage", 0, G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_FILENAME, &medit_opts.ut_coverage_file,
            "File to write coverage data to", NULL },
    { "ut-list", 0, G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_NONE, &medit_opts.ut_list,
            "List unit tests", NULL },
#ifdef __WIN32__
    { "portable", 0, G_OPTION_ARG_NONE, G_OPTION_ARG_NONE, &medit_opts.portable,
            "Run medit in portable mode", NULL },
#endif
    { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &medit_opts.files,
            NULL, /* "FILES" part in "medit [OPTION...] [FILES]" */ N_("FILES") },
    { NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL }
};

/* Check if there is an argument of the form +<number>, and treat it as --line <number> */
static void
check_plus_line_arg (void)
{
    gboolean done = FALSE;
    char **p;
    GRegex *re = NULL;

    re = g_regex_new ("^\\+(?P<line>\\d+)", G_REGEX_OPTIMIZE | G_REGEX_DUPNAMES, GRegexMatchFlags(0), NULL);
    g_return_if_fail (re != NULL);

    for (p = medit_opts.files; !done && p && *p && **p; ++p)
    {
        GMatchInfo *match_info = NULL;

        if (g_regex_match (re, *p, GRegexMatchFlags(0), &match_info))
        {
            int line = 0;
            char *line_string = g_match_info_fetch_named (match_info, "line");

            errno = 0;
            line = strtol (line_string, NULL, 10);
            if (errno != 0)
                line = 0;

            // if a file "+10" exists, open it
            if (line > 0 && g_file_test (*p, G_FILE_TEST_EXISTS))
                line = 0;

            if (line > 0)
            {
                medit_opts.line = line;

                g_free (*p);
                *p = NULL;
                if (*(p + 1) != NULL)
                {
                    int n = g_strv_length (p + 1);
                    memcpy (p, p + 1, n * sizeof(*p));
                    *(p + n) = NULL;
                }

                done = TRUE;
            }

            g_free (line_string);
        }

        g_match_info_free (match_info);
    }

    g_regex_unref (re);
}

static gboolean
post_parse_func (void)
{
    if (medit_opts.show_version)
    {
        g_print ("medit " MOO_DISPLAY_VERSION "\n");
        exit (0);
    }

    if (medit_opts.ut_list)
    {
        list_unit_tests (medit_opts.ut_dir);
        exit (0);
    }

    if (medit_opts.ut)
    {
        medit_opts.ut_tests = medit_opts.files;
        medit_opts.files = NULL;
    }

    if (medit_opts.pid > 0 && medit_opts.instance_name)
    {
        /* error message for wrong commmand line */
        g_printerr (_("%s and %s options may not be used simultaneously\n"),
                    "--app-name", "--pid");
        exit (EXIT_FAILURE);
    }

    if (medit_opts.debug)
        g_setenv ("MOO_DEBUG", medit_opts.debug, FALSE);

    check_plus_line_arg ();

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

#ifdef __WIN32__
static void
push_appdir_to_path (void)
{
    gstr appdir = moo_win32_get_app_dir ();
    g_return_if_fail (!appdir.empty());

    const char *path = g_getenv ("Path");
    gstr new_path;

    if (path)
        new_path.set_printf ("%s;%s", appdir, path);
    else
        new_path = std::move (appdir);

    g_setenv ("Path", new_path, TRUE);
}
#endif

#undef WANT_SYNAPTICS_FIX
#if defined(GDK_WINDOWING_WIN32) && !GTK_CHECK_VERSION(2,24,8)
#define WANT_SYNAPTICS_FIX 1
#endif

#ifdef WANT_SYNAPTICS_FIX

static GdkWindow *
_moo_get_toplevel_window_at_pointer (void)
{
    GList *list, *l;
    GSList *windows = NULL;
    GtkWidget *top;
    POINT point;

    GetCursorPos (&point);

    list = gtk_window_list_toplevels ();

    for (l = list; l != NULL; l = l->next)
    {
        HWND hwnd;
        RECT rect = { 0 };
        GdkWindow *window = GTK_WIDGET (l->data)->window;

        if (!window)
            continue;

        hwnd = (HWND) GDK_WINDOW_HWND (window);
        GetWindowRect(hwnd, &rect);
        if (IsWindowVisible(hwnd) && PtInRect(&rect, point))
            windows = g_slist_prepend (windows, l->data);
    }

    top = windows ? GTK_WIDGET (_moo_get_top_window (windows)) : NULL;

    g_list_free (list);
    g_slist_free (windows);
    return top ? top->window : NULL;
}

// stolen from gdkwindow-win32.c
static GdkModifierType
get_current_mask (void)
{
    GdkModifierType mask = GdkModifierType(0);
    BYTE kbd[256];

    GetKeyboardState (kbd);

    if (kbd[VK_SHIFT] & 0x80)
        mask |= GDK_SHIFT_MASK;
    if (kbd[VK_CAPITAL] & 0x80)
        mask |= GDK_LOCK_MASK;
    if (kbd[VK_CONTROL] & 0x80)
        mask |= GDK_CONTROL_MASK;
    if (kbd[VK_MENU] & 0x80)
        mask |= GDK_MOD1_MASK;
    if (kbd[VK_LBUTTON] & 0x80)
        mask |= GDK_BUTTON1_MASK;
    if (kbd[VK_MBUTTON] & 0x80)
        mask |= GDK_BUTTON2_MASK;
    if (kbd[VK_RBUTTON] & 0x80)
        mask |= GDK_BUTTON3_MASK;

    return mask;
}

// stolen from gdkwindow-win32.c
static GdkWindow *
get_pointer (GdkWindow *window,
             gint      *x,
             gint      *y)
{
    POINT point;
    HWND hwnd, hwndc;
    GdkWindow *retval = window;
    GdkWindow *child = NULL;

    hwnd = (HWND) GDK_WINDOW_HWND (window);
    GetCursorPos (&point);
    ScreenToClient (hwnd, &point);

    *x = point.x;
    *y = point.y;

    hwndc = ChildWindowFromPointEx (hwnd, point, CWP_SKIPINVISIBLE);
    if (hwndc != NULL && hwndc != hwnd)
        child = reinterpret_cast<GdkWindow*> (gdk_win32_handle_table_lookup ((GdkNativeWindow) hwndc));
    if (child != NULL)
        retval = child;

    return retval;
}

static GdkWindow *
get_youngest_child_at_pointer (GdkWindow *window,
                               int       *x,
                               int       *y)
{
    gboolean first_time = TRUE;

    while (TRUE)
    {
        int tmp_x, tmp_y;
        GdkWindow *child_window = get_pointer (window, &tmp_x, &tmp_y);

        if (first_time)
        {
            *x = tmp_x;
            *y = tmp_y;
        }

        if (!child_window || child_window == window)
            return window;
        else
            window = child_window;
    }
}

static gboolean
fill_scroll_event (GdkEvent  *event,
                   const MSG *msg)
{
    GdkWindow *window, *child_window;
    int x, y;

    if (!(window = _moo_get_toplevel_window_at_pointer ()))
        return FALSE;

    child_window = get_youngest_child_at_pointer (window, &x, &y);

    if (child_window && child_window != window)
    {
        int origin_x, origin_y;
        int origin_child_x, origin_child_y;
        gdk_window_get_origin (window, &origin_x, &origin_y);
        gdk_window_get_origin (child_window, &origin_child_x, &origin_child_y);
        x -= (origin_child_x - origin_x);
        y -= (origin_child_y - origin_y);
        window = child_window;
    }

    event->type = GDK_SCROLL;
    event->scroll.window = window;
    event->scroll.direction = (((short) HIWORD (msg->wParam)) > 0) ? GDK_SCROLL_UP : GDK_SCROLL_DOWN;
    event->scroll.time = 0;
    event->scroll.x = x;
    event->scroll.y = y;
    event->scroll.x_root = GET_X_LPARAM (msg->lParam);
    event->scroll.y_root = GET_Y_LPARAM (msg->lParam);
    event->scroll.state = get_current_mask ();
    event->scroll.device = gdk_device_get_core_pointer ();

    return TRUE;
}

/* Synaptics touchpad is smart: it doesn't generate normal scroll events
 * for some miraculous reason if the window doesn't have fancy scrolling
 * window styles; instead it creates its own window to draw neat image
 * of a scrollbar under the mouse; and sends a scroll event to the gdk
 * window underneath. Now, gdk is smart too, it checks what window is
 * under the mouse and does nothing if that window isn't gdk's. That window
 * isn't gdk's because it's that neat image of a scrollbar. Thank you very
 * much Synaptics. */
static GdkFilterReturn
touchpad_filter_func (GdkXEvent *xevent,
                      GdkEvent  *event,
                      G_GNUC_UNUSED gpointer data)
{
    const MSG *msg = (MSG*) xevent;
    wchar_t class_name[256];
    HWND mouse_hwnd;
    POINT point;

    if (msg->message != WM_MOUSEWHEEL)
        return GDK_FILTER_CONTINUE;

    point.x = GET_X_LPARAM (msg->lParam);
    point.y = GET_Y_LPARAM (msg->lParam);

    if ((mouse_hwnd = WindowFromPoint (point)) == NULL)
        return GDK_FILTER_CONTINUE;

    if (GetClassName (mouse_hwnd, class_name, G_N_ELEMENTS (class_name)) == 0 ||
        wcscmp (class_name, L"SynTrackCursorWindowClass") != 0)
            return GDK_FILTER_CONTINUE;

    return fill_scroll_event (event, msg) ? GDK_FILTER_TRANSLATE : GDK_FILTER_CONTINUE;
}

static void
hookup_synaptics_touchpad (void)
{
    gdk_window_add_filter (NULL, touchpad_filter_func, NULL);
}

#endif // WANT_SYNAPTICS_FIX

static void
unit_test_func (void)
{
    MooTestOptions opts = MOO_TEST_OPTIONS_NONE;
    int status;

    if (!medit_opts.ut_uninstalled)
        opts |= MOO_TEST_INSTALLED;

    status = unit_tests_main (opts, medit_opts.ut_tests, medit_opts.ut_dir, medit_opts.ut_coverage_file);
    App::instance().set_exit_status (status);
    App::instance().quit ();
}

static void
run_script_func (void)
{
    char **p;
    for (p = medit_opts.run_script; p && *p; ++p)
        App::instance().run_script (*p);
}

static void
install_log_handlers (void)
{
    if (medit_opts.log_file)
        moo_set_log_func_file (medit_opts.log_file);
    else if (medit_opts.log_window)
        moo_set_log_func_window (TRUE);
#ifdef __WIN32__
    // this will install do-nothing log and print handlers plus
    // a fatal error win32 message handler (it will also turn off
    // console output, but that is not visible anyway)
    else
        moo_set_log_func_silent ();
#endif
}

#ifdef __WIN32__
static void
setup_portable_mode (void)
{
    gstr appdir = moo_win32_get_app_dir ();
    g_return_if_fail (!appdir.empty());

    gstr share = g::build_filename (appdir, "..", "share");
    g_return_if_fail (!share.empty());

    gstr datadir;
    gstr cachedir;

    if (g_file_test (share, G_FILE_TEST_IS_DIR))
    {
        datadir = g::build_filename (share, MEDIT_PORTABLE_DATA_DIR);
        cachedir = g::build_filename (share, MEDIT_PORTABLE_CACHE_DIR);
    }
    else
    {
        datadir = g::build_filename (appdir, MEDIT_PORTABLE_DATA_DIR);
        cachedir = g::build_filename (appdir, MEDIT_PORTABLE_CACHE_DIR);
    }

    g_return_if_fail (!datadir.empty() && !cachedir.empty());

    gstr tmp = _moo_normalize_file_path (datadir);
    moo_set_user_data_dir (tmp);

    tmp = _moo_normalize_file_path (cachedir);
    moo_set_user_cache_dir (tmp);
}

static void
check_portable_mode (void)
{
    bool portable = medit_opts.portable;

    if (!portable)
    {
        gstr appdir = moo_win32_get_app_dir ();
        g_return_if_fail (!appdir.empty());
        gstr magic_file = g::build_filename (appdir, MEDIT_PORTABLE_MAGIC_FILE_NAME);
        g_return_if_fail (!magic_file.empty());
        if (g_file_test (magic_file, G_FILE_TEST_EXISTS))
            portable = TRUE;
    }

    if (portable)
        setup_portable_mode ();
}
#endif // __WIN32__

namespace moo
{
namespace _test
{
void test();
}
}

static int
medit_main (int argc, char *argv[])
{
    MooEditor *editor;
    int retval;
    gboolean new_instance = FALSE;
    gboolean run_input = TRUE;
    guint32 stamp;
    const char *name = NULL;
    char pid_buf[32];
    GOptionContext *ctx;
    MooOpenInfoArray *files;

#ifdef __WIN32__
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
#endif // __WIN32__

    init_mem_stuff ();
    moo_thread_init ();
    g_set_prgname ("medit");

    ctx = parse_args (argc, argv);

    stamp = get_time_stamp ();

#ifdef WANT_SYNAPTICS_FIX
    hookup_synaptics_touchpad ();
#endif

#if 0
    gdk_window_set_debug_updates (TRUE);
#endif

#if 0
    g_idle_add_full (G_PRIORITY_LOW, (GSourceFunc) exit, NULL, NULL);
#endif

#ifdef __WIN32__
    check_portable_mode ();
#endif

    if (medit_opts.new_app)
        new_instance = TRUE;

    run_input = !medit_opts.new_app || medit_opts.instance_name ||
                 medit_opts.use_session == 1;

    if (medit_opts.ut)
    {
        new_instance = TRUE;
        run_input = FALSE;
        medit_opts.use_session = FALSE;
    }

    if (medit_opts.pid > 0)
    {
        sprintf (pid_buf, "%d", medit_opts.pid);
        name = pid_buf;
    }
    else if (medit_opts.instance_name)
        name = medit_opts.instance_name;
    else if (!medit_opts.new_app)
        name = g_getenv ("MEDIT_PID");

    if (name && !name[0])
        name = NULL;

    if (medit_opts.send_script)
    {
        char **p;
        for (p = medit_opts.send_script; *p; ++p)
        {
            GString *msg = g_string_new ("e");
            g_string_append (msg, *p);
            App::send_msg (name, msg->str, msg->len + 1);
        }
        notify_startup_complete ();
        exit (0);
    }

    files = parse_files ();

    if (name)
    {
        if (App::send_files (files, stamp, name))
            exit (0);

        if (!medit_opts.instance_name)
        {
            g_printerr ("Could not send files to instance '%s'\n", name);
            exit (EXIT_FAILURE);
        }
    }

    if (!new_instance && !medit_opts.instance_name &&
         App::send_files (files, stamp, NULL))
    {
        notify_startup_complete ();
        exit (0);
    }

#ifdef __WIN32__
    push_appdir_to_path ();
#endif

    gtk_init (NULL, NULL);

    install_log_handlers ();

    MeditApp::StartupOptions sopts;
    sopts.run_input = run_input;
    sopts.use_session = medit_opts.use_session;
    sopts.instance_name.set(medit_opts.instance_name);
    gref_ptr<MeditApp> app = MeditApp::create<MeditApp>(sopts);

    if (!app->init())
    {
        gdk_notify_startup_complete ();
        return EXIT_FAILURE;
    }

    //moo::_test::test ();

    if (medit_opts.geometry && *medit_opts.geometry)
        moo_window_set_default_geometry (medit_opts.geometry);

    app->load_session ();

    editor = app->get_editor ();
    if (!moo_editor_get_active_window (editor))
        moo_editor_new_window (editor);

    if (files)
        app->open_files (files, stamp);

    moo_open_info_array_free (files);
    g_option_context_free (ctx);

    if (medit_opts.ut)
        app->connect ("started", G_CALLBACK (unit_test_func), NULL);
    if (medit_opts.run_script)
        app->connect ("started", G_CALLBACK (run_script_func), NULL);

    retval = app->run ();

#ifdef __WIN32__
    CoUninitialize();
#endif // __WIN32__

    return retval;
}

int
main (int argc, char *argv[])
{
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
