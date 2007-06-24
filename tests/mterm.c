/*
 *   tests/mterm.c
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

#include <gtk/gtk.h>
#include "mooterm/mooterm.h"
#include "mooutils/mooutils-misc.h"
#include <string.h>
#include <stdlib.h>


static void
init (int *argc, char ***argv, const char **cmd)
{
    int i;
    gboolean gdk_debug = FALSE;

    gtk_init (argc, argv);

    for (i = 1; i < *argc; ++i)
    {
        if (!strcmp ((*argv)[i], "--gdk-debug"))
            gdk_debug = TRUE;
        else
            *cmd = (*argv)[i];
    }

    if (gdk_debug)
        gdk_window_set_debug_updates (TRUE);

//     moo_set_log_func_file ("/tmp/mterm");
}


static void
set_width (MooTerm *term, guint width, GtkWindow *window)
{
    guint height;
    height = moo_term_char_height (term) * 25;
    width *= moo_term_char_width (term);
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

    moo_term_fork_command_line (MOO_TERM (term), cmd, NULL, NULL, NULL);

    g_signal_connect (win, "destroy", gtk_main_quit, NULL);
    g_signal_connect_swapped (term, "child-died",
                              G_CALLBACK (gtk_widget_destroy), win);
    g_signal_connect (term, "bell", G_CALLBACK (gdk_beep), NULL);

    gtk_main ();
    return 0;
}
