/*
 *   mooedit.h
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

#ifndef MOO_EDIT_H
#define MOO_EDIT_H

#include <mooedit/mooeditconfig.h>
#include <mooedit/mooedit-enums.h>
#include <mooedit/mooedittypes.h>
#include <mooedit/moolang.h>
#include <mooutils/mooprefs.h>

G_BEGIN_DECLS

#define MOO_TYPE_EDIT                       (moo_edit_get_type ())
#define MOO_EDIT(object)                    (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_EDIT, MooEdit))
#define MOO_EDIT_CLASS(klass)               (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_EDIT, MooEditClass))
#define MOO_IS_EDIT(object)                 (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_EDIT))
#define MOO_IS_EDIT_CLASS(klass)            (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_EDIT))
#define MOO_EDIT_GET_CLASS(obj)             (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_EDIT, MooEditClass))

#define MOO_EDIT_IS_MODIFIED(edit)  (moo_edit_get_status (edit) & MOO_EDIT_MODIFIED)
#define MOO_EDIT_IS_CLEAN(edit)     (moo_edit_get_status (edit) & MOO_EDIT_CLEAN)
#define MOO_EDIT_IS_BUSY(edit)      (moo_edit_get_state (edit) != MOO_EDIT_STATE_NORMAL)

typedef struct _MooEditPrivate MooEditPrivate;
typedef struct _MooEditClass   MooEditClass;

struct _MooEdit
{
    GObject parent;
    MooEditConfig *config;
    MooEditPrivate *priv;
};

struct _MooEditClass
{
    GObjectClass parent_class;

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

MooEditor       *moo_edit_get_editor            (MooEdit        *doc);
MooEditBuffer   *moo_edit_get_buffer            (MooEdit        *doc);
MooEditView     *moo_edit_get_view              (MooEdit        *doc);
MooEditWindow   *moo_edit_get_window            (MooEdit        *doc);

GFile           *moo_edit_get_file              (MooEdit        *edit);

char            *moo_edit_get_uri               (MooEdit        *edit);
char            *moo_edit_get_filename          (MooEdit        *edit);
char            *moo_edit_get_norm_filename     (MooEdit        *edit);
const char      *moo_edit_get_display_name      (MooEdit        *edit);
const char      *moo_edit_get_display_basename  (MooEdit        *edit);

const char      *moo_edit_get_encoding          (MooEdit        *edit);
void             moo_edit_set_encoding          (MooEdit        *edit,
                                                 const char     *encoding);

char            *moo_edit_get_utf8_filename     (MooEdit        *edit);

MooLang         *moo_edit_get_lang              (MooEdit        *edit);

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

G_END_DECLS

#endif /* MOO_EDIT_H */
