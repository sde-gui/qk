/*
 *   mooobjcmarshal.m
 *
 *   Copyright (C) 2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#import <config.h>
#import "mooobjcmarshal.h"
#import "moocobject-private.h"
#import <objc/objc-api.h>
#import <ffi.h>

#define MAX_N_FREELIST 16

#if SIZEOF_BOOL == 1
#define ffi_type_objc_bool ffi_type_uchar
#elif SIZEOF_BOOL == 2
#define ffi_type_objc_bool ffi_type_uint16
#elif SIZEOF_BOOL == 4
#define ffi_type_objc_bool ffi_type_uint32
#else
#error "Something is wrong with the BOOL type"
#endif

static gboolean
get_ffi_type (const GValue       *cgvalue,
              ffi_type          **arg_type,
              gpointer           *arg_val,
              gpointer           *freelist,
              guint              *n_freelist)
{
    BOOL *bval;
    GValue *gvalue = (GValue*) cgvalue;

    switch (g_type_fundamental (G_VALUE_TYPE (gvalue)))
    {
        case G_TYPE_BOOLEAN:
            if (*n_freelist == MAX_N_FREELIST)
            {
                g_warning ("%s: too many argumnents pushed", G_STRLOC);
                return FALSE;
            }
            bval = g_new (BOOL, 1);
            *bval = g_value_get_boolean (gvalue) ? 1 : 0;
            freelist[(*n_freelist)++] = bval;
            *arg_type = &ffi_type_sint;
            *arg_val = bval;
            break;

        case G_TYPE_STRING:
        case G_TYPE_OBJECT:
        case G_TYPE_BOXED:
            *arg_type = &ffi_type_pointer;
            *arg_val = &gvalue->data[0].v_pointer;
            break;

        case G_TYPE_CHAR:
        case G_TYPE_INT:
        case G_TYPE_ENUM:
        case G_TYPE_FLAGS:
            *arg_type = &ffi_type_sint;
            *arg_val = &gvalue->data[0].v_int;
            break;
        case G_TYPE_UCHAR:
        case G_TYPE_UINT:
            *arg_type = &ffi_type_uint;
            *arg_val = &gvalue->data[0].v_uint;
            break;
        case G_TYPE_LONG:
            *arg_type = &ffi_type_slong;
            *arg_val = &gvalue->data[0].v_long;
            break;
        case G_TYPE_ULONG:
            *arg_type = &ffi_type_ulong;
            *arg_val = &gvalue->data[0].v_ulong;
            break;
        case G_TYPE_INT64:
            *arg_type = &ffi_type_sint64;
            *arg_val = &gvalue->data[0].v_int64;
            break;
        case G_TYPE_UINT64:
            *arg_type = &ffi_type_uint64;
            *arg_val = &gvalue->data[0].v_uint64;
            break;
        case G_TYPE_FLOAT:
            *arg_type = &ffi_type_float;
            *arg_val = &gvalue->data[0].v_float;
            break;
        case G_TYPE_DOUBLE:
            *arg_type = &ffi_type_double;
            *arg_val = &gvalue->data[0].v_double;
            break;
        case G_TYPE_POINTER:
            *arg_type = &ffi_type_pointer;
            *arg_val = &gvalue->data[0].v_pointer;
            break;

        case G_TYPE_PARAM:
        default:
            g_warning ("Unsupported argument type: %s",
                       g_type_name (G_VALUE_TYPE (gvalue)));
            return FALSE;
    }

    return TRUE;
}

static gboolean
get_ffi_type_for_return (GValue    *gvalue,
                         ffi_type **ret_type,
                         gpointer  *ret_val,
                         gpointer  *freelist,
                         guint     *n_freelist)
{
    BOOL *bval;

    switch (g_type_fundamental (G_VALUE_TYPE (gvalue)))
    {
        case G_TYPE_BOOLEAN:
            if (*n_freelist == MAX_N_FREELIST)
            {
                g_warning ("%s: too many argumnents pushed", G_STRLOC);
                return FALSE;
            }
            bval = g_new (BOOL, 1);
            *bval = 0;
            freelist[(*n_freelist)++] = bval;
            *ret_type = &ffi_type_sint;
            *ret_val = bval;
            break;

        case G_TYPE_STRING:
        case G_TYPE_OBJECT:
        case G_TYPE_BOXED:
            *ret_type = &ffi_type_pointer;
            *ret_val = &gvalue->data[0].v_pointer;
            break;

        case G_TYPE_CHAR:
        case G_TYPE_INT:
        case G_TYPE_ENUM:
        case G_TYPE_FLAGS:
            *ret_type = &ffi_type_sint;
            *ret_val = &gvalue->data[0].v_int;
            break;
        case G_TYPE_UCHAR:
        case G_TYPE_UINT:
            *ret_type = &ffi_type_uint;
            *ret_val = &gvalue->data[0].v_uint;
            break;
        case G_TYPE_LONG:
            *ret_type = &ffi_type_slong;
            *ret_val = &gvalue->data[0].v_long;
            break;
        case G_TYPE_ULONG:
            *ret_type = &ffi_type_ulong;
            *ret_val = &gvalue->data[0].v_ulong;
            break;
        case G_TYPE_INT64:
            *ret_type = &ffi_type_sint64;
            *ret_val = &gvalue->data[0].v_int64;
            break;
        case G_TYPE_UINT64:
            *ret_type = &ffi_type_uint64;
            *ret_val = &gvalue->data[0].v_uint64;
            break;
        case G_TYPE_FLOAT:
            *ret_type = &ffi_type_float;
            *ret_val = &gvalue->data[0].v_float;
            break;
        case G_TYPE_DOUBLE:
            *ret_type = &ffi_type_double;
            *ret_val = &gvalue->data[0].v_double;
            break;
        case G_TYPE_POINTER:
            *ret_type = &ffi_type_pointer;
            *ret_val = &gvalue->data[0].v_pointer;
            break;

        case G_TYPE_PARAM:
        default:
            g_warning ("Unsupported argument type: %s",
                       g_type_name (G_VALUE_TYPE (gvalue)));
            return FALSE;
    }

    return TRUE;
}

static void
convert_return_value (gpointer            ptr,
                      GValue             *gvalue)
{
    BOOL *bval;

    switch (g_type_fundamental (G_VALUE_TYPE (gvalue)))
    {
        case G_TYPE_BOOLEAN:
            bval = ptr;
            g_value_set_boolean (gvalue, *bval);
            break;

        case G_TYPE_STRING:
        case G_TYPE_OBJECT:
        case G_TYPE_BOXED:
        case G_TYPE_POINTER:
            break;

        case G_TYPE_CHAR:
        case G_TYPE_INT:
        case G_TYPE_ENUM:
        case G_TYPE_FLAGS:
        case G_TYPE_UCHAR:
        case G_TYPE_UINT:
        case G_TYPE_LONG:
        case G_TYPE_ULONG:
        case G_TYPE_INT64:
        case G_TYPE_UINT64:
        case G_TYPE_FLOAT:
        case G_TYPE_DOUBLE:
            break;

        case G_TYPE_PARAM:
        default:
            g_return_if_reached ();
    }
}

static void
do_marshal (GCallback          callback,
            id                 add_obj,
            SEL                add_sel,
            GValue            *return_value,
            guint              n_params,
            const GValue      *params,
            id                 data,
            gboolean           use_data,
            gboolean           swap)
{
    ffi_cif cif;
    ffi_type *ret_type, **arg_types;
    guint i_arg, i, n_args;
    gpointer ret_val, *arg_vals;
    gpointer freelist[MAX_N_FREELIST];
    guint n_freelist = 0;

    /* params: object arg1 ... argN */

    n_args = use_data ? n_params + 1 : n_params;

    if (add_obj)
        n_args += 2;

    arg_types = g_new (ffi_type*, n_args);
    arg_vals = g_new (gpointer, n_args);
    i_arg = 0;

    if (data)
        [data retain];
    if (add_obj)
        [add_obj retain];

/*
    if (use_data)
        if (swap)
            callback (data, arg1, ..., argN, object)
        else
            callback (object, arg1, ..., argN, data)
    else
        if (swap)
            callback (arg1, ..., argN, object)
        else
            callback (object, arg1, ..., argN)
*/

    /* Return value */
    if (return_value)
    {
        if (!get_ffi_type_for_return (return_value, &ret_type, &ret_val,
                                      freelist, &n_freelist))
            goto out;
    }
    else
    {
        ret_type = &ffi_type_void;
        ret_val = NULL;
    }

    if (add_obj)
    {
        arg_types[0] = &ffi_type_pointer;
        arg_vals[0] = &add_obj;
        arg_types[1] = &ffi_type_pointer;
        arg_vals[1] = &add_sel;
        i_arg = 2;
    }

    if (use_data && swap)
    {
        arg_types[i_arg] = &ffi_type_pointer;
        arg_vals[i_arg] = &data;
        i_arg++;
    }
    else if (!swap)
    {
        if (!get_ffi_type (&params[0], &arg_types[i_arg], &arg_vals[i_arg],
                           freelist, &n_freelist))
            goto out;
        i_arg++;
    }

    for (i = 1; i < n_params; ++i, ++i_arg)
    {
        if (!get_ffi_type (&params[i], &arg_types[i_arg], &arg_vals[i_arg],
                           freelist, &n_freelist))
            goto out;
    }

    if (use_data && !swap)
    {
        arg_types[i_arg] = &ffi_type_pointer;
        arg_vals[i_arg] = &data;
        i_arg++;
    }
    else if (swap)
    {
        if (!get_ffi_type (&params[0], &arg_types[i_arg], &arg_vals[i_arg],
                           freelist, &n_freelist))
            goto out;
        i_arg++;
    }

    g_assert (i_arg == n_args);

    if (ffi_prep_cif (&cif, FFI_DEFAULT_ABI, n_args, ret_type, arg_types) != FFI_OK)
        goto out;

    ffi_call (&cif, FFI_FN (callback), ret_val, arg_vals);

    if (return_value)
        convert_return_value (ret_val, return_value);

out:
    if (data)
        [data release];
    if (add_obj)
        [add_obj release];
    for (i = 0; i < n_freelist; ++i)
        g_free (freelist[i]);
    g_free (arg_types);
    g_free (arg_vals);
}

void
_moo_objc_marshal (GCallback          callback,
                   id                 add_obj,
                   SEL                add_sel,
                   GValue            *return_value,
                   guint              n_params,
                   const GValue      *params,
                   id                 data,
                   gboolean           use_data,
                   gboolean           swap)
{
    do_marshal (callback, add_obj, add_sel,
                return_value, n_params, params,
                data, use_data, swap);
}


/* -*- objc -*- */
