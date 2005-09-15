/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooeditwindow.c
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
#include "mooedit-private.h"
#include "mooedit/mooeditor.h"
#include "mooedit/moobigpaned.h"
#include "mooedit/moonotebook.h"
#include "mooedit/moofileview/moofileview.h"
#include "mooedit/moofileview/moobookmarkmgr.h"
#include "mooutils/moostock.h"
#include "mooutils/moomarshals.h"
#include "mooui/moomenuaction.h"
#include "mooui/moouiobject-impl.h"
#include <string.h>


#define ACTIVE_DOC moo_edit_window_get_active_doc
#define ACTIVE_PAGE(window) (moo_notebook_get_current_page (window->priv->notebook))


struct _MooEditWindowPrivate {
    MooEditor *editor;
    GtkStatusbar *statusbar;
    guint statusbar_context_id;
    MooNotebook *notebook;
    char *prefix;
    gboolean use_fullname;
    GHashTable *panes;
    GtkWidget *languages_menu_item;
    GHashTable *lang_menu_items;
    GtkWidget *none_lang_item;
};


GObject        *moo_edit_window_constructor (GType                  type,
                                             guint                  n_props,
                                             GObjectConstructParam *props);
static void     moo_edit_window_finalize    (GObject        *object);

static void     moo_edit_window_set_property(GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec);
static void     moo_edit_window_get_property(GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec);


static gboolean moo_edit_window_close       (MooEditWindow  *window);


static GtkMenuItem *create_lang_menu    (MooEditWindow      *window);
static void     lang_menu_item_toggled  (GtkCheckMenuItem   *item,
                                         MooEditWindow      *window);
static void     active_tab_lang_changed (MooEditWindow      *window);

static void     setup_notebook          (MooEditWindow      *window);
static void     update_window_title     (MooEditWindow      *window);

static void     notebook_switch_page    (MooNotebook        *notebook,
                                         gpointer            whatever,
                                         guint               page_num,
                                         MooEditWindow      *window);
static gboolean notebook_populate_popup (MooNotebook        *notebook,
                                         GtkWidget          *child,
                                         GtkMenu            *menu,
                                         MooEditWindow      *window);

static void     edit_changed            (MooEditWindow      *window,
                                         MooEdit            *doc);
static void     edit_filename_changed   (MooEditWindow      *window);
static void     edit_can_undo_redo      (MooEditWindow      *window);
static void     edit_has_selection      (MooEditWindow      *window);
static void     edit_has_text           (MooEditWindow      *window);
static GtkWidget *create_tab_label      (MooEdit            *edit);
static void     update_tab_label        (MooEditWindow      *window,
                                         MooEdit            *doc);
static void     edit_cursor_moved       (MooEditWindow      *window,
                                         GtkTextIter        *iter,
                                         MooEdit            *edit);
static void     edit_lang_changed       (MooEditWindow      *window,
                                         MooEdit            *edit);

static void     update_statusbar        (MooEditWindow      *window);
static MooEdit *get_nth_tab             (MooEditWindow      *window,
                                         guint               n);
static int      get_page_num            (MooEditWindow      *window,
                                         MooEdit            *doc);


/* actions */
static void moo_edit_window_new         (MooEditWindow      *window);
static void moo_edit_window_new_tab     (MooEditWindow      *window);
static void moo_edit_window_open        (MooEditWindow      *window);
static void moo_edit_window_reload      (MooEditWindow      *window);
static void moo_edit_window_save        (MooEditWindow      *window);
static void moo_edit_window_save_as     (MooEditWindow      *window);
static void moo_edit_window_close_tab   (MooEditWindow      *window);
static void moo_edit_window_close_all   (MooEditWindow      *window);
static void moo_edit_window_previous_tab(MooEditWindow      *window);
static void moo_edit_window_next_tab    (MooEditWindow      *window);


/* MOO_TYPE_EDIT_WINDOW */
G_DEFINE_TYPE (MooEditWindow, moo_edit_window, MOO_TYPE_WINDOW)

enum {
    PROP_0,
    PROP_EDITOR,
    PROP_ACTIVE_DOC
};

enum {
    NEW_DOC,
    CLOSE_DOC,
    CLOSE_DOC_AFTER,
    NUM_SIGNALS
};

static guint signals[NUM_SIGNALS];


static void moo_edit_window_class_init (MooEditWindowClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    MooWindowClass *window_class = MOO_WINDOW_CLASS (klass);

    gobject_class->constructor = moo_edit_window_constructor;
    gobject_class->finalize = moo_edit_window_finalize;
    gobject_class->set_property = moo_edit_window_set_property;
    gobject_class->get_property = moo_edit_window_get_property;

    window_class->close = (gboolean (*) (MooWindow*))moo_edit_window_close;

    g_object_class_install_property (gobject_class,
                                     PROP_EDITOR,
                                     g_param_spec_object ("editor",
                                             "editor",
                                             "editor",
                                             MOO_TYPE_EDITOR,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_ACTIVE_DOC,
                                     g_param_spec_object ("active-doc",
                                             "active-doc",
                                             "active-doc",
                                             MOO_TYPE_EDIT,
                                             G_PARAM_READWRITE));

    signals[NEW_DOC] =
            g_signal_new ("new-doc",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooEditWindowClass, new_doc),
                          NULL, NULL,
                          _moo_marshal_VOID__OBJECT,
                          G_TYPE_NONE, 1,
                          MOO_TYPE_EDIT);

    signals[CLOSE_DOC] =
            g_signal_new ("close-doc",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooEditWindowClass, close_doc),
                          NULL, NULL,
                          _moo_marshal_VOID__OBJECT,
                          G_TYPE_NONE, 1,
                          MOO_TYPE_EDIT);

    signals[CLOSE_DOC_AFTER] =
            g_signal_new ("close-doc-after",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooEditWindowClass, close_doc_after),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    moo_ui_object_class_init (gobject_class, "Editor", "Editor");

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "NewWindow",
                                    "name", "New Window",
                                    "label", "_New Window",
                                    "tooltip", "Open new editor window",
                                    "icon-stock-id", GTK_STOCK_NEW,
                                    "accel", "<ctrl>N",
                                    "closure::callback", moo_edit_window_new,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "NewTab",
                                    "name", "New Tab",
                                    "label", "New _Tab",
                                    "tooltip", "Create new document tab",
                                    "icon-stock-id", GTK_STOCK_NEW,
                                    "accel", "<ctrl>T",
                                    "closure::callback", moo_edit_window_new_tab,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "Open",
                                    "name", "Open",
                                    "label", "_Open...",
                                    "tooltip", "Open...",
                                    "icon-stock-id", GTK_STOCK_OPEN,
                                    "accel", "<ctrl>O",
                                    "closure::callback", moo_edit_window_open,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "Reload",
                                    "name", "Reload",
                                    "label", "_Reload",
                                    "tooltip", "Reload document",
                                    "icon-stock-id", GTK_STOCK_REFRESH,
                                    "accel", "F5",
                                    "closure::callback", moo_edit_window_reload,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "Save",
                                    "name", "Save",
                                    "label", "_Save",
                                    "tooltip", "Save document",
                                    "icon-stock-id", GTK_STOCK_SAVE,
                                    "accel", "<ctrl>S",
                                    "closure::callback", moo_edit_window_save,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "SaveAs",
                                    "name", "Save As",
                                    "label", "Save _As...",
                                    "tooltip", "Save as...",
                                    "icon-stock-id", GTK_STOCK_SAVE_AS,
                                    "accel", "<ctrl><shift>S",
                                    "closure::callback", moo_edit_window_save_as,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "Close",
                                    "name", "Close",
                                    "label", "_Close",
                                    "tooltip", "Close document",
                                    "icon-stock-id", GTK_STOCK_CLOSE,
                                    "accel", "<ctrl>W",
                                    "closure::callback", moo_edit_window_close_tab,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "CloseAll",
                                    "name", "Close All",
                                    "label", "Close _All",
                                    "tooltip", "Close all documents",
                                    "accel", "<shift><ctrl>W",
                                    "closure::callback", moo_edit_window_close_all,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "Undo",
                                    "name", "Undo",
                                    "label", "_Undo",
                                    "tooltip", "Undo",
                                    "icon-stock-id", GTK_STOCK_UNDO,
                                    "accel", "<ctrl>Z",
                                    "closure::signal", "undo",
                                    "closure::proxy-func", moo_edit_window_get_active_doc,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "Redo",
                                    "name", "Redo",
                                    "label", "_Redo",
                                    "tooltip", "Redo",
                                    "icon-stock-id", GTK_STOCK_REDO,
                                    "accel", "<shift><ctrl>Z",
                                    "closure::signal", "redo",
                                    "closure::proxy-func", moo_edit_window_get_active_doc,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "Cut",
                                    "name", "Cut",
                                    "label", "Cu_t",
                                    "tooltip", "Cut",
                                    "icon-stock-id", GTK_STOCK_CUT,
                                    "accel", "<ctrl>X",
                                    "closure::signal", "cut-clipboard",
                                    "closure::proxy-func", moo_edit_window_get_active_doc,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "Copy",
                                    "name", "Copy",
                                    "label", "_Copy",
                                    "tooltip", "Copy",
                                    "icon-stock-id", GTK_STOCK_COPY,
                                    "accel", "<ctrl>C",
                                    "closure::signal", "copy-clipboard",
                                    "closure::proxy-func", moo_edit_window_get_active_doc,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "Paste",
                                    "name", "Paste",
                                    "label", "_Paste",
                                    "tooltip", "Paste",
                                    "icon-stock-id", GTK_STOCK_PASTE,
                                    "accel", "<ctrl>V",
                                    "closure::signal", "paste-clipboard",
                                    "closure::proxy-func", moo_edit_window_get_active_doc,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "Delete",
                                    "name", "Delete",
                                    "label", "_Delete",
                                    "tooltip", "Delete",
                                    "icon-stock-id", GTK_STOCK_DELETE,
                                    "closure::signal", "delete-selection",
                                    "closure::proxy-func", moo_edit_window_get_active_doc,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "SelectAll",
                                    "name", "Select All",
                                    "label", "Select _All",
                                    "tooltip", "Select all",
                                    "accel", "<ctrl>A",
                                    "closure::callback", moo_text_view_select_all,
                                    "closure::proxy-func", moo_edit_window_get_active_doc,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "PreviousTab",
                                    "name", "Previous Tab",
                                    "label", "_Previous Tab",
                                    "tooltip", "Previous tab",
                                    "icon-stock-id", GTK_STOCK_GO_BACK,
                                    "accel", "<alt>Left",
                                    "closure::callback", moo_edit_window_previous_tab,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "NextTab",
                                    "name", "Next Tab",
                                    "label", "_Next Tab",
                                    "tooltip", "Next tab",
                                    "icon-stock-id", GTK_STOCK_GO_FORWARD,
                                    "accel", "<alt>Right",
                                    "closure::callback", moo_edit_window_next_tab,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "Find",
                                    "name", "Find",
                                    "label", "_Find",
                                    "tooltip", "Find",
                                    "icon-stock-id", GTK_STOCK_FIND,
                                    "accel", "<ctrl>F",
                                    "closure::signal", "find",
                                    "closure::proxy-func", moo_edit_window_get_active_doc,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "FindNext",
                                    "name", "Find Next",
                                    "label", "Find _Next",
                                    "tooltip", "Find next",
                                    "icon-stock-id", GTK_STOCK_GO_FORWARD,
                                    "accel", "F3",
                                    "closure::signal", "find-next",
                                    "closure::proxy-func", moo_edit_window_get_active_doc,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "FindPrevious",
                                    "name", "Find Previous",
                                    "label", "Find _Previous",
                                    "tooltip", "Find previous",
                                    "icon-stock-id", GTK_STOCK_GO_BACK,
                                    "accel", "<shift>F3",
                                    "closure::signal", "find-previous",
                                    "closure::proxy-func", moo_edit_window_get_active_doc,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "Replace",
                                    "name", "Replace",
                                    "label", "_Replace",
                                    "tooltip", "Replace",
                                    "icon-stock-id", GTK_STOCK_FIND_AND_REPLACE,
                                    "accel", "<ctrl>R",
                                    "closure::signal", "replace",
                                    "closure::proxy-func", moo_edit_window_get_active_doc,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "GoToLine",
                                    "name", "Go to Line",
                                    "label", "_Go to Line",
                                    "tooltip", "Go to line",
                                    "accel", "<ctrl>G",
                                    "closure::signal", "goto-line",
                                    "closure::proxy-func", moo_edit_window_get_active_doc,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "action-type::", MOO_TYPE_MENU_ACTION,
                                    "id", "SyntaxMenu",
                                    "create-menu-func", create_lang_menu,
                                    NULL);
}


static void     moo_edit_window_init        (MooEditWindow  *window)
{
    window->priv = g_new0 (MooEditWindowPrivate, 1);
    window->priv->prefix = g_strdup ("medit");
    window->priv->lang_menu_items =
            g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

    window->priv->panes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

    g_object_set (G_OBJECT (window),
                  "menubar-ui-name", "Editor/Menubar",
                  "toolbar-ui-name", "Editor/Toolbar",
                  NULL);
}


MooEditor       *moo_edit_window_get_editor     (MooEditWindow  *window)
{
    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);
    return window->priv->editor;
}


static void moo_edit_window_finalize       (GObject      *object)
{
    MooEditWindow *window = MOO_EDIT_WINDOW (object);
    /* XXX */
    g_hash_table_destroy (window->priv->lang_menu_items);
       g_hash_table_destroy (window->priv->panes);
    g_free (window->priv->prefix);
    g_free (window->priv);
    G_OBJECT_CLASS (moo_edit_window_parent_class)->finalize (object);
}


static void     moo_edit_window_set_property(GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec)
{
    MooEditWindow *window = MOO_EDIT_WINDOW (object);

    switch (prop_id) {
        case PROP_EDITOR:
            window->priv->editor = g_value_get_object (value);
            break;

        case PROP_ACTIVE_DOC:
            moo_edit_window_set_active_doc (window, g_value_get_object (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void     moo_edit_window_get_property(GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec)
{
    MooEditWindow *window = MOO_EDIT_WINDOW (object);

    switch (prop_id) {
        case PROP_EDITOR:
            g_value_set_object (value, window->priv->editor);
            break;

        case PROP_ACTIVE_DOC:
            g_value_set_object (value, moo_edit_window_get_active_doc (window));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


void         moo_edit_window_set_title_prefix   (MooEditWindow  *window,
                                                 const char     *prefix)
{
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));

    g_free (window->priv->prefix);
    window->priv->prefix = g_strdup (prefix);

    if (GTK_WIDGET_REALIZED (GTK_WIDGET (window)))
        update_window_title (window);
}


/****************************************************************************/
/* Constructing
 */

GObject        *moo_edit_window_constructor (GType                  type,
                                             guint                  n_props,
                                             GObjectConstructParam *props)
{
    GtkWidget *notebook, *paned;
    MooEditWindow *window;

    GObject *object =
            G_OBJECT_CLASS(moo_edit_window_parent_class)->constructor (type, n_props, props);

    window = MOO_EDIT_WINDOW (object);
    g_return_val_if_fail (window->priv->editor != NULL, object);

    window->priv->statusbar = GTK_STATUSBAR (MOO_WINDOW(window)->statusbar);
    gtk_widget_show (MOO_WINDOW(window)->statusbar);
    window->priv->statusbar_context_id =
            gtk_statusbar_get_context_id (window->priv->statusbar,
                                          "MooEditWindow");


    gtk_widget_show (MOO_WINDOW(window)->vbox);

    paned = g_object_new (MOO_TYPE_BIG_PANED,
                          "handle-cursor-type", GDK_FLEUR,
                          "enable-detaching", TRUE,
                          NULL);
    gtk_widget_show (paned);
    gtk_box_pack_start (GTK_BOX (MOO_WINDOW(window)->vbox), paned, TRUE, TRUE, 0);
    window->paned = MOO_BIG_PANED (paned);

    notebook = g_object_new (MOO_TYPE_NOTEBOOK,
                             "show-tabs", TRUE,
                             "enable-popup", TRUE,
                             "enable-reordering", TRUE,
                             NULL);
    gtk_widget_show (notebook);
    moo_big_paned_add_child (window->paned, notebook);
    window->priv->notebook = MOO_NOTEBOOK (notebook);
    setup_notebook (window);

    g_signal_connect (window, "realize", G_CALLBACK (update_window_title), NULL);

    edit_changed (window, NULL);

    return object;
}


/* XXX */
static void     update_window_title     (MooEditWindow      *window)
{
    MooEdit *edit;
    const char *name;
    MooEditStatus status;
    GString *title;

    edit = ACTIVE_DOC (window);

    if (!edit)
    {
        gtk_window_set_title (GTK_WINDOW (window),
                              window->priv->prefix ? window->priv->prefix : "");
        return;
    }

    if (window->priv->use_fullname)
        name = moo_edit_get_display_filename (edit);
    else
        name = moo_edit_get_display_basename (edit);

    if (!name)
        name = "<\?\?\?\?\?>";

    status = moo_edit_get_status (edit);

    title = g_string_new ("");

    if (window->priv->prefix)
        g_string_append_printf (title, "%s - ", window->priv->prefix);

    g_string_append_printf (title, "%s", name);

    if (status & MOO_EDIT_MODIFIED_ON_DISK)
        g_string_append (title, " [modified on disk]");
    else if (status & MOO_EDIT_DELETED)
        g_string_append (title, " [deleted]");

    if ((status & MOO_EDIT_MODIFIED) && !(status & MOO_EDIT_CLEAN))
        g_string_append (title, " [modified]");

    gtk_window_set_title (GTK_WINDOW (window), title->str);
    g_string_free (title, TRUE);
}


/****************************************************************************/
/* Actions
 */

static gboolean moo_edit_window_close       (MooEditWindow      *window)
{
    moo_editor_close_window (window->priv->editor, window);
    return TRUE;
}


static void moo_edit_window_new             (MooEditWindow   *window)
{
    moo_editor_new_window (window->priv->editor);
}


static void moo_edit_window_new_tab         (MooEditWindow   *window)
{
    moo_editor_new_doc (window->priv->editor, window);
}


static void moo_edit_window_open            (MooEditWindow   *window)
{
    moo_editor_open (window->priv->editor, window, GTK_WIDGET (window), NULL);
}


static void moo_edit_window_reload          (MooEditWindow   *window)
{
    MooEdit *edit = moo_edit_window_get_active_doc (window);
    g_return_if_fail (edit != NULL);
    _moo_editor_reload (window->priv->editor, edit);
}


static void moo_edit_window_save            (MooEditWindow   *window)
{
    MooEdit *edit = moo_edit_window_get_active_doc (window);
    g_return_if_fail (edit != NULL);
    _moo_editor_save (window->priv->editor, edit);
}


static void moo_edit_window_save_as         (MooEditWindow   *window)
{
    MooEdit *edit = moo_edit_window_get_active_doc (window);
    g_return_if_fail (edit != NULL);
    _moo_editor_save_as (window->priv->editor, edit, NULL, NULL);
}


static void moo_edit_window_close_tab       (MooEditWindow   *window)
{
    MooEdit *edit = moo_edit_window_get_active_doc (window);
    g_return_if_fail (edit != NULL);
    moo_editor_close_doc (window->priv->editor, edit);
}


static void moo_edit_window_close_all       (MooEditWindow   *window)
{
    GSList *docs = moo_edit_window_list_docs (window);
    moo_editor_close_docs (window->priv->editor, docs);
    g_slist_free (docs);
}


static void moo_edit_window_previous_tab    (MooEditWindow   *window)
{
    int n = moo_notebook_get_current_page (window->priv->notebook);
    if (n <= 0)
        moo_notebook_set_current_page (window->priv->notebook, -1);
    else
        moo_notebook_set_current_page (window->priv->notebook, n - 1);
}


static void moo_edit_window_next_tab        (MooEditWindow   *window)
{
    int n = moo_notebook_get_current_page (window->priv->notebook);
    if (n == moo_notebook_get_n_pages (window->priv->notebook) - 1)
        moo_notebook_set_current_page (window->priv->notebook, 0);
    else
        moo_notebook_set_current_page (window->priv->notebook, n + 1);
}


/****************************************************************************/
/* Documents
 */

static void     setup_notebook          (MooEditWindow      *window)
{
    GtkWidget *button, *icon, *frame;

    g_signal_connect_after (window->priv->notebook, "switch-page",
                            G_CALLBACK (notebook_switch_page), window);
    g_signal_connect (window->priv->notebook, "populate-popup",
                      G_CALLBACK (notebook_populate_popup), window);

    frame = gtk_aspect_frame_new (NULL, 0.5, 0.5, 1.0, FALSE);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    button = gtk_button_new ();
    gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
    g_signal_connect_swapped (button, "clicked",
                              G_CALLBACK (moo_edit_window_close_tab), window);

    icon = gtk_image_new_from_stock (MOO_STOCK_CLOSE, MOO_ICON_SIZE_REAL_SMALL);

    gtk_container_add (GTK_CONTAINER (button), icon);
    gtk_container_add (GTK_CONTAINER (frame), button);
    gtk_widget_show_all (frame);
    moo_notebook_set_action_widget (window->priv->notebook, frame, TRUE);
}


static void     notebook_switch_page    (G_GNUC_UNUSED MooNotebook *notebook,
                                         G_GNUC_UNUSED gpointer whatever,
                                         guint          page_num,
                                         MooEditWindow *window)
{
    edit_changed (window, get_nth_tab (window, page_num));
    g_object_notify (G_OBJECT (window), "active-doc");
}


static void     update_close_and_save   (MooEditWindow      *window,
                                         MooEdit            *doc)
{
    GtkWidget *close_button;
    MooActionGroup *actions;
    MooAction *close, *close_all, *save, *save_as, *paste;
    MooAction *find, *replace, *find_next, *find_prev, *goto_line;

    close_button = moo_notebook_get_action_widget (window->priv->notebook, TRUE);
    gtk_widget_set_sensitive (close_button, doc != NULL);

    actions = moo_ui_object_get_actions (MOO_UI_OBJECT (window));

    close = moo_action_group_get_action (actions, "Close");
    close_all = moo_action_group_get_action (actions, "CloseAll");
    save = moo_action_group_get_action (actions, "Save");
    save_as = moo_action_group_get_action (actions, "SaveAs");
    paste = moo_action_group_get_action (actions, "Paste");
    find = moo_action_group_get_action (actions, "Find");
    replace = moo_action_group_get_action (actions, "Replace");
    find_next = moo_action_group_get_action (actions, "FindNext");
    find_prev = moo_action_group_get_action (actions, "FindPrevious");
    goto_line = moo_action_group_get_action (actions, "GoToLine");

    moo_action_set_sensitive (close, doc != NULL);
    moo_action_set_sensitive (close_all, doc != NULL);
    moo_action_set_sensitive (save, doc != NULL);
    moo_action_set_sensitive (save_as, doc != NULL);
    moo_action_set_sensitive (paste, doc != NULL);
    moo_action_set_sensitive (find, doc != NULL);
    moo_action_set_sensitive (replace, doc != NULL);
    moo_action_set_sensitive (find_next, doc != NULL);
    moo_action_set_sensitive (find_prev, doc != NULL);
    moo_action_set_sensitive (goto_line, doc != NULL);
}


static void     update_next_prev_tab    (MooEditWindow      *window)
{
    MooActionGroup *actions;
    MooAction *next, *prev;
    gboolean has_next = FALSE, has_prev = FALSE;
    int num, current;

    actions = moo_ui_object_get_actions (MOO_UI_OBJECT (window));

    next = moo_action_group_get_action (actions, "NextTab");
    prev = moo_action_group_get_action (actions, "PreviousTab");

    num = moo_edit_window_num_docs (window);

    if (num)
    {
        current = ACTIVE_PAGE (window);
        has_prev = (current > 0);
        has_next = (current < num - 1);
    }

    moo_action_set_sensitive (next, has_next);
    moo_action_set_sensitive (prev, has_prev);
}


static void     edit_changed            (MooEditWindow      *window,
                                         MooEdit            *doc)
{
    if (doc == ACTIVE_DOC (window))
    {
        update_close_and_save (window, doc);
        update_next_prev_tab (window);
        update_window_title (window);
        edit_can_undo_redo (window);
        active_tab_lang_changed (window);
        update_statusbar (window);
        edit_has_selection (window);
        edit_has_text (window);
        edit_filename_changed (window);
    }

    if (doc)
        update_tab_label (window, doc);
}


static void     edit_lang_changed       (MooEditWindow      *window,
                                         MooEdit            *edit)
{
    if (edit == ACTIVE_DOC (window))
        active_tab_lang_changed (window);
}


/* XXX do not recheck for non-active doc */
static void     edit_can_undo_redo      (MooEditWindow      *window)
{
    MooEdit *edit;
    gboolean can_redo, can_undo;
    MooActionGroup *actions;
    MooAction *undo, *redo;

    edit = ACTIVE_DOC (window);

    if (edit)
    {
        can_redo = moo_text_view_can_redo (MOO_TEXT_VIEW (edit));
        can_undo = moo_text_view_can_undo (MOO_TEXT_VIEW (edit));
    }
    else
    {
        can_redo = FALSE;
        can_undo = FALSE;
    }

    actions = moo_ui_object_get_actions (MOO_UI_OBJECT (window));
    undo = moo_action_group_get_action (actions, "Undo");
    redo = moo_action_group_get_action (actions, "Redo");
    g_return_if_fail (undo != NULL && redo != NULL);

    moo_action_set_sensitive (undo, can_undo);
    moo_action_set_sensitive (redo, can_redo);
}


/* XXX do not recheck for non-active doc */
static void     edit_filename_changed   (MooEditWindow      *window)
{
    MooEdit *edit;
    gboolean has_name;
    MooActionGroup *actions;
    MooAction *reload;

    edit = ACTIVE_DOC (window);

    if (edit)
        has_name = (moo_edit_get_filename (edit) != NULL);
    else
        has_name = FALSE;

    actions = moo_ui_object_get_actions (MOO_UI_OBJECT (window));
    reload = moo_action_group_get_action (actions, "Reload");
    g_return_if_fail (reload != NULL);

    moo_action_set_sensitive (reload, has_name);
}


/* XXX do not recheck for non-active doc */
static void     edit_has_selection      (MooEditWindow      *window)
{
    MooEdit *edit;
    gboolean has_selection;
    MooActionGroup *actions;
    MooAction *cut, *copy, *delete;

    edit = ACTIVE_DOC (window);

    if (edit)
        has_selection = moo_text_view_has_selection (MOO_TEXT_VIEW (edit));
    else
        has_selection = FALSE;

    actions = moo_ui_object_get_actions (MOO_UI_OBJECT (window));
    cut = moo_action_group_get_action (actions, "Cut");
    copy = moo_action_group_get_action (actions, "Copy");
    delete = moo_action_group_get_action (actions, "Delete");
    g_return_if_fail (cut != NULL && copy != NULL && delete != NULL);

    moo_action_set_sensitive (cut, has_selection);
    moo_action_set_sensitive (copy, has_selection);
    moo_action_set_sensitive (delete, has_selection);
}


/* XXX do not recheck for non-active doc */
static void     edit_has_text           (MooEditWindow      *window)
{
    MooEdit *edit;
    gboolean has_text;
    MooActionGroup *actions;
    MooAction *select_all;

    edit = ACTIVE_DOC (window);

    if (edit)
        has_text = moo_text_view_has_text (MOO_TEXT_VIEW (edit));
    else
        has_text = FALSE;

    actions = moo_ui_object_get_actions (MOO_UI_OBJECT (window));
    select_all = moo_action_group_get_action (actions, "SelectAll");
    g_return_if_fail (select_all != NULL);

    moo_action_set_sensitive (select_all, has_text);
}


MooEdit *moo_edit_window_get_active_doc (MooEditWindow  *window)
{
    GtkWidget *swin;
    int page;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);

    page = moo_notebook_get_current_page (window->priv->notebook);

    if (page < 0)
        return NULL;

    swin = moo_notebook_get_nth_page (window->priv->notebook, page);
    return MOO_EDIT (gtk_bin_get_child (GTK_BIN (swin)));
}


void     moo_edit_window_set_active_doc (MooEditWindow  *window,
                                         MooEdit        *edit)
{
    GtkWidget *swin;
    int page;

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDIT (edit));

    swin = GTK_WIDGET(edit)->parent;
    page = moo_notebook_page_num (window->priv->notebook, swin);
    g_return_if_fail (page >= 0);

    moo_notebook_set_current_page (window->priv->notebook, page);
}


GSList  *moo_edit_window_list_docs      (MooEditWindow  *window)
{
    GSList *list = NULL;
    int num, i;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);

    num = moo_notebook_get_n_pages (window->priv->notebook);

    for (i = 0; i < num; i++)
        list = g_slist_prepend (list, get_nth_tab (window, i));

    return g_slist_reverse (list);
}


guint    moo_edit_window_num_docs       (MooEditWindow  *window)
{
    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), 0);
    return moo_notebook_get_n_pages (window->priv->notebook);
}


static MooEdit *get_nth_tab             (MooEditWindow      *window,
                                         guint               n)
{
    GtkWidget *swin;

    swin = moo_notebook_get_nth_page (window->priv->notebook, n);

    if (swin)
        return MOO_EDIT (gtk_bin_get_child (GTK_BIN (swin)));
    else
        return NULL;
}


static int      get_page_num            (MooEditWindow      *window,
                                         MooEdit            *doc)
{
    GtkWidget *swin;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), -1);
    g_return_val_if_fail (MOO_IS_EDIT (doc), -1);

    swin = GTK_WIDGET(doc)->parent;
    return moo_notebook_page_num (window->priv->notebook, swin);
}


void             _moo_edit_window_insert_doc    (MooEditWindow  *window,
                                                 MooEdit        *edit,
                                                 int             position)
{
    GtkWidget *label;
    GtkWidget *scrolledwindow;

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDIT (edit));

    label = create_tab_label (edit);
    gtk_widget_show (label);

    scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow),
                                         GTK_SHADOW_ETCHED_IN);
    gtk_container_add (GTK_CONTAINER (scrolledwindow), GTK_WIDGET (edit));
    gtk_widget_show_all (scrolledwindow);

    moo_notebook_insert_page (window->priv->notebook, scrolledwindow, label, position);

    g_signal_connect_swapped (edit, "doc_status_changed",
                              G_CALLBACK (edit_changed), window);
    g_signal_connect_swapped (edit, "filename_changed",
                              G_CALLBACK (edit_filename_changed), window);
    g_signal_connect_swapped (edit, "can-undo",
                              G_CALLBACK (edit_can_undo_redo), window);
    g_signal_connect_swapped (edit, "can-redo",
                              G_CALLBACK (edit_can_undo_redo), window);
    g_signal_connect_swapped (edit, "notify::has-selection",
                              G_CALLBACK (edit_has_selection), window);
    g_signal_connect_swapped (edit, "notify::has-text",
                              G_CALLBACK (edit_has_text), window);
    g_signal_connect_swapped (edit, "lang-changed",
                              G_CALLBACK (edit_lang_changed), window);
    g_signal_connect_swapped (edit, "cursor-moved",
                              G_CALLBACK (edit_cursor_moved), window);

    g_signal_emit (window, signals[NEW_DOC], 0, edit);

    moo_edit_window_set_active_doc (window, edit);
    edit_changed (window, edit);
    gtk_widget_grab_focus (GTK_WIDGET (edit));
}


void             _moo_edit_window_remove_doc    (MooEditWindow  *window,
                                                 MooEdit        *doc)
{
    int page;

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDIT (doc));

    page = get_page_num (window, doc);
    g_return_if_fail (page >= 0);

    g_signal_emit (window, signals[CLOSE_DOC], 0, doc);

    g_signal_handlers_disconnect_by_func (doc,
                                          (gpointer) edit_changed,
                                          window);
    g_signal_handlers_disconnect_by_func (doc,
                                          (gpointer) edit_can_undo_redo,
                                          window);
    g_signal_handlers_disconnect_by_func (doc,
                                          (gpointer) edit_lang_changed,
                                          window);
    g_signal_handlers_disconnect_by_func (doc,
                                          (gpointer) edit_cursor_moved,
                                          window);

    moo_notebook_remove_page (window->priv->notebook, page);
    edit_changed (window, NULL);

    g_signal_emit (window, signals[CLOSE_DOC_AFTER], 0);
    g_object_notify (G_OBJECT (window), "active-doc");
}


static GtkWidget *create_tab_label      (MooEdit            *edit)
{
    GtkWidget *hbox, *modified_icon, *modified_on_disk_icon, *label;
    GtkSizeGroup *group;

    group = gtk_size_group_new (GTK_SIZE_GROUP_VERTICAL);

    hbox = gtk_hbox_new (FALSE, 3);
    gtk_widget_show (hbox);

    modified_on_disk_icon = gtk_image_new ();
    gtk_box_pack_start (GTK_BOX (hbox), modified_on_disk_icon, FALSE, FALSE, 0);

    modified_icon = gtk_image_new_from_stock (MOO_STOCK_DOC_MODIFIED,
                                              GTK_ICON_SIZE_MENU);
    gtk_box_pack_start (GTK_BOX (hbox), modified_icon, FALSE, FALSE, 0);

    label = gtk_label_new (moo_edit_get_display_basename (edit));
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

    gtk_size_group_add_widget (group, modified_on_disk_icon);
    gtk_size_group_add_widget (group, modified_icon);
    gtk_size_group_add_widget (group, label);

    g_object_set_data (G_OBJECT (hbox), "moo-edit-modified-on-disk-icon", modified_on_disk_icon);
    g_object_set_data (G_OBJECT (hbox), "moo-edit-modified-icon", modified_icon);
    g_object_set_data (G_OBJECT (hbox), "moo-edit-label", label);

    return hbox;
}


static void     update_tab_label        (MooEditWindow      *window,
                                         MooEdit            *doc)
{
    GtkWidget *hbox, *modified_icon, *modified_on_disk_icon, *label;
    MooEditStatus status;
    int page = get_page_num (window, doc);

    g_return_if_fail (page >= 0);

    hbox = moo_notebook_get_tab_label (window->priv->notebook,
                                       GTK_WIDGET(doc)->parent);
    modified_icon = g_object_get_data (G_OBJECT (hbox), "moo-edit-modified-icon");
    modified_on_disk_icon = g_object_get_data (G_OBJECT (hbox), "moo-edit-modified-on-disk-icon");
    label = g_object_get_data (G_OBJECT (hbox), "moo-edit-label");

    g_return_if_fail (GTK_IS_WIDGET (hbox) && GTK_IS_WIDGET (modified_icon) &&
            GTK_IS_WIDGET (modified_on_disk_icon));

    status = moo_edit_get_status (doc);

    if (status & MOO_EDIT_MODIFIED_ON_DISK)
    {
        gtk_image_set_from_stock (GTK_IMAGE (modified_on_disk_icon),
                                  MOO_STOCK_DOC_MODIFIED_ON_DISK,
                                  GTK_ICON_SIZE_MENU);
        gtk_widget_show (modified_on_disk_icon);
    }
    else if (status & MOO_EDIT_DELETED)
    {
        gtk_image_set_from_stock (GTK_IMAGE (modified_on_disk_icon),
                                  MOO_STOCK_DOC_DELETED,
                                  GTK_ICON_SIZE_MENU);
        gtk_widget_show (modified_on_disk_icon);
    }
    else
    {
        gtk_widget_hide (modified_on_disk_icon);
    }

    if ((status & MOO_EDIT_MODIFIED) && !(status & MOO_EDIT_CLEAN))
        gtk_widget_show (modified_icon);
    else
        gtk_widget_hide (modified_icon);

    gtk_label_set_text (GTK_LABEL (label), moo_edit_get_display_basename (doc));
}


/****************************************************************************/
/* Notebook popup menu
 */

static void     close_activated         (GtkWidget          *item,
                                         MooEditWindow      *window)
{
    MooEdit *edit = g_object_get_data (G_OBJECT (item), "moo-edit");
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDIT (edit));
    moo_editor_close_doc (window->priv->editor, edit);
}


static void     close_others_activated  (GtkWidget          *item,
                                         MooEditWindow      *window)
{
    GSList *list;
    MooEdit *edit = g_object_get_data (G_OBJECT (item), "moo-edit");

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDIT (edit));

    list = moo_edit_window_list_docs (window);
    list = g_slist_remove (list, edit);

    moo_editor_close_docs (window->priv->editor, list);

    g_slist_free (list);
}


static gboolean notebook_populate_popup (MooNotebook        *notebook,
                                         GtkWidget          *child,
                                         GtkMenu            *menu,
                                         MooEditWindow      *window)
{
    MooEdit *edit;
    GtkWidget *item;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), TRUE);
    g_return_val_if_fail (window->priv->notebook == notebook, TRUE);
    g_return_val_if_fail (GTK_IS_SCROLLED_WINDOW (child), TRUE);

    edit = MOO_EDIT (gtk_bin_get_child (GTK_BIN (child)));
    g_return_val_if_fail (MOO_IS_EDIT (edit), TRUE);

    item = gtk_menu_item_new_with_label ("Close");
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    g_object_set_data (G_OBJECT (item), "moo-edit", edit);
    g_signal_connect (item, "activate",
                      G_CALLBACK (close_activated),
                      window);

    if (moo_edit_window_num_docs (window) > 1)
    {
        item = gtk_menu_item_new_with_label ("Close All Others");
        gtk_widget_show (item);
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
        g_object_set_data (G_OBJECT (item), "moo-edit", edit);
        g_signal_connect (item, "activate",
                          G_CALLBACK (close_others_activated),
                          window);
    }

    return FALSE;
}


/****************************************************************************/
/* Panes
 */

gboolean
moo_edit_window_add_pane (MooEditWindow  *window,
                          const char     *user_id,
                          GtkWidget      *widget,
                          MooPaneLabel   *label,
                          MooPanePosition position)
{
    gboolean result;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), FALSE);
    g_return_val_if_fail (user_id != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);
    g_return_val_if_fail (label != NULL, FALSE);

    g_return_val_if_fail (moo_edit_window_get_pane (window, user_id) == NULL, FALSE);

    gtk_object_sink (GTK_OBJECT (g_object_ref (widget)));

    result = moo_big_paned_insert_pane (window->paned, widget, label,
                                        position, -1);
    result = (result < 0 ? FALSE : TRUE);

    if (result)
        g_hash_table_insert (window->priv->panes,
                             g_strdup (user_id), widget);

    g_object_unref (widget);
    moo_pane_label_free (label);

    return result;
}


gboolean
moo_edit_window_remove_pane (MooEditWindow  *window,
                             const char     *user_id)
{
    GtkWidget *widget;
    gboolean result;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), FALSE);
    g_return_val_if_fail (user_id != NULL, FALSE);

    widget = g_hash_table_lookup (window->priv->panes, user_id);

    if (!widget)
        return FALSE;

    g_hash_table_remove (window->priv->panes, user_id);

    result = moo_big_paned_remove_pane (window->paned, widget);
    g_return_val_if_fail (result, FALSE);
    return TRUE;
}


GtkWidget*
moo_edit_window_get_pane (MooEditWindow  *window,
                          const char     *user_id)
{
    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);
    g_return_val_if_fail (user_id != NULL, NULL);
    return g_hash_table_lookup (window->priv->panes, user_id);
}


/****************************************************************************/
/* Languages menu
 */

/* XXX what does this action do? */
static GtkMenuItem *create_lang_menu        (MooEditWindow      *window)
{
    GSList *sections;
    GSList *section_items = NULL, *section_menus = NULL;
    GSList *l;
    MooEditLangMgr *mgr;
    const GSList *langs, *cl;
    GtkWidget *main_item, *main_menu, *none;
    GSList *group = NULL;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);
    g_return_val_if_fail (window->priv->editor != NULL, NULL);

    main_item = gtk_menu_item_new_with_label ("Language");
    window->priv->languages_menu_item = main_item;
    main_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (main_item), main_menu);

    none = gtk_radio_menu_item_new_with_label (group, "None");
    group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (none));
    gtk_menu_shell_append (GTK_MENU_SHELL (main_menu), none);
    g_signal_connect (none, "toggled",
                      G_CALLBACK (lang_menu_item_toggled),
                      window);
    g_object_set_data (G_OBJECT (none), "moo_edit_window_lang_id", NULL);
    window->priv->none_lang_item = none;

    mgr = moo_editor_get_lang_mgr (window->priv->editor);
    sections = moo_edit_lang_mgr_get_sections (mgr);

    for (l = sections; l != NULL; l = l->next)
    {
        GtkWidget *item, *menu;

        item = gtk_menu_item_new_with_label (l->data);
        gtk_menu_shell_append (GTK_MENU_SHELL (main_menu), item);
        menu = gtk_menu_new ();
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);

        section_items = g_slist_append (section_items, item);
        section_menus = g_slist_append (section_menus, menu);
    }

    langs = moo_edit_lang_mgr_get_available_languages (mgr);

    for (cl = langs; cl != NULL; cl = cl->next)
    {
        MooEditLang *lang;
        const char *section, *name;
        GtkWidget *item;
        GSList *link;
        int pos;

        lang = MOO_EDIT_LANG (cl->data);
        section = moo_edit_lang_get_section (lang);
        name = moo_edit_lang_get_name (lang);

        link = g_slist_find_custom (sections, section, (GCompareFunc) strcmp);
        pos = g_slist_position (sections, link);
        g_assert (pos >= 0);

        item = gtk_radio_menu_item_new_with_label (group, name);
        group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (item));

        g_object_set_data_full (G_OBJECT (item), "moo_edit_window_lang_id",
                                g_strdup (moo_edit_lang_get_id (lang)),
                                (GDestroyNotify) g_free);
        g_signal_connect (item, "toggled",
                          G_CALLBACK (lang_menu_item_toggled),
                          window);
        g_hash_table_insert (window->priv->lang_menu_items,
                             g_strdup (moo_edit_lang_get_id (lang)),
                             item);

        link = g_slist_nth (section_menus, pos);
        g_assert (link != NULL);
        gtk_menu_shell_append (GTK_MENU_SHELL (link->data), item);
    }

    gtk_widget_show_all (main_item);
    active_tab_lang_changed (window);

    g_slist_foreach (sections, (GFunc) g_free, NULL);
    g_slist_free (sections);
    g_slist_free (section_items);
    g_slist_free (section_menus);

    /* XXX watch lang_mgr for changes */
    return GTK_MENU_ITEM (main_item);
}


static void     lang_menu_item_toggled      (GtkCheckMenuItem   *item,
                                             MooEditWindow      *window)
{
    const char *lang_id;
    MooEditLang *lang = NULL;
    MooEdit *edit;
    MooEditor *editor;

    if (!gtk_check_menu_item_get_active (item))
        return;

    editor = window->priv->editor;
    edit = ACTIVE_DOC (window);

    g_return_if_fail (edit != NULL);

    lang_id = g_object_get_data (G_OBJECT (item), "moo_edit_window_lang_id");

    if (lang_id)
        lang = moo_edit_lang_mgr_get_language_by_id (moo_editor_get_lang_mgr (editor),
                                                     lang_id);

    g_signal_handlers_block_by_func (edit, active_tab_lang_changed, window);
    moo_edit_set_lang (edit, lang);
    edit->priv->lang_custom = TRUE;
    g_signal_handlers_unblock_by_func (edit, active_tab_lang_changed, window);
}


static void     active_tab_lang_changed     (MooEditWindow      *window)
{
    const char *lang_id = NULL;
    MooEditLang *lang = NULL;
    MooEdit *edit;
    GtkCheckMenuItem *item;

    edit = ACTIVE_DOC (window);

    if (!window->priv->languages_menu_item)
        return;

    gtk_widget_set_sensitive (window->priv->languages_menu_item, edit != NULL);

    if (!edit)
    {
        item = GTK_CHECK_MENU_ITEM (window->priv->none_lang_item);
    }
    else
    {
        lang = moo_edit_get_lang (edit);

        if (lang)
        {
            lang_id = moo_edit_lang_get_id (lang);
            item = g_hash_table_lookup (window->priv->lang_menu_items, lang_id);
        }
        else
        {
            item = GTK_CHECK_MENU_ITEM (window->priv->none_lang_item);
        }
    }

    g_return_if_fail (item != NULL);

    g_signal_handlers_block_by_func (item, lang_menu_item_toggled, window);
    gtk_check_menu_item_set_active (item, TRUE);
    g_signal_handlers_unblock_by_func (item, lang_menu_item_toggled, window);
}


/****************************************************************************/
/* Statusbar
 */

static void set_statusbar_numbers (MooEditWindow *window,
                                   int            line,
                                   int            column)
{
    char *text = g_strdup_printf ("Line: %d Col: %d", line, column);
    gtk_statusbar_pop (window->priv->statusbar,
                       window->priv->statusbar_context_id);
    gtk_statusbar_push (window->priv->statusbar,
                        window->priv->statusbar_context_id,
                        text);
    g_free (text);
}


/* XXX */
static void update_statusbar (MooEditWindow *window)
{
    MooEdit *edit;
    int line, column;
    GtkTextIter iter;
    GtkTextBuffer *buffer;

    edit = ACTIVE_DOC (window);

    if (!edit)
    {
        gtk_statusbar_pop (window->priv->statusbar,
                           window->priv->statusbar_context_id);
        return;
    }

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit));
    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                      gtk_text_buffer_get_insert (buffer));
    line = gtk_text_iter_get_line (&iter) + 1;
    column = gtk_text_iter_get_line_offset (&iter) + 1;

    set_statusbar_numbers (window, line, column);
}


/* XXX */
static void     edit_cursor_moved       (MooEditWindow      *window,
                                         GtkTextIter        *iter,
                                         MooEdit            *edit)
{
    if (edit == ACTIVE_DOC (window))
    {
        int line = gtk_text_iter_get_line (iter) + 1;
        int column = gtk_text_iter_get_line_offset (iter) + 1;
        set_statusbar_numbers (window, line, column);
    }
}
