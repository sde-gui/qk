/*
 *   mooedit/moofilesystem.c
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

#define MOO_FILE_SYSTEM_COMPILATION
#include "mooedit/moofilesystem.h"
#include "mooutils/moomarshals.h"
#include <string.h>
#include <sys/stat.h>
#include <errno.h>


struct _MooFileSystemPrivate {
    GHashTable *folders;
};

static MooFileSystem *fs_instance = NULL;


static void      moo_file_system_finalize   (GObject        *object);

static MooFolder*get_folder_default     (MooFileSystem  *fs,
                                         const char     *path,
                                         MooFileFlags    wanted,
                                         GError        **error);
static gboolean  create_folder_default  (MooFileSystem *fs,
                                         const char     *path,
                                         GError        **error);
static gboolean  delete_file_default    (MooFileSystem  *fs,
                                         const char     *path,
                                         GError        **error);
static char     *get_parent_default     (MooFileSystem  *fs,
                                         const char     *path,
                                         GError        **error);
static char     *make_path_default      (MooFileSystem  *fs,
                                         const char     *base_path,
                                         const char     *display_name,
                                         GError        **error);


/* MOO_TYPE_FILE_SYSTEM */
G_DEFINE_TYPE (MooFileSystem, moo_file_system, G_TYPE_OBJECT)

enum {
    PROP_0,
};

enum {
    LAST_SIGNAL
};

// static guint signals[LAST_SIGNAL];

static void moo_file_system_class_init (MooFileSystemClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_file_system_finalize;

    klass->get_folder = get_folder_default;
    klass->create_folder = create_folder_default;
    klass->delete_file = delete_file_default;
    klass->get_parent = get_parent_default;
    klass->make_path = make_path_default;
}


static void moo_file_system_init      (MooFileSystem    *fs)
{
    fs->priv = g_new0 (MooFileSystemPrivate, 1);

    fs->priv->folders = g_hash_table_new_full (g_str_hash, g_str_equal,
                                               g_free, g_object_unref);
}


static void moo_file_system_finalize  (GObject      *object)
{
    MooFileSystem *fs = MOO_FILE_SYSTEM (object);

    g_hash_table_destroy (fs->priv->folders);
    fs->priv->folders = NULL;
    g_free (fs->priv);

    G_OBJECT_CLASS (moo_file_system_parent_class)->finalize (object);
}


MooFileSystem   *moo_file_system_create     (void)
{
    if (!fs_instance)
    {
        fs_instance = MOO_FILE_SYSTEM (g_object_new (MOO_TYPE_FILE_SYSTEM, NULL));
        g_object_weak_ref (G_OBJECT (fs_instance),
                           (GWeakNotify) g_nullify_pointer, &fs_instance);
        return fs_instance;
    }
    else
    {
        return g_object_ref (fs_instance);
    }
}


MooFolder       *moo_file_system_get_folder (MooFileSystem  *fs,
                                             const char     *path,
                                             MooFileFlags    wanted,
                                             GError        **error)
{
    return MOO_FILE_SYSTEM_GET_CLASS(fs)->get_folder (fs, path, wanted, error);
}


gboolean         moo_file_system_create_folder (MooFileSystem *fs,
                                             const char     *path,
                                             GError        **error)
{
    return MOO_FILE_SYSTEM_GET_CLASS(fs)->create_folder (fs, path, error);
}


gboolean         moo_file_system_delete_file(MooFileSystem  *fs,
                                             const char     *path,
                                             GError        **error)
{
    return MOO_FILE_SYSTEM_GET_CLASS(fs)->delete_file (fs, path, error);
}


char            *moo_file_system_get_parent (MooFileSystem  *fs,
                                             const char     *path,
                                             GError        **error)
{
    return MOO_FILE_SYSTEM_GET_CLASS(fs)->get_parent (fs, path, error);
}


char            *moo_file_system_make_path  (MooFileSystem  *fs,
                                             const char     *base_path,
                                             const char     *display_name,
                                             GError        **error)
{
    return MOO_FILE_SYSTEM_GET_CLASS(fs)->make_path (fs, base_path, display_name, error);
}


/***************************************************************************/
/* Default (UNIX) methods
 */

MooFolder       *get_folder_default     (MooFileSystem  *fs,
                                         const char     *path,
                                         MooFileFlags    wanted,
                                         GError        **error)
{
    MooFolder *folder;

    g_return_val_if_fail (path != NULL, NULL);
    g_return_val_if_fail (g_path_is_absolute (path), NULL);

    folder = g_hash_table_lookup (fs->priv->folders, path);

    if (folder)
    {
        moo_folder_set_wanted (folder, wanted, TRUE);
        g_object_ref (folder);
        return folder;
    }

    if (!g_file_test (path, G_FILE_TEST_IS_DIR))
    {
        g_set_error (error, MOO_FILE_ERROR,
                     MOO_FILE_ERROR_NOT_FOLDER,
                     "'%s' is not a folder", path);
        return NULL;
    }

    folder = moo_folder_new (fs, path, wanted, error);

    if (folder)
        g_hash_table_insert (fs->priv->folders,
                             g_strdup (path),
                             g_object_ref (folder));

    return folder;
}


static gboolean  create_folder_default  (G_GNUC_UNUSED MooFileSystem *fs,
                                         const char     *path,
                                         GError        **error)
{
    g_return_val_if_fail (path != NULL, FALSE);
    g_return_val_if_fail (g_path_is_absolute (path), FALSE);
    g_set_error (error, MOO_FILE_ERROR,
                 MOO_FILE_ERROR_NOT_IMPLEMENTED,
                 "don't know how to create folder '%s'", path);
    return FALSE;
}


static gboolean  delete_file_default    (G_GNUC_UNUSED MooFileSystem *fs,
                                         const char     *path,
                                         GError        **error)
{
    g_return_val_if_fail (path != NULL, FALSE);
    g_return_val_if_fail (g_path_is_absolute (path), FALSE);
    g_set_error (error, MOO_FILE_ERROR,
                 MOO_FILE_ERROR_NOT_IMPLEMENTED,
                 "don't know how to delete file '%s'", path);
    return FALSE;
}


static char     *get_parent_default     (G_GNUC_UNUSED MooFileSystem *fs,
                                         const char     *path,
                                         GError        **error)
{
    g_return_val_if_fail (path != NULL, NULL);
    g_return_val_if_fail (g_path_is_absolute (path), NULL);
    g_set_error (error, MOO_FILE_ERROR,
                 MOO_FILE_ERROR_NOT_IMPLEMENTED,
                 "don't know how to delete file '%s'", path);
    return NULL;
}


static char     *make_path_default      (G_GNUC_UNUSED MooFileSystem *fs,
                                         const char     *base_path,
                                         const char     *display_name,
                                         GError        **error)
{
    g_return_val_if_fail (base_path != NULL, NULL);
    g_return_val_if_fail (display_name != NULL, NULL);
    g_return_val_if_fail (g_path_is_absolute (base_path), NULL);

    g_set_error (error, MOO_FILE_ERROR,
                 MOO_FILE_ERROR_NOT_IMPLEMENTED,
                 "don't know how to make path of '%s' and '%s'",
                 base_path, display_name);
    return NULL;
}


GQuark  moo_file_error_quark (void)
{
    static GQuark quark = 0;
    if (quark == 0)
        quark = g_quark_from_static_string ("moo-file-error-quark");
    return quark;
}
