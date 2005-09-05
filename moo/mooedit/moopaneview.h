/*
 *   moopaneview.h
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

#ifndef __MOO_PANE_VIEW__
#define __MOO_PANE_VIEW__

#include <gtksourceview/gtksourceview.h>

G_BEGIN_DECLS


#define MOO_TYPE_PANE_VIEW              (moo_pane_view_get_type ())
#define MOO_PANE_VIEW(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_PANE_VIEW, MooPaneView))
#define MOO_PANE_VIEW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_PANE_VIEW, MooPaneViewClass))
#define MOO_IS_PANE_VIEW(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_PANE_VIEW))
#define MOO_IS_PANE_VIEW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_PANE_VIEW))
#define MOO_PANE_VIEW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_PANE_VIEW, MooPaneViewClass))


typedef struct _MooPaneView         MooPaneView;
typedef struct _MooPaneViewClass    MooPaneViewClass;

struct _MooPaneView
{
    GtkSourceView  parent;
};

struct _MooPaneViewClass
{
    GtkSourceViewClass parent_class;
};


GType       moo_pane_view_get_type               (void) G_GNUC_CONST;

GtkWidget  *moo_pane_view_new                    (void);


G_END_DECLS

#endif /* __MOO_PANE_VIEW__ */
