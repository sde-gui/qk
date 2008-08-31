/*
 *   moo-mime.h
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_MIME_H
#define MOO_MIME_H

#include <glib.h>

G_BEGIN_DECLS


/* All public functions here are thread-safe */

#define MOO_MIME_TYPE_UNKNOWN (moo_mime_type_unknown ())

const char  *moo_mime_type_unknown              (void) G_GNUC_CONST;

struct stat;
const char  *moo_get_mime_type_for_file         (const char     *filename,
                                                 struct stat    *statbuf);
const char  *moo_get_mime_type_for_filename     (const char     *filename);
gboolean     moo_mime_type_is_subclass          (const char     *mime_type,
                                                 const char     *base);
const char **moo_mime_type_list_parents         (const char     *mime_type);
void         moo_mime_shutdown                  (void);

const char *const *_moo_get_mime_data_dirs      (void);


G_END_DECLS

#endif /* MOO_MIME_H */
