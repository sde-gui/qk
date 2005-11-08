/*
 *   mooapp/moopythonconsole.h
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

#ifndef GGAPPYCONSOLE_H
#define GGAPPYCONSOLE_H

#include <gtk/gtktextview.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkwindow.h>

G_BEGIN_DECLS


#define MOO_TYPE_PYTHON_CONSOLE              (moo_python_console_get_type ())
#define MOO_PYTHON_CONSOLE(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_PYTHON_CONSOLE, MooPythonConsole))
#define MOO_PYTHON_CONSOLE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_PYTHON_CONSOLE, MooPythonConsoleClass))
#define MOO_IS_PYTHON_CONSOLE(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_PYTHON_CONSOLE))
#define MOO_IS_PYTHON_CONSOLE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_PYTHON_CONSOLE))
#define MOO_PYTHON_CONSOLE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_PYTHON_CONSOLE, MooPythonConsoleClass))


typedef struct _MooPythonConsole        MooPythonConsole;
typedef struct _MooPythonConsoleClass   MooPythonConsoleClass;

struct _MooPythonConsole
{
    GtkWindow        parent;

    gboolean         redirect_output;
    GtkWidget       *entry;
    GtkTextTag      *in_tag;
    GtkTextTag      *out_tag;
    GtkTextTag      *err_tag;
    GtkTextBuffer   *buf;
    GtkTextView     *textview;
    GtkTextMark     *end;
    GQueue          *history;
    guint            current;
    struct _MooPython *python;
};

struct _MooPythonConsoleClass
{
    GtkWindowClass parent_class;
};

GType               moo_python_console_get_type (void) G_GNUC_CONST;
MooPythonConsole   *moo_python_console_new      (struct _MooPython *python,
                                                 gboolean           redirect_output);


G_END_DECLS

#endif /* GGAPPYCONSOLE_H */
