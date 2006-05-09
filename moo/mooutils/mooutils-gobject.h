/*
 *   mooutils-gobject.h
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

#ifndef __MOO_UTILS_GOBJECT_H__
#define __MOO_UTILS_GOBJECT_H__

#include <gtk/gtkwidget.h>
#include <mooutils/mooclosure.h>

G_BEGIN_DECLS


/*****************************************************************************/
/* GType type
 */

#define MOO_TYPE_GTYPE                  (moo_gtype_get_type())

#define MOO_TYPE_PARAM_GTYPE            (moo_param_gtype_get_type())
#define MOO_IS_PARAM_SPEC_GTYPE(pspec)  (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), MOO_TYPE_PARAM_GTYPE))
#define MOO_PARAM_SPEC_GTYPE(pspec)     (G_TYPE_CHECK_INSTANCE_CAST ((pspec), MOO_TYPE_PARAM_GTYPE, MooParamSpecGType))

typedef struct _MooParamSpecGType MooParamSpecGType;

struct _MooParamSpecGType
{
    GParamSpec parent;
    GType base;
};

GType           moo_gtype_get_type          (void) G_GNUC_CONST;
GType           moo_param_gtype_get_type    (void) G_GNUC_CONST;

GParamSpec     *moo_param_spec_gtype        (const char     *name,
                                             const char     *nick,
                                             const char     *blurb,
                                             GType           base,
                                             GParamFlags     flags);

void            moo_value_set_gtype         (GValue         *value,
                                             GType           v_gtype);
GType           moo_value_get_gtype         (const GValue   *value);


/*****************************************************************************/
/* Converting values forth and back
 */

gboolean        moo_value_type_supported    (GType           type);
gboolean        moo_value_convert           (const GValue   *src,
                                             GValue         *dest);
gboolean        moo_value_equal             (const GValue   *a,
                                             const GValue   *b);

gboolean        moo_value_change_type       (GValue         *val,
                                             GType           new_type);

gboolean        moo_value_convert_to_bool   (const GValue   *val);
int             moo_value_convert_to_int    (const GValue   *val);
int             moo_value_convert_to_enum   (const GValue   *val,
                                             GType           enum_type);
int             moo_value_convert_to_flags  (const GValue   *val,
                                             GType           flags_type);
double          moo_value_convert_to_double (const GValue   *val);
const GdkColor *moo_value_convert_to_color  (const GValue   *val);
const char     *moo_value_convert_to_string (const GValue   *val);
gboolean        moo_value_convert_from_string (const char   *string,
                                               GValue       *val);

gboolean        moo_convert_string_to_bool  (const char     *string,
                                             gboolean        default_val);
int             moo_convert_string_to_int   (const char     *string,
                                             int             default_val);
const char     *moo_convert_bool_to_string  (gboolean        value);
const char     *moo_convert_int_to_string   (int             value);


/*****************************************************************************/
/* GParameter array manipulation
 */

typedef GParamSpec* (*MooLookupProperty)    (GObjectClass *klass,
                                             const char   *prop_name);

GParameter  *moo_param_array_collect        (GType       type,
                                             MooLookupProperty lookup_func,
                                             guint      *len,
                                             const char *first_prop_name,
                                             ...);
GParameter  *moo_param_array_collect_valist (GType       type,
                                             MooLookupProperty lookup_func,
                                             guint      *len,
                                             const char *first_prop_name,
                                             va_list     var_args);

GParameter  *moo_param_array_add            (GType       type,
                                             GParameter *src,
                                             guint       len,
                                             guint      *new_len,
                                             const char *first_prop_name,
                                             ...);
GParameter  *moo_param_array_add_valist     (GType       type,
                                             GParameter *src,
                                             guint       len,
                                             guint      *new_len,
                                             const char *first_prop_name,
                                             va_list     var_args);

GParameter  *moo_param_array_add_type       (GParameter *src,
                                             guint       len,
                                             guint      *new_len,
                                             const char *first_prop_name,
                                             ...);
GParameter  *moo_param_array_add_type_valist(GParameter *src,
                                             guint       len,
                                             guint      *new_len,
                                             const char *first_prop_name,
                                             va_list     var_args);

void         moo_param_array_free           (GParameter *array,
                                             guint       len);


/*****************************************************************************/
/* Signal that does not require class method
 */

guint moo_signal_new_cb (const gchar        *signal_name,
                         GType               itype,
                         GSignalFlags        signal_flags,
                         GCallback           handler,
                         GSignalAccumulator  accumulator,
                         gpointer            accu_data,
                         GSignalCMarshaller  c_marshaller,
                         GType               return_type,
                         guint               n_params,
                         ...);


/*****************************************************************************/
/* Property watch
 */

typedef struct _MooObjectWatch MooObjectWatch;
typedef struct _MooObjectWatchClass MooObjectWatchClass;
typedef void (*MooObjectWatchNotify) (MooObjectWatch *watch);

struct _MooObjectWatch {
    MooObjectWatchClass *klass;
    MooObjectPtr *source;
    MooObjectPtr *target;
    GDestroyNotify notify;
    gpointer notify_data;
    guint id;
};

struct _MooObjectWatchClass {
    MooObjectWatchNotify source_notify;
    MooObjectWatchNotify target_notify;
    MooObjectWatchNotify destroy;
};

MooObjectWatch *moo_object_watch_alloc (gsize                size,
                                        MooObjectWatchClass *klass,
                                        gpointer             source,
                                        gpointer             target,
                                        GDestroyNotify       notify,
                                        gpointer             notify_data);
#define moo_object_watch_new(Type_,klass_,src_,tgt_,notify_,data_) \
    ((Type_*) moo_object_watch_alloc (sizeof (Type_), klass_, src_, tgt_, notify_, data_))


guint       moo_bind_signal         (gpointer            target,
                                     const char         *target_signal,
                                     gpointer            source,
                                     const char         *source_signal);

void        moo_bind_sensitive      (GtkWidget          *toggle_btn,
                                     GtkWidget         **dependent,
                                     int                 num_dependent,
                                     gboolean            invert);

guint       moo_bind_bool_property  (gpointer            target,
                                     const char         *target_prop,
                                     gpointer            source,
                                     const char         *source_prop,
                                     gboolean            invert);
gboolean    moo_sync_bool_property  (gpointer            slave,
                                     const char         *slave_prop,
                                     gpointer            master,
                                     const char         *master_prop,
                                     gboolean            invert);

typedef void (*MooTransformPropFunc)(GValue             *target,
                                     const GValue       *source,
                                     gpointer            data);
typedef void (*MooGetPropFunc)      (GObject            *source,
                                     GValue             *target,
                                     gpointer            data);

void        moo_copy_boolean        (GValue             *target,
                                     const GValue       *source,
                                     gpointer            dummy);
void        moo_invert_boolean      (GValue             *target,
                                     const GValue       *source,
                                     gpointer            dummy);

guint       moo_add_property_watch  (gpointer            target,
                                     const char         *target_prop,
                                     gpointer            source,
                                     const char         *source_prop,
                                     MooTransformPropFunc transform,
                                     gpointer            transform_data,
                                     GDestroyNotify      destroy_notify);
void        moo_watch_add_signal    (guint               watch_id,
                                     const char         *source_signal);
void        moo_watch_add_property  (guint               watch_id,
                                     const char         *source_prop);
gboolean    moo_watch_remove        (guint               watch_id);


/*****************************************************************************/
/* Data store
 */

#define MOO_TYPE_PTR    (moo_ptr_get_type ())
#define MOO_TYPE_DATA   (moo_data_get_type ())

typedef struct _MooPtr MooPtr;
typedef struct _MooData MooData;


struct _MooPtr {
    guint ref_count;
    gpointer data;
    GDestroyNotify free_func;
};

GType    moo_ptr_get_type           (void) G_GNUC_CONST;
GType    moo_data_get_type          (void) G_GNUC_CONST;

/* There are no ref-counted NULL pointers! */
MooPtr  *moo_ptr_new                (gpointer        data,
                                     GDestroyNotify  free_func);
MooPtr  *moo_ptr_ref                (MooPtr         *ptr);
void     moo_ptr_unref              (MooPtr         *ptr);

MooData *moo_data_new               (GHashFunc       hash_func,
                                     GEqualFunc      key_equal_func,
                                     GDestroyNotify  key_destroy_func);

/* these accept NULL */
MooData *moo_data_ref               (MooData        *data);
void     moo_data_unref             (MooData        *data);
#define  moo_data_destroy  moo_data_unref

void     moo_data_insert_value      (MooData        *data,
                                     gpointer        key,
                                     const GValue   *value);
void     moo_data_insert_ptr        (MooData        *data,
                                     gpointer        key,
                                     gpointer        value,
                                     GDestroyNotify  destroy);

void     moo_data_remove            (MooData        *data,
                                     gpointer        key);
void     moo_data_clear             (MooData        *data);
guint    moo_data_size              (MooData        *data);

gboolean moo_data_has_key           (MooData        *data,
                                     gpointer        key);
GType    moo_data_get_value_type    (MooData        *data,
                                     gpointer        key);

/* dest must not be initialized */
gboolean moo_data_get_value         (MooData        *data,
                                     gpointer        key,
                                     GValue         *dest);
gpointer moo_data_get_ptr           (MooData        *data,
                                     gpointer        key);


G_END_DECLS

#endif /* __MOO_UTILS_GOBJECT_H__ */
