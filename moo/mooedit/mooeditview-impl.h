/*
 *   mooeditview-impl.h
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

#ifndef MOOEDIT_COMPILATION
#error "This file may not be used"
#endif

#ifndef MOO_EDIT_VIEW_IMPL_H
#define MOO_EDIT_VIEW_IMPL_H

#include <mooedit/mooeditview.h>

G_BEGIN_DECLS

void         _moo_edit_view_set_doc             (MooEditView    *view,
                                                 MooEdit        *doc);

void         _moo_edit_view_do_popup            (MooEditView    *view,
                                                 GdkEventButton *event);
gboolean     _moo_edit_view_line_mark_clicked   (MooTextView    *view,
                                                 int             line);

void         _moo_edit_view_set_state           (MooEditView    *view,
                                                 MooEditState    state,
                                                 const char     *text,
                                                 GDestroyNotify  cancel,
                                                 gpointer        data);
void         _moo_edit_view_set_progress_text   (MooEditView    *view,
                                                 const char     *text);

G_END_DECLS

#endif /* MOO_EDIT_VIEW_IMPL_H */
