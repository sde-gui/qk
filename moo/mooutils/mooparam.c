/*
 *   mooutils/mooparam.c
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

#include "mooutils/mooparam.h"
#include <gobject/gvaluecollector.h>
#include <string.h> /* G_VALUE_COLLECT calls memset */


static void         copy_param          (GParameter *dest,
                                         GParameter *src);


GParameter  *moo_param_array_collect    (GType       type,
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


GParameter  *moo_param_array_add        (GType       type,
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


GParameter  *moo_param_array_add_type   (GParameter *src,
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


GParameter  *moo_param_array_add_valist (GType       type,
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


GParameter  *moo_param_array_add_type_valist
                                        (GParameter *src,
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
        if (G_TYPE_FUNDAMENTAL (type) == G_TYPE_INVALID) {
            g_warning ("%s: invalid type id passed", G_STRLOC);
            moo_param_array_free ((GParameter*)copy->data, copy->len);
            g_array_free (copy, FALSE);
            return NULL;
        }

        param.name = g_strdup (name);
        g_value_init (&param.value, type);
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


GParameter  *moo_param_array_collect_valist (GType       type,
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

        if (!pspec) {
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

        if (error) {
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


void         moo_param_array_free       (GParameter *array,
                                         guint       len)
{
    guint i;
    for (i = 0; i < len; ++i) {
        g_value_unset (&array[i].value);
        g_free ((char*)array[i].name);
    }
    g_free (array);
}


static void         copy_param          (GParameter *dest,
                                         GParameter *src)
{
    dest->name = g_strdup (src->name);
    g_value_init (&dest->value, G_VALUE_TYPE (&src->value));
    g_value_copy (&src->value, &dest->value);
}
