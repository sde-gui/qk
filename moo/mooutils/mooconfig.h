/*
 *   mooconfig.h
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

#ifndef __MOO_CONFIG_H__
#define __MOO_CONFIG_H__

#include <gtk/gtktreemodel.h>

G_BEGIN_DECLS


#define MOO_CONFIG_ERROR            (moo_config_error_quark ())
#define MOO_TYPE_CONFIG_ITEM        (moo_config_item_get_type ())

#define MOO_TYPE_CONFIG             (moo_config_get_type ())
#define MOO_CONFIG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_CONFIG, MooConfig))
#define MOO_CONFIG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_CONFIG, MooConfigClass))
#define MOO_IS_CONFIG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_CONFIG))
#define MOO_IS_CONFIG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_CONFIG))
#define MOO_CONFIG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_CONFIG, MooConfigClass))


typedef enum {
    MOO_CONFIG_ERROR_PARSE,
    MOO_CONFIG_ERROR_FILE
} MooConfigError;

typedef struct _MooConfig MooConfig;
typedef struct _MooConfigPrivate MooConfigPrivate;
typedef struct _MooConfigClass MooConfigClass;
typedef struct _MooConfigItem MooConfigItem;


struct _MooConfig {
    GObject parent;
    MooConfigPrivate *priv;
};

struct _MooConfigClass {
    GObjectClass parent_class;
};


GType           moo_config_get_type         (void) G_GNUC_CONST;
GType           moo_config_item_get_type    (void) G_GNUC_CONST;
GQuark          moo_config_error_quark      (void) G_GNUC_CONST;

MooConfig      *moo_config_new              (void);
MooConfig      *moo_config_new_from_file    (const char     *filename,
                                             gboolean        modified,
                                             GError        **error);
MooConfig      *moo_config_new_from_buffer  (const char     *string,
                                             int             len,
                                             gboolean        modified,
                                             GError        **error);
void            moo_config_free             (MooConfig      *config);

gboolean        moo_config_parse_buffer     (MooConfig      *config,
                                             const char     *string,
                                             int             len,
                                             gboolean        modify,
                                             GError        **error);
gboolean        moo_config_parse_file       (MooConfig      *config,
                                             const char     *filename,
                                             gboolean        modify,
                                             GError        **error);

char           *moo_config_format           (MooConfig      *config);
gboolean        moo_config_save             (MooConfig      *config,
                                             const char     *file,
                                             GError        **error);

gboolean        moo_config_get_modified     (MooConfig      *config);
void            moo_config_set_modified     (MooConfig      *config,
                                             gboolean        modified);

guint           moo_config_n_items          (MooConfig      *config);
MooConfigItem  *moo_config_nth_item         (MooConfig      *config,
                                             guint           n);
MooConfigItem  *moo_config_new_item         (MooConfig      *config,
                                             int             index,
                                             gboolean        modify);
void            moo_config_delete_item      (MooConfig      *config,
                                             guint           index,
                                             gboolean        modify);
void            moo_config_move_item        (MooConfig      *config,
                                             guint           index,
                                             guint           new_index,
                                             gboolean        modify);

void            moo_config_set_default_bool (MooConfig      *config,
                                             const char     *key,
                                             gboolean        value);

const char     *moo_config_item_get         (MooConfigItem  *item,
                                             const char     *key);
void            moo_config_set              (MooConfig      *config,
                                             MooConfigItem  *item,
                                             const char     *key,
                                             const char     *value,
                                             gboolean        modify);
gboolean        moo_config_get_bool         (MooConfig      *config,
                                             MooConfigItem  *item,
                                             const char     *key);
void            moo_config_set_bool         (MooConfig      *config,
                                             MooConfigItem  *item,
                                             const char     *key,
                                             gboolean        value,
                                             gboolean        modify);

const char     *moo_config_item_get_content (MooConfigItem  *item);
void            moo_config_set_item_content (MooConfig      *config,
                                             MooConfigItem  *item,
                                             const char     *content,
                                             gboolean        modify);

void            moo_config_get_item_iter    (MooConfig      *config,
                                             MooConfigItem  *item,
                                             GtkTreeIter    *iter);


G_END_DECLS

#endif /* __MOO_CONFIG_H__ */
