/*
 *   mooplaceholder.h
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

#ifndef MOOEDIT_COMPILATION
#error "This file may not be included"
#endif

#ifndef __MOO_PLACEHOLDER_H__
#define __MOO_PLACEHOLDER_H__

#include <gtk/gtklabel.h>
#include <gtk/gtktextview.h>

G_BEGIN_DECLS


#define MOO_TYPE_PLACEHOLDER              (_moo_placeholder_get_type ())
#define MOO_PLACEHOLDER(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_PLACEHOLDER, MooPlaceholder))
#define MOO_PLACEHOLDER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_PLACEHOLDER, MooPlaceholderClass))
#define MOO_IS_PLACEHOLDER(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_PLACEHOLDER))
#define MOO_IS_PLACEHOLDER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_PLACEHOLDER))
#define MOO_PLACEHOLDER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_PLACEHOLDER, MooPlaceholderClass))

#define MOO_TYPE_TEXT_ANCHOR              (_moo_text_anchor_get_type ())
#define MOO_TEXT_ANCHOR(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_TEXT_ANCHOR, MooTextAnchor))
#define MOO_TEXT_ANCHOR_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_TEXT_ANCHOR, MooTextAnchorClass))
#define MOO_IS_TEXT_ANCHOR(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_TEXT_ANCHOR))
#define MOO_IS_TEXT_ANCHOR_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_TEXT_ANCHOR))
#define MOO_TEXT_ANCHOR_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_TEXT_ANCHOR, MooTextAnchorClass))


typedef struct _MooPlaceholder          MooPlaceholder;
typedef struct _MooPlaceholderPrivate   MooPlaceholderPrivate;
typedef struct _MooPlaceholderClass     MooPlaceholderClass;
typedef struct _MooTextAnchor           MooTextAnchor;
typedef struct _MooTextAnchorPrivate    MooTextAnchorPrivate;
typedef struct _MooTextAnchorClass      MooTextAnchorClass;

struct _MooPlaceholder
{
    GtkLabel parent;
    GtkTextTagTable *table;
    GtkTextTag *tag;
};

struct _MooPlaceholderClass
{
    GtkLabelClass parent_class;
};

struct _MooTextAnchor
{
    GtkTextChildAnchor parent;
    GtkWidget *widget;
};

struct _MooTextAnchorClass
{
    GtkTextChildAnchorClass parent_class;
};


GType       _moo_placeholder_get_type   (void) G_GNUC_CONST;
GType       _moo_text_anchor_get_type   (void) G_GNUC_CONST;

void        _moo_placeholder_set_tag    (MooPlaceholder     *ph,
                                         GtkTextTagTable    *table,
                                         GtkTextTag         *tag);


G_END_DECLS

#endif /* __MOO_PLACEHOLDER_H__ */
