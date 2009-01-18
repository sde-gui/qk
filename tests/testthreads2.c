/*
 *   testthreads.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mooutils/mooutils-thread.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooutils-debug.h"
#include <stdio.h>
#include <stdarg.h>

G_GNUC_UNUSED static void
hole (G_GNUC_UNUSED const char *format, ...)
{
}

#define printf hole

static int thread_count;

// static void
// print (const char *format, ...)
// {
//     va_list args;
//     char *string;
//
//     gdk_threads_enter ();
//
//     va_start (args, format);
//     string = g_strdup_vprintf (format, args);
//     va_end (args);
//
//     {
//         GtkTextIter iter;
//         GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
//         gtk_text_buffer_get_end_iter (buffer, &iter);
//         gtk_text_buffer_place_cursor (buffer, &iter);
//         gtk_text_buffer_insert_at_cursor (buffer, string, -1);
//         gtk_text_buffer_insert_at_cursor (buffer, "\n", -1);
//         gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (textview),
//                                             gtk_text_buffer_get_insert (buffer));
//     }
//
//     g_free (string);
//
//     gdk_threads_leave ();
// }

typedef struct {
    guint id;
    guint event_id;
} ThreadData;

static void
callback (GList      *messages,
          ThreadData *tdata)
{
    g_print ("thread %d: sent %d messages\n", tdata->id, g_list_length (messages));
    while (messages)
    {
        g_print ("    %s\n", (char*) messages->data);
        messages = messages->next;
    }
}

static void
queue_notify (ThreadData *tdata)
{
    int threads_left;

    gdk_threads_enter ();
    threads_left = --thread_count;
    gdk_threads_leave ();

    g_print ("thread %d is dead, %d left\n", tdata->id, threads_left);
    printf ("thread %d is dead, %d left\n", tdata->id, threads_left);

    if (!threads_left)
    {
        gdk_threads_enter ();
        gtk_main_quit ();
        gdk_threads_leave ();
    }
}

static gpointer
thread_main (ThreadData *tdata)
{
    guint i;

    g_usleep (1000);

    g_print ("thread %d: starting\n", tdata->id);
    printf ("thread %d: starting\n", tdata->id);

    for (i = 0; i < 10; ++i)
    {
        printf ("thread %d: hi there #%d\n", tdata->id, i);
        _moo_event_queue_push (tdata->event_id,
                               g_strdup_printf ("thread %d: hi there #%d", tdata->id, i),
                               g_free);
        g_usleep (g_random_int_range (10, 100) * 10000);
        printf ("thread %d: hi there #%d again\n", tdata->id, i);
        g_print ("thread %d: hi there #%d again\n", tdata->id, i);
        g_usleep (g_random_int_range (10, 100) * 1000);
    }

    g_print ("thread %d: exiting\n", tdata->id);
    printf ("thread %d: exiting\n", tdata->id);

    _moo_event_queue_disconnect (tdata->event_id);

    return NULL;
}

#define NTHREADS 100

int main (int argc, char *argv[])
{
    guint i;
    GError *error = NULL;
    GThread *threads[NTHREADS];

    g_thread_init (NULL);
    gdk_threads_init ();

    gtk_init (&argc, &argv);

    moo_set_log_func_file ("testthreads2-log.txt");

    thread_count = NTHREADS;
    for (i = 0; i < NTHREADS; ++i)
    {
        ThreadData *data = moo_new0 (ThreadData);
        data->id = i;
        data->event_id =
            _moo_event_queue_connect ((MooEventQueueCallback) callback, data,
                                      (GDestroyNotify) queue_notify);
        if (!(threads[i] = g_thread_create ((GThreadFunc) thread_main, data, FALSE, NULL)))
            g_error ("%s", error->message);
    }

    gdk_threads_enter ();
    gtk_main ();
    gdk_threads_leave ();

    return 0;
}
