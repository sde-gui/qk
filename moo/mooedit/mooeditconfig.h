/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooeditconfig.h
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

#ifndef __MOO_EDIT_CONFIG_H__
#define __MOO_EDIT_CONFIG_H__

#include <glib-object.h>

G_BEGIN_DECLS


#define MOO_TYPE_EDIT_CONFIG              (moo_edit_config_get_type ())
#define MOO_EDIT_CONFIG(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_EDIT_CONFIG, MooEditConfig))
#define MOO_EDIT_CONFIG_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_EDIT_CONFIG, MooEditConfigClass))
#define MOO_IS_EDIT_CONFIG(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_EDIT_CONFIG))
#define MOO_IS_EDIT_CONFIG_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_EDIT_CONFIG))
#define MOO_EDIT_CONFIG_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_EDIT_CONFIG, MooEditConfigClass))

#define MOO_TYPE_EDIT_CONFIG_SOURCE       (moo_edit_config_source_get_type ())

typedef struct _MooEditConfig         MooEditConfig;
typedef struct _MooEditConfigPrivate  MooEditConfigPrivate;
typedef struct _MooEditConfigClass    MooEditConfigClass;


typedef enum {
    MOO_EDIT_CONFIG_SOURCE_USER     = 0,
    MOO_EDIT_CONFIG_SOURCE_FILE     = 10,
    MOO_EDIT_CONFIG_SOURCE_FILENAME = 20,
    MOO_EDIT_CONFIG_SOURCE_PREFS    = 30,
    MOO_EDIT_CONFIG_SOURCE_AUTO     = 40
} MooEditConfigSource;

struct _MooEditConfig
{
    GObject object;
    MooEditConfigPrivate *priv;
};

struct _MooEditConfigClass
{
    GObjectClass object_class;
};


GType           moo_edit_config_get_type        (void) G_GNUC_CONST;
GType           moo_edit_config_source_get_type (void) G_GNUC_CONST;

MooEditConfig  *moo_edit_config_new             (void);

const char     *moo_edit_config_get_string      (MooEditConfig  *config,
                                                 const char     *setting);
guint           moo_edit_config_get_uint        (MooEditConfig  *config,
                                                 const char     *setting);
gboolean        moo_edit_config_get_bool        (MooEditConfig  *config,
                                                 const char     *setting);

void            moo_edit_config_set             (MooEditConfig  *config,
                                                 const char     *first_setting,
                                                 MooEditConfigSource source,
                                                 ...); /* setting, source, value, ... */
void            moo_edit_config_get             (MooEditConfig  *config,
                                                 const char     *first_setting,
                                                 ...); /* alias for g_object_get() */

void            moo_edit_config_set_global      (const char     *first_setting,
                                                 MooEditConfigSource source,
                                                 ...); /* setting, source, value, ... */
void            moo_edit_config_get_global      (const char     *first_setting,
                                                 ...); /* alias for g_object_get(global, ...) */

void            moo_edit_config_parse           (MooEditConfig  *config,
                                                 const char     *setting_name,
                                                 const char     *value,
                                                 MooEditConfigSource source);

guint           moo_edit_config_install_setting (GParamSpec     *pspec);
void            moo_edit_config_install_alias   (const char     *name,
                                                 const char     *alias);

guint           moo_edit_config_get_setting_id  (GParamSpec     *pspec);
GParamSpec     *moo_edit_config_get_spec        (guint           id);
GParamSpec     *moo_edit_config_lookup_spec     (const char     *name,
                                                 guint          *id,
                                                 gboolean        try_alias);

void            moo_edit_config_unset_by_source (MooEditConfig  *config,
                                                 MooEditConfigSource source);


G_END_DECLS

#endif /* __MOO_EDIT_CONFIG_H__ */
