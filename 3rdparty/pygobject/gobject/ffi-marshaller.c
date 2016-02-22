/* -*- Mode: C; c-basic-offset: 4 -*-
 * pygtk- Python bindings for the GTK toolkit.
 * Copyright (C) 2007 Johan Dahlin
 *
 *   ffi-marshaller: Generic CMarshaller using libffi
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#include <glib-object.h>
#include <ffi.h>

static ffi_type *
g_value_to_ffi_type (const GValue *gvalue, gpointer *value)
{
  ffi_type *rettype = NULL;
  GType type = g_type_fundamental (G_VALUE_TYPE (gvalue));
  g_assert (type != G_TYPE_INVALID);

  switch (type) {
  case G_TYPE_BOOLEAN:
  case G_TYPE_CHAR:
  case G_TYPE_INT:
    rettype = &ffi_type_sint;
    *value = (gpointer)&(gvalue->data[0].v_int);
    break;
  case G_TYPE_UCHAR:
  case G_TYPE_UINT:
    rettype = &ffi_type_uint;
    *value = (gpointer)&(gvalue->data[0].v_uint);
    break;
  case G_TYPE_STRING:
  case G_TYPE_OBJECT:
  case G_TYPE_BOXED:
  case G_TYPE_POINTER:
    rettype = &ffi_type_pointer;
    *value = (gpointer)&(gvalue->data[0].v_pointer);
    break;
  case G_TYPE_FLOAT:
    rettype = &ffi_type_float;
    *value = (gpointer)&(gvalue->data[0].v_float);
    break;
  case G_TYPE_DOUBLE:
    rettype = &ffi_type_double;
    *value = (gpointer)&(gvalue->data[0].v_double);
    break;
  case G_TYPE_LONG:
    rettype = &ffi_type_slong;
    *value = (gpointer)&(gvalue->data[0].v_long);
    break;
  case G_TYPE_ULONG:
    rettype = &ffi_type_ulong;
    *value = (gpointer)&(gvalue->data[0].v_ulong);
    break;
  case G_TYPE_INT64:
    rettype = &ffi_type_sint64;
    *value = (gpointer)&(gvalue->data[0].v_int64);
    break;
  case G_TYPE_UINT64:
    rettype = &ffi_type_uint64;
    *value = (gpointer)&(gvalue->data[0].v_uint64);
    break;
  default:
    rettype = &ffi_type_pointer;
    *value = NULL;
    g_warning ("Unsupported fundamental type: %s", g_type_name (type));
    break;
  }
  return rettype;
}

static void
g_value_from_ffi_type (GValue *gvalue, gpointer *value)
{
  switch (g_type_fundamental (G_VALUE_TYPE (gvalue))) {
  case G_TYPE_INT:
      g_value_set_int (gvalue, *(gint*)value);
      break;
  case G_TYPE_FLOAT:
      g_value_set_float (gvalue, *(gfloat*)value);
      break;
  case G_TYPE_DOUBLE:
      g_value_set_double (gvalue, *(gdouble*)value);
      break;
  case G_TYPE_BOOLEAN:
      g_value_set_boolean (gvalue, *(gboolean*)value);
      break;
  case G_TYPE_STRING:
      g_value_set_string (gvalue, *(gchar**)value);
      break;
  case G_TYPE_CHAR:
      g_value_set_char (gvalue, *(gchar*)value);
      break;
  case G_TYPE_UCHAR:
      g_value_set_uchar (gvalue, *(guchar*)value);
      break;
  case G_TYPE_UINT:
      g_value_set_uint (gvalue, *(guint*)value);
      break;
  case G_TYPE_POINTER:
      g_value_set_pointer (gvalue, *(gpointer*)value);
      break;
  case G_TYPE_LONG:
      g_value_set_long (gvalue, *(glong*)value);
      break;
  case G_TYPE_ULONG:
      g_value_set_ulong (gvalue, *(gulong*)value);
      break;
  case G_TYPE_INT64:
      g_value_set_int64 (gvalue, *(gint64*)value);
      break;
  case G_TYPE_UINT64:
      g_value_set_uint64 (gvalue, *(guint64*)value);
      break;
  case G_TYPE_BOXED:
      g_value_set_boxed (gvalue, *(gpointer*)value);
      break;
  default:
    g_warning ("Unsupported fundamental type: %s",
	       g_type_name (g_type_fundamental (G_VALUE_TYPE (gvalue))));
  }

}

void
g_cclosure_marshal_generic_ffi (GClosure *closure,
				GValue *return_gvalue,
				guint n_param_values,
				const GValue *param_values,
				gpointer invocation_hint,
				gpointer marshal_data)
{
  ffi_type *rtype;
  void *rvalue;
  int n_args;
  ffi_type **atypes;
  void **args;
  int i;
  ffi_cif cif;
  GCClosure *cc = (GCClosure*) closure;

  if (return_gvalue && G_VALUE_TYPE (return_gvalue)) 
    {
      rtype = g_value_to_ffi_type (return_gvalue, &rvalue);
    }
  else 
    {
      rtype = &ffi_type_void;
    }

  rvalue = g_alloca (MAX (rtype->size, sizeof (ffi_arg)));
  
  n_args = n_param_values + 1;
  atypes = g_alloca (sizeof (ffi_type *) * n_args);
  args =  g_alloca (sizeof (gpointer) * n_args);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      atypes[n_args-1] = g_value_to_ffi_type (param_values + 0,  
					      &args[n_args-1]);
      atypes[0] = &ffi_type_pointer;
      args[0] = &closure->data;
    }
  else
    {
      atypes[0] = g_value_to_ffi_type (param_values + 0, &args[0]);
      atypes[n_args-1] = &ffi_type_pointer;
      args[n_args-1] = &closure->data;
    }

  for (i = 1; i < n_args - 1; i++)
    atypes[i] = g_value_to_ffi_type (param_values + i, &args[i]);

  if (ffi_prep_cif (&cif, FFI_DEFAULT_ABI, n_args, rtype, atypes) != FFI_OK)
    return;

  ffi_call (&cif, marshal_data ? marshal_data : cc->callback, rvalue, args);

  if (return_gvalue && G_VALUE_TYPE (return_gvalue))
    g_value_from_ffi_type (return_gvalue, rvalue);
}
