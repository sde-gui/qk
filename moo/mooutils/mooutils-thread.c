/*
 *   mooutils-thread.c
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

#include "config.h"
#include "mooutils/mooutils-thread.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooutils-debug.h"

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef __WIN32__
#include <windows.h>
#endif

#ifdef __WIN32__
#ifndef pipe
#define pipe(phandles)	_pipe (phandles, 4096, _O_BINARY)
#endif
#endif


typedef struct {
    MooEventQueueCallback callback;
    gpointer callback_data;
    GDestroyNotify notify;
    guint id;
} QueueClient;

typedef struct {
    gpointer data;
    GDestroyNotify destroy;
    guint id;
} EventData;

typedef struct {
    GSList *clients;
    guint last_id;

    int pipe_in;
    int pipe_out;
    GIOChannel *io;

    GHashTable *data;
} EventQueue;


static GStaticMutex queue_lock = G_STATIC_MUTEX_INIT;
static volatile EventQueue *queue;


static QueueClient *
get_event_client (guint id)
{
    GSList *l;

    g_return_val_if_fail (queue != NULL, NULL);

    for (l = queue->clients; l != NULL; l = l->next)
    {
        QueueClient *s = l->data;

        if (s->id == id)
            return s;
    }

    return NULL;
}


static void
invoke_callback (gpointer  id,
                 GQueue   *events)
{
    GList *l;
    QueueClient *client;

    _moo_message ("processing events for id %u", GPOINTER_TO_UINT (id));
    client = get_event_client (GPOINTER_TO_UINT (id));

    if (client)
    {
        GList *data_list = NULL;

        for (l = events->head; l != NULL; l = l->next)
        {
            EventData *data = l->data;
            data_list = g_list_prepend (data_list, data->data);
        }

        data_list = g_list_reverse (data_list);

        client->callback (data_list, client->callback_data);

        g_list_free (data_list);
    }

    for (l = events->head; l != NULL; l = l->next)
    {
        EventData *data = l->data;

        if (data->destroy)
            data->destroy (data->data);

        g_free (data);
    }

    g_queue_free (events);
}


static gboolean
got_data (GIOChannel *io)
{
    GHashTable *data;
    char buf[1];

    g_static_mutex_lock (&queue_lock);
    data = queue->data;
    queue->data = NULL;
    g_io_channel_read_chars (io, buf, 1, NULL, NULL);
    g_static_mutex_unlock (&queue_lock);

    g_hash_table_foreach (data, (GHFunc) invoke_callback, NULL);
    g_hash_table_destroy (data);

    return TRUE;
}


void
_moo_event_queue_do_events (guint event_id)
{
    GQueue *events = NULL;

    g_return_if_fail (queue != NULL);

    g_static_mutex_lock (&queue_lock);

    if (queue->data)
    {
        events = g_hash_table_lookup (queue->data, GUINT_TO_POINTER (event_id));

        if (events)
            g_hash_table_remove (queue->data, GUINT_TO_POINTER (event_id));
    }

    g_static_mutex_unlock (&queue_lock);

    if (events)
        invoke_callback (GUINT_TO_POINTER (event_id), events);
}


static void
init_queue (void)
{
    g_static_mutex_lock (&queue_lock);

    if (!queue)
    {
        int fds[2];
        GSource *source;

        if (pipe (fds) != 0)
        {
            perror ("pipe");
            goto out;
        }

        queue = g_new0 (EventQueue, 1);

        queue->clients = NULL;

        queue->pipe_in = fds[1];
        queue->pipe_out = fds[0];

#ifdef __WIN32__
        queue->io = g_io_channel_win32_new_fd (queue->pipe_out);
#else
        queue->io = g_io_channel_unix_new (queue->pipe_out);
#endif

        source = g_io_create_watch (queue->io, G_IO_IN);
        g_source_set_callback (source, (GSourceFunc) got_data, NULL, NULL);
        g_source_set_can_recurse (source, TRUE);
        g_source_attach (source, NULL);

        queue->data = NULL;
    }

out:
    g_static_mutex_unlock (&queue_lock);
}


guint
_moo_event_queue_connect (MooEventQueueCallback callback,
                          gpointer              data,
                          GDestroyNotify        notify)
{
    QueueClient *client;

    g_return_val_if_fail (callback != NULL, 0);

    init_queue ();

    client = g_new0 (QueueClient, 1);
    client->callback = callback;
    client->callback_data = data;
    client->notify = notify;

    g_static_mutex_lock (&queue_lock);
    client->id = queue->last_id++;
    queue->clients = g_slist_prepend (queue->clients, client);
    g_static_mutex_unlock (&queue_lock);

    return client->id;
}


void
_moo_event_queue_disconnect (guint event_id)
{
    QueueClient *client;

    g_return_if_fail (event_id != 0);
    g_return_if_fail (queue != NULL);

    g_static_mutex_lock (&queue_lock);

    client = get_event_client (event_id);

    if (!client)
        g_warning ("%s: no client with id %d", G_STRLOC, event_id);
    else
        queue->clients = g_slist_remove (queue->clients, client);

    g_static_mutex_unlock (&queue_lock);

    if (client && client->notify)
        client->notify (client->callback_data);

    g_free (client);
}


/* called from a thread */
void
_moo_event_queue_push (guint          event_id,
                       gpointer       data,
                       GDestroyNotify data_destroy)
{
    char c = 'd';
    EventData *event_data;
    GQueue *events;

    event_data = g_new (EventData, 1);
    event_data->data = data;
    event_data->destroy = data_destroy;
    event_data->id = event_id;

    g_static_mutex_lock (&queue_lock);

    if (!queue->data)
    {
        write (queue->pipe_in, &c, 1);
        queue->data = g_hash_table_new (g_direct_hash, g_direct_equal);
    }

    events = g_hash_table_lookup (queue->data, GUINT_TO_POINTER (event_id));

    if (!events)
    {
        events = g_queue_new ();
        g_hash_table_insert (queue->data, GUINT_TO_POINTER (event_id), events);
    }

    g_queue_push_tail (events, event_data);

    g_static_mutex_unlock (&queue_lock);
}


/****************************************************************************/
/* Messages
 */

static GStaticMutex message_lock = G_STATIC_MUTEX_INIT;
static volatile int message_event_id = 0;
static volatile int print_event_id = 0;

static void
message_callback (GList *events, G_GNUC_UNUSED gpointer data)
{
    while (events)
    {
        gdk_threads_enter ();
        _moo_message ("%s", (char*) events->data);
        gdk_threads_leave ();
        events = events->next;
    }
}

static void
print_callback (GList *events, G_GNUC_UNUSED gpointer data)
{
    while (events)
    {
        gdk_threads_enter ();
        g_print ("%s", (char*) events->data);
        gdk_threads_leave ();
        events = events->next;
    }
}

static void
init_message_queue (void)
{
    g_static_mutex_lock (&message_lock);

    if (!message_event_id)
        message_event_id = _moo_event_queue_connect (message_callback, NULL, NULL);
    if (!print_event_id)
        print_event_id = _moo_event_queue_connect (print_callback, NULL, NULL);

    g_static_mutex_unlock (&message_lock);
}

void
_moo_print_async (const char *format,
                  ...)
{
    char *msg;
    va_list args;

    va_start (args, format);
    msg = g_strdup_vprintf (format, args);
    va_end (args);

    if (msg)
    {
        init_message_queue ();
        _moo_event_queue_push (print_event_id, msg, g_free);
    }
}

void
_moo_message_async (const char *format,
                    ...)
{
    char *msg;
    va_list args;

    va_start (args, format);
    msg = g_strdup_vprintf (format, args);
    va_end (args);

    if (msg)
    {
        init_message_queue ();
        _moo_event_queue_push (message_event_id, msg, g_free);
    }
}
