/*
 *   mooedit-impl.h
 *
 *   Copyright (C) 2004-2010 by Yevgen Muntyan <muntyan@tamu.edu>
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

#ifndef MOO_EDIT_IMPL_H
#define MOO_EDIT_IMPL_H

#include "mooedit/moolinemark.h"
#include "mooedit/mooeditor.h"
#include "mooedit/mootextview.h"
#include "mooutils/mdhistorymgr.h"
#include "mooutils/moolist.h"
#include <gio/gio.h>

G_BEGIN_DECLS

struct MooEditFileInfo {
    GFile *file;
    char *encoding;
};

MOO_DEFINE_SLIST(MooEditList, moo_edit_list, MooEdit)

extern MooEditList *_moo_edit_instances;

void        _moo_edit_add_class_actions     (MooEdit        *edit);
void        _moo_edit_check_actions         (MooEdit        *edit);
void        _moo_edit_class_init_actions    (MooEditClass   *klass);

void        _moo_edit_do_popup              (MooEdit        *edit,
                                             GdkEventButton *event);

gboolean    _moo_edit_has_comments          (MooEdit        *edit,
                                             gboolean       *single_line,
                                             gboolean       *multi_line);

#define MOO_EDIT_GOTO_BOOKMARK_ACTION "GoToBookmark"
void        _moo_edit_delete_bookmarks      (MooEdit        *edit,
                                             gboolean        in_destroy);
void        _moo_edit_line_mark_moved       (MooEdit        *edit,
                                             MooLineMark    *mark);
void        _moo_edit_line_mark_deleted     (MooEdit        *edit,
                                             MooLineMark    *mark);
gboolean    _moo_edit_line_mark_clicked     (MooTextView    *view,
                                             int             line);
void        _moo_edit_update_bookmarks_style(MooEdit        *edit);

void        _moo_edit_history_item_set_encoding (MdHistoryItem  *item,
                                                 const char     *encoding);
void        _moo_edit_history_item_set_line     (MdHistoryItem  *item,
                                                 int             line);
const char *_moo_edit_history_item_get_encoding (MdHistoryItem  *item);
int         _moo_edit_history_item_get_line     (MdHistoryItem  *item);


/***********************************************************************/
/* Preferences
 */
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

extern guint *_moo_edit_settings;

void        _moo_edit_update_global_config      (void);
void        _moo_edit_init_config               (void);
void        _moo_edit_update_lang_config        (void);

void        _moo_edit_apply_prefs               (MooEdit        *edit);


/***********************************************************************/
/* File operations
 */

GFile       *_moo_edit_get_file                 (MooEdit        *edit);

void         _moo_edit_set_file                 (MooEdit        *edit,
                                                 GFile          *file,
                                                 const char     *encoding);
const char  *_moo_edit_get_default_encoding     (void);
void         _moo_edit_ensure_newline           (MooEdit        *edit);

void         _moo_edit_stop_file_watch          (MooEdit        *edit);

void         _moo_edit_set_status               (MooEdit        *edit,
                                                 MooEditStatus   status);

void         _moo_edit_set_state                (MooEdit        *edit,
                                                 MooEditState    state,
                                                 const char     *text,
                                                 GDestroyNotify  cancel,
                                                 gpointer        data);
void         _moo_edit_create_progress_dialog   (MooEdit        *edit);
void         _moo_edit_set_progress_text        (MooEdit        *edit,
                                                 const char     *text);

GdkPixbuf   *_moo_edit_get_icon                 (MooEdit        *edit,
                                                 GtkWidget      *widget,
                                                 GtkIconSize     size);

char *_moo_edit_normalize_filename_for_comparison (const char *filename);
char *_moo_edit_normalize_uri_for_comparison (const char *uri);


G_END_DECLS

#endif /* MOO_EDIT_IMPL_H */