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
#include "mooutils/mooutils-misc.h"

#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <glib/gstdio.h>

#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#include <direct.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#define BROKEN_NAME "<" "????" ">"


static int  _moo_chdir  (const char *path);
static int  _moo_rmdir  (const char *path);


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

        errno = 0;

        if (_moo_remove (file_path))
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

        if (_moo_rmdir (path) != 0)
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


char *
_moo_normalize_file_path (const char *filename)
{
    char *freeme = NULL;
    char *working_dir, *basename, *dirname;
    char *real_filename = NULL;

    g_return_val_if_fail (filename != NULL, NULL);

    working_dir = g_get_current_dir ();
    g_return_val_if_fail (working_dir != NULL, g_strdup (filename));

    if (!g_path_is_absolute (filename))
    {
        freeme = g_build_filename (working_dir, filename, NULL);
        filename = freeme;
    }

    /* It's an error to call this function for a directory, so if
     * it's the case, just return what we got. */
    if (g_file_test (filename, G_FILE_TEST_IS_DIR) ||
        /* and this totally screws up dirname/basename */
        g_str_has_suffix (filename, G_DIR_SEPARATOR_S))
    {
        if (!freeme)
            freeme = g_strdup (filename);
        g_free (working_dir);
        return freeme;
    }

    dirname = g_path_get_dirname (filename);
    basename = g_path_get_basename (filename);
    g_return_val_if_fail (dirname && basename, g_strdup (filename));

    errno = 0;

    if (_moo_chdir (dirname) != 0)
    {
        int err = errno;
        g_warning ("%s: %s", G_STRLOC, g_strerror (err));
    }
    else
    {
        char *real_dirname = g_get_current_dir ();
        real_filename = g_build_filename (real_dirname, basename, NULL);
        g_free (real_dirname);

        errno = 0;

        if (_moo_chdir (working_dir) != 0)
        {
            int err = errno;
            g_warning ("%s: %s", G_STRLOC, g_strerror (err));
        }
    }

    if (!real_filename)
        real_filename = g_strdup (filename);

    g_free (freeme);
    g_free (dirname);
    g_free (basename);
    g_free (working_dir);

    return real_filename;
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
#ifdef __WIN32__
    CCALL_1 (unlink, _wunlink, path);
#else
    return unlink (path);
#endif
}


static int
_moo_rmdir (const char *path)
{
#ifdef __WIN32__
    CCALL_1 (rmdir, _wrmdir, path);
#else
    return rmdir (path);
#endif
}


static int
_moo_chdir (const char *path)
{
#ifdef __WIN32__
    CCALL_1 (_chdir, _wchdir, path);
#else
    return chdir (path);
#endif
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

    errno = 0;

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
#include <mooutils/eggregex.h>
#endif

typedef struct _MooGlob {
#ifdef MOO_GLOB_REGEX
    EggRegex *re;
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
                    escaped = egg_regex_escape_string (p + 2, bracket - p - 2);
                }
                else
                {
                    escaped = egg_regex_escape_string (p + 1, bracket - p - 1);
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
    EggRegex *re;
    char *re_pattern;
    EggRegexCompileFlags flags = 0;
    GError *error = NULL;

    g_return_val_if_fail (pattern != NULL, NULL);

#ifdef __WIN32__
    flags = EGG_REGEX_CASELESS;
#endif

    if (!(re_pattern = glob_to_re (pattern)))
        return NULL;

    re = egg_regex_new (re_pattern, flags, 0, &error);

    g_free (re_pattern);

    if (!re)
    {
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
        return NULL;
    }

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

    return egg_regex_match (glob->re, filename_utf8, 0);
}
#endif


static void
_moo_glob_free (MooGlob *glob)
{
    if (glob)
    {
#ifdef MOO_GLOB_REGEX
        egg_regex_free (glob->re);
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
