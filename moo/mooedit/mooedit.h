/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooedit.h
 *
 *   Copyright (C) 2004-2005 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef __MOO_EDIT_H__
#define __MOO_EDIT_H__

#include <mooedit/mootextview.h>
#include <mooedit/moolang.h>
#include <mooutils/mooprefs.h>

G_BEGIN_DECLS


#define MOO_TYPE_EDIT                       (moo_edit_get_type ())
#define MOO_TYPE_EDIT_ON_EXTERNAL_CHANGES   (moo_edit_on_external_changes_get_type ())
#define MOO_TYPE_EDIT_STATUS                (moo_edit_status_get_type ())
#define MOO_TYPE_EDIT_FILE_INFO             (moo_edit_file_info_get_type ())
#define MOO_TYPE_EDIT_VAR_DEP               (moo_edit_var_dep_get_type ())

#define MOO_EDIT(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_EDIT, MooEdit))
#define MOO_EDIT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_EDIT, MooEditClass))
#define MOO_IS_EDIT(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_EDIT))
#define MOO_IS_EDIT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_EDIT))
#define MOO_EDIT_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_EDIT, MooEditClass))


typedef enum {
    MOO_EDIT_VAR_DEP_NONE       = 0,
    MOO_EDIT_VAR_DEP_FILENAME   = 1,
    MOO_EDIT_VAR_DEP_AUTO       = 2
} MooEditVarDep;

#define MOO_EDIT_VAR_LANG   "lang"
#define MOO_EDIT_VAR_INDENT "indent"
#define MOO_EDIT_VAR_STRIP  "strip"


typedef enum {
    MOO_EDIT_DONT_WATCH_FILE = 0,
    MOO_EDIT_ALWAYS_ALERT,
    MOO_EDIT_ALWAYS_RELOAD,
    MOO_EDIT_RELOAD_IF_SAFE
} MooEditOnExternalChanges;

typedef enum {
    MOO_EDIT_MODIFIED_ON_DISK   = 1 << 0,
    MOO_EDIT_DELETED            = 1 << 1,
    MOO_EDIT_CHANGED_ON_DISK    = MOO_EDIT_MODIFIED_ON_DISK | MOO_EDIT_DELETED,
    MOO_EDIT_MODIFIED           = 1 << 2,
    MOO_EDIT_CLEAN              = 1 << 3  /* doesn't prompt if it's closed, even if it's modified*/
} MooEditStatus;

#define MOO_EDIT_IS_MODIFIED(edit)  (moo_edit_get_status (edit) & MOO_EDIT_MODIFIED)
#define MOO_EDIT_IS_CLEAN(edit)     (moo_edit_get_status (edit) & MOO_EDIT_CLEAN)

struct _MooEditFileInfo {
    char *filename;
    char *encoding;
};


typedef struct _MooEditFileInfo MooEditFileInfo;
typedef struct _MooEdit         MooEdit;
typedef struct _MooEditPrivate  MooEditPrivate;
typedef struct _MooEditClass    MooEditClass;

struct _MooEdit
{
    MooTextView parent;

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

    void (* lang_changed)       (MooEdit    *edit);

    void (* variable_changed)   (MooEdit    *edit,
                                 const char *name);

    void (* save_before)        (MooEdit    *edit);
    void (* save_after)         (MooEdit    *edit);
};


GType            moo_edit_get_type                       (void) G_GNUC_CONST;
GType            moo_edit_status_get_type                (void) G_GNUC_CONST;
GType            moo_edit_on_external_changes_get_type   (void) G_GNUC_CONST;
GType            moo_edit_file_info_get_type             (void) G_GNUC_CONST;
GType            moo_edit_var_dep_get_type               (void) G_GNUC_CONST;

const char      *moo_edit_get_filename          (MooEdit        *edit);
const char      *moo_edit_get_basename          (MooEdit        *edit);
const char      *moo_edit_get_display_filename  (MooEdit        *edit);
const char      *moo_edit_get_display_basename  (MooEdit        *edit);

const char      *moo_edit_get_encoding          (MooEdit        *edit);

gboolean         moo_edit_is_empty              (MooEdit        *edit);
void             moo_edit_set_modified          (MooEdit        *edit,
                                                 gboolean        modified);
gboolean         moo_edit_get_clean             (MooEdit        *edit);
void             moo_edit_set_clean             (MooEdit        *edit,
                                                 gboolean        clean);
MooEditStatus    moo_edit_get_status            (MooEdit        *edit);
void             moo_edit_status_changed        (MooEdit        *edit);

gboolean         moo_edit_get_readonly          (MooEdit        *edit);
void             moo_edit_set_readonly          (MooEdit        *edit,
                                                 gboolean        readonly);

MooEditFileInfo *moo_edit_file_info_new         (const char     *filename,
                                                 const char     *encoding);
MooEditFileInfo *moo_edit_file_info_copy        (const MooEditFileInfo *info);
void             moo_edit_file_info_free        (MooEditFileInfo       *info);

void             moo_edit_set_highlight         (MooEdit        *edit,
                                                 gboolean        highlight);
gboolean         moo_edit_get_highlight         (MooEdit        *edit);

void             moo_edit_set_var               (MooEdit        *edit,
                                                 const char     *name,
                                                 const GValue   *value,
                                                 MooEditVarDep   dep);
gboolean         moo_edit_get_var               (MooEdit        *edit,
                                                 const char     *name,
                                                 GValue         *value);

char            *moo_edit_get_string            (MooEdit        *edit,
                                                 const char     *name);
gboolean         moo_edit_get_bool              (MooEdit        *edit,
                                                 const char     *name,
                                                 gboolean        default_val);
int              moo_edit_get_int               (MooEdit        *edit,
                                                 const char     *name,
                                                 int             default_val);
guint            moo_edit_get_uint              (MooEdit        *edit,
                                                 const char     *name,
                                                 guint           default_val);
void             moo_edit_set_string            (MooEdit        *edit,
                                                 const char     *name,
                                                 const char     *value,
                                                 MooEditVarDep   dep);

void             moo_edit_register_var          (GParamSpec     *pspec);
void             moo_edit_register_var_alias    (const char     *name,
                                                 const char     *alias);

gboolean         moo_edit_save                  (MooEdit        *edit);
gboolean         moo_edit_save_as               (MooEdit        *edit,
                                                 const char     *filename,
                                                 const char     *encoding);
gboolean         moo_edit_save_copy             (MooEdit        *edit,
                                                 const char     *filename,
                                                 const char     *encoding,
                                                 GError        **error);


G_END_DECLS

#endif /* __MOO_EDIT_H__ */
