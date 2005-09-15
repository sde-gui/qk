/*
 *   mooutils-gobject.h
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

#ifndef __MOO_UTILS_GOBJECT_H__
#define __MOO_UTILS_GOBJECT_H__

#include <gdk/gdkcolor.h>

G_BEGIN_DECLS


/*****************************************************************************/
/* GType type
 */

#define MOO_TYPE_GTYPE                  (moo_gtype_get_type())

GType           moo_gtype_get_type          (void);

void            moo_value_set_gtype         (GValue     *value,
                                             GType       v_gtype);
GType           moo_value_get_gtype         (const GValue *value);


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
double          moo_value_convert_to_double (const GValue   *val);
const GdkColor *moo_value_convert_to_color  (const GValue   *val);
const char     *moo_value_convert_to_string (const GValue   *val);


/*****************************************************************************/
/* GParameter array manipulation
 */

GParameter  *moo_param_array_collect        (GType       type,
                                             guint      *len,
                                             const char *first_prop_name,
                                             ...);
GParameter  *moo_param_array_collect_valist (GType       type,
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
/* MooObjectFactory
 */

#define MOO_TYPE_OBJECT_FACTORY              (moo_object_factory_get_type ())
#define MOO_OBJECT_FACTORY(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_OBJECT_FACTORY, MooObjectFactory))
#define MOO_OBJECT_FACTORY_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_OBJECT_FACTORY, MooObjectFactoryClass))
#define MOO_IS_OBJECT_FACTORY(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_OBJECT_FACTORY))
#define MOO_IS_OBJECT_FACTORY_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_OBJECT_FACTORY))
#define MOO_OBJECT_FACTORY_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_OBJECT_FACTORY, MooObjectFactoryClass))


typedef struct _MooObjectFactory        MooObjectFactory;
typedef struct _MooObjectFactoryClass   MooObjectFactoryClass;

typedef GObject *(*MooObjectFactoryFunc) (gpointer             data,
                                          MooObjectFactory    *factory);

struct _MooObjectFactory
{
    GObject      object;

    MooObjectFactoryFunc factory_func;
    gpointer             factory_func_data;

    GType       object_type;
    guint       n_params;
    GParameter *params;
};

struct _MooObjectFactoryClass
{
    GObjectClass parent_class;
};


GType               moo_object_factory_get_type         (void) G_GNUC_CONST;

MooObjectFactory   *moo_object_factory_new_func         (MooObjectFactoryFunc factory_func,
                                                         gpointer            data);

MooObjectFactory   *moo_object_factory_new              (GType               object_type,
                                                         const char         *first_prop_name,
                                                         ...);
MooObjectFactory   *moo_object_factory_new_valist       (GType               object_type,
                                                         const char         *first_prop_name,
                                                         va_list             var_args);
MooObjectFactory   *moo_object_factory_new_a            (GType               object_type,
                                                         GParameter         *params,
                                                         guint               n_params);

gpointer            moo_object_factory_create_object    (MooObjectFactory   *factory,
                                                         gpointer            data,
                                                         const char         *additional_prop_name,
                                                         ...);


G_END_DECLS

#endif /* __MOO_UTILS_GOBJECT_H__ */
