/*
 *   mooedit/mooedit.h
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

#ifndef MOOEDIT_MOOEDIT_H
#define MOOEDIT_MOOEDIT_H

#include <gtksourceview/gtksourceview.h>
#include "mooutils/mooprefs.h"
#include "mooedit/mooeditlang.h"

G_BEGIN_DECLS


#define MOO_TYPE_EDIT                       (moo_edit_get_type ())
#define MOO_TYPE_EDIT_SELECTION_TYPE        (moo_edit_selection_type_get_type ())
#define MOO_TYPE_EDIT_ON_EXTERNAL_CHANGES   (moo_edit_on_external_changes_get_type ())
#define MOO_TYPE_EDIT_DOC_STATUS            (moo_edit_doc_status_get_type ())
#define MOO_TYPE_EDIT_FILE_INFO             (moo_edit_file_info_get_type ())

#define MOO_EDIT(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_EDIT, MooEdit))
#define MOO_EDIT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_EDIT, MooEditClass))
#define MOO_IS_EDIT(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_EDIT))
#define MOO_IS_EDIT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_EDIT))
#define MOO_EDIT_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_EDIT, MooEditClass))


typedef enum {
    MOO_EDIT_SELECT_CHARS,
    MOO_EDIT_SELECT_WORDS,
    MOO_EDIT_SELECT_LINES
} MooEditSelectionType;

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
    GtkSourceView  parent;

    MooEditPrivate *priv;
};

struct _MooEditClass
{
    GtkSourceViewClass parent_class;

    /* emitted when filename, modified status, or file on disk
       are changed. for use in editor to adjust title bar, etc. */
    void (* doc_status_changed)     (MooEdit    *edit);

    void (* filename_changed)       (MooEdit    *edit,
                                     const char *new_filename);

    void (* lang_changed)           (MooEdit    *edit);

    void (* delete_selection)       (MooEdit    *edit);

    /* these two are buffer signals */
    void (* can_redo)               (MooEdit    *edit,
                                     gboolean    arg);
    void (* can_undo)               (MooEdit    *edit,
                                     gboolean    arg);

    /* these are made signals for convenience */
    void (* find)                   (MooEdit    *edit);
    void (* replace)                (MooEdit    *edit);
    void (* find_next)              (MooEdit    *edit);
    void (* find_previous)          (MooEdit    *edit);
    void (* goto_line)              (MooEdit    *edit);

    /* methods */
    /* adjusts start and end so that selection bound goes to start
       and insert goes to end,
       returns whether selection is not empty */
    gboolean (* extend_selection)   (MooEdit                *edit,
                                     MooEditSelectionType    type,
                                     GtkTextIter            *start,
                                     GtkTextIter            *end);
};


GType       moo_edit_get_type                       (void) G_GNUC_CONST;
GType       moo_edit_doc_status_get_type            (void) G_GNUC_CONST;
GType       moo_edit_selection_type_get_type        (void) G_GNUC_CONST;
GType       moo_edit_on_external_changes_get_type   (void) G_GNUC_CONST;
GType       moo_edit_file_info_get_type             (void) G_GNUC_CONST;


MooEdit    *moo_edit_new                    (void);

const char *moo_edit_get_filename           (MooEdit            *edit);
const char *moo_edit_get_basename           (MooEdit            *edit);
const char *moo_edit_get_display_filename   (MooEdit            *edit);
const char *moo_edit_get_display_basename   (MooEdit            *edit);

const char *moo_edit_get_encoding           (MooEdit            *edit);
void        moo_edit_set_encoding           (MooEdit            *edit,
                                             const char         *encoding);

void        moo_edit_select_all             (MooEdit            *edit);

gboolean    moo_edit_is_empty               (MooEdit            *edit);
void        moo_edit_set_modified           (MooEdit            *edit,
                                             gboolean            modified);
gboolean    moo_edit_get_clean              (MooEdit            *edit);
void        moo_edit_set_clean              (MooEdit            *edit,
                                             gboolean            clean);
MooEditStatus moo_edit_get_status           (MooEdit            *edit);
void        moo_edit_status_changed         (MooEdit            *edit);

gboolean    moo_edit_get_read_only          (MooEdit            *edit);
void        moo_edit_set_read_only          (MooEdit            *edit,
                                             gboolean            readonly);

char       *moo_edit_get_selection          (MooEdit            *edit);
char       *moo_edit_get_text               (MooEdit            *edit);

void        moo_edit_delete_selection       (MooEdit            *edit);

MooEditFileInfo *moo_edit_file_info_new     (const char         *filename,
                                             const char         *encoding);
MooEditFileInfo *moo_edit_file_info_copy    (const MooEditFileInfo *info);
void        moo_edit_file_info_free         (MooEditFileInfo    *info);

gboolean    moo_edit_can_redo               (MooEdit            *edit);
gboolean    moo_edit_can_undo               (MooEdit            *edit);

void        moo_edit_find                   (MooEdit            *edit);
void        moo_edit_replace                (MooEdit            *edit);
void        moo_edit_find_next              (MooEdit            *edit);
void        moo_edit_find_previous          (MooEdit            *edit);
void        moo_edit_goto_line              (MooEdit            *edit,
                                             int                 line);


void        moo_edit_set_highlight          (MooEdit            *edit,
                                             gboolean            highlight);
MooEditLang*moo_edit_get_lang               (MooEdit            *edit);
void        moo_edit_set_lang               (MooEdit            *edit,
                                             MooEditLang        *lang);

void        moo_edit_set_font_from_string   (MooEdit            *edit,
                                             const char         *font);


G_END_DECLS

#endif /* MOOEDIT_MOOEDIT_H */
