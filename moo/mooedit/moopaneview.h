/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
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

#include "mooedit/mootextview.h"

G_BEGIN_DECLS


#define MOO_TYPE_PANE_VIEW              (moo_pane_view_get_type ())
#define MOO_PANE_VIEW(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_PANE_VIEW, MooPaneView))
#define MOO_PANE_VIEW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_PANE_VIEW, MooPaneViewClass))
#define MOO_IS_PANE_VIEW(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_PANE_VIEW))
#define MOO_IS_PANE_VIEW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_PANE_VIEW))
#define MOO_PANE_VIEW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_PANE_VIEW, MooPaneViewClass))


typedef struct _MooPaneView         MooPaneView;
typedef struct _MooPaneViewPrivate  MooPaneViewPrivate;
typedef struct _MooPaneViewClass    MooPaneViewClass;

struct _MooPaneView
{
    MooTextView parent;
    MooPaneViewPrivate *priv;
};

struct _MooPaneViewClass
{
    MooTextViewClass parent_class;

    void     (*activate) (MooPaneView *view,
                          gpointer     line_data,
                          int          line);
};


GType       moo_pane_view_get_type      (void) G_GNUC_CONST;

GtkWidget  *moo_pane_view_new           (void);

gboolean    moo_pane_view_grab          (MooPaneView    *view,
                                         gpointer        user_id);
void        moo_pane_view_ungrab        (MooPaneView    *view,
                                         gpointer        user_id);

void        moo_pane_view_set_line_data (MooPaneView    *view,
                                         int             line,
                                         gpointer        data,
                                         GDestroyNotify  free_func);
gpointer    moo_pane_view_get_line_data (MooPaneView    *view,
                                         int             line);

GtkTextTag *moo_pane_view_create_tag    (MooPaneView    *view,
                                         const char     *tag_name,
                                         const char     *first_property_name,
                                         ...);
GtkTextTag *moo_pane_view_lookup_tag    (MooPaneView    *view,
                                         const char     *tag_name);

void        moo_pane_view_clear         (MooPaneView    *view);

int         moo_pane_view_start_line    (MooPaneView    *view);
void        moo_pane_view_write         (MooPaneView    *view,
                                         const char     *text,
                                         gssize          len,
                                         GtkTextTag     *tag);
void        moo_pane_view_end_line      (MooPaneView    *view);

int         moo_pane_view_write_line    (MooPaneView    *view,
                                         const char     *text,
                                         gssize          len,
                                         GtkTextTag     *tag);



G_END_DECLS

#endif /* __MOO_PANE_VIEW__ */
