/*
 *   mookeyfile.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_KEY_FILE_H
#define MOO_KEY_FILE_H

#include <glib-object.h>

G_BEGIN_DECLS


#define MOO_TYPE_KEY_FILE_ITEM        (moo_key_file_item_get_type ())

#define MOO_TYPE_KEY_FILE             (moo_key_file_get_type ())
#define MOO_KEY_FILE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_KEY_FILE, MooKeyFile))
#define MOO_KEY_FILE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_KEY_FILE, MooKeyFileClass))
#define MOO_IS_KEY_FILE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_KEY_FILE))
#define MOO_IS_KEY_FILE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_KEY_FILE))
#define MOO_KEY_FILE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_KEY_FILE, MooKeyFileClass))


typedef struct _MooKeyFile      MooKeyFile;
typedef struct _MooKeyFileItem  MooKeyFileItem;


GType           moo_key_file_get_type           (void) G_GNUC_CONST;
GType           moo_key_file_item_get_type      (void) G_GNUC_CONST;

MooKeyFile     *moo_key_file_new                (void);
MooKeyFile     *moo_key_file_new_from_file      (const char         *filename,
                                                 GError            **error);

MooKeyFile     *moo_key_file_ref                (MooKeyFile         *key_file);
void            moo_key_file_unref              (MooKeyFile         *key_file);

char            *moo_key_file_format            (MooKeyFile         *key_file,
                                                 const char         *comment,
                                                 guint               indent);

guint            moo_key_file_n_items           (MooKeyFile         *key_file);
MooKeyFileItem  *moo_key_file_nth_item          (MooKeyFile         *key_file,
                                                 guint               n);
MooKeyFileItem  *moo_key_file_new_item          (MooKeyFile         *key_file,
                                                 const char         *name);

const char      *moo_key_file_item_name         (MooKeyFileItem     *item);

const char      *moo_key_file_item_get          (MooKeyFileItem     *item,
                                                 const char         *key);
char            *moo_key_file_item_steal        (MooKeyFileItem     *item,
                                                 const char         *key);
void             moo_key_file_item_set          (MooKeyFileItem     *item,
                                                 const char         *key,
                                                 const char         *value);
gboolean         moo_key_file_item_steal_bool   (MooKeyFileItem     *item,
                                                 const char         *key,
                                                 gboolean            default_val);
void             moo_key_file_item_set_bool     (MooKeyFileItem     *item,
                                                 const char         *key,
                                                 gboolean            value);
void             moo_key_file_item_foreach      (MooKeyFileItem     *item,
                                                 GHFunc              func,
                                                 gpointer            data);

const char      *moo_key_file_item_get_content  (MooKeyFileItem     *item);
char            *moo_key_file_item_steal_content(MooKeyFileItem     *item);
void             moo_key_file_item_set_content  (MooKeyFileItem     *item,
                                                 const char         *content);


G_END_DECLS

#endif /* MOO_KEY_FILE_H */
