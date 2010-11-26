/*
 *   mooeditview.h
 *
 *   Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@sourceforge.net>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MOO_EDIT_VIEW_H
#define MOO_EDIT_VIEW_H

#include <mooedit/mootextview.h>
#include <mooedit/mooedittypes.h>

G_BEGIN_DECLS


#define MOO_TYPE_EDIT_VIEW                       (moo_edit_view_get_type ())
#define MOO_EDIT_VIEW(object)                    (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_EDIT_VIEW, MooEditView))
#define MOO_EDIT_VIEW_CLASS(klass)               (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_EDIT_VIEW, MooEditViewClass))
#define MOO_IS_EDIT_VIEW(object)                 (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_EDIT_VIEW))
#define MOO_IS_EDIT_VIEW_CLASS(klass)            (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_EDIT_VIEW))
#define MOO_EDIT_VIEW_GET_CLASS(obj)             (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_EDIT_VIEW, MooEditViewClass))

typedef struct MooEditViewPrivate  MooEditViewPrivate;
typedef struct MooEditViewClass    MooEditViewClass;

struct MooEditView
{
    MooTextView parent;
    MooEditViewPrivate *priv;
};

struct MooEditViewClass
{
    MooTextViewClass parent_class;
};


GType            moo_edit_view_get_type                 (void) G_GNUC_CONST;

MooEditWindow   *moo_edit_view_get_window               (MooEditView    *view);
MooEditBuffer   *moo_edit_view_get_buffer               (MooEditView    *view);
MooEdit         *moo_edit_view_get_doc                  (MooEditView    *view);
MooEditor       *moo_edit_view_get_editor               (MooEditView    *view);

void             moo_edit_view_set_line_wrap_mode       (MooEditView    *view,
                                                         gboolean        enabled);
void             moo_edit_view_set_show_line_numbers    (MooEditView    *view,
                                                         gboolean        show);


G_END_DECLS

#endif /* MOO_EDIT_VIEW_H */
