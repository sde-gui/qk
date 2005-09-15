/*
 *   mooui/moowindow.h
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

#ifndef MOOUI_MOOWINDOW_H
#define MOOUI_MOOWINDOW_H

#include "mooui/mooactiongroup.h"
#include <gtk/gtkwindow.h>

G_BEGIN_DECLS


#define MOO_TYPE_WINDOW              (moo_window_get_type ())
#define MOO_WINDOW(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_WINDOW, MooWindow))
#define MOO_WINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_WINDOW, MooWindowClass))
#define MOO_IS_WINDOW(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_WINDOW))
#define MOO_IS_WINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_WINDOW))
#define MOO_WINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_WINDOW, MooWindowClass))


typedef struct _MooWindow        MooWindow;
typedef struct _MooWindowPrivate MooWindowPrivate;
typedef struct _MooWindowClass   MooWindowClass;

struct _MooWindow
{
    GtkWindow            gtkwindow;

    GtkAccelGroup       *accel_group;
    MooWindowPrivate    *priv;

    GtkWidget           *menubar;
    GtkWidget           *toolbar;
    GtkWidget           *vbox;
    GtkWidget           *statusbar;
};

struct _MooWindowClass
{
    GtkWindowClass      parent_class;

    /* signals */
    gboolean (*close)   (MooWindow *window);
};


GType       moo_window_get_type             (void) G_GNUC_CONST;

void        moo_window_update_ui            (MooWindow      *window);

gboolean    moo_window_close                (MooWindow      *window);


G_END_DECLS

#endif /* MOOUI_MOOWINDOW_H */
