/*
 *   moolineview.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef __MOO_LINE_VIEW__
#define __MOO_LINE_VIEW__

#include <mooedit/mootextview.h>

G_BEGIN_DECLS


#define MOO_TYPE_LINE_VIEW_DATA         (moo_line_view_data_get_type ())
#define MOO_TYPE_LINE_VIEW              (moo_line_view_get_type ())
#define MOO_LINE_VIEW(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_LINE_VIEW, MooLineView))
#define MOO_LINE_VIEW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_LINE_VIEW, MooLineViewClass))
#define MOO_IS_LINE_VIEW(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_LINE_VIEW))
#define MOO_IS_LINE_VIEW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_LINE_VIEW))
#define MOO_LINE_VIEW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_LINE_VIEW, MooLineViewClass))


typedef struct _MooLineView         MooLineView;
typedef struct _MooLineViewPrivate  MooLineViewPrivate;
typedef struct _MooLineViewClass    MooLineViewClass;

struct _MooLineView
{
    MooTextView parent;
    MooLineViewPrivate *priv;
};

struct _MooLineViewClass
{
    MooTextViewClass parent_class;

    void     (*activate) (MooLineView *view,
                          int          line);
};


GType       moo_line_view_get_type      (void) G_GNUC_CONST;
GType       moo_line_view_data_get_type (void) G_GNUC_CONST;

GtkWidget  *moo_line_view_new           (void);

void        moo_line_view_set_data      (MooLineView    *view,
                                         int             line,
                                         gpointer        data,
                                         GDestroyNotify  destroy);
void        moo_line_view_set_line_data (MooLineView    *view,
                                         int             line,
                                         const GValue   *data);
void        moo_line_view_set_boxed     (MooLineView    *view,
                                         int             line,
                                         GType           type,
                                         gpointer        data);
gpointer    moo_line_view_get_data      (MooLineView    *view,
                                         int             line);
gboolean    moo_line_view_get_line_data (MooLineView    *view,
                                         int             line,
                                         GValue         *dest);
/* returns a copy */
gpointer    moo_line_view_get_boxed     (MooLineView    *view,
                                         int             line,
                                         GType           type);

GtkTextTag *moo_line_view_create_tag    (MooLineView    *view,
                                         const char     *tag_name,
                                         const char     *first_property_name,
                                         ...);
GtkTextTag *moo_line_view_lookup_tag    (MooLineView    *view,
                                         const char     *tag_name);

void        moo_line_view_clear         (MooLineView    *view);

int         moo_line_view_start_line    (MooLineView    *view);
void        moo_line_view_write         (MooLineView    *view,
                                         const char     *text,
                                         gssize          len,
                                         GtkTextTag     *tag);
void        moo_line_view_end_line      (MooLineView    *view);

int         moo_line_view_write_line    (MooLineView    *view,
                                         const char     *text,
                                         gssize          len,
                                         GtkTextTag     *tag);



G_END_DECLS

#endif /* __MOO_LINE_VIEW__ */
