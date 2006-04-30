/*
 *   mooutils-fs.c
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
# include "config.h"
#endif

#include "mooutils/mooutils-fs.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>

#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#include <direct.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#define BROKEN_NAME "<" "????" ">"


/* XXX fix this */
gboolean
moo_save_file_utf8 (const char *name,
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
    GError *error_here = NULL;
    const char *file;
    gboolean success = TRUE;

    g_return_val_if_fail (path != NULL, FALSE);

    dir = g_dir_open (path, 0, &error_here);

    if (!dir)
    {
        g_propagate_error (error, error_here);
        return FALSE;
    }

    while ((file = g_dir_read_name (dir)))
    {
        char *file_path = g_build_filename (path, file, NULL);

        if (m_remove (file_path))
        {
            int err = errno;

            switch (err)
            {
                case ENOTEMPTY:
                case EEXIST:
                    if (!rm_r (file_path, error))
                        success = FALSE;
                    break;

                default:
                    success = FALSE;
                    g_set_error (error, MOO_FILE_ERROR,
                                 moo_file_error_from_errno (err),
                                 "Could not remove '%s': %s", file_path,
                                 g_strerror (err));
            }
        }

        g_free (file_path);
    }

    g_dir_close (dir);

    if (success && m_remove (path))
    {
        int err = errno;
        success = FALSE;
        g_set_error (error, MOO_FILE_ERROR,
                     moo_file_error_from_errno (err),
                     "Could not remove '%s': %s", path,
                     g_strerror (err));
    }

    return success;
}
#endif


gboolean
moo_rmdir (const char *path,
           gboolean    recursive,
           GError    **error)
{
    g_return_val_if_fail (path != NULL, FALSE);

    if (!recursive)
    {
        if (m_rmdir (path))
        {
            int err = errno;
            char *path_utf8 = g_filename_to_utf8 (path, -1, NULL, NULL, NULL);
            g_set_error (error, MOO_FILE_ERROR,
                         moo_file_error_from_errno (err),
                         "Could not remove '%s': %s",
                         path_utf8 ? path_utf8 : BROKEN_NAME,
                         g_strerror (err));
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


gboolean
moo_mkdir (const char *path,
           GError    **error)
{
    struct stat buf;
    char *utf8_path;

    g_return_val_if_fail (path != NULL, FALSE);

    if (stat (path, &buf) == -1 && errno != ENOENT)
    {
        int err_code = errno;
        utf8_path = g_filename_to_utf8 (path, -1, NULL, NULL, NULL);

        g_set_error (error,
                     MOO_FILE_ERROR,
                     moo_file_error_from_errno (err_code),
                     "Could not create directory '%s': %s",
                     utf8_path ? utf8_path : BROKEN_NAME,
                     g_strerror (err_code));

        g_free (utf8_path);
        return FALSE;
    }

    if (errno != 0)
    {
        if (m_mkdir (path) == -1)
        {
            int err_code = errno;
            utf8_path = g_filename_to_utf8 (path, -1, NULL, NULL, NULL);

            g_set_error (error,
                         MOO_FILE_ERROR,
                         moo_file_error_from_errno (err_code),
                         "Could not create directory '%s': %s",
                         utf8_path ? utf8_path : BROKEN_NAME,
                         g_strerror (err_code));

            g_free (utf8_path);
            return FALSE;
        }

        return TRUE;
    }

    if (S_ISDIR (buf.st_mode))
        return TRUE;

    utf8_path = g_filename_to_utf8 (path, -1, NULL, NULL, NULL);
    g_set_error (error, MOO_FILE_ERROR,
                 MOO_FILE_ERROR_ALREADY_EXISTS,
                 "Could not create directory '%s': %s",
                 utf8_path ? utf8_path : BROKEN_NAME,
                 g_strerror (EEXIST));
    g_free (utf8_path);

    return FALSE;
}


GQuark
moo_file_error_quark (void)
{
    static GQuark quark = 0;

    if (quark == 0)
        quark = g_quark_from_static_string ("moo-file-error-quark");

    return quark;
}


MooFileError
moo_file_error_from_errno (int code)
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
#ifdef __WIN32__
        conv[i] = g_locale_to_utf8 (*files, -1, NULL, NULL, NULL);

        if (!conv[i])
            g_warning ("%s: could not convert '%s' to UTF8", G_STRLOC, *files);
        else
            ++i;
#else
        conv[i++] = g_strdup (*files);
#endif
    }

    return conv;
}


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
m_unlink (const char *path)
{
#ifdef __WIN32__
    CCALL_1 (unlink, _wunlink, path);
#else
    return unlink (path);
#endif
}


int
m_rmdir (const char *path)
{
#ifdef __WIN32__
    CCALL_1 (rmdir, _wrmdir, path);
#else
    return rmdir (path);
#endif
}


int
m_mkdir (const char *path)
{
#ifdef __WIN32__
    CCALL_1 (mkdir, _wmkdir, path);
#else
    return mkdir (path, S_IRWXU);
#endif
}


int
m_remove (const char *path)
{
#ifdef __WIN32__
    gboolean use_wide_char_api;
    gpointer converted;
    int retval;
    int save_errno;

    converted = convert_filename (path, &use_wide_char_api);

    if (!converted)
    {
        errno = EINVAL;
        return -1;
    }

    if (use_wide_char_api)
        retval = _wremove (converted);
    else
        retval = remove (converted);

    if (retval && errno == ENOENT)
    {
        if (use_wide_char_api)
            retval = _wrmdir (converted);
        else
            retval = rmdir (converted);
    }

    save_errno = errno;
    g_free (converted);
    errno = save_errno;

    return retval;
#else
    return remove (path);
#endif
}


gpointer
m_fopen (const char *path,
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
m_rename (const char *old_name,
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
