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

static MooFolder   *get_folder_default          (MooFileSystem  *fs,
                                                 const char     *path,
                                                 MooFileFlags    wanted,
                                                 GError        **error);
static MooFolder   *get_parent_folder_default   (MooFileSystem  *fs,
                                                 MooFolder      *folder,
                                                 MooFileFlags    flags);

// static gboolean  create_folder_default  (MooFileSystem *fs,
//                                          const char     *path,
//                                          GError        **error);
// static gboolean  delete_file_default    (MooFileSystem  *fs,
//                                          const char     *path,
//                                          GError        **error);
// static char     *get_parent_default     (MooFileSystem  *fs,
//                                          const char     *path,
//                                          GError        **error);
// static char     *make_path_default      (MooFileSystem  *fs,
//                                          const char     *base_path,
//                                          const char     *display_name,
//                                          GError        **error);

static char     *normalize_folder_path  (const char     *path);
static char     *normalize_path         (const char     *path);
static char     *normalize_path_unix    (const char     *path);


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
    klass->get_parent_folder = get_parent_folder_default;
//     klass->create_folder = create_folder_default;
//     klass->delete_file = delete_file_default;
//     klass->get_parent = get_parent_default;
//     klass->make_path = make_path_default;
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
    g_return_val_if_fail (MOO_IS_FILE_SYSTEM (fs), NULL);
    return MOO_FILE_SYSTEM_GET_CLASS(fs)->get_folder (fs, path, wanted, error);
}


MooFolder   *moo_file_system_get_parent_folder  (MooFileSystem  *fs,
                                                 MooFolder      *folder,
                                                 MooFileFlags    wanted)
{
    g_return_val_if_fail (MOO_IS_FILE_SYSTEM (fs), NULL);
    g_return_val_if_fail (MOO_IS_FOLDER (folder), NULL);
    return MOO_FILE_SYSTEM_GET_CLASS(fs)->get_parent_folder (fs, folder, wanted);
}


// gboolean         moo_file_system_create_folder (MooFileSystem *fs,
//                                              const char     *path,
//                                              GError        **error)
// {
//     return MOO_FILE_SYSTEM_GET_CLASS(fs)->create_folder (fs, path, error);
// }


// gboolean         moo_file_system_delete_file(MooFileSystem  *fs,
//                                              const char     *path,
//                                              GError        **error)
// {
//     return MOO_FILE_SYSTEM_GET_CLASS(fs)->delete_file (fs, path, error);
// }


// char            *moo_file_system_get_parent (MooFileSystem  *fs,
//                                              const char     *path,
//                                              GError        **error)
// {
//     return MOO_FILE_SYSTEM_GET_CLASS(fs)->get_parent (fs, path, error);
// }


// char            *moo_file_system_make_path  (MooFileSystem  *fs,
//                                              const char     *base_path,
//                                              const char     *display_name,
//                                              GError        **error)
// {
//     return MOO_FILE_SYSTEM_GET_CLASS(fs)->make_path (fs, base_path, display_name, error);
// }


/***************************************************************************/
/* Default (UNIX) methods
 */

MooFolder       *get_folder_default     (MooFileSystem  *fs,
                                         const char     *path,
                                         MooFileFlags    wanted,
                                         GError        **error)
{
    MooFolder *folder;
    char *norm_path;

    g_return_val_if_fail (path != NULL, NULL);

    /* TODO: this is not gonna work on windows, where '' is the 'root',
               containing drives */
    g_return_val_if_fail (g_path_is_absolute (path), NULL);

    norm_path = normalize_folder_path (path);
    g_return_val_if_fail (norm_path != NULL, NULL);

    folder = g_hash_table_lookup (fs->priv->folders, norm_path);

    if (folder)
    {
        moo_folder_set_wanted (folder, wanted, TRUE);
        g_object_ref (folder);
        return folder;
    }

    if (!g_file_test (norm_path, G_FILE_TEST_IS_DIR))
    {
        g_set_error (error, MOO_FILE_ERROR,
                     MOO_FILE_ERROR_NOT_FOLDER,
                     "'%s' is not a folder", norm_path);
        g_free (norm_path);
        return NULL;
    }

    folder = moo_folder_new (fs, norm_path, wanted, error);

    if (folder)
        g_hash_table_insert (fs->priv->folders,
                             g_strdup (norm_path),
                             g_object_ref (folder));

    g_free (norm_path);
    return folder;
}


static MooFolder   *get_parent_folder_default   (MooFileSystem  *fs,
                                                 MooFolder      *folder,
                                                 MooFileFlags    wanted)
{
    char *parent_path;
    MooFolder *parent;

    g_return_val_if_fail (MOO_IS_FILE_SYSTEM (fs), NULL);
    g_return_val_if_fail (MOO_IS_FOLDER (folder), NULL);
    g_return_val_if_fail (moo_folder_get_file_system (folder) == fs, NULL);

    parent_path = g_strdup_printf ("%s/..", moo_folder_get_path (folder));

    parent = moo_file_system_get_folder (fs, parent_path, wanted, NULL);
    g_assert (parent != NULL);

    g_free (parent_path);

    return parent;
}


// static gboolean  create_folder_default  (G_GNUC_UNUSED MooFileSystem *fs,
//                                          const char     *path,
//                                          GError        **error)
// {
//     g_return_val_if_fail (path != NULL, FALSE);
//     g_return_val_if_fail (g_path_is_absolute (path), FALSE);
//     g_set_error (error, MOO_FILE_ERROR,
//                  MOO_FILE_ERROR_NOT_IMPLEMENTED,
//                  "don't know how to create folder '%s'", path);
//     return FALSE;
// }
//
//
// static gboolean  delete_file_default    (G_GNUC_UNUSED MooFileSystem *fs,
//                                          const char     *path,
//                                          GError        **error)
// {
//     g_return_val_if_fail (path != NULL, FALSE);
//     g_return_val_if_fail (g_path_is_absolute (path), FALSE);
//     g_set_error (error, MOO_FILE_ERROR,
//                  MOO_FILE_ERROR_NOT_IMPLEMENTED,
//                  "don't know how to delete file '%s'", path);
//     return FALSE;
// }
//
//
// static char     *get_parent_default     (G_GNUC_UNUSED MooFileSystem *fs,
//                                          const char     *path,
//                                          GError        **error)
// {
//     g_return_val_if_fail (path != NULL, NULL);
//     g_return_val_if_fail (g_path_is_absolute (path), NULL);
//     g_set_error (error, MOO_FILE_ERROR,
//                  MOO_FILE_ERROR_NOT_IMPLEMENTED,
//                  "don't know how to delete file '%s'", path);
//     return NULL;
// }
//
//
// static char     *make_path_default      (G_GNUC_UNUSED MooFileSystem *fs,
//                                          const char     *base_path,
//                                          const char     *display_name,
//                                          GError        **error)
// {
//     g_return_val_if_fail (base_path != NULL, NULL);
//     g_return_val_if_fail (display_name != NULL, NULL);
//     g_return_val_if_fail (g_path_is_absolute (base_path), NULL);
//
//     g_set_error (error, MOO_FILE_ERROR,
//                  MOO_FILE_ERROR_NOT_IMPLEMENTED,
//                  "don't know how to make path of '%s' and '%s'",
//                  base_path, display_name);
//     return NULL;
// }


GQuark  moo_file_error_quark (void)
{
    static GQuark quark = 0;
    if (quark == 0)
        quark = g_quark_from_static_string ("moo-file-error-quark");
    return quark;
}


static char     *normalize_path_unix    (const char     *path)
{
    GPtrArray *comps;
    gboolean first_slash;
    char **pieces, **p;
    char *normpath;

    g_return_val_if_fail (path != NULL, NULL);

    first_slash = (path[0] == '/');

    pieces = g_strsplit (path, "/", 0);
    g_return_val_if_fail (pieces != NULL, NULL);

    comps = g_ptr_array_new ();

    for (p = pieces; *p != NULL; ++p)
    {
        char *s = *p;
        gboolean push = TRUE;
        gboolean pop = FALSE;

        if (!strcmp (s, "") || !strcmp (s, "."))
        {
            push = FALSE;
        }
        else if (!strcmp (s, ".."))
        {
            if (!comps->len && first_slash)
            {
                push = FALSE;
            }
            else if (comps->len)
            {
                push = FALSE;
                pop = TRUE;
            }
        }

        if (pop)
        {
            g_free (comps->pdata[comps->len - 1]);
            g_ptr_array_remove_index (comps, comps->len - 1);
        }

        if (push)
            g_ptr_array_add (comps, g_strdup (s));
    }

    g_ptr_array_add (comps, NULL);

    if (comps->len == 1)
    {
        if (first_slash)
            normpath = g_strdup ("/");
        else
            normpath = g_strdup ("");
    }
    else
    {
        char *tmp = g_strjoinv ("/", (char**) comps->pdata);

        if (first_slash)
        {
            normpath = g_strdup_printf ("/%s", tmp);
            g_free (tmp);
        }
        else
        {
            normpath = tmp;
        }
    }

    g_strfreev (pieces);
    g_strfreev ((char**) comps->pdata);
    g_ptr_array_free (comps, FALSE);

    return normpath;
}


static char     *normalize_path         (const char     *path)
{
    return normalize_path_unix (path);
}


static char     *normalize_folder_path  (const char     *path)
{
    guint len;
    char *normpath, *tmp;

    g_return_val_if_fail (path != NULL, NULL);

    tmp = normalize_path (path);
    len = strlen (tmp);
    g_return_val_if_fail (len > 0, tmp);

    if (tmp[len-1] != '/')
    {
        normpath = g_strdup_printf ("%s/", tmp);
        g_free (tmp);
    }
    else
    {
        g_assert (len == 1);
        normpath = tmp;
    }

    g_print ("path: '%s'\nnormpath: '%s'\n",
             path, normpath);

    return normpath;
}
