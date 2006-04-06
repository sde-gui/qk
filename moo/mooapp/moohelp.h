/*
 *   moohelp.h
 *
 *   Copyright (C) 2004-2006 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef __MOO_HELP_H__
#define __MOO_HELP_H__

#include <libgtkhtml/gtkhtml.h>

G_BEGIN_DECLS


#define MOO_TYPE_HELP              (moo_help_get_type ())
#define MOO_HELP(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_HELP, MooHelp))
#define MOO_HELP_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_HELP, MooHelpClass))
#define MOO_IS_HELP(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_HELP))
#define MOO_IS_HELP_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_HELP))
#define MOO_HELP_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_HELP, MooHelpClass))

typedef struct _MooHelp             MooHelp;
typedef struct _MooHelpPrivate      MooHelpPrivate;
typedef struct _MooHelpClass        MooHelpClass;

struct _MooHelp
{
    HtmlView parent;
};

struct _MooHelpClass
{
    HtmlViewClass parent_class;
};


GType           moo_help_get_type       (void) G_GNUC_CONST;
GtkWidget      *moo_help_new            (void);


G_END_DECLS

#endif /* __MOO_HELP_H__ */
