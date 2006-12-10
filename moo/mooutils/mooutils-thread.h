/*
 *   mooutils-thread.h
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

#ifndef __MOO_UTILS_WIN32_H__
#define __MOO_UTILS_WIN32_H__

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
                                         ...);
void        _moo_message_async          (const char            *format,
                                         ...);


G_END_DECLS

#endif /* __MOO_UTILS_WIN32_H__ */
