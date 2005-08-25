/*
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
#include "moofilewatch.h"
#include "mooutils/moomarshals.h"


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
#ifdef __WIN32__
typedef struct _WinFamConnection  WinFamConnection;
#endif

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
#ifdef __WIN32__
    WinFamConnection *fam_connection;
    guint fam_timeout;
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


static gboolean moo_file_watch_create_monitor       (MooFileWatch   *watch,
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

/* For directories it uses FindFirstChangeNotification and friends; for
   files - stat() method. */

typedef MooFileWatchEvent WinFamEvent;

typedef struct {
    WinFamConnection *fc;
    int reqnum;
    gboolean suspended;
    gpointer user_data;
    char *filename;
} Request;

static WinFamEvent *win_fam_event_new       (MooFileWatchEventCode   code,
                                             Request                *request);
static void         win_fam_event_free      (WinFamEvent    *fe);

struct _WinFamConnection {
    GAsyncQueue *events;
};


/* Starts WinFam and creates new connection to it. Returned structure
   must not be modified or freed, it must be passed to win_fam_close(). */
static WinFamConnection *win_fam_open       (GError            **error);

/* Removes given connection and stops WinFam if it's the last connection.
   fc structure is freed, so it must not be accessed after win_fam_close()
   call, regardless of returned status. */
static gboolean          win_fam_close      (WinFamConnection   *fc,
                                             GError            **error);

static gboolean win_fam_monitor_directory   (WinFamConnection   *fc,
                                             const char         *filename,
                                             int                *request,
                                             gpointer            user_data,
                                             GError            **error);

static gboolean win_fam_suspend_monitor     (WinFamConnection   *fc,
                                             int                 request,
                                             GError            **error);
static gboolean win_fam_resume_monitor      (WinFamConnection   *fc,
                                             int                 request,
                                             GError            **error);
static gboolean win_fam_cancel_monitor      (WinFamConnection   *fc,
                                             int                 request,
                                             GError            **error);


/*****************************************************************************/
/* MooFileWatch
 */

#define MOO_WIN_FAM_WATCH_PRIORITY  G_PRIORITY_DEFAULT
#define MOO_WIN_FAM_WATCH_TIMEOUT   500


static gboolean check_fam_events        (MooFileWatch   *watch);

static gboolean watch_win32_start       (MooFileWatch   *watch,
                                         GError        **error)
{
    watch->priv->fam_connection = win_fam_open (error);

    if (!watch->priv->fam_connection)
        return FALSE;

    watch->priv->fam_timeout =
            g_timeout_add_full (MOO_WIN_FAM_WATCH_TIMEOUT,
                                MOO_WIN_FAM_WATCH_PRIORITY,
                                (GSourceFunc) check_fam_events, watch, NULL);

    return watch_stat_start (watch, error);
}


static gboolean watch_win32_shutdown    (MooFileWatch   *watch,
                                         GError        **error)
{
    WinFamConnection *fc = watch->priv->fam_connection;
    gboolean result;

    watch->priv->fam_connection = NULL;

    if (watch->priv->fam_timeout)
        g_source_remove (watch->priv->fam_timeout);

    result = win_fam_close (fc, error);
    watch_stat_shutdown (watch, NULL);
    return result;
}


static Monitor *monitor_win32_create    (MooFileWatch   *watch,
                                         MonitorType     type,
                                         const char     *filename,
                                         gpointer        data,
                                         int            *request,
                                         GError        **error)
{
    int reqnum;
    gboolean result;
    Monitor *monitor;

    g_return_val_if_fail (filename != NULL, FALSE);

    if (type == MONITOR_FILE)
        return monitor_stat_create (watch, type, filename, data, request, error);

    result = win_fam_monitor_directory (watch->priv->fam_connection,
                                        filename, &reqnum, data, error);

    if (!result)
    {
        DEBUG_PRINT ("Creating monitor for '%s' failed", filename);
        return FALSE;
    }
    else
    {
        DEBUG_PRINT ("Created monitor %d for %s '%s'", reqnum,
                     type == MONITOR_DIR ? "directory" : "file",
                     filename);
    }

    if (request)
        *request = reqnum;

    monitor = g_new0 (Monitor, 1);

    monitor->parent = watch;
    monitor->type = type;
    monitor->filename = g_strdup (filename);
    monitor->user_data = data;
    monitor->request = reqnum;
    monitor->suspended = FALSE;

    return monitor;
}


static void     monitor_win32_suspend   (Monitor        *monitor)
{
    int result;

    g_return_if_fail (monitor != NULL);

    if (monitor->suspended)
        return;

    if (monitor->type == MONITOR_FILE)
        return monitor_stat_suspend (monitor);

    result = win_fam_suspend_monitor (monitor->parent->priv->fam_connection,
                                      monitor->request, NULL);

    if (!result)
    {
        DEBUG_PRINT ("win_fam_suspend_monitor for %s '%s' failed",
                     monitor->type == MONITOR_DIR ? "directory" : "file",
                     monitor->filename);
        return;
    }
    else
    {
        DEBUG_PRINT ("Suspended monitor %d for %s '%s'", monitor->request,
                     monitor->type == MONITOR_DIR ? "directory" : "file",
                     monitor->filename);
    }

    monitor->suspended = TRUE;
}


static void     monitor_win32_resume    (Monitor        *monitor)
{
    gboolean result;

    g_return_if_fail (monitor != NULL);

    if (!monitor->suspended)
        return;

    if (monitor->type == MONITOR_FILE)
        return monitor_stat_resume (monitor);

    result = win_fam_resume_monitor (monitor->parent->priv->fam_connection,
                                     monitor->request, NULL);

    if (!result)
    {
        DEBUG_PRINT ("win_fam_resume_monitor for %s '%s' failed",
                     monitor->type == MONITOR_DIR ? "directory" : "file",
                     monitor->filename);
        return;
    }
    else
    {
        DEBUG_PRINT ("Resumed monitor %d for %s '%s'", monitor->request,
                     monitor->type == MONITOR_DIR ? "directory" : "file",
                     monitor->filename);
    }

    monitor->suspended = FALSE;
}


static void     monitor_win32_delete    (Monitor        *monitor)
{
    int result;

    g_return_if_fail (monitor != NULL);

    if (monitor->type == MONITOR_FILE)
        return monitor_stat_delete (monitor);

    result = win_fam_cancel_monitor (monitor->parent->priv->fam_connection,
                                     monitor->request, NULL);

    if (!result)
    {
        DEBUG_PRINT ("win_fam_cancel_monitor for %s '%s' failed",
                     monitor->type == MONITOR_DIR ? "directory" : "file",
                     monitor->filename);
    }

    g_free (monitor->filename);
    g_free (monitor);
}


static gboolean check_fam_events        (MooFileWatch   *watch)
{
    GError *error = NULL;
    WinFamConnection *connection = watch->priv->fam_connection;
    GAsyncQueue *queue = connection->events;
    gboolean retval = TRUE;

    g_object_ref (watch);

    while (g_async_queue_length (queue))
    {
        WinFamEvent *event = g_async_queue_pop (queue);

        if (!event)
        {
            g_set_error (&error, MOO_FILE_WATCH_ERROR,
                         MOO_FILE_WATCH_ERROR_FAILED, "Failed");
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

        emit_event (watch, event);
        win_fam_event_free (event);
    }

out:
    g_object_unref (watch);
    return retval;
}



/*****************************************************************************/
/* WinFam - aka FAM on windows
 */

typedef enum {
    SIGNAL_NEW_CONNECTION   = 1,
    SIGNAL_CLOSE_CONNECTION = 2,
    SIGNAL_MONITOR          = 3,
    SIGNAL_SUSPEND_MONITOR  = 4,
    SIGNAL_RESUME_MONITOR   = 5,
    SIGNAL_CANCEL_MONITOR   = 6
} SignalCode;

typedef struct {
    GError **error;
} SignalNew;

typedef struct {
    GError **error;
    WinFamConnection *fc;
} SignalClose;

typedef struct {
    GError **error;
    WinFamConnection *fc;
    const char *filename;
    int request;
    gpointer user_data;
} SignalMonitor;

typedef struct {
    GError **error;
    WinFamConnection *fc;
    int request;
} SignalModifyMonitor;

typedef union {
    SignalNew new;
    SignalClose close;
    SignalMonitor monitor;
    SignalModifyMonitor modify_monitor;
} Signal;

typedef struct {
    HANDLE signal;
    SignalCode signal_code;
    Signal signal_data;
    GAsyncQueue *answer;
} WinFam;

/* Single WinFam instance */
static WinFam *fam_instance = NULL;
// static GMutex *fam_mutex = NULL;

static gpointer fam_thread_main (WinFam *fam);

/* checks if fam is started */
static gboolean win_fam_start                   (GError            **error)
{
    GThread *thread;

    if (fam_instance != NULL)
        return TRUE;

    fam_instance = g_new0 (WinFam, 1);

    fam_instance->signal = CreateEvent (NULL, FALSE, FALSE, NULL);

    if (!fam_instance->signal)
    {
        int err = GetLastError ();
        char *msg = g_win32_error_message (err);
        g_set_error (error, MOO_FILE_WATCH_ERROR,
                     MOO_FILE_WATCH_ERROR_FAILED,
                     "CreateEvent: %s", msg);
        g_free (msg);
        goto error;
    }

    fam_instance->answer = g_async_queue_new ();

    thread = g_thread_create ((GThreadFunc) fam_thread_main,
                               fam_instance, FALSE, error);
    if (!thread)
        goto error;

    return TRUE;

error:
    if (fam_instance)
    {
        if (fam_instance->signal)
            CloseHandle (fam_instance->signal);
        if (fam_instance->answer)
            g_async_queue_unref (fam_instance->answer);
        g_free (fam_instance);
        fam_instance = NULL;
    }

    return FALSE;
}


static gboolean win_fam_signal  (GError **error)
{
    g_assert (fam_instance != NULL);

    if (!SetEvent (fam_instance->signal))
    {
        int err = GetLastError ();
        char *msg = g_win32_error_message (err);
        fam_instance->signal_code = 0;
        g_set_error (error, MOO_FILE_WATCH_ERROR,
                     MOO_FILE_WATCH_ERROR_FAILED,
                     "SetEvent: %s", msg);
        g_free (msg);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


static WinFamConnection *win_fam_open       (GError            **error)
{
    if (!win_fam_start (error))
        return NULL;

    fam_instance->signal_code = SIGNAL_NEW_CONNECTION;
    fam_instance->signal_data.new.error = error;

    if (!win_fam_signal (error))
        return NULL;

    return g_async_queue_pop (fam_instance->answer);
}


static gboolean          win_fam_close      (WinFamConnection   *fc,
                                             GError            **error)
{
    gpointer result;

    g_return_val_if_fail (fc != NULL, FALSE);
    g_return_val_if_fail (fam_instance != NULL, FALSE);

    fam_instance->signal_code = SIGNAL_CLOSE_CONNECTION;
    fam_instance->signal_data.close.error = error;
    fam_instance->signal_data.close.fc = fc;

    if (!win_fam_signal (error))
        return FALSE;

    result = g_async_queue_pop (fam_instance->answer);
    return GPOINTER_TO_INT (result);
}


static gboolean win_fam_monitor_directory   (WinFamConnection   *fc,
                                             const char         *filename,
                                             int                *request,
                                             gpointer            user_data,
                                             GError            **error)
{
    gpointer result;

    g_return_val_if_fail (fc != NULL, FALSE);
    g_return_val_if_fail (fam_instance != NULL, FALSE);

    fam_instance->signal_code = SIGNAL_MONITOR;
    fam_instance->signal_data.monitor.error = error;
    fam_instance->signal_data.monitor.fc = fc;
    fam_instance->signal_data.monitor.filename = filename;
    fam_instance->signal_data.monitor.user_data = user_data;

    if (!win_fam_signal (error))
        return FALSE;

    result = g_async_queue_pop (fam_instance->answer);

    if (result)
    {
        if (request)
            *request = fam_instance->signal_data.monitor.request;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


static gboolean win_fam_modify_monitor      (WinFamConnection   *fc,
                                             int                 request,
                                             SignalCode          code,
                                             GError            **error)
{
    gpointer result;

    g_return_val_if_fail (fc != NULL, FALSE);
    g_return_val_if_fail (fam_instance != NULL, FALSE);

    fam_instance->signal_code = code;
    fam_instance->signal_data.modify_monitor.error = error;
    fam_instance->signal_data.modify_monitor.fc = fc;
    fam_instance->signal_data.modify_monitor.request = request;

    if (!win_fam_signal (error))
        return FALSE;

    result = g_async_queue_pop (fam_instance->answer);
    return GPOINTER_TO_INT (result);
}


static gboolean win_fam_suspend_monitor     (WinFamConnection   *fc,
                                             int                 request,
                                             GError            **error)
{
    return win_fam_modify_monitor (fc, request, SIGNAL_SUSPEND_MONITOR, error);
}


static gboolean win_fam_resume_monitor      (WinFamConnection   *fc,
                                             int                 request,
                                             GError            **error)
{
    return win_fam_modify_monitor (fc, request, SIGNAL_RESUME_MONITOR, error);
}


static gboolean win_fam_cancel_monitor      (WinFamConnection   *fc,
                                             int                 request,
                                             GError            **error)
{
    return win_fam_modify_monitor (fc, request, SIGNAL_CANCEL_MONITOR, error);
}


/*****************************************************************************/
/* Monitoring thread
 */

typedef struct {
    GHashTable *handles;
    GSList *connections;
    GPtrArray *objects;
} WinFamData;

static void     fam_thread_process_signal   (WinFam     *fam,
                                             WinFamData *data);
static void     fam_thread_process_event    (WinFam     *fam,
                                             WinFamData *data,
                                             int         obj_no);

static gpointer fam_thread_main             (WinFam *fam)
{
    WinFamData data;

    data.objects = g_ptr_array_new ();
    g_ptr_array_add (data.objects, fam->signal);

    data.handles = g_hash_table_new (g_direct_hash, g_direct_equal);
    data.connections = NULL;

    while (TRUE)
    {
        int ret = WaitForMultipleObjects (data.objects->len,
                                          data.objects->pdata,
                                          FALSE, INFINITE);

        if (ret == WAIT_FAILED)
        {
            /* die */
            guint i;
            for (i = 1; i < data.objects->len; ++i)
                CloseHandle (data.objects->pdata[i]);
            g_ptr_array_set_size (data.objects, 1);
            return NULL;
        }

        if (ret >= WAIT_OBJECT_0 + data.objects->len)
        {
            /* ??? */
            guint i;
            for (i = 1; i < data.objects->len; ++i)
                CloseHandle (data.objects->pdata[i]);
            g_ptr_array_set_size (data.objects, 1);
            return NULL;
        }

        if (ret == WAIT_OBJECT_0)
            fam_thread_process_signal (fam, &data);
        else
            fam_thread_process_event (fam, &data, ret - WAIT_OBJECT_0);
    }
}


// typedef enum {
//     SIGNAL_NEW_CONNECTION   = 1,
//     SIGNAL_CLOSE_CONNECTION = 2,
//     SIGNAL_MONITOR          = 3,
//     SIGNAL_SUSPEND_MONITOR  = 4,
//     SIGNAL_RESUME_MONITOR   = 5,
//     SIGNAL_CANCEL_MONITOR   = 6
// } SignalCode;
//
// typedef struct {
//     GError **error;
// } SignalNew;
//
// typedef struct {
//     GError **error;
//     WinFamConnection *fc;
// } SignalClose;
//
// typedef struct {
//     GError **error;
//     MonitorType type;
//     WinFamConnection *fc;
//     const char *filename;
//     int *request;
//     gpointer user_data;
// } SignalMonitor;
//
// typedef struct {
//     GError **error;
//     WinFamConnection *fc;
//     int request;
// } SignalModifyMonitor;
//
// typedef union {
//     SignalNew new;
//     SignalClose close;
//     SignalMonitor monitor;
//     SignalModifyMonitor modify_monitor;
// } Signal;
//
// typedef struct {
//     HANDLE signal;
//     SignalCode signal_code;
//     Signal signal_data;
//     GAsyncQueue *answer;
// } WinFam;

static void     fam_thread_delete_handle    (WinFamData     *data,
                                             HANDLE          monitor);
static void     fam_thread_new_connection   (WinFamData     *data,
                                             SignalNew      *signal,
                                             GAsyncQueue    *answer);
static void     fam_thread_close_connection (WinFamData     *data,
                                             SignalClose    *signal,
                                             GAsyncQueue    *answer);
static void     fam_thread_create_monitor   (WinFamData     *data,
                                             SignalMonitor  *signal,
                                             GAsyncQueue    *answer);
static void     fam_thread_suspend_monitor  (WinFamData     *data,
                                             SignalModifyMonitor *signal,
                                             GAsyncQueue    *answer);
static void     fam_thread_resume_monitor   (WinFamData     *data,
                                             SignalModifyMonitor *signal,
                                             GAsyncQueue    *answer);
static void     fam_thread_cancel_monitor   (WinFamData     *data,
                                             SignalModifyMonitor *signal,
                                             GAsyncQueue    *answer);


static void     fam_thread_process_signal   (WinFam     *fam,
                                             WinFamData *data)
{
    switch (fam->signal_code)
    {
        case SIGNAL_NEW_CONNECTION:
            return fam_thread_new_connection (data, &fam->signal_data.new,
                                              fam->answer);

        case SIGNAL_CLOSE_CONNECTION:
            return fam_thread_close_connection (data, &fam->signal_data.close,
                                                fam->answer);

        case SIGNAL_MONITOR:
            return fam_thread_create_monitor (data, &fam->signal_data.monitor,
                                              fam->answer);

        case SIGNAL_SUSPEND_MONITOR:
            return fam_thread_suspend_monitor (data, &fam->signal_data.modify_monitor,
                                               fam->answer);

        case SIGNAL_RESUME_MONITOR:
            return fam_thread_resume_monitor (data, &fam->signal_data.modify_monitor,
                                              fam->answer);

        case SIGNAL_CANCEL_MONITOR:
            return fam_thread_cancel_monitor (data, &fam->signal_data.modify_monitor,
                                              fam->answer);

        default:
            g_warning ("%s: unknown signal code %d",
                       G_STRLOC, fam->signal_code);
            g_async_queue_push (fam->answer, NULL);
    }
}


static void     fam_thread_new_connection   (WinFamData     *data,
                                             SignalNew      *signal,
                                             GAsyncQueue    *answer)
{
    WinFamConnection *fc = g_new0 (WinFamConnection, 1);
    fc->events = g_async_queue_new ();
    data->connections = g_slist_prepend (data->connections, fc);
    g_clear_error (signal->error);
    g_async_queue_push (answer, fc);
}


static void     fam_thread_close_connection (WinFamData     *data,
                                             SignalClose    *signal,
                                             GAsyncQueue    *answer)
{
    guint i;
    GSList *list = NULL, *l;
    WinFamConnection *fc = signal->fc;

    if (!g_slist_find (data->connections, fc))
    {
        g_set_error (signal->error,
                     MOO_FILE_WATCH_ERROR,
                     MOO_FILE_WATCH_ERROR_FAILED,
                     "connection not found");
        g_async_queue_push (answer, GINT_TO_POINTER (FALSE));
        return;
    }

    data->connections = g_slist_remove (data->connections, fc);

    for (i = 1; i < data->objects->len; i++)
    {
        Request *req = g_hash_table_lookup (data->handles,
                                            data->objects->pdata[i]);
        g_assert (req != NULL);
        if (req->fc == fc)
            list = g_slist_prepend (list, data->objects->pdata[i]);
    }

    if (list)
    {
        g_warning ("%s: closing FAM connection with open requests",
                   G_STRLOC);

        for (l = list; l != NULL; l = l->next)
            fam_thread_delete_handle (data, l->data);
        g_slist_free (list);
    }

    while (g_async_queue_length (fc->events))
        win_fam_event_free (g_async_queue_pop (fc->events));

    g_async_queue_unref (fc->events);
    g_free (fc);
    g_clear_error (signal->error);
    g_async_queue_push (answer, GINT_TO_POINTER (TRUE));
}


static void     fam_thread_create_monitor   (WinFamData     *data,
                                             SignalMonitor  *signal,
                                             GAsyncQueue    *answer)
{
    HANDLE handle;
    Request *request;
    WinFamConnection *fc = signal->fc;

    if (!g_slist_find (data->connections, fc))
    {
        g_set_error (signal->error,
                     MOO_FILE_WATCH_ERROR,
                     MOO_FILE_WATCH_ERROR_FAILED,
                     "connection not found");
        g_async_queue_push (answer, GINT_TO_POINTER (FALSE));
        return;
    }

        /* TODO: this really sucks */
    if (data->objects->len == MAXIMUM_WAIT_OBJECTS)
    {
        g_set_error (signal->error,
                     MOO_FILE_WATCH_ERROR,
                     MOO_FILE_WATCH_ERROR_TOO_MANY,
                     "maximum number of monitored files reached");
        g_async_queue_push (answer, GINT_TO_POINTER (FALSE));
        return;
    }

    /* TODO: filename is utf8 */
    handle = FindFirstChangeNotification (signal->filename, FALSE,
                FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                        FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE);

    if (handle == INVALID_HANDLE_VALUE)
    {
        int err = GetLastError ();
        char *msg = g_win32_error_message (err);
        g_set_error (signal->error,
                     MOO_FILE_WATCH_ERROR,
                     MOO_FILE_WATCH_ERROR_FAILED,
                     "FindFirstChangeNotification: %s", msg);
        g_async_queue_push (answer, GINT_TO_POINTER (FALSE));
        g_free (msg);
        return;
    }

    request = g_new0 (Request, 1);
    request->fc = fc;
    request->reqnum = ++last_monitor_id;
    request->suspended = FALSE;
    request->user_data = signal->user_data;
    request->filename = g_strdup (signal->filename);

    g_hash_table_insert (data->handles, handle, request);
    g_ptr_array_add (data->objects, handle);

    g_clear_error (signal->error);
    signal->request = request->reqnum;

    g_async_queue_push (answer, GINT_TO_POINTER (TRUE));
}


static gboolean find_request                (WinFamData         *data,
                                             WinFamConnection   *fc,
                                             int                 reqnum,
                                             Request           **request,
                                             HANDLE             *handle)
{
    guint i;

    for (i = 1; i < data->objects->len; ++i)
    {
        HANDLE h = data->objects->pdata[i];
        Request *req = g_hash_table_lookup (data->handles, h);
        g_assert (req != NULL);
        if (req->fc == fc && req->reqnum == reqnum)
        {
            *request = req;
            *handle = h;
            return TRUE;
        }
    }

    return FALSE;
}


static void     fam_thread_suspend_monitor  (WinFamData     *data,
                                             SignalModifyMonitor *signal,
                                             GAsyncQueue    *answer)
{
    Request *request;
    HANDLE handle;

    if (!find_request (data, signal->fc, signal->request, &request, &handle))
    {
        g_set_error (signal->error,
                     MOO_FILE_WATCH_ERROR,
                     MOO_FILE_WATCH_ERROR_FAILED,
                     "No monitor with id %d",
                     signal->request);
        g_async_queue_push (answer, GINT_TO_POINTER (FALSE));
        return;
    }

    g_set_error (signal->error,
                 MOO_FILE_WATCH_ERROR,
                 MOO_FILE_WATCH_ERROR_NOT_IMPLEMENTED,
                 "suspend_monitor not implemented for folders");
    g_async_queue_push (answer, GINT_TO_POINTER (FALSE));
}


static void     fam_thread_resume_monitor   (WinFamData     *data,
                                             SignalModifyMonitor *signal,
                                             GAsyncQueue    *answer)
{
    Request *request;
    HANDLE handle;

    if (!find_request (data, signal->fc, signal->request, &request, &handle))
    {
        g_set_error (signal->error,
                     MOO_FILE_WATCH_ERROR,
                     MOO_FILE_WATCH_ERROR_FAILED,
                     "No monitor with id %d",
                     signal->request);
        g_async_queue_push (answer, GINT_TO_POINTER (FALSE));
        return;
    }

    g_set_error (signal->error,
                 MOO_FILE_WATCH_ERROR,
                 MOO_FILE_WATCH_ERROR_NOT_IMPLEMENTED,
                 "resume_monitor not implemented for folders");
    g_async_queue_push (answer, GINT_TO_POINTER (FALSE));
}


static void     fam_thread_cancel_monitor   (WinFamData     *data,
                                             SignalModifyMonitor *signal,
                                             GAsyncQueue    *answer)
{
    Request *request;
    HANDLE handle;

    if (!find_request (data, signal->fc, signal->request, &request, &handle))
    {
        g_set_error (signal->error,
                     MOO_FILE_WATCH_ERROR,
                     MOO_FILE_WATCH_ERROR_FAILED,
                     "No monitor with id %d",
                     signal->request);
        g_async_queue_push (answer, GINT_TO_POINTER (FALSE));
        return;
    }

    fam_thread_delete_handle (data, handle);

    g_clear_error (signal->error);
    g_async_queue_push (answer, GINT_TO_POINTER (TRUE));
}


static void     fam_thread_delete_handle    (WinFamData     *data,
                                             HANDLE          handle)
{
    guint i;
    Request *request;

    for (i = 1; i < data->objects->len; ++i)
        if (data->objects->pdata[i] == handle)
            break;

    g_return_if_fail (i < data->objects->len);

    g_ptr_array_remove_index (data->objects, i);

    request = g_hash_table_lookup (data->handles, handle);
    g_assert (request != NULL);
    g_free (request->filename);
    g_free (request);

    g_hash_table_remove (data->handles, handle);
}


#define SOME_NUMBER 16

static void     fam_thread_process_event    (WinFam     *fam,
                                             WinFamData *data,
                                             int         obj_no)
{
    HANDLE handle;
    Request *request;
    gboolean result;
//     FILE_NOTIFY_INFORMATION info[SOME_NUMBER];
//     DWORD bytes_returned;
    WinFamEvent *event;

    handle = data->objects->pdata[obj_no];
    request = g_hash_table_lookup (data->handles, handle);
    g_assert (request != NULL);

//     result = ReadDirectoryChangesW (request->dir_handle,
//                                     info, sizeof (info),
//                                     FALSE,
//                                     FILE_NOTIFY_CHANGE_FILE_NAME |
//                                             FILE_NOTIFY_CHANGE_DIR_NAME |
//                                             FILE_NOTIFY_CHANGE_SIZE |
//                                             FILE_NOTIFY_CHANGE_LAST_WRITE,
//                                     &bytes_returned, NULL, NULL);

//     if (!result)
//     {
//         g_warning ("%s: ReadDirectoryChangesW failed", G_STRLOC);
//         fam_thread_delete_handle (data, handle);
//         return;
//     }

    if (!g_file_test (request->filename, G_FILE_TEST_IS_DIR))
    {
        g_message ("folder '%s' deleted", request->filename);
        event = win_fam_event_new (MOO_FILE_WATCH_DELETED, request);
        fam_thread_delete_handle (data, handle);
    }
    else
    {
        event = win_fam_event_new (MOO_FILE_WATCH_CHANGED, request);
        /* WIN32_XXX */
        result = FindNextChangeNotification (handle);
        if (!result)
        {
            g_warning ("%s: FindNextChangeNotification failed", G_STRLOC);
            fam_thread_delete_handle (data, handle);
        }
    }

    g_async_queue_push (request->fc->events, event);
}


static WinFamEvent *win_fam_event_new       (MooFileWatchEventCode   code,
                                             Request                *request)
{
    WinFamEvent *event = g_new (WinFamEvent, 1);
    event->code = code;
    event->monitor_id = request->reqnum;
    event->filename = g_strdup (request->filename);
    event->data = request->user_data;
    return event;
}


static void         win_fam_event_free      (WinFamEvent    *event)
{
    if (event)
    {
        g_free (event->filename);
        g_free (event);
    }
}


#endif /* __WIN32__ */
