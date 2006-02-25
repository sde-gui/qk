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


static ASValue *AS_None;
static ASValue *AS_True;
static ASValue *AS_False;


static ASValue *
as_value_new (ASValueType type)
{
    ASValue *val = g_new0 (ASValue, 1);
    val->ref_count = 1;
    val->type = type;
    return val;
}


ASValue *
as_value_none (void)
{
    if (!AS_None)
        return AS_None = as_value_new (AS_VALUE_NONE);
    else
        return as_value_ref (AS_None);
}


ASValue *
as_value_false (void)
{
    if (!AS_False)
        return AS_False = as_value_int (FALSE);
    else
        return as_value_ref (AS_False);
}


ASValue *
as_value_true (void)
{
    if (!AS_True)
        return AS_True = as_value_int (TRUE);
    else
        return as_value_ref (AS_True);
}


ASValue *
as_value_bool (gboolean val)
{
    return val ? as_value_true () : as_value_false ();
}


ASValue *
as_value_int (int ival)
{
    ASValue *val = as_value_new (AS_VALUE_INT);
    val->ival = ival;
    return val;
}


ASValue *
as_value_string (const char *string)
{
    return as_value_take_string (g_strdup (string));
}


ASValue *
as_value_take_string (char *string)
{
    ASValue *val;
    g_return_val_if_fail (string != NULL, NULL);
    val = as_value_new (AS_VALUE_STRING);
    val->str = string;
    return val;
}


ASValue *
as_value_object (gpointer object)
{
    ASValue *val;
    g_return_val_if_fail (G_IS_OBJECT (object), NULL);
    val = as_value_new (AS_VALUE_OBJECT);
    val->ptr = g_object_ref (object);
    return val;
}


ASValue *
as_value_gvalue (GValue *gval)
{
    ASValue *val;
    g_return_val_if_fail (G_IS_VALUE (gval), NULL);
    val = as_value_new (AS_VALUE_GVALUE);

    val->gval = g_new (GValue, 1);
    val->gval->g_type = 0;
    g_value_init (val->gval, G_VALUE_TYPE (gval));
    g_value_copy (gval, val->gval);

    return val;
}


ASValue *
as_value_list (guint n_elms)
{
    ASValue *val;
    guint i;

    val = as_value_new (AS_VALUE_LIST);
    val->list.elms = g_new (ASValue*, n_elms);
    val->list.n_elms = n_elms;

    for (i = 0; i < n_elms; ++i)
        val->list.elms[i] = as_value_none ();

    return val;
}


void
as_value_list_set_elm (ASValue    *list,
                       guint       index,
                       ASValue    *elm)
{
    g_return_if_fail (list != NULL);
    g_return_if_fail (elm != NULL);
    g_return_if_fail (list->type == AS_VALUE_LIST);
    g_return_if_fail (index < list->list.n_elms);

    if (list->list.elms[index] != elm)
    {
        as_value_unref (list->list.elms[index]);
        list->list.elms[index] = as_value_ref (elm);
    }
}


ASValue *
as_value_ref (ASValue *val)
{
    g_return_val_if_fail (val != NULL, NULL);
    val->ref_count++;
    return val;
}


void
as_value_unref (ASValue *val)
{
    guint i;

    g_return_if_fail (val != NULL);

    if (--val->ref_count)
        return;

    switch (val->type)
    {
        case AS_VALUE_STRING:
            g_free (val->str);
            break;

        case AS_VALUE_NONE:
            if (val == AS_None)
                AS_None = NULL;
            break;

        case AS_VALUE_INT:
            if (val == AS_False)
                AS_False = NULL;
            else if (val == AS_True)
                AS_True = NULL;
            break;

        case AS_VALUE_OBJECT:
            g_object_unref (val->ptr);
            break;

        case AS_VALUE_GVALUE:
            g_value_unset (val->gval);
            g_free (val->gval);
            break;

        case AS_VALUE_LIST:
            for (i = 0; i < val->list.n_elms; ++i)
                as_value_unref (val->list.elms[i]);
            g_free (val->list.elms);
            break;
    }

    g_free (val);
}


const char *
as_binary_op_name (ASBinaryOp op)
{
    static const char *names[AS_BINARY_OP_LAST] = {
        "@PLUS", "@MINUS", "@MULT", "@DIV", "@AND", "@OR",
        "@EQ", "@NEQ", "@LT", "@GT", "@LE", "@GE", "@FORMAT"
    };

    g_return_val_if_fail (op < AS_BINARY_OP_LAST, NULL);
    return names[op];
}


const char *
as_unary_op_name (ASUnaryOp op)
{
    static const char *names[AS_UNARY_OP_LAST] = {
        "@UMINUS", "@NOT", "@LEN"
    };

    g_return_val_if_fail (op < AS_UNARY_OP_LAST, NULL);
    return names[op];
}


gboolean
as_value_get_bool (ASValue *val)
{
    g_return_val_if_fail (val != NULL, FALSE);

    switch (val->type)
    {
        case AS_VALUE_STRING:
            return val->str[0] != 0;
        case AS_VALUE_INT:
            return val->ival != 0;
        case AS_VALUE_NONE:
            return FALSE;
        case AS_VALUE_OBJECT:
            return val->ptr != NULL;
        case AS_VALUE_LIST:
            return val->list.n_elms != 0;
        case AS_VALUE_GVALUE:
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
as_value_get_int (ASValue    *val,
                  int        *ival)
{
    g_return_val_if_fail (val != NULL, FALSE);
    g_return_val_if_fail (ival != NULL, FALSE);

    switch (val->type)
    {
        case AS_VALUE_INT:
            *ival = val->ival;
            return TRUE;

        case AS_VALUE_NONE:
            *ival = 0;
            return TRUE;

        case AS_VALUE_GVALUE:
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
print_list (ASValue **elms,
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
        s = as_value_print (elms[i]);
        g_string_append (string, s);
        g_free (s);
    }

    g_string_append_c (string, ']');
    return g_string_free (string, FALSE);
}


char *
as_value_print (ASValue *val)
{
    g_return_val_if_fail (val != NULL, NULL);

    switch (val->type)
    {
        case AS_VALUE_STRING:
            return g_strdup (val->str);
        case AS_VALUE_INT:
            return g_strdup_printf ("%d", val->ival);
        case AS_VALUE_NONE:
            return g_strdup ("None");
        case AS_VALUE_OBJECT:
            return g_strdup_printf ("Object %p", val->ptr);
        case AS_VALUE_LIST:
            return print_list (val->list.elms, val->list.n_elms);
        case AS_VALUE_GVALUE:
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


static ASValue *
func_plus (ASValue *a, ASValue *b, ASContext *ctx)
{
    if (a->type == AS_VALUE_INT && b->type == AS_VALUE_INT)
        return as_value_int (a->ival + b->ival);
    else if (a->type == AS_VALUE_STRING && b->type == AS_VALUE_STRING)
        return as_value_take_string (g_strdup_printf ("%s%s", a->str, b->str));

    return as_context_set_error (ctx, AS_ERROR_TYPE,
                                 "invalid PLUS");
}


static ASValue *
func_minus (ASValue *a, ASValue *b, ASContext *ctx)
{
    if (a->type == AS_VALUE_INT && b->type == AS_VALUE_INT)
        return as_value_int (a->ival - b->ival);
    return as_context_set_error (ctx, AS_ERROR_TYPE,
                                 "invalid MINUS");
}


static ASValue *
func_mult (ASValue *a, ASValue *b, ASContext *ctx)
{
    if (a->type == AS_VALUE_INT && b->type == AS_VALUE_INT)
        return as_value_int (a->ival * b->ival);

    if (a->type == AS_VALUE_STRING && b->type == AS_VALUE_INT)
    {
        char *s;
        guint len;
        int i;

        if (b->ival < 0)
            return as_context_set_error (ctx, AS_ERROR_TYPE,
                                         "string * negative int");
        if (b->ival == 0)
            return as_value_string ("");

        len = strlen (a->str);
        s = g_new (char, len * b->ival + 1);
        s[len * b->ival] = 0;

        for (i = 0; i < b->ival; ++i)
            memcpy (&s[i*len], a->str, len);

        return as_value_take_string (s);
    }

    return as_context_set_error (ctx, AS_ERROR_TYPE,
                                 "invalid MULT");
}


static ASValue *
func_div (ASValue *a, ASValue *b, ASContext *ctx)
{
    if (a->type == AS_VALUE_INT && b->type == AS_VALUE_INT)
    {
        if (b->ival)
            return as_value_int (a->ival / b->ival);
        else
            return as_context_set_error (ctx, AS_ERROR_VALUE,
                                         "division by zero");
    }

    return as_context_set_error (ctx, AS_ERROR_TYPE,
                                 "invalid DIV");
}


static ASValue *
func_and (ASValue *a, ASValue *b)
{
    if (as_value_get_bool (a) && as_value_get_bool (b))
        return as_value_ref (b);
    else
        return as_value_false ();
}


static ASValue *
func_or (ASValue *a, ASValue *b)
{
    if (as_value_get_bool (a))
        return as_value_ref (a);
    else if (as_value_get_bool (b))
        return as_value_ref (b);
    else
        return as_value_false ();
}


static gboolean
list_equal (ASValue *a, ASValue *b)
{
    guint i;

    if (a->list.n_elms != b->list.n_elms)
        return FALSE;

    for (i = 0; i < a->list.n_elms; ++i)
        if (!as_value_equal (a->list.elms[i], b->list.elms[i]))
            return FALSE;

    return TRUE;
}


gboolean
as_value_equal (ASValue *a, ASValue *b)
{
    if (a == b)
        return TRUE;

    if (a->type != b->type)
        return FALSE;

    switch (a->type)
    {
        case AS_VALUE_INT:
            return a->ival == b->ival;
        case AS_VALUE_NONE:
            return TRUE;
        case AS_VALUE_STRING:
            return !strcmp (a->str, b->str);
        case AS_VALUE_OBJECT:
            return a->ptr == b->ptr;
        case AS_VALUE_LIST:
            return list_equal (a, b);
        case AS_VALUE_GVALUE:
            g_return_val_if_reached (FALSE);
    }

    g_return_val_if_reached (FALSE);
}


#define CMP(a, b) ((a) < (b) ? -1 : ((a) > (b) ? 1 : 0))

static int
list_cmp (ASValue *a, ASValue *b)
{
    guint i;

    for (i = 0; i < a->list.n_elms && i < b->list.n_elms; ++i)
    {
        int c = as_value_cmp (a->list.elms[i], b->list.elms[i]);

        if (c)
            return c;
    }

    return CMP (a->list.n_elms, b->list.n_elms);
}


int
as_value_cmp (ASValue *a, ASValue *b)
{
    if (a == b)
        return 0;

    if (a->type != b->type)
        return a < b ? -1 : 1;

    switch (a->type)
    {
        case AS_VALUE_INT:
            return CMP (a->ival, b->ival);
        case AS_VALUE_NONE:
            return 0;
        case AS_VALUE_STRING:
            return strcmp (a->str, b->str);
        case AS_VALUE_OBJECT:
            return CMP (a->ptr, b->ptr);
        case AS_VALUE_LIST:
            return list_cmp (a, b);
        case AS_VALUE_GVALUE:
            g_return_val_if_reached (CMP (a, b));
    }

    g_return_val_if_reached (CMP (a, b));
}


static ASValue *
func_eq (ASValue *a, ASValue *b)
{
    return as_value_bool (as_value_equal (a, b));
}


static ASValue *
func_neq (ASValue *a, ASValue *b)
{
    return as_value_bool (!as_value_equal (a, b));
}


static ASValue *
func_lt (ASValue *a, ASValue *b)
{
    return as_value_bool (as_value_cmp (a, b) < 0);
}

static ASValue *
func_gt (ASValue *a, ASValue *b)
{
    return as_value_bool (as_value_cmp (a, b) > 0);
}

static ASValue *
func_le (ASValue *a, ASValue *b)
{
    return as_value_bool (as_value_cmp (a, b) <= 0);
}

static ASValue *
func_ge (ASValue *a, ASValue *b)
{
    return as_value_bool (as_value_cmp (a, b) >= 0);
}


static char *
format_value (char       format,
              ASValue   *value,
              ASContext *ctx)
{
    int ival;

    switch (format)
    {
        case 's':
            return as_value_print (value);

        case 'd':
            if (!as_value_get_int (value, &ival))
            {
                as_context_set_error (ctx, AS_ERROR_TYPE, NULL);
                return NULL;
            }

            return g_strdup_printf ("%d", ival);

        default:
            as_context_set_error (ctx, AS_ERROR_VALUE, "invalid format");
            return NULL;
    }
}


static ASValue *
func_format (ASValue *format, ASValue *tuple, ASContext *ctx)
{
    GString *ret;
    guint n_items, items_written;
    char *str, *p, *s;
    ASValue *val;

    if (format->type != AS_VALUE_STRING)
        return as_context_set_error (ctx, AS_ERROR_TYPE, "invalid '%'");

    if (tuple->type == AS_VALUE_LIST)
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
                        as_context_set_error (ctx, AS_ERROR_VALUE,
                                              "invalid conversion");
                        g_string_free (ret, TRUE);
                        return NULL;
                    }

                    if (tuple->type == AS_VALUE_LIST)
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
                    as_context_set_error (ctx, AS_ERROR_VALUE,
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

    return as_value_take_string (g_string_free (ret, FALSE));
}


gpointer
as_binary_op_cfunc (ASBinaryOp op)
{
    static gpointer funcs[AS_BINARY_OP_LAST] = {
        func_plus, func_minus, func_mult, func_div,
        func_and, func_or,
        func_eq, func_neq, func_lt, func_gt, func_le, func_ge,
        func_format
    };

    g_return_val_if_fail (op < AS_BINARY_OP_LAST, NULL);
    return funcs[op];
}


static ASValue *
func_uminus (ASValue    *val,
             ASContext  *ctx)
{
    if (val->type == AS_VALUE_INT)
        return as_value_int (-val->ival);
    return as_context_set_error (ctx, AS_ERROR_TYPE, NULL);
}


static ASValue *
func_not (ASValue *val)
{
    return !as_value_get_bool (val) ?
            as_value_true () : as_value_false ();
}


static ASValue *
func_len (ASValue    *val,
          ASContext  *ctx)
{
    switch (val->type)
    {
        case AS_VALUE_STRING:
            return as_value_int (strlen (val->str));
        case AS_VALUE_LIST:
            return as_value_int (val->list.n_elms);
        default:
            return as_context_set_error (ctx, AS_ERROR_TYPE, NULL);
    }
}


gpointer
as_unary_op_cfunc (ASUnaryOp op)
{
    static gpointer funcs[AS_UNARY_OP_LAST] = {
        func_uminus, func_not, func_len
    };

    g_return_val_if_fail (op < AS_UNARY_OP_LAST, NULL);
    return funcs[op];
}
