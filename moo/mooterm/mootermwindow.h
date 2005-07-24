/*
 *   mooterm/mootermwindow.h
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

#ifndef MOOTERM_MOOTERMWINDOW_H
#define MOOTERM_MOOTERMWINDOW_H

#include "mooui/moowindow.h"
#include "mooterm/mooterm.h"

G_BEGIN_DECLS


#define MOO_TYPE_TERM_WINDOW             (moo_term_window_get_type ())
#define MOO_TERM_WINDOW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_TERM_WINDOW, MooTermWindow))
#define MOO_TERM_WINDOW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_TERM_WINDOW, MooTermWindowClass))
#define MOO_IS_TERM_WINDOW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_TERM_WINDOW))
#define MOO_IS_TERM_WINDOW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_TERM_WINDOW))
#define MOO_TERM_WINDOW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_TERM_WINDOW, MooTermWindowClass))


typedef struct _MooTermWindow        MooTermWindow;
typedef struct _MooTermWindowClass   MooTermWindowClass;


struct _MooTermWindow
{
    MooWindow       parent;

    MooTerm        *terminal;
};

struct _MooTermWindowClass
{
    MooWindowClass parent_class;
};


GType            moo_term_window_get_type       (void) G_GNUC_CONST;
GtkWidget       *moo_term_window_new            (void);

void             moo_term_window_apply_settings (MooTermWindow  *window);

MooTerm         *moo_term_window_get_term       (MooTermWindow  *window);


G_END_DECLS

#endif /* MOOTERM_MOOTERMWINDOW_H */
