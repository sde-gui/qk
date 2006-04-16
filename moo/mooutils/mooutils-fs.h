/*
 *   mooutils-fs.h
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

#ifndef __MOO_UTILS_FS_H__
#define __MOO_UTILS_FS_H__

#include <glib.h>

G_BEGIN_DECLS


#define MOO_FILE_ERROR (moo_file_error_quark ())

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

GQuark          moo_file_error_quark        (void) G_GNUC_CONST;
MooFileError    moo_file_error_from_errno   (int         err_code);

gboolean        moo_save_file_utf8          (const char *name,
                                             const char *text,
                                             gssize      len,
                                             GError    **error);
gboolean        moo_rmdir                   (const char *path,
                                             gboolean    recursive,
                                             GError    **error);
gboolean        moo_mkdir                   (const char *path,
                                             GError    **error);

char          **moo_filenames_from_locale   (char      **files);

/*
 * C library and WinAPI functions wrappers analogous to glib/gstdio.h
 */

int             m_unlink                    (const char *path);
int             m_mkdir                     (const char *path); /* S_IRWXU on unix */
int             m_rmdir                     (const char *path);
int             m_remove                    (const char *path);
gpointer        m_fopen                     (const char *path,
                                             const char *mode);
int             m_rename                    (const char *old_name,
                                             const char *new_name);


G_END_DECLS

#endif /* __MOO_UTILS_FS_H__ */
