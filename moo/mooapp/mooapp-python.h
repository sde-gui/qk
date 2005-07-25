/*
 *   mooapp/mooapp-python.h
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

#ifndef MOOAPP_MOOAPP_PYTHON_H
#define MOOAPP_MOOAPP_PYTHON_H

#include "mooapp/mooapp.h"

G_BEGIN_DECLS


void        moo_app_python_execute_file (GtkWindow  *parent);
gboolean    moo_app_python_run_string   (MooApp     *app,
                                         const char *string);
gboolean    moo_app_python_run_file     (MooApp     *app,
                                         const char *filename);

GtkWidget  *moo_app_get_python_console  (MooApp     *app);
void        moo_app_show_python_console (MooApp     *app);
void        moo_app_hide_python_console (MooApp     *app);


G_END_DECLS

#endif /* MOOAPP_MOOAPP_PYTHON_H */
