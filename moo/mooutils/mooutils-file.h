/*
 *   mooutils-file.h
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_UTILS_FILE_H
#define MOO_UTILS_FILE_H

#include <glib-object.h>

G_BEGIN_DECLS


typedef struct MooFileWriter MooFileWriter;

MooFileWriter  *moo_file_writer_new             (const char     *filename,
                                                 gboolean        save_backup,
                                                 GError        **error);
MooFileWriter  *moo_text_writer_new             (const char     *filename,
                                                 gboolean        save_backup,
                                                 GError        **error);
MooFileWriter  *moo_string_writer_new           (void);

gboolean        moo_file_writer_write           (MooFileWriter  *writer,
                                                 const char     *data,
                                                 gssize          len);
gboolean        moo_file_writer_printf          (MooFileWriter  *writer,
                                                 const char     *fmt,
                                                 ...) G_GNUC_PRINTF (2, 3);
gboolean        moo_file_writer_close           (MooFileWriter  *writer,
                                                 GError        **error);

const char     *moo_string_writer_get_string    (MooFileWriter  *writer,
                                                 gsize          *len);


G_END_DECLS

#endif /* MOO_UTILS_FILE_H */
