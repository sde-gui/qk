/*
 *   mooutils-gobject-private.h
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

#ifndef __MOO_UTILS_GOBJECT_PRIVATE_H__
#define __MOO_UTILS_GOBJECT_PRIVATE_H__

#include <mooutils/mooutils-gobject.h>

G_BEGIN_DECLS


/*****************************************************************************/
/* GType type
 */

#define MOO_TYPE_PARAM_GTYPE            (_moo_param_gtype_get_type())
#define MOO_IS_PARAM_SPEC_GTYPE(pspec)  (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), MOO_TYPE_PARAM_GTYPE))
#define MOO_PARAM_SPEC_GTYPE(pspec)     (G_TYPE_CHECK_INSTANCE_CAST ((pspec), MOO_TYPE_PARAM_GTYPE, MooParamSpecGType))

typedef struct _MooParamSpecGType MooParamSpecGType;

struct _MooParamSpecGType
{
    GParamSpec parent;
    GType base;
};

GType           _moo_param_gtype_get_type   (void) G_GNUC_CONST;
GParamSpec     *_moo_param_spec_gtype       (const char     *name,
                                             const char     *nick,
                                             const char     *blurb,
                                             GType           base,
                                             GParamFlags     flags);
void            _moo_value_set_gtype        (GValue         *value,
                                             GType           v_gtype);


/*****************************************************************************/
/* Converting values forth and back
 */

gboolean        _moo_value_convert_to_bool  (const GValue   *val);
int             _moo_value_convert_to_int   (const GValue   *val);
int             _moo_value_convert_to_enum  (const GValue   *val,
                                             GType           enum_type);
int             _moo_value_convert_to_flags (const GValue   *val,
                                             GType           flags_type);
const GdkColor *_moo_value_convert_to_color (const GValue   *val);


/*****************************************************************************/
/* GParameter array manipulation
 */

typedef GParamSpec* (*MooLookupProperty)    (GObjectClass *klass,
                                             const char   *prop_name);

GParameter  *_moo_param_array_collect       (GType       type,
                                             MooLookupProperty lookup_func,
                                             guint      *len,
                                             const char *first_prop_name,
                                             ...) G_GNUC_NULL_TERMINATED;
GParameter  *_moo_param_array_collect_valist(GType       type,
                                             MooLookupProperty lookup_func,
                                             guint      *len,
                                             const char *first_prop_name,
                                             va_list     var_args);

GParameter  *_moo_param_array_add           (GType       type,
                                             GParameter *src,
                                             guint       len,
                                             guint      *new_len,
                                             const char *first_prop_name,
                                             ...) G_GNUC_NULL_TERMINATED;
GParameter  *_moo_param_array_add_valist    (GType       type,
                                             GParameter *src,
                                             guint       len,
                                             guint      *new_len,
                                             const char *first_prop_name,
                                             va_list     var_args);

GParameter  *_moo_param_array_add_type      (GParameter *src,
                                             guint       len,
                                             guint      *new_len,
                                             const char *first_prop_name,
                                             ...) G_GNUC_NULL_TERMINATED;
GParameter  *_moo_param_array_add_type_valist(GParameter *src,
                                             guint       len,
                                             guint      *new_len,
                                             const char *first_prop_name,
                                             va_list     var_args);


/*****************************************************************************/
/* Property watch
 */

guint       _moo_bind_signal                (gpointer        target,
                                             const char     *target_signal,
                                             gpointer        source,
                                             const char     *source_signal);
gboolean    _moo_sync_bool_property         (gpointer        slave,
                                             const char     *slave_prop,
                                             gpointer        master,
                                             const char     *master_prop,
                                             gboolean        invert);

void        _moo_copy_boolean               (GValue         *target,
                                             const GValue   *source,
                                             gpointer        dummy);
void        _moo_invert_boolean             (GValue         *target,
                                             const GValue   *source,
                                             gpointer        dummy);

typedef void (*MooTransformPropFunc)        (GValue         *target,
                                             const GValue   *source,
                                             gpointer        data);
typedef void (*MooGetPropFunc)              (GObject        *source,
                                             GValue         *target,
                                             gpointer        data);

guint       _moo_add_property_watch         (gpointer        target,
                                             const char     *target_prop,
                                             gpointer        source,
                                             const char     *source_prop,
                                             MooTransformPropFunc transform,
                                             gpointer        transform_data,
                                             GDestroyNotify  destroy_notify);

void        _moo_watch_add_signal           (guint           watch_id,
                                             const char     *source_signal);
void        _moo_watch_add_property         (guint           watch_id,
                                             const char     *source_prop);
gboolean    _moo_watch_remove               (guint           watch_id);


/*****************************************************************************/
/* Data store
 */

#define MOO_TYPE_PTR    (_moo_ptr_get_type ())
#define MOO_TYPE_DATA   (_moo_data_get_type ())

typedef struct _MooPtr MooPtr;

struct _MooPtr {
    guint ref_count;
    gpointer data;
    GDestroyNotify free_func;
};

GType    _moo_ptr_get_type          (void) G_GNUC_CONST;
GType    _moo_data_get_type         (void) G_GNUC_CONST;

/* There are no ref-counted NULL pointers! */
MooPtr  *_moo_ptr_new               (gpointer        data,
                                     GDestroyNotify  free_func);
MooPtr  *_moo_ptr_ref               (MooPtr         *ptr);
void     _moo_ptr_unref             (MooPtr         *ptr);

guint    _moo_data_size             (MooData        *data);

gboolean _moo_data_has_key          (MooData        *data,
                                     gpointer        key);
GType    _moo_data_get_value_type   (MooData        *data,
                                     gpointer        key);



G_END_DECLS

#endif /* __MOO_UTILS_GOBJECT_PRIVATE_H__ */
