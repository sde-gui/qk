/*
 *   mooutils-fs.c
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "mooutils/mooutils-fs.h"
#include "mooutils/mooutils-file.h"
#include "mooutils/mooutils-debug.h"
#include "mooutils/mooutils-mem.h"
#include "mooutils/mootype-macros.h"
#include "mooutils/moocompat.h"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <glib/gstdio.h>
/* for glib-2.6 */
#include <glib/gmappedfile.h>

#ifdef __WIN32__
#include <windows.h>
#include <io.h>
#include <direct.h>
#endif

#ifndef S_IRWXU
#define S_IRWXU 0
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#define BROKEN_NAME "<" "????" ">"


/* XXX fix this */
gboolean
_moo_save_file_utf8 (const char *name,
                     const char *text,
                     gssize      len,
                     GError    **error)
{
    GIOChannel *file;
    GIOStatus status;
    gsize bytes_written;
    gsize real_len;

    file = g_io_channel_new_file (name, "w", error);

    if (!file)
        return FALSE;

    real_len = len < 0 ? strlen (text) : (gsize) len;

    status = g_io_channel_write_chars (file, text,
                                       len, &bytes_written,
                                       error);

    if (status != G_IO_STATUS_NORMAL || bytes_written != real_len)
    {
        /* glib #320668 */
        g_io_channel_flush (file, NULL);
        g_io_channel_shutdown (file, FALSE, NULL);
        g_io_channel_unref (file);
        return FALSE;
    }

    /* glib #320668 */
    g_io_channel_flush (file, NULL);
    g_io_channel_shutdown (file, FALSE, NULL);
    g_io_channel_unref (file);
    return TRUE;
}


#ifndef __WIN32__
static gboolean
rm_fr (const char *path,
       GError    **error)
{
    GError *error_here = NULL;
    char **argv;
    char *child_err;
    int status;

    argv = g_new0 (char*, 5);
    argv[0] = g_strdup ("/bin/rm");
    argv[1] = g_strdup ("-fr");
    argv[2] = g_strdup ("--");
    argv[3] = g_strdup (path);

    if (!g_spawn_sync (NULL, argv, NULL, G_SPAWN_STDOUT_TO_DEV_NULL,
                       NULL, NULL, NULL, &child_err, &status, &error_here))
    {
        g_set_error (error, MOO_FILE_ERROR, MOO_FILE_ERROR_FAILED,
                     "Could not run 'rm' command: %s",
                     error_here->message);
        g_error_free (error_here);
        g_strfreev (argv);
        return FALSE;
    }

    g_strfreev (argv);

    if (!WIFEXITED (status) || WEXITSTATUS (status))
    {
        g_set_error (error, MOO_FILE_ERROR,
                     MOO_FILE_ERROR_FAILED,
                     "'rm' command failed: %s",
                     child_err ? child_err : "");
        g_free (child_err);
        return FALSE;
    }
    else
    {
        g_free (child_err);
        return TRUE;
    }
}
#endif /* __WIN32__ */


#ifdef __WIN32__
static gboolean
rm_r (const char *path,
      GError    **error)
{
    GDir *dir;
    const char *file;
    gboolean success = TRUE;

    g_return_val_if_fail (path != NULL, FALSE);

    dir = g_dir_open (path, 0, error);

    if (!dir)
        return FALSE;

    while (success && (file = g_dir_read_name (dir)))
    {
        char *file_path = g_build_filename (path, file, NULL);

        if (g_file_test (file_path, G_FILE_TEST_IS_DIR))
        {
            if (!rm_r (file_path, error))
                success = FALSE;
        }
        else
        {
            errno = 0;

            if (_moo_remove (file_path) != 0)
            {
                int err = errno;
                success = FALSE;
                g_set_error (error, MOO_FILE_ERROR,
                             _moo_file_error_from_errno (err),
                             "Could not remove '%s': %s", file_path,
                             g_strerror (err));
            }
        }

        g_free (file_path);
    }

    g_dir_close (dir);

    if (success)
    {
        errno = 0;

        if (_moo_remove (path) != 0)
        {
            int err = errno;
            success = FALSE;
            g_set_error (error, MOO_FILE_ERROR,
                         _moo_file_error_from_errno (err),
                         "Could not remove '%s': %s", path,
                         g_strerror (err));
        }
    }

    return success;
}
#endif


gboolean
_moo_remove_dir (const char *path,
                 gboolean    recursive,
                 GError    **error)
{
    g_return_val_if_fail (path != NULL, FALSE);

    if (!recursive)
    {
        errno = 0;

        if (_moo_remove (path) != 0)
        {
            int err = errno;
            char *path_utf8 = g_filename_display_name (path);
            g_set_error (error, MOO_FILE_ERROR,
                         _moo_file_error_from_errno (err),
                         "Could not remove '%s': %s",
                         path_utf8, g_strerror (err));
            g_free (path_utf8);
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }

#ifndef __WIN32__
    return rm_fr (path, error);
#else
    return rm_r (path, error);
#endif
}


int
_moo_mkdir_with_parents (const char *path)
{
    return g_mkdir_with_parents (path, S_IRWXU);
}


gboolean
_moo_create_dir (const char *path,
                 GError    **error)
{
    struct stat buf;
    char *utf8_path;

    g_return_val_if_fail (path != NULL, FALSE);

    errno = 0;

    if (g_stat (path, &buf) != 0 && errno != ENOENT)
    {
        int err_code = errno;
        utf8_path = g_filename_display_name (path);

        g_set_error (error,
                     MOO_FILE_ERROR,
                     _moo_file_error_from_errno (err_code),
                     "Could not create directory '%s': %s",
                     utf8_path, g_strerror (err_code));

        g_free (utf8_path);
        return FALSE;
    }

    if (errno != 0)
    {
        errno = 0;

        if (_moo_mkdir (path) == -1)
        {
            int err_code = errno;
            utf8_path = g_filename_display_name (path);

            g_set_error (error,
                         MOO_FILE_ERROR,
                         _moo_file_error_from_errno (err_code),
                         "Could not create directory '%s': %s",
                         utf8_path, g_strerror (err_code));

            g_free (utf8_path);
            return FALSE;
        }

        return TRUE;
    }

    if (S_ISDIR (buf.st_mode))
        return TRUE;

    utf8_path = g_filename_display_name (path);
    g_set_error (error, MOO_FILE_ERROR,
                 MOO_FILE_ERROR_ALREADY_EXISTS,
                 "Could not create directory '%s': %s",
                 utf8_path, g_strerror (EEXIST));
    g_free (utf8_path);

    return FALSE;
}


gboolean
_moo_rename_file (const char *path,
                  const char *new_path,
                  GError    **error)
{
    g_return_val_if_fail (path != NULL, FALSE);
    g_return_val_if_fail (new_path != NULL, FALSE);

    errno = 0;

    if (_moo_rename (path, new_path) != 0)
    {
        int err_code = errno;
        char *utf8_path = g_filename_display_name (path);
        char *utf8_new_path = g_filename_display_name (new_path);

        g_set_error (error,
                     MOO_FILE_ERROR,
                     _moo_file_error_from_errno (err_code),
                     "Could not rename file '%s' to '%s': %s",
                     utf8_path, utf8_new_path, g_strerror (err_code));

        g_free (utf8_path);
        g_free (utf8_new_path);
        return FALSE;
    }

    return TRUE;
}


GQuark
_moo_file_error_quark (void)
{
    static GQuark quark = 0;

    if (quark == 0)
        quark = g_quark_from_static_string ("moo-file-error-quark");

    return quark;
}


MooFileError
_moo_file_error_from_errno (int code)
{
    switch (code)
    {
        case EACCES:
        case EPERM:
            return MOO_FILE_ERROR_ACCESS_DENIED;
        case EEXIST:
            return MOO_FILE_ERROR_ALREADY_EXISTS;
#ifndef __WIN32__
        case ELOOP:
#endif
        case ENAMETOOLONG:
            return MOO_FILE_ERROR_BAD_FILENAME;
        case ENOENT:
            return MOO_FILE_ERROR_NONEXISTENT;
        case ENOTDIR:
            return MOO_FILE_ERROR_NOT_FOLDER;
        case EROFS:
            return MOO_FILE_ERROR_READONLY;
        case EXDEV:
            return MOO_FILE_ERROR_DIFFERENT_FS;
    }

    return MOO_FILE_ERROR_FAILED;
}


char **
moo_filenames_from_locale (char **files)
{
    guint i;
    char **conv;

    if (!files)
        return NULL;

    conv = g_new0 (char*, g_strv_length (files) + 1);

    for (i = 0; files && *files; ++files)
    {
        conv[i] = moo_filename_from_locale (*files);

        if (!conv[i])
            g_warning ("%s: could not convert '%s' to UTF8", G_STRLOC, *files);
        else
            ++i;
    }

    return conv;
}

char *
moo_filename_from_locale (const char *file)
{
    g_return_val_if_fail (file != NULL, NULL);
#ifdef __WIN32__
    return g_locale_to_utf8 (file, -1, NULL, NULL, NULL);
#else
    return g_strdup (file);
#endif
}


#if 0
static int
_moo_chdir (const char *path)
{
#ifdef __WIN32__
    CCALL_1 (_chdir, _wchdir, path);
#else
    return chdir (path);
#endif
}

// char *
// _moo_normalize_file_path (const char *filename)
// {
//     char *freeme = NULL;
//     char *working_dir, *basename, *dirname;
//     char *real_filename = NULL;
//
//     g_return_val_if_fail (filename != NULL, NULL);
//
//     working_dir = g_get_current_dir ();
//     g_return_val_if_fail (working_dir != NULL, g_strdup (filename));
//
//     if (!_moo_path_is_absolute (filename))
//     {
//         freeme = g_build_filename (working_dir, filename, NULL);
//         filename = freeme;
//     }
//
//     /* It's an error to call this function for a directory, so if
//      * it's the case, just return what we got. */
//     if (g_file_test (filename, G_FILE_TEST_IS_DIR) ||
//         /* and this totally screws up dirname/basename */
//         g_str_has_suffix (filename, G_DIR_SEPARATOR_S))
//     {
//         if (!freeme)
//             freeme = g_strdup (filename);
//         g_free (working_dir);
//         return freeme;
//     }
//
//     dirname = g_path_get_dirname (filename);
//     basename = g_path_get_basename (filename);
//     g_return_val_if_fail (dirname && basename, g_strdup (filename));
//
//     errno = 0;
//
//     if (_moo_chdir (dirname) != 0)
//     {
//         int err = errno;
//         g_warning ("%s: %s", G_STRLOC, g_strerror (err));
//     }
//     else
//     {
//         char *real_dirname = g_get_current_dir ();
//         real_filename = g_build_filename (real_dirname, basename, NULL);
//         g_free (real_dirname);
//
//         errno = 0;
//
//         if (_moo_chdir (working_dir) != 0)
//         {
//             int err = errno;
//             g_warning ("%s: %s", G_STRLOC, g_strerror (err));
//         }
//     }
//
//     if (!real_filename)
//         real_filename = g_strdup (filename);
//
//     g_free (freeme);
//     g_free (dirname);
//     g_free (basename);
//     g_free (working_dir);
//
//     return real_filename;
// }
#endif


#ifndef __WIN32__

static char *
normalize_path_string (const char *path)
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
            normpath = g_renew (char, tmp, len + 2);
            memmove (normpath + 1, normpath, len + 1);
            normpath[0] = G_DIR_SEPARATOR;
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

static char *
normalize_full_path_unix (const char *path)
{
    guint len;
    char *normpath;

    g_return_val_if_fail (path != NULL, NULL);

    normpath = normalize_path_string (path);
    g_return_val_if_fail (normpath != NULL, NULL);

    len = strlen (normpath);
    g_return_val_if_fail (len > 0, normpath);

    if (len > 1 && normpath[len-1] == G_DIR_SEPARATOR)
        normpath[len-1] = 0;

    return normpath;
}

#endif /* !__WIN32__ */

#if defined(__WIN32__) || defined(MOO_ENABLE_UNIT_TESTS)

static char *
normalize_full_path_win32 (const char *fullpath)
{
    char *prefix;
    char *path;
    guint slashes;

    g_return_val_if_fail (fullpath && fullpath[0], g_strdup (fullpath));

    if (fullpath[0] && fullpath[1] == ':')
    {
        prefix = g_strndup (fullpath, 3);
        prefix[2] = '\\';

        if (fullpath[2] == '\\')
            path = g_strdup (fullpath + 3);
        else
            path = g_strdup (fullpath + 2);
    }
    else
    {
        prefix = NULL;
        path = g_strdup (fullpath);
    }

    g_strdelimit (path, "/", '\\');

    for (slashes = 0; path[slashes] == '\\'; ++slashes) ;

    if (!prefix && slashes)
        prefix = g_strndup (path, slashes);

    if (slashes)
        memmove (path, path + slashes, strlen (path) + 1 - slashes);

    g_assert (prefix || *path);
    g_assert (*path != '\\');

    if (*path)
    {
        char **comps;
        guint n_comps, i;

        comps = g_strsplit (path, "\\", 0);
        n_comps = g_strv_length (comps);

        for (i = 0; i < n_comps; )
        {
            if (strcmp (comps[i], "") == 0 || strcmp (comps[i], ".") == 0 ||
                (i == 0 && strcmp (comps[i], "..") == 0))
            {
                g_free (comps[i]);
                MOO_ELMMOVE (comps + i, comps + i + 1, n_comps - i);
                n_comps -= 1;
            }
            else if (strcmp (comps[i], "..") == 0)
            {
                g_free (comps[i-1]);
                g_free (comps[i]);
                MOO_ELMMOVE (comps + i - 1, comps + i + 1, n_comps - i);
                n_comps -= 2;
                i -= 1;
            }
            else
            {
                i += 1;
            }
        }

        if (n_comps > 0)
        {
            char *tmp = g_strjoinv ("\\", comps);
            g_free (path);
            path = tmp;
        }
        else
        {
            path[0] = 0;
        }

        g_strfreev (comps);
    }

    if (*path)
    {
        if (prefix)
        {
            char *tmp = g_strconcat (prefix, path, NULL);
            g_free (path);
            path = tmp;
        }
    }
    else if (prefix)
    {
        g_free (path);
        path = prefix;
        prefix = NULL;
    }
    else
    {
        g_free (path);
        path = g_strdup (".");
    }

    g_free (prefix);
    return path;
}

#endif

static char *
normalize_path (const char *filename)
{
    char *freeme = NULL;
    char *norm_filename;

    g_assert (filename && filename[0]);

    if (!_moo_path_is_absolute (filename))
    {
        char *working_dir = g_get_current_dir ();
        g_return_val_if_fail (working_dir != NULL, g_strdup (filename));
        freeme = g_build_filename (working_dir, filename, NULL);
        filename = freeme;
        g_free (working_dir);
    }

#ifdef __WIN32__
    norm_filename = normalize_full_path_win32 (filename);
#else
    norm_filename = normalize_full_path_unix (filename);
#endif

    g_free (freeme);
    return norm_filename;
}

char *
_moo_normalize_file_path (const char *filename)
{
    g_return_val_if_fail (filename != NULL, NULL);
    /* empty filename is an error, but we don't want to crash here */
    g_return_val_if_fail (filename[0] != 0, g_strdup (""));
    return normalize_path (filename);
}

gboolean
_moo_path_is_absolute (const char *path)
{
    g_return_val_if_fail (path != NULL, FALSE);
    return g_path_is_absolute (path)
#ifdef __WIN32__
        /* 'C:' is an absolute path even if glib doesn't like it */
        /* This will match nonsense like 1:23:23 too, but that's not
         * a valid path, and it's better to have "1:23:23" in the error
         * message than "c:\some\silly\current\dir\1:23:23" */
        || path[1] == ':'
#endif
        ;
}


#ifdef MOO_ENABLE_UNIT_TESTS

#include <moo-tests.h>

static void
test_normalize_path_one (const char *path,
                         const char *expected,
                         char *(*func) (const char*),
                         const char *func_name)
{
    char *result;

    TEST_EXPECT_WARNING (!(expected && expected[0]),
                         "%s(%s)", func_name, TEST_FMT_STR (path));
    result = func (path);
    TEST_CHECK_WARNING ();

    TEST_ASSERT_STR_EQ_MSG (result, expected, "%s(%s)",
                            func_name, TEST_FMT_STR (path));

    g_free (result);
}

static GPtrArray *
make_cases (gboolean unix_paths)
{
    guint i;
    char *current_dir;
    GPtrArray *paths = g_ptr_array_new ();

    const char *abs_files_common[] = {
        NULL, NULL,
        "", "",
    };

    const char *abs_files_unix[] = {
        "/usr", "/usr",
        "/usr/", "/usr",
        "/usr///", "/usr",
        "///usr////", "/usr",
        "/", "/",
        "//", "/",
        "/usr/../bin/../usr", "/usr",
        "/usr/bin/../bin/.", "/usr/bin",
        "/..", "/",
        "/../../../", "/",
        "/../whatever/..", "/",
        "/../whatever/.././././", "/",
    };

    const char *abs_files_win32[] = {
        "C:", "C:\\",
        "C:\\", "C:\\",
        "C:\\\\", "C:\\",
        "C:\\foobar", "C:\\foobar",
        "C:\\foobar\\", "C:\\foobar",
        "C:\\foobar\\\\\\", "C:\\foobar",
        "C:/", "C:\\",
        "C:/foobar", "C:\\foobar",
        "C:/foobar//", "C:\\foobar",
        "C:/foobar///", "C:\\foobar",
        "\\\\.host\\dir\\root", "\\\\.host\\dir\\root",
        "\\\\.host\\dir\\root\\", "\\\\.host\\dir\\root",
        "\\\\.host\\dir\\root\\..\\..\\", "\\\\.host",
    };

    const char *rel_files_common[] = {
        ".", NULL,
        "././././/", NULL,
        "foobar", "foobar",
        "foobar/", "foobar",
        "foobar//", "foobar",
        "foobar/..", NULL,
        "foobar/./..", NULL,
        "foobar/../", NULL,
        "foobar/./../", NULL,
    };

#ifndef __WIN32__
    const char *rel_files_unix[] = {
        "foobar/com", "foobar/com",
    };
#endif

#ifdef __WIN32__
    const char *rel_files_win32[] = {
        "foobar/com", "foobar\\com",
        ".\\.\\.\\.\\\\", NULL,
        "foobar\\", "foobar",
        "foobar\\\\", "foobar",
        "foobar\\..", NULL,
        "foobar\\.\\..", NULL,
        "foobar\\..\\", NULL,
        "foobar\\.\\..\\", NULL,
    };
#endif

    for (i = 0; i < G_N_ELEMENTS (abs_files_common); i += 2)
    {
        g_ptr_array_add (paths, g_strdup (abs_files_common[i]));
        g_ptr_array_add (paths, g_strdup (abs_files_common[i+1]));
    }

    if (unix_paths)
        for (i = 0; i < G_N_ELEMENTS (abs_files_unix); i += 2)
        {
            g_ptr_array_add (paths, g_strdup (abs_files_unix[i]));
            g_ptr_array_add (paths, g_strdup (abs_files_unix[i+1]));
        }
    else
        for (i = 0; i < G_N_ELEMENTS (abs_files_win32); i += 2)
        {
            g_ptr_array_add (paths, g_strdup (abs_files_win32[i]));
            g_ptr_array_add (paths, g_strdup (abs_files_win32[i+1]));
        }

    current_dir = g_get_current_dir ();

#ifndef __WIN32__
    if (unix_paths)
#endif
        for (i = 0; i < G_N_ELEMENTS (rel_files_common); i += 2)
        {
            g_ptr_array_add (paths, g_strdup (rel_files_common[i]));
            if (rel_files_common[i+1])
                g_ptr_array_add (paths, g_build_filename (current_dir, rel_files_common[i+1], NULL));
            else
                g_ptr_array_add (paths, g_strdup (current_dir));
        }

#ifndef __WIN32__
    if (unix_paths)
        for (i = 0; i < G_N_ELEMENTS (rel_files_unix); i += 2)
        {
            g_ptr_array_add (paths, g_strdup (rel_files_unix[i]));
            if (rel_files_unix[i+1])
                g_ptr_array_add (paths, g_build_filename (current_dir, rel_files_unix[i+1], NULL));
            else
                g_ptr_array_add (paths, g_strdup (current_dir));
        }
#endif

#ifdef __WIN32__
    for (i = 0; i < G_N_ELEMENTS (rel_files_win32); i += 2)
    {
        g_ptr_array_add (paths, g_strdup (rel_files_win32[i]));
        if (rel_files_win32[i+1])
            g_ptr_array_add (paths, g_build_filename (current_dir, rel_files_win32[i+1], NULL));
        else
            g_ptr_array_add (paths, g_strdup (current_dir));
    }
#endif

#ifndef __WIN32__
    if (unix_paths)
#endif
    {
        char *parent_dir = g_path_get_dirname (current_dir);
        g_ptr_array_add (paths, g_strdup (".."));
        g_ptr_array_add (paths, g_strdup (parent_dir));
        g_ptr_array_add (paths, g_strdup ("../"));
        g_ptr_array_add (paths, g_strdup (parent_dir));
        g_ptr_array_add (paths, g_strdup ("../."));
        g_ptr_array_add (paths, g_strdup (parent_dir));
        g_ptr_array_add (paths, g_strdup (".././"));
        g_ptr_array_add (paths, g_strdup (parent_dir));
        g_ptr_array_add (paths, g_strdup ("..//"));
        g_ptr_array_add (paths, g_strdup (parent_dir));
#ifdef __WIN32__
        g_ptr_array_add (paths, g_strdup ("..\\"));
        g_ptr_array_add (paths, g_strdup (parent_dir));
        g_ptr_array_add (paths, g_strdup ("..\\."));
        g_ptr_array_add (paths, g_strdup (parent_dir));
        g_ptr_array_add (paths, g_strdup ("..\\.\\"));
        g_ptr_array_add (paths, g_strdup (parent_dir));
        g_ptr_array_add (paths, g_strdup ("..\\\\"));
        g_ptr_array_add (paths, g_strdup (parent_dir));
#endif
        g_free (parent_dir);
    }

    g_free (current_dir);
    return paths;
}

static void
test_normalize_file_path (void)
{
    GPtrArray *paths;
    guint i;

#ifndef __WIN32__
    paths = make_cases (TRUE);
#else
    paths = make_cases (FALSE);
#endif

    for (i = 0; i < paths->len; i += 2)
    {
        test_normalize_path_one (paths->pdata[i], paths->pdata[i+1],
                                 _moo_normalize_file_path,
                                 "_moo_normalize_file_path");
        g_free (paths->pdata[i]);
        g_free (paths->pdata[i+1]);
    }

    g_ptr_array_free (paths, TRUE);
}

#ifndef __WIN32__
static void
test_normalize_file_path_win32 (void)
{
    GPtrArray *paths;
    guint i;

    paths = make_cases (FALSE);

    for (i = 0; i < paths->len; i += 2)
    {
        test_normalize_path_one (paths->pdata[i], paths->pdata[i+1],
                                 normalize_full_path_win32,
                                 "normalize_full_path_win32");
        g_free (paths->pdata[i]);
        g_free (paths->pdata[i+1]);
    }

    g_ptr_array_free (paths, TRUE);
}
#endif

void
moo_test_mooutils_fs (void)
{
    MooTestSuite *suite;

    suite = moo_test_suite_new ("mooutils/mooutils-fs.c", NULL, NULL, NULL);

    moo_test_suite_add_test (suite, "test of _moo_normalize_file_path()",
                             (MooTestFunc) test_normalize_file_path, NULL);
#ifndef __WIN32__
    moo_test_suite_add_test (suite, "test of normalize_full_path_win32()",
                             (MooTestFunc) test_normalize_file_path_win32, NULL);
#endif
}

#endif /* MOO_ENABLE_UNIT_TESTS */


/**********************************************************************/
/* MSLU for poor
 */

#ifdef __WIN32__
static gpointer
convert_filename (const char *filename,
                  gboolean   *use_wide_char_api)
{
    if (G_WIN32_HAVE_WIDECHAR_API ())
    {
        *use_wide_char_api = TRUE;
        return g_utf8_to_utf16 (filename, -1, NULL, NULL, NULL);
    }
    else
    {
        *use_wide_char_api = FALSE;
        return g_locale_from_utf8 (filename, -1, NULL,
                                   NULL, NULL);
    }
}
#endif


#define CCALL_1(_AFunc, _WFunc, path)                           \
G_STMT_START {                                                  \
    gboolean use_wide_char_api;                                 \
    gpointer converted;                                         \
    int retval;                                                 \
    int save_errno;                                             \
                                                                \
    converted = convert_filename (path, &use_wide_char_api);    \
                                                                \
    if (!converted)                                             \
    {                                                           \
        errno = EINVAL;                                         \
        return -1;                                              \
    }                                                           \
                                                                \
    errno = 0;                                                  \
                                                                \
    if (use_wide_char_api)                                      \
        retval = _WFunc (converted);                            \
    else                                                        \
        retval = _AFunc (converted);                            \
                                                                \
    save_errno = errno;                                         \
    g_free (converted);                                         \
    errno = save_errno;                                         \
                                                                \
    return retval;                                              \
} G_STMT_END


int
_moo_unlink (const char *path)
{
    return g_unlink (path);
}


int
_moo_mkdir (const char *path)
{
#ifdef __WIN32__
    CCALL_1 (mkdir, _wmkdir, path);
#else
    return mkdir (path, S_IRWXU);
#endif
}


int
_moo_remove (const char *path)
{
    return g_remove (path);
}


gpointer
_moo_fopen (const char *path,
            const char *mode)
{
#ifdef __WIN32__
    gboolean use_wide_char_api;
    gpointer path_conv, mode_conv;
    FILE *retval;
    int save_errno;

    if (G_WIN32_HAVE_WIDECHAR_API ())
    {
        use_wide_char_api = TRUE;
        path_conv = g_utf8_to_utf16 (path, -1, NULL, NULL, NULL);
        mode_conv = g_utf8_to_utf16 (mode, -1, NULL, NULL, NULL);
    }
    else
    {
        use_wide_char_api = FALSE;
        path_conv = g_locale_from_utf8 (path, -1, NULL, NULL, NULL);
        mode_conv = g_locale_from_utf8 (mode, -1, NULL, NULL, NULL);
    }

    if (!path_conv || !mode_conv)
    {
        g_free (path_conv);
        g_free (mode_conv);
        errno = EINVAL;
        return NULL;
    }

    errno = 0;

    if (use_wide_char_api)
        retval = _wfopen (path_conv, mode_conv);
    else
        retval = fopen (path_conv, mode_conv);

    save_errno = errno;
    g_free (path_conv);
    g_free (mode_conv);
    errno = save_errno;

    return retval;
#else
    return fopen (path, mode);
#endif
}


int
_moo_rename (const char *old_name,
             const char *new_name)
{
#ifdef __WIN32__
    gboolean use_wide_char_api;
    gpointer old_conv, new_conv;
    int retval, save_errno;

    if (G_WIN32_HAVE_WIDECHAR_API ())
    {
        use_wide_char_api = TRUE;
        old_conv = g_utf8_to_utf16 (old_name, -1, NULL, NULL, NULL);
        new_conv = g_utf8_to_utf16 (new_name, -1, NULL, NULL, NULL);
    }
    else
    {
        use_wide_char_api = FALSE;
        old_conv = g_locale_from_utf8 (old_name, -1, NULL, NULL, NULL);
        new_conv = g_locale_from_utf8 (new_name, -1, NULL, NULL, NULL);
    }

    if (!old_conv || !new_conv)
    {
        g_free (old_conv);
        g_free (new_conv);
        errno = EINVAL;
        return -1;
    }

    errno = 0;

    if (use_wide_char_api)
        retval = _wrename (old_conv, new_conv);
    else
        retval = rename (old_conv, new_conv);

    save_errno = errno;
    g_free (old_conv);
    g_free (new_conv);
    errno = save_errno;

    return retval;
#else
    return rename (old_name, new_name);
#endif
}


/***********************************************************************/
/* Glob matching
 */

#if 0 && defined(HAVE_FNMATCH_H)
#include <fnmatch.h>
#else
#define MOO_GLOB_REGEX
#include <glib/gregex.h>
#endif

typedef struct _MooGlob {
#ifdef MOO_GLOB_REGEX
    GRegex *re;
#else
    char *pattern;
#endif
} MooGlob;

#ifdef MOO_GLOB_REGEX
static char *
glob_to_re (const char *pattern)
{
    GString *string;
    const char *p, *piece, *bracket;
    char *escaped;

    g_return_val_if_fail (pattern != NULL, NULL);

    p = piece = pattern;
    string = g_string_new (NULL);

    while (*p)
    {
        switch (*p)
        {
            case '*':
                if (p != piece)
                    g_string_append_len (string, piece, p - piece);
                g_string_append (string, ".*");
                piece = ++p;
                break;

            case '?':
                if (p != piece)
                    g_string_append_len (string, piece, p - piece);
                g_string_append_c (string, '.');
                piece = ++p;
                break;

            case '[':
                if (!(bracket = strchr (p + 1, ']')))
                {
                    g_warning ("in %s: unmatched '['", pattern);
                    goto error;
                }

                if (p != piece)
                    g_string_append_len (string, piece, p - piece);

                g_string_append_c (string, '[');
                if (p[1] == '^')
                {
                    g_string_append_c (string, '^');
                    escaped = g_regex_escape_string (p + 2, bracket - p - 2);
                }
                else
                {
                    escaped = g_regex_escape_string (p + 1, bracket - p - 1);
                }
                g_string_append (string, escaped);
                g_free (escaped);
                g_string_append_c (string, ']');
                piece = p = bracket + 1;
                break;

            case '\\':
            case '|':
            case '(':
            case ')':
            case ']':
            case '{':
            case '}':
            case '^':
            case '$':
            case '+':
            case '.':
                if (p != piece)
                    g_string_append_len (string, piece, p - piece);
                g_string_append_c (string, '\\');
                g_string_append_c (string, *p);
                piece = ++p;
                break;

            default:
                p = g_utf8_next_char (p);
        }
    }

    if (*piece)
        g_string_append (string, piece);

    g_string_append_c (string, '$');

    if (0)
        _moo_message ("converted '%s' to '%s'\n", pattern, string->str);

    return g_string_free (string, FALSE);

error:
    g_string_free (string, TRUE);
    return NULL;
}


static MooGlob *
_moo_glob_new (const char *pattern)
{
    MooGlob *gl;
    GRegex *re;
    char *re_pattern;
    GRegexCompileFlags flags = 0;
    GError *error = NULL;

    g_return_val_if_fail (pattern != NULL, NULL);

#ifdef __WIN32__
    flags = G_REGEX_CASELESS;
#endif

    if (!(re_pattern = glob_to_re (pattern)))
        return NULL;

    re = g_regex_new (re_pattern, flags, 0, &error);

    g_free (re_pattern);

    if (error)
    {
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
    }

    if (!re)
        return NULL;

    gl = g_new0 (MooGlob, 1);
    gl->re = re;

    return gl;
}


static gboolean
_moo_glob_match (MooGlob    *glob,
                 const char *filename_utf8)
{
    g_return_val_if_fail (glob != NULL, FALSE);
    g_return_val_if_fail (filename_utf8 != NULL, FALSE);
    g_return_val_if_fail (g_utf8_validate (filename_utf8, -1, NULL), FALSE);

    return g_regex_match (glob->re, filename_utf8, 0, NULL);
}
#endif


static void
_moo_glob_free (MooGlob *glob)
{
    if (glob)
    {
#ifdef MOO_GLOB_REGEX
        g_regex_unref (glob->re);
#else
        g_free (glob->pattern);
#endif
        g_free (glob);
    }
}


gboolean
_moo_glob_match_simple (const char *pattern,
                        const char *filename)
{
    MooGlob *gl;
    gboolean result = FALSE;

    g_return_val_if_fail (pattern != NULL, FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);

    if ((gl = _moo_glob_new (pattern)))
        result = _moo_glob_match (gl, filename);

    if (result && 0)
        _moo_message ("'%s' matched '%s'", filename, pattern);

    _moo_glob_free (gl);
    return result;
}


/************************************************************************/
/* MooFileWriter
 */

typedef struct MooFileWriterClass MooFileWriterClass;

struct MooFileWriter {
    GObject base;
};

struct MooFileWriterClass {
    GObjectClass base_class;

    gboolean (*meth_write)  (MooFileWriter *writer,
                             const char    *data,
                             gsize          len);
    gboolean (*meth_printf) (MooFileWriter  *writer,
                             const char     *fmt,
                             va_list         args) G_GNUC_PRINTF (2, 0);
    gboolean (*meth_close)  (MooFileWriter  *writer,
                             GError        **error);
};

#define MOO_TYPE_FILE_WRITER            (moo_file_writer_get_type ())
#define MOO_FILE_WRITER(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_FILE_WRITER, MooFileWriter))
#define MOO_FILE_WRITER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_FILE_WRITER, MooFileWriterClass))
#define MOO_IS_FILE_WRITER(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_FILE_WRITER))
#define MOO_IS_FILE_WRITER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_FILE_WRITER))
#define MOO_FILE_WRITER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_FILE_WRITER, MooFileWriterClass))

MOO_DEFINE_TYPE_STATIC (MooFileWriter, moo_file_writer, G_TYPE_OBJECT)

static void
moo_file_writer_init (G_GNUC_UNUSED MooFileWriter *writer)
{
}

static void
moo_file_writer_class_init (G_GNUC_UNUSED MooFileWriterClass *klass)
{
}

gboolean
moo_file_writer_write (MooFileWriter *writer,
                       const char    *data,
                       gssize         len)
{
    g_return_val_if_fail (MOO_IS_FILE_WRITER (writer), FALSE);
    g_return_val_if_fail (data != NULL, FALSE);

    if (len < 0)
        len = strlen (data);
    if (!len)
        return TRUE;

    return MOO_FILE_WRITER_GET_CLASS (writer)->meth_write (writer, data, len);
}

gboolean
moo_file_writer_printf (MooFileWriter  *writer,
                        const char     *fmt,
                        ...)
{
    va_list args;
    gboolean ret;

    g_return_val_if_fail (MOO_IS_FILE_WRITER (writer), FALSE);
    g_return_val_if_fail (fmt != NULL, FALSE);

    va_start (args, fmt);
    ret = MOO_FILE_WRITER_GET_CLASS (writer)->meth_printf (writer, fmt, args);
    va_end (args);

    return ret;
}

gboolean
moo_file_writer_close (MooFileWriter  *writer,
                       GError        **error)
{
    gboolean ret;

    g_return_val_if_fail (MOO_IS_FILE_WRITER (writer), FALSE);
    g_return_val_if_fail (!error || !*error, FALSE);

    ret = MOO_FILE_WRITER_GET_CLASS (writer)->meth_close (writer, error);

    g_object_unref (writer);
    return ret;
}


/************************************************************************/
/* MooFileWriter
 */

typedef struct MooLocalFileWriter MooLocalFileWriter;
typedef struct MooLocalFileWriterClass MooLocalFileWriterClass;

struct MooLocalFileWriter {
    MooFileWriter base;

    FILE *file;
    char *filename;
    char *temp_filename;
    gboolean save_backup;
    GError *error;
};

struct MooLocalFileWriterClass {
    MooFileWriterClass base_class;
};

#define MOO_TYPE_LOCAL_FILE_WRITER (moo_local_file_writer_get_type ())

MOO_DEFINE_TYPE_STATIC (MooLocalFileWriter, moo_local_file_writer, MOO_TYPE_FILE_WRITER)

static gboolean
same_content (const char *filename1,
              const char *filename2)
{
    GMappedFile *file1, *file2;
    char *content1, *content2;
    gsize len;
    gboolean equal = FALSE;

    file1 = g_mapped_file_new (filename1, FALSE, NULL);
    file2 = g_mapped_file_new (filename2, FALSE, NULL);

    if (!file1 || !file2 ||
        g_mapped_file_get_length (file1) != g_mapped_file_get_length (file2))
            goto out;

    len = g_mapped_file_get_length (file1);
    content1 = g_mapped_file_get_contents (file1);
    content2 = g_mapped_file_get_contents (file2);

    if (memcmp (content1, content2, len) == 0)
        equal = TRUE;

out:
    if (file1)
        g_mapped_file_free (file1);
    if (file2)
        g_mapped_file_free (file2);
    return equal;
}

static MooFileWriter *
moo_local_file_writer_new (const char  *filename,
                           gboolean     text_mode,
                           gboolean     save_backup,
                           GError     **error)
{
    int fd;
    FILE *file;
    char *temp_filename = NULL;
    char *dirname = NULL;
    MooLocalFileWriter *writer = NULL;

    g_return_val_if_fail (filename != NULL, NULL);

    dirname = g_path_get_dirname (filename);

    if (_moo_mkdir_with_parents (dirname) != 0)
    {
        int err = errno;
        char *display_name = g_filename_display_name (dirname);
        g_set_error (error, G_FILE_ERROR,
                     g_file_error_from_errno (err),
                     "could not create directory '%s': %s",
                     display_name, g_strerror (err));
        g_free (display_name);
        goto out;
    }

    temp_filename = g_strdup_printf ("%s/.cfg-tmp-XXXXXX", dirname);
    errno = 0;
    fd = g_mkstemp (temp_filename);

    if (fd == -1)
    {
        int err = errno;
        char *display_name = g_filename_display_name (temp_filename);
        g_set_error (error, G_FILE_ERROR,
                     g_file_error_from_errno (err),
                     "could not create temporary file '%s': %s",
                     display_name, g_strerror (err));
        g_free (display_name);
        goto out;
    }

    close (fd);

    if (!(file = g_fopen (temp_filename, text_mode ? "w" : "wb")))
    {
        int err = errno;
        char *display_name = g_filename_display_name (temp_filename);
        g_set_error (error, G_FILE_ERROR,
                     g_file_error_from_errno (err),
                     "could not create temporary file '%s': %s",
                     display_name, g_strerror (err));
        g_free (display_name);
        g_unlink (temp_filename);
        goto out;
    }

    writer = g_object_new (MOO_TYPE_LOCAL_FILE_WRITER, NULL);
    writer->file = file;
    writer->temp_filename = temp_filename;
    temp_filename = NULL;
    writer->filename = g_strdup (filename);
    writer->error = NULL;
    writer->save_backup = save_backup;

out:
    g_free (temp_filename);
    g_free (dirname);
    return MOO_FILE_WRITER (writer);
}

MooFileWriter *
moo_file_writer_new (const char  *filename,
                     gboolean     save_backup,
                     GError     **error)
{
    g_return_val_if_fail (filename != NULL, NULL);
    g_return_val_if_fail (!error || !*error, NULL);
    return moo_local_file_writer_new (filename, FALSE, save_backup, error);
}

MooFileWriter *
moo_text_writer_new (const char  *filename,
                     gboolean     save_backup,
                     GError     **error)
{
    g_return_val_if_fail (filename != NULL, NULL);
    g_return_val_if_fail (!error || !*error, NULL);
    return moo_local_file_writer_new (filename, TRUE, save_backup, error);
}


static gboolean
moo_local_file_writer_write (MooFileWriter *fwriter,
                             const char    *data,
                             gsize          len)
{
    MooLocalFileWriter *writer = (MooLocalFileWriter*) fwriter;

    if (writer->error)
        return FALSE;

    while (len > 0)
    {
        gsize written;

        errno = 0;
        written = fwrite (data, 1, len, writer->file);

        if (written < len && ferror (writer->file))
        {
            int err = errno;
            g_set_error (&writer->error, G_FILE_ERROR,
                         g_file_error_from_errno (err),
                         "%s", g_strerror (err));
            break;
        }

        len -= written;
        data += written;
    }

    return writer->error == NULL;
}

G_GNUC_PRINTF (2, 0)
static gboolean
moo_local_file_writer_printf (MooFileWriter *fwriter,
                              const char    *fmt,
                              va_list        args)
{
    MooLocalFileWriter *writer = (MooLocalFileWriter*) fwriter;

    if (writer->error)
        return FALSE;

    errno = 0;

    if (vfprintf (writer->file, fmt, args) < 0)
    {
        int err = errno;
        g_set_error (&writer->error, G_FILE_ERROR,
                     g_file_error_from_errno (err),
                     "%s", g_strerror (err));
    }

    return writer->error == NULL;
}

static gboolean
moo_local_file_writer_close (MooFileWriter *fwriter,
                             GError       **error)
{
    MooLocalFileWriter *writer = (MooLocalFileWriter*) fwriter;
    gboolean retval;

    if (!writer->error)
    {
        errno = 0;

        if (fclose (writer->file) != 0)
        {
            int err = errno;
            g_set_error (&writer->error, G_FILE_ERROR,
                         g_file_error_from_errno (err),
                         "%s", g_strerror (err));
        }

        writer->file = NULL;
    }

    if (!writer->error &&
        !same_content (writer->filename, writer->temp_filename))
    {
        if (writer->save_backup)
        {
            char *bak_file;

            bak_file = g_strdup_printf ("%s.bak", writer->filename);

            if (g_file_test (writer->filename, G_FILE_TEST_EXISTS))
            {
                g_unlink (bak_file);
                _moo_rename_file (writer->filename, bak_file, NULL);
            }

            g_free (bak_file);
        }

        _moo_rename_file (writer->temp_filename,
                          writer->filename,
                          &writer->error);
    }

    g_unlink (writer->temp_filename);
    g_free (writer->temp_filename);
    g_free (writer->filename);
    writer->temp_filename = NULL;
    writer->filename = NULL;

    retval = writer->error == NULL;

    if (writer->error)
        g_propagate_error (error, writer->error);

    writer->error = NULL;
    return retval;
}


static void
moo_local_file_writer_class_init (MooLocalFileWriterClass *klass)
{
    MooFileWriterClass *writer_class = MOO_FILE_WRITER_CLASS (klass);
    writer_class->meth_write = moo_local_file_writer_write;
    writer_class->meth_printf = moo_local_file_writer_printf;
    writer_class->meth_close = moo_local_file_writer_close;
}

static void
moo_local_file_writer_init (MooLocalFileWriter *writer)
{
    writer->file = NULL;
    writer->temp_filename = NULL;
    writer->filename = NULL;
    writer->error = NULL;
}


/************************************************************************/
/* MooFileWriter
 */

typedef struct MooStringWriter MooStringWriter;
typedef struct MooStringWriterClass MooStringWriterClass;

struct MooStringWriter {
    MooFileWriter base;
    GString *string;
};

struct MooStringWriterClass {
    MooFileWriterClass base_class;
};

#define MOO_TYPE_STRING_WRITER (moo_string_writer_get_type ())

MOO_DEFINE_TYPE_STATIC (MooStringWriter, moo_string_writer, MOO_TYPE_FILE_WRITER)

static gboolean
moo_string_writer_write (MooFileWriter *fwriter,
                         const char    *data,
                         gsize          len)
{
    MooStringWriter *writer = (MooStringWriter*) fwriter;
    g_string_append_len (writer->string, data, len);
    return TRUE;
}

G_GNUC_PRINTF (2, 0)
static gboolean
moo_string_writer_printf (MooFileWriter *fwriter,
                          const char    *fmt,
                          va_list        args)
{
    MooStringWriter *writer = (MooStringWriter*) fwriter;
    g_string_append_vprintf (writer->string, fmt, args);
    return TRUE;
}

static gboolean
moo_string_writer_close (MooFileWriter *fwriter,
                         G_GNUC_UNUSED GError **error)
{
    MooStringWriter *writer = (MooStringWriter*) fwriter;
    g_string_free (writer->string, TRUE);
    writer->string = NULL;
    return TRUE;
}

static void
moo_string_writer_class_init (MooStringWriterClass *klass)
{
    MooFileWriterClass *writer_class = MOO_FILE_WRITER_CLASS (klass);
    writer_class->meth_write = moo_string_writer_write;
    writer_class->meth_printf = moo_string_writer_printf;
    writer_class->meth_close = moo_string_writer_close;
}

static void
moo_string_writer_init (MooStringWriter *writer)
{
    writer->string = g_string_new (NULL);
}

MooFileWriter *
moo_string_writer_new (void)
{
    return g_object_new (MOO_TYPE_STRING_WRITER, NULL);
}

const char *
moo_string_writer_get_string (MooFileWriter *fwriter,
                              gsize         *len)
{
    MooStringWriter *writer = (MooStringWriter*) fwriter;

    g_return_val_if_fail (G_TYPE_CHECK_INSTANCE_TYPE ((writer), MOO_TYPE_STRING_WRITER), NULL);
    g_return_val_if_fail (writer->string != NULL, NULL);

    if (len)
        *len = writer->string->len;

    return writer->string->str;
}


#ifdef MOO_ENABLE_UNIT_TESTS

#include <moo-tests.h>

static gboolean
check_file_contents (const char *filename,
                     const char *contents)
{
    gboolean equal;
    char *real_contents;

    if (!g_file_get_contents (filename, &real_contents, NULL, NULL))
        return FALSE;

    equal = strcmp (real_contents, contents) == 0;

    g_free (real_contents);
    return equal;
}

static void
test_moo_file_writer (void)
{
    const char *dir;
    char *my_dir, *filename, *bak_filename;
    MooFileWriter *writer;
    GError *error = NULL;

    dir = moo_test_get_working_dir ();
    my_dir = g_build_filename (dir, "cfg-writer", NULL);
    filename = g_build_filename (my_dir, "configfile", NULL);
    bak_filename = g_strdup_printf ("%s.bak", filename);

    writer = moo_text_writer_new (filename, TRUE, &error);
    TEST_ASSERT_MSG (writer != NULL,
                     "moo_cfg_writer_new failed: %s",
                     error ? error->message : "");
    if (error)
    {
        g_error_free (error);
        error = NULL;
    }

    if (writer)
    {
        moo_file_writer_write (writer, "first line\n", -1);
        moo_file_writer_printf (writer, "second line #%d\n", 2);
        moo_file_writer_write (writer, "third\nlalalala\n", 6);
        TEST_ASSERT_MSG (moo_file_writer_close (writer, &error),
                         "moo_file_writer_close failed: %s",
                         error ? error->message : "");
        if (error)
        {
            g_error_free (error);
            error = NULL;
        }

#ifdef __WIN32__
#define LE "\r\n"
#else
#define LE "\n"
#endif

        TEST_ASSERT (g_file_test (filename, G_FILE_TEST_EXISTS));
        TEST_ASSERT (!g_file_test (bak_filename, G_FILE_TEST_EXISTS));
        TEST_ASSERT (check_file_contents (filename, "first line" LE "second line #2" LE "third" LE));
    }

    writer = moo_text_writer_new (filename, TRUE, &error);
    TEST_ASSERT_MSG (writer != NULL,
                     "moo_text_writer_new failed: %s",
                     error ? error->message : "");
    if (writer)
    {
        moo_file_writer_write (writer, "First line\n", -1);
        moo_file_writer_printf (writer, "Second line #%d\n", 2);
        moo_file_writer_write (writer, "Third\nlalalala\n", 6);
        TEST_ASSERT_MSG (moo_file_writer_close (writer, &error),
                         "moo_file_writer_close failed: %s",
                         error ? error->message : "");
        if (error)
        {
            g_error_free (error);
            error = NULL;
        }

        TEST_ASSERT (g_file_test (filename, G_FILE_TEST_EXISTS));
        TEST_ASSERT (g_file_test (bak_filename, G_FILE_TEST_EXISTS));
        TEST_ASSERT (check_file_contents (filename, "First line" LE "Second line #2" LE "Third" LE));
        TEST_ASSERT (check_file_contents (bak_filename, "first line" LE "second line #2" LE "third" LE));
    }

    TEST_ASSERT (_moo_remove_dir (my_dir, TRUE, NULL));

#ifndef __WIN32__
    writer = moo_text_writer_new ("/usr/test-mooutils-fs", TRUE, &error);
#else
    writer = moo_text_writer_new ("K:\\nowayyouhaveit\\file.ini", TRUE, &error);
#endif
    TEST_ASSERT (writer == NULL);
    TEST_ASSERT (error != NULL);

    if (error)
        g_error_free (error);
    error = NULL;

    g_free (bak_filename);
    g_free (filename);
    g_free (my_dir);
}

void
moo_test_moo_file_writer (void)
{
    MooTestSuite *suite;

    suite = moo_test_suite_new ("MooFileWriter", NULL, NULL, NULL);

    moo_test_suite_add_test (suite, "test of MooFileWriter",
                             (MooTestFunc) test_moo_file_writer, NULL);
}

#endif /* MOO_ENABLE_UNIT_TESTS */
