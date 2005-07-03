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
#include "mooterm/mooterm.h"
#include <string.h>
#include <stdlib.h>


int main (int argc, char *argv[])
{
    const char *cmd = NULL;
    GtkWidget *win, *swin, *term;
    MooTermBuffer *buf;

    gtk_init (&argc, &argv);

    if (argc > 1)
    {
        cmd = argv[1];
    }
    else
    {
        cmd = g_getenv ("SHELL");
        if (!cmd)
            cmd = "sh";
    }

    win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size (GTK_WINDOW (win), 400, 400);
    swin = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_add (GTK_CONTAINER (win), swin);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
                                    GTK_POLICY_NEVER,
                                    GTK_POLICY_AUTOMATIC);

    term = GTK_WIDGET (g_object_new (MOO_TYPE_TERM, NULL));
    gtk_container_add (GTK_CONTAINER (swin), term);

    gtk_widget_show_all (win);

    buf = moo_term_get_buffer (MOO_TERM (term));
    g_object_set (buf,
                  "am-mode", TRUE,
                  "insert-mode", FALSE,
                  "cursor-visible", TRUE,
                  NULL);

    g_signal_connect_swapped (buf, "set-window-title",
                              G_CALLBACK (gtk_window_set_title), win);

    moo_term_fork_command (MOO_TERM (term), cmd, NULL, NULL);

    g_signal_connect (G_OBJECT (win), "destroy", gtk_main_quit, NULL);
    gtk_main ();
}
