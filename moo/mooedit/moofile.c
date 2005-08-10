/*
 *   mooedit/moofile.c
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

/*
 *  Icon handling code was copied (and modified) from gtk/gtkfilesystemunix.c,
 *  Copyright (C) 2003, Red Hat, Inc.
 *
 *  Don't ask me why and how rest of code works/done this way, it's all
 *  experimental.
 */

#define MOO_FILE_SYSTEM_COMPILATION
#include "mooedit/moofilesystem.h"
#include "mooedit/xdgmime/xdgmime.h"
#include "mooutils/moomarshals.h"
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#define NORMAL_PRIORITY         G_PRIORITY_DEFAULT_IDLE
#define NORMAL_TIMEOUT          0.1
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
    GSList *files;
    GSList *files_copy;
    char *path;
    GSourceFunc populate_func;
    int populate_priority;
    guint populate_idle_id;
    double populate_timeout;
    Debug debug;
    GTimer *timer;
};


static void     moo_folder_finalize         (GObject    *object);

static void     moo_folder_deleted          (MooFolder  *folder);

static void     folder_emit_deleted         (MooFolder  *folder);
static void     folder_emit_files           (MooFolder  *folder,
                                             guint       signal,
                                             GSList     *files);

static void     stop_populate               (MooFolder  *folder);

static GSList  *files_list_copy             (GSList     *list);
static void     files_list_free             (GSList    **list);

static gboolean get_icons_a_bit             (MooFolder  *folder);
static gboolean get_stat_a_bit              (MooFolder  *folder);
static double   get_names                   (MooFolder  *folder);

static const char   *get_default_file_icon  (void);
static const char   *get_nonexistent_icon   (void);
static const char   *get_blank_icon         (void);
static const char   *get_icon               (MooFolder      *folder,
                                             MooFile        *file);
static const char   *get_folder_icon        (const char     *path);
static GdkPixbuf    *get_named_icon         (const char     *name,
                                             GtkWidget      *widget,
                                             GtkIconSize     size);
static GdkPixbuf    *create_named_icon      (GtkIconTheme   *icon_theme,
                                             const char     *name,
                                             GtkWidget      *widget,
                                             GtkIconSize     size);
static GdkPixbuf    *create_fallback_icon   (GtkIconTheme   *icon_theme,
                                             const char     *name,
                                             GtkWidget      *widget,
                                             GtkIconSize     size);
static GdkPixbuf    *create_icon_for_mime_type (GtkIconTheme *icon_theme,
                                             const char     *mime_type,
                                             int             pixel_size);

#define FILE_PATH(folder,file) g_build_filename (folder->priv->path, file->basename, NULL)


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
    folder->priv->files = NULL;
    folder->priv->files_copy = NULL;
    folder->priv->path = NULL;
    folder->priv->populate_func = NULL;
    folder->priv->populate_idle_id = 0;
    folder->priv->populate_timeout = BACKGROUND_TIMEOUT;
    folder->priv->timer = g_timer_new ();
    g_timer_stop (folder->priv->timer);
}


static void moo_folder_finalize  (GObject      *object)
{
    MooFolder *folder = MOO_FOLDER (object);

    files_list_free (&folder->priv->files);
    files_list_free (&folder->priv->files_copy);
    if (folder->priv->dir)
        g_dir_close (folder->priv->dir);
    g_free (folder->priv->path);
    if (folder->priv->populate_idle_id)
        g_source_remove (folder->priv->populate_idle_id);
    g_timer_destroy (folder->priv->timer);

    g_free (folder->priv);
    folder->priv = NULL;

    G_OBJECT_CLASS (moo_folder_parent_class)->finalize (object);
}


MooFolder   *moo_folder_new             (MooFileSystem  *fs,
                                         const char     *path,
                                         MooFileFlags    wanted,
                                         GError        **error)
{
    char *norm_path;
    GDir *dir;
    MooFolder *folder;
    GError *file_error = NULL;

    g_return_val_if_fail (MOO_IS_FILE_SYSTEM (fs), NULL);

    g_return_val_if_fail (path != NULL, NULL);
    norm_path = moo_normalize_path (path);
    g_return_val_if_fail (norm_path != NULL, NULL);

    dir = g_dir_open (norm_path, 0, &file_error);

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
        g_free (norm_path);
        return NULL;
    }

    folder = g_object_new (MOO_TYPE_FOLDER, NULL);
    folder->priv->fs = fs;
    folder->priv->path = norm_path;

    folder->priv->dir = dir;

    moo_folder_set_wanted (folder, wanted, TRUE);

    return folder;
}


static void      moo_folder_deleted         (MooFolder  *folder)
{
    stop_populate (folder);
    folder->priv->deleted = TRUE;
    files_list_free (&folder->priv->files);
}


void         moo_folder_set_wanted      (MooFolder      *folder,
                                         MooFileFlags    wanted,
                                         gboolean        bit_now)
{
    Stage wanted_stage = STAGE_NAMES;
    double elapsed = 0.0;

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

    if (folder->priv->wanted != 0)
    {
        g_assert (folder->priv->populate_idle_id != 0);
        folder->priv->wanted = MAX (folder->priv->wanted, wanted_stage);
        return;
    }

    folder->priv->wanted = wanted_stage;

    if (!folder->priv->done)
    {
        g_assert (folder->priv->dir != NULL);
        elapsed = get_names (folder);
    }

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

    folder->priv->populate_timeout = NORMAL_TIMEOUT;
    folder->priv->populate_priority = NORMAL_PRIORITY;

    TIMER_CLEAR (folder->priv->timer);

    if (!bit_now ||
        elapsed > folder->priv->populate_timeout ||
         folder->priv->populate_func (folder))
    {
        folder->priv->populate_idle_id =
                g_idle_add_full (folder->priv->populate_priority,
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
    return files_list_copy (folder->priv->files);
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

    timer = g_timer_new ();

    for (name = g_dir_read_name (folder->priv->dir);
            name != NULL;
            name = g_dir_read_name (folder->priv->dir))
    {
        file = moo_file_new (folder->priv->path, name);
        file->icon = get_blank_icon ();
        folder->priv->files = g_slist_prepend (folder->priv->files, file);
        added = g_slist_prepend (added, file);
    }

    elapsed = folder->priv->debug.names_timer =
            g_timer_elapsed (timer, NULL);
    g_timer_destroy (timer);

    folder_emit_files (folder, FILES_ADDED, added);
    g_slist_free (added);

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
    GSList *changed = NULL;
    double elapsed;

    g_assert (folder->priv->dir == NULL);
    g_assert (folder->priv->done == STAGE_NAMES);
    g_assert (folder->priv->path != NULL);

    elapsed = g_timer_elapsed (folder->priv->timer, NULL);
    g_timer_continue (folder->priv->timer);

    if (!folder->priv->files_copy)
        folder->priv->files_copy =
                files_list_copy (folder->priv->files);
    if (!folder->priv->files_copy)
        done = TRUE;

    while (!done)
    {
        MooFile *file = folder->priv->files_copy->data;
        folder->priv->files_copy =
                g_slist_delete_link (folder->priv->files_copy,
                                     folder->priv->files_copy);

        changed = g_slist_prepend (changed, file);

        if (!(file->flags & MOO_FILE_HAS_STAT))
            moo_file_stat (file, folder->priv->path);

        if (!folder->priv->files_copy)
            done = TRUE;

        if (g_timer_elapsed (folder->priv->timer, NULL) > folder->priv->populate_timeout)
            break;
    }

    elapsed = g_timer_elapsed (folder->priv->timer, NULL) - elapsed;
    folder->priv->debug.stat_timer += elapsed;
    folder->priv->debug.stat_counter += 1;
    g_timer_stop (folder->priv->timer);

    folder_emit_files (folder, FILES_CHANGED, changed);
    files_list_free (&changed);

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
                            g_idle_add_full (folder->priv->populate_priority,
                                             folder->priv->populate_func,
                                             folder, NULL);
            }
            else
            {
                TIMER_CLEAR (folder->priv->timer);
                folder->priv->populate_idle_id =
                        g_idle_add_full (folder->priv->populate_priority,
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
    GSList *changed = NULL;

    g_assert (folder->priv->dir == NULL);
    g_assert (folder->priv->done == STAGE_STAT);
    g_assert (folder->priv->path != NULL);

    elapsed = g_timer_elapsed (folder->priv->timer, NULL);
    g_timer_continue (folder->priv->timer);

    if (!folder->priv->files_copy)
        folder->priv->files_copy =
                files_list_copy (folder->priv->files);
    if (!folder->priv->files_copy)
        done = TRUE;

    while (!done)
    {
        MooFile *file = folder->priv->files_copy->data;
        char *path;

        folder->priv->files_copy =
                g_slist_delete_link (folder->priv->files_copy,
                                     folder->priv->files_copy);

        if (file->info & MOO_FILE_EXISTS &&
            !(file->flags & MOO_FILE_HAS_MIME_TYPE))
        {
            changed = g_slist_prepend (changed, file);
            path = FILE_PATH (folder, file);
            file->mime_type = xdg_mime_get_mime_type_for_file (path);
            file->flags |= MOO_FILE_HAS_MIME_TYPE;
            file->flags |= MOO_FILE_HAS_ICON;
            file->icon = get_icon (folder, file);
            g_free (path);
        }
        else
        {
            moo_file_unref (file);
        }

        if (!folder->priv->files_copy)
            done = TRUE;

        if (g_timer_elapsed (folder->priv->timer, NULL) > folder->priv->populate_timeout)
            break;
    }

    elapsed = g_timer_elapsed (folder->priv->timer, NULL) - elapsed;
    folder->priv->debug.icons_timer += elapsed;
    folder->priv->debug.icons_counter += 1;
    TIMER_CLEAR (folder->priv->timer);

    folder_emit_files (folder, FILES_CHANGED, changed);
    files_list_free (&changed);

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
    }

    return !done;
}


/********************************************************************/
/* MooFile
 */

MooFile     *moo_file_new               (const char     *dirname,
                                         const char     *basename)
{
    MooFile *file;
    char *path;

    g_return_val_if_fail (dirname != NULL && basename != NULL, NULL);

    file = g_new0 (MooFile, 1);

    file->basename = g_strdup (basename);
    path = g_build_filename (dirname, basename, NULL);
    file->display_basename = g_filename_display_basename (path);
    g_free (path);
    file->ref_count = 1;

    if (basename[0] == '.')
        file->info = MOO_FILE_IS_HIDDEN;

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
        g_free (file->basename);
        g_free (file->display_basename);
        g_free (file);
    }
}


void         moo_file_stat              (MooFile        *file,
                                         const char     *dirname)
{
    char *fullname;

    g_return_if_fail (file != NULL);

    fullname = g_build_filename (dirname, file->basename, NULL);

    file->info = MOO_FILE_EXISTS;
    file->flags = MOO_FILE_HAS_STAT;

    if (stat (fullname, &file->statbuf) != 0)
    {
        if (errno == ENOENT && !lstat (fullname, &file->statbuf))
        {
            gchar *display_name = g_filename_display_name (fullname);
            g_warning ("%s: file '%s' is a broken link",
                       G_STRLOC, display_name);
            g_free (display_name);
            file->info = MOO_FILE_IS_LINK;
        }
        else
        {
            int save_errno = errno;
            gchar *display_name = g_filename_display_name (fullname);
            g_warning ("%s: error getting information for '%s': %s",
                       G_STRLOC, display_name,
                       g_strerror (save_errno));
            g_free (display_name);
            file->info = 0;
            file->flags = 0;
        }
    }

    if (file->info & MOO_FILE_EXISTS)
    {
        if (S_ISDIR (file->statbuf.st_mode))
            file->info |= MOO_FILE_IS_FOLDER;
        if (S_ISLNK (file->statbuf.st_mode))
            file->info |= MOO_FILE_IS_LINK;
    }

    if (file->info & MOO_FILE_EXISTS)
    {
        if (file->info & MOO_FILE_IS_FOLDER)
        {
            file->flags |= MOO_FILE_HAS_MIME_TYPE;
            file->flags |= MOO_FILE_HAS_ICON;
            file->icon = get_folder_icon (fullname);
        }
        else
        {
            file->icon = get_default_file_icon ();
        }
    }
    else
    {
        file->icon = get_nonexistent_icon ();
    }

    if (file->basename[0] == '.')
        file->info |= MOO_FILE_IS_HIDDEN;

    g_free (fullname);
}


gboolean     moo_file_test              (const MooFile  *file,
                                         MooFileInfo     test)
{
    g_return_val_if_fail (file != NULL, FALSE);
    return file->info & test;
}


char        *moo_normalize_path         (const char     *path)
{
    return g_strdup (path);
}


const char   *moo_file_get_display_basename  (const MooFile  *file)
{
    g_return_val_if_fail (file != NULL, NULL);
    return file->display_basename;
}


const char  *moo_file_get_basename          (const MooFile  *file)
{
    g_return_val_if_fail (file != NULL, NULL);
    return file->basename;
}


const char  *moo_file_get_mime_type         (const MooFile  *file)
{
    g_return_val_if_fail (file != NULL, NULL);
    return file->mime_type;
}


gconstpointer moo_file_get_stat             (const MooFile  *file)
{
    g_return_val_if_fail (file != NULL, NULL);
    if (file->flags & MOO_FILE_HAS_STAT && file->info & MOO_FILE_EXISTS)
        return &file->statbuf;
    else
        return NULL;
}


GdkPixbuf   *moo_file_get_icon              (const MooFile  *file,
                                             GtkWidget      *widget,
                                             GtkIconSize     size)
{
    g_return_val_if_fail (file != NULL, NULL);
    return get_named_icon (file->icon, widget, size);
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
        { MOO_FILE_EXISTS, (char*)"MOO_FILE_EXISTS", (char*)"exists" },
        { MOO_FILE_IS_FOLDER, (char*)"MOO_FILE_IS_FOLDER", (char*)"is-folder" },
        { MOO_FILE_IS_HIDDEN, (char*)"MOO_FILE_IS_HIDDEN", (char*)"is-hidden" },
        { MOO_FILE_IS_LINK, (char*)"MOO_FILE_IS_LINK", (char*)"is-link" },
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


static GSList  *files_list_copy             (GSList     *list)
{
    GSList *copy, *l;
    for (copy = NULL, l = list; l != NULL; l = l->next)
        copy = g_slist_prepend (copy, moo_file_ref (l->data));
    return g_slist_reverse (copy);
}


static void     files_list_free             (GSList    **list)
{
    g_slist_foreach (*list, (GFunc) moo_file_unref, NULL);
    g_slist_free (*list);
    *list = NULL;
}




/***************************************************************************/

#define FILE_ICON_NAME                  "gnome-fs-regular"
#define BLOCK_DEVICE_ICON_NAME          "gnome-fs-blockdev"
#define EXECUTABLE_ICON_NAME            "gnome-fs-executable"
#define SYMBOLIC_LINK_ICON_NAME         "gnome-fs-symlink"
#define CHARACTER_DEVICE_ICON_NAME      "gnome-fs-chardev"
#define FIFO_ICON_NAME                  "gnome-fs-fifo"
#define SOCKET_ICON_NAME                "gnome-fs-socket"
#define NONEXISTENT_ICON_NAME           "BROKEN"
#define BROKEN_LINK_ICON_NAME           "BROKEN_LINK"
#define BLANK_ICON_NAME                 ""


static const char   *get_default_file_icon  (void)
{
    return FILE_ICON_NAME;
}

static const char   *get_blank_icon         (void)
{
    return BLANK_ICON_NAME;
}

static const char   *get_nonexistent_icon   (void)
{
    return NONEXISTENT_ICON_NAME;
}


static GdkPixbuf    *create_icon_for_mime_type (GtkIconTheme   *icon_theme,
                                                const char     *mime_type,
                                                int             pixel_size)
{
    const char *separator;
    GString *icon_name;
    GdkPixbuf *pixbuf;

    separator = strchr (mime_type, '/');
    if (!separator)
        return NULL;

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

    return pixbuf;
}


static GdkPixbuf    *get_named_icon         (const char     *name,
                                             GtkWidget      *widget,
                                             GtkIconSize     size)
{
    GtkIconTheme *icon_theme;
    GHashTable *all_sizes_cache, *cache;
    GdkPixbuf *pixbuf;

    if (!name)
        return NULL;

    icon_theme = gtk_icon_theme_get_for_screen (gtk_widget_get_screen (widget));
    all_sizes_cache = g_object_get_data (G_OBJECT (icon_theme), "moo-file-icon-cache");

    if (!all_sizes_cache)
    {
        all_sizes_cache =
                g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                       NULL,
                                       (GDestroyNotify) g_hash_table_destroy);
        g_object_set_data_full (G_OBJECT (icon_theme),
                                "moo-file-icon-cache",
                                all_sizes_cache,
                                (GDestroyNotify) g_hash_table_destroy);
    }

    cache = g_hash_table_lookup (all_sizes_cache, GINT_TO_POINTER (size));

    if (!cache)
    {
        cache = g_hash_table_new_full (g_str_hash, g_str_equal,
                                       g_free, g_object_unref);
        g_hash_table_insert (all_sizes_cache, GINT_TO_POINTER (size), cache);
    }

    pixbuf = g_hash_table_lookup (cache, name);

    if (pixbuf)
        return pixbuf;

    pixbuf = create_named_icon (icon_theme, name, widget, size);
    g_assert (pixbuf != NULL);
    g_hash_table_insert (cache, g_strdup (name), pixbuf);

    return pixbuf;
}


static GdkPixbuf    *create_named_icon      (GtkIconTheme   *icon_theme,
                                             const char     *name,
                                             GtkWidget      *widget,
                                             GtkIconSize     size)
{
    int width, height;
    GdkPixbuf *pixbuf;

    g_return_val_if_fail (name != NULL, NULL);

    if (!strcmp (name, BLANK_ICON_NAME))
    {
        pixbuf = gtk_widget_render_icon (widget, GTK_STOCK_FILE, size, "");
        if (!pixbuf)
            g_warning ("%s: could not create a stock icon for %s",
                       G_STRLOC, GTK_STOCK_FILE);
        return pixbuf;
    }

    if (!strcmp (name, NONEXISTENT_ICON_NAME) ||
         !strcmp (name, BROKEN_LINK_ICON_NAME))
    {
        pixbuf = gtk_widget_render_icon (widget, GTK_STOCK_MISSING_IMAGE, size, "");
        if (!pixbuf)
            g_warning ("%s: could not create a stock icon for %s",
                       G_STRLOC, GTK_STOCK_MISSING_IMAGE);
        return pixbuf;
    }

    if (!gtk_icon_size_lookup (size, &width, &height))
        if (!gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &width, &height))
            width = height = 16;

    if (!strncmp (name, "MIME-", 5))
    {
        const char *mime_type = name + 5;
        pixbuf = create_icon_for_mime_type (icon_theme, mime_type, width);
        if (pixbuf)
            return pixbuf;
        else
            return get_named_icon (FILE_ICON_NAME, widget, size);
    }

    pixbuf = gtk_icon_theme_load_icon (icon_theme, name, width, 0, NULL);

    if (!pixbuf)
        pixbuf = create_fallback_icon (icon_theme, name, widget, size);

    return pixbuf;
}


static const char   *get_folder_icon        (const char     *path)
{
    static const char *home_path = NULL;
    static char *desktop_path = NULL;
    static char *trash_path = NULL;

    if (!home_path)
        home_path = g_get_home_dir ();

    if (!home_path)
        return "gnome-fs-directory";

    if (!desktop_path)
        desktop_path = g_build_filename (home_path, "Desktop", NULL);

    if (!trash_path)
        trash_path = g_build_filename (desktop_path, "Trash", NULL);

    if (strcmp (home_path, path) == 0)
        return "gnome-fs-home";
    else if (strcmp (desktop_path, path) == 0)
        return "gnome-fs-desktop";
    else if (strcmp (trash_path, path) == 0)
        return "gnome-fs-trash-full";
    else
        return "gnome-fs-directory";
}


static GdkPixbuf    *create_fallback_icon   (GtkIconTheme   *icon_theme,
                                             const char     *name,
                                             GtkWidget      *widget,
                                             GtkIconSize     size)
{
    const char *stock_name;
    GdkPixbuf *pixbuf;

    if (!strcmp (name, BLOCK_DEVICE_ICON_NAME))
        stock_name = GTK_STOCK_HARDDISK;
    else if (!strcmp (name, EXECUTABLE_ICON_NAME))
        stock_name = GTK_STOCK_EXECUTE;
    else
    {
        int width, height;

        if (!gtk_icon_size_lookup (size, &width, &height))
            if (!gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &width, &height))
                width = height = 16;

        pixbuf = gtk_icon_theme_load_icon (icon_theme, FILE_ICON_NAME,
                                           width, 0, NULL);

        if (pixbuf)
            return pixbuf;
        else
            stock_name = GTK_STOCK_FILE;
    }

    pixbuf = gtk_widget_render_icon (widget, stock_name, size, NULL);

    if (!pixbuf)
        g_warning ("%s: could not create a stock icon for %s",
                   G_STRLOC, stock_name);

    return pixbuf;
}


static const char   *get_icon               (MooFolder      *folder,
                                             MooFile        *file)
{
    static GHashTable *mime_type_names = NULL;

    if (!(file->info & MOO_FILE_EXISTS))
        return NONEXISTENT_ICON_NAME;

    if (file->info & MOO_FILE_IS_FOLDER)
    {
        char *path = FILE_PATH (folder, file);
        const char *name = get_folder_icon (path);
        g_free (path);
        return name;
    }

    if (file->flags & MOO_FILE_HAS_STAT)
    {
        struct stat *statp = &file->statbuf;

        if (S_ISBLK (statp->st_mode))
            return BLOCK_DEVICE_ICON_NAME;
        else if (S_ISLNK (statp->st_mode))
            return BROKEN_LINK_ICON_NAME;
        else if (S_ISCHR (statp->st_mode))
            return CHARACTER_DEVICE_ICON_NAME;
#ifdef S_ISFIFO
        else if (S_ISFIFO (statp->st_mode))
            return FIFO_ICON_NAME;
#endif
#ifdef S_ISSOCK
        else if (S_ISSOCK (statp->st_mode))
            return SOCKET_ICON_NAME;
#endif
    }

    if (file->flags & MOO_FILE_HAS_MIME_TYPE && file->mime_type)
    {
        char *name;

        if (!mime_type_names)
            mime_type_names =
                    g_hash_table_new_full (g_str_hash, g_str_equal,
                                           g_free, g_free);

        name = g_hash_table_lookup (mime_type_names, file->mime_type);

        if (!name)
        {
            name = g_strdup_printf ("MIME-%s", file->mime_type);
            g_hash_table_insert (mime_type_names,
                                 g_strdup (file->mime_type),
                                 name);
        }

        return name;
    }

    return FILE_ICON_NAME;
}
