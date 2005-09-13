/*
 *   moovalue.c
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

#include "mooutils/moovalue.h"
#include <string.h>


static gpointer copy_gtype (gpointer boxed)
{
    return boxed;
}

static void     free_gtype (G_GNUC_UNUSED gpointer boxed)
{
}


GType       moo_gtype_get_type              (void)
{
    static GType type = 0;
    if (!type)
    {
        type = g_boxed_type_register_static ("GType",
                                             copy_gtype, free_gtype);
    }

    return type;
}


void        moo_value_set_gtype             (GValue     *value,
                                             GType       v_gtype)
{
    g_value_set_static_boxed (value, (gconstpointer) v_gtype);
}


GType       moo_value_get_gtype             (const GValue *value)
{
    return (GType) g_value_get_boxed (value);
}


/****************************************************************************/
/* Converting values
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


gboolean        moo_value_equal             (const GValue   *a,
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
