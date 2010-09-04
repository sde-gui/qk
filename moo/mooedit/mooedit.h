/*
 *   mooedit.h
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
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

#ifndef MOO_EDIT_H
#define MOO_EDIT_H

#include <mooedit/mootextview.h>
#include <mooedit/mooeditconfig.h>
#include <mooedit/mooedit-enums.h>
#include <mooutils/mooprefs.h>

G_BEGIN_DECLS


#define MOO_TYPE_EDIT_FILE_INFO             (moo_edit_file_info_get_type ())

#define MOO_TYPE_EDIT                       (moo_edit_get_type ())
#define MOO_EDIT(object)                    (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_EDIT, MooEdit))
#define MOO_EDIT_CLASS(klass)               (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_EDIT, MooEditClass))
#define MOO_IS_EDIT(object)                 (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_EDIT))
#define MOO_IS_EDIT_CLASS(klass)            (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_EDIT))
#define MOO_EDIT_GET_CLASS(obj)             (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_EDIT, MooEditClass))

#define MOO_EDIT_IS_MODIFIED(edit)  (moo_edit_get_status (edit) & MOO_EDIT_MODIFIED)
#define MOO_EDIT_IS_CLEAN(edit)     (moo_edit_get_status (edit) & MOO_EDIT_CLEAN)
#define MOO_EDIT_IS_BUSY(edit)      (moo_edit_get_state (edit) != MOO_EDIT_STATE_NORMAL)

typedef struct MooEdit         MooEdit;
typedef struct MooEditPrivate  MooEditPrivate;
typedef struct MooEditClass    MooEditClass;

struct MooEdit
{
    MooTextView parent;
    MooEditConfig *config;
    MooEditPrivate *priv;
};

struct MooEditClass
{
    MooTextViewClass parent_class;

    /* emitted when filename, modified status, or file on disk
       are changed. for use in editor to adjust title bar, etc. */
    void (* doc_status_changed) (MooEdit    *edit);

    void (* filename_changed)   (MooEdit    *edit,
                                 const char *new_filename);

    void (* bookmarks_changed)  (MooEdit    *edit);

    void (* config_notify)      (MooEdit    *edit,
                                 guint       var_id,
                                 GParamSpec *pspec);

    void (* save_before)        (MooEdit    *edit);
    void (* save_after)         (MooEdit    *edit);
};


GType            moo_edit_get_type              (void) G_GNUC_CONST;
GType            moo_edit_file_info_get_type    (void) G_GNUC_CONST;

char            *moo_edit_get_uri               (MooEdit        *edit);
char            *moo_edit_get_filename          (MooEdit        *edit);
char            *moo_edit_get_norm_filename     (MooEdit        *edit);
const char      *moo_edit_get_display_name      (MooEdit        *edit);
const char      *moo_edit_get_display_basename  (MooEdit        *edit);

const char      *moo_edit_get_encoding          (MooEdit        *edit);
void             moo_edit_set_encoding          (MooEdit        *edit,
                                                 const char     *encoding);

char            *moo_edit_get_utf8_filename     (MooEdit        *edit);

#ifdef __WIN32__
#define MOO_LE_DEFAULT MOO_LE_WIN32
#else
#define MOO_LE_DEFAULT MOO_LE_UNIX
#endif

MooLineEndType   moo_edit_get_line_end_type     (MooEdit        *edit);
void             moo_edit_set_line_end_type     (MooEdit        *edit,
                                                 MooLineEndType  le);

gboolean         moo_edit_is_empty              (MooEdit        *edit);
gboolean         moo_edit_is_untitled           (MooEdit        *edit);
void             moo_edit_set_modified          (MooEdit        *edit,
                                                 gboolean        modified);
gboolean         moo_edit_get_clean             (MooEdit        *edit);
void             moo_edit_set_clean             (MooEdit        *edit,
                                                 gboolean        clean);
MooEditStatus    moo_edit_get_status            (MooEdit        *edit);
void             moo_edit_status_changed        (MooEdit        *edit);
MooEditState     moo_edit_get_state             (MooEdit        *edit);

void             moo_edit_reload                (MooEdit        *edit,
                                                 const char     *encoding,
                                                 GError        **error);
gboolean         moo_edit_close                 (MooEdit        *edit,
                                                 gboolean        ask_confirm);
gboolean         moo_edit_save                  (MooEdit        *edit,
                                                 GError        **error);
gboolean         moo_edit_save_as               (MooEdit        *edit,
                                                 const char     *filename,
                                                 const char     *encoding,
                                                 GError        **error);
gboolean         moo_edit_save_copy             (MooEdit        *edit,
                                                 const char     *filename,
                                                 const char     *encoding,
                                                 GError        **error);

void             moo_edit_comment               (MooEdit        *edit);
void             moo_edit_uncomment             (MooEdit        *edit);

typedef struct MooEditFileInfo MooEditFileInfo;
MooEditFileInfo *moo_edit_file_info_new_path    (const char         *path,
                                                 const char         *encoding);
MooEditFileInfo *moo_edit_file_info_new_uri     (const char         *uri,
                                                 const char         *encoding);
MooEditFileInfo *moo_edit_file_info_copy        (MooEditFileInfo    *info);
void             moo_edit_file_info_free        (MooEditFileInfo    *info);

void             moo_edit_ui_set_line_wrap_mode     (MooEdit        *edit,
                                                     gboolean        enabled);
void             moo_edit_ui_set_show_line_numbers  (MooEdit        *edit,
                                                     gboolean        show);


G_END_DECLS

#endif /* MOO_EDIT_H */
