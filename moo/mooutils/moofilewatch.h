/*
 *   moofilewatch.h
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

/* Files and directory monitor. Uses FAM if present, or stat() otherwise.
   On win32 does FindFirstChangeNotification and ReadDirectoryChangesW. */

#ifndef __MOO_FILE_WATCH_H__
#define __MOO_FILE_WATCH_H__

#include <glib-object.h>

G_BEGIN_DECLS


#define MOO_FILE_WATCH_ERROR (moo_file_watch_error_quark ())

typedef enum
{
    MOO_FILE_WATCH_ERROR_CLOSED,
    MOO_FILE_WATCH_ERROR_FAILED,
    MOO_FILE_WATCH_ERROR_NOT_IMPLEMENTED,
    MOO_FILE_WATCH_ERROR_TOO_MANY,
    MOO_FILE_WATCH_ERROR_NOT_DIR,
    MOO_FILE_WATCH_ERROR_IS_DIR,
    MOO_FILE_WATCH_ERROR_NONEXISTENT,
    MOO_FILE_WATCH_ERROR_BAD_FILENAME,
    MOO_FILE_WATCH_ERROR_ACCESS_DENIED
} MooFileWatchError;

GQuark  moo_file_watch_error_quark (void);


#define MOO_TYPE_FILE_WATCH         (moo_file_watch_get_type ())
#define MOO_TYPE_FILE_EVENT         (moo_file_event_get_type ())
#define MOO_TYPE_FILE_EVENT_CODE    (moo_file_event_code_get_type ())

typedef enum {
    MOO_FILE_EVENT_CHANGED,
    MOO_FILE_EVENT_DELETED,
    MOO_FILE_EVENT_ERROR
} MooFileEventCode;

struct _MooFileEvent {
    MooFileEventCode code;
    guint            monitor_id;
    char            *filename;
    GError          *error;
};

typedef struct _MooFileWatch          MooFileWatch;
typedef struct _MooFileEvent          MooFileEvent;

typedef void (*MooFileWatchCallback) (MooFileWatch *watch,
                                      MooFileEvent *event,
                                      gpointer      user_data);


GType           moo_file_watch_get_type             (void) G_GNUC_CONST;
GType           moo_file_event_get_type             (void) G_GNUC_CONST;
GType           moo_file_event_code_get_type        (void) G_GNUC_CONST;

/* FAMOpen */
MooFileWatch   *moo_file_watch_new                  (GError        **error);

MooFileWatch   *moo_file_watch_ref                  (MooFileWatch   *watch);
void            moo_file_watch_unref                (MooFileWatch   *watch);

/* FAMClose */
gboolean        moo_file_watch_close                (MooFileWatch   *watch,
                                                     GError        **error);

/* FAMMonitorDirectory, FAMMonitorFile */
guint           moo_file_watch_create_monitor       (MooFileWatch   *watch,
                                                     const char     *filename,
                                                     MooFileWatchCallback callback,
                                                     gpointer        data,
                                                     GDestroyNotify  notify,
                                                     GError        **error);
/* FAMCancelMonitor */
void            moo_file_watch_cancel_monitor       (MooFileWatch   *watch,
                                                     guint           monitor_id);
/* FAMSuspendMonitor, FAMResumeMonitor */
void            moo_file_watch_suspend_monitor      (MooFileWatch   *watch,
                                                     guint           monitor_id);
void            moo_file_watch_resume_monitor       (MooFileWatch   *watch,
                                                     guint           monitor_id);


G_END_DECLS

#endif /* __MOO_FILE_WATCH_H__ */
