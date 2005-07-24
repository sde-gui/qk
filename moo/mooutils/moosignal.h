/*
 *   mooutils/moosignal.h
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

#ifndef MOOUTILS_MOOSIGNAL_H
#define MOOUTILS_MOOSIGNAL_H

#include <glib-object.h>

G_BEGIN_DECLS


guint moo_signal_new_cb (const gchar        *signal_name,
                         GType               itype,
                         GSignalFlags        signal_flags,
                         GCallback           handler,
                         GSignalAccumulator  accumulator,
                         gpointer            accu_data,
                         GSignalCMarshaller  c_marshaller,
                         GType               return_type,
                         guint               n_params,
                         ...);


G_END_DECLS

#endif /* MOOUTILS_MOOSIGNAL_H */
