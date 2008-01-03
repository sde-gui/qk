/*
 *   mooutils-fs.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_UTILS_FS_H
#define MOO_UTILS_FS_H

#include <glib.h>

/* MSVC */
#if defined(WIN32) && !defined(__GNUC__)
#include <sys/stat.h>
#ifndef S_ISREG
#define S_ISREG(m) (((m) & (_S_IFMT)) == (_S_IFREG))
#define S_ISDIR(m) (((m) & (_S_IFMT)) == (_S_IFDIR))
#endif
#endif

G_BEGIN_DECLS


#define MOO_FILE_ERROR (_moo_file_error_quark ())

/* XXX */
#define _moo_save_file_utf8 moo_save_file_utf8
#define _moo_unlink moo_unlink

typedef enum
{
    MOO_FILE_ERROR_NONEXISTENT,
    MOO_FILE_ERROR_NOT_FOLDER,
    MOO_FILE_ERROR_BAD_FILENAME,
    MOO_FILE_ERROR_FAILED,
    MOO_FILE_ERROR_ALREADY_EXISTS,
    MOO_FILE_ERROR_ACCESS_DENIED,
    MOO_FILE_ERROR_READONLY,
    MOO_FILE_ERROR_DIFFERENT_FS,
    MOO_FILE_ERROR_NOT_IMPLEMENTED
} MooFileError;

GQuark          _moo_file_error_quark       (void) G_GNUC_CONST;
MooFileError    _moo_file_error_from_errno  (int         err_code);

gboolean        _moo_save_file_utf8         (const char *name,
                                             const char *text,
                                             gssize      len,
                                             GError    **error);
gboolean        _moo_remove_dir             (const char *path,
                                             gboolean    recursive,
                                             GError    **error);
gboolean        _moo_create_dir             (const char *path,
                                             GError    **error);
int             _moo_mkdir_with_parents     (const char *path); /* S_IRWXU on unix */
gboolean        _moo_rename_file            (const char *path,
                                             const char *new_path,
                                             GError    **error);

char          **moo_filenames_from_locale   (char      **files);
char           *moo_filename_from_locale    (const char *file);

char           *_moo_normalize_file_path    (const char *filename);
char           *_moo_normalize_dir_path     (const char *filename);

/*
 * C library and WinAPI functions wrappers analogous to glib/gstdio.h
 */

int             _moo_unlink                 (const char *path);
int             _moo_mkdir                  (const char *path); /* S_IRWXU on unix */
int             _moo_remove                 (const char *path);
gpointer        _moo_fopen                  (const char *path,
                                             const char *mode);
int             _moo_rename                 (const char *old_name,
                                             const char *new_name);

gboolean        _moo_glob_match_simple      (const char *pattern,
                                             const char *filename);


G_END_DECLS

#endif /* MOO_UTILS_FS_H */
