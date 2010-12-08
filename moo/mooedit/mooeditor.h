/*
 *   mooeditor.h
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

#ifndef MOO_EDITOR_H
#define MOO_EDITOR_H

#include <mooedit/mooedittypes.h>
#include <mooedit/mooeditwindow.h>
#include <mooutils/moouixml.h>

G_BEGIN_DECLS


#define MOO_TYPE_EDITOR                 (moo_editor_get_type ())
#define MOO_EDITOR(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_EDITOR, MooEditor))
#define MOO_EDITOR_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_EDITOR, MooEditorClass))
#define MOO_IS_EDITOR(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_EDITOR))
#define MOO_IS_EDITOR_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_EDITOR))
#define MOO_EDITOR_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_EDITOR, MooEditorClass))

typedef struct MooEditorPrivate MooEditorPrivate;
typedef struct MooEditorClass   MooEditorClass;

struct MooEditor
{
    GObject base;
    MooEditorPrivate *priv;
};

struct MooEditorClass
{
    GObjectClass base_class;

    gboolean (*close_window) (MooEditor     *editor,
                              MooEditWindow *window,
                              gboolean       ask_confirm);
};


GType                moo_editor_get_type        (void) G_GNUC_CONST;

MooEditor           *moo_editor_instance        (void);
MooEditor           *moo_editor_create          (gboolean        embedded);

/* this creates 'windowless' MooEdit instance */
MooEdit             *moo_editor_create_doc      (MooEditor      *editor,
                                                 const char     *filename,
                                                 const char     *encoding,
                                                 GError        **error);

MooEditWindow       *moo_editor_new_window      (MooEditor      *editor);
MooEdit             *moo_editor_new_doc         (MooEditor      *editor,
                                                 MooEditWindow  *window);

gboolean             moo_editor_open            (MooEditor          *editor,
                                                 MooEditWindow      *window,
                                                 GtkWidget          *parent,
                                                 MooFileEncArray    *files);
MooEdit             *moo_editor_open_file       (MooEditor      *editor,
                                                 MooEditWindow  *window,
                                                 GtkWidget      *parent,
                                                 const char     *filename,
                                                 const char     *encoding);
MooEdit             *moo_editor_open_file_line  (MooEditor      *editor,
                                                 const char     *filename,
                                                 int             line,
                                                 MooEditWindow  *window);
MooEdit             *moo_editor_new_file        (MooEditor      *editor,
                                                 MooEditWindow  *window,
                                                 GtkWidget      *parent,
                                                 const char     *filename,
                                                 const char     *encoding);
MooEdit             *moo_editor_open_uri        (MooEditor      *editor,
                                                 MooEditWindow  *window,
                                                 GtkWidget      *parent,
                                                 const char     *uri,
                                                 const char     *encoding);

MooEdit             *moo_editor_get_doc         (MooEditor      *editor,
                                                 const char     *filename);
MooEdit             *moo_editor_get_doc_for_uri (MooEditor      *editor,
                                                 const char     *uri);

MooEdit             *moo_editor_get_active_doc  (MooEditor      *editor);
MooEditWindow       *moo_editor_get_active_window (MooEditor    *editor);

void                 moo_editor_set_active_window (MooEditor    *editor,
                                                 MooEditWindow  *window);
void                 moo_editor_set_active_doc  (MooEditor      *editor,
                                                 MooEdit        *doc);

void                 moo_editor_present         (MooEditor      *editor,
                                                 guint32         stamp);

MooEditWindowArray  *moo_editor_get_windows     (MooEditor      *editor);
MooEditArray        *moo_editor_get_docs        (MooEditor      *editor);

gboolean             moo_editor_close_window    (MooEditor      *editor,
                                                 MooEditWindow  *window,
                                                 gboolean        ask_confirm);
gboolean             moo_editor_close_doc       (MooEditor      *editor,
                                                 MooEdit        *doc,
                                                 gboolean        ask_confirm);
gboolean             moo_editor_close_docs      (MooEditor      *editor,
                                                 MooEditArray   *docs,
                                                 gboolean        ask_confirm);
gboolean             moo_editor_close_all       (MooEditor      *editor,
                                                 gboolean        ask_confirm,
                                                 gboolean        leave_one);

MooUiXml            *moo_editor_get_ui_xml      (MooEditor      *editor);
void                 moo_editor_set_ui_xml      (MooEditor      *editor,
                                                 MooUiXml       *xml);
MooUiXml            *moo_editor_get_doc_ui_xml  (MooEditor      *editor);

void                 moo_editor_set_window_type (MooEditor      *editor,
                                                 GType           type);
void                 moo_editor_set_edit_type   (MooEditor      *editor,
                                                 GType           type);

gboolean             moo_editor_save_copy       (MooEditor      *editor,
                                                 MooEdit        *doc,
                                                 const char     *filename,
                                                 const char     *encoding,
                                                 GError        **error);

void                 moo_editor_apply_prefs     (MooEditor      *editor);
void                 moo_editor_queue_apply_prefs (MooEditor    *editor);

void                _moo_editor_load_session    (MooEditor      *editor,
                                                 MooMarkupNode  *xml);
void                _moo_editor_save_session    (MooEditor      *editor,
                                                 MooMarkupNode  *xml);


enum {
    MOO_EDIT_OPEN_NEW_WINDOW = 1 << 0,
    MOO_EDIT_OPEN_NEW_TAB    = 1 << 1,
    MOO_EDIT_OPEN_RELOAD     = 1 << 2
};

void            _moo_editor_open_uri        (MooEditor      *editor,
                                             const char     *filename,
                                             const char     *encoding,
                                             guint           line,
                                             guint           options);


G_END_DECLS

#endif /* MOO_EDITOR_H */
