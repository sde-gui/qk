/*
 *   tests/mterm-app.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef __MOO_TERM_APP_H__
#define __MOO_TERM_APP_H__

#include <mooapp/mooapp.h>
#include <mooterm/mootermwindow.h>
#include <gtk/gtk.h>


#define MOO_TYPE_TERM_APP                (moo_term_app_get_type ())
#define MOO_TERM_APP(object)             (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_TERM_APP, MooTermApp))
#define MOO_TERM_APP_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_TERM_APP, MooTermAppClass))
#define MOO_IS_TERM_APP(object)          (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_TERM_APP))
#define MOO_IS_TERM_APP_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_TERM_APP))
#define MOO_TERM_APP_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_TERM_APP, MooTermAppClass))


typedef struct _MooTermApp      MooTermApp;
typedef struct _MooTermAppClass MooTermAppClass;

struct _MooTermApp
{
    MooApp parent;
    MooTermWindow *window;
    MooTerm *term;
};

struct _MooTermAppClass
{
    MooAppClass parent_class;
};


GType  moo_term_app_get_type (void) G_GNUC_CONST;


#endif /* __MOO_TERM_APP_H__ */
