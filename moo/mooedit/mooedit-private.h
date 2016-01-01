/*
 *   mooedit-private.h
 *
 *   Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
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
#include "mooedit/mooeditprogress.h"
#include "moocpp/utils.h"
#include "moocpp/gobjectptr.h"

G_BEGIN_DECLS

#define MOO_EDIT_IS_UNTITLED(edit) (!(edit)->priv->file)

struct MooEditPrivate {
    MooEditPrivate();
    ~MooEditPrivate();

    MooEditor*                      editor;

    moo::GObjRefPtr<GtkTextBuffer>  buffer;
    std::vector<MooEditViewPtr>     views;
    MooEditView*                    active_view;
    bool                            dead_active_view;

    gulong                          changed_handler_id;
    gulong                          modified_changed_handler_id;
    guint                           apply_config_idle;
    bool                            in_recheck_config;

    /***********************************************************************/
    /* Document
     */
    moo::GObjRefPtr<GFile>          file;
    moo::mg_str                     filename;
    moo::mg_str                     norm_name;
    moo::mg_str                     display_filename;
    moo::mg_str                     display_basename;

    moo::mg_str                     encoding;
    MooLineEndType                  line_end_type;
    MooEditStatus                   status;

    guint                           file_monitor_id;
    bool                            modified_on_disk;
    bool                            deleted_from_disk;

    // file sync event source ID
    guint                           sync_timeout_id;

    MooEditState                    state;
    MooEditProgress*                progress;

    /***********************************************************************/
    /* Bookmarks
     */
    GSList*                         bookmarks; /* sorted by line number */
    guint                           update_bookmarks_idle;
    bool                            enable_bookmarks;

    /***********************************************************************/
    /* Actions
     */
    MooActionCollection*            actions;
};

void    _moo_edit_remove_untitled   (MooEdit    *doc);

G_END_DECLS

#endif /* MOO_EDIT_PRIVATE_H */
