/*
 *   mooedit-private.h
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

#ifndef MOO_EDIT_PRIVATE_H
#define MOO_EDIT_PRIVATE_H

#include "mooedit/mooedit-impl.h"

G_BEGIN_DECLS

#define MOO_EDIT_IS_UNTITLED(edit) (!(edit)->priv->file)

struct MooEditPrivate {
    MooEditView *view;
    MooEditor *editor;

    gulong modified_changed_handler_id;
    guint apply_config_idle;

    /***********************************************************************/
    /* Document
     */
    GFile *file;
    char *filename;
    char *norm_filename;
    char *display_filename;
    char *display_basename;
    int untitled_no;

    char *encoding;
    MooLineEndType line_end_type;
    MooEditStatus status;

    guint file_monitor_id;
    gboolean modified_on_disk;
    gboolean deleted_from_disk;

    /***********************************************************************/
    /* Progress dialog
     */
    MooEditState state;

    /***********************************************************************/
    /* Bookmarks
     */
    gboolean enable_bookmarks;
    GSList *bookmarks; /* sorted by line number */
    guint update_bookmarks_idle;

    /***********************************************************************/
    /* Actions
     */
    MooActionCollection *actions;
};

G_END_DECLS

#endif /* MOO_EDIT_PRIVATE_H */
