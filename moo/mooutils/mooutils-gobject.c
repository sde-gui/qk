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
#include "mooutils/mooclosure.h"
#include <gobject/gvaluecollector.h>
#include <string.h>


/*****************************************************************************/
/* GType type
 */

#define MOO_GTYPE_PEEK(val_) (val_)->data[0].v_pointer

static void
moo_gtype_value_init (G_GNUC_UNUSED GValue *value)
{
}


static void
moo_gtype_value_copy (const GValue   *src,
                      GValue         *dest)
{
    MOO_GTYPE_PEEK(dest) = MOO_GTYPE_PEEK(src);
}


static char*
moo_gtype_collect_value (GValue         *value,
                         G_GNUC_UNUSED guint n_collect_values,
                         GTypeCValue    *collect_values,
                         G_GNUC_UNUSED guint collect_flags)
{
    MOO_GTYPE_PEEK(value) = collect_values->v_pointer;
    return NULL;
}


static char*
moo_gtype_lcopy_value (const GValue   *value,
                       G_GNUC_UNUSED guint n_collect_values,
                       GTypeCValue    *collect_values,
                       G_GNUC_UNUSED guint collect_flags)
{
    GType *ptr = collect_values->v_pointer;
    *ptr = moo_value_get_gtype (value);
    return NULL;
}


GType
moo_gtype_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        static GTypeValueTable val_table = {
            moo_gtype_value_init,
            NULL,
            moo_gtype_value_copy,
            NULL,
            (char*) "p",
            moo_gtype_collect_value,
            (char*) "p",
            moo_gtype_lcopy_value
        };

        static GTypeInfo info = {
            /* interface types, classed types, instantiated types */
            0, /*class_size*/
            NULL, /*base_init*/
            NULL, /*base_finalize*/
            NULL,/*class_init*/
            NULL,/*class_finalize*/
            NULL,/*class_data*/
            0,/*instance_size*/
            0,/*n_preallocs*/
            NULL,/*instance_init*/
            /* value handling */
            &val_table
        };

        static GTypeFundamentalInfo finfo = { 0 };

        type = g_type_register_fundamental (g_type_fundamental_next (),
                                            "MooGType",
                                            &info, &finfo, 0);
    }

    return type;
}


void
moo_value_set_gtype (GValue     *value,
                     GType       v_gtype)
{
    MOO_GTYPE_PEEK(value) = (gpointer) v_gtype;
}


GType
moo_value_get_gtype (const GValue *value)
{
    return (GType) MOO_GTYPE_PEEK(value);
}


static void
param_gtype_set_default (GParamSpec *pspec,
                         GValue     *value)
{
    MooParamSpecGType *mspec = MOO_PARAM_SPEC_GTYPE (pspec);
    moo_value_set_gtype (value, mspec->base);
}


static gboolean
param_gtype_validate (GParamSpec   *pspec,
                      GValue       *value)
{
    MooParamSpecGType *mspec = MOO_PARAM_SPEC_GTYPE (pspec);
    GType t = moo_value_get_gtype (value);
    gboolean changed = FALSE;

    if (G_TYPE_IS_DERIVABLE (mspec->base))
    {
        if (!g_type_is_a (t, mspec->base))
        {
            moo_value_set_gtype (value, mspec->base);
            changed = TRUE;
        }
    }
    else if (!g_type_name (t))
    {
        moo_value_set_gtype (value, mspec->base);
        changed = TRUE;
    }

    return changed;
}


static int
param_gtype_cmp (G_GNUC_UNUSED GParamSpec *pspec,
                 const GValue *value1,
                 const GValue *value2)
{
    GType t1 = moo_value_get_gtype (value1);
    GType t2 = moo_value_get_gtype (value2);
    return t1 == t2 ? 0 : (t1 < t2 ? -1 : 1);
}


GType
moo_param_gtype_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        GParamSpecTypeInfo info = {
            sizeof (MooParamSpecGType), /* instance_size */
            16,                         /* n_preallocs */
            NULL,                       /* instance_init */
            MOO_TYPE_GTYPE,             /* value_type */
            NULL,                       /* finalize */
            param_gtype_set_default,    /* value_set_default */
            param_gtype_validate,       /* value_validate */
            param_gtype_cmp,            /* values_cmp */
        };

        type = g_param_type_register_static ("MooParamGType", &info);
    }

    return type;
}


GParamSpec*
moo_param_spec_gtype (const char     *name,
                      const char     *nick,
                      const char     *blurb,
                      GType           base,
                      GParamFlags     flags)
{
    MooParamSpecGType *pspec;

    g_return_val_if_fail (g_type_name (base) != NULL, NULL);

    pspec = g_param_spec_internal (MOO_TYPE_PARAM_GTYPE,
                                   name, nick, blurb, flags);
    pspec->base = base;

    return G_PARAM_SPEC (pspec);
}


GType       g_param_type_register_static    (const gchar *name,
                                             const GParamSpecTypeInfo *pspec_info);


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
            if (!string || !string[0])
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
            if (!string || !string[0])
                g_value_set_double (dest, 0);
            else
                g_value_set_double (dest, g_ascii_strtod (string, NULL));
            return TRUE;
        }

        if (dest_type == G_TYPE_INT)
        {
            if (!string || !string[0])
                g_value_set_int (dest, 0);
            else
                g_value_set_int (dest, g_ascii_strtod (string, NULL));
            return TRUE;
        }

        if (dest_type == GDK_TYPE_COLOR)
        {
            GdkColor color;

            if (!string || !string[0])
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
                return TRUE;
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


int
moo_convert_string_to_int (const char     *string,
                           int             default_val)
{
    int int_val;

    if (string)
    {
        GValue str_val;
        str_val.g_type = 0;
        g_value_init (&str_val, G_TYPE_STRING);
        g_value_set_static_string (&str_val, string);
        int_val = moo_value_convert_to_int (&str_val);
        g_value_unset (&str_val);
    }
    else
    {
        int_val = default_val;
    }

    return int_val;
}


gboolean
moo_convert_string_to_bool (const char     *string,
                            gboolean        default_val)
{
    gboolean bool_val;

    if (string)
    {
        GValue str_val;
        str_val.g_type = 0;
        g_value_init (&str_val, G_TYPE_STRING);
        g_value_set_static_string (&str_val, string);
        bool_val = moo_value_convert_to_bool (&str_val);
        g_value_unset (&str_val);
    }
    else
    {
        bool_val = default_val;
    }

    return bool_val;
}


const char*
moo_convert_bool_to_string (gboolean        value)
{
    GValue bool_val;

    bool_val.g_type = 0;
    g_value_init (&bool_val, G_TYPE_BOOLEAN);
    g_value_set_boolean (&bool_val, value);

    return moo_value_convert_to_string (&bool_val);
}


const char*
moo_convert_int_to_string (int             value)
{
    GValue int_val;

    int_val.g_type = 0;
    g_value_init (&int_val, G_TYPE_INT);
    g_value_set_int (&int_val, value);

    return moo_value_convert_to_string (&int_val);
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
        GParameter param = {NULL, {0, {{0}, {0}}}};
        copy_param (&param, &src[i]);
        g_array_append_val (copy, param);
    }

    name = first_prop_name;
    while (name)
    {
        char *error = NULL;
        GParameter param = {NULL, {0, {{0}, {0}}}};
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
        GParameter param = {NULL, {0, {{0}, {0}}}};
        copy_param (&param, &src[i]);
        g_array_append_val (copy, param);
    }

    name = first_prop_name;
    while (name)
    {
        char *error = NULL;
        GParameter param = {NULL, {0, {{0}, {0}}}};
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
                                     moo_param_spec_gtype ("object_type",
                                             "object_type",
                                             "object_type",
                                             G_TYPE_OBJECT,
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


/****************************************************************************/
/* Property watch
 */

typedef struct _Watch Watch;
typedef void (*WatchSourceNotify)   (Watch *watch);
typedef void (*WatchTargetNotify)   (Watch *watch);
typedef void (*WatchDestroy)        (Watch *watch);

typedef struct {
    WatchSourceNotify source_notify;
    WatchTargetNotify target_notify;
    WatchDestroy destroy;
} WatchClass;

#define watch_new(Type_,klass_,src_,tgt_,notify_,data_) \
    ((Type_*) watch_alloc (sizeof (Type_), klass_, src_, tgt_, notify_, data_))

struct _Watch {
    WatchClass *klass;
    MooObjectPtr *source;
    MooObjectPtr *target;
    GDestroyNotify notify;
    gpointer notify_data;
    guint id;
};

static GHashTable *watches = NULL;
static guint watch_last_id = 0;


static void
watch_destroy (Watch *w)
{
    if (w)
    {
        if (w->klass->destroy)
            w->klass->destroy (w);
        if (w->notify)
            w->notify (w->notify_data);
        moo_object_ptr_free (w->source);
        moo_object_ptr_free (w->target);
        g_free (w);
    }
}


static void
watch_source_died (Watch *w)
{
    if (w->klass->source_notify)
        w->klass->source_notify (w);
    moo_object_ptr_free (w->source);
    w->source = NULL;
    g_hash_table_remove (watches, GUINT_TO_POINTER (w->id));
}

static void
watch_target_died (Watch *w)
{
    if (w->klass->target_notify)
        w->klass->target_notify (w);
    moo_object_ptr_free (w->target);
    w->target = NULL;
    g_hash_table_remove (watches, GUINT_TO_POINTER (w->id));
}


static Watch *
watch_alloc (gsize          size,
             WatchClass    *klass,
             GObject       *source,
             GObject       *target,
             GDestroyNotify notify,
             gpointer       notify_data)
{
    Watch *w;

    g_return_val_if_fail (size >= sizeof (Watch), NULL);
    g_return_val_if_fail (G_IS_OBJECT (source), NULL);
    g_return_val_if_fail (G_IS_OBJECT (target), NULL);

    w = g_malloc0 (size);
    w->source = moo_object_ptr_new (source, (GWeakNotify) watch_source_died, w);
    w->target = moo_object_ptr_new (target, (GWeakNotify) watch_target_died, w);
    w->klass = klass;
    w->notify = notify;
    w->notify_data = notify_data;
    w->id = ++watch_last_id;

    if (!watches)
        watches = g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                         NULL, (GDestroyNotify) watch_destroy);
    g_hash_table_insert (watches, GUINT_TO_POINTER (w->id), w);

    return w;
}


typedef struct {
    Watch parent;
    GParamSpec *source_pspec;
    GParamSpec *target_pspec;
    MooTransformPropFunc transform;
    gpointer transform_data;
} PropWatch;


static void prop_watch_check        (PropWatch  *watch);
static void prop_watch_destroy      (Watch      *watch);

static WatchClass PropWatchClass = {NULL, NULL, prop_watch_destroy};


static PropWatch*
prop_watch_new (GObject            *target,
                const char         *target_prop,
                GObject            *source,
                const char         *source_prop,
                const char         *signal,
                MooTransformPropFunc transform,
                gpointer            transform_data,
                GDestroyNotify      destroy_notify,
                gpointer            destroy_notify_data)
{
    PropWatch *watch;
    GObjectClass *target_class, *source_class;
    char *signal_name = NULL;
    GParamSpec *source_pspec;
    GParamSpec *target_pspec;

    g_return_val_if_fail (G_IS_OBJECT (target), NULL);
    g_return_val_if_fail (G_IS_OBJECT (source), NULL);
    g_return_val_if_fail (target_prop != NULL, NULL);
    g_return_val_if_fail (source_prop != NULL, NULL);
    g_return_val_if_fail (transform != NULL, NULL);

    target_class = g_type_class_peek (G_OBJECT_TYPE (target));
    source_class = g_type_class_peek (G_OBJECT_TYPE (source));

    source_pspec = g_object_class_find_property (source_class, source_prop);
    target_pspec = g_object_class_find_property (target_class, target_prop);

    if (!source_pspec || !target_pspec)
    {
        if (!source_pspec)
            g_warning ("%s: no property '%s' in class '%s'",
                       G_STRLOC, source_prop, g_type_name (G_OBJECT_TYPE (source)));
        if (!target_pspec)
            g_warning ("%s: no property '%s' in class '%s'",
                       G_STRLOC, target_prop, g_type_name (G_OBJECT_TYPE (target)));
        return NULL;
    }

    watch = watch_new (PropWatch, &PropWatchClass, source, target,
                       destroy_notify, destroy_notify_data);

    watch->source_pspec = source_pspec;
    watch->target_pspec = target_pspec;
    watch->transform = transform;
    watch->transform_data = transform_data;

    if (!signal)
    {
        signal_name = g_strdup_printf ("notify::%s", source_prop);
        signal = signal_name;
    }

    g_signal_connect_swapped (source, signal,
                              G_CALLBACK (prop_watch_check),
                              watch);

    g_free (signal_name);
    return watch;
}


static void
prop_watch_destroy (Watch *watch)
{
    if (MOO_OBJECT_PTR_GET (watch->source))
        g_signal_handlers_disconnect_by_func (MOO_OBJECT_PTR_GET (watch->source),
                                              (gpointer) prop_watch_check,
                                              watch);
}


guint
moo_add_property_watch (gpointer            target,
                        const char         *target_prop,
                        gpointer            source,
                        const char         *source_prop,
                        MooTransformPropFunc transform,
                        gpointer            transform_data,
                        GDestroyNotify      destroy_notify)
{
    PropWatch *watch;

    g_return_val_if_fail (G_IS_OBJECT (target), 0);
    g_return_val_if_fail (G_IS_OBJECT (source), 0);
    g_return_val_if_fail (target_prop != NULL, 0);
    g_return_val_if_fail (source_prop != NULL, 0);
    g_return_val_if_fail (transform != NULL, 0);

    watch = prop_watch_new (target, target_prop, source, source_prop,
                            NULL, transform, transform_data,
                            destroy_notify, transform_data);

    if (!watch)
        return 0;

    prop_watch_check (watch);
    return watch->parent.id;
}


static void
prop_watch_check (PropWatch *watch)
{
    GValue source_val, target_val, old_target_val;
    GObject *source, *target;

    source = MOO_OBJECT_PTR_GET (watch->parent.source);
    target = MOO_OBJECT_PTR_GET (watch->parent.target);
    g_return_if_fail (source && target);

    source_val.g_type = 0;
    target_val.g_type = 0;
    old_target_val.g_type = 0;

    g_value_init (&source_val, watch->source_pspec->value_type);
    g_value_init (&target_val, watch->target_pspec->value_type);
    g_value_init (&old_target_val, watch->target_pspec->value_type);

    g_object_ref (source);
    g_object_ref (target);

    g_object_get_property (source,
                           watch->source_pspec->name,
                           &source_val);
    g_object_get_property (target,
                           watch->target_pspec->name,
                           &old_target_val);

    watch->transform (&target_val, &source_val, watch->transform_data);

    if (g_param_values_cmp (watch->target_pspec, &target_val, &old_target_val))
        g_object_set_property (target,
                               watch->target_pspec->name,
                               &target_val);

    g_object_unref (source);
    g_object_unref (target);
    g_value_unset (&source_val);
    g_value_unset (&target_val);
    g_value_unset (&old_target_val);
}


void
moo_copy_boolean (GValue             *target,
                  const GValue       *source,
                  G_GNUC_UNUSED gpointer dummy)
{
    g_value_set_boolean (target, g_value_get_boolean (source) ? TRUE : FALSE);
}


void
moo_invert_boolean (GValue             *target,
                    const GValue       *source,
                    G_GNUC_UNUSED gpointer dummy)
{
    g_value_set_boolean (target, !g_value_get_boolean (source));
}


guint
moo_bind_bool_property (gpointer            target,
                        const char         *target_prop,
                        gpointer            source,
                        const char         *source_prop,
                        gboolean            invert)
{
    if (invert)
        return moo_add_property_watch (target, target_prop, source, source_prop,
                                       moo_invert_boolean, NULL, NULL);
    else
        return moo_add_property_watch (target, target_prop, source, source_prop,
                                       moo_copy_boolean, NULL, NULL);
}


gboolean
moo_sync_bool_property (gpointer            slave,
                        const char         *slave_prop,
                        gpointer            master,
                        const char         *master_prop,
                        gboolean            invert)
{
    guint id1 = 0, id2 = 0;

    id1 = moo_bind_bool_property (slave, slave_prop, master, master_prop, invert);

    if (id1)
    {
        id2 = moo_bind_bool_property (master, master_prop, slave, slave_prop, invert);

        if (!id2)
            moo_watch_remove (id1);
    }

    return id1 && id2;
}


void
moo_bind_sensitive (GtkWidget          *btn,
                    GtkWidget         **dependent,
                    int                 num_dependent,
                    gboolean            invert)
{
    int i;
    for (i = 0; i < num_dependent; ++i)
        moo_bind_bool_property (G_OBJECT (dependent[i]), "sensitive",
                                G_OBJECT (btn), "active", invert);
}


gboolean
moo_watch_remove (guint watch_id)
{
    Watch *watch;

    if (!watches)
        return FALSE;

    watch = g_hash_table_lookup (watches, GUINT_TO_POINTER (watch_id));

    if (!watch)
        return FALSE;

    g_hash_table_remove (watches, GUINT_TO_POINTER (watch_id));

    return TRUE;
}


void
moo_watch_add_signal (guint       watch_id,
                      const char *source_signal)
{
    Watch *watch;

    g_return_if_fail (watches != NULL);
    g_return_if_fail (source_signal != NULL);

    watch = g_hash_table_lookup (watches, GUINT_TO_POINTER (watch_id));
    g_return_if_fail (watch != NULL);
    g_return_if_fail (watch->klass == &PropWatchClass);

    g_signal_connect_swapped (MOO_OBJECT_PTR_GET (watch->source),
                              source_signal, G_CALLBACK (prop_watch_check),
                              watch);
}


void
moo_watch_add_property (guint       watch_id,
                        const char *source_prop)
{
    char *signal;

    g_return_if_fail (source_prop != NULL);

    signal = g_strdup_printf ("notify::%s", source_prop);
    moo_watch_add_signal (watch_id, signal);

    g_free (signal);
}


/************************************************************/
/* SignalWatch
 */

typedef struct {
    Watch parent;
    guint signal_id;
    GQuark detail;
} SignalWatch;


static void signal_watch_invoke     (SignalWatch    *watch);
static void signal_watch_destroy    (Watch          *watch);

static WatchClass SignalWatchClass = {NULL, NULL, signal_watch_destroy};


static gboolean
check_signal (GObject    *obj,
              const char *signal,
              guint      *signal_id,
              GQuark     *detail,
              gboolean    any_return)
{
    GSignalQuery query;

    if (!g_signal_parse_name (signal, G_OBJECT_TYPE (obj), signal_id, detail, FALSE))
    {
        g_warning ("%s: could not parse signal '%s' of '%s' object",
                   G_STRLOC, signal, g_type_name (G_OBJECT_TYPE (obj)));
        return FALSE;
    }

    g_signal_query (*signal_id, &query);

    if (query.n_params > 0)
    {
        g_warning ("%s: implement me", G_STRLOC);
        return FALSE;
    }

    switch (query.return_type)
    {
        case G_TYPE_NONE:
            break;

        case G_TYPE_BOOLEAN:
        case G_TYPE_INT:
        case G_TYPE_UINT:
            if (!any_return)
                g_warning ("%s: implement me", G_STRLOC);
            return FALSE;
            break;

        default:
            g_warning ("%s: implement me", G_STRLOC);
            return FALSE;
    }

    return TRUE;
}


static SignalWatch*
signal_watch_new (GObject       *target,
                  const char    *target_signal,
                  GObject       *source,
                  const char    *source_signal)
{
    SignalWatch *watch;
    guint t_id, s_id;
    GQuark t_detail, s_detail;

    g_return_val_if_fail (G_IS_OBJECT (target), NULL);
    g_return_val_if_fail (G_IS_OBJECT (source), NULL);
    g_return_val_if_fail (target_signal != NULL, NULL);
    g_return_val_if_fail (source_signal != NULL, NULL);

    if (!check_signal (target, target_signal, &t_id, &t_detail, TRUE))
        return NULL;

    if (!check_signal (source, source_signal, &s_id, &s_detail, FALSE))
        return NULL;

    watch = watch_new (SignalWatch, &SignalWatchClass, source, target, NULL, NULL);

    watch->signal_id = t_id;
    watch->detail = t_detail;

    g_signal_connect_swapped (source, source_signal,
                              G_CALLBACK (signal_watch_invoke),
                              watch);

    return watch;
}


static void
signal_watch_destroy (Watch *watch)
{
    if (MOO_OBJECT_PTR_GET (watch->source))
        g_signal_handlers_disconnect_by_func (MOO_OBJECT_PTR_GET (watch->source),
                                              (gpointer) signal_watch_invoke,
                                              watch);
}


static void
signal_watch_invoke (SignalWatch *watch)
{
    int ret[4];
    g_signal_emit (MOO_OBJECT_PTR_GET (watch->parent.target),
                   watch->signal_id, watch->detail, ret);
}


guint
moo_bind_signal (gpointer            target,
                 const char         *target_signal,
                 gpointer            source,
                 const char         *source_signal)
{
    SignalWatch *watch;

    watch = signal_watch_new (target, target_signal, source, source_signal);

    if (!watch)
        return 0;

    return watch->parent.id;
}
