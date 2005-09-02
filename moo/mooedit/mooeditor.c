/*
 *   mooeditor.c
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
#include "mooedit/mooeditdialogs.h"
#include "mooedit/mooeditfileops.h"
#include "mooui/moouiobject.h"
#include "mooui/moomenuaction.h"
#include "mooutils/moocompat.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moowin.h"
#include "mooutils/moosignal.h"
#include <string.h>


typedef struct {
    MooEditWindow *window;
    GSList        *docs;
} WindowInfo;

static WindowInfo   *window_info_new        (MooEditWindow  *win);
static void          window_info_free       (WindowInfo     *win);
static void          window_info_add        (WindowInfo     *win,
                                             MooEdit        *doc);
static void          window_info_remove     (WindowInfo     *win,
                                             MooEdit        *doc);
static MooEdit      *window_info_find       (WindowInfo     *win,
                                             const char     *filename);

static void          window_list_free       (MooEditor      *editor);
static void          window_list_delete     (MooEditor      *editor,
                                             WindowInfo     *win);
static WindowInfo   *window_list_add        (MooEditor      *editor,
                                             MooEditWindow  *win);
static WindowInfo   *window_list_find       (MooEditor      *editor,
                                             MooEditWindow  *win);
static WindowInfo   *window_list_find_doc   (MooEditor      *editor,
                                             MooEdit        *edit);
static WindowInfo   *window_list_find_filename (MooEditor   *editor,
                                             const char     *filename,
                                             MooEdit       **edit);

static GtkMenuItem  *create_recent_menu     (MooEditWindow  *window,
                                             MooAction      *action);

static void          moo_editor_add_window  (MooEditor      *editor,
                                             MooEditWindow  *window);
static void          moo_editor_add_doc     (MooEditor      *editor,
                                             MooEditWindow  *window,
                                             MooEdit        *doc,
                                             MooEditLoader  *loader,
                                             MooEditSaver   *saver);
static void          do_close_window        (MooEditor      *editor,
                                             MooEditWindow  *window);
static void          do_close_doc           (MooEditor      *editor,
                                             MooEdit        *doc);
static gboolean      close_docs_real        (MooEditor      *editor,
                                             GSList         *docs);
static GSList       *find_modified          (GSList         *docs);
static MooEditLoader*get_loader             (MooEditor      *editor,
                                             MooEdit        *doc);
static MooEditSaver *get_saver              (MooEditor      *editor,
                                             MooEdit        *doc);


struct _MooEditorPrivate {
    GSList          *windows; /* WindowInfo* */
    GHashTable      *loaders;
    GHashTable      *savers;
    char            *app_name;
    MooEditLangMgr  *lang_mgr;
    MooUIXML        *ui_xml;
    MooFilterMgr    *filter_mgr;
    MooRecentMgr    *recent_mgr;
    gboolean         open_single;
    gboolean         allow_empty_window;
};


static void             moo_editor_finalize (GObject        *object);

static void             open_recent         (MooEditor      *editor,
                                             MooEditFileInfo *info,
                                             MooEditWindow  *window);


enum {
    ALL_WINDOWS_CLOSED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];


/* MOO_TYPE_EDITOR */
G_DEFINE_TYPE (MooEditor, moo_editor, G_TYPE_OBJECT)


static void moo_editor_class_init (MooEditorClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GObjectClass *edit_window_class;

    gobject_class->finalize = moo_editor_finalize;

    signals[ALL_WINDOWS_CLOSED] =
            moo_signal_new_cb ("all-windows-closed",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST,
                               NULL, NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);

    edit_window_class = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);
    moo_ui_object_class_new_action (edit_window_class,
                                    "action-type::", MOO_TYPE_MENU_ACTION,
                                    "id", "OpenRecent",
                                    "create-menu-func", create_recent_menu,
                                    NULL);
    g_type_class_unref (edit_window_class);
}


static void     moo_editor_init        (MooEditor  *editor)
{
    editor->priv = g_new0 (MooEditorPrivate, 1);

    editor->priv->filter_mgr = moo_filter_mgr_new ();
    editor->priv->recent_mgr = moo_recent_mgr_new ();
    g_signal_connect_swapped (editor->priv->recent_mgr, "open-recent",
                              G_CALLBACK (open_recent), editor);
    editor->priv->windows = NULL;
    editor->priv->open_single = TRUE;

    editor->priv->loaders =
            g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                   NULL, (GDestroyNotify) moo_edit_loader_unref);
    editor->priv->savers =
            g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                   NULL, (GDestroyNotify) moo_edit_saver_unref);
}


static void moo_editor_finalize       (GObject      *object)
{
    MooEditor *editor = MOO_EDITOR (object);

    g_free (editor->priv->app_name);

    if (editor->priv->lang_mgr)
        g_object_unref (editor->priv->lang_mgr);
    if (editor->priv->ui_xml)
        g_object_unref (editor->priv->ui_xml);
    if (editor->priv->filter_mgr)
        g_object_unref (editor->priv->filter_mgr);
    if (editor->priv->recent_mgr)
        g_object_unref (editor->priv->recent_mgr);

    /* XXX it must be empty here */
    window_list_free (editor);

    g_hash_table_destroy (editor->priv->loaders);
    g_hash_table_destroy (editor->priv->savers);

    g_free (editor->priv);

    G_OBJECT_CLASS (moo_editor_parent_class)->finalize (object);
}


MooEditor       *moo_editor_new                 (void)
{
    return MOO_EDITOR (g_object_new (MOO_TYPE_EDITOR, NULL));
}


static MooEditWindow    *get_top_window (MooEditor      *editor)
{
    GSList *list = NULL, *l;
    GtkWindow *window;

    if (!editor->priv->windows)
        return NULL;

    for (l = editor->priv->windows; l != NULL; l = l->next)
    {
        WindowInfo *info = l->data;
        list = g_slist_prepend (list, info->window);
    }

    list = g_slist_reverse (list);
    window = moo_get_top_window (list);

    g_slist_free (list);
    return MOO_EDIT_WINDOW (window);
}


static void file_info_list_free (GSList *list)
{
    g_slist_foreach (list, (GFunc) moo_edit_file_info_free, NULL);
    g_slist_free (list);
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


MooFilterMgr    *moo_editor_get_filter_mgr      (MooEditor      *editor)
{
    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
    return editor->priv->filter_mgr;
}


MooRecentMgr    *moo_editor_get_recent_mgr      (MooEditor      *editor)
{
    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
    return editor->priv->recent_mgr;
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

    if (editor->priv->ui_xml)
        g_object_ref (editor->priv->ui_xml);

    for (l = editor->priv->windows; l != NULL; l = l->next)
        moo_ui_object_set_ui_xml (MOO_UI_OBJECT (l->data),
                                  editor->priv->ui_xml);
}


static WindowInfo   *window_info_new    (MooEditWindow  *win)
{
    WindowInfo *w = g_new0 (WindowInfo, 1);
    w->window = win;
    return w;
}

static void          window_info_free   (WindowInfo     *win)
{
    if (win)
    {
        g_slist_free (win->docs);
        g_free (win);
    }
}

static void          window_info_add        (WindowInfo     *win,
                                             MooEdit        *edit)
{
    g_return_if_fail (!g_slist_find (win->docs, edit));
    win->docs = g_slist_prepend (win->docs, edit);
}

static void          window_info_remove     (WindowInfo     *win,
                                             MooEdit        *edit)
{
    g_return_if_fail (g_slist_find (win->docs, edit));
    win->docs = g_slist_remove (win->docs, edit);
}


static int edit_and_file_cmp (MooEdit *edit, const char *filename)
{
    const char *edit_filename;
    g_return_val_if_fail (MOO_IS_EDIT (edit) && filename != NULL, TRUE);
    edit_filename = moo_edit_get_filename (edit);
    if (edit_filename)
        return strcmp (edit_filename, filename);
    else
        return TRUE;
}

static MooEdit      *window_info_find       (WindowInfo     *win,
                                             const char     *filename)
{
    GSList *l;
    g_return_val_if_fail (win != NULL && filename != NULL, NULL);
    l = g_slist_find_custom (win->docs, filename,
                             (GCompareFunc) edit_and_file_cmp);
    return l ? l->data : NULL;
}


static void          window_list_free   (MooEditor      *editor)
{
    g_slist_foreach (editor->priv->windows,
                     (GFunc) window_info_free, NULL);
    g_slist_free (editor->priv->windows);
    editor->priv->windows = NULL;
}

static void          window_list_delete (MooEditor      *editor,
                                         WindowInfo     *win)
{
    g_return_if_fail (g_slist_find (editor->priv->windows, win));
    window_info_free (win);
    editor->priv->windows =
            g_slist_remove (editor->priv->windows, win);
}

static WindowInfo   *window_list_add    (MooEditor      *editor,
                                         MooEditWindow  *win)
{
    WindowInfo *w = window_info_new (win);
    editor->priv->windows =
            g_slist_prepend (editor->priv->windows, w);
    return w;
}

static int window_cmp (WindowInfo *w, MooEditWindow *e)
{
    g_return_val_if_fail (w != NULL, TRUE);
    return !(w->window == e);
}

static WindowInfo   *window_list_find   (MooEditor      *editor,
                                         MooEditWindow  *win)
{
    GSList *l = g_slist_find_custom (editor->priv->windows, win,
                                     (GCompareFunc) window_cmp);
    if (l)
        return l->data;
    else
        return NULL;
}

static int doc_cmp (WindowInfo *w, MooEdit *e)
{
    g_return_val_if_fail (w != NULL, 1);
    return !g_slist_find (w->docs, e);
}

static WindowInfo   *window_list_find_doc   (MooEditor      *editor,
                                             MooEdit        *edit)
{
    GSList *l = g_slist_find_custom (editor->priv->windows, edit,
                                     (GCompareFunc) doc_cmp);
    if (l)
        return l->data;
    else
        return NULL;
}


static int filename_and_doc_cmp (WindowInfo *w, gpointer user_data)
{
    struct {
        const char *filename;
        MooEdit    *edit;
    } *data = user_data;

    g_return_val_if_fail (w != NULL, TRUE);

    data->edit = window_info_find (w, data->filename);
    return data->edit == NULL;
}

static WindowInfo   *window_list_find_filename (MooEditor   *editor,
                                                const char     *filename,
                                                MooEdit       **edit)
{
    struct {
        const char *filename;
        MooEdit    *edit;
    } data = {filename, NULL};

    GSList *l = g_slist_find_custom (editor->priv->windows, &data,
                                     (GCompareFunc) filename_and_doc_cmp);
    if (l)
    {
        g_assert (data.edit != NULL);
        if (edit) *edit = data.edit;
        return l->data;
    }
    else
    {
        return NULL;
    }
}


static GtkMenuItem  *create_recent_menu     (MooEditWindow  *window,
                                             G_GNUC_UNUSED MooAction *action)
{
    MooEditor *editor = _moo_edit_window_get_editor (window);
    g_return_val_if_fail (editor != NULL, NULL);
    return moo_recent_mgr_create_menu (editor->priv->recent_mgr, window);
}


static void             open_recent         (MooEditor       *editor,
                                             MooEditFileInfo *info,
                                             MooEditWindow   *window)
{
    WindowInfo *win_info;
    GSList *files;

    g_return_if_fail (MOO_IS_EDITOR (editor));
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (info != NULL);

    win_info = window_list_find (editor, window);
    g_return_if_fail (win_info != NULL);

    files = g_slist_prepend (NULL, info);
    moo_editor_open (editor, window, GTK_WIDGET (window), files);
    g_slist_free (files);
}


/*****************************************************************************/

static void          moo_editor_add_window  (MooEditor      *editor,
                                             MooEditWindow  *window)
{
    g_return_if_fail (window_list_find (editor, window) == NULL);
    window_list_add (editor, window);
}


static void          moo_editor_add_doc     (MooEditor      *editor,
                                             MooEditWindow  *window,
                                             MooEdit        *doc,
                                             MooEditLoader  *loader,
                                             MooEditSaver   *saver)
{
    WindowInfo *info = window_list_find (editor, window);

    g_return_if_fail (info != NULL);
    g_return_if_fail (g_slist_find (info->docs, doc) == NULL);

    window_info_add (info, doc);

    g_hash_table_insert (editor->priv->loaders, doc, moo_edit_loader_ref (loader));
    g_hash_table_insert (editor->priv->savers, doc, moo_edit_saver_ref (saver));
}


MooEditWindow   *moo_editor_new_window      (MooEditor      *editor)
{
    MooEditWindow *window;
    MooEdit *doc;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    window = _moo_edit_window_new (editor);
    moo_editor_add_window (editor, window);

    doc = MOO_EDIT (moo_edit_new ());
    _moo_edit_window_insert_doc (window, doc, -1);
    moo_editor_add_doc (editor, window, doc,
                        moo_edit_loader_get_default (),
                        moo_edit_saver_get_default ());

    return window;
}


void             moo_editor_open            (MooEditor      *editor,
                                             MooEditWindow  *window,
                                             GtkWidget      *parent,
                                             GSList         *files)
{
    GSList *l;
    MooEdit *doc;
    MooEditLoader *loader;
    MooEditSaver *saver;

    g_return_if_fail (MOO_IS_EDITOR (editor));
    g_return_if_fail (!window || MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (!parent || GTK_IS_WIDGET (parent));
    g_return_if_fail (!window || window_list_find (editor, window) != NULL);

    if (window && !parent)
        parent = GTK_WIDGET (window);

    if (!files)
    {
        files = moo_edit_open_dialog (parent, editor->priv->filter_mgr);

        if (!files)
            return;

        moo_editor_open (editor, window, parent, files);
        file_info_list_free (files);
        return;
    }

    loader = moo_edit_loader_get_default ();
    saver = moo_edit_saver_get_default ();

    for (l = files; l != NULL; l = l->next)
    {
        MooEditFileInfo *info = l->data;
        GError *error = NULL;

        doc = MOO_EDIT (moo_edit_new ());
        gtk_object_sink (g_object_ref (doc));

        /* XXX open_single */
        if (!moo_edit_load (loader, doc, info->filename, info->encoding, &error))
        {
            moo_edit_open_error_dialog (parent, error ? error->message : NULL);
        }
        else
        {
            if (!window)
            {
                window = _moo_edit_window_new (editor);
                moo_editor_add_window (editor, window);
            }

            _moo_edit_window_insert_doc (window, doc, -1);
            moo_editor_add_doc (editor, window, doc, loader, saver);

            parent = GTK_WIDGET (window);
        }

        g_object_unref (doc);
    }
}


MooEdit         *moo_editor_get_active_doc  (MooEditor      *editor)
{
    MooEditWindow *window = moo_editor_get_active_window (editor);
    return window ? moo_edit_window_get_active_doc (window) : NULL;
}


MooEditWindow   *moo_editor_get_active_window (MooEditor    *editor)
{
    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
    return get_top_window (editor);
}


void             moo_editor_set_active_window (MooEditor    *editor,
                                               MooEditWindow  *window)
{
    WindowInfo *info;

    g_return_if_fail (MOO_IS_EDITOR (editor));
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));

    info = window_list_find (editor, window);
    g_return_if_fail (info != NULL);

    gtk_window_present (GTK_WINDOW (info->window));
}


void             moo_editor_set_active_doc  (MooEditor      *editor,
                                             MooEdit        *doc)
{
    WindowInfo *info;

    g_return_if_fail (MOO_IS_EDITOR (editor));
    g_return_if_fail (MOO_IS_EDIT (doc));

    info = window_list_find_doc (editor, doc);
    g_return_if_fail (info != NULL);

    gtk_window_present (GTK_WINDOW (info->window));
    moo_edit_window_set_active_doc (info->window, doc);
}


gboolean         moo_editor_close_window    (MooEditor      *editor,
                                             MooEditWindow  *window)
{
    WindowInfo *info;
    MooEditDialogResponse response;
    GSList *modified;
    gboolean do_close = FALSE;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), FALSE);
    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), FALSE);

    info = window_list_find (editor, window);
    g_return_val_if_fail (info != NULL, FALSE);

    modified = find_modified (info->docs);

    if (!modified)
    {
        do_close = TRUE;
    }
    else if (!modified->next)
    {
        moo_edit_window_set_active_doc (info->window, modified->data);
        response = moo_edit_save_changes_dialog (modified->data);

        switch (response)
        {
            case MOO_EDIT_RESPONSE_SAVE:
                if (_moo_editor_save (editor, modified->data))
                    do_close = TRUE;
                break;

            case MOO_EDIT_RESPONSE_CANCEL:
                break;

            default:
                do_close = TRUE;
                break;
        }
    }
    else
    {
        GSList *to_save = NULL, *l;
        gboolean saved = TRUE;

        response = moo_edit_save_multiple_changes_dialog (modified, &to_save);

        switch (response)
        {
            case MOO_EDIT_RESPONSE_SAVE:
                for (l = to_save; l != NULL; l = l->next)
                    if (!_moo_editor_save (editor, l->data))
                    {
                        saved = FALSE;
                        break;
                    }

                if (saved)
                    do_close = TRUE;

                g_slist_free (to_save);
                break;

            case MOO_EDIT_RESPONSE_CANCEL:
                break;

            default:
                do_close = TRUE;
                break;
        }
    }

    if (do_close)
        do_close_window (editor, window);

    g_slist_free (modified);
    return do_close;
}


static void          do_close_window        (MooEditor      *editor,
                                             MooEditWindow  *window)
{
    WindowInfo *info;
    GSList *l, *list;

    info = window_list_find (editor, window);
    g_return_if_fail (info != NULL);

    list = g_slist_copy (info->docs);

    for (l = list; l != NULL; l = l->next)
        do_close_doc (editor, l->data);

    window_list_delete (editor, info);

    /* XXX */
    gtk_widget_destroy (GTK_WIDGET (window));

    g_slist_free (list);
}


static void          do_close_doc           (MooEditor      *editor,
                                             MooEdit        *doc)
{
    WindowInfo *info = window_list_find_doc (editor, doc);
    g_return_if_fail (info != NULL);
    window_info_remove (info, doc);
    _moo_edit_window_remove_doc (info->window, doc);
    g_hash_table_remove (editor->priv->loaders, doc);
    g_hash_table_remove (editor->priv->savers, doc);
}


gboolean         moo_editor_close_doc       (MooEditor      *editor,
                                             MooEdit        *doc)
{
    gboolean result;
    GSList *list;

    list = g_slist_prepend (NULL, doc);
    result = moo_editor_close_docs (editor, list);

    g_slist_free (list);
    return result;
}


gboolean         moo_editor_close_docs      (MooEditor      *editor,
                                             GSList         *list)
{
    WindowInfo *info;
    GSList *l;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), FALSE);
    g_return_val_if_fail (list != NULL, FALSE);

    for (l = list; l != NULL; l = l->next)
    {
        g_return_val_if_fail (MOO_IS_EDIT (l->data), FALSE);
        g_return_val_if_fail (window_list_find (editor, l->data) != NULL, FALSE);
    }

    /* do i care? */
    info = window_list_find (editor, list->data);
    g_return_val_if_fail (info != NULL, FALSE);

    if (close_docs_real (editor, list))
    {
        if (!moo_edit_window_num_docs (info->window) &&
             !editor->priv->allow_empty_window)
        {
            MooEdit *doc = moo_edit_new ();
            _moo_edit_window_insert_doc (info->window, doc, -1);
            moo_editor_add_doc (editor, info->window, doc,
                                moo_edit_loader_get_default (),
                                moo_edit_saver_get_default ());
        }

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


static gboolean      close_docs_real        (MooEditor      *editor,
                                             GSList         *docs)
{
    MooEditDialogResponse response;
    GSList *modified, *l;
    gboolean do_close = FALSE;

    modified = find_modified (docs);

    if (!modified)
    {
        do_close = TRUE;
    }
    else if (!modified->next)
    {
        WindowInfo *info = window_list_find (editor, modified->data);
        g_return_val_if_fail (info != NULL, FALSE);

        moo_edit_window_set_active_doc (info->window, modified->data);
        response = moo_edit_save_changes_dialog (modified->data);

        switch (response)
        {
            case MOO_EDIT_RESPONSE_SAVE:
                if (_moo_editor_save (editor, modified->data))
                    do_close = TRUE;
                break;

            case MOO_EDIT_RESPONSE_CANCEL:
                break;

            default:
                do_close = TRUE;
                break;
        }
    }
    else
    {
        GSList *to_save = NULL;
        gboolean saved = TRUE;

        response = moo_edit_save_multiple_changes_dialog (modified, &to_save);

        switch (response)
        {
            case MOO_EDIT_RESPONSE_SAVE:
                for (l = to_save; l != NULL; l = l->next)
                    if (!_moo_editor_save (editor, l->data))
                {
                    saved = FALSE;
                    break;
                }

                if (saved)
                    do_close = TRUE;

                g_slist_free (to_save);
                break;

            case MOO_EDIT_RESPONSE_CANCEL:
                break;

            default:
                do_close = TRUE;
                break;
        }
    }

    if (do_close)
        for (l = docs; l != NULL; l = l->next)
            do_close_doc (editor, l->data);

    g_slist_free (modified);
    return do_close;
}


static GSList       *find_modified          (GSList         *docs)
{
    GSList *modified = NULL, *l;
    for (l = docs; l != NULL; l = l->next)
        if (MOO_EDIT_IS_MODIFIED (l->data) && !MOO_EDIT_IS_CLEAN (l->data))
            modified = g_slist_prepend (modified, l->data);
    return g_slist_reverse (modified);
}


gboolean         moo_editor_close_all       (MooEditor      *editor)
{
    GSList *windows, *l;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), FALSE);

    windows = moo_editor_list_windows (editor);

    for (l = windows; l != NULL; l = l->next)
    {
        if (!moo_editor_close_window (editor, l->data))
        {
            g_slist_free (windows);
            return FALSE;
        }
    }

    g_slist_free (windows);
    return TRUE;
}


GSList          *moo_editor_list_windows    (MooEditor      *editor)
{
    GSList *windows = NULL, *l;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    for (l = editor->priv->windows; l != NULL; l = l->next)
    {
        WindowInfo *info = l->data;
        windows = g_slist_prepend (windows, info->window);
    }

    return g_slist_reverse (windows);
}


void             moo_editor_open_file       (MooEditor      *editor,
                                             MooEditWindow  *window,
                                             GtkWidget      *parent,
                                             const char     *filename,
                                             const char     *encoding)
{
    MooEditFileInfo *info;
    GSList *list;

    if (!filename)
        return moo_editor_open (editor, window, parent, NULL);

    info = moo_edit_file_info_new (filename, encoding);
    list = g_slist_prepend (NULL, info);
    moo_editor_open (editor, window, parent, list);
    moo_edit_file_info_free (info);
    g_slist_free (list);
}


void        _moo_editor_reload      (MooEditor      *editor,
                                     MooEdit        *doc)
{
    WindowInfo *info;
    MooEditLoader *loader;
    GError *error = NULL;

    g_return_if_fail (MOO_IS_EDITOR (editor));

    info = window_list_find_doc (editor, doc);
    g_return_if_fail (info != NULL);

    loader = get_loader (editor, doc);
    g_return_if_fail (loader != NULL);

    /* XXX */
    g_return_if_fail (moo_edit_get_filename (doc) != NULL);

    if (!MOO_EDIT_IS_CLEAN (doc) && MOO_EDIT_IS_MODIFIED (doc) &&
         !moo_edit_reload_dialog (doc))
            return;

    if (!moo_edit_reload (loader, doc, &error))
    {
        moo_edit_reload_error_dialog (GTK_WIDGET (doc),
                                      error ? error->message : NULL);
        if (error)
            g_error_free (error);
    }
}


gboolean    _moo_editor_save        (MooEditor      *editor,
                                     MooEdit        *doc)
{
    WindowInfo *info;
    MooEditSaver *saver;
    GError *error = NULL;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), FALSE);

    info = window_list_find_doc (editor, doc);
    g_return_val_if_fail (info != NULL, FALSE);

    saver = get_saver (editor, doc);
    g_return_val_if_fail (saver != NULL, FALSE);

    if (!moo_edit_get_filename (doc))
        return _moo_editor_save_as (editor, doc, NULL, NULL);

    if (!moo_edit_save (saver, doc, moo_edit_get_filename (doc),
                        moo_edit_get_encoding (doc), &error))
    {
        moo_edit_save_error_dialog (GTK_WIDGET (doc),
                                    error ? error->message : NULL);
        if (error)
            g_error_free (error);
        return FALSE;
    }

    return TRUE;
}


gboolean    _moo_editor_save_as     (MooEditor      *editor,
                                     MooEdit        *doc,
                                     const char     *filename,
                                     const char     *encoding)
{
    WindowInfo *info;
    MooEditSaver *saver;
    GError *error = NULL;
    MooEditFileInfo *file_info = NULL;
    gboolean result = TRUE;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), FALSE);

    info = window_list_find_doc (editor, doc);
    g_return_val_if_fail (info != NULL, FALSE);

    saver = get_saver (editor, doc);
    g_return_val_if_fail (saver != NULL, FALSE);

    if (!filename)
    {
        file_info = moo_edit_save_as_dialog (doc, editor->priv->filter_mgr);

        if (!file_info)
            return FALSE;

        filename = file_info->filename;
        encoding = file_info->encoding;
    }

    if (!moo_edit_save (saver, doc, filename, encoding, &error))
    {
        moo_edit_save_error_dialog (GTK_WIDGET (doc),
                                    error ? error->message : NULL);
        if (error)
            g_error_free (error);
        result = FALSE;
        goto out;
    }

    result = TRUE;

    /* fall through */
out:
    moo_edit_file_info_free (file_info);
    return result;
}


static MooEditLoader*get_loader             (MooEditor      *editor,
                                             MooEdit        *doc)
{
    return g_hash_table_lookup (editor->priv->loaders, doc);
}


static MooEditSaver *get_saver              (MooEditor      *editor,
                                             MooEdit        *doc)
{
    return g_hash_table_lookup (editor->priv->savers, doc);
}
