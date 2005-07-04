/*
 *   tests/termbuffer.c
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
#include <string.h>
#include <stdlib.h>

#ifndef __WIN32__
#define CMD "sh"
#else
#define CMD "gapw95.exe"
#endif

#define CLEAR       "\033[H\033[2J"
#define HOME        "\033[H"
#define PARM_RIGHT  "\033[%dC"
#define PARM_LEFT   "\033[%dD"
#define ADDRESS     "\033[%d;%dH"

#define GREEN       "\033[0;40;32m"
#define NORMAL      "\033[0m"


G_GNUC_UNUSED static gboolean print (MooTermBuffer *buf)
{
    guint i;

    for (i = 0; i < 1000; ++i)
        moo_term_buffer_write (buf, "kjhr jerhgjh erkjg hekrjghkerg ", -1);

    return FALSE;
}


G_GNUC_UNUSED static gboolean print_random_hard (MooTerm *term)
{
    MooTermBuffer *buf = term->priv->buffer;
    guint width = buf_screen_width (buf);
    guint height = buf_screen_height (buf);
    guint row, col;
    guint i;

    for (i = 0; i < 10000; ++i)
    {
        char r;
        char *s;

        r = 32 + (int) (94.0 * rand() / (RAND_MAX+1.0));
        row = 1 + (int) (((double)height) * rand() / (RAND_MAX+1.0));
        col = 1 + (int) (((double)width) * rand() / (RAND_MAX+1.0));

        s = g_strdup_printf (ADDRESS "%c", row, col, r);
        moo_term_buffer_write (buf, s, -1);
        g_free (s);

        moo_term_force_update (term);
    }

    g_print ("buffer: %ldx%ld\nterm: %ldx%ld\n",
             buf_total_height (buf), buf_screen_width (buf),
             term_height (term), term_width (term));

    gtk_main_quit ();
    return FALSE;
}


G_GNUC_UNUSED static gboolean print_random_soft (MooTerm *term)
{
    MooTermBuffer *buf = term->priv->buffer;
    guint width = buf_screen_width (buf);
    guint height = buf_screen_height (buf);
    guint row, col;
    char r;
    char *s;

    r = 32 + (int) (94.0 * rand() / (RAND_MAX+1.0));
    row = 1 + (int) (((double)height) * rand() / (RAND_MAX+1.0));
    col = 1 + (int) (((double)width) * rand() / (RAND_MAX+1.0));

    s = g_strdup_printf (ADDRESS "%c", row, col, r);
    moo_term_buffer_write (buf, s, -1);
    g_free (s);

    return TRUE;
}


int main (int argc, char *argv[])
{
    GtkWidget *win, *swin, *term;
    MooTermBuffer *buf;
    const char *cmd = CMD;

    gtk_init (&argc, &argv);

    if (argc > 1)
        cmd = argv[1];

    win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request (win, 400, 400);
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
                  "mode-DECAWM", FALSE,
                  "mode-IRM", FALSE,
                  "cursor-visible", TRUE,
                  NULL);

    g_idle_add ((GSourceFunc) print_random_hard, term);

    g_signal_connect (win, "destroy", gtk_main_quit, NULL);
    gtk_main ();
}
