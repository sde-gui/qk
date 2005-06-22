/*
 *   mooedit/mooeditor.c
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

#define MOOEDIT_COMPILATION
#include "mooedit/mooeditor.h"
#include "mooui/moouiobject.h"
#include "mooutils/moocompat.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moowin.h"


struct _MooEditorPrivate {
    GSList          *windows;
    char            *app_name;
    MooEditLangMgr  *lang_mgr;
    MooUIXML        *ui_xml;
};


static void             moo_editor_finalize (GObject        *object);

static void             add_window          (MooEditor      *editor,
                                             MooEditWindow  *window);
static void             remove_window       (MooEditor      *editor,
                                             MooEditWindow  *window);
static gboolean         contains_window     (MooEditor      *editor,
                                             MooEditWindow  *window);
static MooEditWindow   *get_top_window      (MooEditor      *editor);
static gboolean         window_closed       (MooEditor      *editor,
                                             MooEditWindow  *window);


enum {
    NEW_WINDOW,
    NEW_DOCUMENT,
    ALL_WINDOWS_CLOSED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];


/* MOO_TYPE_EDITOR */
G_DEFINE_TYPE (MooEditor, moo_editor, G_TYPE_OBJECT)


static void moo_editor_class_init (MooEditorClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_editor_finalize;

    klass->new_window = NULL;
    klass->new_document = NULL;
    klass->all_windows_closed = NULL;

    signals[NEW_WINDOW] =
            g_signal_new ("new-window",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooEditorClass, new_window),
                          NULL, NULL,
                          _moo_marshal_VOID__OBJECT,
                          /* G_TYPE_OBJECT to not put MOO_TYPE_WHATEVER into marshals.list */
                          G_TYPE_NONE, 1, G_TYPE_OBJECT);

    signals[NEW_DOCUMENT] =
            g_signal_new ("new-document",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooEditorClass, new_window),
                          NULL, NULL,
                          _moo_marshal_VOID__OBJECT,
                          G_TYPE_NONE, 1, G_TYPE_OBJECT);

    signals[ALL_WINDOWS_CLOSED] =
            g_signal_new ("all-windows-closed",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooEditorClass, all_windows_closed),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);
}


static void     moo_editor_init        (MooEditor  *editor)
{
    editor->priv = g_new0 (MooEditorPrivate, 1);
}


static void moo_editor_finalize       (GObject      *object)
{
    MooEditor *editor = MOO_EDITOR (object);

    g_free (editor->priv->app_name);
    if (editor->priv->lang_mgr)
        g_object_unref (editor->priv->lang_mgr);
    if (editor->priv->ui_xml)
        g_object_unref (editor->priv->ui_xml);

    g_free (editor->priv);

    G_OBJECT_CLASS (moo_editor_parent_class)->finalize (object);
}


MooEditor       *moo_editor_new                 (void)
{
    return MOO_EDITOR (g_object_new (MOO_TYPE_EDITOR, NULL));
}


MooEditWindow   *moo_editor_new_window          (MooEditor  *editor)
{
    MooEditWindow *window;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    window = MOO_EDIT_WINDOW (_moo_edit_window_new (editor));
    g_return_val_if_fail (window != NULL, NULL);

    add_window (editor, window);
    gtk_widget_show (GTK_WIDGET (window));

    return window;
}


static void              add_window     (MooEditor      *editor,
                                         MooEditWindow  *window)
{
    MooUIXML *ui_xml;

    g_return_if_fail (!contains_window (editor, window));
    g_return_if_fail (window != NULL);

    /* it may not be destroyed until all windows have been closed*/
    g_object_ref (editor);

    editor->priv->windows =
            g_slist_prepend (editor->priv->windows, window);
    _moo_edit_window_set_app_name (MOO_EDIT_WINDOW (window),
                                   editor->priv->app_name);

    ui_xml = moo_editor_get_ui_xml (editor);
    moo_ui_object_set_ui_xml (MOO_UI_OBJECT (window), ui_xml);

    g_signal_connect_data (window, "close",
                           G_CALLBACK (window_closed), editor,
                           NULL,
                           G_CONNECT_AFTER | G_CONNECT_SWAPPED);
}


static void              remove_window  (MooEditor      *editor,
                                         MooEditWindow  *window)
{
    g_return_if_fail (contains_window (editor, window));
    g_return_if_fail (window != NULL);

    g_object_unref (editor);

    editor->priv->windows = g_slist_remove (editor->priv->windows, window);

    if (!editor->priv->windows)
        g_signal_emit (editor, signals[ALL_WINDOWS_CLOSED], 0, NULL);
}


static MooEditWindow    *get_top_window (MooEditor      *editor)
{
    if (editor->priv->windows)
        return MOO_EDIT_WINDOW (moo_get_top_window (editor->priv->windows));
    else
        return NULL;
}


static gboolean         contains_window     (MooEditor      *editor,
                                             MooEditWindow  *window)
{
    return g_slist_find (editor->priv->windows, window) != NULL;
}


static gboolean         window_closed       (MooEditor      *editor,
                                             MooEditWindow  *window)
{
    g_return_val_if_fail (contains_window (editor, window), TRUE);
    remove_window (editor, window);
    return FALSE;
}


gboolean         moo_editor_open                (MooEditor      *editor,
                                                 MooEditWindow  *window,
                                                 const char     *filename,
                                                 const char     *encoding)
{
    g_return_val_if_fail (MOO_IS_EDITOR (editor), FALSE);
    g_return_val_if_fail (window == NULL || MOO_IS_EDIT_WINDOW (window), FALSE);

    if (!window)
        window = get_top_window (editor);
    if (!window)
        window = moo_editor_new_window (editor);
    g_return_val_if_fail (window != NULL, FALSE);

    gtk_window_present (GTK_WINDOW (window));
    return moo_edit_window_open (window, filename, encoding);
}


MooEdit         *moo_editor_get_active_doc      (MooEditor      *editor)
{
    MooEditWindow *window;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    window = get_top_window (editor);
    if (window)
        return moo_edit_window_get_active_doc (window);
    else
        return NULL;
}


MooEditWindow   *moo_editor_get_active_window   (MooEditor      *editor)
{
    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
    return get_top_window (editor);
}


gboolean         moo_editor_close_all           (MooEditor      *editor)
{
    g_return_val_if_fail (MOO_IS_EDITOR (editor), FALSE);

    GSList *list = g_slist_copy (editor->priv->windows);
    GSList *l;
    for (l = list; l != NULL; l = l->next) {
        MooWindow *win = MOO_WINDOW (l->data);
        if (!(GTK_IN_DESTRUCTION & GTK_OBJECT_FLAGS (win)) &&
              moo_window_close (win))
        {
            g_slist_free (list);
            return FALSE;
        }
    }
    g_slist_free (list);
    return TRUE;
}


void             moo_editor_set_app_name        (MooEditor      *editor,
                                                 const char     *name)
{
    GSList *l;

    g_return_if_fail (MOO_IS_EDITOR (editor));

    g_free (editor->priv->app_name);
    editor->priv->app_name = g_strdup (name);

    for (l = editor->priv->windows; l != NULL; l = l->next)
        _moo_edit_window_set_app_name (MOO_EDIT_WINDOW (l->data),
                                       name);
}


MooEditLangMgr  *moo_editor_get_lang_mgr        (MooEditor      *editor)
{
    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
    if (!editor->priv->lang_mgr)
        editor->priv->lang_mgr = moo_edit_lang_mgr_new ();
    return editor->priv->lang_mgr;
}


MooUIXML        *moo_editor_get_ui_xml          (MooEditor      *editor)
{
    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    if (!editor->priv->ui_xml)
        editor->priv->ui_xml = moo_ui_xml_new ();

    return editor->priv->ui_xml;
}


void             moo_editor_set_ui_xml          (MooEditor      *editor,
                                                 MooUIXML       *xml)
{
    GSList *l;

    g_return_if_fail (MOO_IS_EDITOR (editor));

    if (editor->priv->ui_xml == xml)
        return;

    if (editor->priv->ui_xml)
        g_object_unref (editor->priv->ui_xml);
    editor->priv->ui_xml = xml;
    if (xml)
        g_object_ref (editor->priv->ui_xml);

    for (l = editor->priv->windows; l != NULL; l = l->next)
        moo_ui_object_set_ui_xml (MOO_UI_OBJECT (l->data),
                                  editor->priv->ui_xml);
}
