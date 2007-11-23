/*
 *   mooeditor.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_EDITOR_H
#define MOO_EDITOR_H

#include <mooedit/mooeditwindow.h>
#include <mooedit/moolangmgr.h>
#include <mooutils/moouixml.h>
#include <mooutils/moohistorylist.h>
#include <mooutils/moofiltermgr.h>

G_BEGIN_DECLS


#define MOO_TYPE_EDITOR                 (moo_editor_get_type ())
#define MOO_EDITOR(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_EDITOR, MooEditor))
#define MOO_EDITOR_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_EDITOR, MooEditorClass))
#define MOO_IS_EDITOR(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_EDITOR))
#define MOO_IS_EDITOR_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_EDITOR))
#define MOO_EDITOR_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_EDITOR, MooEditorClass))

typedef struct _MooEditor               MooEditor;
typedef struct _MooEditorPrivate        MooEditorPrivate;
typedef struct _MooEditorClass          MooEditorClass;

struct _MooEditor
{
    GObject                 parent;
    MooEditorPrivate       *priv;
};

struct _MooEditorClass
{
    GObjectClass parent_class;

    gboolean (*close_window) (MooEditor     *editor,
                              MooEditWindow *window,
                              gboolean       ask_confirm);
};


GType            moo_editor_get_type        (void) G_GNUC_CONST;

MooEditor       *moo_editor_instance        (void);
MooEditor       *moo_editor_create_instance (void);

/* this creates 'windowless' MooEdit instance */
MooEdit         *moo_editor_create_doc      (MooEditor      *editor,
                                             const char     *filename,
                                             const char     *encoding,
                                             GError        **error);

MooEditWindow   *moo_editor_new_window      (MooEditor      *editor);
MooEdit         *moo_editor_new_doc         (MooEditor      *editor,
                                             MooEditWindow  *window);

gboolean         moo_editor_open            (MooEditor      *editor,
                                             MooEditWindow  *window,
                                             GtkWidget      *parent,
                                             GSList         *files); /* list of MooEditFileInfo* */
MooEdit         *moo_editor_open_file       (MooEditor      *editor,
                                             MooEditWindow  *window,
                                             GtkWidget      *parent,
                                             const char     *filename,
                                             const char     *encoding);
MooEdit         *moo_editor_open_file_line  (MooEditor      *editor,
                                             const char     *filename,
                                             int             line,
                                             MooEditWindow  *window);
MooEdit         *moo_editor_new_file        (MooEditor      *editor,
                                             MooEditWindow  *window,
                                             GtkWidget      *parent,
                                             const char     *filename,
                                             const char     *encoding);
MooEdit         *moo_editor_open_uri        (MooEditor      *editor,
                                             MooEditWindow  *window,
                                             GtkWidget      *parent,
                                             const char     *uri,
                                             const char     *encoding);

MooEdit         *moo_editor_get_doc         (MooEditor      *editor,
                                             const char     *filename);

MooEdit         *moo_editor_get_active_doc  (MooEditor      *editor);
MooEditWindow   *moo_editor_get_active_window (MooEditor    *editor);

void             moo_editor_set_active_window (MooEditor    *editor,
                                             MooEditWindow  *window);
void             moo_editor_set_active_doc  (MooEditor      *editor,
                                             MooEdit        *doc);

void             moo_editor_present         (MooEditor      *editor,
                                             guint32         stamp);

/* lists must be freed, content must not be unrefed */
GSList          *moo_editor_list_windows    (MooEditor      *editor);
GSList          *moo_editor_list_docs       (MooEditor      *editor);

gboolean         moo_editor_close_window    (MooEditor      *editor,
                                             MooEditWindow  *window,
                                             gboolean        ask_confirm);
gboolean         moo_editor_close_doc       (MooEditor      *editor,
                                             MooEdit        *doc,
                                             gboolean        ask_confirm);
gboolean         moo_editor_close_docs      (MooEditor      *editor,
                                             GSList         *list,
                                             gboolean        ask_confirm);
gboolean         moo_editor_close_all       (MooEditor      *editor,
                                             gboolean        ask_confirm,
                                             gboolean        leave_one);

void             moo_editor_set_app_name    (MooEditor      *editor,
                                             const char     *name);
const char      *moo_editor_get_app_name    (MooEditor      *editor);

MooHistoryList  *moo_editor_get_history     (MooEditor      *editor);
MooFilterMgr    *moo_editor_get_filter_mgr  (MooEditor      *editor);

MooUIXML        *moo_editor_get_ui_xml      (MooEditor      *editor);
void             moo_editor_set_ui_xml      (MooEditor      *editor,
                                             MooUIXML       *xml);
MooUIXML        *moo_editor_get_doc_ui_xml  (MooEditor      *editor);

MooEditor       *moo_edit_window_get_editor (MooEditWindow  *window);
MooEditor       *moo_edit_get_editor        (MooEdit        *doc);

MooLangMgr      *moo_editor_get_lang_mgr    (MooEditor      *editor);

void             moo_editor_set_window_type (MooEditor      *editor,
                                             GType           type);
void             moo_editor_set_edit_type   (MooEditor      *editor,
                                             GType           type);

gboolean         moo_editor_save_copy       (MooEditor      *editor,
                                             MooEdit        *doc,
                                             const char     *filename,
                                             const char     *encoding,
                                             GError        **error);

void             moo_editor_apply_prefs     (MooEditor      *editor);

void            _moo_editor_load_session    (MooEditor      *editor,
                                             MooMarkupNode  *xml);
void            _moo_editor_save_session    (MooEditor      *editor,
                                             MooMarkupNode  *xml);


enum {
    MOO_EDIT_OPEN_NEW_WINDOW    = 1u << 0,
    MOO_EDIT_OPEN_NEW_TAB       = 1u << 1
};

char           *_moo_edit_filename_to_uri   (const char     *filename,
                                             guint           line,
                                             guint           options);
char           *_moo_edit_uri_to_filename   (const char     *uri,
                                             guint          *line,
                                             guint          *options);
void            _moo_editor_open_file       (MooEditor      *editor,
                                             const char     *filename,
                                             guint           line,
                                             guint           options);


G_END_DECLS

#endif /* MOO_EDITOR_H */
