/*
 *   moolinemark.h
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

#ifndef MOO_LINE_MARK_H
#define MOO_LINE_MARK_H

#include <gtk/gtkwidget.h>

G_BEGIN_DECLS


#define MOO_TYPE_FOLD                   (moo_fold_get_type ())
#define MOO_FOLD(object)                (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_FOLD, MooFold))
#define MOO_FOLD_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_FOLD, MooFoldClass))
#define MOO_IS_FOLD(object)             (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_FOLD))
#define MOO_IS_FOLD_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_FOLD))
#define MOO_FOLD_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_FOLD, MooFoldClass))

#define MOO_TYPE_LINE_MARK              (moo_line_mark_get_type ())
#define MOO_LINE_MARK(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_LINE_MARK, MooLineMark))
#define MOO_LINE_MARK_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_LINE_MARK, MooLineMarkClass))
#define MOO_IS_LINE_MARK(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_LINE_MARK))
#define MOO_IS_LINE_MARK_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_LINE_MARK))
#define MOO_LINE_MARK_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_LINE_MARK, MooLineMarkClass))


typedef struct _MooTextBuffer       MooTextBuffer;

typedef struct _MooFold             MooFold;
typedef struct _MooFoldClass        MooFoldClass;

typedef struct _MooLineMark         MooLineMark;
typedef struct _MooLineMarkPrivate  MooLineMarkPrivate;
typedef struct _MooLineMarkClass    MooLineMarkClass;

struct _MooLineMark
{
    GObject parent;
    MooLineMarkPrivate *priv;
};

struct _MooLineMarkClass
{
    GObjectClass parent_class;

    /* signal */
    void (*changed) (MooLineMark *mark);

    /* method */
    void (*deleted) (MooLineMark *mark);
};


GType       moo_line_mark_get_type              (void) G_GNUC_CONST;
GType       moo_fold_get_type                   (void) G_GNUC_CONST;

void        moo_line_mark_set_background_gdk    (MooLineMark    *mark,
                                                 const GdkColor *color);
void        moo_line_mark_set_background        (MooLineMark    *mark,
                                                 const char     *color);

int         moo_line_mark_get_line              (MooLineMark    *mark);
MooTextBuffer *moo_line_mark_get_buffer         (MooLineMark    *mark);
gboolean    moo_line_mark_get_visible           (MooLineMark    *mark);

gboolean    moo_line_mark_get_deleted           (MooLineMark    *mark);

void        moo_line_mark_set_stock_id          (MooLineMark    *mark,
                                                 const char     *stock_id);
void        moo_line_mark_set_pixbuf            (MooLineMark    *mark,
                                                 GdkPixbuf      *pixbuf);
void        moo_line_mark_set_markup            (MooLineMark    *mark,
                                                 const char     *markup);
const char *moo_line_mark_get_stock_id          (MooLineMark    *mark);
GdkPixbuf  *moo_line_mark_get_pixbuf            (MooLineMark    *mark);
const char *moo_line_mark_get_markup            (MooLineMark    *mark);
GdkGC      *moo_line_mark_get_background_gc     (MooLineMark    *mark);


G_END_DECLS

#endif /* MOO_LINE_MARK_H */
