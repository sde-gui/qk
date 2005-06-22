/*
 *   mooedit/mooeditor.h
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

#ifndef MOOEDIT_MOOEDITOR_H
#define MOOEDIT_MOOEDITOR_H

#include "mooui/moowindow.h"
#include "mooui/moouixml.h"
#include "mooedit/mooedit.h"
#include "mooedit/mooeditlangmgr.h"

G_BEGIN_DECLS

#define MOO_TYPE_EDIT_WINDOW            (moo_edit_window_get_type ())
#define MOO_EDIT_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_EDIT_WINDOW, MooEditWindow))
#define MOO_EDIT_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_EDIT_WINDOW, MooEditWindowClass))
#define MOO_IS_EDIT_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_EDIT_WINDOW))
#define MOO_IS_EDIT_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_EDIT_WINDOW))
#define MOO_EDIT_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_EDIT_WINDOW, MooEditWindowClass))

#define MOO_TYPE_EDITOR                 (moo_editor_get_type ())
#define MOO_EDITOR(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_EDITOR, MooEditor))
#define MOO_EDITOR_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_EDITOR, MooEditorClass))
#define MOO_IS_EDITOR(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_EDITOR))
#define MOO_IS_EDITOR_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_EDITOR))
#define MOO_EDITOR_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_EDITOR, MooEditorClass))


typedef struct _MooEditWindow           MooEditWindow;
typedef struct _MooEditWindowPrivate    MooEditWindowPrivate;
typedef struct _MooEditWindowClass      MooEditWindowClass;
typedef struct _MooEditor               MooEditor;
typedef struct _MooEditorPrivate        MooEditorPrivate;
typedef struct _MooEditorClass          MooEditorClass;

struct _MooEditWindow
{
    MooWindow               parent;
    MooEditWindowPrivate   *priv;
};

struct _MooEditWindowClass
{
    MooWindowClass          parent_class;
};

struct _MooEditor
{
    GObject                 parent;
    MooEditorPrivate       *priv;
};

struct _MooEditorClass
{
    GObjectClass            parent_class;

    void (*new_window)          (MooEditor       *editor,
                                 MooEditWindow   *window);
    void (*new_document)        (MooEditor       *editor,
                                 MooEdit         *doc);
    void (*all_windows_closed)  (MooEditor       *editor);
};


GType            moo_editor_get_type            (void) G_GNUC_CONST;
GType            moo_edit_window_get_type       (void) G_GNUC_CONST;

/*****************************************************************************/
/* MooEditor
 */

MooEditor       *moo_editor_new                 (void);

MooEditWindow   *moo_editor_new_window          (MooEditor      *editor);
gboolean         moo_editor_open                (MooEditor      *editor,
                                                 MooEditWindow  *window,
                                                 const char     *filename,
                                                 const char     *encoding);

MooEdit         *moo_editor_get_active_doc      (MooEditor      *editor);
MooEditWindow   *moo_editor_get_active_window   (MooEditor      *editor);

gboolean         moo_editor_close_all           (MooEditor      *editor);

void             moo_editor_set_app_name        (MooEditor      *editor,
                                                 const char     *name);

MooEditLangMgr  *moo_editor_get_lang_mgr        (MooEditor      *editor);

MooUIXML        *moo_editor_get_ui_xml          (MooEditor      *editor);
void             moo_editor_set_ui_xml          (MooEditor      *editor,
                                                 MooUIXML       *xml);


/*****************************************************************************/
/* MooEditWindow
 */

gboolean         moo_edit_window_open           (MooEditWindow  *window,
                                                 const char     *filename,
                                                 const char     *encoding);

MooEdit         *moo_edit_window_get_active_doc (MooEditWindow  *window);

gboolean         moo_edit_window_close_doc      (MooEditWindow  *window,
                                                 MooEdit        *doc);


#ifdef MOOEDIT_COMPILATION
GtkWidget       *_moo_edit_window_new           (MooEditor      *editor);

void             _moo_edit_window_set_app_name  (MooEditWindow  *window,
                                                 const char     *name);
#endif /* MOOEDIT_COMPILATION */


G_END_DECLS

#endif /* MOOEDIT_MOOEDITOR_H */
