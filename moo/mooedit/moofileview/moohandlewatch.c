/*
 *   moohandlewatch.c
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

#include "moohandlewatch.h"


static gboolean SetEvent (G_GNUC_UNUSED HANDLE handle)
{
    return FALSE;
}
static HANDLE CreateEvent (G_GNUC_UNUSED gpointer sec_attrs,
                           G_GNUC_UNUSED gboolean manual_reset,
                           G_GNUC_UNUSED gboolean initial_state,
                           G_GNUC_UNUSED const char *name)
{
    return NULL;
}
static int GetLastError (void)
{
    return 0;
}
static char *g_win32_error_message (G_GNUC_UNUSED int err)
{
    return NULL;
}
static void CloseHandle (G_GNUC_UNUSED HANDLE handle)
{
}


typedef enum {
    REQUEST_ADD,
    REQUEST_REMOVE,
    REQUEST_CONTINUE,
    REQUEST_DIE
} RequestCode;

typedef struct {
    RequestCode code;
    HANDLE handle;
    HANDLE event;
    GAsyncQueue *answer;
} Request;


typedef struct {
    guint           handle_id;
    HANDLE          handle;
    MooHandleFunc   func;
    gpointer        user_data;
} HandleInfo;

typedef struct {
    GSource source;
    GPollFD thread_event;
    Request *request;
    GAsyncQueue *events;
    GSList *handles; /* HandleInfo* */
} HandleWatch;


static gboolean watch_thread_start      (HANDLE          event,
                                         Request        *request,
                                         GAsyncQueue    *events);

static void     watch_resume_thread     (HandleWatch    *watch);
static void     watch_kill_thread       (HandleWatch    *watch);
static gboolean watch_do_event          (HandleWatch    *watch,
                                         gpointer        event);

static gboolean handle_watch_prepare    (G_GNUC_UNUSED GSource *source,
                                         gint       *timeout_)
{
    *timeout_ = -1;
    return FALSE;
}


static gboolean handle_watch_check      (GSource    *source)
{
    HandleWatch *watch = (HandleWatch*) source;
    return watch->thread_event.revents != 0;
}


static gboolean handle_watch_dispatch   (G_GNUC_UNUSED GSource *source,
                                         GSourceFunc callback,
                                         gpointer    user_data)
{
    return callback (user_data);
}


static void     handle_watch_finalize   (G_GNUC_UNUSED GSource *source)
{
    g_critical ("Oh no, I'm finalized!");
}


static gboolean handle_watch_callback   (HandleWatch *watch)
{
    if (watch->thread_event.revents & (G_IO_HUP | G_IO_ERR))
    {
        g_critical ("Oh no, error!");
        return FALSE;
    }

    while (g_async_queue_length (watch->events))
    {
        gpointer event = g_async_queue_pop (watch->events);
        if (!watch_do_event (watch, event))
            watch_kill_thread (watch);
    }

    watch_resume_thread (watch);
    return TRUE;
}


static HandleWatch  *handle_watch_get   (gboolean  create)
{
    static HandleWatch *instance = NULL;
    GSource *source;

    static GSourceFuncs handle_watch_funcs = {
        handle_watch_prepare,
        handle_watch_check,
        handle_watch_dispatch,
        handle_watch_finalize,
        NULL, NULL
    };

    if (!create || instance != NULL)
        return instance;

    source = g_source_new (&handle_watch_funcs,
                            sizeof (HandleWatch));
    g_return_val_if_fail (source != NULL, NULL);

    g_source_set_callback (source,
                           (GSourceFunc) handle_watch_callback,
                           source, NULL);

    instance = (HandleWatch*)source;
    instance->handles = NULL;
    instance->events = g_async_queue_new ();

    instance->thread_event.fd = (int) CreateEvent (NULL, FALSE, FALSE, NULL);
    instance->thread_event.events = TRUE;

    if (!instance->thread_event.fd)
    {
        int err = GetLastError ();
        char *msg = g_win32_error_message (err);
        g_critical ("%s: CreateEvent failed: %s", G_STRLOC, msg);
        g_free (msg);
        goto error;
    }

    instance->request = g_new0 (Request, 1);
    instance->request->answer = g_async_queue_new ();
    instance->request->event = CreateEvent (NULL, FALSE, FALSE, NULL);

    if (!instance->request->event)
    {
        int err = GetLastError ();
        char *msg = g_win32_error_message (err);
        g_critical ("%s: CreateEvent failed: %s", G_STRLOC, msg);
        g_free (msg);
        goto error;
    }

    if (!watch_thread_start ((HANDLE) instance->thread_event.fd,
                             instance->request, instance->events))
    {
        g_critical ("%s: could not start thread", G_STRLOC);
        goto error;
    }

    g_source_add_poll (source, &instance->thread_event);
    g_source_attach (source, NULL);
    return instance;

error:
    if (instance->thread_event.fd)
        CloseHandle ((HANDLE) instance->thread_event.fd);
    instance->thread_event.fd = 0;

    if (instance->request)
    {
        if (instance->request->answer)
            g_async_queue_unref (instance->request->answer);
        if (instance->request->event)
            CloseHandle (instance->request->event);
        g_free (instance->request);
        instance->request = NULL;
    }

    if (instance->events)
        g_async_queue_unref (instance->events);
    instance->events = NULL;

    instance = NULL;
    g_source_destroy (source);
    return NULL;
}


guint       moo_handle_watch_add            (HANDLE         handle,
                                             MooHandleFunc  func,
                                             gpointer       user_data)
{
    Request *request;
    gpointer result;
    HandleWatch *watch;

    watch = handle_watch_get (TRUE);
    g_return_val_if_fail (watch != NULL, 0);

    request = watch->request;

    request->code = REQUEST_ADD;
    request->handle = handle;

    if (!SetEvent (request->event))
    {
        int err = GetLastError ();
        char *msg = g_win32_error_message (err);
        g_critical ("%s: SetEvent failed: %s", G_STRLOC, msg);
        g_free (msg);
        return 0;
    }

    result = g_async_queue_pop (request->answer);

    if (result)
    {
        HandleInfo *info = g_new (HandleInfo, 1);

        info->handle_id = GPOINTER_TO_UINT (result);
        info->handle = handle;
        info->func = func;
        info->user_data = user_data;

        watch->handles = g_slist_prepend (watch->handles, info);

        g_message ("%s: added handle, id %d",
                   G_STRLOC, info->handle_id);

        return info->handle_id;
    }
    else
    {
        g_warning ("%s: adding handle failed", G_STRLOC);
        return 0;
    }
}


static int  cmp_handle (HandleInfo *info,
                        HANDLE      handle)
{
    return info->handle != handle;
}

guint       moo_handle_watch_remove_handle  (HANDLE handle)
{
    Request *request;
    gpointer result;
    HandleWatch *watch;
    GSList *link;
    HandleInfo *info;

    watch = handle_watch_get (FALSE);
    g_return_val_if_fail (watch != NULL, 0);

    link = g_slist_find_custom (watch->handles, handle,
                                (GCompareFunc) cmp_handle);
    g_return_val_if_fail (link != NULL, 0);

    info = link->data;
    request = watch->request;

    request->code = REQUEST_REMOVE;
    request->handle = handle;

    if (!SetEvent (request->event))
    {
        int err = GetLastError ();
        char *msg = g_win32_error_message (err);
        g_critical ("%s: SetEvent failed: %s", G_STRLOC, msg);
        g_free (msg);
        result = 0;
    }
    else
    {
        result = g_async_queue_pop (request->answer);

        if (result)
        {
            if (info->handle_id != GPOINTER_TO_UINT (result) ||
                info->handle != handle)
            {
                g_critical ("%s: oops", G_STRLOC);
            }
            else
            {
                g_message ("%s: removed handle id %d",
                           G_STRLOC, info->handle_id);
            }
        }
        else
        {
            g_warning ("%s: removing handle id %d failed",
                       G_STRLOC, info->handle_id);
        }
    }

    g_free (link->data);
    watch->handles = g_slist_delete_link (watch->handles, link);

    return GPOINTER_TO_UINT (result);
}


static int  cmp_handle_id (HandleInfo *info,
                           gpointer    id)
{
    return info->handle_id != GPOINTER_TO_UINT (id);
}

guint       moo_handle_watch_remove         (guint  handle_id)
{
    HandleWatch *watch;
    GSList *link;
    HandleInfo *info;

    watch = handle_watch_get (FALSE);
    g_return_val_if_fail (watch != NULL, 0);

    link = g_slist_find_custom (watch->handles,
                                GUINT_TO_POINTER (handle_id),
                                (GCompareFunc) cmp_handle_id);
    g_return_val_if_fail (link != NULL, 0);

    info = link->data;

    return moo_handle_watch_remove_handle (info->handle);
}
