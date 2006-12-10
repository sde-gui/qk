/*
 *   testthreads.c
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

#include "mooutils/mooutils-thread.h"
#include "mooutils/mooutils-misc.h"
#include <stdio.h>


static void
callback (void)
{
}

static gpointer
thread_main (gpointer data)
{
    guint i;

    /* g_print may not be used here */
    printf ("thread %d: starting\n", GPOINTER_TO_UINT (data));

    for (i = 0; i < 10; ++i)
    {
        _moo_message_async ("thread %d: hi there #%d", GPOINTER_TO_UINT (data), i);
        g_usleep (g_random_int_range (10, 100) * 1000);
        gdk_threads_enter ();
        _moo_message ("thread %d: hi there #%d again", GPOINTER_TO_UINT (data), i);
        gdk_threads_leave ();
        g_usleep (g_random_int_range (10, 100) * 1000);
    }

    printf ("thread %d: exiting\n", GPOINTER_TO_UINT (data));
    return NULL;
}

int main (int argc, char *argv[])
{
    guint i;
    GError *error = NULL;

    g_thread_init (NULL);
    gdk_threads_init ();
    gdk_threads_enter ();

    gtk_init (&argc, &argv);

    moo_set_log_func_window (TRUE);

    _moo_message ("Hi there");

    _moo_event_queue_connect ((MooEventQueueCallback) callback, NULL, NULL);

    for (i = 0; i < 10; ++i)
        if (!g_thread_create (thread_main, GUINT_TO_POINTER (i), FALSE, NULL))
            g_error ("%s", error->message);

    gtk_main ();
    gdk_threads_leave ();

    return 0;
}
