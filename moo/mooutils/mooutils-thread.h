/*
 *   mooutils-thread.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_UTILS_WIN32_H
#define MOO_UTILS_WIN32_H

#include <glib.h>

G_BEGIN_DECLS


typedef void (*MooEventQueueCallback)   (GList   *events,
                                         gpointer data);


guint       _moo_event_queue_connect    (MooEventQueueCallback  callback,
                                         gpointer               data,
                                         GDestroyNotify         notify);
void        _moo_event_queue_disconnect (guint                  event_id);

void        _moo_event_queue_do_events  (guint                  event_id);

/* called from a thread */
void        _moo_event_queue_push       (guint                  event_id,
                                         gpointer               data,
                                         GDestroyNotify         data_destroy);

void        _moo_print_async            (const char            *format,
                                         ...) G_GNUC_PRINTF(1,2);
void        _moo_message_async          (const char            *format,
                                         ...) G_GNUC_PRINTF(1,2);


G_END_DECLS

#endif /* MOO_UTILS_WIN32_H */
