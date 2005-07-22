/*
 *   tests/markup.c
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

#include <gtk/gtk.h>
#define MOOTERM_COMPILATION
#include "mooterm/mooterm-private.h"
#include "mooterm/mootermbuffer-private.h"
#include <string.h>
#include <stdlib.h>


static void dump_trace_log_handler (const gchar   *log_domain,
                                    GLogLevelFlags log_level,
                                    const gchar   *message,
                                    gpointer       user_data)
{
    g_log_default_handler (log_domain, log_level, message, user_data);

    if (log_level & (G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL))
    {
        g_on_error_stack_trace (NULL);
    }
}

static void breakpoint_log_handler (const gchar   *log_domain,
                                    GLogLevelFlags log_level,
                                    const gchar   *message,
                                    gpointer       user_data)
{
    g_log_default_handler (log_domain, log_level, message, user_data);

    if (log_level &
        (G_LOG_LEVEL_ERROR | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL))
    {
        G_BREAKPOINT ();
    }
}


static void init (int *argc, char ***argv, const char **cmd)
{
    int i;
    gboolean gdk_debug = FALSE;
    gboolean dump_trace = FALSE;
    gboolean set_breakpoint = FALSE;

    gtk_init (argc, argv);

    for (i = 1; i < *argc; ++i)
    {
        if (!strcmp ((*argv)[i], "--gdk-debug"))
            gdk_debug = TRUE;
        else if (!strcmp ((*argv)[i], "--dump-trace"))
            dump_trace = TRUE;
        else if (!strcmp ((*argv)[i], "--set-breakpoint"))
            set_breakpoint = TRUE;
        else
            *cmd = (*argv)[i];
    }

    if (gdk_debug)
    {
        gdk_window_set_debug_updates (TRUE);
    }

    if (dump_trace)
        g_log_set_default_handler (dump_trace_log_handler, NULL);
    else if (set_breakpoint)
        g_log_set_default_handler (breakpoint_log_handler, NULL);
}


static void set_width (MooTerm *term, guint width, GtkWindow *window)
{
    guint height;
    height = term_char_height (term) * 25;
    width *= term_char_width (term);
    gtk_widget_set_size_request (GTK_WIDGET (term), width, height);
    gtk_window_resize (window, 10, 10);
    gtk_container_check_resize (GTK_CONTAINER (window));
    gdk_window_process_updates (GTK_WIDGET(window)->window, TRUE);
}


int main (int argc, char *argv[])
{
    const char *cmd = NULL;
    GtkWidget *win, *swin, *term;

    init (&argc, &argv, &cmd);

    if (!cmd)
    {
        cmd = g_getenv ("SHELL");
        if (!cmd)
            cmd = "sh";
    }

    win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size (GTK_WINDOW (win), 700, 500);
    swin = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_add (GTK_CONTAINER (win), swin);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
                                    GTK_POLICY_NEVER,
//                                     GTK_POLICY_NEVER);
//                                     GTK_POLICY_AUTOMATIC);
                                    GTK_POLICY_ALWAYS);

    term = GTK_WIDGET (g_object_new (MOO_TYPE_TERM,
                                     "cursor-blinks", FALSE,
                                     "font-name", "Courier New 11",
                                     NULL));

    gtk_container_add (GTK_CONTAINER (swin), term);

    gtk_widget_show_all (win);
    gtk_container_set_resize_mode (GTK_CONTAINER (win),
                                   GTK_RESIZE_IMMEDIATE);
    gtk_container_set_resize_mode (GTK_CONTAINER (swin),
                                   GTK_RESIZE_IMMEDIATE);

    g_signal_connect (term, "set-width",
                      G_CALLBACK (set_width), win);
    g_signal_connect_swapped (term, "set-window-title",
                              G_CALLBACK (gtk_window_set_title), win);
    g_signal_connect_swapped (term, "set-icon-name",
                              G_CALLBACK (gdk_window_set_icon_name), win->window);

    moo_term_fork_command (MOO_TERM (term), cmd, NULL, NULL);

    g_signal_connect (win, "destroy", gtk_main_quit, NULL);
    g_signal_connect_swapped (term, "child-died",
                              G_CALLBACK (gtk_widget_destroy), win);
    g_signal_connect (term, "bell", G_CALLBACK (gdk_beep), NULL);

    gtk_main ();
}
