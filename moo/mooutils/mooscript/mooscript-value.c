/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   as-script-value.c
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

#include "mooscript-value.h"
#include "mooscript-context.h"
#include <string.h>


static MSValue *MS_None;
static MSValue *MS_True;
static MSValue *MS_False;


static MSValue *
ms_value_new (MSValueType type)
{
    MSValue *val = g_new0 (MSValue, 1);
    val->ref_count = 1;
    val->type = type;
    return val;
}


MSValue *
ms_value_none (void)
{
    if (!MS_None)
        return MS_None = ms_value_new (MS_VALUE_NONE);
    else
        return ms_value_ref (MS_None);
}


gboolean
ms_value_is_none (MSValue *value)
{
    g_return_val_if_fail (value != NULL, FALSE);
    return value->type == MS_VALUE_NONE;
}


MSValue *
ms_value_false (void)
{
    if (!MS_False)
        return MS_False = ms_value_int (FALSE);
    else
        return ms_value_ref (MS_False);
}


MSValue *
ms_value_true (void)
{
    if (!MS_True)
        return MS_True = ms_value_int (TRUE);
    else
        return ms_value_ref (MS_True);
}


MSValue *
ms_value_bool (gboolean val)
{
    return val ? ms_value_true () : ms_value_false ();
}


MSValue *
ms_value_int (int ival)
{
    MSValue *val = ms_value_new (MS_VALUE_INT);
    val->ival = ival;
    return val;
}


MSValue *
ms_value_string (const char *string)
{
    return ms_value_take_string (g_strdup (string));
}


MSValue *
ms_value_string_len (const char *string,
                     int         chars)
{
    if (chars < 0)
        return ms_value_string (string);
    else
        return ms_value_take_string (g_strndup (string,
                                     g_utf8_offset_to_pointer (string, chars) - string));
}


MSValue *
ms_value_take_string (char *string)
{
    MSValue *val;
    g_return_val_if_fail (string != NULL, NULL);
    val = ms_value_new (MS_VALUE_STRING);
    val->str = string;
    return val;
}


MSValue *
ms_value_object (gpointer object)
{
    MSValue *val;
    g_return_val_if_fail (G_IS_OBJECT (object), NULL);
    val = ms_value_new (MS_VALUE_OBJECT);
    val->ptr = g_object_ref (object);
    return val;
}


MSValue *
ms_value_gvalue (GValue *gval)
{
    MSValue *val;
    g_return_val_if_fail (G_IS_VALUE (gval), NULL);
    val = ms_value_new (MS_VALUE_GVALUE);

    val->gval = g_new (GValue, 1);
    val->gval->g_type = 0;
    g_value_init (val->gval, G_VALUE_TYPE (gval));
    g_value_copy (gval, val->gval);

    return val;
}


MSValue *
ms_value_list (guint n_elms)
{
    MSValue *val;
    guint i;

    val = ms_value_new (MS_VALUE_LIST);
    val->list.elms = g_new (MSValue*, n_elms);
    val->list.n_elms = n_elms;

    for (i = 0; i < n_elms; ++i)
        val->list.elms[i] = ms_value_none ();

    return val;
}


void
ms_value_list_set_elm (MSValue    *list,
                       guint       index,
                       MSValue    *elm)
{
    g_return_if_fail (list != NULL);
    g_return_if_fail (elm != NULL);
    g_return_if_fail (list->type == MS_VALUE_LIST);
    g_return_if_fail (index < list->list.n_elms);

    if (list->list.elms[index] != elm)
    {
        ms_value_unref (list->list.elms[index]);
        list->list.elms[index] = ms_value_ref (elm);
    }
}


MSValue *
ms_value_ref (MSValue *val)
{
    g_return_val_if_fail (val != NULL, NULL);
    val->ref_count++;
    return val;
}


void
ms_value_unref (MSValue *val)
{
    guint i;

    g_return_if_fail (val != NULL);

    if (--val->ref_count)
        return;

    switch (val->type)
    {
        case MS_VALUE_STRING:
            g_free (val->str);
            break;

        case MS_VALUE_NONE:
            if (val == MS_None)
                MS_None = NULL;
            break;

        case MS_VALUE_INT:
            if (val == MS_False)
                MS_False = NULL;
            else if (val == MS_True)
                MS_True = NULL;
            break;

        case MS_VALUE_OBJECT:
            g_object_unref (val->ptr);
            break;

        case MS_VALUE_GVALUE:
            g_value_unset (val->gval);
            g_free (val->gval);
            break;

        case MS_VALUE_LIST:
            for (i = 0; i < val->list.n_elms; ++i)
                ms_value_unref (val->list.elms[i]);
            g_free (val->list.elms);
            break;
    }

    g_free (val);
}


const char *
ms_binary_op_name (MSBinaryOp op)
{
    static const char *names[MS_BINARY_OP_LAST] = {
        "@PLUS", "@MINUS", "@MULT", "@DIV", "@AND", "@OR",
        "@EQ", "@NEQ", "@LT", "@GT", "@LE", "@GE", "@FORMAT"
    };

    g_return_val_if_fail (op < MS_BINARY_OP_LAST, NULL);
    return names[op];
}


const char *
ms_unary_op_name (MSUnaryOp op)
{
    static const char *names[MS_UNARY_OP_LAST] = {
        "@UMINUS", "@NOT", "@LEN"
    };

    g_return_val_if_fail (op < MS_UNARY_OP_LAST, NULL);
    return names[op];
}


gboolean
ms_value_get_bool (MSValue *val)
{
    g_return_val_if_fail (val != NULL, FALSE);

    switch (val->type)
    {
        case MS_VALUE_STRING:
            return val->str[0] != 0;
        case MS_VALUE_INT:
            return val->ival != 0;
        case MS_VALUE_NONE:
            return FALSE;
        case MS_VALUE_OBJECT:
            return val->ptr != NULL;
        case MS_VALUE_LIST:
            return val->list.n_elms != 0;
        case MS_VALUE_GVALUE:
            switch (G_TYPE_FUNDAMENTAL (G_VALUE_TYPE (val->gval)))
            {
                case G_TYPE_NONE:
                    return FALSE;
                case G_TYPE_CHAR:
                    return g_value_get_char (val->gval) != 0;
                case G_TYPE_UCHAR:
                    return g_value_get_uchar (val->gval) != 0;
                case G_TYPE_BOOLEAN:
                    return g_value_get_boolean (val->gval);
                case G_TYPE_INT:
                    return g_value_get_int (val->gval) != 0;
                case G_TYPE_UINT:
                    return g_value_get_uint (val->gval) != 0;
                case G_TYPE_LONG:
                    return g_value_get_long (val->gval) != 0;
                case G_TYPE_ULONG:
                    return g_value_get_ulong (val->gval) != 0;
                case G_TYPE_INT64:
                    return g_value_get_int64 (val->gval) != 0;
                case G_TYPE_UINT64:
                    return g_value_get_uint64 (val->gval) != 0;
                case G_TYPE_ENUM:
                    return g_value_get_enum (val->gval) != 0;
                case G_TYPE_FLAGS:
                    return g_value_get_flags (val->gval) != 0;
                case G_TYPE_FLOAT:
                    return g_value_get_float (val->gval) != 0;
                case G_TYPE_DOUBLE:
                    return g_value_get_double (val->gval) != 0;
                case G_TYPE_STRING:
                    return g_value_get_string (val->gval) != NULL &&
                            *g_value_get_string (val->gval) != 0;
                case G_TYPE_POINTER:
                    return g_value_get_pointer (val->gval) != NULL;
                case G_TYPE_BOXED:
                    return g_value_get_boxed (val->gval) != NULL;
                case G_TYPE_OBJECT:
                    return g_value_get_object (val->gval) != NULL;
                default:
                    g_return_val_if_reached (FALSE);
            }
    }

    g_return_val_if_reached (FALSE);
}


gboolean
ms_value_get_int (MSValue    *val,
                  int        *ival)
{
    g_return_val_if_fail (val != NULL, FALSE);
    g_return_val_if_fail (ival != NULL, FALSE);

    switch (val->type)
    {
        case MS_VALUE_INT:
            *ival = val->ival;
            return TRUE;

        case MS_VALUE_NONE:
            *ival = 0;
            return TRUE;

        case MS_VALUE_GVALUE:
            switch (G_TYPE_FUNDAMENTAL (G_VALUE_TYPE (val->gval)))
            {
                case G_TYPE_NONE:
                    *ival = 0;
                    return TRUE;
                case G_TYPE_CHAR:
                    *ival = g_value_get_char (val->gval) != 0;
                    return TRUE;
                case G_TYPE_UCHAR:
                    *ival = g_value_get_uchar (val->gval) != 0;
                    return TRUE;
                case G_TYPE_BOOLEAN:
                    *ival = g_value_get_boolean (val->gval);
                    return TRUE;
                case G_TYPE_INT:
                    *ival = g_value_get_int (val->gval) != 0;
                    return TRUE;
                case G_TYPE_UINT:
                    *ival = g_value_get_uint (val->gval) != 0;
                    return TRUE;
                case G_TYPE_LONG:
                    *ival = g_value_get_long (val->gval) != 0;
                    return TRUE;
                case G_TYPE_ULONG:
                    *ival = g_value_get_ulong (val->gval) != 0;
                    return TRUE;
                case G_TYPE_INT64:
                    *ival = g_value_get_int64 (val->gval) != 0;
                    return TRUE;
                case G_TYPE_UINT64:
                    *ival = g_value_get_uint64 (val->gval) != 0;
                    return TRUE;
                case G_TYPE_ENUM:
                    *ival = g_value_get_enum (val->gval) != 0;
                    return TRUE;
                case G_TYPE_FLAGS:
                    *ival = g_value_get_flags (val->gval) != 0;
                    return TRUE;

                default:
                    return FALSE;
            }

        default:
            return FALSE;
    }

    return FALSE;
}


static char *
print_list (MSValue **elms,
            guint     n_elms)
{
    guint i;
    GString *string = g_string_sized_new (6 * n_elms + 2);

    g_string_append_c (string, '[');

    for (i = 0; i < n_elms; ++i)
    {
        char *s;
        if (i)
            g_string_append (string, ", ");
        s = ms_value_print (elms[i]);
        g_string_append (string, s);
        g_free (s);
    }

    g_string_append_c (string, ']');
    return g_string_free (string, FALSE);
}


char *
ms_value_print (MSValue *val)
{
    g_return_val_if_fail (val != NULL, NULL);

    switch (val->type)
    {
        case MS_VALUE_STRING:
            return g_strdup (val->str);
        case MS_VALUE_INT:
            return g_strdup_printf ("%d", val->ival);
        case MS_VALUE_NONE:
            return g_strdup ("None");
        case MS_VALUE_OBJECT:
            return g_strdup_printf ("Object %p", val->ptr);
        case MS_VALUE_LIST:
            return print_list (val->list.elms, val->list.n_elms);
        case MS_VALUE_GVALUE:
            switch (G_TYPE_FUNDAMENTAL (G_VALUE_TYPE (val->gval)))
            {
                case G_TYPE_NONE:
                    return g_strdup ("None");
                case G_TYPE_CHAR:
                    return g_strdup_printf ("%c", g_value_get_char (val->gval));
                case G_TYPE_UCHAR:
                    return g_strdup_printf ("%c", g_value_get_uchar (val->gval));
                case G_TYPE_BOOLEAN:
                    return g_strdup_printf ("%d", g_value_get_boolean (val->gval));
                case G_TYPE_INT:
                    return g_strdup_printf ("%d", g_value_get_int (val->gval));
                case G_TYPE_UINT:
                    return g_strdup_printf ("%d", g_value_get_uint (val->gval));
                case G_TYPE_LONG:
                    return g_strdup_printf ("%ld", g_value_get_long (val->gval));
                case G_TYPE_ULONG:
                    return g_strdup_printf ("%ld", g_value_get_ulong (val->gval));
                case G_TYPE_INT64:
                    return g_strdup_printf ("%" G_GINT64_FORMAT, g_value_get_int64 (val->gval));
                case G_TYPE_UINT64:
                    return g_strdup_printf ("%" G_GUINT64_FORMAT, g_value_get_uint64 (val->gval));
                case G_TYPE_ENUM:
                    return g_strdup_printf ("%d", g_value_get_enum (val->gval));
                case G_TYPE_FLAGS:
                    return g_strdup_printf ("%d", g_value_get_flags (val->gval));
                case G_TYPE_FLOAT:
                    return g_strdup_printf ("%f", g_value_get_float (val->gval));
                case G_TYPE_DOUBLE:
                    return g_strdup_printf ("%f", g_value_get_double (val->gval));
                case G_TYPE_STRING:
                    return g_value_get_string (val->gval) ?
                            g_strdup (g_value_get_string (val->gval)) : g_strdup ("NULL");
                case G_TYPE_POINTER:
                    return g_strdup_printf ("Pointer %p", g_value_get_pointer (val->gval));
                case G_TYPE_BOXED:
                    return g_strdup_printf ("Boxed %p", g_value_get_boxed (val->gval));
                case G_TYPE_OBJECT:
                    return g_strdup_printf ("Object %p", g_value_get_object (val->gval));
                default:
                    g_return_val_if_reached (NULL);
            }
    }

    g_return_val_if_reached (NULL);
}


static MSValue *
func_plus (MSValue *a, MSValue *b, MSContext *ctx)
{
    if (a->type == MS_VALUE_INT && b->type == MS_VALUE_INT)
        return ms_value_int (a->ival + b->ival);
    else if (a->type == MS_VALUE_STRING && b->type == MS_VALUE_STRING)
        return ms_value_take_string (g_strdup_printf ("%s%s", a->str, b->str));

    return ms_context_set_error (ctx, MS_ERROR_TYPE,
                                 "invalid PLUS");
}


static MSValue *
func_minus (MSValue *a, MSValue *b, MSContext *ctx)
{
    if (a->type == MS_VALUE_INT && b->type == MS_VALUE_INT)
        return ms_value_int (a->ival - b->ival);
    return ms_context_set_error (ctx, MS_ERROR_TYPE,
                                 "invalid MINUS");
}


static MSValue *
func_mult (MSValue *a, MSValue *b, MSContext *ctx)
{
    if (a->type == MS_VALUE_INT && b->type == MS_VALUE_INT)
        return ms_value_int (a->ival * b->ival);

    if (a->type == MS_VALUE_STRING && b->type == MS_VALUE_INT)
    {
        char *s;
        guint len;
        int i;

        if (b->ival < 0)
            return ms_context_set_error (ctx, MS_ERROR_TYPE,
                                         "string * negative int");
        if (b->ival == 0)
            return ms_value_string ("");

        len = strlen (a->str);
        s = g_new (char, len * b->ival + 1);
        s[len * b->ival] = 0;

        for (i = 0; i < b->ival; ++i)
            memcpy (&s[i*len], a->str, len);

        return ms_value_take_string (s);
    }

    return ms_context_set_error (ctx, MS_ERROR_TYPE,
                                 "invalid MULT");
}


static MSValue *
func_div (MSValue *a, MSValue *b, MSContext *ctx)
{
    if (a->type == MS_VALUE_INT && b->type == MS_VALUE_INT)
    {
        if (b->ival)
            return ms_value_int (a->ival / b->ival);
        else
            return ms_context_set_error (ctx, MS_ERROR_VALUE,
                                         "division by zero");
    }

    return ms_context_set_error (ctx, MS_ERROR_TYPE,
                                 "invalid DIV");
}


static MSValue *
func_and (MSValue *a, MSValue *b)
{
    if (ms_value_get_bool (a) && ms_value_get_bool (b))
        return ms_value_ref (b);
    else
        return ms_value_false ();
}


static MSValue *
func_or (MSValue *a, MSValue *b)
{
    if (ms_value_get_bool (a))
        return ms_value_ref (a);
    else if (ms_value_get_bool (b))
        return ms_value_ref (b);
    else
        return ms_value_false ();
}


static gboolean
list_equal (MSValue *a, MSValue *b)
{
    guint i;

    if (a->list.n_elms != b->list.n_elms)
        return FALSE;

    for (i = 0; i < a->list.n_elms; ++i)
        if (!ms_value_equal (a->list.elms[i], b->list.elms[i]))
            return FALSE;

    return TRUE;
}


gboolean
ms_value_equal (MSValue *a, MSValue *b)
{
    if (a == b)
        return TRUE;

    if (a->type != b->type)
        return FALSE;

    switch (a->type)
    {
        case MS_VALUE_INT:
            return a->ival == b->ival;
        case MS_VALUE_NONE:
            return TRUE;
        case MS_VALUE_STRING:
            return !strcmp (a->str, b->str);
        case MS_VALUE_OBJECT:
            return a->ptr == b->ptr;
        case MS_VALUE_LIST:
            return list_equal (a, b);
        case MS_VALUE_GVALUE:
            g_return_val_if_reached (FALSE);
    }

    g_return_val_if_reached (FALSE);
}


#define CMP(a, b) ((a) < (b) ? -1 : ((a) > (b) ? 1 : 0))

static int
list_cmp (MSValue *a, MSValue *b)
{
    guint i;

    for (i = 0; i < a->list.n_elms && i < b->list.n_elms; ++i)
    {
        int c = ms_value_cmp (a->list.elms[i], b->list.elms[i]);

        if (c)
            return c;
    }

    return CMP (a->list.n_elms, b->list.n_elms);
}


int
ms_value_cmp (MSValue *a, MSValue *b)
{
    if (a == b)
        return 0;

    if (a->type != b->type)
        return a < b ? -1 : 1;

    switch (a->type)
    {
        case MS_VALUE_INT:
            return CMP (a->ival, b->ival);
        case MS_VALUE_NONE:
            return 0;
        case MS_VALUE_STRING:
            return strcmp (a->str, b->str);
        case MS_VALUE_OBJECT:
            return CMP (a->ptr, b->ptr);
        case MS_VALUE_LIST:
            return list_cmp (a, b);
        case MS_VALUE_GVALUE:
            g_return_val_if_reached (CMP (a, b));
    }

    g_return_val_if_reached (CMP (a, b));
}


static MSValue *
func_eq (MSValue *a, MSValue *b)
{
    return ms_value_bool (ms_value_equal (a, b));
}


static MSValue *
func_neq (MSValue *a, MSValue *b)
{
    return ms_value_bool (!ms_value_equal (a, b));
}


static MSValue *
func_lt (MSValue *a, MSValue *b)
{
    return ms_value_bool (ms_value_cmp (a, b) < 0);
}

static MSValue *
func_gt (MSValue *a, MSValue *b)
{
    return ms_value_bool (ms_value_cmp (a, b) > 0);
}

static MSValue *
func_le (MSValue *a, MSValue *b)
{
    return ms_value_bool (ms_value_cmp (a, b) <= 0);
}

static MSValue *
func_ge (MSValue *a, MSValue *b)
{
    return ms_value_bool (ms_value_cmp (a, b) >= 0);
}


static char *
format_value (char       format,
              MSValue   *value,
              MSContext *ctx)
{
    int ival;

    switch (format)
    {
        case 's':
            return ms_value_print (value);

        case 'd':
            if (!ms_value_get_int (value, &ival))
            {
                ms_context_set_error (ctx, MS_ERROR_TYPE, NULL);
                return NULL;
            }

            return g_strdup_printf ("%d", ival);

        default:
            ms_context_set_error (ctx, MS_ERROR_VALUE, "invalid format");
            return NULL;
    }
}


static MSValue *
func_format (MSValue *format, MSValue *tuple, MSContext *ctx)
{
    GString *ret;
    guint n_items, items_written;
    char *str, *p, *s;
    MSValue *val;

    if (format->type != MS_VALUE_STRING)
        return ms_context_set_error (ctx, MS_ERROR_TYPE, "invalid '%'");

    if (tuple->type == MS_VALUE_LIST)
        n_items = tuple->list.n_elms;
    else
        n_items = 1;

    p = str = format->str;
    ret = g_string_new (NULL);
    items_written = 0;

    while (*p)
    {
        if (*p == '%')
        {
            if (p > str)
                g_string_append_len (ret, str, p - str);

            switch (p[1])
            {
                case '%':
                    g_string_append_c (ret, '%');
                    p += 2;
                    str = p;
                    break;

                case 's':
                case 'd':
                    if (items_written == n_items)
                    {
                        ms_context_set_error (ctx, MS_ERROR_VALUE,
                                              "invalid conversion");
                        g_string_free (ret, TRUE);
                        return NULL;
                    }

                    if (tuple->type == MS_VALUE_LIST)
                        val = tuple->list.elms[items_written];
                    else
                        val = tuple;

                    s = format_value (p[1], val, ctx);

                    if (!s)
                    {
                        g_string_free (ret, TRUE);
                        return NULL;
                    }

                    g_string_append (ret, s);
                    g_free (s);
                    items_written++;
                    p += 2;
                    str = p;
                    break;

                default:
                    ms_context_set_error (ctx, MS_ERROR_VALUE,
                                          "invalid conversion");
                    g_string_free (ret, TRUE);
                    return NULL;
            }
        }
        else
        {
            ++p;
        }
    }

    if (str < p)
        g_string_append (ret, str);

    return ms_value_take_string (g_string_free (ret, FALSE));
}


gpointer
ms_binary_op_cfunc (MSBinaryOp op)
{
    static gpointer funcs[MS_BINARY_OP_LAST] = {
        func_plus, func_minus, func_mult, func_div,
        func_and, func_or,
        func_eq, func_neq, func_lt, func_gt, func_le, func_ge,
        func_format
    };

    g_return_val_if_fail (op < MS_BINARY_OP_LAST, NULL);
    return funcs[op];
}


static MSValue *
func_uminus (MSValue    *val,
             MSContext  *ctx)
{
    if (val->type == MS_VALUE_INT)
        return ms_value_int (-val->ival);
    return ms_context_set_error (ctx, MS_ERROR_TYPE, NULL);
}


static MSValue *
func_not (MSValue *val)
{
    return !ms_value_get_bool (val) ?
            ms_value_true () : ms_value_false ();
}


static MSValue *
func_len (MSValue    *val,
          MSContext  *ctx)
{
    switch (val->type)
    {
        case MS_VALUE_STRING:
            return ms_value_int (strlen (val->str));
        case MS_VALUE_LIST:
            return ms_value_int (val->list.n_elms);
        default:
            return ms_context_set_error (ctx, MS_ERROR_TYPE, NULL);
    }
}


gpointer
ms_unary_op_cfunc (MSUnaryOp op)
{
    static gpointer funcs[MS_UNARY_OP_LAST] = {
        func_uminus, func_not, func_len
    };

    g_return_val_if_fail (op < MS_UNARY_OP_LAST, NULL);
    return funcs[op];
}
