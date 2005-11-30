/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   moofilewatch.h
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef MOO_USE_FAM
#include <fam.h>
#endif

#ifdef __WIN32__
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

#ifndef __WIN32__
#include <time.h>
#endif

#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include "mooutils/moofilewatch.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moocompat.h"


#if 0
#define DEBUG_PRINT g_message
#else
static void DEBUG_PRINT (G_GNUC_UNUSED const char *format, ...)
{
}
#endif

static const char *event_code_strings[] = {
    NULL,
    "CHANGED",
    "DELETED",
    "CREATED",
    "MOVED"
};

typedef struct _Monitor Monitor;

typedef enum {
    MONITOR_DIR,
    MONITOR_FILE
} MonitorType;

typedef struct {
    gboolean (*start)   (MooFileWatch   *watch,
                         GError        **error);
    gboolean (*shutdown)(MooFileWatch   *watch,
                         GError        **error);

    Monitor *(*create)  (MooFileWatch   *watch,
                         MonitorType     type,
                         const char     *filename,
                         gpointer        data,
                         int            *request,
                         GError        **error);
    void    (*suspend)  (Monitor        *monitor);
    void    (*resume)   (Monitor        *monitor);
    void    (*delete)   (Monitor        *monitor);
} WatchFuncs;


struct _MooFileWatchPrivate {
    guint stat_timeout;
#ifdef MOO_USE_FAM
    FAMConnection fam_connection;
    guint fam_connection_watch;
#endif
    WatchFuncs funcs;
    MooFileWatchMethod method;
    GSList      *monitors;
    GHashTable  *requests;  /* int -> Monitor* */
    guint alive : 1;
};

/* stat and win32 assign id themselves */
static int last_monitor_id = 0;


#ifdef MOO_USE_FAM
static gboolean watch_fam_start         (MooFileWatch   *watch,
                                         GError        **error);
static gboolean watch_fam_shutdown      (MooFileWatch   *watch,
                                         GError        **error);
static Monitor *monitor_fam_create      (MooFileWatch   *watch,
                                         MonitorType     type,
                                         const char     *filename,
                                         gpointer        data,
                                         int            *request,
                                         GError        **error);
static void     monitor_fam_suspend     (Monitor        *monitor);
static void     monitor_fam_resume      (Monitor        *monitor);
static void     monitor_fam_delete      (Monitor        *monitor);
#endif /* MOO_USE_FAM */

static gboolean watch_stat_start        (MooFileWatch   *watch,
                                         GError        **error);
static gboolean watch_stat_shutdown     (MooFileWatch   *watch,
                                         GError        **error);
static Monitor *monitor_stat_create     (MooFileWatch   *watch,
                                         MonitorType     type,
                                         const char     *filename,
                                         gpointer        data,
                                         int            *request,
                                         GError        **error);
static void     monitor_stat_suspend    (Monitor        *monitor);
static void     monitor_stat_resume     (Monitor        *monitor);
static void     monitor_stat_delete     (Monitor        *monitor);

#ifdef __WIN32__
static gboolean watch_win32_start       (MooFileWatch   *watch,
                                         GError        **error);
static gboolean watch_win32_shutdown    (MooFileWatch   *watch,
                                         GError        **error);
static Monitor *monitor_win32_create    (MooFileWatch   *watch,
                                         MonitorType     type,
                                         const char     *filename,
                                         gpointer        data,
                                         int            *request,
                                         GError        **error);
static void     monitor_win32_suspend   (Monitor        *monitor);
static void     monitor_win32_resume    (Monitor        *monitor);
static void     monitor_win32_delete    (Monitor        *monitor);
#endif /* __WIN32__ */


/* MOO_TYPE_FILE_WATCH */
G_DEFINE_TYPE (MooFileWatch, moo_file_watch, G_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_METHOD
};

enum {
    EVENT,
    ERROR_SIGNAL,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

static void moo_file_watch_finalize     (GObject            *object);

static void moo_file_watch_class_init   (MooFileWatchClass  *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_file_watch_finalize;

    signals[EVENT] =
            g_signal_new ("event",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooFileWatchClass, event),
                          NULL, NULL,
                          _moo_marshal_VOID__POINTER,
                          G_TYPE_NONE, 1,
                          G_TYPE_POINTER);

    signals[ERROR_SIGNAL] =
            g_signal_new ("error",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooFileWatchClass, error),
                          NULL, NULL,
                          _moo_marshal_VOID__POINTER,
                          G_TYPE_NONE, 1,
                          G_TYPE_POINTER);
}


static void moo_file_watch_init (MooFileWatch   *watch)
{
    watch->priv = g_new0 (MooFileWatchPrivate, 1);
    watch->priv->requests = g_hash_table_new (g_direct_hash, g_direct_equal);

#if defined (__WIN32__)
    watch->priv->method = MOO_FILE_WATCH_WIN32;
#elif defined (MOO_USE_FAM)
    watch->priv->method = MOO_FILE_WATCH_FAM;
#else
    watch->priv->method = MOO_FILE_WATCH_STAT;
#endif

#if defined (__WIN32__)
    g_assert (watch->priv->method == MOO_FILE_WATCH_WIN32);
    watch->priv->funcs.start = watch_win32_start;
    watch->priv->funcs.shutdown = watch_win32_shutdown;
    watch->priv->funcs.create = monitor_win32_create;
    watch->priv->funcs.suspend = monitor_win32_suspend;
    watch->priv->funcs.resume = monitor_win32_resume;
    watch->priv->funcs.delete = monitor_win32_delete;
#else /* !__WIN32__ */
    if (watch->priv->method == MOO_FILE_WATCH_STAT)
    {
        watch->priv->funcs.start = watch_stat_start;
        watch->priv->funcs.shutdown = watch_stat_shutdown;
        watch->priv->funcs.create = monitor_stat_create;
        watch->priv->funcs.suspend = monitor_stat_suspend;
        watch->priv->funcs.resume = monitor_stat_resume;
        watch->priv->funcs.delete = monitor_stat_delete;
    }
#ifdef MOO_USE_FAM
    else
    {
        watch->priv->funcs.start = watch_fam_start;
        watch->priv->funcs.shutdown = watch_fam_shutdown;
        watch->priv->funcs.create = monitor_fam_create;
        watch->priv->funcs.suspend = monitor_fam_suspend;
        watch->priv->funcs.resume = monitor_fam_resume;
        watch->priv->funcs.delete = monitor_fam_delete;
    }
#else /* !MOO_USE_FAM */
    else
    {
        g_assert_not_reached ();
    }
#endif /* !MOO_USE_FAM */
#endif /* !__WIN32__ */
}


static void moo_file_watch_finalize     (GObject            *object)
{
    GError *error = NULL;
    MooFileWatch *watch = MOO_FILE_WATCH (object);

    if (watch->priv->alive)
    {
        g_warning ("%s: finalizing open watch", G_STRLOC);
        if (!moo_file_watch_close (watch, &error))
        {
            g_warning ("%s: error in moo_file_watch_close()", G_STRLOC);
            g_warning ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }
    }

    g_hash_table_destroy (watch->priv->requests);

    g_free (watch->priv);
    watch->priv = NULL;

    G_OBJECT_CLASS(moo_file_watch_parent_class)->finalize (object);
}


GQuark  moo_file_watch_error_quark (void)
{
    static GQuark quark = 0;
    if (!quark)
        quark = g_quark_from_static_string ("moo-file-watch-error");
    return quark;
}


MooFileWatchMethod
moo_file_watch_get_method (MooFileWatch   *watch)
{
    g_return_val_if_fail (MOO_IS_FILE_WATCH (watch), -1);
    return watch->priv->method;
}


GType
moo_file_watch_method_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GEnumValue values[] = {
            {MOO_FILE_WATCH_STAT, (char*) "MOO_FILE_WATCH_STAT", (char*) "stat"},
            {MOO_FILE_WATCH_FAM, (char*) "MOO_FILE_WATCH_FAM", (char*) "fam"},
            {MOO_FILE_WATCH_WIN32, (char*) "MOO_FILE_WATCH_WIN32", (char*) "win32"},
            { 0, NULL, NULL }
        };

        type = g_enum_register_static ("MooFileWatchMethod", values);
    }

    return type;
}


GType
moo_file_watch_event_code_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GEnumValue values[] = {
            {MOO_FILE_WATCH_CHANGED, (char*) "MOO_FILE_WATCH_CHANGED", (char*) "changed"},
            {MOO_FILE_WATCH_DELETED, (char*) "MOO_FILE_WATCH_DELETED", (char*) "deleted"},
            {MOO_FILE_WATCH_CREATED, (char*) "MOO_FILE_WATCH_CREATED", (char*) "created"},
            {MOO_FILE_WATCH_MOVED, (char*) "MOO_FILE_WATCH_MOVED", (char*) "moved"},
            { 0, NULL, NULL }
        };

        type = g_enum_register_static ("MooFileWatchEventCode", values);
    }

    return type;
}


static gboolean
moo_file_watch_create_monitor (MooFileWatch   *watch,
                               MonitorType     type,
                               const char     *filename,
                               gpointer        data,
                               int            *request,
                               GError        **error)
{
    Monitor *monitor;
    int req;

    g_return_val_if_fail (MOO_IS_FILE_WATCH (watch), FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);

    if (!watch->priv->alive)
    {
        g_set_error (error, MOO_FILE_WATCH_ERROR,
                     MOO_FILE_WATCH_ERROR_CLOSED,
                     "MooFileWatch is closed");
        return FALSE;
    }

    monitor = watch->priv->funcs.create (watch, type, filename, data, &req, error);

    if (!monitor)
        return FALSE;

    watch->priv->monitors = g_slist_prepend (watch->priv->monitors, monitor);
    g_hash_table_insert (watch->priv->requests, GINT_TO_POINTER (req), monitor);

    if (request)
        *request = req;

    return TRUE;
}


gboolean        moo_file_watch_monitor_directory    (MooFileWatch   *watch,
                                                     const char     *filename,
                                                     gpointer        data,
                                                     int            *monitor_id,
                                                     GError        **error)
{
    return moo_file_watch_create_monitor (watch, MONITOR_DIR, filename,
                                          data, monitor_id, error);
}


gboolean        moo_file_watch_monitor_file         (MooFileWatch   *watch,
                                                     const char     *filename,
                                                     gpointer        data,
                                                     int            *monitor_id,
                                                     GError        **error)
{
    return moo_file_watch_create_monitor (watch, MONITOR_FILE, filename,
                                          data, monitor_id, error);
}


void            moo_file_watch_suspend_monitor      (MooFileWatch   *watch,
                                                     int             monitor_id)
{
    Monitor *monitor;

    g_return_if_fail (MOO_IS_FILE_WATCH (watch));

    monitor = g_hash_table_lookup (watch->priv->requests,
                                   GINT_TO_POINTER (monitor_id));
    g_return_if_fail (monitor != NULL);

    watch->priv->funcs.suspend (monitor);
}


void            moo_file_watch_resume_monitor       (MooFileWatch   *watch,
                                                     int             monitor_id)
{
    Monitor *monitor;

    g_return_if_fail (MOO_IS_FILE_WATCH (watch));

    monitor = g_hash_table_lookup (watch->priv->requests,
                                   GINT_TO_POINTER (monitor_id));
    g_return_if_fail (monitor != NULL);

    watch->priv->funcs.resume (monitor);
}


void            moo_file_watch_cancel_monitor       (MooFileWatch   *watch,
                                                     int             monitor_id)
{
    Monitor *monitor;

    g_return_if_fail (MOO_IS_FILE_WATCH (watch));

    monitor = g_hash_table_lookup (watch->priv->requests,
                                   GINT_TO_POINTER (monitor_id));
    g_return_if_fail (monitor != NULL);

    watch->priv->funcs.delete (monitor);
    watch->priv->monitors = g_slist_remove (watch->priv->monitors, monitor);
    g_hash_table_remove (watch->priv->requests,
                         GINT_TO_POINTER (monitor_id));
}


static void emit_event  (MooFileWatch       *watch,
                         MooFileWatchEvent  *event)
{
    DEBUG_PRINT ("watch %p, monitor %d: event %s for %s",
                 watch, event->monitor_id,
                 event_code_strings[event->code],
                 event->filename);
    g_signal_emit (watch, signals[EVENT], 0, event);
}


MooFileWatch   *moo_file_watch_new                  (GError        **error)
{
    MooFileWatch *watch;

    watch = MOO_FILE_WATCH (g_object_new (MOO_TYPE_FILE_WATCH, NULL));

    if (!watch->priv->funcs.start (watch, error))
    {
        g_object_unref (watch);
        return NULL;
    }

    watch->priv->alive = TRUE;
    return watch;
}


gboolean        moo_file_watch_close                (MooFileWatch   *watch,
                                                     GError        **error)
{
    g_return_val_if_fail (MOO_IS_FILE_WATCH (watch), FALSE);

    if (!watch->priv->alive)
        return TRUE;

    watch->priv->alive = FALSE;

    g_slist_foreach (watch->priv->monitors,
                     (GFunc) watch->priv->funcs.delete, NULL);
    g_slist_free (watch->priv->monitors);
    watch->priv->monitors = NULL;

    return watch->priv->funcs.shutdown (watch, error);
}


struct _Monitor {
    MooFileWatch *parent;
    MonitorType type;
    char *filename;
    gpointer user_data;
    int request;
    guint suspended : 1;
    struct stat statbuf;
};


/*****************************************************************************/
/* FAM
 */
#ifdef MOO_USE_FAM

#define SET_FAM_ERROR(func,error)                       \
G_STMT_START {                                          \
    g_set_error (error, MOO_FILE_WATCH_ERROR,           \
                 MOO_FILE_WATCH_ERROR_FAILED,           \
                 #func " failed: %s",                   \
                 ((FAMErrno && FamErrlist[FAMErrno]) ?  \
                    FamErrlist[FAMErrno] :              \
                    "unknown error"));                  \
} G_STMT_END

#define RETURN_FAM_ERROR(func,error,ret)                \
G_STMT_START {                                          \
    SET_FAM_ERROR (func,error);                         \
    return ret;                                         \
} G_STMT_END


#define MOO_FAM_SOCKET_WATCH_PRIORITY   G_PRIORITY_DEFAULT


static gboolean read_fam_events         (GIOChannel     *source,
                                         GIOCondition    condition,
                                         MooFileWatch   *watch);

static gboolean watch_fam_start         (MooFileWatch   *watch,
                                         GError        **error)
{
    GIOChannel *fam_socket;

    if (FAMOpen (&watch->priv->fam_connection))
        RETURN_FAM_ERROR (FAMOpen, error, FALSE);

    fam_socket = g_io_channel_unix_new (watch->priv->fam_connection.fd);
    watch->priv->fam_connection_watch =
            g_io_add_watch_full (fam_socket, MOO_FAM_SOCKET_WATCH_PRIORITY,
                                    G_IO_IN | G_IO_PRI | G_IO_PRI | G_IO_HUP,
                                    (GIOFunc) read_fam_events, watch, NULL);
    g_io_channel_unref (fam_socket);

    return TRUE;
}


static gboolean watch_fam_shutdown      (MooFileWatch   *watch,
                                         GError        **error)
{
    if (watch->priv->fam_connection_watch)
        g_source_remove (watch->priv->fam_connection_watch);

    if (FAMClose (&watch->priv->fam_connection))
        RETURN_FAM_ERROR (FAMOpen, error, FALSE);
    else
        return TRUE;
}


static Monitor *monitor_fam_create  (MooFileWatch   *watch,
                                     MonitorType     type,
                                     const char     *filename,
                                     gpointer        data,
                                     int            *request,
                                     GError        **error)
{
    FAMRequest fr;
    int result;
    Monitor *monitor;

    g_return_val_if_fail (filename != NULL, FALSE);

    if (type == MONITOR_DIR)
        result = FAMMonitorDirectory (&watch->priv->fam_connection,
                                       filename, &fr, data);
    else
        result = FAMMonitorFile (&watch->priv->fam_connection,
                                  filename, &fr, data);

    if (result)
    {
        DEBUG_PRINT ("Connection %d: creating monitor for '%s' failed",
                     watch->priv->fam_connection.fd, filename);
        if (type == MONITOR_DIR)
            RETURN_FAM_ERROR (FAMMonitorDirectory, error, NULL);
        else
            RETURN_FAM_ERROR (FAMMonitorFile, error, NULL);
    }
    else
    {
        DEBUG_PRINT ("Connection %d: created monitor %d for %s '%s'",
                     watch->priv->fam_connection.fd, fr.reqnum,
                     type == MONITOR_DIR ? "directory" : "file",
                     filename);
    }

    if (request)
        *request = fr.reqnum;

    monitor = g_new0 (Monitor, 1);

    monitor->parent = watch;
    monitor->type = type;
    monitor->filename = g_strdup (filename);
    monitor->user_data = data;
    monitor->request = fr.reqnum;
    monitor->suspended = FALSE;

    return monitor;
}


static void     monitor_fam_suspend (Monitor        *monitor)
{
    FAMRequest fr;
    int result;

    g_return_if_fail (monitor != NULL);

    if (monitor->suspended)
        return;

    fr.reqnum = monitor->request;

    result = FAMSuspendMonitor (&monitor->parent->priv->fam_connection, &fr);

    if (result)
    {
        DEBUG_PRINT ("Connection %d: FAMSuspendMonitor for %s '%s' failed",
                     monitor->parent->priv->fam_connection.fd,
                     monitor->type == MONITOR_DIR ? "directory" : "file",
                     monitor->filename);
        return;
    }
    else
    {
        DEBUG_PRINT ("Connection %d: suspended monitor %d for %s '%s'",
                     monitor->parent->priv->fam_connection.fd,
                     monitor->request,
                     monitor->type == MONITOR_DIR ? "directory" : "file",
                     monitor->filename);
    }

    monitor->suspended = TRUE;
}


static void     monitor_fam_resume  (Monitor        *monitor)
{
    FAMRequest fr;
    int result;

    g_return_if_fail (monitor != NULL);

    if (!monitor->suspended)
        return;

    fr.reqnum = monitor->request;

    result = FAMResumeMonitor (&monitor->parent->priv->fam_connection, &fr);

    if (result)
    {
        DEBUG_PRINT ("Connection %d: FAMResumeMonitor for %s '%s' failed",
                     monitor->parent->priv->fam_connection.fd,
                     monitor->type == MONITOR_DIR ? "directory" : "file",
                     monitor->filename);
        return;
    }
    else
    {
        DEBUG_PRINT ("Connection %d: resumed monitor %d for %s '%s'",
                     monitor->parent->priv->fam_connection.fd,
                     monitor->request,
                     monitor->type == MONITOR_DIR ? "directory" : "file",
                     monitor->filename);
    }

    monitor->suspended = FALSE;
}


static void     monitor_fam_delete  (Monitor        *monitor)
{
    FAMRequest fr;
    int result;

    g_return_if_fail (monitor != NULL);

    fr.reqnum = monitor->request;

    result = FAMCancelMonitor (&monitor->parent->priv->fam_connection, &fr);

    if (result)
    {
        DEBUG_PRINT ("Connection %d: FAMCancelMonitor for %s '%s' failed",
                     monitor->parent->priv->fam_connection.fd,
                     monitor->type == MONITOR_DIR ? "directory" : "file",
                     monitor->filename);
    }

    g_free (monitor->filename);
    g_free (monitor);
}


static gboolean read_fam_events (G_GNUC_UNUSED GIOChannel *source,
                                 GIOCondition    condition,
                                 MooFileWatch   *watch)
{
    GError *error = NULL;
    int result;
    FAMConnection *connection = &watch->priv->fam_connection;
    gboolean retval = TRUE;

    g_object_ref (watch);

    if (condition & (G_IO_ERR | G_IO_HUP))
    {
        DEBUG_PRINT ("Connection %d: FAM socket broke",
                     connection->fd);

        g_set_error (&error, MOO_FILE_WATCH_ERROR,
                      MOO_FILE_WATCH_ERROR_FAILED,
                      "Connection to FAM is broken");
        g_signal_emit (watch, signals[ERROR_SIGNAL], 0, error);
        g_error_free (error);
        error = NULL;

        if (!moo_file_watch_close (watch, &error))
        {
            DEBUG_PRINT ("%s: error in moo_file_watch_close()", G_STRLOC);
            DEBUG_PRINT ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }

        retval = FALSE;
        goto out;
    }

    while ((result = FAMPending (connection)))
    {
        FAMEvent fe;
        MooFileWatchEvent event;
        gboolean emit = TRUE;

        if (result < 0)
        {
            SET_FAM_ERROR (FAMPending, &error);
            DEBUG_PRINT ("Connection %d: FAMPending failed", connection->fd);
        }
        else if (FAMNextEvent (connection, &fe) != 1)
        {
            SET_FAM_ERROR (FAMNextEvent, &error);
            DEBUG_PRINT ("Connection %d: FAMNextEvent failed", connection->fd);
        }

        if (error)
        {
            g_signal_emit (watch, signals[ERROR_SIGNAL], 0, error);
            g_error_free (error);
            error = NULL;

            if (!moo_file_watch_close (watch, &error))
            {
                DEBUG_PRINT ("%s: error in moo_file_watch_close()", G_STRLOC);
                DEBUG_PRINT ("%s: %s", G_STRLOC, error->message);
                g_error_free (error);
            }

            retval = FALSE;
            goto out;
        }

        /* TODO: check monitor here */
        event.monitor_id = fe.fr.reqnum;
        event.filename = fe.filename;
        event.data = fe.userdata;

        switch (fe.code)
        {
            case FAMChanged:
                event.code = MOO_FILE_WATCH_CHANGED;
                break;
            case FAMDeleted:
                event.code = MOO_FILE_WATCH_DELETED;
                break;
            case FAMCreated:
                event.code = MOO_FILE_WATCH_CREATED;
                break;
            case FAMMoved:
                event.code = MOO_FILE_WATCH_MOVED;
                break;

            case FAMStartExecuting:
            case FAMStopExecuting:
            case FAMAcknowledge:
            case FAMExists:
            case FAMEndExist:
                emit = FALSE;
                break;

            default:
                emit = FALSE;
                g_warning ("%s: unknown FAM code %d", G_STRLOC, fe.code);
        }

        if (emit)
        {
            emit_event (watch, &event);
        }
    }

out:
    g_object_unref (watch);
    return retval;
}


#endif /* MOO_USE_FAM */


/*****************************************************************************/
/* stat()
 */

#define MOO_STAT_PRIORITY               G_PRIORITY_DEFAULT
#define MOO_STAT_TIMEOUT                500

static MooFileWatchError errno_to_file_error    (int             code);
static gboolean do_stat                         (MooFileWatch   *watch);


static gboolean watch_stat_start        (MooFileWatch   *watch,
                                         G_GNUC_UNUSED GError **error)
{
    watch->priv->stat_timeout =
            g_timeout_add_full (MOO_STAT_PRIORITY,
                                MOO_STAT_TIMEOUT,
                                (GSourceFunc) do_stat,
                                watch, NULL);
    return TRUE;
}


static gboolean watch_stat_shutdown     (MooFileWatch   *watch,
                                         G_GNUC_UNUSED GError **error)
{
    if (watch->priv->stat_timeout)
        g_source_remove (watch->priv->stat_timeout);
    watch->priv->stat_timeout = 0;
    return TRUE;
}


static Monitor *monitor_stat_create     (MooFileWatch   *watch,
                                         MonitorType     type,
                                         const char     *filename,
                                         gpointer        data,
                                         int            *request,
                                         GError        **error)
{
    Monitor *monitor;
    struct stat buf;

    g_return_val_if_fail (MOO_IS_FILE_WATCH (watch), NULL);
    g_return_val_if_fail (filename != NULL, NULL);

    if (stat (filename, &buf))
    {
        int saved_errno = errno;
        g_set_error (error, MOO_FILE_WATCH_ERROR,
                     errno_to_file_error (saved_errno),
                     "stat: %s", g_strerror (saved_errno));
        return NULL;
    }

    if (type == MONITOR_DIR && !S_ISDIR (buf.st_mode))
    {
        char *display_name = g_filename_display_name (filename);
        g_set_error (error, MOO_FILE_WATCH_ERROR,
                     MOO_FILE_WATCH_ERROR_NOT_DIR,
                     "%s is not a directory", display_name);
        g_free (display_name);
        return NULL;
    }
    else if (type == MONITOR_FILE && S_ISDIR (buf.st_mode)) /* it's fatal on windows */
    {
        char *display_name = g_filename_display_name (filename);
        g_set_error (error, MOO_FILE_WATCH_ERROR,
                     MOO_FILE_WATCH_ERROR_IS_DIR,
                     "%s is a directory", display_name);
        g_free (display_name);
        return NULL;
    }

    DEBUG_PRINT ("created monitor for '%s'", filename);

    monitor = g_new0 (Monitor, 1);
    monitor->parent = watch;
    monitor->type = type;
    monitor->filename = g_strdup (filename);
    monitor->user_data = data;
    monitor->request = ++last_monitor_id;
    monitor->suspended = FALSE;

    memcpy (&monitor->statbuf, &buf, sizeof (buf));

    if (request)
        *request = monitor->request;

    return monitor;
}


static void     monitor_stat_suspend    (Monitor        *monitor)
{
    g_return_if_fail (monitor != NULL);
    monitor->suspended = TRUE;
}


static void     monitor_stat_resume     (Monitor        *monitor)
{
    g_return_if_fail (monitor != NULL);
    monitor->suspended = FALSE;
}


static void     monitor_stat_delete     (Monitor        *monitor)
{
    g_return_if_fail (monitor != NULL);
    DEBUG_PRINT ("removing monitor for '%s'", monitor->filename);
    g_free (monitor->filename);
    g_free (monitor);
}


static gboolean do_stat                 (MooFileWatch   *watch)
{
    GSList *l, *list, *to_remove = NULL;
    gboolean result = TRUE;

    g_return_val_if_fail (MOO_IS_FILE_WATCH (watch), FALSE);

    g_object_ref (watch);

    if (!watch->priv->monitors)
        goto out;

    list = g_slist_copy (watch->priv->monitors);

    for (l = list; l != NULL; l = l->next)
    {
        MooFileWatchEvent event;
        Monitor *monitor = l->data;
        time_t old;

        g_assert (monitor != NULL);

        if (!g_hash_table_lookup (watch->priv->requests,
             GINT_TO_POINTER (monitor->request)))
                continue;

        if (monitor->suspended)
            continue;

        old = monitor->statbuf.st_mtime;

        if (stat (monitor->filename, &monitor->statbuf))
        {
            event.code = MOO_FILE_WATCH_DELETED;
            event.monitor_id = monitor->request;
            event.filename = monitor->filename;
            event.data = monitor->user_data;

            DEBUG_PRINT ("'%s' deleted", monitor->filename);

            to_remove = g_slist_prepend (to_remove,
                                         GINT_TO_POINTER (monitor->request));

            emit_event (watch, &event);
        }
        else if (monitor->statbuf.st_mtime > old)
        {
            event.code = MOO_FILE_WATCH_CHANGED;
            event.monitor_id = monitor->request;
            event.filename = monitor->filename;
            event.data = monitor->user_data;

            DEBUG_PRINT ("'%s' changed", monitor->filename);

            emit_event (watch, &event);
        }
    }

    for (l = to_remove; l != NULL; l = l->next)
        if (g_hash_table_lookup (watch->priv->requests, GINT_TO_POINTER (l->data)))
            moo_file_watch_cancel_monitor (watch, GPOINTER_TO_INT (l->data));

    g_slist_free (to_remove);
    g_slist_free (list);

out:
    g_object_unref (watch);
    return result;
}


static MooFileWatchError errno_to_file_error (int code)
{
    MooFileWatchError fcode = MOO_FILE_WATCH_ERROR_FAILED;

    switch (code)
    {
        case EACCES:
            fcode = MOO_FILE_WATCH_ERROR_ACCESS_DENIED;
            break;
        case ENAMETOOLONG:
            fcode = MOO_FILE_WATCH_ERROR_BAD_FILENAME;
            break;
        case ENOENT:
            fcode = MOO_FILE_WATCH_ERROR_NONEXISTENT;
            break;
        case ENOTDIR:
            fcode = MOO_FILE_WATCH_ERROR_NOT_DIR;
            break;
    }

    return fcode;
}



/*****************************************************************************/
/* win32
 */
#ifdef __WIN32__

static gboolean watch_win32_start       (MooFileWatch   *watch,
                                         GError        **error)
{
    return watch_stat_start (watch, error);
}


static gboolean watch_win32_shutdown    (MooFileWatch   *watch,
                                         GError        **error)
{
    return watch_stat_shutdown (watch, error);
}


static Monitor *monitor_win32_create    (MooFileWatch   *watch,
                                         MonitorType     type,
                                         const char     *filename,
                                         gpointer        data,
                                         int            *request,
                                         GError        **error)
{
    g_return_val_if_fail (filename != NULL, FALSE);

    if (type == MONITOR_FILE)
        return monitor_stat_create (watch, type, filename, data, request, error);

#warning "Implement me"
    g_set_error (error, MOO_FILE_WATCH_ERROR,
                 MOO_FILE_WATCH_ERROR_NOT_IMPLEMENTED,
                 "watching folders is not implemented on win32");
    return NULL;
}


static void     monitor_win32_suspend   (Monitor        *monitor)
{
    g_return_if_fail (monitor != NULL);

    if (monitor->suspended)
        return;

    if (monitor->type == MONITOR_FILE)
        return monitor_stat_suspend (monitor);

    g_return_if_reached ();
}


static void     monitor_win32_resume    (Monitor        *monitor)
{
    g_return_if_fail (monitor != NULL);

    if (!monitor->suspended)
        return;

    if (monitor->type == MONITOR_FILE)
        return monitor_stat_resume (monitor);

    g_return_if_reached ();
}


static void     monitor_win32_delete    (Monitor        *monitor)
{
    g_return_if_fail (monitor != NULL);

    if (monitor->type == MONITOR_FILE)
        return monitor_stat_delete (monitor);

    g_return_if_reached ();
}
#endif /* __WIN32__ */
