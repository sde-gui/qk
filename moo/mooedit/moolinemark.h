/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   moolinemark.h
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

#ifndef __MOO_LINE_MARK_H__
#define __MOO_LINE_MARK_H__

#include <glib-object.h>
#include <gdk/gdkcolor.h>

G_BEGIN_DECLS


#define MOO_TYPE_LINE_MARK              (moo_line_mark_get_type ())
#define MOO_LINE_MARK(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_LINE_MARK, MooLineMark))
#define MOO_LINE_MARK_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_LINE_MARK, MooLineMarkClass))
#define MOO_IS_LINE_MARK(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_LINE_MARK))
#define MOO_IS_LINE_MARK_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_LINE_MARK))
#define MOO_LINE_MARK_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_LINE_MARK, MooLineMarkClass))


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

    void (*changed) (MooLineMark *mark);
    void (*moved)   (MooLineMark *mark);
};


GType       moo_line_mark_get_type              (void) G_GNUC_CONST;

void        moo_line_mark_set_background_gdk    (MooLineMark    *mark,
                                                 const GdkColor *color);
void        moo_line_mark_set_background        (MooLineMark    *mark,
                                                 const char     *color);

void        moo_line_mark_set_name              (MooLineMark    *mark,
                                                 const char     *name);
const char *moo_line_mark_get_name              (MooLineMark    *mark);

int         moo_line_mark_get_line              (MooLineMark    *mark);

void        _moo_line_mark_set_line             (MooLineMark    *mark,
                                                 gpointer        line,
                                                 int             line_no);


G_END_DECLS

#endif /* __MOO_LINE_MARK_H__ */
