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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef MOO_USE_FAM
#include <fam.h>
#endif

#ifdef __WIN32__
#include "mooutils/mooutils-thread.h"
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

#ifndef __WIN32__
#include <time.h>
#endif

#include <glib/gstdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
/* sys/stat.h macros */
#include "mooutils/mooutils-fs.h"
#include "mooutils/moofilewatch.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moocompat.h"


#if defined(__WIN32__) && 0
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
    "MOVED",
    "ERROR"
};

typedef struct _Monitor Monitor;

typedef enum {
    MONITOR_DIR,
    MONITOR_FILE
} MonitorType;

struct _Monitor {
    MooFileWatch *parent;
    MonitorType type;
    char *filename;
    gpointer user_data;
    int request;
    guint suspended : 1;
    struct stat statbuf;
};

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
    void    (*delete)   (MooFileWatch   *watch,
                         Monitor        *monitor);
} WatchFuncs;


struct _MooFileWatchPrivate {
    guint stat_timeout;
#ifdef MOO_USE_FAM
    FAMConnection fam_connection;
    guint fam_connection_watch;
#endif
#ifdef __WIN32__
    guint id;
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
static void     monitor_fam_delete      (MooFileWatch   *watch,
                                         Monitor        *monitor);
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
static void     monitor_stat_delete     (MooFileWatch   *watch,
                                         Monitor        *monitor);

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
static void     monitor_win32_delete    (MooFileWatch   *watch,
                                         Monitor        *monitor);
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

static void
moo_file_watch_class_init (MooFileWatchClass *klass)
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
                          MOO_TYPE_FILE_WATCH_EVENT);

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


static void
moo_file_watch_init (MooFileWatch *watch)
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


static void
moo_file_watch_finalize (GObject *object)
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


GQuark
moo_file_watch_error_quark (void)
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
moo_file_watch_event_get_type (void)
{
    static GType type = 0;

    if (!type)
        type = g_pointer_type_register_static ("MooFileWatchEvent");

    return type;
}


GType
moo_file_watch_event_code_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GEnumValue values[] = {
            { MOO_FILE_WATCH_EVENT_CHANGED, (char*) "MOO_FILE_WATCH_EVENT_CHANGED", (char*) "changed"},
            { MOO_FILE_WATCH_EVENT_DELETED, (char*) "MOO_FILE_WATCH_EVENT_DELETED", (char*) "deleted"},
            { MOO_FILE_WATCH_EVENT_CREATED, (char*) "MOO_FILE_WATCH_EVENT_CREATED", (char*) "created"},
            { MOO_FILE_WATCH_EVENT_MOVED, (char*) "MOO_FILE_WATCH_EVENT_MOVED", (char*) "moved"},
            { MOO_FILE_WATCH_EVENT_ERROR, (char*) "MOO_FILE_WATCH_EVENT_ERROR", (char*) "error"},
            { 0, NULL, NULL }
        };

        type = g_enum_register_static ("MooFileWatchEventCode", values);
    }

    return type;
}


static int
moo_file_watch_create_monitor (MooFileWatch   *watch,
                               MonitorType     type,
                               const char     *filename,
                               gpointer        data,
                               GError        **error)
{
    Monitor *monitor;
    int request;

    g_return_val_if_fail (MOO_IS_FILE_WATCH (watch), FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);

    if (!watch->priv->alive)
    {
        g_set_error (error, MOO_FILE_WATCH_ERROR,
                     MOO_FILE_WATCH_ERROR_CLOSED,
                     "MooFileWatch is closed");
        return FALSE;
    }

    monitor = watch->priv->funcs.create (watch, type, filename, data, &request, error);

    if (!monitor)
        return 0;

    watch->priv->monitors = g_slist_prepend (watch->priv->monitors, monitor);
    g_hash_table_insert (watch->priv->requests, GINT_TO_POINTER (request), monitor);

    return request;
}


int
moo_file_watch_monitor_directory (MooFileWatch   *watch,
                                  const char     *filename,
                                  gpointer        data,
                                  GError        **error)
{
    return moo_file_watch_create_monitor (watch, MONITOR_DIR, filename, data, error);
}


int
moo_file_watch_monitor_file (MooFileWatch   *watch,
                             const char     *filename,
                             gpointer        data,
                             GError        **error)
{
    return moo_file_watch_create_monitor (watch, MONITOR_FILE, filename, data, error);
}


void
moo_file_watch_suspend_monitor (MooFileWatch   *watch,
                                int             monitor_id)
{
    Monitor *monitor;

    g_return_if_fail (MOO_IS_FILE_WATCH (watch));

    monitor = g_hash_table_lookup (watch->priv->requests,
                                   GINT_TO_POINTER (monitor_id));
    g_return_if_fail (monitor != NULL);

    watch->priv->funcs.suspend (monitor);
}


void
moo_file_watch_resume_monitor (MooFileWatch   *watch,
                               int             monitor_id)
{
    Monitor *monitor;

    g_return_if_fail (MOO_IS_FILE_WATCH (watch));

    monitor = g_hash_table_lookup (watch->priv->requests,
                                   GINT_TO_POINTER (monitor_id));
    g_return_if_fail (monitor != NULL);

    watch->priv->funcs.resume (monitor);
}


void
moo_file_watch_cancel_monitor (MooFileWatch   *watch,
                               int             monitor_id)
{
    Monitor *monitor;

    g_return_if_fail (MOO_IS_FILE_WATCH (watch));

    monitor = g_hash_table_lookup (watch->priv->requests,
                                   GINT_TO_POINTER (monitor_id));
    g_return_if_fail (monitor != NULL);

    watch->priv->funcs.delete (watch, monitor);
    watch->priv->monitors = g_slist_remove (watch->priv->monitors, monitor);
    g_hash_table_remove (watch->priv->requests,
                         GINT_TO_POINTER (monitor_id));
}


static void
emit_event (MooFileWatch       *watch,
            MooFileWatchEvent  *event)
{
    DEBUG_PRINT ("watch %p, monitor %d: event %s for %s",
                 watch, event->monitor_id,
                 event_code_strings[event->code],
                 event->filename ? event->filename : "<NULL>");
    g_signal_emit (watch, signals[EVENT], 0, event);
}


MooFileWatch *
moo_file_watch_new (GError **error)
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


gboolean
moo_file_watch_close (MooFileWatch   *watch,
                      GError        **error)
{
    g_return_val_if_fail (MOO_IS_FILE_WATCH (watch), FALSE);

    if (!watch->priv->alive)
        return TRUE;

    watch->priv->alive = FALSE;

    while (watch->priv->monitors)
    {
        watch->priv->funcs.delete (watch, watch->priv->monitors->data);
        watch->priv->monitors =
            g_slist_delete_link (watch->priv->monitors, watch->priv->monitors);
    }

    return watch->priv->funcs.shutdown (watch, error);
}


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

static gboolean
watch_fam_start (MooFileWatch   *watch,
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


static gboolean
watch_fam_shutdown (MooFileWatch   *watch,
                    GError        **error)
{
    if (watch->priv->fam_connection_watch)
        g_source_remove (watch->priv->fam_connection_watch);

    if (FAMClose (&watch->priv->fam_connection))
        RETURN_FAM_ERROR (FAMOpen, error, FALSE);
    else
        return TRUE;
}


static Monitor *
monitor_fam_create (MooFileWatch   *watch,
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


static void
monitor_fam_suspend (Monitor *monitor)
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


static void
monitor_fam_resume (Monitor *monitor)
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


static void
monitor_fam_delete (G_GNUC_UNUSED MooFileWatch *watch,
                    Monitor *monitor)
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


static gboolean
read_fam_events (G_GNUC_UNUSED GIOChannel *source,
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
        event.error = NULL;

        switch (fe.code)
        {
            case FAMChanged:
                event.code = MOO_FILE_WATCH_EVENT_CHANGED;
                break;
            case FAMDeleted:
                event.code = MOO_FILE_WATCH_EVENT_DELETED;
                break;
            case FAMCreated:
                event.code = MOO_FILE_WATCH_EVENT_CREATED;
                break;
            case FAMMoved:
                event.code = MOO_FILE_WATCH_EVENT_MOVED;
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


static gboolean
watch_stat_start (MooFileWatch   *watch,
                  G_GNUC_UNUSED GError **error)
{
    watch->priv->stat_timeout =
            g_timeout_add_full (MOO_STAT_PRIORITY,
                                MOO_STAT_TIMEOUT,
                                (GSourceFunc) do_stat,
                                watch, NULL);
    return TRUE;
}


static gboolean
watch_stat_shutdown (MooFileWatch   *watch,
                     G_GNUC_UNUSED GError **error)
{
    if (watch->priv->stat_timeout)
        g_source_remove (watch->priv->stat_timeout);
    watch->priv->stat_timeout = 0;
    return TRUE;
}


static Monitor *
monitor_stat_create (MooFileWatch   *watch,
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

    errno = 0;

    if (g_stat (filename, &buf) != 0)
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


static void
monitor_stat_suspend (Monitor *monitor)
{
    g_return_if_fail (monitor != NULL);
    monitor->suspended = TRUE;
}


static void
monitor_stat_resume (Monitor *monitor)
{
    g_return_if_fail (monitor != NULL);
    monitor->suspended = FALSE;
}


static void
monitor_stat_delete (G_GNUC_UNUSED MooFileWatch *watch,
                     Monitor *monitor)
{
    g_return_if_fail (monitor != NULL);
    DEBUG_PRINT ("removing monitor for '%s'", monitor->filename);
    g_free (monitor->filename);
    g_free (monitor);
}


static gboolean
do_stat (MooFileWatch *watch)
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

        errno = 0;

        if (g_stat (monitor->filename, &monitor->statbuf) != 0)
        {
            event.code = MOO_FILE_WATCH_EVENT_DELETED;
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
            event.code = MOO_FILE_WATCH_EVENT_CHANGED;
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


static MooFileWatchError
errno_to_file_error (int code)
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

typedef struct {
    guint watch_id;
    guint request;
    char *path;
} FAMThreadWatch;

typedef struct {
    GMutex *lock;
    GAsyncQueue *incoming;
    HANDLE events[MAXIMUM_WAIT_OBJECTS];
    FAMThreadWatch watches[MAXIMUM_WAIT_OBJECTS];
    guint n_events;
} FAMThread;

typedef enum {
    COMMAND_ADD_PATH,
    COMMAND_REMOVE_PATH
} FAMThreadCommandType;

typedef struct {
    FAMThreadCommandType type;
    char *path;
    guint watch_id;
    guint request;
} FAMThreadCommand;

typedef struct {
    FAMThread thread_data;
    guint event_id;
    gboolean running;
    GSList *watches;
} FAMWin32;

typedef struct {
    gboolean error;
    guint watch_id;
    guint request;
    guint code;
    char *msg;
} FAMEvent;

static FAMWin32 *fam;
static guint last_watch_id;

/****************************************************************************/
/* Watch thread
 */

static void
fam_event_free (FAMEvent *event)
{
    if (event)
    {
        g_free (event->msg);
        g_free (event);
    }
}

static void
fam_thread_event (guint       code,
                  gboolean    error,
                  const char *msg,
                  guint       watch_id,
                  guint       request)
{
    FAMEvent *event = g_new0 (FAMEvent, 1);
    event->error = error;
    event->code = code;
    event->watch_id = watch_id;
    event->request = request;
    event->msg = g_strdup (msg);
    _moo_event_queue_push (fam->event_id, event, (GDestroyNotify) fam_event_free);
}

static void
fam_thread_add_path (FAMThread  *thr,
                     const char *path,
                     guint       watch_id,
                     guint       request)
{
    gunichar2 *win_path;

    if (thr->n_events == MAXIMUM_WAIT_OBJECTS)
    {
        g_critical ("%s: too many folders watched", G_STRLOC);
        return;
    }

    win_path = g_utf8_to_utf16 (path, -1, NULL, NULL, NULL);

    if (!win_path)
    {
        char *msg = g_strdup_printf ("Could not convert filename '%s' to UTF16",
                                     path);
        fam_thread_event (MOO_FILE_WATCH_ERROR_BAD_FILENAME, TRUE, msg, watch_id, request);
        g_free (msg);
        return;
    }

    thr->events[thr->n_events] =
        FindFirstChangeNotificationW (win_path, FALSE,
                                      FILE_NOTIFY_CHANGE_FILE_NAME |
                                      FILE_NOTIFY_CHANGE_DIR_NAME);

    if (thr->events[thr->n_events] == INVALID_HANDLE_VALUE)
    {
        char *err = g_win32_error_message (GetLastError ());
        char *msg = g_strdup_printf ("Could not convert start watch for '%s': %s", path, err);
        fam_thread_event (MOO_FILE_WATCH_ERROR_FAILED, TRUE, msg, watch_id, request);
        g_free (msg);
        g_free (err);
        return;
    }

    thr->watches[thr->n_events].watch_id = watch_id;
    thr->watches[thr->n_events].request = request;
    thr->watches[thr->n_events].path = g_strdup (path);

    thr->n_events += 1;
}

static void
fam_thread_remove_path (FAMThread  *thr,
                        guint       watch_id,
                        guint       request)
{
    guint i;

    for (i = 1; i < thr->n_events; ++i)
    {
        if (thr->watches[i].watch_id == watch_id &&
            thr->watches[i].request == request)
        {
            FindCloseChangeNotification (thr->events[i]);
            g_free (thr->watches[i].path);

            if (i < thr->n_events - 1)
            {
                memmove (&thr->watches[i], &thr->watches[i+1],
                         (thr->n_events - i - 1) * sizeof(thr->watches[i]));
                memmove (&thr->events[i], &thr->events[i+1],
                         (thr->n_events - i - 1) * sizeof(thr->events[i]));
            }

            thr->n_events -= 1;

            return;
        }
    }

    g_critical ("%s: can't remove watch %d, request %d",
                G_STRLOC, watch_id, request);
}

static void
fam_thread_check_dir (FAMThread *thr,
                      guint      idx)
{
    struct stat buf;

    errno = 0;

    if (g_stat (thr->watches[idx].path, &buf) != 0 &&
        errno == ENOENT)
    {
        fam_thread_event (MOO_FILE_WATCH_EVENT_DELETED,
                          FALSE, NULL,
                          thr->watches[idx].watch_id,
                          thr->watches[idx].request);
        fam_thread_remove_path (thr,
                                thr->watches[idx].watch_id,
                                thr->watches[idx].request);
    }
    else
    {
        fam_thread_event (MOO_FILE_WATCH_EVENT_CHANGED,
                          FALSE, NULL,
                          thr->watches[idx].watch_id,
                          thr->watches[idx].request);

        if (!FindNextChangeNotification (thr->events[idx]))
        {
            char *err = g_win32_error_message (GetLastError ());
            char *msg = g_strdup_printf ("Error in FindNextChangeNotification: %s", err);

            fam_thread_event (MOO_FILE_WATCH_ERROR_FAILED, TRUE, msg,
                              thr->watches[idx].watch_id,
                              thr->watches[idx].request);
            fam_thread_remove_path (thr,
                                    thr->watches[idx].watch_id,
                                    thr->watches[idx].request);

            g_free (msg);
            g_free (err);
        }
    }
}

static void
fam_thread_do_command (FAMThread *thr)
{
    FAMThreadCommand *cmd;
    GSList *list = NULL;

    DEBUG_PRINT ("fam_thread_do_command start");

    g_mutex_lock (thr->lock);
    while ((cmd = g_async_queue_try_pop (thr->incoming)))
        list = g_slist_prepend (list, cmd);
    ResetEvent (thr->events[0]);
    g_mutex_unlock (thr->lock);

    list = g_slist_reverse (list);

    while (list)
    {
        cmd = list->data;

        switch (cmd->type)
        {
            case COMMAND_ADD_PATH:
                fam_thread_add_path (thr, cmd->path, cmd->watch_id, cmd->request);
                break;

            case COMMAND_REMOVE_PATH:
                fam_thread_remove_path (thr, cmd->watch_id, cmd->request);
                break;
        }

        list = g_slist_delete_link (list, list);
        g_free (cmd->path);
        g_free (cmd);
    }

    DEBUG_PRINT ("fam_thread_do_command end");
}


static gpointer
fam_thread_main (FAMThread *thr)
{
    while (TRUE)
    {
        int ret;

        DEBUG_PRINT ("calling WaitForMultipleObjects for %d handles", thr->n_events);
        ret = WaitForMultipleObjects (thr->n_events, thr->events, FALSE, INFINITE);
        DEBUG_PRINT ("WaitForMultipleObjects returned %d", ret);

        if (ret == (int) WAIT_FAILED)
        {
            char *msg = g_win32_error_message (GetLastError ());
            g_critical ("%s: %s", G_STRLOC, msg);
            g_free (msg);
            break;
        }

        if (ret == WAIT_OBJECT_0)
            fam_thread_do_command (thr);
        else if (WAIT_OBJECT_0 < ret && ret < (int) thr->n_events)
            fam_thread_check_dir (thr, ret - WAIT_OBJECT_0);
        else
        {
            g_critical ("%s: oops", G_STRLOC);
            break;
        }
    }

    /* XXX cleanup */
    return NULL;
}

static void
fam_thread_command (FAMThreadCommandType type,
                    const char          *filename,
                    guint                watch_id,
                    guint                request)
{
    FAMThreadCommand *cmd;

    cmd = g_new0 (FAMThreadCommand, 1);
    cmd->type = type;
    cmd->path = g_strdup (filename);
    cmd->watch_id = watch_id;
    cmd->request = request;

    g_mutex_lock (fam->thread_data.lock);
    g_async_queue_push (fam->thread_data.incoming, cmd);
    SetEvent (fam->thread_data.events[0]);
    g_mutex_unlock (fam->thread_data.lock);
}

/****************************************************************************/
/* Monitors
 */

static MooFileWatch *
find_watch (guint id)
{
    GSList *l;

    for (l = fam->watches; l != NULL; l = l->next)
    {
        MooFileWatch *watch = l->data;
        if (watch->priv->id == id)
            return watch;
    }

    return NULL;
}

static void
do_event (FAMEvent *event)
{
    MooFileWatch *watch;
    Monitor *monitor;
    MooFileWatchEvent watch_event;

    watch = find_watch (event->watch_id);

    if (!watch)
    {
        DEBUG_PRINT ("got event for dead watch %d", event->watch_id);
        return;
    }

    monitor = g_hash_table_lookup (watch->priv->requests,
                                   GUINT_TO_POINTER (event->request));

    if (!monitor)
    {
        DEBUG_PRINT ("got event for dead monitor %d", event->request);
        return;
    }

    watch_event.monitor_id = event->request;
    watch_event.filename = monitor->filename;
    watch_event.data = monitor->user_data;
    watch_event.error = NULL;

    if (event->error)
    {
        watch_event.code = MOO_FILE_WATCH_EVENT_ERROR;
        g_set_error (&watch_event.error, MOO_FILE_WATCH_ERROR,
                     event->code,
                     event->msg ? event->msg : "FAILED");
        DEBUG_PRINT ("got error for watch %d: %s", event->watch_id, watch_event.error->message);
    }
    else
    {
        DEBUG_PRINT ("got event for filename %s", monitor->filename);
        watch_event.code = event->code;
    }

    emit_event (watch, &watch_event);

    if (watch_event.error)
        g_error_free (watch_event.error);
}

static void
event_callback (GList *events)
{
    GList *trimmed = NULL;

    while (events)
    {
        GList *l;
        FAMEvent *event = events->data;
        gboolean found = FALSE;

        event = events->data;
        events = events->next;

        for (l = trimmed; l != NULL; l = l->next)
        {
            FAMEvent *old_event = l->data;

            if (old_event->watch_id == event->watch_id &&
                old_event->request == event->request)
            {
                found = TRUE;

                if (!old_event->error && event->error)
                    l->data = event;

                break;
            }
        }

        if (!found)
            trimmed = g_list_prepend (trimmed, event);
    }

    trimmed = g_list_reverse (trimmed);

    while (trimmed)
    {
        do_event (trimmed->data);
        trimmed = g_list_delete_link (trimmed, trimmed);
    }
}

static gboolean
fam_win32_init (void)
{
    if (!fam)
    {
        GError *error = NULL;

        fam = g_new0 (FAMWin32, 1);
        fam->running = FALSE;

        fam->thread_data.n_events = 1;
        fam->thread_data.events[0] = CreateEvent (NULL, TRUE, FALSE, NULL);;

        if (!fam->thread_data.events[0])
        {
            char *msg = g_win32_error_message (GetLastError ());
            g_critical ("%s: could not create incoming event", G_STRLOC);
            g_critical ("%s: %s", G_STRLOC, msg);
            g_free (msg);
            return FALSE;
        }

        fam->thread_data.incoming = g_async_queue_new ();
        fam->thread_data.lock = g_mutex_new ();
        fam->event_id = _moo_event_queue_connect ((MooEventQueueCallback) event_callback,
                                                  NULL, NULL);

        if (!g_thread_create ((GThreadFunc) fam_thread_main, &fam->thread_data, FALSE, &error))
        {
            g_critical ("%s: could not start watch thread", G_STRLOC);
            g_critical ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }
        else
        {
            fam->running = TRUE;
            DEBUG_PRINT ("initialized folder watch");
        }
    }

    return fam->running;
}

static gboolean
watch_win32_start (MooFileWatch   *watch,
                   GError        **error)
{
    if (!watch_stat_start (watch, error))
        return FALSE;

    if (!fam_win32_init ())
    {
        g_set_error (error, MOO_FILE_WATCH_ERROR,
                     MOO_FILE_WATCH_ERROR_FAILED,
                     "Could not initialize folder watch thread");
        watch_stat_shutdown (watch, NULL);
        return FALSE;
    }

    watch->priv->id = ++last_watch_id;
    fam->watches = g_slist_prepend (fam->watches, watch);
    DEBUG_PRINT ("started watch %d", watch->priv->id);

    return TRUE;
}


static gboolean
watch_win32_shutdown (MooFileWatch   *watch,
                      GError        **error)
{
    return watch_stat_shutdown (watch, error);
}


static Monitor *
monitor_win32_create (MooFileWatch   *watch,
                      MonitorType     type,
                      const char     *filename,
                      gpointer        data,
                      int            *request,
                      GError        **error)
{
    Monitor *monitor;
    struct stat buf;

    g_return_val_if_fail (MOO_IS_FILE_WATCH (watch), NULL);
    g_return_val_if_fail (filename != NULL, FALSE);

    if (type == MONITOR_FILE)
        return monitor_stat_create (watch, type, filename, data, request, error);

    errno = 0;

    if (g_stat (filename, &buf) != 0)
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

    if (request)
        *request = monitor->request;

    fam_thread_command (COMMAND_ADD_PATH, filename, watch->priv->id, monitor->request);

    return monitor;
}


static void
monitor_win32_suspend (Monitor *monitor)
{
    g_return_if_fail (monitor != NULL);

    if (monitor->suspended)
        return;

    if (monitor->type == MONITOR_FILE)
    {
        monitor_stat_suspend (monitor);
        return;
    }

    g_critical ("%s: implement me", G_STRFUNC);
}


static void
monitor_win32_resume (Monitor *monitor)
{
    g_return_if_fail (monitor != NULL);

    if (!monitor->suspended)
        return;

    if (monitor->type == MONITOR_FILE)
    {
        monitor_stat_resume (monitor);
        return;
    }

    g_critical ("%s: implement me", G_STRFUNC);
}


static void
monitor_win32_delete (MooFileWatch *watch,
                      Monitor      *monitor)
{
    g_return_if_fail (monitor != NULL);

    if (monitor->type == MONITOR_FILE)
    {
        monitor_stat_delete (watch, monitor);
        return;
    }

    DEBUG_PRINT ("removing monitor for '%s'", monitor->filename);
    fam_thread_command (COMMAND_REMOVE_PATH, monitor->filename, watch->priv->id, monitor->request);
    g_free (monitor->filename);
    g_free (monitor);
}
#endif /* __WIN32__ */
