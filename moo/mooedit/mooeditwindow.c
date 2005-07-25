/*
 *   mooedit/mooeditwindow.c
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
#include "mooedit/mooedit-private.h"
#include "mooedit/mooeditdialogs.h"
#include "mooui/moouiobject-impl.h"
#include "mooui/moomenuaction.h"
#include "mooutils/moocompat.h"
#include "mooutils/moostock.h"
#include "mooutils/moomarshals.h"
#include <string.h>


struct _MooEditWindowPrivate {
    GtkNotebook     *notebook;
    gboolean         use_fullname;
    char            *app_name;
    MooEditor       *editor;
    GSList          *lang_actions;
};


static void     moo_edit_window_class_init  (MooEditWindowClass *klass);

static void     moo_edit_window_init        (MooEditWindow  *window);
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

static gboolean moo_edit_window_close       (MooEditWindow      *window);

static void     set_active_tab              (MooEditWindow      *window,
                                             MooEdit            *edit);
static void     add_tab                     (MooEditWindow      *window,
                                             MooEdit            *edit);
static gboolean close_current_tab           (MooEditWindow      *window);
static gboolean close_tab                   (MooEditWindow      *window,
                                             MooEdit            *edit);
static MooEdit *get_nth_tab                 (MooEditWindow      *window,
                                             int                 n);
static void     notebook_switch_page        (GtkNotebook        *notebook,
                                             GtkNotebookPage    *page,
                                             guint               page_num,
                                             MooEditWindow      *window);
static void     update_window_title         (MooEditWindow      *window);
static void     edit_changed                (MooEdit            *edit,
                                             MooEditWindow      *window);
static void     edit_can_undo_redo          (MooEditWindow      *window);
static GtkWidget *create_tab_label          (MooEdit            *edit);
static void     update_tab_label            (GtkWidget          *label,
                                             MooEdit            *edit);

static GtkMenuItem *create_lang_menu        (MooEditWindow      *window,
                                             MooMenuAction      *action);
static void     lang_menu_item_toggled      (GtkCheckMenuItem   *checkmenuitem,
                                             MooEditWindow      *window);
static void     active_tab_lang_changed     (MooEditWindow      *window);


/* actions */
static void moo_edit_window_new_cb          (MooEditWindow   *window);
static void moo_edit_window_new_tab         (MooEditWindow   *window);
static void moo_edit_window_open_cb         (MooEditWindow   *window);
static void moo_edit_window_close_tab       (MooEditWindow   *window);
static void moo_edit_window_previous_tab    (MooEditWindow   *window);
static void moo_edit_window_next_tab        (MooEditWindow   *window);


/* MOO_TYPE_EDIT_WINDOW */
G_DEFINE_TYPE (MooEditWindow, moo_edit_window, MOO_TYPE_WINDOW)

enum {
    PROP_0,
    PROP_EDITOR
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

    moo_ui_object_class_init (gobject_class, "Editor", "Editor");

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "NewWindow",
                                    "name", "New Window",
                                    "label", "_New Window",
                                    "tooltip", "Open new editor window",
                                    "icon-stock-id", GTK_STOCK_NEW,
                                    "accel", "<ctrl>N",
                                    "closure::callback", moo_edit_window_new_cb,
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
                                    "closure::callback", moo_edit_window_open_cb,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "Reload",
                                    "name", "Reload",
                                    "label", "_Reload",
                                    "tooltip", "Reload document",
                                    "icon-stock-id", GTK_STOCK_REFRESH,
                                    "accel", "F5",
                                    "closure::signal", "reload-interactive",
                                    "closure::proxy-func", moo_edit_window_get_active_doc,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "Save",
                                    "name", "Save",
                                    "label", "_Save",
                                    "tooltip", "Save document",
                                    "icon-stock-id", GTK_STOCK_SAVE,
                                    "accel", "<ctrl>S",
                                    "closure::callback", moo_edit_save,
                                    "closure::proxy-func", moo_edit_window_get_active_doc,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "SaveAs",
                                    "name", "Save As",
                                    "label", "Save _As...",
                                    "tooltip", "Save as...",
                                    "icon-stock-id", GTK_STOCK_SAVE_AS,
                                    "accel", "<ctrl><shift>S",
                                    "closure::signal", "save-as-interactive",
                                    "closure::proxy-func", moo_edit_window_get_active_doc,
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
}


static void     moo_edit_window_init        (MooEditWindow  *window)
{
    window->priv = g_new0 (MooEditWindowPrivate, 1);
    gtk_window_set_default_size (GTK_WINDOW (window), 500, 450);
    g_object_set (G_OBJECT (window),
                  "menubar-ui-name", "Editor/Menubar",
                  "toolbar-ui-name", "Editor/Toolbar",
                  NULL);
}


GObject        *moo_edit_window_constructor (GType                  type,
                                             guint                  n_props,
                                             GObjectConstructParam *props)
{
    GtkWidget *notebook;
    MooEdit *edit;
    MooEditWindow *window;

    GObject *object =
            G_OBJECT_CLASS(moo_edit_window_parent_class)->constructor (type, n_props, props);

    window = MOO_EDIT_WINDOW (object);

    gtk_widget_show (MOO_WINDOW(window)->vbox);
    notebook = gtk_notebook_new ();
    gtk_widget_show (notebook);
    gtk_box_pack_start (GTK_BOX (MOO_WINDOW(window)->vbox), notebook, TRUE, TRUE, 0);

    window->priv->notebook = GTK_NOTEBOOK (notebook);
    g_signal_connect_after (window->priv->notebook, "switch-page",
                            G_CALLBACK (notebook_switch_page), window);

    window = MOO_EDIT_WINDOW (object);
    edit = MOO_EDIT (_moo_edit_new (window->priv->editor));
    add_tab (window, edit);
    gtk_widget_grab_focus (GTK_WIDGET (edit));
    GTK_WIDGET_SET_FLAGS (edit, GTK_CAN_DEFAULT);
    gtk_widget_grab_default (GTK_WIDGET (edit));

    g_signal_connect (window, "realize", G_CALLBACK (update_window_title), NULL);

    return object;
}


static void moo_edit_window_finalize       (GObject      *object)
{
    MooEditWindow *window = MOO_EDIT_WINDOW (object);
    g_free (window->priv->app_name);
    g_free (window->priv);
    G_OBJECT_CLASS (moo_edit_window_parent_class)->finalize (object);
}


#define SCROLLED_WINDOW_QUARK (scrolled_window_quark())
static GQuark scrolled_window_quark (void)
{
    static GQuark q = 0;
    if (!q)
        q = g_quark_from_static_string ("moo_edit_window_edit_scrolled_window");
    return q;
}


static void     add_tab                 (MooEditWindow      *window,
                                         MooEdit            *edit)
{
    GtkWidget *label;
    GtkWidget *scrolledwindow;
    int n;

    g_return_if_fail (MOO_IS_WINDOW (window) && MOO_EDIT (edit));

    label = create_tab_label (edit);
    gtk_widget_show (label);
    scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (scrolledwindow), GTK_WIDGET (edit));
    gtk_widget_show_all (scrolledwindow);
    g_object_set_qdata (G_OBJECT (edit), SCROLLED_WINDOW_QUARK, scrolledwindow);

    if (!gtk_notebook_get_n_pages (window->priv->notebook))
        gtk_notebook_set_show_tabs (window->priv->notebook, FALSE);
    else
        gtk_notebook_set_show_tabs (window->priv->notebook, TRUE);

#if GTK_MINOR_VERSION >= 4
    n = gtk_notebook_append_page (window->priv->notebook, scrolledwindow, label);
#else /* GTK_MINOR_VERSION < 4 */
    gtk_notebook_append_page (window->priv->notebook, scrolledwindow, label);
    n = gtk_notebook_page_num (window->priv->notebook, scrolledwindow);
#endif /* GTK_MINOR_VERSION < 4 */

    gtk_notebook_set_current_page (window->priv->notebook, n);
    g_signal_connect (edit, "doc_status_changed",
                      G_CALLBACK (edit_changed), window);
    g_signal_connect_swapped (edit, "can-undo",
                              G_CALLBACK (edit_can_undo_redo), window);
    g_signal_connect_swapped (edit, "can-redo",
                              G_CALLBACK (edit_can_undo_redo), window);
    g_signal_connect_swapped (edit, "lang-changed",
                              G_CALLBACK (active_tab_lang_changed), window);

    active_tab_lang_changed (window);
}


static void     notebook_switch_page        (G_GNUC_UNUSED GtkNotebook        *notebook,
                                             G_GNUC_UNUSED GtkNotebookPage    *page,
                                             guint               page_num,
                                             MooEditWindow      *window)
{
    edit_changed (get_nth_tab (window, page_num), window);
}


static MooEdit *get_nth_tab             (MooEditWindow      *window,
                                         int                 n)
{
    GtkWidget *swin = gtk_notebook_get_nth_page (window->priv->notebook, n);
    g_return_val_if_fail (swin != NULL, NULL);
    return MOO_EDIT (gtk_bin_get_child (GTK_BIN (swin)));
}


static void     set_active_tab              (MooEditWindow      *window,
                                             MooEdit            *edit)
{
    int num = gtk_notebook_page_num (window->priv->notebook,
                                     g_object_get_qdata (G_OBJECT (edit),
                                             SCROLLED_WINDOW_QUARK));
    gtk_notebook_set_current_page (window->priv->notebook, num);
}


static void     update_window_title         (MooEditWindow      *window)
{
    MooEdit *edit;
    const char *name;
    MooEditDocStatus status;
    gboolean modified, deleted, modified_on_disk, clean;
    GString *title;

    edit = moo_edit_window_get_active_doc (window);
    g_return_if_fail (edit != NULL);
    if (window->priv->use_fullname)
        name = moo_edit_get_display_filename (edit);
    else
        name = moo_edit_get_display_basename (edit);
    if (!name) name = "<\?\?\?\?\?>";

    status = moo_edit_get_doc_status (edit);
    clean = status & MOO_EDIT_DOC_CLEAN;
    modified = status & MOO_EDIT_DOC_MODIFIED;
    deleted = status & MOO_EDIT_DOC_DELETED;
    modified_on_disk = status & MOO_EDIT_DOC_MODIFIED_ON_DISK;

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


static void     edit_changed                (G_GNUC_UNUSED MooEdit            *edit,
                                             MooEditWindow      *window)
{
    update_window_title (window);
    edit_can_undo_redo (window);
    active_tab_lang_changed (window);
}


static void     edit_can_undo_redo      (MooEditWindow      *window)
{
    MooEdit *edit;
    gboolean can_redo, can_undo;
    MooActionGroup *actions;
    MooAction *undo, *redo;

    g_return_if_fail (window != NULL);
    edit = moo_edit_window_get_active_doc (window);
    g_return_if_fail (edit != NULL);

    can_redo = moo_edit_can_redo (edit);
    can_undo = moo_edit_can_undo (edit);

    actions = moo_ui_object_get_actions (MOO_UI_OBJECT (window));
    undo = moo_action_group_get_action (actions, "Undo");
    redo = moo_action_group_get_action (actions, "Redo");
    g_return_if_fail (undo != NULL && redo != NULL);

    moo_action_set_sensitive (undo, can_undo);
    moo_action_set_sensitive (redo, can_redo);
}


static GtkWidget *create_tab_label          (MooEdit            *edit)
{
    GtkWidget *hbox, *filename, *icon;

    hbox = gtk_hbox_new (FALSE, 0);
    icon = gtk_image_new ();
    filename = gtk_label_new (moo_edit_get_display_basename (edit));
    gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), filename, TRUE, TRUE, 3);

    g_object_set_data (G_OBJECT (hbox), "filename", filename);
    g_object_set_data (G_OBJECT (hbox), "icon", icon);
    gtk_widget_show (filename);

    g_signal_connect_swapped (edit, "doc-status-changed",
                              G_CALLBACK (update_tab_label), hbox);

    return hbox;
}


static void     update_tab_label            (GtkWidget          *label,
                                             MooEdit            *edit)
{
    const char *name;
    MooEditDocStatus status;
    gboolean clean, modified, deleted, modified_on_disk;
    GString *title;
    GtkWidget *filename, *icon;

    filename = g_object_get_data (G_OBJECT (label), "filename");
    icon = g_object_get_data (G_OBJECT (label), "icon");

    name = moo_edit_get_display_basename (edit);
    if (!name) name = "<\?\?\?\?\?>";

    status = moo_edit_get_doc_status (edit);
    clean = status & MOO_EDIT_DOC_CLEAN;
    modified = status & MOO_EDIT_DOC_MODIFIED;
    deleted = status & MOO_EDIT_DOC_DELETED;
    modified_on_disk = status & MOO_EDIT_DOC_MODIFIED_ON_DISK;

    title = g_string_new ("");
    g_string_append_printf (title, "%s%s",
                            (modified && !clean) ? "* " : "",
                            name);
    gtk_label_set_text (GTK_LABEL (filename), title->str);
    g_string_free (title, TRUE);

    if (deleted || modified_on_disk)
        gtk_widget_show (icon);
    else
        gtk_widget_hide (icon);

    if (deleted)
        gtk_image_set_from_stock (GTK_IMAGE (icon),
                                  MOO_STOCK_DOC_DELETED,
                                  GTK_ICON_SIZE_MENU);
    else if (modified_on_disk)
        gtk_image_set_from_stock (GTK_IMAGE (icon),
                                  MOO_STOCK_DOC_MODIFIED_ON_DISK,
                                  GTK_ICON_SIZE_MENU);
}


static gboolean moo_edit_window_close       (MooEditWindow      *window)
{
    /* TODO */

    while (gtk_notebook_get_n_pages (window->priv->notebook) &&
           close_current_tab (window)) ;

    if (gtk_notebook_get_n_pages (window->priv->notebook))
        return TRUE;
    else
        return FALSE;
}


static gboolean     close_current_tab       (MooEditWindow      *window)
{
    MooEdit *edit = moo_edit_window_get_active_doc (window);
    g_return_val_if_fail (edit != NULL, FALSE);
    return close_tab (window, edit);
}


static gboolean close_tab                   (MooEditWindow      *window,
                                             MooEdit            *edit)
{
    if (moo_edit_close (edit)) {
        GtkWidget *label =
                gtk_notebook_get_tab_label (window->priv->notebook,
                                            g_object_get_qdata (G_OBJECT (edit),
                                                    SCROLLED_WINDOW_QUARK));
        g_signal_handlers_disconnect_by_func(edit, update_tab_label, label);
        gtk_notebook_remove_page (window->priv->notebook,
                                  gtk_notebook_get_current_page (window->priv->notebook));
        if (gtk_notebook_get_n_pages (window->priv->notebook) <= 1)
            gtk_notebook_set_show_tabs (window->priv->notebook, FALSE);
        return TRUE;
    }
    else
        return FALSE;
}


static void moo_edit_window_new_cb          (MooEditWindow   *window)
{
    if (window->priv->editor)
        moo_editor_new_window (window->priv->editor);
    else
        gtk_widget_show (_moo_edit_window_new (NULL));
}


static void moo_edit_window_new_tab         (MooEditWindow   *window)
{
    MooEdit *edit = _moo_edit_new (window->priv->editor);
    add_tab (window, edit);
}


static void moo_edit_window_open_cb         (MooEditWindow   *window)
{
    moo_edit_window_open (window, NULL, NULL);
}


static void moo_edit_window_close_tab       (MooEditWindow   *window)
{
    close_current_tab (window);
    if (!gtk_notebook_get_n_pages (window->priv->notebook))
        moo_edit_window_new_tab (window);
}


static void moo_edit_window_previous_tab    (MooEditWindow   *window)
{
    int n = gtk_notebook_get_current_page (window->priv->notebook);
    if (n <= 0)
        gtk_notebook_set_current_page (window->priv->notebook, -1);
    else
        gtk_notebook_set_current_page (window->priv->notebook, n - 1);
}


static void moo_edit_window_next_tab        (MooEditWindow   *window)
{
    int n = gtk_notebook_get_current_page (window->priv->notebook);
    if (n == gtk_notebook_get_n_pages (window->priv->notebook) - 1)
        gtk_notebook_set_current_page (window->priv->notebook, 0);
    else
        gtk_notebook_set_current_page (window->priv->notebook, n + 1);
}


GtkWidget   *_moo_edit_window_new               (MooEditor  *editor)
{
    MooEditWindow *window =
            MOO_EDIT_WINDOW (g_object_new (MOO_TYPE_EDIT_WINDOW,
                                           "editor", editor,
                                           NULL));
    return GTK_WIDGET (window);
}


gboolean     moo_edit_window_open               (MooEditWindow  *window,
                                                 const char     *filename,
                                                 const char     *encoding)
{
    MooEdit *edit, *old_edit;
    MooEditFileInfo *info = NULL;
    gboolean result;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), FALSE);

    if (!filename) {
        info = moo_edit_open_dialog (GTK_WIDGET (window));
        if (!info)
            return FALSE;
        else {
            filename = info->filename;
            encoding = info->encoding;
        }
    }

    edit = old_edit = moo_edit_window_get_active_doc (window);
    if (!edit) {
        moo_edit_file_info_free (info);
        g_return_val_if_fail (edit != NULL, FALSE);
    }

    if (!moo_edit_is_empty (edit)) {
        edit = _moo_edit_new (window->priv->editor);
        add_tab (window, edit);
    }

    result = moo_edit_open (edit, filename, encoding);
    moo_edit_file_info_free (info);
    if (!result && old_edit != edit) {
        close_tab (window, edit);
        set_active_tab (window, old_edit);
    }
    return result;
}


MooEdit     *moo_edit_window_get_active_doc    (MooEditWindow  *window)
{
    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);
    g_return_val_if_fail (gtk_notebook_get_n_pages (window->priv->notebook) > 0,
                          NULL);
    return get_nth_tab (window,
                        gtk_notebook_get_current_page (window->priv->notebook));
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


static void     moo_edit_window_set_property(GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec)
{
    MooEditWindow *window = MOO_EDIT_WINDOW (object);
    switch (prop_id) {
        case PROP_EDITOR:
            window->priv->editor = MOO_EDITOR (g_value_get_object (value));
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

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static GtkMenuItem *create_lang_menu        (MooEditWindow      *window,
                                             MooMenuAction      *action)
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
    main_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (main_item), main_menu);
    none = gtk_radio_menu_item_new_with_label (group, "None");
    group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (none));
    gtk_menu_shell_append (GTK_MENU_SHELL (main_menu), none);
    g_signal_connect (none, "toggled",
                      G_CALLBACK (lang_menu_item_toggled),
                      window);
    /* TODO TODO TODO crash */
    g_object_set_qdata (G_OBJECT (action),
                        g_quark_from_string ("moo_edit_window_null_lang"),
                        none);

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

        g_object_set_qdata (G_OBJECT (action),
                            g_quark_from_string (moo_edit_lang_get_id (lang)),
                            item);

        link = g_slist_nth (section_menus, pos);
        g_assert (link != NULL);
        gtk_menu_shell_append (GTK_MENU_SHELL (link->data), item);
    }

    /* TODO TODO TODO crash */
    window->priv->lang_actions =
               g_slist_append (window->priv->lang_actions, action);
    gtk_widget_show_all (main_item);
    active_tab_lang_changed (window);

    g_slist_foreach (sections, (GFunc) g_free, NULL);
    g_slist_free (sections);
    g_slist_free (section_items);
    g_slist_free (section_menus);

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
    g_return_if_fail (editor != NULL);
    edit = moo_edit_window_get_active_doc (window);
    if (!edit)
        return;

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
    GSList *l;

    edit = moo_edit_window_get_active_doc (window);
    if (!edit)
        return;

    lang = moo_edit_get_lang (edit);
    if (lang)
        lang_id = moo_edit_lang_get_id (lang);
    else
        lang_id = "moo_edit_window_null_lang";

    for (l = window->priv->lang_actions; l != NULL; l = l->next)
    {
        GtkCheckMenuItem *item =
                g_object_get_qdata (G_OBJECT (l->data),
                                    g_quark_try_string (lang_id));
        g_assert (item != NULL);
        g_signal_handlers_block_by_func (item, lang_menu_item_toggled, window);
        gtk_check_menu_item_set_active (item, TRUE);
        g_signal_handlers_unblock_by_func (item, lang_menu_item_toggled, window);
    }
}
