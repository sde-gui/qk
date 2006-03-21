/*
 *   mooutils-misc.c
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
# include "config.h"
#endif

#include "mooutils/mooutils-misc.h"
#include "mooutils/moologwindow-glade.h"
#include "mooutils/mooglade.h"
#include <gtk/gtk.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>

#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#endif

#ifdef __WIN32__

static gboolean
open_uri (const char *uri,
          G_GNUC_UNUSED gboolean email)
{
    HINSTANCE h;

    g_return_val_if_fail (uri != NULL, FALSE);

    h = ShellExecute (NULL, "open", uri, NULL, NULL, SW_SHOWNORMAL);
    return (int)h > 32;
}

#else /* ! __WIN32__ */

#include <gdk/gdkx.h>

typedef enum {
    KDE,
    GNOME,
    DEBIAN,
    UNKNOWN
} Desktop;


static gboolean open_uri (const char *uri,
                          gboolean email /* = FALSE */)
{
    gboolean result = FALSE;
    char **argv = NULL;

    Desktop desktop = UNKNOWN;
    char *kfmclient = g_find_program_in_path ("kfmclient");
    char *gnome_open = g_find_program_in_path ("gnome-open");
    char *x_www_browser = g_find_program_in_path ("x-www-browser");

    if (!email && x_www_browser &&
        g_file_test ("/etc/debian_version", G_FILE_TEST_EXISTS))
            desktop = DEBIAN;

    if (desktop == UNKNOWN)
    {
        if (kfmclient && g_getenv ("KDE_FULL_SESSION"))
            desktop = KDE;
        else if (gnome_open && g_getenv ("GNOME_DESKTOP_SESSION_ID"))
            desktop = GNOME;
    }

#ifdef GDK_WINDOWING_X11
    if (desktop == UNKNOWN)
    {
        const char *wm =
            gdk_x11_screen_get_window_manager_name (gdk_screen_get_default ());

        if (wm)
        {
            if (!g_ascii_strcasecmp (wm, "kwin") && kfmclient)
                desktop = KDE;
            else if (!g_ascii_strcasecmp (wm, "metacity") && gnome_open)
                desktop = GNOME;
        }
    }
#endif /* GDK_WINDOWING_X11 */

    if (desktop == UNKNOWN)
    {
        if (!email && x_www_browser)
            desktop = DEBIAN;
        else if (kfmclient)
            desktop = KDE;
        else if (gnome_open)
            desktop = GNOME;
    }

    switch (desktop)
    {
        case KDE:
            argv = g_new0 (char*, 4);
            argv[0] = g_strdup (kfmclient);
            argv[1] = g_strdup ("exec");
            argv[2] = g_strdup (uri);
            break;

        case GNOME:
            argv = g_new0 (char*, 3);
            argv[0] = g_strdup (gnome_open);
            argv[1] = g_strdup (uri);
            break;

        case DEBIAN:
            argv = g_new0 (char*, 3);
            argv[0] = g_strdup (x_www_browser);
            argv[1] = g_strdup (uri);
            break;

        case UNKNOWN:
            if (uri)
                g_warning ("could not find a way to open uri '%s'", uri);
            else
                g_warning ("could not find a way to open uri");
            break;

        default:
            g_assert_not_reached ();
    }

    if (argv)
    {
        GError *err = NULL;

        result = g_spawn_async (NULL, argv, NULL, (GSpawnFlags)0, NULL, NULL,
                                NULL, &err);

        if (err)
        {
            g_warning ("%s: error in g_spawn_async", G_STRLOC);
            g_warning ("%s: %s", G_STRLOC, err->message);
            g_error_free (err);
        }
    }

    if (argv)
        g_strfreev (argv);

    g_free (gnome_open);
    g_free (kfmclient);
    g_free (x_www_browser);

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
    return open_uri (url, FALSE);
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
                              (guchar**) &data);

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


GtkWindow*
moo_get_top_window (GSList *windows)
{
    GArray *xids;
    Display *display;
    Atom actual_type_return;
    int actual_format_return;
    gulong nitems_return;
    gulong bytes_after_return;
    XID *data;
    int ret;
    long i;
    GSList *l;

    static Atom list_stacking_atom = None;

    g_return_val_if_fail (windows != NULL, NULL);

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
                              (guchar**) &data);

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


gboolean
moo_window_is_hidden (GtkWindow  *window)
{
    g_return_val_if_fail (GTK_IS_WINDOW (window), FALSE);
    return is_minimized (GDK_WINDOW_XDISPLAY (GTK_WIDGET(window)->window),
                         GDK_WINDOW_XID (GTK_WIDGET(window)->window));
}


#elif defined(__WIN32__)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gdk/gdkwin32.h>


#define get_handle(w) \
    gdk_win32_drawable_get_handle (GTK_WIDGET(w)->window)

gboolean
moo_window_is_hidden (GtkWindow  *window)
{
    HANDLE h;
    WINDOWPLACEMENT info = {0};

    g_return_val_if_fail (GTK_IS_WINDOW (window), FALSE);

    h = get_handle (window);
    g_return_val_if_fail (h != NULL, FALSE);

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


GtkWindow*
moo_get_top_window (GSList *windows)
{
    GSList *l;
    HWND top = NULL;
    HWND current = NULL;

    g_return_val_if_fail (windows != NULL, NULL);

    for (l = windows; l != NULL; l = l->next)
    {
        g_return_val_if_fail (GTK_IS_WINDOW (l->data), NULL);

        if (!moo_window_is_hidden (GTK_WINDOW (l->data)))
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

#else /* neither X nor WIN32 ?? */

GtkWindow*
moo_get_top_window (GSList *windows)
{
    g_return_val_if_fail (windows != NULL, NULL);
    g_critical ("%s: don't know how to do it", G_STRLOC);
    return GTK_WINDOW (windows->data);
}

#endif


/* TODO use gtk_window_present_with_time(), use Xlib? */
void
moo_window_present (GtkWindow *window)
{
    guint32 stamp;

    g_return_if_fail (GTK_IS_WINDOW (window));

#if !GTK_CHECK_VERSION(2,8,0) || !defined(GDK_WINDOWING_X11)
    gtk_window_present (window);
#else
    if (!GTK_WIDGET_REALIZED (window))
        return gtk_window_present (window);

    stamp = gdk_x11_get_server_time (GTK_WIDGET(window)->window);
    gtk_window_present_with_time (window, stamp);
#endif
}


/* XXX check what gtk_window_set_icon_name() does */
gboolean
moo_window_set_icon_from_stock (G_GNUC_UNUSED GtkWindow      *window,
                                G_GNUC_UNUSED const char     *stock_id)
{
#ifndef __WIN32__
    GdkPixbuf *icon;

    g_return_val_if_fail (GTK_IS_WINDOW (window), FALSE);
    g_return_val_if_fail (stock_id != NULL, FALSE);

    icon = gtk_widget_render_icon (GTK_WIDGET (window), stock_id,
                                   GTK_ICON_SIZE_BUTTON, 0);

    if (icon)
    {
        gtk_window_set_icon (GTK_WINDOW (window), icon);
        gdk_pixbuf_unref (icon);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
#else /* __WIN32__ */
    return TRUE;
#endif /* __WIN32__ */
}


GtkWindow*
moo_get_toplevel_window (void)
{
    GList *list, *l;
    GSList *windows = NULL;
    GtkWindow *top;

    list = gtk_window_list_toplevels ();

    for (l = list; l != NULL; l = l->next)
        if (GTK_IS_WINDOW (l->data) && GTK_WIDGET(l->data)->window)
            windows = g_slist_prepend (windows, l->data);

    top = moo_get_top_window (windows);

    g_list_free (list);
    g_slist_free (windows);
    return top;
}


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
    MooGladeXML *xml;
    MooLogWindow *log;
    PangoFontDescription *font;

    xml = moo_glade_xml_new_from_buf (MOO_LOG_WINDOW_GLADE_UI, -1, NULL);
    log = g_new (MooLogWindow, 1);

    log->window = moo_glade_xml_get_widget (xml, "window");
    log->textview = moo_glade_xml_get_widget (xml, "textview");

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
    static MooLogWindow *log = NULL;

    if (!log)
    {
        log = moo_log_window_new ();
        g_object_add_weak_pointer (G_OBJECT (log->window), (gpointer*) &log);
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
static char *moo_log_file;
static gboolean moo_log_file_written;
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

    g_log_set_handler ("Glib", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_func, NULL);
    g_log_set_handler ("Glib-GObject", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_func, NULL);
    g_log_set_handler ("GModule", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_func, NULL);
    g_log_set_handler ("GThread", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_func, NULL);
    g_log_set_handler ("Gtk", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_func, NULL);
    g_log_set_handler ("Gdk", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_func, NULL);
    g_log_set_handler ("Gdk-Pixbuf-CSource", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_func, NULL);
    g_log_set_handler ("GdkPixbuf", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_func, NULL);
    g_log_set_handler ("Pango", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_func, NULL);
    g_log_set_handler ("Moo", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_func, NULL);
    g_log_set_handler (NULL, G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_func, NULL);
#if GLIB_CHECK_VERSION(2,6,0)
    g_log_set_default_handler (log_func, NULL);
#endif /* GLIB_CHECK_VERSION(2,6,0) */
}


void
moo_reset_log_func (void)
{
    if (moo_log_handlers_set)
        set_print_funcs (moo_log_func, moo_print_func, moo_printerr_func);
}


void
moo_print (const char *string)
{
    g_print ("%s", string);
}

void
moo_print_err (const char *string)
{
    g_printerr ("%s", string);
}


/*
 * Win32 message box for fatal errors
 */

#ifdef __WIN32__

#define PLEASE_REPORT \
    "Please report it to emuntyan@sourceforge.net and provide "\
    "steps needed to reproduce this error."

static void
show_fatal_error_win32 (const char *domain,
                        const char *logmsg)
{
    char *msg = NULL;

    if (domain)
        msg = g_strdup_printf ("Fatal GGAP error:\n---\n%s: %s\n---\n"
                PLEASE_REPORT, domain, logmsg);
    else
        msg = g_strdup_printf ("Fatal GGAP error:\n---\n%s\n---\n"
                PLEASE_REPORT, logmsg);

    MessageBox (NULL, msg, "Error",
                MB_ICONERROR | MB_APPLMODAL | MB_SETFOREGROUND);

    g_free (msg);
}
#endif /* __WIN32__ */


/*
 * Display log messages in a window
 */

static void
log_func_window (const gchar    *log_domain,
                 GLogLevelFlags  flags,
                 const gchar    *message,
                 G_GNUC_UNUSED gpointer dummy)
{
    char *text;
    GtkTextTag *tag;
    MooLogWindow *log;

#ifdef __WIN32__
    if (flags & (G_LOG_LEVEL_ERROR | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION))
    {
        show_fatal_error_win32 (log_domain, message);
        return;
    }
#endif /* __WIN32__ */

    if (log_domain)
        text = g_strdup_printf ("%s: %s\n", log_domain, message);
    else
        text = g_strdup_printf ("%s\n", message);

    log = moo_log_window ();

    if (flags >= G_LOG_LEVEL_MESSAGE)
        tag = log->message_tag;
    else if (flags >= G_LOG_LEVEL_WARNING)
        tag = log->warning_tag;
    else
        tag = log->critical_tag;

    moo_log_window_insert (log, text, tag);
    g_free (text);
}


static void
print_func_window (const char *string)
{
    MooLogWindow *log = moo_log_window ();
    moo_log_window_insert (log, string, NULL);
}

static void
printerr_func_window (const char *string)
{
    MooLogWindow *log = moo_log_window ();
    moo_log_window_insert (log, string, log->warning_tag);
}


void
moo_set_log_func_window (gboolean show_now)
{
    if (show_now)
        gtk_widget_show (GTK_WIDGET (moo_log_window()->window));

    set_print_funcs (log_func_window, print_func_window, printerr_func_window);
}


/*
 * Write to a file
 */

static void
print_func_file (const char *string)
{
    FILE *file;

    g_return_if_fail (moo_log_file != NULL);

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
        show_fatal_error_win32 (log_domain, message);
        return;
    }
#endif /* __WIN32__ */

#ifdef __WIN32__
#define LT "\r\n"
#else
#define LT "\n"
#endif
    if (log_domain)
        string = g_strdup_printf ("%s: %s" LT, log_domain, message);
    else
        string = g_strdup_printf ("%s" LT, message);
#undef LT

    print_func_file (string);
    g_free (string);
}


void
moo_set_log_func_file (const char *log_file)
{
    g_free (moo_log_file);
    moo_log_file = g_strdup (log_file);
    set_print_funcs (log_func_file, print_func_file, print_func_file);
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
        show_fatal_error_win32 (log_domain, message);
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

void
moo_segfault (void)
{
    char *var = (char*) -1;
    var[18] = 8;
}


/***************************************************************************/
/* GtkSelection helpers
 */

void
moo_selection_data_set_pointer (GtkSelectionData *data,
                                GdkAtom         type,
                                gpointer        ptr)
{
    g_return_if_fail (data != NULL);
    gtk_selection_data_set (data, type, 8, /* 8 bits per byte */
                            (gpointer) &ptr, sizeof (ptr));
}


gpointer
moo_selection_data_get_pointer (GtkSelectionData *data,
                                GdkAtom         type)
{
    gpointer result = NULL;

    g_return_val_if_fail (data != NULL, NULL);
    g_return_val_if_fail (data->target == type, NULL);
    g_return_val_if_fail (data->length == sizeof (result), NULL);

    memcpy (&result, data->data, sizeof (result));

    return result;
}


/***************************************************************************/
/* Get currently pressed modifier keys
 */

GdkModifierType
moo_get_modifiers (GtkWidget *widget)
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

void
moo_menu_item_set_accel_label (GtkWidget      *menu_item,
                               const char     *label)
{
    GtkWidget *accel_label;

    g_return_if_fail (GTK_IS_MENU_ITEM (menu_item));

    if (!label)
        label = "";

    accel_label = gtk_bin_get_child (GTK_BIN (menu_item));
    g_return_if_fail (GTK_IS_ACCEL_LABEL (accel_label));

    g_free (GTK_ACCEL_LABEL(accel_label)->accel_string);
    GTK_ACCEL_LABEL(accel_label)->accel_string = g_strdup (label);

    gtk_widget_queue_resize (accel_label);
}


void
moo_menu_item_set_label (GtkWidget      *item,
                         const char     *text,
                         gboolean        mnemonic)
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
