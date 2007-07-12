/*
 *   mooutils-gobject.c
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

#include "mooutils/mooutils-gobject-private.h"
#include "mooutils/mooclosure.h"
#include <gobject/gvaluecollector.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>


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
    *ptr = _moo_value_get_gtype (value);
    return NULL;
}


GType
_moo_gtype_get_type (void)
{
    static GType type = 0;

    if (G_UNLIKELY (!type))
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


static void
_moo_value_set_gtype (GValue *value,
                      GType   v_gtype)
{
    MOO_GTYPE_PEEK(value) = (gpointer) v_gtype;
}


GType
_moo_value_get_gtype (const GValue *value)
{
    return (GType) MOO_GTYPE_PEEK(value);
}


static void
param_gtype_set_default (GParamSpec *pspec,
                         GValue     *value)
{
    MooParamSpecGType *mspec = MOO_PARAM_SPEC_GTYPE (pspec);
    _moo_value_set_gtype (value, mspec->base);
}


static gboolean
param_gtype_validate (GParamSpec   *pspec,
                      GValue       *value)
{
    MooParamSpecGType *mspec = MOO_PARAM_SPEC_GTYPE (pspec);
    GType t = _moo_value_get_gtype (value);
    gboolean changed = FALSE;

    if (G_TYPE_IS_DERIVABLE (mspec->base))
    {
        if (!g_type_is_a (t, mspec->base))
        {
            _moo_value_set_gtype (value, mspec->base);
            changed = TRUE;
        }
    }
    else if (!g_type_name (t))
    {
        _moo_value_set_gtype (value, mspec->base);
        changed = TRUE;
    }

    return changed;
}


static int
param_gtype_cmp (G_GNUC_UNUSED GParamSpec *pspec,
                 const GValue *value1,
                 const GValue *value2)
{
    GType t1 = _moo_value_get_gtype (value1);
    GType t2 = _moo_value_get_gtype (value2);
    return t1 == t2 ? 0 : (t1 < t2 ? -1 : 1);
}


GType
_moo_param_gtype_get_type (void)
{
    static GType type = 0;

    if (G_UNLIKELY (!type))
    {
        static GParamSpecTypeInfo info = {
            sizeof (MooParamSpecGType), /* instance_size */
            16,                         /* n_preallocs */
            NULL,                       /* instance_init */
            0,							/* value_type */
            NULL,                       /* finalize */
            param_gtype_set_default,    /* value_set_default */
            param_gtype_validate,       /* value_validate */
            param_gtype_cmp,            /* values_cmp */
        };

		info.value_type = MOO_TYPE_GTYPE;
        type = g_param_type_register_static ("MooParamGType", &info);
    }

    return type;
}


#if 0
GParamSpec*
_moo_param_spec_gtype (const char     *name,
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
#endif


/*****************************************************************************/
/* Converting values forth and back
 */

static char *
flags_to_string (int flags)
{
    if (flags)
        return g_strdup_printf ("%d", flags);
    else
        return g_strdup ("");
}


static gboolean
string_to_flags (const char *string,
                 int        *flags)
{
    GValue ival;

    if (!string || !string[0])
    {
        *flags = 0;
        return TRUE;
    }

    ival.g_type = 0;
    g_value_init (&ival, G_TYPE_INT);

    if (!_moo_value_convert_from_string (string, &ival))
        return FALSE;

    *flags = g_value_get_int (&ival);
    return TRUE;
}


gboolean
_moo_value_convert (const GValue *src,
                    GValue       *dest)
{
    GType src_type, dest_type;

    g_return_val_if_fail (G_IS_VALUE (src) && G_IS_VALUE (dest), FALSE);

    src_type = G_VALUE_TYPE (src);
    dest_type = G_VALUE_TYPE (dest);

    g_return_val_if_fail (_moo_value_type_supported (src_type), FALSE);
    g_return_val_if_fail (_moo_value_type_supported (dest_type), FALSE);

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

        if (src_type == G_TYPE_UINT)
        {
            char *string =
                    g_strdup_printf ("%u", g_value_get_uint (src));
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

        if (G_TYPE_IS_FLAGS (src_type))
        {
            char *string = flags_to_string (g_value_get_flags (src));
            g_value_set_string (dest, string);
            g_free (string);
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
            {
                g_value_set_int (dest, 0);
            }
            else
            {
                long v;

                errno = 0;
                v = strtol (string, NULL, 10);

                if (errno)
                    return FALSE;

#if G_MAXLONG > G_MAXINT
                if (v > G_MAXINT || v < G_MININT)
                    return FALSE;
#endif

                g_value_set_int (dest, v);
            }

            return TRUE;
        }

        if (dest_type == G_TYPE_UINT)
        {
            if (!string || !string[0])
            {
                g_value_set_uint (dest, 0);
            }
            else
            {
                gulong v;

                errno = 0;
                v = strtoul (string, NULL, 10);

                if (errno)
                    return FALSE;

#if G_MAXULONG > G_MAXUINT
                if (v > G_MAXUINT)
                    return FALSE;
#endif

                g_value_set_uint (dest, v);
            }

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

        if (G_TYPE_IS_FLAGS (dest_type))
        {
            int flags;

            if (string_to_flags (string, &flags))
            {
                g_value_set_flags (dest, flags);
                return TRUE;
            }

            return FALSE;
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

    if (G_TYPE_IS_FLAGS (src_type) && dest_type == G_TYPE_INT)
    {
        g_value_set_int (dest, g_value_get_flags (src));
        return TRUE;
    }

    if (G_TYPE_IS_FLAGS (dest_type) && src_type == G_TYPE_INT)
    {
        g_value_set_flags (dest, g_value_get_int (src));
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
_moo_value_equal (const GValue *a,
                  const GValue *b)
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

    if (type == G_TYPE_UINT)
        return g_value_get_uint (a) == g_value_get_uint (b);

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
_moo_value_type_supported (GType type)
{
    return type == G_TYPE_BOOLEAN ||
            type == G_TYPE_INT ||
            type == G_TYPE_UINT ||
            type == G_TYPE_DOUBLE ||
            type == G_TYPE_STRING ||
            type == GDK_TYPE_COLOR ||
            G_TYPE_IS_ENUM (type) ||
            G_TYPE_IS_FLAGS (type);
}


static gboolean
_moo_value_convert_to_bool (const GValue *val)
{
    GValue result;
    result.g_type = 0;
    g_value_init (&result, G_TYPE_BOOLEAN);
    _moo_value_convert (val, &result);
    return g_value_get_boolean (&result);
}


static int
_moo_value_convert_to_int (const GValue *val)
{
    GValue result;
    result.g_type = 0;
    g_value_init (&result, G_TYPE_INT);
    _moo_value_convert (val, &result);
    return g_value_get_int (&result);
}


double
_moo_value_convert_to_double (const GValue *val)
{
    GValue result;
    result.g_type = 0;
    g_value_init (&result, G_TYPE_DOUBLE);
    _moo_value_convert (val, &result);
    return g_value_get_double (&result);
}


#if 0
int
_moo_value_convert_to_flags (const GValue *val,
                             GType         flags_type)
{
    GValue result;
    result.g_type = 0;
    g_value_init (&result, flags_type);
    _moo_value_convert (val, &result);
    return g_value_get_flags (&result);
}

int
_moo_value_convert_to_enum (const GValue *val,
                            GType         enum_type)
{
    GValue result;
    result.g_type = 0;
    g_value_init (&result, enum_type);
    _moo_value_convert (val, &result);
    return g_value_get_enum (&result);
}

const GdkColor*
_moo_value_convert_to_color (const GValue *val)
{
    static GValue result;

    if (G_IS_VALUE (&result))
        g_value_unset (&result);

    g_value_init (&result, GDK_TYPE_COLOR);

    if (!_moo_value_convert (val, &result))
        return NULL;
    else
        return g_value_get_boxed (&result);
}
#endif


const char*
_moo_value_convert_to_string (const GValue *val)
{
    static GValue result;

    if (G_IS_VALUE (&result))
        g_value_unset (&result);

    g_value_init (&result, G_TYPE_STRING);

    if (!_moo_value_convert (val, &result))
        return NULL;
    else
        return g_value_get_string (&result);
}


gboolean
_moo_value_convert_from_string (const char *string,
                                GValue     *val)
{
    GValue str_val;
    gboolean result;

    g_return_val_if_fail (G_IS_VALUE (val), FALSE);
    g_return_val_if_fail (string != NULL, FALSE);

    str_val.g_type = 0;
    g_value_init (&str_val, G_TYPE_STRING);
    g_value_set_static_string (&str_val, string);
    result = _moo_value_convert (&str_val, val);
    g_value_unset (&str_val);

    return result;
}


int
_moo_convert_string_to_int (const char *string,
                            int         default_val)
{
    int int_val;

    if (string)
    {
        GValue str_val;
        str_val.g_type = 0;
        g_value_init (&str_val, G_TYPE_STRING);
        g_value_set_static_string (&str_val, string);
        int_val = _moo_value_convert_to_int (&str_val);
        g_value_unset (&str_val);
    }
    else
    {
        int_val = default_val;
    }

    return int_val;
}


gboolean
_moo_convert_string_to_bool (const char *string,
                             gboolean    default_val)
{
    gboolean bool_val;

    if (string)
    {
        GValue str_val;
        str_val.g_type = 0;
        g_value_init (&str_val, G_TYPE_STRING);
        g_value_set_static_string (&str_val, string);
        bool_val = _moo_value_convert_to_bool (&str_val);
        g_value_unset (&str_val);
    }
    else
    {
        bool_val = default_val;
    }

    return bool_val;
}


const char*
_moo_convert_bool_to_string (gboolean value)
{
    GValue bool_val;

    bool_val.g_type = 0;
    g_value_init (&bool_val, G_TYPE_BOOLEAN);
    g_value_set_boolean (&bool_val, value);

    return _moo_value_convert_to_string (&bool_val);
}


const char*
_moo_convert_int_to_string (int value)
{
    GValue int_val;

    int_val.g_type = 0;
    g_value_init (&int_val, G_TYPE_INT);
    g_value_set_int (&int_val, value);

    return _moo_value_convert_to_string (&int_val);
}


gboolean
_moo_value_change_type (GValue *val,
                        GType   new_type)
{
    GValue tmp;
    gboolean result;

    g_return_val_if_fail (G_IS_VALUE (val), FALSE);
    g_return_val_if_fail (_moo_value_type_supported (new_type), FALSE);

    tmp.g_type = 0;
    g_value_init (&tmp, new_type);
    result = _moo_value_convert (val, &tmp);

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

#if 0
static void         copy_param          (GParameter *dest,
                                         GParameter *src);
typedef GParamSpec* (*MooLookupProperty)    (GObjectClass *klass,
                                             const char   *prop_name);

GParameter*
_moo_param_array_collect (GType       type,
                          MooLookupProperty lookup_func,
                          guint      *len,
                          const char *first_prop_name,
                          ...)
{
    GParameter *array;
    va_list var_args;

    va_start (var_args, first_prop_name);
    array = _moo_param_array_collect_valist (type, lookup_func, len,
                                             first_prop_name,
                                             var_args);
    va_end (var_args);

    return array;
}

GParameter*
_moo_param_array_add (GType       type,
                      GParameter *src,
                      guint       len,
                      guint      *new_len,
                      const char *first_prop_name,
                      ...)
{
    GParameter *copy;
    va_list var_args;

    va_start (var_args, first_prop_name);
    copy = _moo_param_array_add_valist (type, src, len, new_len,
                                        first_prop_name,
                                        var_args);
    va_end (var_args);

    return copy;
}

GParameter*
_moo_param_array_add_type (GParameter *src,
                           guint       len,
                           guint      *new_len,
                           const char *first_prop_name,
                           ...)
{
    GParameter *copy;
    va_list var_args;

    va_start (var_args, first_prop_name);
    copy = _moo_param_array_add_type_valist (src, len, new_len,
                                             first_prop_name,
                                             var_args);
    va_end (var_args);

    return copy;
}

GParameter*
_moo_param_array_add_type_valist (GParameter *src,
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
            _moo_param_array_free ((GParameter*)copy->data, copy->len);
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
            _moo_param_array_free ((GParameter*)copy->data, copy->len);
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
_moo_param_array_add_valist (GType       type,
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
            _moo_param_array_free ((GParameter*)copy->data, copy->len);
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
            _moo_param_array_free ((GParameter*)copy->data, copy->len);
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
_moo_param_array_collect_valist (GType       type,
                                 MooLookupProperty lookup_func,
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
        GParamSpec *pspec;

        pspec = lookup_func ? lookup_func (klass, prop_name) :
                g_object_class_find_property (klass, prop_name);

        if (!pspec)
        {
            g_warning ("%s: class '%s' doesn't have property '%s'",
                       G_STRLOC, g_type_name (type), prop_name);
            _moo_param_array_free ((GParameter*)params->data, params->len);
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
            _moo_param_array_free ((GParameter*)params->data, params->len);
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

static void
copy_param (GParameter *dest,
            GParameter *src)
{
    dest->name = g_strdup (src->name);
    g_value_init (&dest->value, G_VALUE_TYPE (&src->value));
    g_value_copy (&src->value, &dest->value);
}
#endif


void
_moo_param_array_free (GParameter *array,
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


/*****************************************************************************/
/* Signal that does not require class method
 */

guint
_moo_signal_new_cb (const gchar        *signal_name,
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
    GClosure *closure = NULL;

    g_return_val_if_fail (signal_name != NULL, 0);

    va_start (args, n_params);

    if (handler)
        closure = g_cclosure_new (handler, NULL, NULL);

    signal_id = g_signal_new_valist (signal_name, itype, signal_flags, closure,
                                     accumulator, accu_data, c_marshaller,
                                     return_type, n_params, args);

    va_end (args);

    return signal_id;
}


/****************************************************************************/
/* Property watch
 */

static GHashTable *watches = NULL;
static guint watch_last_id = 0;

#define Watch MooObjectWatch
#define WatchClass MooObjectWatchClass
#define watch_new _moo_object_watch_new
#define watch_alloc _moo_object_watch_alloc


static void
watch_destroy (Watch *w)
{
    if (w)
    {
        if (w->klass->destroy)
            w->klass->destroy (w);
        if (w->notify)
            w->notify (w->notify_data);
        _moo_object_ptr_free (w->source);
        _moo_object_ptr_free (w->target);
        g_free (w);
    }
}


static void
watch_source_died (Watch *w)
{
    if (w->klass->source_notify)
        w->klass->source_notify (w);
    _moo_object_ptr_free (w->source);
    w->source = NULL;
    g_hash_table_remove (watches, GUINT_TO_POINTER (w->id));
}

static void
watch_target_died (Watch *w)
{
    if (w->klass->target_notify)
        w->klass->target_notify (w);
    _moo_object_ptr_free (w->target);
    w->target = NULL;
    g_hash_table_remove (watches, GUINT_TO_POINTER (w->id));
}


Watch *
watch_alloc (gsize          size,
             WatchClass    *klass,
             gpointer       source,
             gpointer       target,
             GDestroyNotify notify,
             gpointer       notify_data)
{
    Watch *w;

    g_return_val_if_fail (size >= sizeof (Watch), NULL);
    g_return_val_if_fail (G_IS_OBJECT (source), NULL);
    g_return_val_if_fail (G_IS_OBJECT (target), NULL);

    w = g_malloc0 (size);
    w->source = _moo_object_ptr_new (source, (GWeakNotify) watch_source_died, w);
    w->target = _moo_object_ptr_new (target, (GWeakNotify) watch_target_died, w);
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
_moo_add_property_watch (gpointer            target,
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


static void
_moo_copy_boolean (GValue             *target,
                   const GValue       *source,
                   G_GNUC_UNUSED gpointer dummy)
{
    g_value_set_boolean (target, g_value_get_boolean (source) ? TRUE : FALSE);
}


static void
_moo_invert_boolean (GValue             *target,
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
    g_return_val_if_fail (G_IS_OBJECT (target), 0);
    g_return_val_if_fail (G_IS_OBJECT (source), 0);
    g_return_val_if_fail (target_prop && source_prop, 0);

    if (invert)
        return _moo_add_property_watch (target, target_prop, source, source_prop,
                                        _moo_invert_boolean, NULL, NULL);
    else
        return _moo_add_property_watch (target, target_prop, source, source_prop,
                                        _moo_copy_boolean, NULL, NULL);
}


#if 0
gboolean
_moo_sync_bool_property (gpointer            slave,
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
            _moo_watch_remove (id1);
    }

    return id1 && id2;
}
#endif


void
moo_bind_sensitive (GtkWidget          *btn,
                    GtkWidget         **dependent,
                    int                 num_dependent,
                    gboolean            invert)
{
    int i;

    g_return_if_fail (G_IS_OBJECT (btn));

    for (i = 0; i < num_dependent; ++i)
        moo_bind_bool_property (dependent[i], "sensitive", btn, "active", invert);
}


#if 0
gboolean
_moo_watch_remove (guint watch_id)
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
_moo_watch_add_signal (guint       watch_id,
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
_moo_watch_add_property (guint       watch_id,
                         const char *source_prop)
{
    char *signal;

    g_return_if_fail (source_prop != NULL);

    signal = g_strdup_printf ("notify::%s", source_prop);
    _moo_watch_add_signal (watch_id, signal);

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
_moo_bind_signal (gpointer            target,
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
#endif


/*****************************************************************************/
/* Data store
 */

struct _MooData
{
    GHashTable *hash;
    guint ref_count;
    GHashFunc hash_func;
    GEqualFunc key_equal_func;
    GDestroyNotify key_destroy_func;
};


static MooPtr *
_moo_ptr_ref (MooPtr *ptr)
{
    if (ptr)
        ptr->ref_count++;
    return ptr;
}


static void
_moo_ptr_unref (MooPtr *ptr)
{
    if (ptr && !--(ptr->ref_count))
    {
        if (ptr->free_func)
            ptr->free_func (ptr->data);
        g_free (ptr);
    }
}


static MooPtr *
_moo_ptr_new (gpointer        data,
              GDestroyNotify  free_func)
{
    MooPtr *ptr;

    g_return_val_if_fail (data != NULL, NULL);

    ptr = g_new (MooPtr, 1);

    ptr->data = data;
    ptr->free_func = free_func;
    ptr->ref_count = 1;

    return ptr;
}


GType
_moo_ptr_get_type (void)
{
    static GType type = 0;

    if (G_UNLIKELY (!type))
        type = g_boxed_type_register_static ("MooPtr",
                                             (GBoxedCopyFunc) _moo_ptr_ref,
                                             (GBoxedFreeFunc) _moo_ptr_unref);

    return type;
}


static MooData *
_moo_data_ref (MooData *data)
{
    if (data)
        data->ref_count++;
    return data;
}


void
_moo_data_unref (MooData *data)
{
    if (data && !--(data->ref_count))
    {
        g_hash_table_destroy (data->hash);
        g_free (data);
    }
}


GType
_moo_data_get_type (void)
{
    static GType type = 0;

    if (G_UNLIKELY (!type))
        type = g_boxed_type_register_static ("MooData",
                                             (GBoxedCopyFunc) _moo_data_ref,
                                             (GBoxedFreeFunc) _moo_data_unref);

    return type;
}


static void
free_gvalue (GValue *val)
{
    if (val)
    {
        g_value_unset (val);
        g_free (val);
    }
}


static GValue *
copy_gvalue (const GValue *val)
{
    GValue *copy;

    g_return_val_if_fail (G_IS_VALUE (val), NULL);

    copy = g_new (GValue, 1);
    copy->g_type = 0;
    g_value_init (copy, G_VALUE_TYPE (val));
    g_value_copy (val, copy);

    return copy;
}


MooData *
_moo_data_new (GHashFunc       hash_func,
               GEqualFunc      key_equal_func,
               GDestroyNotify  key_destroy_func)
{
    MooData *data;

    g_return_val_if_fail (hash_func != NULL, NULL);
    g_return_val_if_fail (key_equal_func != NULL, NULL);

    data = g_new0 (MooData, 1);

    data->hash = g_hash_table_new_full (hash_func, key_equal_func,
                                        key_destroy_func,
                                        (GDestroyNotify) free_gvalue);
    data->ref_count = 1;
    data->hash_func = hash_func;
    data->key_equal_func = key_equal_func;
    data->key_destroy_func = key_destroy_func;

    return data;
}


void
_moo_data_insert_value (MooData        *data,
                        gpointer        key,
                        const GValue   *value)
{
    g_return_if_fail (data != NULL);
    g_return_if_fail (G_IS_VALUE (value));
    g_hash_table_insert (data->hash, key, copy_gvalue (value));
}


void
_moo_data_insert_ptr (MooData        *data,
                      gpointer        key,
                      gpointer        value,
                      GDestroyNotify  destroy)
{
    MooPtr *ptr;
    GValue gval;

    g_return_if_fail (data != NULL);
    g_return_if_fail (value != NULL);

    ptr = _moo_ptr_new (value, destroy);
    gval.g_type = 0;
    g_value_init (&gval, MOO_TYPE_PTR);
    g_value_set_boxed (&gval, ptr);

    _moo_data_insert_value (data, key, &gval);

    g_value_unset (&gval);
    _moo_ptr_unref (ptr);
}


void
_moo_data_remove (MooData  *data,
                  gpointer  key)
{
    g_return_if_fail (data != NULL);
    g_hash_table_remove (data->hash, key);
}


void
_moo_data_clear (MooData *data)
{
    g_return_if_fail (data != NULL);

    g_hash_table_destroy (data->hash);

    data->hash = g_hash_table_new_full (data->hash_func,
                                        data->key_equal_func,
                                        data->key_destroy_func,
                                        (GDestroyNotify) free_gvalue);
}


#if 0
guint
_moo_data_size (MooData *data)
{
    g_return_val_if_fail (data != NULL, 0);
    return g_hash_table_size (data->hash);
}

gboolean
_moo_data_has_key (MooData  *data,
                   gpointer  key)
{
    g_return_val_if_fail (data != NULL, FALSE);
    return g_hash_table_lookup (data->hash, key) != NULL;
}

GType
_moo_data_get_value_type (MooData  *data,
                          gpointer  key)
{
    GValue *value;
    g_return_val_if_fail (data != NULL, 0);
    value = g_hash_table_lookup (data->hash, key);
    return value ? G_VALUE_TYPE (value) : G_TYPE_NONE;
}
#endif


gboolean
_moo_data_get_value (MooData        *data,
                     gpointer        key,
                     GValue         *dest)
{
    GValue *value;

    g_return_val_if_fail (data != NULL, FALSE);
    g_return_val_if_fail (!dest || dest->g_type == 0, FALSE);

    value = g_hash_table_lookup (data->hash, key);

    if (value && dest)
    {
        g_value_init (dest, G_VALUE_TYPE (value));
        g_value_copy (value, dest);
    }

    return value != NULL;
}


gpointer
_moo_data_get_ptr (MooData  *data,
                   gpointer  key)
{
    MooPtr *ptr;
    GValue *value;

    g_return_val_if_fail (data != NULL, NULL);

    value = g_hash_table_lookup (data->hash, key);

    if (value)
    {
        g_return_val_if_fail (G_VALUE_TYPE (value) == MOO_TYPE_PTR, NULL);
        ptr = g_value_get_boxed (value);
        return ptr->data;
    }
    else
    {
        return NULL;
    }
}
