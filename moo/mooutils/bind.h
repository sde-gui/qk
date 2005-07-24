/*
 *   mooutils/bind.h
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

#ifndef MOOUTILS_BIND_H
#define MOOUTILS_BIND_H

#include <gtk/gtktogglebutton.h>
#include <gtk/gtkentry.h>

G_BEGIN_DECLS


typedef const char  *(*MooQueryTextFunc)        (gpointer data, GtkWidget *button);
typedef char        *(*MooTransformTextFunc)    (const char *text, gpointer data);

char       *moo_quote_text      (const char *text, gpointer data);

void        moo_bind_button     (GtkButton                  *button,
                                 GtkEntry                   *entry,
                                 MooQueryTextFunc            func,
                                 MooTransformTextFunc        text_func,
                                 gpointer                    data);

void        moo_bind_sensitive  (GtkToggleButton    *btn,
                                 GtkWidget         **dependent,
                                 int                 num_dependent,
                                 gboolean            invert);


G_END_DECLS

#endif /* MOOUTILS_BIND_H */
