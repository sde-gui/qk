/*
 *   tests/termbuffer.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#include <gtk/gtk.h>
#define MOOTERM_COMPILATION
#include "mooterm/mooterm-private.h"
#include "mooterm/mootermbuffer-private.h"
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


G_GNUC_UNUSED static gboolean print (MooTerm *term)
{
    guint i;

    for (i = 0; i < 1000; ++i)
        moo_term_feed (term, "kjhr jerhgjh erkjg hekrjghkerg ", -1);

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
        moo_term_feed (term, s, -1);
        g_free (s);

        gdk_window_process_updates (GTK_WIDGET(term)->window, FALSE);
    }

    g_print ("buffer: %dx%d\nterm: %dx%d\n",
             buf_total_height (buf), buf_screen_width (buf),
             term->priv->height, term->priv->width);

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
    moo_term_feed (term, s, -1);
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

    buf = MOO_TERM(term)->priv->buffer;

    g_idle_add ((GSourceFunc) print_random_hard, term);

    g_signal_connect (win, "destroy", gtk_main_quit, NULL);
    gtk_main ();
    return 0;
}
