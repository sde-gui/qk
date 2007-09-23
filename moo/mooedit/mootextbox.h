/*
 *   mootextbox.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOOEDIT_COMPILATION
#error "This file may not be included"
#endif

#ifndef MOO_TEXT_BOX_H
#define MOO_TEXT_BOX_H

#include <gtk/gtklabel.h>
#include <gtk/gtktextview.h>

G_BEGIN_DECLS


#define MOO_TYPE_TEXT_BOX                   (_moo_text_box_get_type ())
#define MOO_TEXT_BOX(object)                (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_TEXT_BOX, MooTextBox))
#define MOO_TEXT_BOX_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_TEXT_BOX, MooTextBoxClass))
#define MOO_IS_TEXT_BOX(object)             (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_TEXT_BOX))
#define MOO_IS_TEXT_BOX_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_TEXT_BOX))
#define MOO_TEXT_BOX_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_TEXT_BOX, MooTextBoxClass))

#define MOO_TYPE_TEXT_ANCHOR                (_moo_text_anchor_get_type ())
#define MOO_TEXT_ANCHOR(object)             (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_TEXT_ANCHOR, MooTextAnchor))
#define MOO_TEXT_ANCHOR_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_TEXT_ANCHOR, MooTextAnchorClass))
#define MOO_IS_TEXT_ANCHOR(object)          (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_TEXT_ANCHOR))
#define MOO_IS_TEXT_ANCHOR_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_TEXT_ANCHOR))
#define MOO_TEXT_ANCHOR_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_TEXT_ANCHOR, MooTextAnchorClass))

#define MOO_TEXT_UNKNOWN_CHAR       0xFFFC
#define MOO_TEXT_UNKNOWN_CHAR_S     "\xEF\xBF\xBC"


typedef struct _MooTextBox              MooTextBox;
typedef struct _MooTextBoxClass         MooTextBoxClass;
typedef struct _MooTextAnchor           MooTextAnchor;
typedef struct _MooTextAnchorClass      MooTextAnchorClass;

struct _MooTextBox
{
    GtkLabel parent;
};

struct _MooTextBoxClass
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


GType       _moo_text_box_get_type      (void) G_GNUC_CONST;
GType       _moo_text_anchor_get_type   (void) G_GNUC_CONST;


G_END_DECLS

#endif /* MOO_TEXT_BOX_H */
