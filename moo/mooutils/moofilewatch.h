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


#define MOO_TYPE_FILE_WATCH                (moo_file_watch_get_type ())
#define MOO_FILE_WATCH(object)             (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_FILE_WATCH, MooFileWatch))
#define MOO_FILE_WATCH_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_FILE_WATCH, MooFileWatchClass))
#define MOO_IS_FILE_WATCH(object)          (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_FILE_WATCH))
#define MOO_IS_FILE_WATCH_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_FILE_WATCH))
#define MOO_FILE_WATCH_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_FILE_WATCH, MooFileWatchClass))

#define MOO_TYPE_FILE_WATCH_EVENT_CODE     (moo_file_watch_event_code_get_type ())
#define MOO_TYPE_FILE_WATCH_METHOD         (moo_file_watch_method_get_type ())

typedef enum {
    MOO_FILE_WATCH_STAT,
    MOO_FILE_WATCH_FAM,
    MOO_FILE_WATCH_WIN32
} MooFileWatchMethod;

/* Stripped FAMEventCode enumeration */
typedef enum {
    MOO_FILE_WATCH_CHANGED         = 1,
    MOO_FILE_WATCH_DELETED         = 2,
    MOO_FILE_WATCH_CREATED         = 3,
    MOO_FILE_WATCH_MOVED           = 4,
} MooFileWatchEventCode;

/*  The structure has the same meaning as the FAMEvent
    (it is a simple copy of FAMEvent structure when
    FAM is used). In the case when stat() is used, when
    directory content is changed, MooFileWatch does not
    try to learn what happened, and just emits CHANGED
    event with filename set to directory name.
 */
struct _MooFileWatchEvent {
    MooFileWatchEventCode code; /* FAMEventCode */
    int          monitor_id;    /* FAMRequest */
    char        *filename;
    gpointer     data;
};

typedef struct _MooFileWatch          MooFileWatch;
typedef struct _MooFileWatchPrivate   MooFileWatchPrivate;
typedef struct _MooFileWatchClass     MooFileWatchClass;
typedef struct _MooFileWatchEvent     MooFileWatchEvent;

struct _MooFileWatch
{
    GObject parent;
    MooFileWatchPrivate *priv;
};

struct _MooFileWatchClass
{
    GObjectClass parent_class;

    void    (*event)    (MooFileWatch       *watch,
                         MooFileWatchEvent  *event);
    void    (*error)    (MooFileWatch       *watch,
                         GError             *error);
};

GType           moo_file_watch_get_type             (void) G_GNUC_CONST;
GType           moo_file_watch_event_code_get_type  (void) G_GNUC_CONST;
GType           moo_file_watch_method_get_type      (void) G_GNUC_CONST;

/* FAMOpen */
MooFileWatch   *moo_file_watch_new                  (GError        **error);

/* FAMClose */
gboolean        moo_file_watch_close                (MooFileWatch   *watch,
                                                     GError        **error);

/* FAMMonitorDirectory, FAMMonitorFile */
gboolean        moo_file_watch_monitor_directory    (MooFileWatch   *watch,
                                                     const char     *filename,
                                                     gpointer        data,
                                                     int            *monitor_id,
                                                     GError        **error);
gboolean        moo_file_watch_monitor_file         (MooFileWatch   *watch,
                                                     const char     *filename,
                                                     gpointer        data,
                                                     int            *monitor_id,
                                                     GError        **error);

/* FAMSuspendMonitor, FAMResumeMonitor, FAMCancelMonitor */
void            moo_file_watch_suspend_monitor      (MooFileWatch   *watch,
                                                     int             monitor_id);
void            moo_file_watch_resume_monitor       (MooFileWatch   *watch,
                                                     int             monitor_id);
void            moo_file_watch_cancel_monitor       (MooFileWatch   *watch,
                                                     int             monitor_id);

MooFileWatchMethod moo_file_watch_get_method        (MooFileWatch   *watch);


G_END_DECLS

#endif /* __MOO_FILE_WATCH_H__ */
