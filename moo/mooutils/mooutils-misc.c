/*
 *   mooutils-misc.c
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

#include "config.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooutils-fs.h"
#include "mooutils/mooutils-file.h"
#include "mooutils/mooutils-debug.h"
#include "mooutils/mooi18n.h"
#include "mooutils/mooatom.h"
#include "mooutils/mooonce.h"
#include "mooutils/mooutils-misc.h"
#include <mooutils/mooutils-tests.h>
#include "moologwindow-gxml.h"
#include <gtk/gtk.h>
#include <glib/gmappedfile.h>
#include <glib/gstdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>

#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif

#ifdef MOO_USE_QUARTZ
#include <gdk/gdkquartz.h>
#endif

#ifdef __WIN32__
#include <windows.h>
#include <shellapi.h>
#endif


G_LOCK_DEFINE_STATIC (moo_user_data_dir);
static char *moo_app_instance_name;
static char *moo_display_app_name;
static char *moo_user_data_dir;
static char *moo_temp_dir;

static void         moo_install_atexit      (void);
static void         moo_remove_tempdir      (void);


#ifdef __WIN32__

static gboolean
open_uri (const char *uri,
          G_GNUC_UNUSED gboolean email)
{
    return _moo_win32_open_uri (uri);
}

#else /* ! __WIN32__ */

char *
_moo_find_script (const char *name,
                  gboolean    search_path)
{
    char *path = NULL;

    g_return_val_if_fail (name != NULL, NULL);

    if (search_path)
        path = g_find_program_in_path (name);

    if (!path)
    {
        char **dirs, **p;

        dirs = moo_get_data_subdirs ("scripts");

        for (p = dirs; p && *p; ++p)
        {
            path = g_build_filename (*p, name, NULL);

            if (g_file_test (path, G_FILE_TEST_IS_EXECUTABLE))
                break;

            g_free (path);
            path = NULL;
        }

        g_strfreev (dirs);
    }

    if (!path)
    {
        g_warning ("%s: could not find %s script", G_STRLOC, name);
        path = g_strdup (name);
    }

    _moo_message ("%s: %s", name, path);
    return path;
}

enum {
    SCRIPT_XDG_OPEN,
    SCRIPT_XDG_EMAIL
};

static const char *
get_xdg_script (int type)
{
    static char *path[2];

    g_return_val_if_fail (type < 2, NULL);

    if (!path[0])
    {
        path[SCRIPT_XDG_OPEN] = _moo_find_script ("xdg-open", TRUE);
        path[SCRIPT_XDG_EMAIL] = _moo_find_script ("xdg-email", TRUE);
    }

    return path[type];
}

static gboolean
open_uri (const char *uri,
          gboolean    email)
{
    gboolean result = FALSE;
    const char *argv[3];
    GError *err = NULL;

    argv[0] = get_xdg_script (email ? SCRIPT_XDG_EMAIL : SCRIPT_XDG_OPEN);
    argv[1] = uri;
    argv[2] = NULL;

    result = g_spawn_async (NULL, (char**) argv, NULL, (GSpawnFlags) 0, NULL, NULL,
                            NULL, &err);

    if (err)
    {
        g_warning ("%s: error in g_spawn_async", G_STRLOC);
        g_warning ("%s: %s", G_STRLOC, err->message);
        g_error_free (err);
    }

    return result;
}

#endif /* ! __WIN32__ */


gboolean
moo_open_email (const char *address,
                const char *subject,
                const char *body)
{
    GString *uri;
    gboolean res;

    g_return_val_if_fail (address != NULL, FALSE);
    uri = g_string_new ("mailto:");
    g_string_append_printf (uri, "%s%s", address,
                            subject || body ? "?" : "");
    if (subject)
        g_string_append_printf (uri, "subject=%s%s", subject,
                                body ? "&" : "");
    if (body)
        g_string_append_printf (uri, "body=%s", body);

    res = open_uri (uri->str, TRUE);
    g_string_free (uri, TRUE);
    return res;
}


gboolean
moo_open_url (const char *url)
{
    g_return_val_if_fail (url != NULL, FALSE);
    return open_uri (url, FALSE);
}


gboolean
moo_open_file (const char *path)
{
    char *uri;
    gboolean ret;

    g_return_val_if_fail (path != NULL, FALSE);

    uri = _moo_filename_to_uri (path, NULL);
    g_return_val_if_fail (uri != NULL, FALSE);

    ret = open_uri (uri, FALSE);

    g_free (uri);
    return ret;
}


/********************************************************************/
/* Windowing stuff
 */

#ifdef GDK_WINDOWING_X11

#include <X11/Xatom.h>
#include <gdk/gdkx.h>


/* TODO TODO is it 64-bits safe? */
/* TODO TODO rewrite all of this */
/* TODO TODO is X needed here at all? */

static void
add_xid (GtkWindow  *window,
         GArray     *array)
{
    XID xid;

    if (GTK_IS_WINDOW(window) && GTK_WIDGET(window)->window)
    {
        xid = GDK_WINDOW_XID (GTK_WIDGET(window)->window);
        g_array_append_val (array, xid);
    }
}


static gboolean
contains (GArray *xids, XID w)
{
    guint i;
    XID *wins = (XID*) xids->data;

    for (i = 0; i < xids->len; ++i)
        if (wins[i] == w)
            return TRUE;

    return FALSE;
}


static gboolean
is_minimized (Display *display, XID w)
{
    Atom actual_type_return;
    int actual_format_return;
    gulong nitems_return;
    gulong bytes_after_return;
    Atom *data;
    guchar *cdata;
    int ret;
    gulong i;

    static Atom wm_state = None;
    static Atom wm_state_hidden = None;

    if (wm_state == None)
    {
        wm_state = XInternAtom (display, "_NET_WM_STATE", FALSE);
        wm_state_hidden = XInternAtom (display, "_NET_WM_STATE_HIDDEN", FALSE);
    }

    gdk_error_trap_push ();

    ret = XGetWindowProperty (display, w,
                              wm_state, 0,
                              G_MAXLONG, FALSE, XA_ATOM,
                              &actual_type_return,
                              &actual_format_return,
                              &nitems_return,
                              &bytes_after_return,
                              &cdata);
    data = (Atom*) cdata;

    if (gdk_error_trap_pop () || ret != Success)
    {
        g_warning ("%s: oops", G_STRLOC);
        return FALSE;
    }

    if (!nitems_return)
    {
        XFree (data);
        return FALSE;
    }

    if (actual_type_return != XA_ATOM)
    {
        g_critical ("%s: actual_type_return != XA_WINDOW", G_STRLOC);
        XFree (data);
        return FALSE;
    }

    for (i = 0; i < nitems_return; ++i)
    {
        if (data[i] == wm_state_hidden)
        {
            XFree (data);
            return TRUE;
        }
    }

    XFree (data);
    return FALSE;
}


static GtkWindow*
find_by_xid (GSList *windows, XID w)
{
    GSList *l;

    for (l = windows; l != NULL; l = l->next)
        if (GDK_WINDOW_XID (GTK_WIDGET(l->data)->window) == w)
            return l->data;

    return NULL;
}


GtkWindow *
_moo_get_top_window (GSList *windows)
{
    GArray *xids;
    Display *display;
    Atom actual_type_return;
    int actual_format_return;
    gulong nitems_return;
    gulong bytes_after_return;
    XID *data;
    guchar *cdata;
    int ret;
    long i;
    GSList *l;

    static Atom list_stacking_atom = None;

    g_return_val_if_fail (windows != NULL, NULL);

    if (!windows->next)
        return windows->data;

    for (l = windows; l != NULL; l = l->next)
    {
        if (!GTK_IS_WINDOW (l->data))
        {
            g_critical ("%s: invalid parameter passed", G_STRLOC);
            return NULL;
        }
    }

    xids = g_array_new (FALSE, FALSE, sizeof (XID));
    g_slist_foreach (windows, (GFunc) add_xid, xids);

    if (!xids->len)
    {
        g_critical ("%s: zero length array of x ids", G_STRLOC);
        g_array_free (xids, TRUE);
        return NULL;
    }

    display = GDK_WINDOW_XDISPLAY (GTK_WIDGET(windows->data)->window);

    if (!display)
    {
        g_critical ("%s: !display", G_STRLOC);
        g_array_free (xids, TRUE);
        return NULL;
    }

    if (list_stacking_atom == None)
        list_stacking_atom = XInternAtom (display, "_NET_CLIENT_LIST_STACKING", FALSE);

    gdk_error_trap_push ();

    ret = XGetWindowProperty (display, GDK_ROOT_WINDOW(),
                              list_stacking_atom, 0,
                              G_MAXLONG, FALSE, XA_WINDOW,
                              &actual_type_return,
                              &actual_format_return,
                              &nitems_return,
                              &bytes_after_return,
                              &cdata);
    data = (XID*) cdata;

    if (gdk_error_trap_pop () || ret != Success)
    {
        g_critical ("%s: error in XGetWindowProperty", G_STRLOC);
        g_array_free (xids, TRUE);
        return NULL;
    }

    if (!nitems_return)
    {
        g_critical ("%s: !nitems_return", G_STRLOC);
        XFree (data);
        g_array_free (xids, TRUE);
        return NULL;
    }

    if (actual_type_return != XA_WINDOW)
    {
        g_critical ("%s: actual_type_return != XA_WINDOW", G_STRLOC);
        XFree (data);
        g_array_free (xids, TRUE);
        return NULL;
    }

    for (i = nitems_return - 1; i >= 0; --i)
    {
        if (contains (xids, data[i]) && !is_minimized (display, data[i]))
        {
            XID id = data[i];
            XFree (data);
            g_array_free (xids, TRUE);
            return find_by_xid (windows, id);
        }
    }

    XFree (data);
    g_array_free (xids, TRUE);
    g_warning ("%s: all minimized?", G_STRLOC);
    return GTK_WINDOW (windows->data);
}


#if 0
static gboolean
_moo_window_is_hidden (GtkWindow  *window)
{
    g_return_val_if_fail (GTK_IS_WINDOW (window), FALSE);
    return is_minimized (GDK_WINDOW_XDISPLAY (GTK_WIDGET(window)->window),
                         GDK_WINDOW_XID (GTK_WIDGET(window)->window));
}
#endif


#elif defined(__WIN32__)

#include <windows.h>
#include <gdk/gdkwin32.h>


#define get_handle(w) \
    gdk_win32_drawable_get_handle (GTK_WIDGET(w)->window)

static gboolean
_moo_window_is_hidden (GtkWindow  *window)
{
    HANDLE h;
    WINDOWPLACEMENT info;

    g_return_val_if_fail (GTK_IS_WINDOW (window), FALSE);

    h = get_handle (window);
    g_return_val_if_fail (h != NULL, FALSE);

    memset (&info, 0, sizeof (info));
    info.length = sizeof (WINDOWPLACEMENT);

    if (!GetWindowPlacement (h, &info))
    {
        DWORD err = GetLastError ();
        char *msg = g_win32_error_message (err);
        g_return_val_if_fail (msg != NULL, FALSE);
        g_warning ("%s: %s", G_STRLOC, msg);
        g_free (msg);
        return FALSE;
    }

    return info.showCmd == SW_MINIMIZE ||
            info.showCmd == SW_HIDE ||
            info.showCmd == SW_SHOWMINIMIZED;
}


GtkWindow *
_moo_get_top_window (GSList *windows)
{
    GSList *l;
    HWND top = NULL;
    HWND current = NULL;

    g_return_val_if_fail (windows != NULL, NULL);

    if (!windows->next)
        return windows->data;

    for (l = windows; l != NULL; l = l->next)
    {
        g_return_val_if_fail (GTK_IS_WINDOW (l->data), NULL);

        if (!_moo_window_is_hidden (GTK_WINDOW (l->data)))
            break;
    }

    if (!l)
        return GTK_WINDOW (windows->data);

    top = get_handle (windows->data);
    current = top;

    while (TRUE)
    {
        current = GetNextWindow (current, GW_HWNDPREV);
        if (!current)
            break;

        for (l = windows; l != NULL; l = l->next)
            if (current == get_handle (l->data))
                break;

        if (l != NULL)
            top = get_handle (l->data);
    }

    for (l = windows; l != NULL; l = l->next)
        if (top == get_handle (l->data))
            break;

    g_return_val_if_fail (l != NULL, GTK_WINDOW (windows->data));
    return GTK_WINDOW (l->data);
}

#else /* neither X nor WIN32 */

/* XXX implement me */

GtkWindow *
_moo_get_top_window (GSList *windows)
{
    g_return_val_if_fail (windows != NULL, NULL);

    if (!windows->next)
        return GTK_WINDOW (windows->data);

    g_message ("%s: don't know how to do it", G_STRFUNC);
    return GTK_WINDOW (windows->data);
}

#endif


#ifdef GDK_WINDOWING_X11
static void
present_window_x11 (GtkWindow *window,
                     guint32   stamp)
{
    if (!GTK_WIDGET_REALIZED (window))
        gtk_widget_realize (GTK_WIDGET (window));

#if GTK_CHECK_VERSION(2,8,0)
    gdk_x11_window_move_to_current_desktop (GTK_WIDGET(window)->window);
#endif

    gtk_widget_show (GTK_WIDGET (window));

    if (!stamp)
        stamp = gdk_x11_get_server_time (GTK_WIDGET (window)->window);

    gdk_window_focus (GTK_WIDGET (window)->window, stamp);
}
#endif

void
moo_window_present (GtkWindow *window,
                    G_GNUC_UNUSED guint32 stamp)
{
    g_return_if_fail (GTK_IS_WINDOW (window));

#if defined(GDK_WINDOWING_X11)
    present_window_x11 (window, stamp);
#elif defined(MOO_USE_QUARTZ)
    gtk_window_present (window);
    [[NSApplication sharedApplication] activateIgnoringOtherApps:YES];
#else
    gtk_window_present (window);
#endif
}


void
_moo_window_set_icon_from_stock (GtkWindow  *window,
                                 const char *name)
{
    GdkPixbuf *icon;

    g_return_if_fail (GTK_IS_WINDOW (window));
    g_return_if_fail (name != NULL);

    icon = gtk_widget_render_icon (GTK_WIDGET (window), name,
                                   GTK_ICON_SIZE_BUTTON, 0);

    if (icon)
    {
        gtk_window_set_icon (GTK_WINDOW (window), icon);
        gdk_pixbuf_unref (icon);
    }
}


#if 0
GtkWindow *
_moo_get_toplevel_window (void)
{
    GList *list, *l;
    GSList *windows = NULL;
    GtkWindow *top;

    list = gtk_window_list_toplevels ();

    for (l = list; l != NULL; l = l->next)
        if (GTK_IS_WINDOW (l->data) && GTK_WIDGET(l->data)->window)
            windows = g_slist_prepend (windows, l->data);

    top = _moo_get_top_window (windows);

    g_list_free (list);
    g_slist_free (windows);
    return top;
}
#endif


/***************************************************************************/
/* Log window
 */

typedef struct {
    GtkWidget *window;
    GtkTextTag *message_tag;
    GtkTextTag *warning_tag;
    GtkTextTag *critical_tag;
    GtkTextBuffer *buf;
    GtkTextView *textview;
    GtkTextMark *insert;
} MooLogWindow;


static MooLogWindow *moo_log_window             (void);
static GtkWidget    *moo_log_window_get_widget  (void);


static MooLogWindow*
moo_log_window_new (void)
{
    MooLogWindow *log;
    PangoFontDescription *font;
    LogWindowXml *xml;

    xml = log_window_xml_new ();
    log = g_new (MooLogWindow, 1);

    log->window = GTK_WIDGET (xml->LogWindow);
    log->textview = xml->textview;

    g_signal_connect (log->window, "delete-event",
                      G_CALLBACK (gtk_widget_hide_on_delete), NULL);

    log->buf = gtk_text_view_get_buffer (log->textview);
    log->insert = gtk_text_buffer_get_insert (log->buf);

    log->message_tag =
            gtk_text_buffer_create_tag (log->buf, "message", "foreground", "blue", NULL);
    log->warning_tag =
            gtk_text_buffer_create_tag (log->buf, "warning", "foreground", "red", NULL);
    log->critical_tag =
            gtk_text_buffer_create_tag (log->buf, "critical", "foreground", "red",
                                        "weight", PANGO_WEIGHT_BOLD, NULL);

    font = pango_font_description_from_string ("Monospace");

    if (font)
    {
        gtk_widget_modify_font (GTK_WIDGET (log->textview), font);
        pango_font_description_free (font);
    }

    return log;
}


static MooLogWindow*
moo_log_window (void)
{
    static gpointer log = NULL;

    if (!log)
    {
        log = moo_log_window_new ();
        g_object_add_weak_pointer (G_OBJECT (((MooLogWindow*)log)->window), &log);
    }

    return log;
}


static GtkWidget*
moo_log_window_get_widget (void)
{
    MooLogWindow *log = moo_log_window ();
    return log->window;
}


void
moo_log_window_show (void)
{
    gtk_window_present (GTK_WINDOW (moo_log_window_get_widget ()));
}

void
moo_log_window_hide (void)
{
    gtk_widget_hide (moo_log_window_get_widget ());
}


static void
moo_log_window_insert (MooLogWindow *log,
                       const char   *text,
                       GtkTextTag   *tag)
{
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter (log->buf, &iter);
    gtk_text_buffer_insert_with_tags (log->buf, &iter, text, -1, tag, NULL);
    gtk_text_view_scroll_mark_onscreen (log->textview, log->insert);
}


/******************************************************************************/
/* Custom g_log and g_print handlers
 */

static GLogFunc moo_log_func;
static GPrintFunc moo_print_func;
static GPrintFunc moo_printerr_func;
static gboolean moo_log_handlers_set;

static void
set_print_funcs (GLogFunc   log_func,
                 GPrintFunc print_func,
                 GPrintFunc printerr_func)
{
    moo_log_func = log_func;
    moo_print_func = print_func;
    moo_printerr_func = printerr_func;
    moo_log_handlers_set = TRUE;

    g_set_print_handler (print_func);
    g_set_printerr_handler (printerr_func);

#define set(domain) \
    g_log_set_handler (domain, G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_func, NULL)

    set ("Glib");
    set ("Glib-GObject");
    set ("GModule");
    set ("GThread");
    set ("Gtk");
    set ("Gdk");
    set ("GdkPixbuf");
    set ("Pango");
    set ("Moo");
    set (NULL);
#undef set

    g_log_set_default_handler (log_func, NULL);
}


/* it doesn't care about encoding, but well, noone cares */
static void
default_print (const char *string)
{
    fputs (string, stdout);
    fflush (stdout);
}

static void
default_printerr (const char *string)
{
    fputs (string, stderr);
    fflush (stderr);
}

void
moo_reset_log_func (void)
{
    if (moo_log_handlers_set)
        set_print_funcs (moo_log_func, moo_print_func, moo_printerr_func);
    else
        set_print_funcs (g_log_default_handler, default_print, default_printerr);
}


/*
 * Display log messages in a window
 */

#if !defined(__WIN32__) || !defined(DEBUG)

static void
on_warning_or_critical_message (G_GNUC_UNUSED const char *message)
{
}

#else /* __WIN32__ && DEBUG */

static void
on_warning_or_critical_message (const char *message)
{
    _moo_abort_debug_ignore (MOO_CODE_LOC_UNKNOWN, message);
}

#endif

static void
log_func_window (const gchar    *log_domain,
                 GLogLevelFlags  flags,
                 const gchar    *message,
                 G_GNUC_UNUSED gpointer dummy)
{
    char *text;

#ifdef __WIN32__
    if (flags & (G_LOG_LEVEL_ERROR | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION))
    {
        _moo_win32_show_fatal_error (log_domain, message);
        return;
    }
#endif /* __WIN32__ */

    if (!g_utf8_validate (message, -1, NULL))
        message = "<corrupted string, invalid UTF8>";

    if (log_domain)
        text = g_strdup_printf ("%s: %s\n", log_domain, message);
    else
        text = g_strdup_printf ("%s\n", message);

    if (flags < G_LOG_LEVEL_MESSAGE)
        on_warning_or_critical_message (text);

    {
        GtkTextTag *tag;
        MooLogWindow *log;

        gdk_threads_enter ();

        if ((log = moo_log_window ()))
        {
            if (flags >= G_LOG_LEVEL_MESSAGE)
                tag = log->message_tag;
            else if (flags >= G_LOG_LEVEL_WARNING)
                tag = log->warning_tag;
            else
                tag = log->critical_tag;

            moo_log_window_insert (log, text, tag);
            if (flags <= G_LOG_LEVEL_WARNING)
                gtk_window_present (GTK_WINDOW (log->window));
        }

        gdk_threads_leave ();
    }

    g_free (text);
}


static void
print_func_window (const char *string)
{
    MooLogWindow *log;
    gdk_threads_enter ();
    if ((log = moo_log_window ()))
        moo_log_window_insert (log, string, NULL);
    gdk_threads_leave ();
}

static void
printerr_func_window (const char *string)
{
    MooLogWindow *log;
    gdk_threads_enter ();
    if ((log = moo_log_window ()))
        moo_log_window_insert (log, string, log->warning_tag);
    gdk_threads_leave ();
}


void
moo_set_log_func_window (gboolean show_now)
{
    gtk_init (NULL, NULL);

    if (show_now)
        gtk_widget_show (GTK_WIDGET (moo_log_window()->window));

    set_print_funcs (log_func_window, print_func_window, printerr_func_window);
}


/*
 * Write to a file
 */

static char *moo_log_file;
static gboolean moo_log_file_written;
static GStaticRecMutex moo_log_file_mutex = G_STATIC_REC_MUTEX_INIT;

static void
print_func_file (const char *string)
{
    FILE *file;

    g_static_rec_mutex_lock (&moo_log_file_mutex);

    if (!moo_log_file)
    {
        g_static_rec_mutex_unlock (&moo_log_file_mutex);
        g_return_if_reached ();
    }

    if (!moo_log_file_written)
    {
        file = fopen (moo_log_file, "w+");
        moo_log_file_written = TRUE;
    }
    else
    {
        file = fopen (moo_log_file, "a+");
    }

    if (file)
    {
        fprintf (file, "%s", string);
        fclose (file);
    }
    else
    {
        /* TODO ??? */
    }

    g_static_rec_mutex_unlock (&moo_log_file_mutex);
}


static void
log_func_file (const char       *log_domain,
               G_GNUC_UNUSED GLogLevelFlags flags,
               const char       *message,
               G_GNUC_UNUSED gpointer dummy)
{
    char *string;

#ifdef __WIN32__
    if (flags & (G_LOG_LEVEL_ERROR | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION))
    {
        _moo_win32_show_fatal_error (log_domain, message);
        return;
    }
#endif /* __WIN32__ */

    if (log_domain)
        string = g_strdup_printf ("%s: %s\n", log_domain, message);
    else
        string = g_strdup_printf ("%s\n", message);

    print_func_file (string);
    g_free (string);
}


void
moo_set_log_func_file (const char *log_file)
{
    g_static_rec_mutex_lock (&moo_log_file_mutex);
    g_free (moo_log_file);
    moo_log_file = g_strdup (log_file);
    set_print_funcs (log_func_file, print_func_file, print_func_file);
    g_static_rec_mutex_unlock (&moo_log_file_mutex);
}


/*
 * Do nothing
 */

static void
log_func_silent (G_GNUC_UNUSED const gchar    *log_domain,
                 G_GNUC_UNUSED GLogLevelFlags  flags,
                 G_GNUC_UNUSED const gchar    *message,
                 G_GNUC_UNUSED gpointer        data_unused)
{
#ifdef __WIN32__
    if (flags & (G_LOG_LEVEL_ERROR | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION))
    {
        _moo_win32_show_fatal_error (log_domain, message);
        return;
    }
#endif /* __WIN32__ */
}

static void
print_func_silent (G_GNUC_UNUSED const char *s)
{
}

void
moo_set_log_func_silent (void)
{
    set_print_funcs ((GLogFunc) log_func_silent,
                      (GPrintFunc) print_func_silent,
                      (GPrintFunc) print_func_silent);
}


/***************************************************************************/
/* Very useful function
 */

void MOO_NORETURN
moo_segfault (void)
{
    char *var = (char*) -1;
    var[18] = 8;
    moo_abort ();
}

void MOO_NORETURN
moo_abort (void)
{
    abort ();
}


/***************************************************************************/
/* GtkSelection helpers
 */

void
moo_selection_data_set_pointer (GtkSelectionData *data,
                                GdkAtom           type,
                                gpointer          ptr)
{
    g_return_if_fail (data != NULL);
    gtk_selection_data_set (data, type, 8, /* 8 bits per byte */
                            (gpointer) &ptr, sizeof (ptr));
}


gpointer
moo_selection_data_get_pointer (GtkSelectionData *data,
                                GdkAtom           type)
{
    GdkWindow *owner;
    gpointer result = NULL;

    g_return_val_if_fail (data != NULL, NULL);
    g_return_val_if_fail (data->target == type, NULL);
    g_return_val_if_fail (data->length == sizeof (result), NULL);

    /* If we can get the owner, the selection is in-process */
    owner = gdk_selection_owner_get_for_display (data->display, data->selection);

    if (!owner || gdk_window_get_window_type (owner) == GDK_WINDOW_FOREIGN)
        return NULL;

    memcpy (&result, data->data, sizeof (result));

    return result;
}


/***************************************************************************/
/* Get currently pressed modifier keys
 */

GdkModifierType
_moo_get_modifiers (GtkWidget *widget)
{
    GdkModifierType mask;
    GdkDisplay *display;

    g_return_val_if_fail (GTK_IS_WIDGET (widget), 0);
    g_return_val_if_fail (GTK_WIDGET_REALIZED (widget), 0);

    display = gtk_widget_get_display (widget);
    g_return_val_if_fail (display != NULL, 0);

    gdk_display_get_pointer (display, NULL, NULL, NULL, &mask);

    return mask;
}


/***************************************************************************/
/* GtkAccelLabel helpers
 */

static void
accel_label_set_string (GtkWidget  *accel_label,
                        const char *label)
{
    g_return_if_fail (GTK_IS_ACCEL_LABEL (accel_label));
    g_free (GTK_ACCEL_LABEL(accel_label)->accel_string);
    GTK_ACCEL_LABEL(accel_label)->accel_string = g_strdup (label);
    gtk_widget_queue_resize (accel_label);
}

static void
accel_label_screen_changed (GtkWidget  *accel_label)
{
    const char *label = g_object_get_data (G_OBJECT (accel_label), "moo-accel-label-accel");
    accel_label_set_string (accel_label, label);
}

void
_moo_menu_item_set_accel_label (GtkWidget  *menu_item,
                                const char *label)
{
    GtkWidget *accel_label;

    g_return_if_fail (GTK_IS_MENU_ITEM (menu_item));

    if (!label)
        label = "";

    accel_label = gtk_bin_get_child (GTK_BIN (menu_item));
    g_return_if_fail (GTK_IS_ACCEL_LABEL (accel_label));

    g_signal_connect_after (accel_label, "screen-changed",
                            G_CALLBACK (accel_label_screen_changed), NULL);
    g_object_set_data_full (G_OBJECT (accel_label), "moo-accel-label-accel",
                            g_strdup (label), g_free);
    accel_label_set_string (accel_label, label);
}


void
_moo_menu_item_set_label (GtkWidget  *item,
                          const char *text,
                          gboolean    mnemonic)
{
    GtkWidget *label;

    g_return_if_fail (GTK_IS_MENU_ITEM (item));
    g_return_if_fail (text != NULL);

    label = gtk_bin_get_child (GTK_BIN (item));
    g_return_if_fail (GTK_IS_LABEL (label));

    if (mnemonic)
        gtk_label_set_text_with_mnemonic (GTK_LABEL (label), text);
    else
        gtk_label_set_text (GTK_LABEL (label), text);
}


/***************************************************************************/
/* data dirs
 */

const char *
_moo_get_pid_string (void)
{
    static char *moo_pid_string;

    MOO_DO_ONCE_BEGIN

#ifdef __WIN32__
    moo_pid_string = g_strdup_printf ("%ld", GetCurrentProcessId ());
#else
    moo_pid_string = g_strdup_printf ("%ld", (long) getpid ());
#endif

    MOO_DO_ONCE_END

    return moo_pid_string;
}


gboolean
moo_getenv_bool (const char *var)
{
    const char *value = g_getenv (var);
    return value && *value && strcmp (value, "0") != 0;
}


char *
moo_tempnam (void)
{
    int i;
    char *filename = NULL;
    static int counter;
    G_LOCK_DEFINE_STATIC (counter);

    MOO_DO_ONCE_BEGIN
    {
        char *dirname = NULL;
        const char *short_name;

        short_name = MOO_PACKAGE_NAME;

        for (i = 0; i < 1000; ++i)
        {
            char *basename;

            basename = g_strdup_printf ("%s-%08x", short_name, g_random_int ());
            dirname = g_build_filename (g_get_tmp_dir (), basename, NULL);
            g_free (basename);

            if (_moo_mkdir (dirname) == 0)
                break;

            g_free (dirname);
            dirname = NULL;
        }

        moo_temp_dir = dirname;
        moo_install_atexit ();
    }
    MOO_DO_ONCE_END

    g_return_val_if_fail (moo_temp_dir != NULL, NULL);

    G_LOCK (counter);

    for (i = counter + 1; i < counter + 1000; ++i)
    {
        char *basename;

        basename = g_strdup_printf ("tmpfile-%03d", i);
        filename = g_build_filename (moo_temp_dir, basename, NULL);
        g_free (basename);

        if (!g_file_test (filename, G_FILE_TEST_EXISTS))
            break;

        g_free (filename);
        filename = NULL;
    }

    counter = i;

    G_UNLOCK (counter);

    if (!filename)
        g_warning ("%s: could not generate temp file name", G_STRLOC);

    return filename;
}

static void
moo_remove_tempdir (void)
{
    if (moo_temp_dir)
    {
        GError *error = NULL;
        _moo_remove_dir (moo_temp_dir, TRUE, &error);

        if (error)
        {
            _moo_message ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }

        g_free (moo_temp_dir);
        moo_temp_dir = NULL;
    }
}

static void
moo_atexit_handler (void)
{
    moo_remove_tempdir ();
}

static void
moo_install_atexit (void)
{
    static gboolean installed;

    if (!installed)
    {
        atexit (moo_atexit_handler);
        installed = TRUE;
    }
}


void
moo_cleanup (void)
{
    moo_remove_tempdir ();
}


char *
moo_get_user_cache_dir (void)
{
    return g_build_filename (g_get_user_cache_dir (), MOO_PACKAGE_NAME, NULL);
}

char *
moo_get_user_data_dir (void)
{
    G_LOCK (moo_user_data_dir);

    if (!moo_user_data_dir)
    {
#ifdef __WIN32__
        const char *basedir = g_get_user_config_dir ();
#else
        const char *basedir = g_get_user_data_dir ();
#endif

        moo_user_data_dir = g_build_filename (basedir,
                                              MOO_PACKAGE_NAME,
                                              NULL);
    }

    G_UNLOCK (moo_user_data_dir);

    return g_strdup (moo_user_data_dir);
}

void
moo_set_user_data_dir (const char *path)
{
    G_LOCK (moo_user_data_dir);

    if (moo_user_data_dir)
        g_critical ("%s: user data dir already set", G_STRLOC);

    g_free (moo_user_data_dir);
    moo_user_data_dir = g_strdup (path);

    G_UNLOCK (moo_user_data_dir);
}

void
_moo_set_app_instance_name (const char *name)
{
    if (moo_app_instance_name)
        g_critical ("%s: app instance name already set", G_STRFUNC);

    if (name && strcmp (name, "main") == 0)
        name = NULL;
    if (name && !name[0])
        name = NULL;

    g_free (moo_app_instance_name);
    moo_app_instance_name = g_strdup (name);
}

void
moo_set_display_app_name (const char *name)
{
    g_return_if_fail (name && *name);
    g_free (moo_display_app_name);
    moo_display_app_name = g_strdup (name);
}

const char *
moo_get_display_app_name (void)
{
    return moo_display_app_name ? moo_display_app_name : g_get_prgname ();
}


gboolean
moo_make_user_data_dir (const char *path)
{
    int result = 0;
    char *full_path;
    char *user_dir;

    user_dir = moo_get_user_data_dir ();
    g_return_val_if_fail (user_dir != NULL, FALSE);

    full_path = g_build_filename (user_dir, path, NULL);
    result = _moo_mkdir_with_parents (full_path);

    if (result != 0)
    {
        int err = errno;
        g_critical ("could not create directory '%s': %s",
                    full_path, g_strerror (err));
    }

    g_free (user_dir);
    g_free (full_path);
    return result == 0;
}


static gboolean
cmp_dirs (const char *dir1,
          const char *dir2)
{
    g_return_val_if_fail (dir1 != NULL, FALSE);
    g_return_val_if_fail (dir2 != NULL, FALSE);
    /* XXX */
    return !strcmp (dir1, dir2);
}


static void
add_dir_list_from_env (GPtrArray  *list,
                       const char *var)
{
    char **dirs, **p;

#ifdef __WIN32__
    dirs = g_strsplit (var, ";", 0);
    p = moo_filenames_from_locale (dirs);
    g_strfreev (dirs);
    dirs = p;
#else
    dirs = g_strsplit (var, ":", 0);
#endif

    for (p = dirs; p && *p; ++p)
        g_ptr_array_add (list, *p);

    g_free (dirs);
}


static char **
moo_get_data_dirs_real (MooDataDirType   type,
                        gboolean         include_user,
                        guint           *n_dirs)
{
    static char **moo_data_dirs[2];
    static guint n_data_dirs[2];
    G_LOCK_DEFINE_STATIC(moo_data_dirs);

    g_return_val_if_fail (type < 2, NULL);

    G_LOCK (moo_data_dirs);

    if (!moo_data_dirs[type])
    {
        const char *env[2];
        GPtrArray *dirs;
        GPtrArray *all_dirs;
        char **ptr;
        guint i;

        all_dirs = g_ptr_array_new ();
        dirs = g_ptr_array_new ();

        env[0] = g_getenv ("MOO_APP_DIRS");
        env[1] = type == MOO_DATA_SHARE ? g_getenv ("MOO_DATA_DIRS") : g_getenv ("MOO_LIB_DIRS");

        g_ptr_array_add (all_dirs, moo_get_user_data_dir ());

        /* environment variables override everything */
        if (env[0] || env[1])
        {
            if (env[1])
                add_dir_list_from_env (all_dirs, env[1]);
            else
                add_dir_list_from_env (all_dirs, env[0]);
        }
        else
        {
#ifdef __WIN32__
            _moo_win32_add_data_dirs (all_dirs, type == MOO_DATA_SHARE ? "share" : "lib");
#else
            if (type == MOO_DATA_SHARE)
            {
                const char* const *p;

                for (p = g_get_system_data_dirs (); p && *p; ++p)
                    g_ptr_array_add (all_dirs, g_build_filename (*p, MOO_PACKAGE_NAME, NULL));

                g_ptr_array_add (all_dirs, g_strdup (MOO_DATA_DIR));
            }
            else
            {
                g_ptr_array_add (all_dirs, g_strdup (MOO_LIB_DIR));
            }
#endif
        }

        g_ptr_array_add (all_dirs, NULL);

        for (ptr = (char**) all_dirs->pdata; *ptr; ++ptr)
        {
            gboolean found = FALSE;
            char *path;

            path = *ptr;

            if (!path || !path[0])
            {
                g_free (path);
                continue;
            }

            for (i = 0; i < dirs->len; ++i)
            {
                if (cmp_dirs (path, dirs->pdata[i]))
                {
                    found = TRUE;
                    break;
                }
            }

            if (!found)
                g_ptr_array_add (dirs, path);
            else
                g_free (path);
        }

        g_ptr_array_add (dirs, NULL);
        n_data_dirs[type] = dirs->len - 1;
        moo_data_dirs[type] = (char**) g_ptr_array_free (dirs, FALSE);
        g_ptr_array_free (all_dirs, TRUE);
    }

    G_UNLOCK (moo_data_dirs);

    if (include_user || !n_data_dirs[type])
    {
        if (n_dirs)
            *n_dirs = n_data_dirs[type];
        return g_strdupv (moo_data_dirs[type]);
    }
    else
    {
        if (n_dirs)
            *n_dirs = n_data_dirs[type] - 1;
        return g_strdupv (moo_data_dirs[type] + 1);
    }
}

char **
moo_get_data_dirs (MooDataDirType type,
                   guint         *n_dirs)
{
    return moo_get_data_dirs_real (type, TRUE, n_dirs);
}


const char *
moo_get_locale_dir (void)
{
#ifdef __WIN32__
    return _moo_win32_get_locale_dir ();
#else
    const char *dir = g_getenv ("MOO_LOCALE_DIR");
    return dir && dir[0] ? dir : MOO_LOCALE_DIR;
#endif
}


static char **
moo_get_stuff_subdirs (const char    *subdir,
                       MooDataDirType type,
                       gboolean       include_user)
{
    char **data_dirs, **dirs;
    guint n_dirs, i;

    g_return_val_if_fail (subdir != NULL, NULL);

    data_dirs = moo_get_data_dirs_real (type, include_user, &n_dirs);
    g_return_val_if_fail (data_dirs != NULL, NULL);

    dirs = g_new0 (char*, n_dirs + 1);

    for (i = 0; i < n_dirs; ++i)
        dirs[i] = g_build_filename (data_dirs[i], subdir, NULL);

    g_strfreev (data_dirs);
    return dirs;
}

char **
moo_get_data_subdirs (const char *subdir)
{
    return moo_get_stuff_subdirs (subdir, MOO_DATA_SHARE, TRUE);
}

char **
moo_get_sys_data_subdirs (const char *subdir)
{
    return moo_get_stuff_subdirs (subdir, MOO_DATA_SHARE, FALSE);
}

char **
moo_get_lib_subdirs (const char *subdir)
{
    return moo_get_stuff_subdirs (subdir, MOO_DATA_LIB, TRUE);
}


static char *
get_user_data_file (const char *basename,
                    gboolean    cache)
{
    char *dir, *file;

    g_return_val_if_fail (basename && basename[0], NULL);

    if (cache)
        dir = moo_get_user_cache_dir ();
    else
        dir = moo_get_user_data_dir ();

    g_return_val_if_fail (dir != NULL, NULL);

    file = g_build_filename (dir, basename, NULL);

    g_free (dir);
    return file;
}

char *
moo_get_named_user_data_file (const char *basename)
{
    char *freeme = NULL;
    char *file;

    g_return_val_if_fail (basename && basename[0], NULL);

    if (moo_app_instance_name)
    {
        freeme = g_strdup_printf ("%s-%s", moo_app_instance_name, basename);
        basename = freeme;
    }

    file = moo_get_user_data_file (basename);

    g_free (freeme);
    return file;
}

char *
moo_get_user_data_file (const char *basename)
{
    return get_user_data_file (basename, FALSE);
}

char *
moo_get_user_cache_file (const char *basename)
{
    return get_user_data_file (basename, TRUE);
}


static gboolean
save_config_file (const char     *dir,
                  const char     *filename,
                  const char     *content,
                  gssize          len,
                  GError        **error)
{
    MooFileWriter *writer;
    gboolean retval;

    g_return_val_if_fail (dir != NULL, FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);
    g_return_val_if_fail (content != NULL, FALSE);

    if (!(writer = moo_config_writer_new (filename, TRUE, error)))
        return FALSE;

    moo_file_writer_write (writer, content, len);
    retval = moo_file_writer_close (writer, error);

    return retval;
}

static gboolean
save_user_data_file (const char     *basename,
                     gboolean        cache,
                     const char     *content,
                     gssize          len,
                     GError        **error)
{
    char *dir, *file;
    gboolean result;

    g_return_val_if_fail (basename != NULL, FALSE);
    g_return_val_if_fail (content != NULL, FALSE);

    if (cache)
    {
        dir = moo_get_user_cache_dir ();
        file = moo_get_user_cache_file (basename);
    }
    else
    {
        dir = moo_get_user_data_dir ();
        file = moo_get_user_data_file (basename);
    }

    result = save_config_file (dir, file, content, len, error);

    g_free (dir);
    g_free (file);
    return result;
}

gboolean
moo_save_user_data_file (const char  *basename,
                         const char  *content,
                         gssize       len,
                         GError     **error)
{
    return save_user_data_file (basename, FALSE, content, len, error);
}

gboolean
moo_save_user_cache_file (const char  *basename,
                          const char  *content,
                          gssize       len,
                          GError     **error)
{
    return save_user_data_file (basename, TRUE, content, len, error);
}

gboolean
moo_save_config_file (const char   *filename,
                      const char   *content,
                      gssize        len,
                      GError      **error)
{
    char *dir;
    gboolean result;

    g_return_val_if_fail (filename != NULL, FALSE);
    g_return_val_if_fail (content != NULL, FALSE);

    dir = g_path_get_dirname (filename);

    result = save_config_file (dir, filename, content, len, error);

    g_free (dir);
    return result;
}


void
_moo_widget_set_tooltip (GtkWidget  *widget,
                         const char *tip)
{
#if !GTK_CHECK_VERSION(2,11,6)
    static GtkTooltips *tooltips;

    g_return_if_fail (GTK_IS_WIDGET (widget));

    if (!tooltips)
        tooltips = gtk_tooltips_new ();

    if (GTK_IS_TOOL_ITEM (widget))
        gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (widget), tooltips, tip, NULL);
    else
        gtk_tooltips_set_tip (tooltips, widget, tip, tip);
#else
    g_return_if_fail (GTK_IS_WIDGET (widget));

    if (GTK_IS_TOOL_ITEM (widget))
        gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (widget), tip);
    else
        gtk_widget_set_tooltip_text (widget, tip);
#endif
}


/*******************************************************************************
 * Asserts
 */

#if !defined(__WIN32__)

void
_moo_abort_debug_ignore (MooCodeLoc loc, const char *message)
{
    if (moo_code_loc_valid (loc))
	g_error ("file '%s', function '%s', line %d: %s\n", loc.file, loc.func, loc.line, message);
    else
        g_error ("%s\n", message);
}

#else /* !__WIN32__ */

static void
break_into_debugger (void)
{
    RaiseException(0, EXCEPTION_NONCONTINUABLE, 0, NULL);
}

void
_moo_abort_debug_ignore (MooCodeLoc loc, const char *message)
{
    gboolean skip = FALSE;
    static GHashTable *locs_hash;
    char *loc_id = NULL;
    gboolean use_location;

    use_location = moo_code_loc_valid (loc);

    if (use_location)
        loc_id = g_strdup_printf ("%s#%d#%d", loc.file, loc.line, loc.counter);
    else
        loc_id = g_strdup (message);

    if (loc_id != NULL)
    {
        if (!locs_hash)
            locs_hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
        if (g_hash_table_lookup (locs_hash, loc_id))
            skip = TRUE;
    }

    if (!skip)
    {
        int ret;

        if (use_location)
            ret = _moo_win32_message_box (NULL, MB_ABORTRETRYIGNORE, NULL,
                                          Q_("console message|in file %s, line %d, function %s:\n%s"),
                                          loc.file, loc.line, loc.func, message);
        else
            ret = _moo_win32_message_box (NULL, MB_ABORTRETRYIGNORE, NULL,
                                          "%s", message);

        switch (ret)
        {
            case IDABORT:
                TerminateProcess (GetCurrentProcess (), 1);
                break;

            case IDRETRY:
                break_into_debugger ();
                break;

            case IDIGNORE:
                if (GetKeyState(VK_CONTROL) < 0)
                    skip = TRUE;
                break;

            default:
                moo_abort ();
                break;
        }

        if (skip && loc_id)
        {
            g_hash_table_insert (locs_hash, loc_id, GINT_TO_POINTER (1));
            loc_id = NULL;
        }
    }

    g_free (loc_id);
}

#endif /* !__WIN32__ */

#if !defined(MOO_DEV_MODE) && !defined(DEBUG)
NORETURN
#endif
void
_moo_assert_message (MooCodeLoc loc, const char *message)
{
#if defined(MOO_DEV_MODE) && defined(__WIN32__)
    _moo_abort_debug_ignore (loc, message);
#elif defined(MOO_DEV_MODE) || defined(DEBUG)
    g_critical ("file '%s', function '%s', line %d: %s\n", loc.file, loc.func, loc.line, message);
#else
    g_error ("file '%s', function '%s', line %d: %s\n", loc.file, loc.func, loc.line, message);
#endif
}

void
_moo_log (MooCodeLoc loc, GLogLevelFlags flags, const char *format, ...)
{
    va_list args;
    va_start (args, format);
    _moo_logv (loc, flags, format, args);
    va_end (args);
}

static gboolean
moo_log_debug_enabled (void)
{
    static int enabled = -1;

    if (enabled == -1)
    {
        enabled = moo_debug_enabled ("misc",
#ifdef MOO_DEBUG
                                     TRUE);
#else
                                     FALSE);
#endif
    }

    return enabled;
}

void _moo_logv (MooCodeLoc loc, GLogLevelFlags flags, const char *format, va_list args)
{
    char *message = g_strdup_vprintf (format, args);

    if (flags >= G_LOG_LEVEL_DEBUG && !moo_log_debug_enabled ())
        return;

#if defined(MOO_DEV_MODE) && !defined(__WIN32__)
    if (flags < G_LOG_LEVEL_MESSAGE)
    {
        _moo_abort_debug_ignore (loc, message);
        flags = G_LOG_LEVEL_MESSAGE;
    }
#endif

    if (moo_code_loc_valid (loc))
        g_log (G_LOG_DOMAIN, flags,
               Q_("console message|in file %s, line %d, function %s: %s"),
               loc.file, loc.line, loc.func, message);
    else
        g_log (G_LOG_DOMAIN, flags, "%s", message);

    g_free (message);
}

void MOO_NORETURN _moo_error (MooCodeLoc loc, const char *format, ...)
{
    va_list args;
    va_start (args, format);
    _moo_errorv (loc, format, args);
    va_end (args);
}

void MOO_NORETURN _moo_errorv (MooCodeLoc loc, const char *format, va_list args)
{
    MOO_UNUSED (loc);
    g_logv (G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, format, args);
    moo_abort ();
}


/*******************************************************************************
 * Former eggregex stuff
 */
gboolean
_moo_regex_escape (const char *string,
                   int         bytes,
                   GString    *dest)
{
    const char *p, *piece, *end;
    gboolean escaped = FALSE;

    g_return_val_if_fail (string != NULL, TRUE);
    g_return_val_if_fail (dest != NULL, TRUE);

    if (bytes < 0)
        bytes = strlen (string);

    end = string + bytes;
    p = piece = string;

    while (p < end)
    {
        switch (*p)
        {
            case '\\':
            case '|':
            case '(':
            case ')':
            case '[':
            case ']':
            case '{':
            case '}':
            case '^':
            case '$':
            case '*':
            case '+':
            case '?':
            case '.':
                escaped = TRUE;
                if (p != piece)
                    g_string_append_len (dest, piece, p - piece);
                g_string_append_c (dest, '\\');
                g_string_append_c (dest, *p);
                piece = ++p;
                break;

            default:
                p = g_utf8_next_char (p);
                break;
        }
    }

    if (escaped && piece < end)
        g_string_append_len (dest, piece, end - piece);

    return escaped;
}


char **
moo_strnsplit_lines (const char *string,
                     gssize      len,
                     guint      *n_lines)
{
    MooLineReader lr;
    GPtrArray *array = NULL;
    const char *line;
    gsize line_len;

    for (moo_line_reader_init (&lr, string, len);
         (line = moo_line_reader_get_line (&lr, &line_len, NULL)); )
    {
        if (!array)
            array = g_ptr_array_new ();
        g_ptr_array_add (array, g_strndup (line, line_len));
    }

    if (array)
    {
        if (n_lines)
            *n_lines = array->len;
        g_ptr_array_add (array, NULL);
        return (char**) g_ptr_array_free (array, FALSE);
    }
    else
    {
        if (n_lines)
            *n_lines = 0;
        return NULL;
    }
}

char **
moo_splitlines (const char *string)
{
    GPtrArray *array = NULL;
    MooLineReader lr;
    const char *line;
    gsize line_len;

    for (moo_line_reader_init (&lr, string, -1);
         (line = moo_line_reader_get_line (&lr, &line_len, NULL)); )
    {
        if (!array)
            array = g_ptr_array_new ();
        g_ptr_array_add (array, g_strndup (line, line_len));
    }

    if (array)
    {
        g_ptr_array_add (array, NULL);
        return (char**) g_ptr_array_free (array, FALSE);
    }
    else
    {
        return NULL;
    }
}

void
moo_line_reader_init (MooLineReader *lr,
                      const char    *text,
                      gssize         len)
{
    g_return_if_fail (lr != NULL);

    if (len < 0)
        len = text ? strlen (text) : 0;

    lr->text = text && len ? text : NULL;
    lr->len = lr->text ? len : 0;
}

const char *
moo_line_reader_get_line (MooLineReader *lr,
                          gsize         *line_len,
                          gsize         *lt_len)
{
    const char *le, *end, *line;
    gsize le_len;

    g_return_val_if_fail (lr != NULL, NULL);

    if (!lr->text)
        return NULL;

    for (le = lr->text, le_len = 0, end = lr->text + lr->len; le < end && le_len == 0; )
    {
        switch (*le)
        {
            case '\r':
                if (le + 1 < end && le[1] == '\n')
                    le_len = 2;
                else
                    le_len = 1;
                break;
            case '\n':
                le_len = 1;
                break;
            case '\xe2': /* Unicode paragraph separator "\xe2\x80\xa9" */
                if (le + 2 < end && le[1] == '\x80' && le[2] == '\xa9')
                {
                    le_len = 3;
                    break;
                }
                /* fallthrough */
            default:
                le += 1;
                break;
        }
    }

    line = lr->text;

    if (le_len)
    {
        lr->text = le + le_len;
        lr->len -= (le - line) + le_len;
    }
    else
    {
        lr->text = NULL;
        lr->len = 0;
    }

    if (line_len)
        *line_len = le - line;
    if (lt_len)
        *lt_len = le_len;

    return line;
}


gboolean
moo_find_line_end (const char *string,
                   gssize      len,
                   gsize      *line_len,
                   gsize      *lt_len)
{
    const char *le, *end;
    gsize le_len;

    g_return_val_if_fail (string != NULL || len == 0, FALSE);

    if (len < 0)
        len = strlen (string);

    for (le = string, le_len = 0, end = string + len; le < end && le_len == 0; )
    {
        switch (*le)
        {
            case '\r':
                if (le + 1 < end && le[1] == '\n')
                    le_len = 2;
                else
                    le_len = 1;
                break;
            case '\n':
                le_len = 1;
                break;
            case '\xe2': /* Unicode paragraph separator "\xe2\x80\xa9" */
                if (le + 2 < end && le[1] == '\x80' && le[2] == '\xa9')
                {
                    le_len = 3;
                    break;
                }
                /* fallthrough */
            default:
                le += 1;
                break;
        }
    }

    if (line_len != NULL)
        *line_len = le - string;
    if (lt_len != NULL)
        *lt_len = le_len;

    return le_len != 0;
}


char **
_moo_strv_reverse (char **str_array)
{
    guint i, len;

    if (str_array)
    {
        for (len = 0; str_array[len]; len++) ;

        for (i = 0; i < len/2; ++i)
        {
            char *tmp = str_array[i];
            str_array[i] = str_array[len-i-1];
            str_array[len-i-1] = tmp;
        }
    }

    return str_array;
}


gboolean
_moo_str_equal (const char *s1,
                const char *s2)
{
    return strcmp (s1 ? s1 : "", s2 ? s2 : "") == 0;
}


#if defined(__WIN32__) && !defined(MOO_DEBUG)
static guint saved_win32_error_mode;
#endif

void
moo_disable_win32_error_message (void)
{
#if defined(__WIN32__) && !defined(MOO_DEBUG)
    saved_win32_error_mode = SetErrorMode (SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
#endif
}

void
moo_enable_win32_error_message (void)
{
#if defined(__WIN32__) && !defined(MOO_DEBUG)
    SetErrorMode (saved_win32_error_mode);
#endif
}


#if 0
#undef moo_str_equal
gboolean
moo_str_equal (const char *s1,
               const char *s2)
{
    return !strcmp (s1 ? s1 : "", s2 ? s2 : "");
}
#endif


typedef struct {
    GSourceFunc func;
    gpointer data;
    GDestroyNotify notify;
} SourceData;

static void
source_data_free (gpointer data)
{
    SourceData *sd = data;
    if (sd && sd->notify)
        sd->notify (sd->data);
    g_free (sd);
}

static gboolean
thread_source_func (gpointer data)
{
    SourceData *sd = data;
    gboolean ret = FALSE;

    gdk_threads_enter ();

#if GLIB_CHECK_VERSION(2,12,0)
    if (!g_source_is_destroyed (g_main_current_source ()))
#endif
        ret = sd->func (sd->data);

    gdk_threads_leave ();
    return ret;
}

guint
moo_idle_add_full (gint           priority,
                   GSourceFunc    function,
                   gpointer       data,
                   GDestroyNotify notify)
{
    SourceData *sd;

    sd = g_new (SourceData, 1);
    sd->func = function;
    sd->data = data;
    sd->notify = notify;

    return g_idle_add_full (priority, thread_source_func,
                            sd, source_data_free);
}


guint
moo_idle_add (GSourceFunc function,
              gpointer    data)
{
    return moo_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
                              function, data, NULL);
}


guint
_moo_timeout_add_full (gint           priority,
                       guint          interval,
                       GSourceFunc    function,
                       gpointer       data,
                       GDestroyNotify notify)
{
    SourceData *sd;

    sd = g_new (SourceData, 1);
    sd->func = function;
    sd->data = data;
    sd->notify = notify;

    return g_timeout_add_full (priority, interval,
                               thread_source_func,
                               sd, source_data_free);
}


guint
_moo_timeout_add (guint       interval,
                  GSourceFunc function,
                  gpointer    data)
{
    return _moo_timeout_add_full (G_PRIORITY_DEFAULT, interval,
                                  function, data, NULL);
}


static gboolean
thread_io_func (GIOChannel  *source,
                GIOCondition condition,
                gpointer     data)
{
    SourceData *sd = data;
    gboolean ret = FALSE;

    gdk_threads_enter ();

#if GLIB_CHECK_VERSION(2,12,0)
    if (!g_source_is_destroyed (g_main_current_source ()))
#endif
        ret = ((GIOFunc) sd->func) (source, condition, sd->data);

    gdk_threads_leave ();
    return ret;
}

guint
_moo_io_add_watch (GIOChannel   *channel,
                   GIOCondition  condition,
                   GIOFunc       func,
                   gpointer      data)
{
    return _moo_io_add_watch_full (channel, G_PRIORITY_DEFAULT,
                                   condition, func, data, NULL);
}


guint
_moo_io_add_watch_full (GIOChannel    *channel,
                        int            priority,
                        GIOCondition   condition,
                        GIOFunc        func,
                        gpointer       data,
                        GDestroyNotify notify)
{
    SourceData *sd;

    sd = g_new (SourceData, 1);
    sd->func = (GSourceFunc) func;
    sd->data = data;
    sd->notify = notify;

    return g_io_add_watch_full (channel, priority, condition,
                                thread_io_func, sd, source_data_free);
}


const char *
_moo_intern_string (const char *string)
{
    if (!string)
        return NULL;

#if GLIB_CHECK_VERSION(2,10,0)
    return g_intern_string (string);
#else
    {
        static GHashTable *hash;
        gpointer original;
        gpointer dummy;

        if (G_UNLIKELY (!hash))
            hash = g_hash_table_new (g_str_hash, g_str_equal);

        if (!g_hash_table_lookup_extended (hash, string, &original, &dummy))
        {
            char *copy = g_strdup (string);
            g_hash_table_insert (hash, copy, NULL);
            original = copy;
        }

        return original;
    }
#endif
}

GdkAtom
moo_atom_uri_list (void)
{
    MOO_DEFINE_ATOM_ (text/uri-list)
}


static char *debug_domains;
void _moo_set_debug (const char *domains);
gboolean moo_debug_enabled (const char *domain, gboolean def_enabled);

void
_moo_set_debug (const char *domains)
{
    g_free (debug_domains);
    debug_domains = g_strdup (domains);
}

gboolean
moo_debug_enabled (const char *domain,
                   gboolean    def_enabled)
{
    const char *val;
    char **domains, **p;

    val = g_getenv ("MOO_DEBUG");
    if (!val || !val[0])
        val = debug_domains;

    if (!val || !val[0])
        return def_enabled;
    if (!strcmp (val, "none"))
        return FALSE;
    if (!strcmp (val, "all"))
        return TRUE;

    domains = g_strsplit_set (val, ",;", 0);
    for (p = domains; p && *p; ++p)
    {
        if (strcmp (domain, *p) == 0)
        {
            g_strfreev (domains);
            return TRUE;
        }
    }

    g_strfreev (domains);
    return FALSE;
}


static void
test_strv_one (const char *string,
               gssize      len,
               char const *expected[])
{
    const char *s;
    char *freeme = NULL;
    char **res;
    guint n_toks;

    if (len < 0)
        s = string;
    else if (len == 0)
        s = "";
    else
        s = freeme = g_strndup (string, len);

    res = moo_splitlines (s);
    TEST_ASSERT_STRV_EQ_MSG (res, (char**) expected,
                             "moo_splitlines(%s)", TEST_FMT_STR (s));
    g_strfreev (res);
    g_free (freeme);
    freeme = NULL;

    res = moo_strnsplit_lines (string, len, &n_toks);
    TEST_ASSERT_STRV_EQ_MSG (res, (char**) expected,
                             "moo_strnsplit_lines(%s, %d)",
                             TEST_FMT_STR (s), (int) len);
    TEST_ASSERT_INT_EQ (n_toks, res ? g_strv_length (res) : 0);
    g_strfreev (res);
}

static void
test_moo_splitlines (void)
{
    guint i;

    struct {
        const char *s;
        gssize len;
        char const *toks[10];
    } cases[] = {
        { "abcd", -1, {"abcd", NULL} },
        { "abcd\n", -1, {"abcd", "", NULL} },
        { "abcd\r\n", -1, {"abcd", "", NULL} },
        { "\rabcd\n", -1, {"", "abcd", "", NULL} },
        { "\r\nabcd\n", -1, {"", "abcd", "", NULL} },
        { "abcd\nabc", -1, {"abcd", "abc", NULL} },
        { "abcd\n", 4, {"abcd", NULL} },
        { "a\nb\rc\r\nd\xe2\x80\xa9""e", -1, {"a", "b", "c", "d", "e", NULL} },
        { "a\xe2\x80\xa9""b", 1, {"a", NULL} },
        { "a\xe2\x80\xa9""b", 3, {"a\xe2\x80", NULL} },
        { "a\xe2\x80\xa9""b", 4, {"a", "", NULL} },
        { "a\xe2\x80\xa9""b", 5, {"a", "b", NULL} }
    };

    struct {
        const char *s;
        gssize len;
    } nulls[] = {
        { NULL, -1 },
        { "", -1 },
        { "", 0 },
        { "abcd", 0 }
    };

    for (i = 0; i < G_N_ELEMENTS (cases); ++i)
        test_strv_one (cases[i].s, cases[i].len, cases[i].toks);

    for (i = 0; i < G_N_ELEMENTS (nulls); ++i)
        test_strv_one (nulls[i].s, nulls[i].len, NULL);
}


void
moo_test_mooutils_misc (void)
{
    MooTestSuite *suite;

    suite = moo_test_suite_new ("mooutils-misc", "mooutils/mooutils-misc.c", NULL, NULL, NULL);

    moo_test_suite_add_test (suite, "moo_splitlines", "test of moo_splitlines()",
                             (MooTestFunc) test_moo_splitlines, NULL);
}
