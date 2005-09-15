/*
 *   mooutils-gobject.c
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

#include "mooutils/mooutils-gobject.h"
#include <gobject/gvaluecollector.h>
#include <string.h>


/*****************************************************************************/
/* GType type
 */

static gpointer
copy_gtype (gpointer boxed)
{
    return boxed;
}

static void
free_gtype (G_GNUC_UNUSED gpointer boxed)
{
}


GType
moo_gtype_get_type (void)
{
    static GType type = 0;
    if (!type)
    {
        type = g_boxed_type_register_static ("GType",
                                             copy_gtype, free_gtype);
    }

    return type;
}


void
moo_value_set_gtype (GValue     *value,
                     GType       v_gtype)
{
    g_value_set_static_boxed (value, (gconstpointer) v_gtype);
}


GType
moo_value_get_gtype (const GValue *value)
{
    return (GType) g_value_get_boxed (value);
}


/*****************************************************************************/
/* Converting values forth and back
 */

gboolean
moo_value_convert (const GValue   *src,
                   GValue         *dest)
{
    GType src_type, dest_type;

    g_return_val_if_fail (G_IS_VALUE (src) && G_IS_VALUE (dest), FALSE);

    src_type = G_VALUE_TYPE (src);
    dest_type = G_VALUE_TYPE (dest);

    g_return_val_if_fail (moo_value_type_supported (src_type), FALSE);
    g_return_val_if_fail (moo_value_type_supported (dest_type), FALSE);

    if (src_type == dest_type)
    {
        g_value_copy (src, dest);
        return TRUE;
    }

    if (dest_type == G_TYPE_STRING)
    {
        if (src_type == G_TYPE_BOOLEAN)
        {
            const char *string =
                    g_value_get_boolean (src) ? "TRUE" : "FALSE";
            g_value_set_static_string (dest, string);
            return TRUE;
        }

        if (src_type == G_TYPE_DOUBLE)
        {
            char *string =
                    g_strdup_printf ("%f", g_value_get_double (src));
            g_value_take_string (dest, string);
            return TRUE;
        }

        if (src_type == G_TYPE_INT)
        {
            char *string =
                    g_strdup_printf ("%d", g_value_get_int (src));
            g_value_take_string (dest, string);
            return TRUE;
        }

        if (src_type == GDK_TYPE_COLOR)
        {
            char string[14];
            const GdkColor *color = g_value_get_boxed (src);

            if (!color)
            {
                g_value_set_string (dest, NULL);
                return TRUE;
            }
            else
            {
                g_snprintf (string, 8, "#%02x%02x%02x",
                            color->red >> 8,
                            color->green >> 8,
                            color->blue >> 8);
                g_value_set_string (dest, string);
                return TRUE;
            }
        }

        if (G_TYPE_IS_ENUM (src_type))
        {
            gpointer klass;
            GEnumClass *enum_class;
            GEnumValue *enum_value;

            klass = g_type_class_ref (src_type);
            g_return_val_if_fail (G_IS_ENUM_CLASS (klass), FALSE);
            enum_class = G_ENUM_CLASS (klass);

            enum_value = g_enum_get_value (enum_class,
                                           g_value_get_enum (src));

            if (!enum_value)
            {
                char *string = g_strdup_printf ("%d", g_value_get_enum (src));
                g_value_take_string (dest, string);
                g_type_class_unref (klass);
                g_return_val_if_reached (TRUE);
            }

            g_value_set_static_string (dest, enum_value->value_nick);
            g_type_class_unref (klass);
            return TRUE;
        }

        g_return_val_if_reached (FALSE);
    }

    if (src_type == G_TYPE_STRING)
    {
        const char *string = g_value_get_string (src);

        if (dest_type == G_TYPE_BOOLEAN)
        {
            if (!string)
                g_value_set_boolean (dest, FALSE);
            else
                g_value_set_boolean (dest,
                                     ! g_ascii_strcasecmp (string, "1") ||
                                             ! g_ascii_strcasecmp (string, "yes") ||
                                             ! g_ascii_strcasecmp (string, "true"));
            return TRUE;
        }

        if (dest_type == G_TYPE_DOUBLE)
        {
            if (!string)
                g_value_set_double (dest, 0);
            else
                g_value_set_double (dest, g_ascii_strtod (string, NULL));
            return TRUE;
        }

        if (dest_type == G_TYPE_INT)
        {
            if (!string)
                g_value_set_int (dest, 0);
            else
                g_value_set_int (dest, g_ascii_strtod (string, NULL));
            return TRUE;
        }

        if (dest_type == GDK_TYPE_COLOR)
        {
            GdkColor color;

            if (!string)
            {
                g_value_set_boxed (dest, NULL);
                return TRUE;
            }

            g_return_val_if_fail (gdk_color_parse (string, &color),
                                  FALSE);

            g_value_set_boxed (dest, &color);
            return TRUE;
        }

        if (G_TYPE_IS_ENUM (dest_type))
        {
            gpointer klass;
            GEnumClass *enum_class;
            GEnumValue *enum_value;
            int ival;

            if (!string || !string[0])
            {
                g_value_set_enum (dest, 0);
                g_return_val_if_reached (TRUE);
            }

            klass = g_type_class_ref (dest_type);
            g_return_val_if_fail (G_IS_ENUM_CLASS (klass), FALSE);
            enum_class = G_ENUM_CLASS (klass);

            enum_value = g_enum_get_value_by_name (enum_class, string);
            if (!enum_value)
                enum_value = g_enum_get_value_by_nick (enum_class, string);

            if (enum_value)
            {
                ival = enum_value->value;
            }
            else
            {
                ival = g_ascii_strtod (string, NULL);

                if (ival < enum_class->minimum || ival > enum_class->maximum)
                {
                    g_value_set_enum (dest, ival);
                    g_type_class_unref (klass);
                    g_return_val_if_reached (TRUE);
                }
            }

            g_value_set_enum (dest, ival);
            g_type_class_unref (klass);
            return TRUE;
        }

        g_return_val_if_reached (FALSE);
    }

    if (G_TYPE_IS_ENUM (src_type) && dest_type == G_TYPE_INT)
    {
        g_value_set_int (dest, g_value_get_enum (src));
        return TRUE;
    }

    if (G_TYPE_IS_ENUM (dest_type) && src_type == G_TYPE_INT)
    {
        g_value_set_enum (dest, g_value_get_int (src));
        return TRUE;
    }

    if (src_type == G_TYPE_DOUBLE && dest_type == G_TYPE_INT)
    {
        g_value_set_int (dest, g_value_get_double (src));
        return TRUE;
    }

    if (dest_type == G_TYPE_DOUBLE && src_type == G_TYPE_INT)
    {
        g_value_set_double (dest, g_value_get_int (src));
        return TRUE;
    }

    g_return_val_if_reached (FALSE);
}


gboolean
moo_value_equal (const GValue   *a,
                 const GValue   *b)
{
    GType type;

    g_return_val_if_fail (G_IS_VALUE (a) && G_IS_VALUE (b), a == b);
    g_return_val_if_fail (G_VALUE_TYPE (a) == G_VALUE_TYPE (b), a == b);
    g_return_val_if_fail (G_VALUE_TYPE (a) == G_VALUE_TYPE (b), a == b);

    type = G_VALUE_TYPE (a);

    if (type == G_TYPE_BOOLEAN)
    {
        gboolean ba = g_value_get_boolean (a);
        gboolean bb = g_value_get_boolean (b);
        return (ba && bb) || (!ba && !bb);
    }

    if (type == G_TYPE_INT)
        return g_value_get_int (a) == g_value_get_int (b);

    if (type == G_TYPE_DOUBLE)
        return g_value_get_double (a) == g_value_get_double (b);

    if (type == G_TYPE_STRING)
    {
        const char *sa, *sb;

        sa = g_value_get_string (a);
        sb = g_value_get_string (b);

        if (!sa || !sb)
            return sa == sb;
        else
            return !strcmp (sa, sb);
    }

    if (type == GDK_TYPE_COLOR)
    {
        const GdkColor *ca, *cb;

        ca = g_value_get_boxed (a);
        cb = g_value_get_boxed (b);

        if (!ca || !cb)
            return ca == cb;
        else
            return ca->red == cb->red &&
                    ca->green == cb->green &&
                    ca->blue == cb->blue;
    }

    if (G_TYPE_IS_ENUM (type))
        return g_value_get_enum (a) == g_value_get_enum (b);

    if (G_TYPE_IS_FLAGS (type))
        return g_value_get_flags (a) == g_value_get_flags (b);

    g_return_val_if_reached (a == b);
}


gboolean
moo_value_type_supported (GType type)
{
    return type == G_TYPE_BOOLEAN ||
            type == G_TYPE_INT ||
            type == G_TYPE_DOUBLE ||
            type == G_TYPE_STRING ||
            type == GDK_TYPE_COLOR ||
            G_TYPE_IS_ENUM (type);
}


gboolean
moo_value_convert_to_bool (const GValue *val)
{
    GValue result;
    result.g_type = 0;
    g_value_init (&result, G_TYPE_BOOLEAN);
    moo_value_convert (val, &result);
    return g_value_get_boolean (&result);
}


int
moo_value_convert_to_int (const GValue *val)
{
    GValue result;
    result.g_type = 0;
    g_value_init (&result, G_TYPE_INT);
    moo_value_convert (val, &result);
    return g_value_get_int (&result);
}


int
moo_value_convert_to_enum (const GValue   *val,
                           GType           enum_type)
{
    GValue result;
    result.g_type = 0;
    g_value_init (&result, enum_type);
    moo_value_convert (val, &result);
    return g_value_get_enum (&result);
}


double
moo_value_convert_to_double (const GValue *val)
{
    GValue result;
    result.g_type = 0;
    g_value_init (&result, G_TYPE_DOUBLE);
    moo_value_convert (val, &result);
    return g_value_get_double (&result);
}


const GdkColor*
moo_value_convert_to_color (const GValue *val)
{
    static GValue result;

    if (G_IS_VALUE (&result))
        g_value_unset (&result);

    g_value_init (&result, GDK_TYPE_COLOR);

    if (!moo_value_convert (val, &result))
        return NULL;
    else
        return g_value_get_boxed (&result);
}


const char*
moo_value_convert_to_string (const GValue *val)
{
    static GValue result;

    if (G_IS_VALUE (&result))
        g_value_unset (&result);

    g_value_init (&result, G_TYPE_STRING);

    if (!moo_value_convert (val, &result))
        return NULL;
    else
        return g_value_get_string (&result);
}


gboolean
moo_value_change_type (GValue         *val,
                       GType           new_type)
{
    GValue tmp;
    gboolean result;

    g_return_val_if_fail (G_IS_VALUE (val), FALSE);
    g_return_val_if_fail (moo_value_type_supported (new_type), FALSE);

    tmp.g_type = 0;
    g_value_init (&tmp, new_type);
    result = moo_value_convert (val, &tmp);

    if (result)
    {
        g_value_unset (val);
        memcpy (val, &tmp, sizeof (GValue));
    }

    return result;
}


/*****************************************************************************/
/* GParameter array manipulation
 */

static void         copy_param          (GParameter *dest,
                                         GParameter *src);

GParameter*
moo_param_array_collect (GType       type,
                         guint      *len,
                         const char *first_prop_name,
                         ...)
{
    GParameter *array;
    va_list var_args;

    va_start (var_args, first_prop_name);
    array = moo_param_array_collect_valist (type, len,
                                            first_prop_name,
                                            var_args);
    va_end (var_args);

    return array;
}


GParameter*
moo_param_array_add (GType       type,
                     GParameter *src,
                     guint       len,
                     guint      *new_len,
                     const char *first_prop_name,
                     ...)
{
    GParameter *copy;
    va_list var_args;

    va_start (var_args, first_prop_name);
    copy = moo_param_array_add_valist (type, src, len, new_len,
                                       first_prop_name,
                                       var_args);
    va_end (var_args);

    return copy;
}


GParameter*
moo_param_array_add_type (GParameter *src,
                          guint       len,
                          guint      *new_len,
                          const char *first_prop_name,
                          ...)
{
    GParameter *copy;
    va_list var_args;

    va_start (var_args, first_prop_name);
    copy = moo_param_array_add_type_valist (src, len, new_len,
                                            first_prop_name,
                                            var_args);
    va_end (var_args);

    return copy;
}


GParameter*
moo_param_array_add_valist (GType       type,
                            GParameter *src,
                            guint       len,
                            guint      *new_len,
                            const char *first_prop_name,
                            va_list     var_args)
{
    const char *name;
    GArray *copy;
    guint i;
    GObjectClass *klass = g_type_class_ref (type); /* TODO: unref */

    g_return_val_if_fail (klass != NULL, NULL);
    g_return_val_if_fail (src != NULL, NULL);

    copy = g_array_new (FALSE, TRUE, sizeof (GParameter));

    for (i = 0; i < len; ++i)
    {
        GParameter param = {0};
        copy_param (&param, &src[i]);
        g_array_append_val (copy, param);
    }

    name = first_prop_name;
    while (name)
    {
        char *error = NULL;
        GParameter param = {0};
        GParamSpec *pspec = g_object_class_find_property (klass, name);

        if (!pspec) {
            g_warning ("%s: class '%s' doesn't have property '%s'",
                       G_STRLOC, g_type_name (type), name);
            moo_param_array_free ((GParameter*)copy->data, copy->len);
            g_array_free (copy, FALSE);
            return NULL;
        }

        param.name = g_strdup (name);
        g_value_init (&param.value, G_PARAM_SPEC_VALUE_TYPE (pspec));
        G_VALUE_COLLECT (&param.value, var_args, 0, &error);

        if (error) {
            g_critical ("%s: %s", G_STRLOC, error);
            g_free (error);
            g_value_unset (&param.value);
            g_free ((char*)param.name);
            moo_param_array_free ((GParameter*)copy->data, copy->len);
            g_array_free (copy, FALSE);
            return NULL;
        }

        g_array_append_val (copy, param);

        name = va_arg (var_args, char*);
    }

    *new_len = copy->len;
    return (GParameter*) g_array_free (copy, FALSE);
}


GParameter*
moo_param_array_add_type_valist (GParameter *src,
                                 guint       len,
                                 guint      *new_len,
                                 const char *first_prop_name,
                                 va_list     var_args)
{
    const char *name;
    GArray *copy;
    guint i;

    g_return_val_if_fail (src != NULL, NULL);

    copy = g_array_new (FALSE, TRUE, sizeof (GParameter));
    for (i = 0; i < len; ++i) {
        GParameter param = {0};
        copy_param (&param, &src[i]);
        g_array_append_val (copy, param);
    }

    name = first_prop_name;
    while (name)
    {
        char *error = NULL;
        GParameter param = {0};
        GType type;

        type = va_arg (var_args, GType);

        if (G_TYPE_FUNDAMENTAL (type) == G_TYPE_INVALID)
        {
            g_warning ("%s: invalid type id passed", G_STRLOC);
            moo_param_array_free ((GParameter*)copy->data, copy->len);
            g_array_free (copy, FALSE);
            return NULL;
        }

        param.name = g_strdup (name);
        g_value_init (&param.value, type);
        G_VALUE_COLLECT (&param.value, var_args, 0, &error);

        if (error)
        {
            g_critical ("%s: %s", G_STRLOC, error);
            g_free (error);
            g_value_unset (&param.value);
            g_free ((char*)param.name);
            moo_param_array_free ((GParameter*)copy->data, copy->len);
            g_array_free (copy, FALSE);
            return NULL;
        }

        g_array_append_val (copy, param);

        name = va_arg (var_args, char*);
    }

    *new_len = copy->len;
    return (GParameter*) g_array_free (copy, FALSE);
}


GParameter*
moo_param_array_collect_valist (GType       type,
                                guint      *len,
                                const char *first_prop_name,
                                va_list     var_args)
{
    GObjectClass *klass;
    GArray *params = NULL;
    const char *prop_name;

    g_return_val_if_fail (G_TYPE_IS_OBJECT (type), NULL);
    g_return_val_if_fail (first_prop_name != NULL, NULL);

    klass = g_type_class_ref (type);

    params = g_array_new (FALSE, TRUE, sizeof (GParameter));

    prop_name = first_prop_name;
    while (prop_name)
    {
        char *error = NULL;
        GParameter param;
        GParamSpec *pspec = g_object_class_find_property (klass, prop_name);

        if (!pspec)
        {
            g_warning ("%s: class '%s' doesn't have property '%s'",
                       G_STRLOC, g_type_name (type), prop_name);
            moo_param_array_free ((GParameter*)params->data, params->len);
            g_array_free (params, FALSE);
            g_type_class_unref (klass);
            return NULL;
        }

        param.name = g_strdup (prop_name);
        param.value.g_type = 0;
        g_value_init (&param.value, G_PARAM_SPEC_VALUE_TYPE (pspec));
        G_VALUE_COLLECT (&param.value, var_args, 0, &error);

        if (error)
        {
            g_critical ("%s: %s", G_STRLOC, error);
            g_free (error);
            g_value_unset (&param.value);
            g_free ((char*)param.name);
            moo_param_array_free ((GParameter*)params->data, params->len);
            g_array_free (params, FALSE);
            g_type_class_unref (klass);
            return NULL;
        }

        g_array_append_val (params, param);

        prop_name = va_arg (var_args, char*);
    }

    g_type_class_unref (klass);

    *len = params->len;
    return (GParameter*) g_array_free (params, FALSE);
}


void
moo_param_array_free (GParameter *array,
                      guint       len)
{
    guint i;

    for (i = 0; i < len; ++i)
    {
        g_value_unset (&array[i].value);
        g_free ((char*)array[i].name);
    }

    g_free (array);
}


static void
copy_param (GParameter *dest,
            GParameter *src)
{
    dest->name = g_strdup (src->name);
    g_value_init (&dest->value, G_VALUE_TYPE (&src->value));
    g_value_copy (&src->value, &dest->value);
}


/*****************************************************************************/
/* Signal that does not require class method
 */

static void
void_marshal (G_GNUC_UNUSED GClosure *closure,
              G_GNUC_UNUSED GValue *return_value,
              G_GNUC_UNUSED guint n_param_values,
              G_GNUC_UNUSED const GValue *param_values,
              G_GNUC_UNUSED gpointer invocation_hint,
              G_GNUC_UNUSED gpointer marshal_data)
{
}


static GClosure*
void_closure_new (void)
{
    GClosure *closure = g_closure_new_simple (sizeof (GClosure), NULL);
    g_closure_set_marshal (closure, void_marshal);
    return closure;
}


guint
moo_signal_new_cb (const gchar        *signal_name,
                   GType               itype,
                   GSignalFlags        signal_flags,
                   GCallback           handler,
                   GSignalAccumulator  accumulator,
                   gpointer            accu_data,
                   GSignalCMarshaller  c_marshaller,
                   GType               return_type,
                   guint               n_params,
                   ...)
{
    va_list args;
    guint signal_id;

    g_return_val_if_fail (signal_name != NULL, 0);

    va_start (args, n_params);

    if (handler)
        signal_id = g_signal_new_valist (signal_name, itype, signal_flags,
                                         g_cclosure_new (handler, NULL, NULL),
                                         accumulator, accu_data, c_marshaller,
                                         return_type, n_params, args);
    else
        signal_id = g_signal_new_valist (signal_name, itype, signal_flags,
                                         void_closure_new (),
                                         accumulator, accu_data, c_marshaller,
                                         return_type, n_params, args);

    va_end (args);

    return signal_id;
}


/*****************************************************************************/
/* MooObjectFactory
 */

static void moo_object_factory_finalize     (GObject        *object);

static void moo_object_factory_set_property (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec);
static void moo_object_factory_get_property (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec);


enum {
    PROP_0,
    PROP_OBJECT_TYPE,
    PROP_FACTORY_FUNC,
    PROP_FACTORY_FUNC_DATA
};


/* MOO_TYPE_OBJECT_FACTORY */
G_DEFINE_TYPE (MooObjectFactory, moo_object_factory, G_TYPE_OBJECT)


static void
moo_object_factory_class_init (MooObjectFactoryClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = moo_object_factory_set_property;
    gobject_class->get_property = moo_object_factory_get_property;
    gobject_class->finalize = moo_object_factory_finalize;

    g_object_class_install_property (gobject_class,
                                     PROP_OBJECT_TYPE,
                                     g_param_spec_boxed ("object_type",
                                             "object_type",
                                             "object_type",
                                             MOO_TYPE_GTYPE,
                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE));
    g_object_class_install_property (gobject_class,
                                     PROP_FACTORY_FUNC,
                                     g_param_spec_pointer ("factory_func",
                                             "factory_func",
                                             "factory_func",
                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE));
    g_object_class_install_property (gobject_class,
                                     PROP_FACTORY_FUNC_DATA,
                                     g_param_spec_pointer ("factory_func_data",
                                             "factory_func_data",
                                             "factory_func_data",
                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE));
}


static void
moo_object_factory_init (MooObjectFactory *factory)
{
    factory->factory_func = NULL;
    factory->factory_func_data = NULL;
    factory->object_type = 0;
    factory->n_params = 0;
    factory->params = NULL;
}


static void
moo_object_factory_get_property (GObject        *object,
                                 guint           prop_id,
                                 GValue         *value,
                                 GParamSpec     *pspec)
{
    MooObjectFactory *factory = MOO_OBJECT_FACTORY (object);

    switch (prop_id)
    {
        case PROP_FACTORY_FUNC:
            g_value_set_pointer (value, (gpointer)factory->factory_func);
            break;

        case PROP_FACTORY_FUNC_DATA:
            g_value_set_pointer (value, factory->factory_func_data);
            break;

        case PROP_OBJECT_TYPE:
            moo_value_set_gtype (value, factory->object_type);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
moo_object_factory_set_property (GObject        *object,
                                 guint           prop_id,
                                 const GValue   *value,
                                 GParamSpec     *pspec)
{
    MooObjectFactory *factory = MOO_OBJECT_FACTORY (object);

    switch (prop_id)
    {
        case PROP_FACTORY_FUNC:
            factory->factory_func = (MooObjectFactoryFunc)g_value_get_pointer (value);
            break;

        case PROP_FACTORY_FUNC_DATA:
            factory->factory_func_data = g_value_get_pointer (value);
            break;

        case PROP_OBJECT_TYPE:
            factory->object_type = moo_value_get_gtype (value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
moo_object_factory_finalize (GObject *object)
{
    MooObjectFactory *factory = MOO_OBJECT_FACTORY (object);
    moo_param_array_free (factory->params, factory->n_params);
    G_OBJECT_CLASS (moo_object_factory_parent_class)->finalize (object);
}


gpointer
moo_object_factory_create_object (MooObjectFactory   *factory,
                                  gpointer            data,
                                  const char         *prop_name,
                                  ...)
{
    g_return_val_if_fail (MOO_IS_OBJECT_FACTORY (factory), NULL);

    if (factory->factory_func)
    {
        if (factory->factory_func_data)
            data = factory->factory_func_data;
        return factory->factory_func (data, factory);
    }

    if (!prop_name)
    {
        return g_object_newv (factory->object_type,
                              factory->n_params,
                              factory->params);
    }
    else
    {
        GObject *object;
        GParameter *params;
        guint n_params;
        va_list var_args;

        va_start (var_args, prop_name);
        params = moo_param_array_add_valist (factory->object_type,
                                             factory->params,
                                             factory->n_params,
                                             &n_params,
                                             prop_name,
                                             var_args);
        va_end (var_args);

        g_return_val_if_fail (params != NULL, NULL);
        object = g_object_newv (factory->object_type,
                                n_params,
                                params);
        moo_param_array_free (params, n_params);
        return object;
    }
}


MooObjectFactory*
moo_object_factory_new_func (MooObjectFactoryFunc factory_func,
                             gpointer            data)
{
    return g_object_new (MOO_TYPE_OBJECT_FACTORY,
                         "factory_func", factory_func,
                         "factory_func_data", data,
                         NULL);
}


MooObjectFactory*
moo_object_factory_new (GType               object_type,
                        const char         *first_prop_name,
                        ...)
{
    MooObjectFactory *factory;
    va_list var_args;

    g_return_val_if_fail (G_TYPE_IS_OBJECT (object_type), NULL);

    va_start (var_args, first_prop_name);
    factory = moo_object_factory_new_valist (object_type, first_prop_name, var_args);
    va_end (var_args);

    return factory;
}


MooObjectFactory*
moo_object_factory_new_valist (GType               object_type,
                               const char         *first_prop_name,
                               va_list             var_args)
{
    GParameter *params = NULL;
    guint n_params = 0;
    MooObjectFactory *factory;

    g_return_val_if_fail (G_TYPE_IS_OBJECT (object_type), NULL);

    if (!first_prop_name)
        return g_object_new (MOO_TYPE_OBJECT_FACTORY,
                             "object_type", object_type,
                             NULL);

    params = moo_param_array_collect_valist (object_type,
                                             &n_params,
                                             first_prop_name,
                                             var_args);
    g_return_val_if_fail (params != NULL, NULL);

    factory = g_object_new (MOO_TYPE_OBJECT_FACTORY,
                            "object_type", object_type,
                            NULL);
    factory->n_params = n_params;
    factory->params = params;

    return factory;
}


MooObjectFactory*
moo_object_factory_new_a (GType               object_type,
                          GParameter         *params,
                          guint               n_params)
{
    MooObjectFactory *factory;

    g_return_val_if_fail (G_TYPE_IS_OBJECT (object_type), NULL);
    g_return_val_if_fail (params != NULL, NULL);

    factory = g_object_new (MOO_TYPE_OBJECT_FACTORY,
                            "object_type", object_type,
                            NULL);

    factory->params = params;
    factory->n_params = n_params;

    return factory;
}
