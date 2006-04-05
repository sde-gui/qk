/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   moofile.c
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

/*
 *  Icon handling code was copied (and modified) from gtk/gtkfilesystemunix.c,
 *  Copyright (C) 2003, Red Hat, Inc.
 *
 *  g_utf8_collate_key_for_filename is taken from glib/gunicollate.c
 *  Copyright 2001,2005 Red Hat, Inc.
 *
 *  TODO!!! fix this mess
 */

#define MOO_FILE_SYSTEM_COMPILATION
#include "mooutils/moofileview/moofilesystem.h"
#include "mooutils/moofileview/symlink.h"
#include MOO_MARSHALS_H
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <gtk/gtkicontheme.h>
#include <gtk/gtkiconfactory.h>
#include <gtk/gtkstock.h>

#ifndef __WIN32__
#include "mooutils/xdgmime/xdgmime.h"
#endif

#if !GLIB_CHECK_VERSION(2,8,0)
static gchar *g_utf8_collate_key_for_filename   (const gchar *str,
                                                 gssize       len);
#endif


#define NORMAL_PRIORITY         G_PRIORITY_DEFAULT_IDLE
#define NORMAL_TIMEOUT          0.04
#define BACKGROUND_PRIORITY     G_PRIORITY_LOW
#define BACKGROUND_TIMEOUT      0.001

#define TIMER_CLEAR(timer)  \
G_STMT_START {              \
    g_timer_start (timer);  \
    g_timer_stop (timer);   \
} G_STMT_END

#if 0
#define PRINT_TIMES g_print
#else
static void PRINT_TIMES (G_GNUC_UNUSED const char *format, ...)
{
}
#endif

typedef enum {
    STAGE_NAMES     = 1,
    STAGE_STAT      = 2,
    STAGE_MIME_TYPE = 3
} Stage;

typedef struct {
    double names_timer;
    double stat_timer;
    guint stat_counter;
    double icons_timer;
    guint icons_counter;
} Debug;

struct _MooFolderPrivate {
    guint deleted : 1;
    Stage done;
    Stage wanted;
    Stage wanted_bg;
    MooFileSystem *fs;
    GDir *dir;
    GHashTable *files; /* basename -> MooFile* */
    GSList *files_copy;
    char *path;
    GSourceFunc populate_func;
    int populate_priority;
    guint populate_idle_id;
    double populate_timeout;
    Debug debug;
    GTimer *timer;
    MooFileWatch *fam;
    int fam_request;
    guint reload_idle;
};


static void     moo_folder_finalize         (GObject    *object);

static void     moo_folder_deleted          (MooFolder  *folder);

static void     folder_emit_deleted         (MooFolder  *folder);
static void     folder_emit_files           (MooFolder  *folder,
                                             guint       signal,
                                             GSList     *files);

static gboolean moo_folder_reload           (MooFolder  *folder);

static void     stop_populate               (MooFolder  *folder);

// static GSList  *files_list_copy             (GSList     *list);
static void     files_list_free             (GSList    **list);

static gboolean get_icons_a_bit             (MooFolder  *folder);
static gboolean get_stat_a_bit              (MooFolder  *folder);
static double   get_names                   (MooFolder  *folder);

static guint8   get_icon                    (MooFile        *file,
                                             const char     *dirname);
static guint8   get_blank_icon              (void);
static GdkPixbuf *render_icon               (const MooFile  *file,
                                             GtkWidget      *widget,
                                             GtkIconSize     size);
static GdkPixbuf *render_icon_for_path      (const char     *path,
                                             GtkWidget      *widget,
                                             GtkIconSize     size);

#define FILE_PATH(folder,file)  g_build_filename (folder->priv->path, file->name, NULL)
#define MAKE_PATH(dirname,file) g_build_filename (dirname, file->name, NULL)

static void start_monitor   (MooFolder  *folder);
static void stop_monitor    (MooFolder  *folder);

static GSList *hash_table_to_file_list  (GHashTable *files);
static void diff_hash_tables    (GHashTable *table1,
                                 GHashTable *table2,
                                 GSList    **only_1,
                                 GSList    **only_2);

static MooFile  *moo_file_new               (const char     *dirname,
                                             const char     *basename);
// #ifdef __WIN32__
// #define moo_file_stat moo_file_stat_win32
// static void      moo_file_stat_win32        (MooFile        *file,
//                                              const char     *dirname);
// #else
#define moo_file_stat moo_file_stat_unix
static void      moo_file_stat_unix         (MooFile        *file,
                                             const char     *dirname);
// #endif


/* MOO_TYPE_FOLDER */
G_DEFINE_TYPE (MooFolder, moo_folder, G_TYPE_OBJECT)

enum {
    DELETED,
    FILES_ADDED,
    FILES_REMOVED,
    FILES_CHANGED,
    NUM_SIGNALS
};

static guint signals[NUM_SIGNALS];

static void moo_folder_class_init (MooFolderClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_folder_finalize;

    klass->deleted = moo_folder_deleted;

    signals[DELETED] =
            g_signal_new ("deleted",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_FIRST,
                          G_STRUCT_OFFSET (MooFolderClass, deleted),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[FILES_ADDED] =
            g_signal_new ("files-added",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_FIRST,
                          G_STRUCT_OFFSET (MooFolderClass, files_added),
                          NULL, NULL,
                          _moo_marshal_VOID__POINTER,
                          G_TYPE_NONE, 1,
                          G_TYPE_POINTER);

    signals[FILES_REMOVED] =
            g_signal_new ("files-removed",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_FIRST,
                          G_STRUCT_OFFSET (MooFolderClass, files_removed),
                          NULL, NULL,
                          _moo_marshal_VOID__POINTER,
                          G_TYPE_NONE, 1,
                          G_TYPE_POINTER);

    signals[FILES_CHANGED] =
            g_signal_new ("files-changed",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_FIRST,
                          G_STRUCT_OFFSET (MooFolderClass, files_changed),
                          NULL, NULL,
                          _moo_marshal_VOID__POINTER,
                          G_TYPE_NONE, 1,
                          G_TYPE_POINTER);
}


static void moo_folder_init      (MooFolder    *folder)
{
    folder->priv = g_new0 (MooFolderPrivate, 1);
    folder->priv->deleted = FALSE;
    folder->priv->done = 0;
    folder->priv->wanted = 0;
    folder->priv->fs = NULL;
    folder->priv->dir = NULL;
    folder->priv->files_copy = NULL;
    folder->priv->path = NULL;
    folder->priv->populate_func = NULL;
    folder->priv->populate_idle_id = 0;
    folder->priv->populate_timeout = BACKGROUND_TIMEOUT;
    folder->priv->timer = g_timer_new ();
    g_timer_stop (folder->priv->timer);

    folder->priv->files =
            g_hash_table_new_full (g_str_hash, g_str_equal,
                                   g_free, (GDestroyNotify) moo_file_unref);
}


static void moo_folder_finalize  (GObject      *object)
{
    MooFolder *folder = MOO_FOLDER (object);

    if (folder->priv->files)
        g_hash_table_destroy (folder->priv->files);

    files_list_free (&folder->priv->files_copy);
    if (folder->priv->dir)
        g_dir_close (folder->priv->dir);
    g_free (folder->priv->path);
    if (folder->priv->populate_idle_id)
        g_source_remove (folder->priv->populate_idle_id);
    g_timer_destroy (folder->priv->timer);
    if (folder->priv->reload_idle)
        g_source_remove (folder->priv->reload_idle);

    g_free (folder->priv);
    folder->priv = NULL;

    G_OBJECT_CLASS (moo_folder_parent_class)->finalize (object);
}


MooFolder   *moo_folder_new             (MooFileSystem  *fs,
                                         const char     *path,
                                         MooFileFlags    wanted,
                                         GError        **error)
{
    GDir *dir;
    MooFolder *folder;
    GError *file_error = NULL;

    g_return_val_if_fail (MOO_IS_FILE_SYSTEM (fs), NULL);
    g_return_val_if_fail (path != NULL, NULL);

    dir = g_dir_open (path, 0, &file_error);

    if (!dir)
    {
        if (file_error->domain != G_FILE_ERROR)
        {
            g_set_error (error, MOO_FILE_ERROR,
                         MOO_FILE_ERROR_FAILED,
                         "%s", file_error->message);
        }
        else switch (file_error->code)
        {
            case G_FILE_ERROR_NOENT:
                g_set_error (error, MOO_FILE_ERROR,
                             MOO_FILE_ERROR_NONEXISTENT,
                             "%s", file_error->message);
                break;
            case G_FILE_ERROR_NOTDIR:
                g_set_error (error, MOO_FILE_ERROR,
                             MOO_FILE_ERROR_NOT_FOLDER,
                             "%s", file_error->message);
                break;
            case G_FILE_ERROR_NAMETOOLONG:
            case G_FILE_ERROR_LOOP:
                g_set_error (error, MOO_FILE_ERROR,
                             MOO_FILE_ERROR_BAD_FILENAME,
                             "%s", file_error->message);
                break;
            default:
                g_set_error (error, MOO_FILE_ERROR,
                             MOO_FILE_ERROR_FAILED,
                             "%s", file_error->message);
                break;
        }

        g_error_free (file_error);
        return NULL;
    }

    folder = g_object_new (MOO_TYPE_FOLDER, NULL);
    folder->priv->fs = fs;
    folder->priv->path = g_strdup (path);

    folder->priv->dir = dir;

    get_names (folder);
    moo_folder_set_wanted (folder, wanted, TRUE);

    return folder;
}


static void      moo_folder_deleted         (MooFolder  *folder)
{
    stop_populate (folder);
    stop_monitor (folder);
    if (folder->priv->reload_idle)
        g_source_remove (folder->priv->reload_idle);
    folder->priv->reload_idle = 0;
    folder->priv->deleted = TRUE;
    g_hash_table_destroy (folder->priv->files);
    folder->priv->files = NULL;
}


void         moo_folder_set_wanted      (MooFolder      *folder,
                                         MooFileFlags    wanted,
                                         gboolean        bit_now)
{
    Stage wanted_stage = STAGE_NAMES;

    g_return_if_fail (MOO_IS_FOLDER (folder));
    g_return_if_fail (!folder->priv->deleted);

    if (wanted & MOO_FILE_HAS_ICON)
        wanted_stage = STAGE_MIME_TYPE;
    else if (wanted & MOO_FILE_HAS_MIME_TYPE)
        wanted_stage = STAGE_MIME_TYPE;
    else if (wanted & MOO_FILE_HAS_STAT)
        wanted_stage = STAGE_STAT;

    if (wanted_stage <= folder->priv->done)
        return;

    if (folder->priv->wanted > folder->priv->done)
    {
        g_assert (folder->priv->populate_idle_id != 0);
        folder->priv->wanted = MAX (folder->priv->wanted, wanted_stage);
        return;
    }

    folder->priv->wanted = wanted_stage;

    if (folder->priv->wanted_bg != 0)
    {
        g_assert (folder->priv->populate_idle_id != 0);
        g_assert (folder->priv->populate_func != NULL);
        g_assert (folder->priv->populate_priority != 0);
        g_source_remove (folder->priv->populate_idle_id);
    }
    else
    {
        g_assert (folder->priv->populate_idle_id == 0);

        switch (folder->priv->done)
        {
            case STAGE_NAMES:
                g_assert (folder->priv->dir == NULL);
                folder->priv->populate_func = (GSourceFunc) get_stat_a_bit;
                break;
            case STAGE_STAT:
                g_assert (folder->priv->dir == NULL);
                folder->priv->populate_func = (GSourceFunc) get_icons_a_bit;
                break;
            default:
                g_assert_not_reached ();
        }
    }

    folder->priv->wanted_bg = STAGE_MIME_TYPE;
    folder->priv->populate_timeout = NORMAL_TIMEOUT;
    folder->priv->populate_priority = NORMAL_PRIORITY;

    TIMER_CLEAR (folder->priv->timer);

    if (!bit_now ||
         folder->priv->populate_func (folder))
    {
        folder->priv->populate_idle_id =
                g_timeout_add_full (folder->priv->populate_priority,
                                    folder->priv->populate_timeout,
                                    folder->priv->populate_func,
                                    folder, NULL);
    }
}


static void      stop_populate              (MooFolder  *folder)
{
    if (folder->priv->populate_idle_id)
        g_source_remove (folder->priv->populate_idle_id);
    folder->priv->populate_idle_id = 0;
    folder->priv->populate_func = 0;
    if (folder->priv->dir)
        g_dir_close (folder->priv->dir);
    folder->priv->dir = NULL;
    files_list_free (&folder->priv->files_copy);
}


static void      folder_emit_deleted        (MooFolder  *folder)
{
    g_signal_emit (folder, signals[DELETED], 0);
}


static void      folder_emit_files          (MooFolder  *folder,
                                             guint       sig,
                                             GSList     *files)
{
    if (files)
        g_signal_emit (folder, signals[sig], 0, files);
}


GSList      *moo_folder_list_files      (MooFolder  *folder)
{
    g_return_val_if_fail (MOO_IS_FOLDER (folder), NULL);
    g_return_val_if_fail (!folder->priv->deleted, NULL);
    return hash_table_to_file_list (folder->priv->files);
}


static double   get_names               (MooFolder  *folder)
{
    GTimer *timer;
    GSList *added = NULL;
    const char *name;
    MooFile *file;
    double elapsed;

    g_assert (folder->priv->path != NULL);
    g_assert (folder->priv->dir != NULL);
    g_assert (g_hash_table_size (folder->priv->files) == 0);

    timer = g_timer_new ();

    file = moo_file_new (folder->priv->path, "..");
    file->flags = MOO_FILE_HAS_MIME_TYPE | MOO_FILE_HAS_ICON;
    file->info = MOO_FILE_INFO_EXISTS | MOO_FILE_INFO_IS_DIR;
    file->icon = get_icon (file, folder->priv->path);

    g_hash_table_insert (folder->priv->files, g_strdup (".."), file);
    added = g_slist_prepend (added, file);

    for (name = g_dir_read_name (folder->priv->dir);
         name != NULL;
         name = g_dir_read_name (folder->priv->dir))
    {
        file = moo_file_new (folder->priv->path, name);

        if (file)
        {
            file->icon = get_blank_icon ();
            g_hash_table_insert (folder->priv->files, g_strdup (name), file);
            added = g_slist_prepend (added, file);
        }
        else
        {
            g_critical ("%s: moo_file_new() failed for '%s'", G_STRLOC, name);
        }
    }

    folder_emit_files (folder, FILES_ADDED, added);
    g_slist_free (added);

    elapsed = folder->priv->debug.names_timer =
            g_timer_elapsed (timer, NULL);
    g_timer_destroy (timer);

    g_dir_close (folder->priv->dir);
    folder->priv->dir = NULL;

    folder->priv->done = STAGE_NAMES;

    PRINT_TIMES ("names folder %s: %f sec\n",
                 folder->priv->path,
                 folder->priv->debug.names_timer);

    return elapsed;
}


static gboolean get_stat_a_bit              (MooFolder  *folder)
{
    gboolean done = FALSE;
    double elapsed;

    g_assert (folder->priv->dir == NULL);
    g_assert (folder->priv->done == STAGE_NAMES);
    g_assert (folder->priv->path != NULL);

    elapsed = g_timer_elapsed (folder->priv->timer, NULL);
    g_timer_continue (folder->priv->timer);

    if (!folder->priv->files_copy)
        folder->priv->files_copy =
                hash_table_to_file_list (folder->priv->files);
    if (!folder->priv->files_copy)
        done = TRUE;

    while (!done)
    {
        GSList *changed = folder->priv->files_copy;
        MooFile *file = changed->data;
        folder->priv->files_copy =
                g_slist_remove_link (folder->priv->files_copy,
                                     folder->priv->files_copy);

        if (!(file->flags & MOO_FILE_HAS_STAT))
        {
            moo_file_stat (file, folder->priv->path);
            folder_emit_files (folder, FILES_CHANGED, changed);
        }
        else
        {
            moo_file_unref (file);
        }

        g_slist_free_1 (changed);

        if (!folder->priv->files_copy)
            done = TRUE;

        if (g_timer_elapsed (folder->priv->timer, NULL) > folder->priv->populate_timeout)
            break;
    }

    elapsed = g_timer_elapsed (folder->priv->timer, NULL) - elapsed;
    folder->priv->debug.stat_timer += elapsed;
    folder->priv->debug.stat_counter += 1;
    g_timer_stop (folder->priv->timer);

    if (!done)
    {
        TIMER_CLEAR (folder->priv->timer);
        return TRUE;
    }
    else
    {
        g_assert (folder->priv->files_copy == NULL);
        folder->priv->populate_idle_id = 0;

        PRINT_TIMES ("stat folder %s: %d iterations, %f sec\n",
                     folder->priv->path,
                     folder->priv->debug.stat_counter,
                     folder->priv->debug.stat_timer);

        folder->priv->done = STAGE_STAT;

        if (folder->priv->wanted >= STAGE_MIME_TYPE || folder->priv->wanted_bg >= STAGE_MIME_TYPE)
        {
            if (folder->priv->wanted >= STAGE_MIME_TYPE)
            {
                folder->priv->populate_priority = NORMAL_PRIORITY;
                folder->priv->populate_timeout = NORMAL_TIMEOUT;
            }
            else if (folder->priv->wanted_bg >= STAGE_MIME_TYPE)
            {
                folder->priv->populate_priority = BACKGROUND_PRIORITY;
                folder->priv->populate_timeout = BACKGROUND_TIMEOUT;
            }

            if (folder->priv->populate_idle_id)
                g_source_remove (folder->priv->populate_idle_id);
            folder->priv->populate_idle_id = 0;
            folder->priv->populate_func = (GSourceFunc) get_icons_a_bit;

            if (g_timer_elapsed (folder->priv->timer, NULL) < folder->priv->populate_timeout)
            {
                /* in this case we may block for as much as twice TIMEOUT, but usually
                   it allows stat and loading icons in one iteration */
                TIMER_CLEAR (folder->priv->timer);
                if (folder->priv->populate_func (folder))
                    folder->priv->populate_idle_id =
                            g_timeout_add_full (folder->priv->populate_priority,
                                                folder->priv->populate_timeout,
                                                folder->priv->populate_func,
                                                folder, NULL);
            }
            else
            {
                TIMER_CLEAR (folder->priv->timer);
                folder->priv->populate_idle_id =
                        g_timeout_add_full (folder->priv->populate_priority,
                                            folder->priv->populate_timeout,
                                            folder->priv->populate_func,
                                            folder, NULL);
            }
        }
        else
        {
            folder->priv->populate_func = NULL;
            folder->priv->populate_priority = 0;
            folder->priv->populate_timeout = 0;
        }

        return FALSE;
    }
}


static gboolean get_icons_a_bit             (MooFolder  *folder)
{
    gboolean done = FALSE;
    double elapsed;

    g_assert (folder->priv->dir == NULL);
    g_assert (folder->priv->done == STAGE_STAT);
    g_assert (folder->priv->path != NULL);

    elapsed = g_timer_elapsed (folder->priv->timer, NULL);
    g_timer_continue (folder->priv->timer);

    if (!folder->priv->files_copy)
        folder->priv->files_copy =
                hash_table_to_file_list (folder->priv->files);
    if (!folder->priv->files_copy)
        done = TRUE;

    while (!done)
    {
        GSList *changed = folder->priv->files_copy;
        MooFile *file = changed->data;

        folder->priv->files_copy =
                g_slist_remove_link (folder->priv->files_copy,
                                     changed);

#ifndef __WIN32__
        if (file->info & MOO_FILE_INFO_EXISTS &&
            !(file->flags & MOO_FILE_HAS_MIME_TYPE))
        {
            char *path = FILE_PATH (folder, file);

            if (file->flags & MOO_FILE_HAS_STAT)
                file->mime_type = xdg_mime_get_mime_type_for_file (path, &file->statbuf);
            else
                file->mime_type = xdg_mime_get_mime_type_for_file (path, NULL);

            file->flags |= MOO_FILE_HAS_MIME_TYPE;
            file->flags |= MOO_FILE_HAS_ICON;
            file->icon = get_icon (file, folder->priv->path);
            folder_emit_files (folder, FILES_CHANGED, changed);
            g_free (path);
        }
#endif

        moo_file_unref (file);
        g_slist_free (changed);

        if (!folder->priv->files_copy)
            done = TRUE;

        if (g_timer_elapsed (folder->priv->timer, NULL) > folder->priv->populate_timeout)
            break;
    }

    elapsed = g_timer_elapsed (folder->priv->timer, NULL) - elapsed;
    folder->priv->debug.icons_timer += elapsed;
    folder->priv->debug.icons_counter += 1;
    TIMER_CLEAR (folder->priv->timer);

    if (done)
    {
        PRINT_TIMES ("icons folder %s: %d iterations, %f sec\n",
                     folder->priv->path,
                     folder->priv->debug.icons_counter,
                     folder->priv->debug.icons_timer);

        g_assert (folder->priv->files_copy == NULL);
        folder->priv->populate_idle_id = 0;
        folder->priv->done = STAGE_MIME_TYPE;
        folder->priv->populate_func = NULL;
        folder->priv->populate_priority = 0;
        folder->priv->populate_timeout = 0;

        start_monitor (folder);
    }

    return !done;
}


MooFile     *moo_folder_get_file        (MooFolder  *folder,
                                         const char *basename)
{
    g_return_val_if_fail (MOO_IS_FOLDER (folder), NULL);
    g_return_val_if_fail (!folder->priv->deleted, NULL);
    g_return_val_if_fail (basename != NULL, NULL);
    return g_hash_table_lookup (folder->priv->files, basename);
}


char*
moo_folder_get_file_path (MooFolder *folder,
                          MooFile   *file)
{
    g_return_val_if_fail (MOO_IS_FOLDER (folder), NULL);
    g_return_val_if_fail (file != NULL, NULL);
    return FILE_PATH (folder, file);
}


char*
moo_folder_get_file_uri (MooFolder *folder,
                         MooFile   *file)
{
    char *path, *uri;

    g_return_val_if_fail (MOO_IS_FOLDER (folder), NULL);
    g_return_val_if_fail (file != NULL, NULL);

    path = FILE_PATH (folder, file);
    g_return_val_if_fail (path != NULL, NULL);

    uri = g_filename_to_uri (path, NULL, NULL);

    g_free (path);
    return uri;
}


MooFolder   *moo_folder_get_parent      (MooFolder      *folder,
                                         MooFileFlags    wanted)
{
    g_return_val_if_fail (MOO_IS_FOLDER (folder), NULL);
    g_return_val_if_fail (!folder->priv->deleted, NULL);
    return moo_file_system_get_parent_folder (folder->priv->fs,
                                              folder, wanted);
}


MooFileSystem *moo_folder_get_file_system       (MooFolder      *folder)
{
    g_return_val_if_fail (MOO_IS_FOLDER (folder), NULL);
    return folder->priv->fs;
}


/*****************************************************************************/
/* Monitoring
 */

static void file_deleted    (MooFolder      *folder,
                             const char     *name);
static void file_changed    (MooFolder      *folder,
                             const char     *name);
static void file_created    (MooFolder      *folder,
                             const char     *name);

static void fam_event       (MooFolder      *folder,
                             MooFileWatchEvent    *event)
{
    if (event->data != folder)
        return;

    g_return_if_fail (event->monitor_id == folder->priv->fam_request);

    switch (event->code)
    {
        case MOO_FILE_WATCH_CHANGED:
            file_changed (folder, event->filename);
            break;
        case MOO_FILE_WATCH_DELETED:
            file_deleted (folder, event->filename);
            break;
        case MOO_FILE_WATCH_CREATED:
            file_created (folder, event->filename);
            break;
        default:
            break;
    }
}


static void fam_error       (MooFolder      *folder,
                             GError         *error)
{
    g_print ("fam error: %s\n", error->message);
    stop_monitor (folder);
}


static void start_monitor   (MooFolder  *folder)
{
    GError *error = NULL;

    g_return_if_fail (!folder->priv->deleted);
    g_return_if_fail (folder->priv->fam_request == 0);
    folder->priv->fam = moo_file_system_get_file_watch (folder->priv->fs);
    g_return_if_fail (folder->priv->fam != NULL);

    if (!moo_file_watch_monitor_directory (folder->priv->fam,
                                           folder->priv->path,
                                           folder,
                                           &folder->priv->fam_request,
                                           &error))
    {
        g_warning ("%s: moo_fam_monitor_directory failed", G_STRLOC);
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
        return;
    }

    g_signal_connect_swapped (folder->priv->fam, "event",
                              G_CALLBACK (fam_event), folder);
    g_signal_connect_swapped (folder->priv->fam, "error",
                              G_CALLBACK (fam_error), folder);
}


static void stop_monitor    (MooFolder  *folder)
{
    if (folder->priv->fam_request)
    {
        g_signal_handlers_disconnect_by_func (folder->priv->fam,
                                              (gpointer)fam_event,
                                              folder);
        g_signal_handlers_disconnect_by_func (folder->priv->fam,
                                              (gpointer)fam_error,
                                              folder);

        moo_file_watch_cancel_monitor (folder->priv->fam,
                                       folder->priv->fam_request);

        folder->priv->fam = NULL;
        folder->priv->fam_request = 0;
    }
}


static void file_deleted    (MooFolder      *folder,
                             const char     *name)
{
    MooFile *file;
    GSList *list;

    g_return_if_fail (!folder->priv->deleted);

    if (!strcmp (name, folder->priv->path))
        return folder_emit_deleted (folder);

    file = g_hash_table_lookup (folder->priv->files, name);
    if (!file) return;

    moo_file_ref (file);
    g_hash_table_remove (folder->priv->files, name);

    list = g_slist_append (NULL, file);
    folder_emit_files (folder, FILES_REMOVED, list);

    g_slist_free (list);
    moo_file_unref (file);
}


static void file_changed    (MooFolder      *folder,
                             const char     *name)
{
    g_return_if_fail (!folder->priv->deleted);

    if (!strcmp (name, folder->priv->path))
    {
        if (!folder->priv->reload_idle)
            folder->priv->reload_idle =
                    g_idle_add ((GSourceFunc) moo_folder_reload,
                                folder);
    }
    else
    {
        /* TODO */
    }
}


static void file_created    (MooFolder      *folder,
                             const char     *name)
{
    MooFile *file;
    GSList *list;

    g_return_if_fail (!folder->priv->deleted);

    file = moo_file_new (folder->priv->path, name);
    g_return_if_fail (file != NULL);

    file->icon = get_icon (file, folder->priv->path);
    moo_file_stat (file, folder->priv->path);

#ifndef __WIN32__
    if (file->info & MOO_FILE_INFO_EXISTS &&
        !(file->flags & MOO_FILE_HAS_MIME_TYPE))
    {
        char *path = FILE_PATH (folder, file);

        if (file->flags & MOO_FILE_HAS_STAT)
            file->mime_type = xdg_mime_get_mime_type_for_file (path, &file->statbuf);
        else
            file->mime_type = xdg_mime_get_mime_type_for_file (path, NULL);

        file->flags |= MOO_FILE_HAS_MIME_TYPE;
        file->flags |= MOO_FILE_HAS_ICON;
        file->icon = get_icon (file, folder->priv->path);
        g_free (path);
    }
#endif

    g_hash_table_insert (folder->priv->files,
                         g_strdup (name), file);
    list = g_slist_append (NULL, file);
    folder_emit_files (folder, FILES_ADDED, list);

    g_slist_free (list);
}


/* TODO */
static gboolean moo_folder_reload           (MooFolder  *folder)
{
    GHashTable *files;
    GDir *dir;
    GError *error = NULL;
    const char *name;
    GSList *new = NULL, *deleted = NULL, *l;

    g_return_val_if_fail (!folder->priv->deleted, FALSE);
    folder->priv->reload_idle = 0;

    dir = g_dir_open (folder->priv->path, 0, &error);

    if (!dir)
    {
        g_warning ("%s: could not open directory %s",
                   G_STRLOC, folder->priv->path);
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
        folder_emit_deleted (folder);
        return FALSE;
    }

    files = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

    while ((name = g_dir_read_name (dir)))
        g_hash_table_insert (files, g_strdup (name), NULL);

    diff_hash_tables (files, folder->priv->files, &new, &deleted);

    for (l = new; l != NULL; l = l->next)
        file_created (folder, l->data);

    for (l = deleted; l != NULL; l = l->next)
        file_deleted (folder, l->data);

    g_slist_foreach (new, (GFunc) g_free, NULL);
    g_slist_foreach (deleted, (GFunc) g_free, NULL);
    g_slist_free (new);
    g_slist_free (deleted);
    g_hash_table_destroy (files);
    g_dir_close (dir);
    return FALSE;
}


/* XXX */
static char *moo_file_get_type_string   (MooFile        *file)
{
    g_return_val_if_fail (MOO_FILE_EXISTS (file), NULL);

    if (MOO_FILE_IS_DIR (file))
        return g_strdup ("folder");
    else if (file->mime_type)
        return g_strdup (file->mime_type);
    else
        return g_strdup ("file");
}


/* XXX */
static char *moo_file_get_size_string   (MooFile        *file)
{
    return g_strdup_printf ("%" G_GINT64_FORMAT, (MooFileSize) file->statbuf.st_size);
}


/* XXX */
static char *moo_file_get_mtime_string  (MooFile        *file)
{
    static char buf[1024];

    if (!MOO_FILE_EXISTS (file))
        return NULL;

#ifdef __WIN32__
    if (MOO_FILE_IS_DIR (file))
        return NULL;
#endif

    if (strftime (buf, 1024, "%x %X", localtime ((time_t*)&file->statbuf.st_mtime)))
        return g_strdup (buf);
    else
        return NULL;
}


char       **moo_folder_get_file_info   (MooFolder      *folder,
                                         MooFile        *file)
{
    GPtrArray *array;
    GSList *list;

    g_return_val_if_fail (MOO_IS_FOLDER (folder), NULL);
    g_return_val_if_fail (file != NULL, NULL);
    g_return_val_if_fail (!folder->priv->deleted, NULL);
    g_return_val_if_fail (folder->priv->files != NULL, NULL);
    g_return_val_if_fail (g_hash_table_lookup (folder->priv->files,
                                    moo_file_name (file)) == file, NULL);

    moo_file_stat (file, folder->priv->path);

#ifndef __WIN32__
    if (file->info & MOO_FILE_INFO_EXISTS &&
        !(file->flags & MOO_FILE_HAS_MIME_TYPE))
    {
        char *path = FILE_PATH (folder, file);

        if (file->flags & MOO_FILE_HAS_STAT)
            file->mime_type = xdg_mime_get_mime_type_for_file (path, &file->statbuf);
        else
            file->mime_type = xdg_mime_get_mime_type_for_file (path, NULL);

        file->flags |= MOO_FILE_HAS_MIME_TYPE;
        file->flags |= MOO_FILE_HAS_ICON;
        file->icon = get_icon (file, folder->priv->path);
        g_free (path);
    }
#endif

    array = g_ptr_array_new ();

    if (file->info & MOO_FILE_INFO_EXISTS)
    {
        char *type, *mtime, *location;

        g_ptr_array_add (array, g_strdup ("Type:"));
        type = moo_file_get_type_string (file);

        if (file->info & MOO_FILE_INFO_IS_LINK)
        {
            g_ptr_array_add (array, g_strdup_printf ("link to %s", type));
            g_free (type);
        }
        else
        {
            g_ptr_array_add (array, type);
        }

        location = g_filename_display_name (moo_folder_get_path (folder));
        g_ptr_array_add (array, g_strdup ("Location:"));
        g_ptr_array_add (array, location);

        if (!(file->info & MOO_FILE_INFO_IS_DIR))
        {
            g_ptr_array_add (array, g_strdup ("Size:"));
            g_ptr_array_add (array, moo_file_get_size_string (file));
        }

        mtime = moo_file_get_mtime_string (file);

        if (mtime)
        {
            g_ptr_array_add (array, g_strdup ("Modified:"));
            g_ptr_array_add (array, mtime);
        }
    }
    else if (file->info & MOO_FILE_INFO_IS_LINK)
    {
        g_ptr_array_add (array, g_strdup ("Type:"));
        g_ptr_array_add (array, g_strdup ("broken symbolic link"));
    }

#ifndef __WIN32__
    if ((file->info & MOO_FILE_INFO_IS_LINK) &&
         moo_file_link_get_target (file))
    {
        g_ptr_array_add (array, g_strdup ("Points to:"));
        g_ptr_array_add (array, g_strdup (moo_file_link_get_target (file)));
    }
#endif

    list = g_slist_append (NULL, moo_file_ref (file));
    g_object_ref (folder);
    folder_emit_files (folder, FILES_CHANGED, list);
    g_object_unref (folder);
    moo_file_unref (file);
    g_slist_free (list);

    g_ptr_array_add (array, NULL);
    return (char**) g_ptr_array_free (array, FALSE);
}


/********************************************************************/
/* MooFile
 */

MooFile     *moo_file_new               (const char     *dirname,
                                         const char     *basename)
{
    MooFile *file = NULL;
    char *path = NULL;
    char *display_name = NULL;

    g_return_val_if_fail (dirname != NULL, NULL);
    g_return_val_if_fail (basename && basename[0], NULL);

    path = g_build_filename (dirname, basename, NULL);
    g_return_val_if_fail (path != NULL, NULL);

    display_name = g_filename_display_basename (path);
    g_assert (g_utf8_validate (display_name, -1, NULL));

    if (!display_name)
    {
        g_free (path);
        g_return_val_if_fail (display_name != NULL, NULL);
    }

    file = g_new0 (MooFile, 1);
    file->ref_count = 1;

    file->name = g_strdup (basename);

    file->display_name = g_utf8_normalize (display_name, -1, G_NORMALIZE_ALL);
    file->case_display_name = g_utf8_casefold (file->display_name, -1);
    file->collation_key = g_utf8_collate_key_for_filename (file->display_name, -1);

    g_free (path);
    g_free (display_name);

#ifndef __WIN32__
    if (basename[0] == '.')
        file->info = MOO_FILE_INFO_IS_HIDDEN;
#endif

    return file;
}


MooFile     *moo_file_ref               (MooFile        *file)
{
    g_return_val_if_fail (file != NULL, NULL);
    file->ref_count++;
    return file;
}


void         moo_file_unref             (MooFile        *file)
{
    if (file && !--file->ref_count)
    {
        g_free (file->name);
        g_free (file->display_name);
        g_free (file->case_display_name);
        g_free (file->collation_key);
        g_free (file->link_target);
        g_free (file);
    }
}


#ifdef __WIN32__
#define lstat stat
#endif

static void      moo_file_stat_unix         (MooFile        *file,
                                             const char     *dirname)
{
    char *fullname;

    g_return_if_fail (file != NULL);

    fullname = g_build_filename (dirname, file->name, NULL);

    file->info = MOO_FILE_INFO_EXISTS;
    file->flags = MOO_FILE_HAS_STAT;

    g_free (file->link_target);
    file->link_target = NULL;

    if (lstat (fullname, &file->statbuf) != 0)
    {
        if (errno == ENOENT)
        {
            gchar *display_name = g_filename_display_name (fullname);
            g_message ("%s: file '%s' does not exist",
                       G_STRLOC, display_name);
            g_free (display_name);
            file->info = 0;
        }
        else
        {
            int save_errno = errno;
            gchar *display_name = g_filename_display_name (fullname);
            g_message ("%s: error getting information for '%s': %s",
                       G_STRLOC, display_name,
                       g_strerror (save_errno));
            g_free (display_name);
            file->info = MOO_FILE_INFO_IS_LOCKED | MOO_FILE_INFO_EXISTS;
            file->flags = 0;
        }
    }
    else
    {
#ifdef S_ISLNK
        if (S_ISLNK (file->statbuf.st_mode))
        {
            static char buf[1024];
            gssize len;

            file->info |= MOO_FILE_INFO_IS_LINK;

            if (stat (fullname, &file->statbuf) != 0)
            {
                if (errno == ENOENT)
                {
                    gchar *display_name = g_filename_display_name (fullname);
                    g_message ("%s: file '%s' is a broken link",
                               G_STRLOC, display_name);
                    g_free (display_name);
                    file->info = MOO_FILE_INFO_IS_LINK;
                }
                else
                {
                    int save_errno = errno;
                    gchar *display_name = g_filename_display_name (fullname);
                    g_message ("%s: error getting information for '%s': %s",
                               G_STRLOC, display_name,
                               g_strerror (save_errno));
                    g_free (display_name);
                    file->info = MOO_FILE_INFO_IS_LOCKED | MOO_FILE_INFO_EXISTS;
                    file->flags = 0;
                }
            }

            len = readlink (fullname, buf, 1024);

            if (len == -1)
            {
                int save_errno = errno;
                gchar *display_name = g_filename_display_name (fullname);
                g_message ("%s: error getting link target for '%s': %s",
                           G_STRLOC, display_name,
                           g_strerror (save_errno));
                g_free (display_name);
            }
            else
            {
                file->link_target = g_strndup (buf, len);
            }
        }
#endif
    }

    if ((file->info & MOO_FILE_INFO_EXISTS) &&
         !(file->info & MOO_FILE_INFO_IS_LOCKED))
    {
        if (S_ISDIR (file->statbuf.st_mode))
            file->info |= MOO_FILE_INFO_IS_DIR;
        else if (S_ISBLK (file->statbuf.st_mode))
            file->info |= MOO_FILE_INFO_IS_BLOCK_DEV;
        else if (S_ISCHR (file->statbuf.st_mode))
            file->info |= MOO_FILE_INFO_IS_CHAR_DEV;
#ifdef S_ISFIFO
        else if (S_ISFIFO (file->statbuf.st_mode))
            file->info |= MOO_FILE_INFO_IS_FIFO;
#endif
#ifdef S_ISSOCK
        else if (S_ISSOCK (file->statbuf.st_mode))
            file->info |= MOO_FILE_INFO_IS_SOCKET;
#endif
    }

    if (file->info & MOO_FILE_INFO_IS_DIR)
    {
        file->flags |= MOO_FILE_HAS_MIME_TYPE;
        file->flags |= MOO_FILE_HAS_ICON;
    }

    file->icon = get_icon (file, dirname);

    if (file->name[0] == '.')
        file->info |= MOO_FILE_INFO_IS_HIDDEN;

    g_free (fullname);
}


gboolean     moo_file_test              (const MooFile  *file,
                                         MooFileInfo     test)
{
    g_return_val_if_fail (file != NULL, FALSE);
    return file->info & test;
}


const char   *moo_file_display_name     (const MooFile  *file)
{
    g_return_val_if_fail (file != NULL, NULL);
    return file->display_name;
}

const char  *moo_file_collation_key     (const MooFile  *file)
{
    g_return_val_if_fail (file != NULL, NULL);
    return file->collation_key;
}

const char  *moo_file_case_display_name (const MooFile *file)
{
    g_return_val_if_fail (file != NULL, NULL);
    return file->case_display_name;
}

const char  *moo_file_name              (const MooFile  *file)
{
    g_return_val_if_fail (file != NULL, NULL);
    return file->name;
}


const char  *moo_file_get_mime_type         (const MooFile  *file)
{
    g_return_val_if_fail (file != NULL, NULL);
    return file->mime_type;
}


#ifndef __WIN32__
gconstpointer moo_file_get_stat             (const MooFile  *file)
{
    g_return_val_if_fail (file != NULL, NULL);
    if (file->flags & MOO_FILE_HAS_STAT && file->info & MOO_FILE_INFO_EXISTS)
        return &file->statbuf;
    else
        return NULL;
}
#endif


GdkPixbuf *
moo_file_get_icon (const MooFile  *file,
                   GtkWidget      *widget,
                   GtkIconSize     size)
{
    g_return_val_if_fail (file != NULL, NULL);
    g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);
    return render_icon (file, widget, size);
}


GdkPixbuf *
moo_get_icon_for_path (const char     *path,
                       GtkWidget      *widget,
                       GtkIconSize     size)
{
    g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);
    return render_icon_for_path (path, widget, size);
}


#ifndef __WIN32__
const char  *moo_file_link_get_target   (const MooFile  *file)
{
    return file->link_target;
}
#endif


MooFileTime  moo_file_get_mtime         (const MooFile  *file)
{
    g_return_val_if_fail (file != NULL, 0);
    return file->statbuf.st_mtime;
}


MooFileSize  moo_file_get_size          (const MooFile  *file)
{
    g_return_val_if_fail (file != NULL, 0);
    return file->statbuf.st_size;
}


GType        moo_file_get_type          (void)
{
    static GType type = 0;

    if (!type)
        type = g_boxed_type_register_static ("MooFile",
                                             (GBoxedCopyFunc) moo_file_ref,
                                             (GBoxedFreeFunc) moo_file_unref);

    return type;
}


GType        moo_file_flags_get_type    (void)
{
    static GType type = 0;

    static const GFlagsValue values[] = {
        { MOO_FILE_HAS_MIME_TYPE, (char*)"MOO_FILE_HAS_MIME_TYPE", (char*)"has-mime-type" },
        { MOO_FILE_HAS_ICON, (char*)"MOO_FILE_HAS_ICON", (char*)"has-icon" },
        { MOO_FILE_HAS_STAT, (char*)"MOO_FILE_HAS_STAT", (char*)"has-stat" },
        { MOO_FILE_ALL_FLAGS, (char*)"MOO_FILE_ALL_FLAGS", (char*)"all-flags" },
        { 0, NULL, NULL }
    };

    if (!type)
        type = g_flags_register_static ("MooFileFlags", values);

    return type;
}


GType        moo_file_info_get_type     (void)
{
    static GType type = 0;

    static const GFlagsValue values[] = {
        { MOO_FILE_INFO_EXISTS, (char*)"MOO_FILE_INFO_EXISTS", (char*)"exists" },
        { MOO_FILE_INFO_IS_DIR, (char*)"MOO_FILE_INFO_IS_DIR", (char*)"is-folder" },
        { MOO_FILE_INFO_IS_HIDDEN, (char*)"MOO_FILE_INFO_IS_HIDDEN", (char*)"is-hidden" },
        { MOO_FILE_INFO_IS_LINK, (char*)"MOO_FILE_INFO_IS_LINK", (char*)"is-link" },
        { 0, NULL, NULL }
    };

    if (!type)
        type = g_flags_register_static ("MooFileInfo", values);

    return type;
}


const char  *moo_folder_get_path        (MooFolder  *folder)
{
    g_return_val_if_fail (MOO_IS_FOLDER (folder), NULL);
    return folder->priv->path;
}


// static GSList  *files_list_copy             (GSList     *list)
// {
//     GSList *copy, *l;
//     for (copy = NULL, l = list; l != NULL; l = l->next)
//         copy = g_slist_prepend (copy, moo_file_ref (l->data));
//     return g_slist_reverse (copy);
// }


static void     files_list_free             (GSList    **list)
{
    g_slist_foreach (*list, (GFunc) moo_file_unref, NULL);
    g_slist_free (*list);
    *list = NULL;
}


static void prepend_file (G_GNUC_UNUSED gpointer key,
                          MooFile *file,
                          GSList **list)
{
    *list = g_slist_prepend (*list, moo_file_ref (file));
}

static GSList *hash_table_to_file_list      (GHashTable *files)
{
    GSList *list = NULL;
    g_return_val_if_fail (files != NULL, NULL);
    g_hash_table_foreach (files, (GHFunc) prepend_file, &list);
    return list;
}


static void check_unique        (const char *key,
                                 G_GNUC_UNUSED gpointer whatever,
                                 gpointer user_data)
{
    struct {
        GSList *list;
        GHashTable *table2;
    } *data = user_data;
    gpointer orig_key, value;

    if (!g_hash_table_lookup_extended (data->table2, key, &orig_key, &value))
        data->list = g_slist_prepend (data->list, g_strdup (key));
}

static void get_unique          (GHashTable *table1,
                                 GHashTable *table2,
                                 GSList    **only_1)
{
    struct {
        GSList *list;
        GHashTable *table2;
    } data = {NULL, table2};

    g_hash_table_foreach (table1, (GHFunc) check_unique, &data);
    *only_1 = data.list;
}

static void diff_hash_tables    (GHashTable *table1,
                                 GHashTable *table2,
                                 GSList    **only_1,
                                 GSList    **only_2)
{
    get_unique (table1, table2, only_1);
    get_unique (table2, table1, only_2);
}


#if !GLIB_CHECK_VERSION(2,8,0)
/* This is a collation key that is very very likely to sort before any
   collation key that libc strxfrm generates. We use this before any
   special case (dot or number) to make sure that its sorted before
   anything else.
 */
#define COLLATION_SENTINEL "\1\1\1"

/**
 * g_utf8_collate_key_for_filename:
 * @str: a UTF-8 encoded string.
 * @len: length of @str, in bytes, or -1 if @str is nul-terminated.
 *
 * Converts a string into a collation key that can be compared
 * with other collation keys produced by the same function using strcmp().
 *
 * In order to sort filenames correctly, this function treats the dot '.'
 * as a special case. Most dictionary orderings seem to consider it
 * insignificant, thus producing the ordering "event.c" "eventgenerator.c"
 * "event.h" instead of "event.c" "event.h" "eventgenerator.c". Also, we
 * would like to treat numbers intelligently so that "file1" "file10" "file5"
 * is sorted as "file1" "file5" "file10".
 *
 * Return value: a newly allocated string. This string should
 *   be freed with g_free() when you are done with it.
 *
 * Since: 2.8
 */
static gchar *g_utf8_collate_key_for_filename   (const gchar *str,
                                                 gssize       len)
{
    GString *result;
    GString *append;
    const gchar *p;
    const gchar *prev;
    gchar *collate_key;
    gint digits;
    gint leading_zeros;

  /*
   * How it works:
    *
    * Split the filename into collatable substrings which do
    * not contain [.0-9] and special-cased substrings. The collatable
    * substrings are run through the normal g_utf8_collate_key() and the
    * resulting keys are concatenated with keys generated from the
    * special-cased substrings.
    *
    * Special cases: Dots are handled by replacing them with '\1' which
    * implies that short dot-delimited substrings are before long ones,
    * e.g.
    *
    *   a\1a   (a.a)
    *   a-\1a  (a-.a)
    *   aa\1a  (aa.a)
    *
    * Numbers are handled by prepending to each number d-1 superdigits
    * where d = number of digits in the number and SUPERDIGIT is a
    * character with an integer value higher than any digit (for instance
    * ':'). This ensures that single-digit numbers are sorted before
    * double-digit numbers which in turn are sorted separately from
    * triple-digit numbers, etc. To avoid strange side-effects when
    * sorting strings that already contain SUPERDIGITs, a '\2'
    * is also prepended, like this
    *
    *   file\21      (file1)
    *   file\25      (file5)
    *   file\2:10    (file10)
    *   file\2:26    (file26)
    *   file\2::100  (file100)
    *   file:foo     (file:foo)
    *
    * This has the side-effect of sorting numbers before everything else (except
    * dots), but this is probably OK.
    *
    * Leading digits are ignored when doing the above. To discriminate
    * numbers which differ only in the number of leading digits, we append
    * the number of leading digits as a byte at the very end of the collation
    * key.
    *
    * To try avoid conflict with any collation key sequence generated by libc we
    * start each switch to a special cased part with a sentinel that hopefully
    * will sort before anything libc will generate.
  */

    if (len < 0)
        len = strlen (str);

    result = g_string_sized_new (len * 2);
    append = g_string_sized_new (0);

    /* No need to use utf8 functions, since we're only looking for ascii chars */
    for (prev = p = str; *p != '\0'; p++)
    {
        switch (*p)
        {
            case '.':
                if (prev != p)
                {
                    collate_key = g_utf8_collate_key (prev, p - prev);
                    g_string_append (result, collate_key);
                    g_free (collate_key);
                }

                g_string_append (result, COLLATION_SENTINEL "\1");

                /* skip the dot */
                prev = p + 1;
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if (prev != p)
                {
                    collate_key = g_utf8_collate_key (prev, p - prev);
                    g_string_append (result, collate_key);
                    g_free (collate_key);
                }

                g_string_append (result, COLLATION_SENTINEL "\2");

                prev = p;

                /* write d-1 colons */
                if (*p == '0')
                {
                    leading_zeros = 1;
                    digits = 0;
                }
                else
                {
                    leading_zeros = 0;
                    digits = 1;
                }

                do
                {
                    p++;

                    if (*p == '0' && !digits)
                        ++leading_zeros;
                    else if (g_ascii_isdigit(*p))
                        ++digits;
                    else
                        break;
                }
                while (*p != '\0');

                while (digits > 1)
                {
                    g_string_append_c (result, ':');
                    --digits;
                }

                if (leading_zeros > 0)
                {
                    g_string_append_c (append, (char)leading_zeros);
                    prev += leading_zeros;
                }

                /* write the number itself */
                g_string_append_len (result, prev, p - prev);

                prev = p;
                --p;	  /* go one step back to avoid disturbing outer loop */
                break;

            default:
                /* other characters just accumulate */
                break;
        }
    }

    if (prev != p)
    {
        collate_key = g_utf8_collate_key (prev, p - prev);
        g_string_append (result, collate_key);
        g_free (collate_key);
    }

    g_string_append (result, append->str);
    g_string_free (append, TRUE);

    return g_string_free (result, FALSE);
}
#endif /* !GLIB_CHECK_VERSION(2,8,0) */


/***************************************************************************/

/* XXX check out IconTheme changes */

typedef enum {
    MOO_ICON_LINK       = 1 << 0,
    MOO_ICON_LOCK       = 1 << 1,
    MOO_ICON_FLAGS_LEN  = 1 << 2
} MooIconFlags;

typedef enum {
    MOO_ICON_MIME = 0,
    MOO_ICON_HOME,
    MOO_ICON_DESKTOP,
    MOO_ICON_TRASH,
    MOO_ICON_DIRECTORY,
    MOO_ICON_BROKEN_LINK,
    MOO_ICON_NONEXISTENT,
    MOO_ICON_BLOCK_DEVICE,
    MOO_ICON_CHARACTER_DEVICE,
    MOO_ICON_FIFO,
    MOO_ICON_SOCKET,
    MOO_ICON_FILE,
    MOO_ICON_BLANK,
    MOO_ICON_MAX
} MooIconType;

typedef struct {
    GdkPixbuf *data[MOO_ICON_FLAGS_LEN];
} MooIconVars;

typedef struct {
    MooIconVars *special_icons[MOO_ICON_MAX];
    GHashTable  *mime_icons;                    /* char* -> MooIconVars* */
} MooIconCache;

static MooIconCache *moo_icon_cache_new         (void);
static void          moo_icon_cache_free        (MooIconCache   *cache);
static MooIconVars  *moo_icon_cache_lookup      (MooIconCache   *cache,
                                                 MooIconType     icon,
                                                 const char     *mime_type);
static void          moo_icon_cache_insert      (MooIconCache   *cache,
                                                 MooIconType     icon,
                                                 const char     *mime_type,
                                                 MooIconVars    *pixbufs);
static MooIconVars  *moo_icon_vars_new          (void);

static GdkPixbuf    *_create_icon_simple        (GtkIconTheme   *icon_theme,
                                                 MooIconType     icon,
                                                 const char     *mime_type,
                                                 GtkWidget      *widget,
                                                 GtkIconSize     size);
static GdkPixbuf    *_create_icon_with_flags    (GdkPixbuf      *original,
                                                 MooIconFlags    flags,
                                                 GtkIconTheme   *icon_theme,
                                                 GtkWidget      *widget,
                                                 GtkIconSize     size);
static GdkPixbuf    *_create_icon_for_mime_type (GtkIconTheme   *icon_theme,
                                                 const char     *mime_type,
                                                 GtkWidget      *widget,
                                                 GtkIconSize     size,
                                                 int             pixel_size);
static GdkPixbuf    *_create_named_icon_with_fallback
                                                (GtkIconTheme   *icon_theme,
                                                 const char     *name,
                                                 const char     *fallback_name,
                                                 const char     *fallback_stock,
                                                 GtkWidget      *widget,
                                                 GtkIconSize     size,
                                                 int             pixel_size);
static GdkPixbuf    *_create_broken_link_icon   (GtkIconTheme   *icon_theme,
                                                 GtkWidget      *widget,
                                                 GtkIconSize     size);
static GdkPixbuf    *_create_broken_icon        (GtkIconTheme   *icon_theme,
                                                 GtkWidget      *widget,
                                                 GtkIconSize     size);

static MooIconType   _get_folder_icon           (const char     *path);
static MooIconFlags  _get_icon_flags            (const MooFile  *file);


static GdkPixbuf    *_render_icon           (MooIconType     icon,
                                             const char     *mime_type,
                                             MooIconFlags    flags,
                                             GtkWidget      *widget,
                                             GtkIconSize     size)
{
    GtkIconTheme *icon_theme;
    GHashTable *all_sizes_cache;
    MooIconCache *cache;
    MooIconVars *pixbufs;

    g_return_val_if_fail (flags < MOO_ICON_FLAGS_LEN,
                          _render_icon (icon, mime_type, 0, widget, size));

    icon_theme = gtk_icon_theme_get_for_screen (gtk_widget_get_screen (widget));
    all_sizes_cache = g_object_get_data (G_OBJECT (icon_theme), "moo-file-icon-cache");

    if (!all_sizes_cache)
    {
        all_sizes_cache =
                g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                       NULL,
                                       (GDestroyNotify) moo_icon_cache_free);
        g_object_set_data_full (G_OBJECT (icon_theme),
                                "moo-file-icon-cache",
                                all_sizes_cache,
                                (GDestroyNotify) g_hash_table_destroy);
    }

    cache = g_hash_table_lookup (all_sizes_cache, GINT_TO_POINTER (size));

    if (!cache)
    {
        cache = moo_icon_cache_new ();
        g_hash_table_insert (all_sizes_cache, GINT_TO_POINTER (size), cache);
    }

    pixbufs = moo_icon_cache_lookup (cache, icon, mime_type);

    if (!pixbufs)
    {
        pixbufs = moo_icon_vars_new ();
        pixbufs->data[0] = _create_icon_simple (icon_theme, icon, mime_type, widget, size);
        g_assert (pixbufs->data[0] != NULL);
        moo_icon_cache_insert (cache, icon, mime_type, pixbufs);
    }

    if (!pixbufs->data[flags])
    {
        g_assert (flags != 0);
        g_assert (pixbufs->data[0] != NULL);
        pixbufs->data[flags] =
                _create_icon_with_flags (pixbufs->data[0], flags,
                                         icon_theme, widget, size);
        g_assert (pixbufs->data[flags] != NULL);
    }

    return pixbufs->data[flags];
}


static GdkPixbuf *
render_icon (const MooFile  *file,
             GtkWidget      *widget,
             GtkIconSize     size)
{
    GdkPixbuf *pixbuf = _render_icon (file->icon, file->mime_type,
                                      _get_icon_flags (file), widget, size);
    g_assert (pixbuf != NULL);
    return pixbuf;
}


static GdkPixbuf *
render_icon_for_path (const char     *path,
                      GtkWidget      *widget,
                      GtkIconSize     size)
{
    MooIconType icon = MOO_ICON_BLANK;
#ifndef __WIN32__
    const char *mime_type = xdg_mime_type_unknown;

    if (path)
    {
        mime_type = xdg_mime_get_mime_type_for_file (path, NULL);

        if (mime_type != xdg_mime_type_unknown)
        {
            icon = MOO_ICON_MIME;
        }
    }
#else
    const char *mime_type = "application/octet-stream";
#endif

    return _render_icon (icon, mime_type, 0, widget, size);
}


static GdkPixbuf    *_create_icon_simple        (GtkIconTheme   *icon_theme,
                                                 MooIconType     icon,
                                                 const char     *mime_type,
                                                 GtkWidget      *widget,
                                                 GtkIconSize     size)
{
    int width, height;
    GdkPixbuf *pixbuf;

    g_return_val_if_fail (icon < MOO_ICON_MAX, NULL);
    g_return_val_if_fail (icon != MOO_ICON_MIME || mime_type != NULL, NULL);

    if (!gtk_icon_size_lookup (size, &width, &height))
        if (!gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &width, &height))
            width = height = 16;

    switch (icon)
    {
        case MOO_ICON_MIME:
            pixbuf = _create_icon_for_mime_type (icon_theme, mime_type,
                                                 widget, size, width);

            if (pixbuf)
                return pixbuf;
            else
                return _create_icon_simple (icon_theme, MOO_ICON_FILE,
                                            NULL, widget, size);

        case MOO_ICON_HOME:
            return _create_named_icon_with_fallback (icon_theme,
                                                     "gnome-fs-home",
                                                     NULL,
                                                     GTK_STOCK_HOME,
                                                     widget, size, width);
        case MOO_ICON_DESKTOP:
            return _create_named_icon_with_fallback (icon_theme,
                                                     "gnome-fs-desktop",
                                                     "gnome-fs-directory",
                                                     GTK_STOCK_DIRECTORY,
                                                     widget, size, width);
        case MOO_ICON_TRASH:
            return _create_named_icon_with_fallback (icon_theme,
                                                     "gnome-fs-trash-full",
                                                     "gnome-fs-directory",
                                                     GTK_STOCK_DIRECTORY,
                                                     widget, size, width);
        case MOO_ICON_DIRECTORY:
            return _create_named_icon_with_fallback (icon_theme,
                                                     "gnome-fs-directory",
                                                     NULL,
                                                     GTK_STOCK_DIRECTORY,
                                                     widget, size, width);
        case MOO_ICON_BROKEN_LINK:
            return _create_broken_link_icon (icon_theme, widget, size);
        case MOO_ICON_NONEXISTENT:
            return _create_broken_icon (icon_theme, widget, size);
        case MOO_ICON_BLOCK_DEVICE:
            return _create_named_icon_with_fallback (icon_theme,
                                                     "gnome-fs-blockdev",
                                                     NULL,
                                                     GTK_STOCK_HARDDISK,
                                                     widget, size, width);
        case MOO_ICON_CHARACTER_DEVICE:
            return _create_named_icon_with_fallback (icon_theme,
                                                     "gnome-fs-chardev",
                                                     "gnome-fs-regular",
                                                     GTK_STOCK_FILE,
                                                     widget, size, width);
        case MOO_ICON_FIFO:
            return _create_named_icon_with_fallback (icon_theme,
                                                     "gnome-fs-fifo",
                                                     "gnome-fs-regular",
                                                     GTK_STOCK_FILE,
                                                     widget, size, width);
        case MOO_ICON_SOCKET:
            return _create_named_icon_with_fallback (icon_theme,
                                                     "gnome-fs-socket",
                                                     "gnome-fs-regular",
                                                     GTK_STOCK_FILE,
                                                     widget, size, width);
        case MOO_ICON_FILE:
            return _create_named_icon_with_fallback (icon_theme,
                                                     "gnome-fs-regular",
                                                     NULL,
                                                     GTK_STOCK_FILE,
                                                     widget, size, width);
        case MOO_ICON_BLANK:
            return _create_named_icon_with_fallback (icon_theme,
                                                     "gnome-fs-regular",
                                                     NULL,
                                                     GTK_STOCK_FILE,
                                                     widget, size, width);

        case MOO_ICON_MAX:
            g_return_val_if_reached (NULL);
    }

    g_return_val_if_reached (NULL);
}


static GdkPixbuf    *_create_named_icon_with_fallback
                                                (GtkIconTheme   *icon_theme,
                                                 const char     *name,
                                                 const char     *fallback_name,
                                                 const char     *fallback_stock,
                                                 GtkWidget      *widget,
                                                 GtkIconSize     size,
                                                 int             pixel_size)
{
    GdkPixbuf *pixbuf;

    g_return_val_if_fail (name != NULL, NULL);

    pixbuf = gtk_icon_theme_load_icon (icon_theme, name, pixel_size, 0, NULL);

    if (!pixbuf)
        g_message ("could not load '%s' icon", name);

    if (!pixbuf && fallback_name)
    {
        pixbuf = gtk_icon_theme_load_icon (icon_theme, fallback_name, pixel_size, 0, NULL);
        if (!pixbuf)
            g_message ("could not load '%s' icon", fallback_name);
    }

    if (!pixbuf && fallback_stock)
    {
        pixbuf = gtk_widget_render_icon (widget, fallback_stock, size, NULL);
        if (!pixbuf)
            g_warning ("could not load stock '%s' icon", fallback_stock);
    }

    if (!pixbuf)
    {
        pixbuf = gtk_widget_render_icon (widget, GTK_STOCK_FILE, size, NULL);
        if (!pixbuf)
            g_warning ("could not load stock '%s' icon", GTK_STOCK_FILE);
    }

    return pixbuf;
}


static guint8   get_icon                    (MooFile        *file,
                                             const char     *dirname)
{
    if (MOO_FILE_IS_BROKEN_LINK (file))
        return MOO_ICON_BROKEN_LINK;

    if (!MOO_FILE_EXISTS (file))
        return MOO_ICON_NONEXISTENT;

    if (MOO_FILE_IS_DIR (file))
    {
        char *path = MAKE_PATH (dirname, file);
        MooIconType icon = _get_folder_icon (path);
        g_free (path);
        return icon;
    }

    if (MOO_FILE_IS_SPECIAL (file))
    {
        if (moo_file_test (file, MOO_FILE_INFO_IS_BLOCK_DEV))
            return MOO_ICON_BLOCK_DEVICE;
        else if (moo_file_test (file, MOO_FILE_INFO_IS_CHAR_DEV))
            return MOO_ICON_CHARACTER_DEVICE;
        else if (moo_file_test (file, MOO_FILE_INFO_IS_FIFO))
            return MOO_ICON_FIFO;
        else if (moo_file_test (file, MOO_FILE_INFO_IS_SOCKET))
            return MOO_ICON_SOCKET;
    }

    if (file->flags & MOO_FILE_HAS_MIME_TYPE && file->mime_type)
        return MOO_ICON_MIME;

    return MOO_ICON_FILE;
}


static guint8   get_blank_icon              (void)
{
    return MOO_ICON_BLANK;
}


static GdkPixbuf    *_create_icon_for_mime_type (GtkIconTheme   *icon_theme,
                                                 const char     *mime_type,
                                                 GtkWidget      *widget,
                                                 GtkIconSize     size,
                                                 int             pixel_size)
{
    const char *separator;
    GString *icon_name;
    GdkPixbuf *pixbuf;
    char **parent_types;

    separator = strchr (mime_type, '/');
    if (!separator)
    {
        g_warning ("%s: mime type '%s' is invalid",
                   G_STRLOC, mime_type);
        return _create_icon_simple (icon_theme, MOO_ICON_FILE, NULL,
                                    widget, size);
    }

    icon_name = g_string_new ("gnome-mime-");
    g_string_append_len (icon_name, mime_type, separator - mime_type);
    g_string_append_c (icon_name, '-');
    g_string_append (icon_name, separator + 1);
    pixbuf = gtk_icon_theme_load_icon (icon_theme, icon_name->str,
                                       pixel_size, 0, NULL);
    g_string_free (icon_name, TRUE);

    if (pixbuf)
        return pixbuf;

    icon_name = g_string_new ("gnome-mime-");
    g_string_append_len (icon_name, mime_type, separator - mime_type);
    pixbuf = gtk_icon_theme_load_icon (icon_theme, icon_name->str,
                                       pixel_size, 0, NULL);
    g_string_free (icon_name, TRUE);

    if (pixbuf)
        return pixbuf;

    icon_name = g_string_new ("");
    g_string_append_len (icon_name, mime_type, separator - mime_type);
    g_string_append_c (icon_name, '-');
    g_string_append (icon_name, separator + 1);
    pixbuf = gtk_icon_theme_load_icon (icon_theme, icon_name->str,
                                       pixel_size, 0, NULL);
    g_string_free (icon_name, TRUE);

    if (pixbuf)
        return pixbuf;

    icon_name = g_string_new ("");
    g_string_append_len (icon_name, mime_type, separator - mime_type);
    pixbuf = gtk_icon_theme_load_icon (icon_theme, icon_name->str,
                                       pixel_size, 0, NULL);
    g_string_free (icon_name, TRUE);

    if (pixbuf)
        return pixbuf;

#ifndef __WIN32__
    parent_types = xdg_mime_list_mime_parents (mime_type);

    if (parent_types && parent_types[0])
        pixbuf = _create_icon_for_mime_type (icon_theme, parent_types[0],
                                             widget, size, pixel_size);

    if (parent_types)
        free (parent_types);

    if (pixbuf)
        return pixbuf;
#endif

    g_message ("%s: could not find icon for mime type '%s'",
               G_STRLOC, mime_type);
    return _create_icon_simple (icon_theme, MOO_ICON_FILE, NULL,
                                widget, size);
}


static GdkPixbuf    *_create_broken_link_icon   (G_GNUC_UNUSED GtkIconTheme *icon_theme,
                                                 GtkWidget      *widget,
                                                 GtkIconSize     size)
{
    /* XXX */
    return gtk_widget_render_icon (widget, GTK_STOCK_MISSING_IMAGE, size, NULL);
}


static GdkPixbuf    *_create_broken_icon        (G_GNUC_UNUSED GtkIconTheme *icon_theme,
                                                 GtkWidget      *widget,
                                                 GtkIconSize     size)
{
    /* XXX */
    return gtk_widget_render_icon (widget, GTK_STOCK_MISSING_IMAGE, size, NULL);
}


static GdkPixbuf    *_create_icon_with_flags    (GdkPixbuf      *original,
                                                 MooIconFlags    flags,
                                                 G_GNUC_UNUSED GtkIconTheme *icon_theme,
                                                 G_GNUC_UNUSED GtkWidget *widget,
                                                 GtkIconSize     size)
{
    static GdkPixbuf *arrow = NULL, *small_arrow = NULL;
    int width, height;
    GdkPixbuf *pixbuf;

    /* XXX */
    g_assert (flags != 0);

    if (!gtk_icon_size_lookup (size, &width, &height))
    {
        width = gdk_pixbuf_get_width (original);
        height = gdk_pixbuf_get_height (original);
    }

    if (flags & MOO_ICON_LINK)
    {
        GdkPixbuf *emblem;

        if (!arrow)
        {
            arrow = gdk_pixbuf_new_from_inline (-1, SYMLINK_ARROW, TRUE, NULL);
            g_return_val_if_fail (arrow != NULL, g_object_ref (original));
        }

        if (!small_arrow)
        {
            small_arrow = gdk_pixbuf_new_from_inline (-1, SYMLINK_ARROW_SMALL, TRUE, NULL);
            g_return_val_if_fail (arrow != NULL, g_object_ref (original));
        }

        if (size == GTK_ICON_SIZE_MENU)
            emblem = small_arrow;
        else
            emblem = arrow;

        pixbuf = gdk_pixbuf_copy (original);
        g_return_val_if_fail (pixbuf != NULL, g_object_ref (original));

        gdk_pixbuf_composite (emblem, pixbuf,
                              0,
                              gdk_pixbuf_get_height (pixbuf) - gdk_pixbuf_get_height (emblem),
                              gdk_pixbuf_get_width (emblem),
                              gdk_pixbuf_get_height (emblem),
                              0,
                              gdk_pixbuf_get_height (pixbuf) - gdk_pixbuf_get_height (emblem),
                              1, 1,
                              GDK_INTERP_BILINEAR,
                              255);
    }
    else
    {
        pixbuf = g_object_ref (original);
    }

    return pixbuf;
}


static MooIconType   _get_folder_icon           (const char     *path)
{
    static const char *home_path = NULL;
    static char *desktop_path = NULL;
    static char *trash_path = NULL;

    if (!path)
        return MOO_ICON_DIRECTORY;

    if (!home_path)
        home_path = g_get_home_dir ();

    if (!home_path)
        return MOO_ICON_DIRECTORY;

    if (!desktop_path)
        desktop_path = g_build_filename (home_path, "Desktop", NULL);

    if (!trash_path)
        trash_path = g_build_filename (desktop_path, "Trash", NULL);

        /* keep this in sync with create_fallback_icon() */
    if (strcmp (home_path, path) == 0)
        return MOO_ICON_HOME;
    else if (strcmp (desktop_path, path) == 0)
        return MOO_ICON_DESKTOP;
    else if (strcmp (trash_path, path) == 0)
        return MOO_ICON_TRASH;
    else
        return MOO_ICON_DIRECTORY;
}


static MooIconFlags  _get_icon_flags            (const MooFile  *file)
{
    return (MOO_FILE_IS_LOCKED (file) ? MOO_ICON_LOCK : 0) |
            (MOO_FILE_IS_LINK (file) ? MOO_ICON_LINK : 0);
}


static MooIconVars  *moo_icon_vars_new          (void)
{
    return g_new0 (MooIconVars, 1);
}


static void          moo_icon_vars_free         (MooIconVars    *pixbufs)
{
    if (pixbufs)
    {
        guint i;

        for (i = 0; i < MOO_ICON_FLAGS_LEN; ++i)
        {
            if (pixbufs->data[i])
                g_object_unref (pixbufs->data[i]);
            pixbufs->data[i] = NULL;
        }

        g_free (pixbufs);
    }
}


static MooIconCache *moo_icon_cache_new         (void)
{
    MooIconCache *cache = g_new0 (MooIconCache, 1);
    cache->mime_icons = g_hash_table_new_full (g_str_hash, g_str_equal,
                                               g_free,
                                               (GDestroyNotify) moo_icon_vars_free);
    return cache;
}


static void          moo_icon_cache_free        (MooIconCache   *cache)
{
    if (cache)
    {
        guint i;

        for (i = 0; i < MOO_ICON_MAX; ++i)
        {
            moo_icon_vars_free (cache->special_icons[i]);
            cache->special_icons[i] = NULL;
        }

        g_hash_table_destroy (cache->mime_icons);
        g_free (cache);
    }
}


static MooIconVars  *moo_icon_cache_lookup      (MooIconCache   *cache,
                                                 MooIconType     icon,
                                                 const char     *mime_type)
{
    if (icon != MOO_ICON_MIME)
    {
        g_assert (icon < MOO_ICON_MAX);
        return cache->special_icons[icon];
    }
    else
    {
        g_assert (mime_type != NULL);
        return g_hash_table_lookup (cache->mime_icons, mime_type);
    }
}


static void          moo_icon_cache_insert      (MooIconCache   *cache,
                                                 MooIconType     icon,
                                                 const char     *mime_type,
                                                 MooIconVars    *pixbufs)
{
    if (icon != MOO_ICON_MIME)
    {
        g_assert (icon < MOO_ICON_MAX);
        g_assert (pixbufs != NULL);
        cache->special_icons[icon] = pixbufs;
    }
    else
    {
        g_assert (mime_type != NULL);
        g_assert (pixbufs != NULL);
        g_hash_table_insert (cache->mime_icons, g_strdup (mime_type), pixbufs);
    }
}
