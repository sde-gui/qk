/*
 *   moofilesystem.c
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

#define MOO_FILE_SYSTEM_COMPILATION
#include "moofilesystem.h"
#include "mooutils/mooutils-fs.h"
#include "mooutils/moomarshals.h"
#include <string.h>
#include <errno.h>
#include <stdio.h>
#ifndef __WIN32__
#include <sys/wait.h>
#else
#include <io.h>
#endif

#define BROKEN_NAME "<" "????" ">"

struct _MooFileSystemPrivate {
    GHashTable *folders;
    MooFileWatch *fam;
};

static MooFileSystem *fs_instance = NULL;


static void moo_file_system_finalize    (GObject        *object);

static MooFolder   *get_folder              (MooFileSystem  *fs,
                                             const char     *path,
                                             MooFileFlags    wanted,
                                             GError        **error);
static gboolean     create_folder           (MooFileSystem  *fs,
                                             const char     *path,
                                             GError        **error);
static MooFolder   *get_parent_folder       (MooFileSystem  *fs,
                                             MooFolder      *folder,
                                             MooFileFlags    flags);
static gboolean     delete_file             (MooFileSystem  *fs,
                                             const char     *path,
                                             gboolean        recursive,
                                             GError        **error);

#ifndef __WIN32__
static MooFolder   *get_root_folder_unix    (MooFileSystem  *fs,
                                             MooFileFlags    wanted);
static gboolean     move_file_unix          (MooFileSystem  *fs,
                                             const char     *old_path,
                                             const char     *new_path,
                                             GError        **error);
static char        *normalize_path_unix     (MooFileSystem  *fs,
                                             const char     *path,
                                             gboolean        is_folder,
                                             GError        **error);
static char        *make_path_unix          (MooFileSystem  *fs,
                                             const char     *base_path,
                                             const char     *display_name,
                                             GError        **error);
static gboolean     parse_path_unix         (MooFileSystem  *fs,
                                             const char     *path_utf8,
                                             char          **dirname,
                                             char          **display_dirname,
                                             char          **display_basename,
                                             GError        **error);
static char        *get_absolute_path_unix  (MooFileSystem  *fs,
                                             const char     *display_name,
                                             const char     *current_dir);

#else /* __WIN32__ */

static MooFolder   *get_root_folder_win32   (MooFileSystem  *fs,
                                             MooFileFlags    wanted);
static gboolean     move_file_win32         (MooFileSystem  *fs,
                                             const char     *old_path,
                                             const char     *new_path,
                                             GError        **error);
static char        *normalize_path_win32    (MooFileSystem  *fs,
                                             const char     *path,
                                             gboolean        is_folder,
                                             GError        **error);
static char        *make_path_win32         (MooFileSystem  *fs,
                                             const char     *base_path,
                                             const char     *display_name,
                                             GError        **error);
static gboolean     parse_path_win32        (MooFileSystem  *fs,
                                             const char     *path_utf8,
                                             char          **dirname,
                                             char          **display_dirname,
                                             char          **display_basename,
                                             GError        **error);
static char        *get_absolute_path_win32 (MooFileSystem  *fs,
                                             const char     *display_name,
                                             const char     *current_dir);
#endif /* __WIN32__ */


/* MOO_TYPE_FILE_SYSTEM */
G_DEFINE_TYPE (MooFileSystem, _moo_file_system, G_TYPE_OBJECT)


static void
_moo_file_system_class_init (MooFileSystemClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_file_system_finalize;

    klass->get_folder = get_folder;
    klass->create_folder = create_folder;
    klass->get_parent_folder = get_parent_folder;
    klass->delete_file = delete_file;

#ifdef __WIN32__
    klass->get_root_folder = get_root_folder_win32;
    klass->move_file = move_file_win32;
    klass->normalize_path = normalize_path_win32;
    klass->make_path = make_path_win32;
    klass->parse_path = parse_path_win32;
    klass->get_absolute_path = get_absolute_path_win32;
#else /* !__WIN32__ */
    klass->get_root_folder = get_root_folder_unix;
    klass->move_file = move_file_unix;
    klass->normalize_path = normalize_path_unix;
    klass->make_path = make_path_unix;
    klass->parse_path = parse_path_unix;
    klass->get_absolute_path = get_absolute_path_unix;
#endif /* !__WIN32__ */
}


static void
_moo_file_system_init (MooFileSystem *fs)
{
    fs->priv = g_new0 (MooFileSystemPrivate, 1);

    fs->priv->folders = g_hash_table_new_full (g_str_hash, g_str_equal,
                                               g_free, g_object_unref);
}


static void
moo_file_system_finalize (GObject *object)
{
    MooFileSystem *fs = MOO_FILE_SYSTEM (object);

    g_hash_table_destroy (fs->priv->folders);

    if (fs->priv->fam)
    {
        moo_file_watch_close (fs->priv->fam, NULL);
        g_object_unref (fs->priv->fam);
    }

    g_free (fs->priv);
    fs->priv = NULL;

    G_OBJECT_CLASS (_moo_file_system_parent_class)->finalize (object);
}


MooFileSystem *
_moo_file_system_create (void)
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


MooFolder *
_moo_file_system_get_root_folder (MooFileSystem  *fs,
                                  MooFileFlags    wanted)
{
    g_return_val_if_fail (MOO_IS_FILE_SYSTEM (fs), NULL);
    return MOO_FILE_SYSTEM_GET_CLASS(fs)->get_root_folder (fs, wanted);
}


MooFolder *
_moo_file_system_get_folder (MooFileSystem  *fs,
                             const char     *path,
                             MooFileFlags    wanted,
                             GError        **error)
{
    g_return_val_if_fail (MOO_IS_FILE_SYSTEM (fs), NULL);
    return MOO_FILE_SYSTEM_GET_CLASS(fs)->get_folder (fs, path, wanted, error);
}


MooFolder *
_moo_file_system_get_parent_folder (MooFileSystem  *fs,
                                    MooFolder      *folder,
                                    MooFileFlags    wanted)
{
    g_return_val_if_fail (MOO_IS_FILE_SYSTEM (fs), NULL);
    g_return_val_if_fail (MOO_IS_FOLDER (folder), NULL);
    return MOO_FILE_SYSTEM_GET_CLASS(fs)->get_parent_folder (fs, folder, wanted);
}


gboolean
_moo_file_system_create_folder (MooFileSystem  *fs,
                                const char     *path,
                                GError        **error)
{
    g_return_val_if_fail (MOO_IS_FILE_SYSTEM (fs), FALSE);
    g_return_val_if_fail (path != NULL, FALSE);
    return MOO_FILE_SYSTEM_GET_CLASS(fs)->create_folder (fs, path, error);
}


gboolean
_moo_file_system_delete_file (MooFileSystem  *fs,
                              const char     *path,
                              gboolean        recursive,
                              GError        **error)
{
    g_return_val_if_fail (MOO_IS_FILE_SYSTEM (fs), FALSE);
    g_return_val_if_fail (path != NULL, FALSE);
    return MOO_FILE_SYSTEM_GET_CLASS(fs)->delete_file (fs, path, recursive, error);
}


gboolean
_moo_file_system_move_file (MooFileSystem  *fs,
                            const char     *old_path,
                            const char     *new_path,
                            GError        **error)
{
    g_return_val_if_fail (MOO_IS_FILE_SYSTEM (fs), FALSE);
    g_return_val_if_fail (old_path && new_path, FALSE);
    return MOO_FILE_SYSTEM_GET_CLASS(fs)->move_file (fs, old_path, new_path, error);
}


char *
_moo_file_system_make_path (MooFileSystem  *fs,
                            const char     *base_path,
                            const char     *display_name,
                            GError        **error)
{
    g_return_val_if_fail (MOO_IS_FILE_SYSTEM (fs), FALSE);
    g_return_val_if_fail (base_path != NULL && display_name != NULL, FALSE);
    return MOO_FILE_SYSTEM_GET_CLASS(fs)->make_path (fs, base_path, display_name, error);
}


char *
_moo_file_system_normalize_path (MooFileSystem  *fs,
                                 const char     *path,
                                 gboolean        is_folder,
                                 GError        **error)
{
    g_return_val_if_fail (MOO_IS_FILE_SYSTEM (fs), FALSE);
    g_return_val_if_fail (path != NULL, FALSE);
    return MOO_FILE_SYSTEM_GET_CLASS(fs)->normalize_path (fs, path, is_folder, error);
}


gboolean
_moo_file_system_parse_path (MooFileSystem  *fs,
                             const char     *path_utf8,
                             char          **dirname,
                             char          **display_dirname,
                             char          **display_basename,
                             GError        **error)
{
    g_return_val_if_fail (MOO_IS_FILE_SYSTEM (fs), FALSE);
    g_return_val_if_fail (path_utf8 != NULL, FALSE);
    return MOO_FILE_SYSTEM_GET_CLASS(fs)->parse_path (fs, path_utf8, dirname,
                                                      display_dirname, display_basename,
                                                      error);
}


char *
_moo_file_system_get_absolute_path (MooFileSystem  *fs,
                                    const char     *display_name,
                                    const char     *current_dir)
{
    g_return_val_if_fail (MOO_IS_FILE_SYSTEM (fs), NULL);
    g_return_val_if_fail (display_name != NULL, NULL);
    return MOO_FILE_SYSTEM_GET_CLASS(fs)->get_absolute_path (fs, display_name, current_dir);
}


static void
fam_error (MooFileWatch   *fam,
           GError         *error,
           MooFileSystem  *fs)
{
    g_return_if_fail (fs->priv->fam == fam);
    g_warning ("%s: fam error", G_STRLOC);
    g_warning ("%s: %s", G_STRLOC, error->message);
    g_object_unref (fs->priv->fam);
    fs->priv->fam = NULL;
}


MooFileWatch *
_moo_file_system_get_file_watch (MooFileSystem *fs)
{
    g_return_val_if_fail (MOO_IS_FILE_SYSTEM (fs), NULL);

    if (!fs->priv->fam)
    {
        GError *error = NULL;
        fs->priv->fam = moo_file_watch_new (&error);
        if (!fs->priv->fam)
        {
            g_warning ("%s: moo_fam_open failed", G_STRLOC);
            g_warning ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }
        else
        {
            g_signal_connect (fs->priv->fam, "error",
                              G_CALLBACK (fam_error), fs);
        }
    }

    return fs->priv->fam;
}


/* TODO what's this? */
static void
folder_deleted (MooFolder      *folder,
                MooFileSystem  *fs)
{
    g_signal_handlers_disconnect_by_func (folder,
                                          (gpointer) folder_deleted, fs);
    g_hash_table_remove (fs->priv->folders, _moo_folder_get_path (folder));
}


MooFolder *
get_folder (MooFileSystem  *fs,
            const char     *path,
            MooFileFlags    wanted,
            GError        **error)
{
    MooFolder *folder;
    char *norm_path = NULL;

    g_return_val_if_fail (path != NULL, NULL);

#ifdef __WIN32__
    if (!*path)
        return get_root_folder_win32 (fs, wanted);
#endif /* __WIN32__ */

    /* XXX check the caller */
    if (!g_path_is_absolute (path))
    {
        g_set_error (error, MOO_FILE_ERROR,
                     MOO_FILE_ERROR_BAD_FILENAME,
                     "folder path '%s' is not absolute",
                     path);
        return NULL;
    }

    norm_path = _moo_file_system_normalize_path (fs, path, TRUE, error);

    if (!norm_path)
        return NULL;

    folder = g_hash_table_lookup (fs->priv->folders, norm_path);

    if (folder)
    {
        _moo_folder_set_wanted (folder, wanted, TRUE);
        g_object_ref (folder);
        goto out;
    }

    if (!g_file_test (norm_path, G_FILE_TEST_EXISTS))
    {
        g_set_error (error, MOO_FILE_ERROR,
                     MOO_FILE_ERROR_NONEXISTENT,
                     "'%s' does not exist", norm_path);
        folder = NULL;
        goto out;
    }

    if (!g_file_test (norm_path, G_FILE_TEST_IS_DIR))
    {
        g_set_error (error, MOO_FILE_ERROR,
                     MOO_FILE_ERROR_NOT_FOLDER,
                     "'%s' is not a folder", norm_path);
        folder = NULL;
        goto out;
    }

    folder = _moo_folder_new (fs, norm_path, wanted, error);

    if (folder)
    {
        g_hash_table_insert (fs->priv->folders,
                             norm_path,
                             g_object_ref (folder));
        g_signal_connect (folder, "deleted",
                          G_CALLBACK (folder_deleted), fs);
        norm_path = NULL;
    }

out:
    g_free (norm_path);
    return folder;
}


/* TODO */
gboolean
create_folder (G_GNUC_UNUSED MooFileSystem *fs,
               const char     *path,
               GError        **error)
{
    g_return_val_if_fail (path != NULL, FALSE);

    /* XXX check the caller */
    if (!g_path_is_absolute (path))
    {
        g_set_error (error, MOO_FILE_ERROR,
                     MOO_FILE_ERROR_BAD_FILENAME,
                     "folder path '%s' is not absolute",
                     path);
        return FALSE;
    }

    /* TODO mkdir must (?) adjust permissions according to umask */
#ifndef __WIN32__
    if (mkdir (path, S_IRWXU | S_IRWXG | S_IRWXO))
#else
    if (_m_mkdir (path))
#endif
    {
        int saved_errno = errno;
        g_set_error (error, MOO_FILE_ERROR,
                     _moo_file_error_from_errno (saved_errno),
                     "%s", g_strerror (saved_errno));
        return FALSE;
    }

    return TRUE;
}


/***************************************************************************/
/* common methods
 */

/* folder may be deleted, but this function returns parent
   folder anyway, if that exists */
static MooFolder *
get_parent_folder (MooFileSystem  *fs,
                   MooFolder      *folder,
                   MooFileFlags    wanted)
{
    char *parent_path;
    MooFolder *parent;

    g_return_val_if_fail (MOO_IS_FILE_SYSTEM (fs), NULL);
    g_return_val_if_fail (MOO_IS_FOLDER (folder), NULL);
    g_return_val_if_fail (_moo_folder_get_file_system (folder) == fs, NULL);

    parent_path = g_strdup_printf ("%s" G_DIR_SEPARATOR_S "..",
                                   _moo_folder_get_path (folder));

    parent = _moo_file_system_get_folder (fs, parent_path, wanted, NULL);

    g_free (parent_path);
    return parent;
}


static char *
normalize_path (const char *path)
{
    GPtrArray *comps;
    gboolean first_slash;
    char **pieces, **p;
    char *normpath;

    g_return_val_if_fail (path != NULL, NULL);

    first_slash = (path[0] == G_DIR_SEPARATOR);

    pieces = g_strsplit (path, G_DIR_SEPARATOR_S, 0);
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
            normpath = g_strdup (G_DIR_SEPARATOR_S);
        else
            normpath = g_strdup (".");
    }
    else
    {
        char *tmp = g_strjoinv (G_DIR_SEPARATOR_S, (char**) comps->pdata);

        if (first_slash)
        {
            guint len = strlen (tmp);
            normpath = g_new (char, len + 2);
            memcpy (normpath + 1, tmp, len + 1);
            normpath[0] = G_DIR_SEPARATOR;
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


gboolean
delete_file (G_GNUC_UNUSED MooFileSystem *fs,
             const char     *path,
             gboolean        recursive,
             GError        **error)
{
    gboolean isdir;

    g_return_val_if_fail (path != NULL, FALSE);
    g_return_val_if_fail (g_path_is_absolute (path), FALSE);

    if (g_file_test (path, G_FILE_TEST_IS_SYMLINK))
        isdir = FALSE;
    else
        isdir = g_file_test (path, G_FILE_TEST_IS_DIR);

    if (isdir)
        return _moo_rmdir (path, recursive, error);

    if (_m_remove (path))
    {
        int err = errno;
        char *path_utf8 = g_filename_to_utf8 (path, -1, NULL, NULL, NULL);
        g_set_error (error, MOO_FILE_ERROR,
                     _moo_file_error_from_errno (err),
                     "Could not delete file '%s': %s",
                     path_utf8 ? path_utf8 : BROKEN_NAME,
                     g_strerror (err));
        g_free (path_utf8);
        return FALSE;
    }

    return TRUE;
}


/***************************************************************************/
/* UNIX methods
 */
#ifndef __WIN32__

static MooFolder *
get_root_folder_unix (MooFileSystem  *fs,
                      MooFileFlags    wanted)
{
    return _moo_file_system_get_folder (fs, "/", wanted, NULL);
}


static gboolean
move_file_unix (G_GNUC_UNUSED MooFileSystem *fs,
                const char     *old_path,
                const char     *new_path,
                GError        **error)
{
    g_return_val_if_fail (old_path && new_path, FALSE);
    g_return_val_if_fail (g_path_is_absolute (old_path), FALSE);
    g_return_val_if_fail (g_path_is_absolute (new_path), FALSE);

    /* XXX */
    if (_m_rename (old_path, new_path))
    {
        int saved_errno = errno;
        g_set_error (error, MOO_FILE_ERROR,
                     _moo_file_error_from_errno (saved_errno),
                     "%s", g_strerror (saved_errno));
        return FALSE;
    }

    return TRUE;
}


static char *
make_path_unix (G_GNUC_UNUSED MooFileSystem *fs,
                const char     *base_path,
                const char     *display_name,
                GError        **error)
{
    GError *error_here = NULL;
    char *path, *name;

    g_return_val_if_fail (base_path != NULL, NULL);
    g_return_val_if_fail (display_name != NULL, NULL);

    /* XXX check the caller */
    if (!g_path_is_absolute (base_path))
    {
        g_set_error (error, MOO_FILE_ERROR,
                     MOO_FILE_ERROR_BAD_FILENAME,
                     "path '%s' is not absolute",
                     base_path);
        return NULL;
    }

    name = g_filename_from_utf8 (display_name, -1, NULL, NULL, &error_here);

    if (error_here)
    {
        g_set_error (error, MOO_FILE_ERROR,
                     MOO_FILE_ERROR_BAD_FILENAME,
                     "Could not convert '%s' to filename encoding: %s",
                     display_name, error_here->message);
        g_free (name);
        g_error_free (error_here);
        return NULL;
    }

    path = g_strdup_printf ("%s/%s", base_path, name);
    g_free (name);
    return path;
}


/* TODO: error checking, etc. */
static char *
normalize_path_unix (G_GNUC_UNUSED MooFileSystem *fs,
                     const char     *path,
                     gboolean        is_folder,
                     G_GNUC_UNUSED GError **error)
{
    guint len;
    char *normpath, *tmp;

    g_return_val_if_fail (path != NULL, NULL);

    tmp = normalize_path (path);

    if (!is_folder)
        return tmp;

    len = strlen (tmp);
    g_return_val_if_fail (len > 0, tmp);

    if (tmp[len-1] != G_DIR_SEPARATOR)
    {
        normpath = g_new (char, len + 2);
        memcpy (normpath, tmp, len);
        normpath[len] = G_DIR_SEPARATOR;
        normpath[len+1] = 0;
        g_free (tmp);
    }
    else
    {
//         g_assert (len == 1);
        normpath = tmp;
    }

#if 0
    g_print ("path: '%s'\nnormpath: '%s'\n",
             path, normpath);
#endif

    return normpath;
}


/* XXX must set error */
static gboolean
parse_path_unix (MooFileSystem  *fs,
                 const char     *path_utf8,
                 char          **dirname_p,
                 char          **display_dirname_p,
                 char          **display_basename_p,
                 GError        **error)
{
    const char *separator;
    char *dirname = NULL, *norm_dirname = NULL;
    char *display_dirname = NULL, *display_basename = NULL;

    g_return_val_if_fail (path_utf8 && path_utf8[0], FALSE);

    /* XXX check the caller */
    if (!g_path_is_absolute (path_utf8))
    {
        g_set_error (error, MOO_FILE_ERROR,
                     MOO_FILE_ERROR_BAD_FILENAME,
                     "path '%s' is not absolute",
                     path_utf8);
        return FALSE;
    }

    if (!strcmp (path_utf8, "/"))
    {
        display_dirname = g_strdup ("/");
        display_basename = g_strdup ("");
        norm_dirname = g_strdup ("/");
        goto success;
    }

    separator = strrchr (path_utf8, '/');
    g_return_val_if_fail (separator != NULL, FALSE);

    display_dirname = g_strndup (path_utf8, separator - path_utf8 + 1);
    display_basename = g_strdup (separator + 1);
    dirname = g_filename_from_utf8 (display_dirname, -1, NULL, NULL, error);

    if (!dirname)
        goto error_label;

    norm_dirname = _moo_file_system_normalize_path (fs, dirname, TRUE, error);

    if (!norm_dirname)
        goto error_label;
    else
        goto success;

    /* no fallthrough */
    g_assert_not_reached ();

error_label:
    g_free (dirname);
    g_free (norm_dirname);
    g_free (display_dirname);
    g_free (display_basename);
    return FALSE;

success:
    g_clear_error (error);
    g_free (dirname);
    *dirname_p = norm_dirname;
    *display_dirname_p = display_dirname;
    *display_basename_p = display_basename;
    return TRUE;
}


/* XXX unicode */
static char *
get_absolute_path_unix (G_GNUC_UNUSED MooFileSystem *fs,
                        const char     *short_name,
                        const char     *current_dir)
{
    g_return_val_if_fail (short_name && short_name[0], NULL);

    if (short_name[0] == '~')
    {
        const char *home = g_get_home_dir ();
        g_return_val_if_fail (home != NULL, NULL);

        if (short_name[1])
            return g_build_filename (home, short_name + 1, NULL);
        else
            return g_strdup (home);
    }

    if (g_path_is_absolute (short_name))
        return g_strdup (short_name);

    if (current_dir)
        return g_build_filename (current_dir, short_name, NULL);

    return NULL;
}

#endif /* !__WIN32__ */


/***************************************************************************/
/* Win32 methods
 */
#ifdef __WIN32__

static MooFolder *
get_root_folder_win32 (MooFileSystem  *fs,
                       MooFileFlags    wanted)
{
#ifdef __GNUC__
#warning "Implement me"
#endif
    return _moo_file_system_get_folder (fs, "c:\\", wanted, NULL);
}


static gboolean
move_file_win32 (G_GNUC_UNUSED MooFileSystem  *fs,
                 G_GNUC_UNUSED const char *old_path,
                 G_GNUC_UNUSED const char *new_path,
                 GError        **error)
{
#ifdef __GNUC__
#warning "Implement me"
#endif
    g_set_error (error, MOO_FILE_ERROR,
                 MOO_FILE_ERROR_NOT_IMPLEMENTED,
                 "Renaming files is not implemented on win32");
    return FALSE;
}


static void
splitdrive (const char *fullpath,
            char      **drive,
            char      **path)
{
    if (fullpath[0] && fullpath[1] == ':')
    {
        *drive = g_strndup (fullpath, 2);
        *path = g_strdup (fullpath + 2);
    }
    else
    {
        *drive = NULL;
        *path = g_strdup (fullpath);
    }
}


static char *
normalize_path_win32 (G_GNUC_UNUSED MooFileSystem *fs,
                      const char     *fullpath,
                      gboolean        isdir,
                      G_GNUC_UNUSED GError **error)
{
    char *drive, *path, *normpath;
    guint slashes;

    g_return_val_if_fail (fullpath != NULL, NULL);

    splitdrive (fullpath, &drive, &path);
    g_strdelimit (path, "/", '\\');

    for (slashes = 0; path[slashes] == '\\'; ++slashes) ;

    if (drive && path[0] != '\\')
    {
        char *tmp = path;
        path = g_strdup_printf ("\\%s", path);
        g_free (tmp);
    }

    if (!drive)
    {
        char *tmp = path;
        drive = g_strndup (path, slashes);
        path = g_strdup (tmp + slashes);
        g_free (tmp);
    }
//     else if (path[0] == '\\')
//     {
//         char *tmp;
//
//         tmp = drive;
//         drive = g_strdup_printf ("%s\\", drive);
//         g_free (tmp);
//
//         tmp = path;
//         path = g_strdup (path + slashes);
//         g_free (tmp);
//     }

    normpath = normalize_path (path);

    if (!normpath[0] && !drive)
    {
        char *tmp = normpath;
        normpath = g_strdup (".");
        g_free (tmp);
    }
    else if (drive)
    {
        char *tmp = normpath;
        normpath = g_strdup_printf ("%s%s", drive, normpath);
        g_free (tmp);
    }

    if (isdir)
    {
        guint len = strlen (normpath);

        if (!len || normpath[len -1] != '\\')
        {
            char *tmp = normpath;
            normpath = g_strdup_printf ("%s\\", normpath);
            g_free (tmp);
        }
    }

    g_free (drive);
    g_free (path);
    return normpath;
}


static char *
make_path_win32 (G_GNUC_UNUSED MooFileSystem *fs,
                 const char     *base_path,
                 const char     *display_name,
                 G_GNUC_UNUSED GError **error)
{
    g_return_val_if_fail (g_path_is_absolute (base_path), NULL);
    g_return_val_if_fail (display_name != NULL, NULL);
    return g_strdup_printf ("%s\\%s", base_path, display_name);
}


static gboolean
parse_path_win32 (MooFileSystem  *fs,
                  const char     *path_utf8,
                  char          **dirname_p,
                  char          **display_dirname_p,
                  char          **display_basename_p,
                  GError        **error)
{
#ifdef __GNUC__
#warning "Implement me"
#endif
    const char *separator;
    char *norm_dirname = NULL, *dirname = NULL, *basename = NULL;

    g_return_val_if_fail (path_utf8 && path_utf8[0], FALSE);
    g_return_val_if_fail (g_path_is_absolute (path_utf8), FALSE);

    separator = strrchr (path_utf8, '\\');
    g_return_val_if_fail (separator != NULL, FALSE);

    dirname = g_path_get_dirname (path_utf8);
    basename = g_path_get_basename (path_utf8);
    norm_dirname = _moo_file_system_normalize_path (fs, dirname, TRUE, error);

    if (!norm_dirname)
        goto error_label;
    else
        goto success;

    /* no fallthrough */
    g_assert_not_reached ();

error_label:
    g_free (dirname);
    g_free (basename);
    g_free (norm_dirname);
    return FALSE;

success:
    g_clear_error (error);
    *dirname_p = norm_dirname;
    *display_dirname_p = dirname;
    *display_basename_p = basename;
    return TRUE;
}


static char *
get_absolute_path_win32 (G_GNUC_UNUSED MooFileSystem *fs,
                         const char     *short_name,
                         const char     *current_dir)
{
    g_return_val_if_fail (short_name && short_name[0], NULL);

    if (g_path_is_absolute (short_name))
        return g_strdup (short_name);

    if (current_dir)
        return g_build_filename (current_dir, short_name, NULL);

    return NULL;
}

#endif /* __WIN32__ */
