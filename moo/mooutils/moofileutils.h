/*
 *   mooutils/moofileutils.h
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

#ifndef MOOUTILS_FILEUTILS_H
#define MOOUTILS_FILEUTILS_H

#include <glib/gerror.h>

G_BEGIN_DECLS


/* TODO */
/* st_mtime can't be negative, so error indicator is
   moo_get_file_mtime(file) < 0 */

#define MOO_EINVAL  ((GTime) -1)
#define MOO_EACCESS ((GTime) -2)
#define MOO_EIO     ((GTime) -3)
#define MOO_ENOENT  ((GTime) -4)

GTime       moo_get_file_mtime      (const char *filename);


gboolean    moo_save_file_utf8      (const char *name,
                                     const char *text,
                                     gssize      len,
                                     GError    **error);

gboolean    moo_open_url            (const char *url);
gboolean    moo_open_email          (const char *address,
                                     const char *subject,
                                     const char *body);


G_END_DECLS

#endif /* MOOUTILS_FILEUTILS_H */
