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

#pragma once

#include "mooedit/mooedit-impl.h"
#include "mooedit/mooeditprogress.h"
#include "mooedit/mooeditbookmark.h"
#include "mooedit/mooeditview-impl.h"
#include "moocpp/strutils.h"
#include "moocpp/utils.h"
#include "moocpp/gobjtypes.h"

struct MooEditPrivate {
    MooEditPrivate();
    ~MooEditPrivate();

    MooEditor*                              editor;

    moo::gobj_ptr<GtkTextBuffer>            buffer;
    std::vector<EditViewPtr>                views;
    EditViewRawPtr                          active_view;
    bool                                    dead_active_view;

    gulong                                  changed_handler_id;
    gulong                                  modified_changed_handler_id;
    guint                                   apply_config_idle;
    bool                                    in_recheck_config;

    /***********************************************************************/
    /* Document
     */
    moo::gobj_ptr<GFile>                    file;
    moo::gstr                               filename;
    moo::gstr                               norm_name;
    moo::gstr                               display_filename;
    moo::gstr                               display_basename;

    moo::gstr                               encoding;
    MooLineEndType                          line_end_type;
    MooEditStatus                           status;

    guint                                   file_monitor_id;
    bool                                    modified_on_disk;
    bool                                    deleted_from_disk;

    // file sync event source ID
    guint                                   sync_timeout_id;

    MooEditState                            state;
    moo::gobj_ptr<MooEditProgress>          progress;

    /***********************************************************************/
    /* Bookmarks
     */
    GSList*                                 bookmarks; /* sorted by line number */
    guint                                   update_bookmarks_idle;
    bool                                    enable_bookmarks;

    /***********************************************************************/
    /* Actions
     */
    moo::gobj_ptr<MooActionCollection>      actions;
};
