//
//   tests/mterm.cpp
//
//   Copyright (C) 2004-2005 by Yevgen Muntyan <muntyan@math.tamu.edu>
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   See COPYING file that comes with this distribution.
//

#include <gtk/gtk.h>
#define MOOTERM_COMPILATION
#include "mooterm/mooterm-private.h"
#include <string.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
    gtk_init (&argc, &argv);

    char *cmd = NULL;

    if (argc > 1)
    {
        cmd = g_strdup (argv[1]);
    }
    else
    {
        const char *dir = g_getenv ("HOME");
        if (!dir)
            dir = "/";
        cmd = g_strdup_printf ("ls -R %s", dir);
    }

    GtkWidget *win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size (GTK_WINDOW (win), 400, 400);
    GtkWidget *swin = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_add (GTK_CONTAINER (win), swin);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
                                    GTK_POLICY_NEVER,
                                    GTK_POLICY_AUTOMATIC);

    GtkWidget *term = GTK_WIDGET (g_object_new (MOO_TYPE_TERM, NULL));
    gtk_container_add (GTK_CONTAINER (swin), term);

    gtk_widget_show_all (win);

    MooTermBuffer *buf = moo_term_get_buffer (MOO_TERM (term));
    g_object_set (buf,
                  "am-mode", TRUE,
                  "insert-mode", TRUE,
                  "cursor-visible", TRUE,
                  NULL);

    moo_term_fork_command (MOO_TERM (term), cmd, NULL, NULL);
    g_free (cmd);

    g_signal_connect (G_OBJECT (win), "destroy", gtk_main_quit, NULL);
    gtk_main ();
}
