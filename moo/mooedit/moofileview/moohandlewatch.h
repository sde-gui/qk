/*
 *   moohandlewatch.h
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

/* Win32 HANDLE monitor. It's a GSource wrapping WaitForMultipleObjects,
   intended to workaround limitation of 64 handles per GMainContext. */

#ifndef __MOO_HANDLE_WATCH_H__
#define __MOO_HANDLE_WATCH_H__

#include <glib.h>
#ifdef __WIN32__
#include <windows.h>
#else
typedef gpointer HANDLE;
#endif

G_BEGIN_DECLS


#define MOO_HANDLE_WATCH_ERROR (moo_handle_watch_error_quark ())

typedef enum {
    MOO_HANDLE_WATCH_ERROR_FAILED,
    MOO_HANDLE_WATCH_ERROR_NOT_IMPLEMENTED
} MooHandleWatchError;

typedef enum {
    MOO_HANDLE_SIGNALLED,
    MOO_HANDLE_ERROR
} MooHandleCondition;

typedef gboolean (*MooHandleFunc)   (HANDLE             handle,
                                     MooHandleCondition condition,
                                     gpointer           user_data);

GQuark      moo_handle_watch_error_quark    (void);

guint       moo_handle_watch_add            (HANDLE         handle,
                                             MooHandleFunc  func,
                                             gpointer       user_data);

guint       moo_handle_watch_remove_handle  (HANDLE handle);
guint       moo_handle_watch_remove         (guint  handle_id);


// #define MOO_TYPE_HANDLE_WATCH                (moo_handle_watch_get_type ())
// #define MOO_HANDLE_WATCH(object)             (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_HANDLE_WATCH, MooHandleWatch))
// #define MOO_HANDLE_WATCH_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_HANDLE_WATCH, MooHandleWatchClass))
// #define MOO_IS_HANDLE_WATCH(object)          (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_HANDLE_WATCH))
// #define MOO_IS_HANDLE_WATCH_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_HANDLE_WATCH))
// #define MOO_HANDLE_WATCH_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_HANDLE_WATCH, MooHandleWatchClass))
//
// #define MOO_TYPE_HANDLE_WATCH_EVENT_CODE     (moo_handle_watch_event_code_get_type ())
// #define MOO_TYPE_HANDLE_WATCH_METHOD         (moo_handle_watch_method_get_type ())
//
// typedef enum {
//     MOO_HANDLE_WATCH_STAT,
//     MOO_HANDLE_WATCH_FAM,
//     MOO_HANDLE_WATCH_WIN32
// } MooHandleWatchMethod;
//
// /* Stripped FAMEventCode enumeration */
// typedef enum {
//     MOO_HANDLE_WATCH_CHANGED         = 1,
//     MOO_HANDLE_WATCH_DELETED         = 2,
//     MOO_HANDLE_WATCH_CREATED         = 3,
//     MOO_HANDLE_WATCH_MOVED           = 4,
// } MooHandleWatchEventCode;
//
// /*  The structure has the same meaning as the FAMEvent
//     (it is a simple copy of FAMEvent structure when
//     FAM is used). In the case when stat() is used, when
//     directory content is changed, MooHandleWatch does not
//     try to learn what happened, and just emits CHANGED
//     event with filename set to directory name.
//  */
// struct _MooHandleWatchEvent {
//     MooHandleWatchEventCode code; /* FAMEventCode */
//     int          monitor_id;    /* FAMRequest */
//     char        *filename;
//     gpointer     data;
// };
//
// typedef struct _MooHandleWatch          MooHandleWatch;
// typedef struct _MooHandleWatchPrivate   MooHandleWatchPrivate;
// typedef struct _MooHandleWatchClass     MooHandleWatchClass;
// typedef struct _MooHandleWatchEvent     MooHandleWatchEvent;
//
// struct _MooHandleWatch
// {
//     GObject parent;
//     MooHandleWatchPrivate *priv;
// };
//
// struct _MooHandleWatchClass
// {
//     GObjectClass parent_class;
//
//     void    (*event)    (MooHandleWatch       *watch,
//     MooHandleWatchEvent  *event);
//     void    (*error)    (MooHandleWatch       *watch,
//     GError             *error);
// };
//
// GType           moo_handle_watch_get_type             (void) G_GNUC_CONST;
// GType           moo_handle_watch_event_code_get_type  (void) G_GNUC_CONST;
// GType           moo_handle_watch_method_get_type      (void) G_GNUC_CONST;
//
// /* FAMOpen */
// MooHandleWatch   *moo_handle_watch_new                  (GError        **error);
//
// /* FAMClose */
// gboolean        moo_handle_watch_close                (MooHandleWatch   *watch,
//         GError        **error);
//
// /* FAMMonitorDirectory, FAMMonitorFile */
// gboolean        moo_handle_watch_monitor_directory    (MooHandleWatch   *watch,
//         const char     *filename,
//         gpointer        data,
//         int            *monitor_id,
//         GError        **error);
// gboolean        moo_handle_watch_monitor_file         (MooHandleWatch   *watch,
//         const char     *filename,
//         gpointer        data,
//         int            *monitor_id,
//         GError        **error);
//
// /* FAMSuspendMonitor, FAMResumeMonitor, FAMCancelMonitor */
// void            moo_handle_watch_suspend_monitor      (MooHandleWatch   *watch,
//         int             monitor_id);
// void            moo_handle_watch_resume_monitor       (MooHandleWatch   *watch,
//         int             monitor_id);
// void            moo_handle_watch_cancel_monitor       (MooHandleWatch   *watch,
//         int             monitor_id);
//
// MooHandleWatchMethod moo_handle_watch_get_method        (MooHandleWatch   *watch);


G_END_DECLS

#endif /* __MOO_HANDLE_WATCH_H__ */
