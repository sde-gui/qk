/*
 *   mooutils-thread.c
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

#include "config.h"
#include "mooutils/mooutils-thread.h"

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
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
static EventQueue *queue;


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

    g_message ("processing events for id %u", GPOINTER_TO_UINT (id));
    client = get_event_client (GPOINTER_TO_UINT (id));

    if (client)
        client->callback (events->head, client->callback_data);

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
    int fds[2];

    if (queue)
        return;

    if (pipe (fds) != 0)
    {
        perror ("pipe");
        return;
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

    g_io_add_watch (queue->io, G_IO_IN, (GIOFunc) got_data, NULL);
    queue->data = NULL;
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
    client->id = queue->last_id++;
    client->callback = callback;
    client->callback_data = data;
    client->notify = notify;

    queue->clients = g_slist_prepend (queue->clients, client);

    return client->id;
}


void
_moo_event_queue_disconnect (guint event_id)
{
    QueueClient *client;

    g_return_if_fail (event_id != 0);
    g_return_if_fail (queue != NULL);

    client = get_event_client (event_id);
    g_return_if_fail (client != NULL);

    queue->clients = g_slist_remove (queue->clients, client);

    if (client->notify)
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
