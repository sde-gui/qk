/*
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
#include "mooedit/mooeditwindow.h"
#include "mooedit/mooeditor.h"
#include "mooedit/moobigpaned.h"
#include "mooedit/moonotebook.h"
#include "mooutils/moostock.h"
#include "mooui/moomenuaction.h"
#include "mooui/moouiobject-impl.h"
#include <string.h>


#define ACTIVE_DOC moo_edit_window_get_active_doc


struct _MooEditWindowPrivate {
    MooEditor *editor;
    GtkStatusbar *statusbar;
    guint statusbar_context_id;
    MooBigPaned *paned;
    MooNotebook *notebook;
    char *app_name;
    gboolean use_fullname;

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

static gboolean moo_edit_window_close   (MooEditWindow      *window);


static GtkMenuItem *create_lang_menu    (MooEditWindow      *window);
static void     lang_menu_item_toggled  (GtkCheckMenuItem   *item,
                                         MooEditWindow      *window);
static void     active_tab_lang_changed (MooEditWindow      *window);

static void     setup_paned             (MooEditWindow      *window);
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
static void     edit_can_undo_redo      (MooEditWindow      *window);
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
                                    "id", "CloseTab",
                                    "name", "Close Tab",
                                    "label", "_Close Tab",
                                    "tooltip", "Close current document tab",
                                    "icon-stock-id", GTK_STOCK_CLOSE,
                                    "accel", "<ctrl>W",
                                    "closure::callback", moo_edit_window_close_tab,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "CloseAll",
                                    "name", "Close All",
                                    "label", "_Close All",
                                    "tooltip", "Close all documents",
                                    "icon-stock-id", GTK_STOCK_CLOSE,
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
                                    "closure::callback", moo_edit_select_all,
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

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "ShowFileSelector",
                                    "dead", TRUE,
                                    NULL);
//     moo_ui_object_class_new_action (gobject_class,
//                                     "action-type::", MOO_TYPE_TOGGLE_ACTION,
//                                     "id", "ShowFileSelector",
//                                     "name", "Show File Selector",
//                                     "label", "Show File Selector",
//                                     "tooltip", "Show File Selector",
//                                     "toggled-callback", show_file_selector_toggled_cb,
//                                     NULL);
}


static void     moo_edit_window_init        (MooEditWindow  *window)
{
    window->priv = g_new0 (MooEditWindowPrivate, 1);

    window->priv->lang_menu_items =
            g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

    g_object_set (G_OBJECT (window),
                  "menubar-ui-name", "Editor/Menubar",
                  "toolbar-ui-name", "Editor/Toolbar",
                  NULL);
}


MooEditWindow   *_moo_edit_window_new           (MooEditor      *editor)
{
    MooEditWindow *window;
    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
    window = g_object_new (MOO_TYPE_EDIT_WINDOW, "editor", editor, NULL);
    gtk_widget_show (GTK_WIDGET (window));
    return window;
}


MooEditor       *_moo_edit_window_get_editor    (MooEditWindow  *window)
{
    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);
    return window->priv->editor;
}


static void moo_edit_window_finalize       (GObject      *object)
{
    MooEditWindow *window = MOO_EDIT_WINDOW (object);
    g_free (window->priv->app_name);
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


void         _moo_edit_window_set_app_name      (MooEditWindow  *window,
                                                 const char     *name)
{
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));

    g_free (window->priv->app_name);
    window->priv->app_name = g_strdup (name);

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
    MooUIXML *xml;

    GObject *object =
            G_OBJECT_CLASS(moo_edit_window_parent_class)->constructor (type, n_props, props);

    window = MOO_EDIT_WINDOW (object);
    g_return_val_if_fail (window->priv->editor != NULL, object);

    xml = moo_editor_get_ui_xml (window->priv->editor);
    moo_ui_object_set_ui_xml (MOO_UI_OBJECT (window), xml);

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
    window->priv->paned = MOO_BIG_PANED (paned);
    setup_paned (window);

    notebook = g_object_new (MOO_TYPE_NOTEBOOK,
                             "show-tabs", TRUE,
                             "enable-popup", TRUE,
                             "enable-reordering", TRUE,
                             NULL);
    gtk_widget_show (notebook);
    moo_big_paned_add_child (window->priv->paned, notebook);
    window->priv->notebook = MOO_NOTEBOOK (notebook);
    setup_notebook (window);

    g_signal_connect (window, "realize", G_CALLBACK (update_window_title), NULL);

    return object;
}


// #define SCROLLED_WINDOW_QUARK (scrolled_window_quark())
// static GQuark scrolled_window_quark (void)
// {
//     static GQuark q = 0;
//     if (!q)
//         q = g_quark_from_static_string ("moo_edit_window_edit_scrolled_window");
//     return q;
// }


// static void     add_tab                 (MooEditWindow      *window,
//                                          MooEdit            *edit)
// {
//     GtkWidget *label;
//     GtkWidget *scrolledwindow;
//     int n;
//
//     g_return_if_fail (MOO_IS_WINDOW (window) && MOO_EDIT (edit));
//
//     label = create_tab_label (edit);
//     gtk_widget_show (label);
//     scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
//     gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
//                                     GTK_POLICY_AUTOMATIC,
//                                     GTK_POLICY_AUTOMATIC);
//     gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow),
//                                          GTK_SHADOW_ETCHED_IN);
//     gtk_container_add (GTK_CONTAINER (scrolledwindow), GTK_WIDGET (edit));
//     gtk_widget_show_all (scrolledwindow);
//     g_object_set_qdata (G_OBJECT (edit), SCROLLED_WINDOW_QUARK, scrolledwindow);
//
//     n = moo_notebook_insert_page (window->priv->notebook, scrolledwindow, label, -1);
//
//     moo_notebook_set_current_page (window->priv->notebook, n);
//     g_signal_connect (edit, "doc_status_changed",
//                       G_CALLBACK (edit_changed), window);
//     g_signal_connect_swapped (edit, "can-undo",
//                               G_CALLBACK (edit_can_undo_redo), window);
//     g_signal_connect_swapped (edit, "can-redo",
//                               G_CALLBACK (edit_can_undo_redo), window);
//     g_signal_connect_swapped (edit, "lang-changed",
//                               G_CALLBACK (active_tab_lang_changed), window);
//     g_signal_connect_swapped (edit, "cursor-moved",
//                               G_CALLBACK (cursor_moved), window);
//
//     edit_changed (edit, window);
// }


// static void     notebook_switch_page        (G_GNUC_UNUSED MooNotebook *notebook,
//                                              G_GNUC_UNUSED gpointer whatever,
//                                              guint               page_num,
//                                              MooEditWindow      *window)
// {
//     edit_changed (get_nth_tab (window, page_num), window);
// }


// static MooEdit *get_nth_tab             (MooEditWindow      *window,
//                                          int                 n)
// {
//     GtkWidget *swin = moo_notebook_get_nth_page (window->priv->notebook, n);
//     g_return_val_if_fail (swin != NULL, NULL);
//     return MOO_EDIT (gtk_bin_get_child (GTK_BIN (swin)));
// }


// void             moo_edit_window_set_active_doc (MooEditWindow  *window,
//                                                  MooEdit        *edit)
// {
//     int index;
//     g_return_if_fail (MOO_IS_EDIT_WINDOW (window) && MOO_IS_EDIT (edit));
//     index = moo_notebook_page_num (window->priv->notebook,
//                                    g_object_get_qdata (G_OBJECT (edit),
//                                            SCROLLED_WINDOW_QUARK));
//     g_return_if_fail (index >= 0);
//     moo_notebook_set_current_page (window->priv->notebook, index);
// }


// static void     update_window_title         (MooEditWindow      *window)
// {
//     MooEdit *edit;
//     const char *name;
//     MooEditStatus status;
//     gboolean modified, deleted, modified_on_disk, clean;
//     GString *title;
//
//     edit = ACTIVE_DOC (window);
//     g_return_if_fail (edit != NULL);
//     if (window->priv->use_fullname)
//         name = moo_edit_get_display_filename (edit);
//     else
//         name = moo_edit_get_display_basename (edit);
//     if (!name) name = "<\?\?\?\?\?>";
//
//     status = moo_edit_get_status (edit);
//     clean = status & MOO_EDIT_CLEAN;
//     modified = status & MOO_EDIT_MODIFIED;
//     deleted = status & MOO_EDIT_DELETED;
//     modified_on_disk = status & MOO_EDIT_MODIFIED_ON_DISK;
//
//     title = g_string_new ("");
//     if (window->priv->app_name)
//         g_string_append_printf (title, "%s - ", window->priv->app_name);
//     g_string_append_printf (title, "%s%s%s",
//                             (modified && !clean) ? "* " : "",
//                             name,
//                             deleted ? " [deleted]" :
//                                 (modified_on_disk ? " [modified on disk]" : ""));
//     gtk_window_set_title (GTK_WINDOW (window), title->str);
//     g_string_free (title, TRUE);
// }


// static void     edit_changed                (G_GNUC_UNUSED MooEdit            *edit,
//                                              MooEditWindow      *window)
// {
//     update_window_title (window);
//     edit_can_undo_redo (window);
//     active_tab_lang_changed (window);
//     update_statusbar (window);
// }


// static void     edit_can_undo_redo      (MooEditWindow      *window)
// {
//     MooEdit *edit;
//     gboolean can_redo, can_undo;
//     MooActionGroup *actions;
//     MooAction *undo, *redo;
//
//     g_return_if_fail (window != NULL);
//     edit = ACTIVE_DOC (window);
//     g_return_if_fail (edit != NULL);
//
//     can_redo = moo_edit_can_redo (edit);
//     can_undo = moo_edit_can_undo (edit);
//
//     actions = moo_ui_object_get_actions (MOO_UI_OBJECT (window));
//     undo = moo_action_group_get_action (actions, "Undo");
//     redo = moo_action_group_get_action (actions, "Redo");
//     g_return_if_fail (undo != NULL && redo != NULL);
//
//     moo_action_set_sensitive (undo, can_undo);
//     moo_action_set_sensitive (redo, can_redo);
// }


// static GtkWidget *create_tab_label          (MooEdit            *edit)
// {
//     GtkWidget *eventbox, *hbox, *filename, *icon;
//
//     eventbox = gtk_event_box_new ();
//     hbox = gtk_hbox_new (FALSE, 0);
//     gtk_widget_show (hbox);
//     icon = gtk_image_new ();
//     filename = gtk_label_new (moo_edit_get_display_basename (edit));
//     gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 0);
//     gtk_box_pack_start (GTK_BOX (hbox), filename, TRUE, TRUE, 3);
//
//     g_object_set_data (G_OBJECT (eventbox), "filename", filename);
//     g_object_set_data (G_OBJECT (eventbox), "icon", icon);
//     gtk_widget_show (filename);
//
//     g_signal_connect_swapped (edit, "doc-status-changed",
//                               G_CALLBACK (update_tab_label), eventbox);
//
//     return hbox;
// }


// static void     update_tab_label            (GtkWidget          *label,
//                                              MooEdit            *edit)
// {
//     const char *name;
//     MooEditStatus status;
//     gboolean clean, modified, deleted, modified_on_disk;
//     GString *title;
//     GtkWidget *filename, *icon;
//
//     filename = g_object_get_data (G_OBJECT (label), "filename");
//     icon = g_object_get_data (G_OBJECT (label), "icon");
//
//     name = moo_edit_get_display_basename (edit);
//     if (!name) name = "<\?\?\?\?\?>";
//
//     status = moo_edit_get_status (edit);
//     clean = status & MOO_EDIT_CLEAN;
//     modified = status & MOO_EDIT_MODIFIED;
//     deleted = status & MOO_EDIT_DELETED;
//     modified_on_disk = status & MOO_EDIT_MODIFIED_ON_DISK;
//
//     title = g_string_new ("");
//     g_string_append_printf (title, "%s%s",
//                             (modified && !clean) ? "* " : "",
//                             name);
//     gtk_label_set_text (GTK_LABEL (filename), title->str);
//     g_string_free (title, TRUE);
//
//     if (deleted || modified_on_disk)
//         gtk_widget_show (icon);
//     else
//         gtk_widget_hide (icon);
//
//     if (deleted)
//         gtk_image_set_from_stock (GTK_IMAGE (icon),
//                                   MOO_STOCK_DOC_DELETED,
//                                   GTK_ICON_SIZE_MENU);
//     else if (modified_on_disk)
//         gtk_image_set_from_stock (GTK_IMAGE (icon),
//                                   MOO_STOCK_DOC_MODIFIED_ON_DISK,
//                                   GTK_ICON_SIZE_MENU);
// }


/* XXX */
static void     update_window_title     (MooEditWindow      *window)
{
    MooEdit *edit;
    const char *name;
    MooEditStatus status;
    gboolean modified, deleted, modified_on_disk, clean;
    GString *title;

    edit = ACTIVE_DOC (window);

    if (!edit)
    {
        gtk_window_set_title (GTK_WINDOW (window),
                              window->priv->app_name ? window->priv->app_name : "");
        return;
    }

    if (window->priv->use_fullname)
        name = moo_edit_get_display_filename (edit);
    else
        name = moo_edit_get_display_basename (edit);

    if (!name)
        name = "<\?\?\?\?\?>";

    status = moo_edit_get_status (edit);
    clean = status & MOO_EDIT_CLEAN;
    modified = status & MOO_EDIT_MODIFIED;
    deleted = status & MOO_EDIT_DELETED;
    modified_on_disk = status & MOO_EDIT_MODIFIED_ON_DISK;

    title = g_string_new ("");

    if (window->priv->app_name)
        g_string_append_printf (title, "%s - ", window->priv->app_name);

    g_string_append_printf (title, "%s%s%s",
                            (modified && !clean) ? "* " : "",
                            name,
                            deleted ? " [deleted]" :
                                (modified_on_disk ? " [modified on disk]" : ""));

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
    GtkWidget *button, *icon;

    g_signal_connect_after (window->priv->notebook, "switch-page",
                            G_CALLBACK (notebook_switch_page), window);
    g_signal_connect (window->priv->notebook, "populate-popup",
                      G_CALLBACK (notebook_populate_popup), window);

    button = gtk_button_new ();
    gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
    icon = gtk_image_new_from_stock (MOO_STOCK_CLOSE, MOO_ICON_SIZE_REAL_SMALL);
    gtk_container_add (GTK_CONTAINER (button), icon);
    gtk_widget_show_all (button);
    moo_notebook_set_action_widget (window->priv->notebook, button, TRUE);
    g_signal_connect_swapped (button, "clicked",
                              G_CALLBACK (moo_edit_window_close_tab), window);
}


static void     notebook_switch_page    (G_GNUC_UNUSED MooNotebook *notebook,
                                         G_GNUC_UNUSED gpointer whatever,
                                         guint          page_num,
                                         MooEditWindow *window)
{
    edit_changed (window, get_nth_tab (window, page_num));
    g_object_notify (G_OBJECT (window), "active-doc");
}


static void     edit_changed            (MooEditWindow      *window,
                                         MooEdit            *doc)
{
    if (doc == ACTIVE_DOC (window))
    {
        GtkWidget *close_button;

        update_window_title (window);
        edit_can_undo_redo (window);
        active_tab_lang_changed (window);
        update_statusbar (window);

        close_button = moo_notebook_get_action_widget (window->priv->notebook, TRUE);
        gtk_widget_set_sensitive (close_button, doc != NULL);
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


static void     edit_can_undo_redo      (MooEditWindow      *window)
{
    MooEdit *edit;
    gboolean can_redo, can_undo;
    MooActionGroup *actions;
    MooAction *undo, *redo;

    edit = ACTIVE_DOC (window);

    if (edit)
    {
        can_redo = moo_edit_can_redo (edit);
        can_undo = moo_edit_can_undo (edit);
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
    g_signal_connect_swapped (edit, "can-undo",
                              G_CALLBACK (edit_can_undo_redo), window);
    g_signal_connect_swapped (edit, "can-redo",
                              G_CALLBACK (edit_can_undo_redo), window);
    g_signal_connect_swapped (edit, "lang-changed",
                              G_CALLBACK (edit_lang_changed), window);
    g_signal_connect_swapped (edit, "cursor-moved",
                              G_CALLBACK (edit_cursor_moved), window);

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
    g_object_notify (G_OBJECT (window), "active-doc");
}


static GtkWidget *create_tab_label      (MooEdit            *edit)
{
    GtkWidget *hbox, *icon, *label;

    hbox = gtk_hbox_new (FALSE, 3);
    gtk_widget_show (hbox);

    icon = gtk_image_new ();
    gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 0);

    label = gtk_label_new (moo_edit_get_display_basename (edit));
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

    g_object_set_data (G_OBJECT (hbox), "moo-edit-icon", icon);
    g_object_set_data (G_OBJECT (hbox), "moo-edit-label", label);

    return hbox;
}


static void     update_tab_label        (MooEditWindow      *window,
                                         MooEdit            *doc)
{
    GtkWidget *hbox, *icon, *label;
    MooEditStatus status;
    int page = get_page_num (window, doc);

    g_return_if_fail (page >= 0);

    hbox = moo_notebook_get_tab_label (window->priv->notebook,
                                       GTK_WIDGET(doc)->parent);
    icon = g_object_get_data (G_OBJECT (hbox), "moo-edit-icon");
    label = g_object_get_data (G_OBJECT (hbox), "moo-edit-label");
    g_return_if_fail (GTK_IS_WIDGET (hbox) && GTK_IS_WIDGET (icon) &&
            GTK_IS_WIDGET (label));

    status = moo_edit_get_status (doc);

    if (status & MOO_EDIT_MODIFIED_ON_DISK)
    {
        gtk_image_set_from_stock (GTK_IMAGE (icon),
                                  MOO_STOCK_DOC_MODIFIED_ON_DISK,
                                  GTK_ICON_SIZE_MENU);
        gtk_widget_show (icon);
    }
    else if (status & MOO_EDIT_DELETED)
    {
        gtk_image_set_from_stock (GTK_IMAGE (icon),
                                  MOO_STOCK_DOC_DELETED,
                                  GTK_ICON_SIZE_MENU);
        gtk_widget_show (icon);
    }
    else if ((status & MOO_EDIT_MODIFIED) && !(status & MOO_EDIT_CLEAN))
    {
        gtk_image_set_from_stock (GTK_IMAGE (icon),
                                  MOO_STOCK_DOC_MODIFIED,
                                  GTK_ICON_SIZE_MENU);
        gtk_widget_show (icon);
    }
    else
    {
        gtk_widget_hide (icon);
    }

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
/* Panes and notebook
 */

static void     setup_paned             (G_GNUC_UNUSED MooEditWindow *window)
{
    /* XXX */
}


// MooEdit     *moo_edit_window_get_active_doc    (MooEditWindow  *window)
// {
//     g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);
//     g_return_val_if_fail (moo_notebook_get_n_pages (window->priv->notebook) > 0,
//                           NULL);
//     return get_nth_tab (window,
//                         moo_notebook_get_current_page (window->priv->notebook));
// }
//
//
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

    edit = ACTIVE_DOC (window);

    if (!edit)
    {
        gtk_statusbar_pop (window->priv->statusbar,
                           window->priv->statusbar_context_id);
        return;
    }

    gtk_text_buffer_get_iter_at_mark (edit->priv->text_buffer, &iter,
                                      gtk_text_buffer_get_insert (edit->priv->text_buffer));
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
