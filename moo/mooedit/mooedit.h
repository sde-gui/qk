/*
 *   mooedit.h
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

#ifndef MOO_EDIT_H
#define MOO_EDIT_H

#include <mooedit/mootextview.h>
#include <mooedit/mooeditconfig.h>
#include <mooutils/mooprefs.h>

G_BEGIN_DECLS


#define MOO_TYPE_EDIT_FILE_INFO             (moo_edit_file_info_get_type ())

#define MOO_TYPE_EDIT                       (moo_edit_get_type ())
#define MOO_EDIT(object)                    (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_EDIT, MooEdit))
#define MOO_EDIT_CLASS(klass)               (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_EDIT, MooEditClass))
#define MOO_IS_EDIT(object)                 (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_EDIT))
#define MOO_IS_EDIT_CLASS(klass)            (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_EDIT))
#define MOO_EDIT_GET_CLASS(obj)             (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_EDIT, MooEditClass))

typedef enum /*< flags >*/
{
    MOO_EDIT_MODIFIED_ON_DISK   = 1 << 0,
    MOO_EDIT_DELETED            = 1 << 1,
    MOO_EDIT_CHANGED_ON_DISK    = MOO_EDIT_MODIFIED_ON_DISK | MOO_EDIT_DELETED,
    MOO_EDIT_MODIFIED           = 1 << 2,
    MOO_EDIT_NEW                = 1 << 3,
    MOO_EDIT_CLEAN              = 1 << 4  /* doesn't prompt if it's closed, even if it's modified*/
} MooEditStatus;

typedef enum
{
    MOO_EDIT_STATE_NORMAL,
    MOO_EDIT_STATE_LOADING,
    MOO_EDIT_STATE_SAVING,
    MOO_EDIT_STATE_PRINTING
} MooEditState;

#define MOO_EDIT_IS_MODIFIED(edit)  (moo_edit_get_status (edit) & MOO_EDIT_MODIFIED)
#define MOO_EDIT_IS_CLEAN(edit)     (moo_edit_get_status (edit) & MOO_EDIT_CLEAN)
#define MOO_EDIT_IS_BUSY(edit)      (moo_edit_get_state (edit) != MOO_EDIT_STATE_NORMAL)

typedef struct _MooEditFileInfo MooEditFileInfo;
typedef struct _MooEdit         MooEdit;
typedef struct _MooEditPrivate  MooEditPrivate;
typedef struct _MooEditClass    MooEditClass;


struct _MooEditFileInfo {
    char *filename;
    char *encoding;
};

struct _MooEdit
{
    MooTextView parent;
    MooEditConfig *config;
    MooEditPrivate *priv;
};

struct _MooEditClass
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


GType            moo_edit_get_type                      (void) G_GNUC_CONST;
GType            moo_edit_file_info_get_type            (void) G_GNUC_CONST;

const char      *moo_edit_get_filename          (MooEdit        *edit);
const char      *moo_edit_get_basename          (MooEdit        *edit);
const char      *moo_edit_get_display_filename  (MooEdit        *edit);
const char      *moo_edit_get_display_basename  (MooEdit        *edit);

const char      *moo_edit_get_encoding          (MooEdit        *edit);

char            *moo_edit_get_uri               (MooEdit        *edit);

gboolean         moo_edit_is_empty              (MooEdit        *edit);
void             moo_edit_set_modified          (MooEdit        *edit,
                                                 gboolean        modified);
gboolean         moo_edit_get_clean             (MooEdit        *edit);
void             moo_edit_set_clean             (MooEdit        *edit,
                                                 gboolean        clean);
MooEditStatus    moo_edit_get_status            (MooEdit        *edit);
void             moo_edit_status_changed        (MooEdit        *edit);
MooEditState     moo_edit_get_state             (MooEdit        *edit);

MooEditFileInfo *moo_edit_file_info_new         (const char     *filename,
                                                 const char     *encoding);
MooEditFileInfo *moo_edit_file_info_copy        (const MooEditFileInfo *info);
void             moo_edit_file_info_free        (MooEditFileInfo       *info);

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
