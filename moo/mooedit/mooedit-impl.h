/*
 *   mooedit-impl.h
 *
 *   Copyright (C) 2004-2016 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
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

#include "mooedit/moolinemark.h"
#include "mooedit/mooeditor.h"
#include "mooedit/mootextview.h"
#include "mooedit/mooedittypes.h"
#include "mooutils/moohistorymgr.h"

#ifdef __cplusplus
#include <moocpp/strutils.h>
#include <moocpp/gobjtypes-gio.h>
#include <vector>

template<>
class moo::gobj_ref<MooEdit> : public moo::gobj_ref_parent<MooEdit>
{
public:
    MOO_DEFINE_GOBJREF_METHODS(MooEdit)

    static std::vector<gobj_raw_ptr<MooEdit>> _moo_edit_instances;

    void                    _add_view                           (EditView           view);
    void                    _remove_view                        (EditView           view);
    void                    _set_active_view                    (EditView           view);

    bool                    _is_busy                            () const;

    void                    _add_class_actions                  ();
    static void             _class_init_actions                 (MooEditClass*      klass);

    void                    _status_changed                     ();

    void                    _delete_bookmarks                   (bool               in_destroy);
    static void             _line_mark_moved                    (MooEdit*           doc,
                                                                 MooLineMark*       mark);
    static void             _line_mark_deleted                  (MooEdit*           doc,
                                                                 MooLineMark*       mark);
    void                    _update_bookmarks_style             ();

    void                    _queue_recheck_config               ();

    void                    _closed                             ();

    void                    _set_file                           (g::FileRawPtr      file,
                                                                 const char*        encoding);
    void                    _remove_untitled                    (Edit               doc);

    void                    _ensure_newline                     ();

    void                    _stop_file_watch                    ();

    void                    _set_status                         (MooEditStatus      status);

    void                    _strip_whitespace                   ();

    MooActionCollection&    _get_actions                        ();

    static moo::gstr        _get_normalized_name                (g::File            file);
    const moo::gstr&        _get_normalized_name                () const;

    MooEditPrivate&         get_priv                            ()                      { return *gobj()->priv; }
    const MooEditPrivate&   get_priv                            () const                { return *gobj()->priv; }
    MooEditConfig*          get_config                          () const                { return gobj()->config; }
    void                    set_config                          (MooEditConfig* cfg)    { gobj()->config = cfg; }
};

MOO_REGISTER_CUSTOM_GOBJ_TYPE(MooEdit);

#endif // __cplusplus

G_BEGIN_DECLS

#define MOO_EDIT_GOTO_BOOKMARK_ACTION "GoToBookmark"

enum {
    MOO_EDIT_SETTING_LANG,
    MOO_EDIT_SETTING_INDENT,
    MOO_EDIT_SETTING_STRIP,
    MOO_EDIT_SETTING_ADD_NEWLINE,
    MOO_EDIT_SETTING_WRAP_MODE,
    MOO_EDIT_SETTING_SHOW_LINE_NUMBERS,
    MOO_EDIT_SETTING_TAB_WIDTH,
    MOO_EDIT_SETTING_WORD_CHARS,
    MOO_EDIT_LAST_SETTING
};

void            _moo_edit_init_config               (void);
gboolean        _moo_edit_has_comments              (MooEdit        *edit,
                                                     gboolean       *single_line,
                                                     gboolean       *multi_line);

void            _moo_edit_queue_recheck_config_all  (void);
void            _moo_edit_update_global_config      (void);

void            _moo_edit_check_actions             (MooEdit*       edit,
                                                     MooEditView*   view);

MooEditState    _moo_edit_get_state                 (MooEdit        *doc);
void            _moo_edit_set_progress_text         (MooEdit        *doc,
                                                     const char     *text);
void            _moo_edit_set_state                 (MooEdit        *doc,
                                                     MooEditState    state,
                                                     const char     *text,
                                                     GDestroyNotify  cancel,
                                                     gpointer        data);

GdkPixbuf*      _moo_edit_get_icon                  (MooEdit        *edit,
                                                     GtkWidget      *widget,
                                                     GtkIconSize     size);

const char*     _moo_edit_get_default_encoding      (void);

G_END_DECLS
