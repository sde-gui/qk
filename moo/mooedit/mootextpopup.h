/*
 *   mootextpopup.h
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

#ifndef __MOO_TEXT_POPUP_H__
#define __MOO_TEXT_POPUP_H__

#include <gtk/gtktextview.h>
#include <gtk/gtktreeviewcolumn.h>

G_BEGIN_DECLS


#define MOO_TYPE_TEXT_POPUP            (moo_text_popup_get_type ())
#define MOO_TEXT_POPUP(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_TEXT_POPUP, MooTextPopup))
#define MOO_TEXT_POPUP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_TEXT_POPUP, MooTextPopupClass))
#define MOO_IS_TEXT_POPUP(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_TEXT_POPUP))
#define MOO_IS_TEXT_POPUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_TEXT_POPUP))
#define MOO_TEXT_POPUP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_TEXT_POPUP, MooTextPopupClass))


typedef struct _MooTextPopup         MooTextPopup;
typedef struct _MooTextPopupPrivate  MooTextPopupPrivate;
typedef struct _MooTextPopupClass    MooTextPopupClass;

struct _MooTextPopup
{
    GObject parent;
    GtkTreeViewColumn *column;
    MooTextPopupPrivate *priv;
};

struct _MooTextPopupClass
{
    GObjectClass parent_class;

    void     (*show)         (MooTextPopup *popup);
    void     (*hide)         (MooTextPopup *popup);

    void     (*activate)     (MooTextPopup *popup,
                              GtkTreeModel *model,
                              GtkTreeIter  *iter);
    void     (*text_changed) (MooTextPopup *popup);
};


GType           moo_text_popup_get_type     (void) G_GNUC_CONST;

MooTextPopup   *moo_text_popup_new          (GtkTextView        *doc);

void            moo_text_popup_set_doc      (MooTextPopup       *popup,
                                             GtkTextView        *doc);
GtkTextView    *moo_text_popup_get_doc      (MooTextPopup       *popup);

void            moo_text_popup_set_model    (MooTextPopup       *popup,
                                             GtkTreeModel       *model);
GtkTreeModel   *moo_text_popup_get_model    (MooTextPopup       *popup);

gboolean        moo_text_popup_show         (MooTextPopup       *popup,
                                             const GtkTextIter  *where);
void            moo_text_popup_update       (MooTextPopup       *popup);
void            moo_text_popup_activate     (MooTextPopup       *popup);
void            moo_text_popup_hide         (MooTextPopup       *popup);

gboolean        moo_text_popup_get_position (MooTextPopup       *popup,
                                             GtkTextIter        *iter);
void            moo_text_popup_set_position (MooTextPopup       *popup,
                                             const GtkTextIter  *iter);

gboolean        moo_text_popup_visible      (MooTextPopup       *popup);


G_END_DECLS

#endif /* __MOO_TEXT_POPUP_H__ */
