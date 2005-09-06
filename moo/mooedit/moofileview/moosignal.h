/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   moosignal.h
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

#ifndef __MOO_SIGNAL_H__
#define __MOO_SIGNAL_H__

#include <glib-object.h>


static void void_marshal (G_GNUC_UNUSED GClosure *closure,
                          G_GNUC_UNUSED GValue *return_value,
                          G_GNUC_UNUSED guint n_param_values,
                          G_GNUC_UNUSED const GValue *param_values,
                          G_GNUC_UNUSED gpointer invocation_hint,
                          G_GNUC_UNUSED gpointer marshal_data)
{
}


static GClosure *void_closure_new (void)
{
    GClosure *closure = g_closure_new_simple (sizeof (GClosure), NULL);
    g_closure_set_marshal (closure, void_marshal);
    return closure;
}


static guint moo_signal_new_cb (const gchar        *signal_name,
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


#endif /* __MOO_SIGNAL_H__ */
