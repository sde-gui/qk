/*
 *   mooutils/moowin.h
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

#ifndef MOOUTILS_MOOWIN_H
#define MOOUTILS_MOOWIN_H

#include <gtk/gtkwindow.h>

G_BEGIN_DECLS


gboolean     moo_window_is_hidden           (GtkWindow  *window);
GtkWindow   *moo_get_top_window             (GSList     *windows);

gboolean     moo_window_set_icon_from_stock (GtkWindow  *window,
                                             const char *stock_id);


G_END_DECLS

#endif /* MOOUTILS_MOOWIN_H */
