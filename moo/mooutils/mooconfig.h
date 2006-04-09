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

#include <glib.h>

G_BEGIN_DECLS


typedef struct _MooConfig MooConfig;
typedef struct _MooConfigItem MooConfigItem;


MooConfig      *moo_config_new              (void);
void            moo_config_free             (MooConfig  *config);

MooConfig      *moo_config_parse            (const char *string,
                                             int         len);
MooConfig      *moo_config_parse_file       (const char *filename);
char           *moo_config_format           (MooConfig  *config);
gboolean        moo_config_save             (MooConfig  *config,
                                             const char *file,
                                             GError    **error);

guint           moo_config_n_items          (MooConfig  *config);
MooConfigItem  *moo_config_nth_item         (MooConfig  *config,
                                             guint       n);
MooConfigItem  *moo_config_new_item         (MooConfig  *config);

const char     *moo_config_item_get_value   (MooConfigItem  *item,
                                             const char     *key);
void            moo_config_item_set_value   (MooConfigItem  *item,
                                             const char     *key,
                                             const char     *value);
gboolean        moo_config_item_get_bool    (MooConfigItem  *item,
                                             const char     *key,
                                             gboolean        default_val);
void            moo_config_item_set_bool    (MooConfigItem  *item,
                                             const char     *key,
                                             gboolean        value);

const char     *moo_config_item_get_content (MooConfigItem  *item);
void            moo_config_item_set_content (MooConfigItem  *item,
                                             const char     *content);


G_END_DECLS

#endif /* __MOO_CONFIG_H__ */
