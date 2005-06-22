/*
 *   mooutils/moolog.c
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

#include <stdio.h>
#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif /* __WIN32__ */
#include "mooutils/moolog.h"
#include <gtk/gtk.h>


typedef struct _MooLogWindow MooLogWindow;
struct _MooLogWindow {
    GtkWidget *window;
    GtkTextTag *message_tag;
    GtkTextTag *warning_tag;
    GtkTextTag *critical_tag;
    GtkTextBuffer *buf;
    GtkTextView *textview;
    GtkTextMark *insert;
};


MooLogWindow    *moo_log_window             (void);
GtkWidget       *moo_log_window_get_widget  (void);


static MooLogWindow *moo_log_window_new (void)
{
    MooLogWindow *log;
    GtkWidget *vbox, *scrolledwindow;
    PangoFontDescription *font;

    log = g_new (MooLogWindow, 1);

    log->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (log->window), "Log Window");
    gtk_window_set_default_size (GTK_WINDOW (log->window), 400, 300);

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);
    gtk_container_add (GTK_CONTAINER (log->window), vbox);

    scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (scrolledwindow);
    gtk_box_pack_start (GTK_BOX (vbox), scrolledwindow, TRUE, TRUE, 0);
    GTK_WIDGET_UNSET_FLAGS (scrolledwindow, GTK_CAN_FOCUS);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow),
                                         GTK_SHADOW_ETCHED_IN);

    log->textview = GTK_TEXT_VIEW (gtk_text_view_new ());
    gtk_widget_show (GTK_WIDGET (log->textview));
    gtk_container_add (GTK_CONTAINER (scrolledwindow), GTK_WIDGET (log->textview));
    GTK_WIDGET_UNSET_FLAGS (GTK_WIDGET (log->textview), GTK_CAN_FOCUS);
    gtk_text_view_set_editable (log->textview, FALSE);
    gtk_text_view_set_wrap_mode (log->textview, GTK_WRAP_WORD);

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

    font = pango_font_description_from_string ("Courier New 9");
    if (font) {
        gtk_widget_modify_font (GTK_WIDGET (log->textview), font);
        pango_font_description_free (font);
    }

    return log;
}


void             moo_log_window_write       (const gchar    *log_domain,
                                             GLogLevelFlags  flags,
                                             const gchar    *message)
{
    char *text;
    GtkTextTag *tag = NULL;
    GtkTextIter end;
    MooLogWindow *log = moo_log_window ();

#ifdef __WIN32__
    if (flags & (G_LOG_LEVEL_ERROR | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION)) {
        moo_show_fatal_error (log_domain, message);
        return;
    }
#endif /* __WIN32__ */

    if (log_domain)
        text = g_strdup_printf ("%s: %s\n", log_domain, message);
    else
        text = g_strdup_printf ("%s\n", message);

    if (flags >= G_LOG_LEVEL_MESSAGE)
        tag = log->message_tag;
    else if (flags >= G_LOG_LEVEL_WARNING)
        tag = log->warning_tag;
    else
        tag = log->critical_tag;

    gtk_text_buffer_get_end_iter (log->buf, &end);
    gtk_text_buffer_insert_with_tags (log->buf, &end, text, -1, tag, NULL);
    gtk_text_view_scroll_mark_onscreen (log->textview, log->insert);
}


MooLogWindow    *moo_log_window             (void)
{
    static MooLogWindow *log = NULL;
    if (!log) log = moo_log_window_new ();
    return log;
}


GtkWidget       *moo_log_window_get_widget  (void)
{
    MooLogWindow *log = moo_log_window ();
    return log->window;
}


void             moo_log_window_show        (void)
{
    gtk_window_present (GTK_WINDOW (moo_log_window()->window));
}

void             moo_log_window_hide        (void)
{
    gtk_widget_hide (moo_log_window()->window);
}


/****************************************************************************/

#define PLEASE_REPORT \
    "Please report it to emuntyan@sourceforge.net and provide "\
    "steps needed to reproduce this error."

#ifdef __WIN32__
void moo_show_fatal_error (const char *logdomain, const char *logmsg)
{
    char *msg = NULL;
    if (logdomain)
        msg = g_strdup_printf ("Fatal GGAP error:\n---\n%s: %s\n---\n"
                PLEASE_REPORT, logdomain, logmsg);
    else
        msg = g_strdup_printf ("Fatal GGAP error:\n---\n%s\n---\n"
                PLEASE_REPORT, logmsg);
    MessageBox (NULL, msg, "Error", MB_ICONERROR | MB_APPLMODAL | MB_SETFOREGROUND);
    g_free (msg);
}
#endif /* __WIN32__ */


static void log_func_file (const gchar      *log_domain,
                           G_GNUC_UNUSED GLogLevelFlags    flags,
                           const gchar      *message,
                           gpointer          filename)
{
    static gboolean firsttime = TRUE;
    static char *name = NULL;
    FILE *logfile;

    g_return_if_fail (filename != NULL);

#ifdef __WIN32__
    if (flags & (G_LOG_LEVEL_ERROR | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION)) {
        moo_show_fatal_error (log_domain, message);
        return;
    }
#endif /* __WIN32__ */

    if (firsttime) {
        name = g_strdup ((char*)filename);
        logfile = fopen (name, "w+");
        firsttime = FALSE;
    }
    else
        logfile = fopen (name, "a+");

    if (logfile) {
        if (log_domain)
            fprintf (logfile, "%s: %s\r\n", log_domain, message);
        else
            fprintf (logfile, "%s\r\n", message);
        fclose (logfile);
    }
    else {
        /* TODO ??? */
    }
}


static void log_func_silent (G_GNUC_UNUSED const gchar    *log_domain,
                             G_GNUC_UNUSED GLogLevelFlags  flags,
                             G_GNUC_UNUSED const gchar    *message,
                             G_GNUC_UNUSED gpointer        data_unused)
{
#ifdef __WIN32__
    if (flags & (G_LOG_LEVEL_ERROR | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION)) {
    moo_show_fatal_error (log_domain, message);
    return;
    }
#endif /* __WIN32__ */
}


typedef void (*LogFunc) (const gchar    *log_domain,
                         GLogLevelFlags  flags,
                         const gchar    *message,
                         gpointer        data);


static void set_handler (LogFunc func, gpointer data)
{
#if !GLIB_CHECK_VERSION(2,6,0)
    g_log_set_handler ("Glib",
                       G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,
                       func, data);
    g_log_set_handler ("Gtk",
                       G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,
                       func, data);
    g_log_set_handler ("Pango",
                       G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,
                       func, data);
    g_log_set_handler (NULL,
                       G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,
                       func, data);
#else /* GLIB_CHECK_VERSION(2,6,0) */
    g_log_set_default_handler (func, data);
#endif /* GLIB_CHECK_VERSION(2,6,0) */
}


void moo_set_log_func_window (int show)
{
    if (show) moo_log_window_show ();
    set_handler ((LogFunc)moo_log_window_write, NULL);
}

void moo_set_log_func_file (const char *log_file)
{
    g_return_if_fail (log_file != NULL);
    set_handler (log_func_file, (char*)log_file);
}

void moo_set_log_func (int show_log)
{
    if (!show_log)
        set_handler (log_func_silent, NULL);
    else {
#ifdef __WIN32__
        moo_set_log_func_window (FALSE);
#endif /* __WIN32__ */
    }
}
