/*
 *   mooedit/moofileview.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef GTK_DISABLE_DEPRECATED
# undef GTK_DISABLE_DEPRECATED
# include <gtk/gtktoolbar.h>
# define GTK_DISABLE_DEPRECATED
#endif

#include "mooedit/moofileview.h"
#include "mooedit/mooeditfilemgr.h"
#include "mooedit/moofilesystem.h"
#include "mooedit/moofoldermodel.h"
#include "mooutils/moomarshals.h"
#include "mooutils/mooiconview.h"
#include "mooutils/moosignal.h"
#include <gdk/gdkkeysyms.h>
#include <glib/gstdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef USE_XDGMIME
#include "mooedit/xdgmime/xdgmime.h"
#endif

#define TIMEOUT 0.1

enum {
    TREEVIEW_PAGE = 0,
    ICONVIEW_PAGE = 1
};


typedef struct _History History;
typedef struct _Completion Completion;

struct _MooFileViewPrivate {
    GtkTreeModel    *model;
    GtkTreeModel    *filter_model;
    MooFolder       *current_dir;
    MooFileSystem   *file_system;

    GtkIconSize      icon_size;
    GtkNotebook     *notebook;
    MooFileViewType  view_type;

    GtkTreeView     *treeview;
    GtkTreeViewColumn *tree_name_column;

    MooIconView     *iconview;

    gboolean         show_hidden_files;
    gboolean         show_two_dots;
    History         *history;
    char            *temp_visible;  /* tempporary visible name, for interactive search */
    GtkTreeRowReference *temp_visible_row;  /* row containing temp_visible (in the store) */

    MooEditFileMgr  *mgr;
    GtkToggleButton *filter_button;
    GtkComboBox     *filter_combo;
    GtkEntry        *filter_entry;
    GtkFileFilter   *current_filter;
    gboolean         use_current_filter;

    GtkEntry        *search_entry;
    gboolean         in_search;
    Completion      *completion;
};


static void         moo_file_view_finalize      (GObject        *object);
static void         moo_file_view_set_property  (GObject        *object,
                                                 guint           prop_id,
                                                 const GValue   *value,
                                                 GParamSpec     *pspec);
static void         moo_file_view_get_property  (GObject        *object,
                                                 guint           prop_id,
                                                 GValue         *value,
                                                 GParamSpec     *pspec);

static gboolean     moo_file_view_key_press     (GtkWidget      *widget,
                                                 GdkEventKey    *event,
                                                 MooFileView    *fileview);

static void         moo_file_view_set_file_mgr  (MooFileView    *fileview,
                                                 MooEditFileMgr *mgr);

static void         moo_file_view_set_current_dir (MooFileView  *fileview,
                                                 MooFolder      *folder);
static gboolean     moo_file_view_chdir_real    (MooFileView    *fileview,
                                                 const char     *dir,
                                                 GError        **error);

static void         moo_file_view_go_up     (MooFileView    *fileview);
static void         moo_file_view_go_home   (MooFileView    *fileview);
static void         moo_file_view_go_back   (MooFileView    *fileview);
static void         moo_file_view_go_forward(MooFileView    *fileview);
static void         toggle_show_hidden      (MooFileView    *fileview);

static void         history_init            (MooFileView    *fileview);
static void         history_free            (MooFileView    *fileview);
static void         history_clear           (MooFileView    *fileview);
static void         history_goto            (MooFileView    *fileview,
                                             const char     *dirname);
static const char  *history_go              (MooFileView    *fileview,
                                             GtkDirectionType where);
static void         history_revert_go       (MooFileView    *fileview);

static gboolean     filter_visible_func     (GtkTreeModel       *model,
                                             GtkTreeIter        *iter,
                                             MooFileView        *fileview);

static gboolean     moo_file_view_check_visible (MooFileView        *fileview,
                                                 MooFile            *file,
                                                 gboolean            ignore_hidden,
                                                 gboolean            ignore_two_dots);

static void icon_data_func  (GObject            *column_or_iconview,
                             GtkCellRenderer    *cell,
                             GtkTreeModel       *model,
                             GtkTreeIter        *iter,
                             MooFileView        *fileview);
static void name_data_func  (GObject            *column_or_iconview,
                             GtkCellRenderer    *cell,
                             GtkTreeModel       *model,
                             GtkTreeIter        *iter,
                             MooFileView        *fileview);
#ifdef USE_SIZE_AND_STUFF
static void date_data_func  (GObject            *column_or_iconview,
                             GtkCellRenderer    *cell,
                             GtkTreeModel       *model,
                             GtkTreeIter        *iter,
                             MooFileView        *fileview);
static void size_data_func  (GObject            *column_or_iconview,
                             GtkCellRenderer    *cell,
                             GtkTreeModel       *model,
                             GtkTreeIter        *iter,
                             MooFileView        *fileview);
#endif

static void         init_gui                (MooFileView    *fileview);
static void         focus_to_file_view      (MooFileView    *fileview);
static void         focus_to_filter_entry   (MooFileView    *fileview);
static GtkWidget   *create_toolbar          (MooFileView    *fileview);
static GtkWidget   *create_notebook         (MooFileView    *fileview);

static GtkWidget   *create_popup_menu       (MooFileView    *fileview);

static void         goto_item_activated     (GtkWidget      *widget,
                                             MooFileView    *fileview);
static void         show_hidden_toggled     (GtkWidget      *widget,
                                             MooFileView    *fileview);
static void         show_two_dots_toggled   (GtkWidget      *widget,
                                             MooFileView    *fileview);
static void         view_type_item_toggled  (GtkWidget      *widget,
                                             MooFileView    *fileview);

static GtkWidget   *create_filter_combo     (MooFileView    *fileview);
static void         init_filter_combo       (MooFileView    *fileview);
static void         filter_button_toggled   (MooFileView    *fileview);
static void         filter_combo_changed    (MooFileView    *fileview);
static void         filter_entry_activate   (MooFileView    *fileview);
static void         fileview_set_filter     (MooFileView    *fileview,
                                             GtkFileFilter  *filter);
static void         fileview_set_use_filter (MooFileView    *fileview,
                                             gboolean        use,
                                             gboolean        block_signals);

static GtkWidget   *create_treeview         (MooFileView    *fileview);
static void         tree_row_activated      (GtkTreeView    *treeview,
                                             GtkTreePath    *treepath,
                                             GtkTreeViewColumn *column,
                                             MooFileView    *fileview);
static gboolean     tree_button_press       (GtkTreeView    *treeview,
                                             GdkEventButton *event,
                                             MooFileView    *fileview);

static GtkWidget   *create_iconview         (MooFileView    *fileview);
static void         icon_item_activated     (MooIconView    *iconview,
                                             GtkTreePath    *treepath,
                                             MooFileView    *fileview);
static gboolean     icon_button_press       (MooIconView    *iconview,
                                             GdkEventButton *event,
                                             MooFileView    *fileview);

static GtkWidget   *get_file_view_widget    (MooFileView    *fileview);
static void         file_view_move_selection(MooFileView    *fileview,
                                             GtkTreeIter    *filter_iter);

static void         completion_init         (MooFileView    *fileview);
static void         completion_free         (MooFileView    *fileview);


/* MOO_TYPE_FILE_VIEW */
G_DEFINE_TYPE (MooFileView, moo_file_view, GTK_TYPE_VBOX)

enum {
    PROP_0,
    PROP_CURRENT_DIRECTORY,
    PROP_FILE_MGR
};

enum {
    CHDIR,
    ACTIVATE,
    POPULATE_POPUP,
    GO_UP,
    GO_BACK,
    GO_FORWARD,
    GO_HOME,
    FOCUS_TO_FILTER_ENTRY,
    FOCUS_TO_FILE_VIEW,
    TOGGLE_SHOW_HIDDEN,
    LAST_SIGNAL
};

#define COLUMN_FILE MOO_FOLDER_MODEL_COLUMN_FILE

static guint signals[LAST_SIGNAL];

static void moo_file_view_class_init (MooFileViewClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkBindingSet *binding_set;

    gobject_class->finalize = moo_file_view_finalize;
    gobject_class->set_property = moo_file_view_set_property;
    gobject_class->get_property = moo_file_view_get_property;

    klass->chdir = moo_file_view_chdir_real;

    g_object_class_install_property (gobject_class,
                                     PROP_CURRENT_DIRECTORY,
                                     g_param_spec_string ("current-directory",
                                             "current-directory",
                                             "current-directory",
                                             NULL,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_FILE_MGR,
                                     g_param_spec_object ("file-mgr",
                                             "file-mgr",
                                             "file-mgr",
                                             MOO_TYPE_EDIT_FILE_MGR,
                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

    signals[CHDIR] =
            g_signal_new ("chdir",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooFileViewClass, chdir),
                          NULL, NULL,
                          _moo_marshal_BOOLEAN__STRING_POINTER,
                          G_TYPE_BOOLEAN, 2,
                          G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE,
                          G_TYPE_POINTER);

    signals[ACTIVATE] =
            moo_signal_new_cb ("activate",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                               NULL,
                               NULL, NULL,
                               _moo_marshal_VOID__STRING,
                               G_TYPE_NONE, 1,
                               G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);

    signals[POPULATE_POPUP] =
            moo_signal_new_cb ("populate-popup",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                               NULL,
                               NULL, NULL,
                               _moo_marshal_VOID__STRING_OBJECT,
                               G_TYPE_NONE, 2,
                               G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE,
                               GTK_TYPE_MENU);

    signals[GO_UP] =
            moo_signal_new_cb ("go-up",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                               G_CALLBACK (moo_file_view_go_up),
                               NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);

    signals[GO_FORWARD] =
            moo_signal_new_cb ("go-forward",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                               G_CALLBACK (moo_file_view_go_forward),
                               NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);

    signals[GO_BACK] =
            moo_signal_new_cb ("go-back",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                               G_CALLBACK (moo_file_view_go_back),
                               NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);

    signals[GO_HOME] =
            moo_signal_new_cb ("go-home",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                               G_CALLBACK (moo_file_view_go_home),
                               NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);

    signals[FOCUS_TO_FILTER_ENTRY] =
            moo_signal_new_cb ("focus-to-filter-entry",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                               G_CALLBACK (focus_to_filter_entry),
                               NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);

    signals[FOCUS_TO_FILE_VIEW] =
            moo_signal_new_cb ("focus-to-file-view",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                               G_CALLBACK (focus_to_file_view),
                               NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);

    signals[TOGGLE_SHOW_HIDDEN] =
            moo_signal_new_cb ("toggle-show-hidden",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                               G_CALLBACK (toggle_show_hidden),
                               NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);

    binding_set = gtk_binding_set_by_class (klass);

    gtk_binding_entry_add_signal (binding_set,
                                  GDK_Up, GDK_MOD1_MASK,
                                  "go-up",
                                  0);
    gtk_binding_entry_add_signal (binding_set,
                                  GDK_KP_Up, GDK_MOD1_MASK,
                                  "go-up",
                                  0);
    gtk_binding_entry_add_signal (binding_set,
                                  GDK_Left, GDK_MOD1_MASK,
                                  "go-back",
                                  0);
    gtk_binding_entry_add_signal (binding_set,
                                  GDK_KP_Left, GDK_MOD1_MASK,
                                  "go-back",
                                  0);
    gtk_binding_entry_add_signal (binding_set,
                                  GDK_Right, GDK_MOD1_MASK,
                                  "go-forward",
                                  0);
    gtk_binding_entry_add_signal (binding_set,
                                  GDK_KP_Right, GDK_MOD1_MASK,
                                  "go-forward",
                                  0);
    gtk_binding_entry_add_signal (binding_set,
                                  GDK_Home, GDK_MOD1_MASK,
                                  "go-home",
                                  0);
    gtk_binding_entry_add_signal (binding_set,
                                  GDK_KP_Home, GDK_MOD1_MASK,
                                  "go-home",
                                  0);

    gtk_binding_entry_add_signal (binding_set,
                                  GDK_f, GDK_MOD1_MASK | GDK_SHIFT_MASK,
                                  "focus-to-filter-entry",
                                  0);
    gtk_binding_entry_add_signal (binding_set,
                                  GDK_b, GDK_MOD1_MASK | GDK_SHIFT_MASK,
                                  "focus-to-file-view",
                                  0);
    gtk_binding_entry_add_signal (binding_set,
                                  GDK_h, GDK_MOD1_MASK | GDK_SHIFT_MASK,
                                  "toggle-show-hidden",
                                  0);
}


static void moo_file_view_init      (MooFileView *fileview)
{
    fileview->priv = g_new0 (MooFileViewPrivate, 1);

    fileview->priv->show_hidden_files = FALSE;

    fileview->priv->view_type = MOO_FILE_VIEW_ICON;
    fileview->priv->use_current_filter = FALSE;

    history_init (fileview);

    fileview->priv->model = moo_folder_model_new (NULL);
    fileview->priv->filter_model = gtk_tree_model_filter_new (fileview->priv->model, NULL);
    gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (fileview->priv->filter_model),
                                            (GtkTreeModelFilterVisibleFunc) filter_visible_func,
                                            fileview, NULL);
    init_gui (fileview);
    completion_init (fileview);

    fileview->priv->current_dir = NULL;
    fileview->priv->file_system = moo_file_system_create ();

    fileview->priv->icon_size = GTK_ICON_SIZE_MENU;
}


static void moo_file_view_finalize  (GObject      *object)
{
    MooFileView *fileview = MOO_FILE_VIEW (object);

    g_object_unref (fileview->priv->model);
    g_object_unref (fileview->priv->filter_model);
    history_free (fileview);

    if (fileview->priv->mgr)
        g_object_unref (fileview->priv->mgr);
    if (fileview->priv->current_filter)
        g_object_unref (fileview->priv->current_filter);

    g_free (fileview->priv->temp_visible);
    gtk_tree_row_reference_free (fileview->priv->temp_visible_row);

    if (fileview->priv->current_dir)
        g_object_unref (fileview->priv->current_dir);
    g_object_unref (fileview->priv->file_system);

    completion_free (fileview);

    g_free (fileview->priv);
    fileview->priv = NULL;

    G_OBJECT_CLASS (moo_file_view_parent_class)->finalize (object);
}


GtkWidget   *moo_file_view_new              (void)
{
    return GTK_WIDGET (g_object_new (MOO_TYPE_FILE_VIEW, NULL));
}


static void         moo_file_view_set_current_dir (MooFileView  *fileview,
                                                   MooFolder    *folder)
{
    GtkTreeIter filter_iter;

    g_return_if_fail (MOO_IS_FILE_VIEW (fileview));
    g_return_if_fail (!folder || MOO_IS_FOLDER (folder));

    if (folder == fileview->priv->current_dir)
        return;

    if (!folder)
    {
        if (fileview->priv->current_dir)
        {
            g_object_unref (fileview->priv->current_dir);
            fileview->priv->current_dir = NULL;
            history_clear (fileview);
            moo_folder_model_set_folder (MOO_FOLDER_MODEL (fileview->priv->model),
                                         NULL);
        }

        return;
    }

    if (fileview->priv->current_dir)
        g_object_unref (fileview->priv->current_dir);

    fileview->priv->current_dir = g_object_ref (folder);
    moo_folder_model_set_folder (MOO_FOLDER_MODEL (fileview->priv->model),
                                 folder);

    if (gtk_tree_model_get_iter_first (fileview->priv->filter_model, &filter_iter))
        file_view_move_selection (fileview, &filter_iter);

    history_goto (fileview, moo_folder_get_path (folder));
}


static gboolean     moo_file_view_chdir_real(MooFileView    *fileview,
                                             const char     *new_dir,
                                             GError        **error)
{
    char *real_new_dir;
    MooFolder *folder;

    g_return_val_if_fail (MOO_IS_FILE_VIEW (fileview), FALSE);

    if (!new_dir)
    {
        moo_file_view_set_current_dir (fileview, NULL);
        return TRUE;
    }

    if (g_path_is_absolute (new_dir) || !fileview->priv->current_dir)
    {
        real_new_dir = g_strdup (new_dir);
    }
    else
    {
        real_new_dir = g_build_filename (moo_folder_get_path (fileview->priv->current_dir),
                                         new_dir, NULL);
    }

    folder = moo_file_system_get_folder (fileview->priv->file_system,
                                         real_new_dir,
                                         MOO_FILE_ALL_FLAGS,
                                         error);
    g_free (real_new_dir);

    if (!folder)
        return FALSE;

    moo_file_view_set_current_dir (fileview, folder);
    g_object_unref (folder);
    return TRUE;
}


static void         init_gui        (MooFileView    *fileview)
{
    GtkBox *box;
    GtkWidget *toolbar, *notebook, *filter_combo, *search_entry;

    box = GTK_BOX (fileview);

    toolbar = create_toolbar (fileview);
    gtk_widget_show (toolbar);
    gtk_box_pack_start (box, toolbar, FALSE, FALSE, 0);
    fileview->toolbar = toolbar;

    notebook = create_notebook (fileview);
    gtk_widget_show (notebook);
    gtk_box_pack_start (box, notebook, TRUE, TRUE, 0);
    fileview->priv->notebook = GTK_NOTEBOOK (notebook);

    search_entry = gtk_entry_new ();
    g_object_set_data (G_OBJECT (search_entry), "moo-file-view", fileview);
    gtk_widget_set_no_show_all (search_entry, TRUE);
    gtk_box_pack_start (box, search_entry, FALSE, FALSE, 0);
    fileview->priv->search_entry = GTK_ENTRY (search_entry);

    filter_combo = create_filter_combo (fileview);
    gtk_widget_show (filter_combo);
    gtk_box_pack_start (box, filter_combo, FALSE, FALSE, 0);

    if (fileview->priv->view_type == MOO_FILE_VIEW_ICON)
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook),
                                       ICONVIEW_PAGE);
    else
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook),
                                       TREEVIEW_PAGE);

    focus_to_file_view (fileview);
}


static void         focus_to_file_view      (MooFileView    *fileview)
{
    gtk_widget_grab_focus (get_file_view_widget (fileview));
}


static void         focus_to_filter_entry   (MooFileView    *fileview)
{
    gtk_widget_grab_focus (GTK_WIDGET(fileview->priv->filter_entry));
}


void        moo_file_view_set_view_type     (MooFileView    *fileview,
                                             MooFileViewType type)
{
    MooIconView *iconview;
    GtkTreeView *treeview;
    GtkTreeSelection *selection;
    GtkTreePath *path;
    GtkTreeIter iter;

    g_return_if_fail (type == MOO_FILE_VIEW_ICON ||
            type == MOO_FILE_VIEW_LIST);

    if (fileview->priv->view_type == type)
        return;

    iconview = fileview->priv->iconview;
    treeview = fileview->priv->treeview;
    selection = gtk_tree_view_get_selection (treeview);

    fileview->priv->view_type = type;

    if (type == MOO_FILE_VIEW_LIST)
    {
        path = moo_icon_view_get_selected (iconview);
        gtk_tree_selection_unselect_all (selection);
        if (path)
        {
            gtk_tree_selection_select_path (selection, path);
            gtk_tree_path_free (path);
        }
    }
    else
    {
        GtkTreeModel *model = NULL;

        if (gtk_tree_selection_get_selected (selection, &model, &iter))
        {
            path = gtk_tree_model_get_path (model, &iter);
            moo_icon_view_select_path (iconview, path);
            gtk_tree_path_free (path);
        }
        else
        {
            moo_icon_view_select_path (iconview, NULL);
        }
    }

    if (type == MOO_FILE_VIEW_ICON)
        gtk_notebook_set_current_page (fileview->priv->notebook,
                                       ICONVIEW_PAGE);
    else
        gtk_notebook_set_current_page (fileview->priv->notebook,
                                       TREEVIEW_PAGE);
}


static void create_button (MooFileView  *fileview,
                           GtkWidget    *box,
                           GtkTooltips  *tooltips,
                           const char   *stock_id,
                           const char   *tip,
                           const char   *signal)
{
    GtkWidget *icon, *button;

    icon = gtk_image_new_from_stock (stock_id,
                                     GTK_ICON_SIZE_MENU);
    gtk_widget_show (GTK_WIDGET (icon));
    button = gtk_button_new ();
    gtk_container_add (GTK_CONTAINER (button), icon);
    gtk_widget_show (button);
    gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
    gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
    gtk_tooltips_set_tip (tooltips, button, tip, tip);
    gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
    g_object_set_data (G_OBJECT (button), "moo-file-view-signal",
                       (gpointer) signal);
    g_signal_connect (button, "clicked",
                      G_CALLBACK (goto_item_activated),
                      fileview);
}

static GtkWidget   *create_toolbar  (MooFileView    *fileview)
{
    GtkWidget *toolbar;
    GtkTooltips *tooltips;

    tooltips = gtk_tooltips_new ();
    toolbar = gtk_hbox_new (FALSE, 0);

    create_button (fileview, toolbar, tooltips,
                   GTK_STOCK_GO_UP, "Up", "go-up");
    create_button (fileview, toolbar, tooltips,
                   GTK_STOCK_GO_BACK, "Back", "go-back");
    create_button (fileview, toolbar, tooltips,
                   GTK_STOCK_GO_FORWARD, "Forward", "go-forward");
    create_button (fileview, toolbar, tooltips,
                   GTK_STOCK_HOME, "Home", "go-home");

    return toolbar;
}


static void         goto_item_activated     (GtkWidget      *widget,
                                             MooFileView    *fileview)
{
    const char *signal = g_object_get_data (G_OBJECT (widget),
                                            "moo-file-view-signal");
    g_return_if_fail (signal != NULL);
    g_signal_emit_by_name (fileview, signal);
}


static GtkWidget   *create_notebook     (MooFileView    *fileview)
{
    GtkWidget *notebook, *swin, *treeview, *iconview;

    notebook = gtk_notebook_new ();
    gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), FALSE);

    swin = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (swin);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), swin, NULL);
    treeview = create_treeview (fileview);
    gtk_widget_show (treeview);
    gtk_container_add (GTK_CONTAINER (swin), treeview);
    fileview->priv->treeview = GTK_TREE_VIEW (treeview);
    g_signal_connect (treeview, "key-press-event",
                      G_CALLBACK (moo_file_view_key_press),
                      fileview);

    swin = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (swin);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
                                    GTK_POLICY_ALWAYS,
                                    GTK_POLICY_AUTOMATIC);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), swin, NULL);
    iconview = create_iconview (fileview);
    gtk_widget_show (iconview);
    gtk_container_add (GTK_CONTAINER (swin), iconview);
    fileview->priv->iconview = MOO_ICON_VIEW (iconview);

    return notebook;
}


static GtkWidget   *create_filter_combo (G_GNUC_UNUSED MooFileView *fileview)
{
    GtkWidget *hbox, *button, *combo;

    hbox = gtk_hbox_new (FALSE, 0);

    button = gtk_toggle_button_new_with_label ("Filter");
    gtk_widget_show (button);
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

    combo = gtk_combo_box_entry_new ();
    gtk_widget_show (combo);
    gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 0);

    fileview->priv->filter_button = GTK_TOGGLE_BUTTON (button);
    fileview->priv->filter_combo = GTK_COMBO_BOX (combo);
    fileview->priv->filter_entry = GTK_ENTRY (GTK_BIN(combo)->child);

    g_signal_connect_swapped (button, "toggled",
                              G_CALLBACK (filter_button_toggled),
                              fileview);
    g_signal_connect_data (combo, "changed",
                           G_CALLBACK (filter_combo_changed),
                           fileview, NULL,
                           G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    g_signal_connect_swapped (GTK_BIN(combo)->child, "activate",
                              G_CALLBACK (filter_entry_activate),
                              fileview);

    return hbox;
}


static GtkWidget   *create_treeview     (MooFileView    *fileview)
{
    GtkWidget *treeview;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;

    treeview = gtk_tree_view_new_with_model (fileview->priv->filter_model);

    g_signal_connect (treeview, "row-activated",
                      G_CALLBACK (tree_row_activated), fileview);
    g_signal_connect (treeview, "button-press-event",
                      G_CALLBACK (tree_button_press), fileview);

    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_title (column, "Name");
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
    fileview->priv->tree_name_column = column;

    cell = gtk_cell_renderer_pixbuf_new ();
    gtk_tree_view_column_pack_start (column, cell, FALSE);
    gtk_tree_view_column_set_cell_data_func (column, cell,
                                             (GtkTreeCellDataFunc) icon_data_func,
                                             fileview, NULL);

    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, cell, TRUE);
    gtk_tree_view_column_set_cell_data_func (column, cell,
                                             (GtkTreeCellDataFunc) name_data_func,
                                             fileview, NULL);

#ifdef USE_SIZE_AND_STUFF
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_title (column, "Size");
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, cell, FALSE);
    gtk_tree_view_column_set_cell_data_func (column, cell,
                                             (GtkTreeCellDataFunc) size_data_func,
                                             fileview, NULL);

    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_title (column, "Date");
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, cell, FALSE);
    gtk_tree_view_column_set_cell_data_func (column, cell,
                                             (GtkTreeCellDataFunc) date_data_func,
                                             fileview, NULL);
#endif

    return treeview;
}


static GtkWidget   *create_iconview         (MooFileView    *fileview)
{
    GtkWidget *iconview;
    GtkCellRenderer *cell;

    iconview = moo_icon_view_new_with_model (fileview->priv->filter_model);
    g_signal_connect (iconview, "key-press-event",
                      G_CALLBACK (moo_file_view_key_press),
                      fileview);

    moo_icon_view_set_cell_data_func (MOO_ICON_VIEW (iconview),
                                      MOO_ICON_VIEW_CELL_PIXBUF,
                                      (MooIconCellDataFunc) icon_data_func,
                                      fileview, NULL);
    moo_icon_view_set_cell_data_func (MOO_ICON_VIEW (iconview),
                                      MOO_ICON_VIEW_CELL_TEXT,
                                      (MooIconCellDataFunc) name_data_func,
                                      fileview, NULL);

    cell = moo_icon_view_get_cell (MOO_ICON_VIEW (iconview),
                                   MOO_ICON_VIEW_CELL_TEXT);
    g_object_set (cell, "xalign", 0.0, NULL);
    cell = moo_icon_view_get_cell (MOO_ICON_VIEW (iconview),
                                   MOO_ICON_VIEW_CELL_PIXBUF);
    g_object_set (cell, "xpad", 1, "ypad", 1, NULL);

    g_signal_connect (iconview, "item-activated",
                      G_CALLBACK (icon_item_activated), fileview);
    g_signal_connect (iconview, "button-press-event",
                      G_CALLBACK (icon_button_press), fileview);

    return iconview;
}


static gboolean moo_file_view_check_visible (MooFileView    *fileview,
                                             MooFile        *file,
                                             gboolean        ignore_hidden,
                                             gboolean        ignore_two_dots)
{
    if (!file)
        return FALSE;

    if (!strcmp (moo_file_get_basename (file), ".."))
    {
        if (!ignore_two_dots)
            return fileview->priv->show_two_dots;
        else
            return FALSE;
    }

    if (moo_file_test (file, MOO_FILE_IS_HIDDEN))
    {
        if (!ignore_hidden)
            return fileview->priv->show_hidden_files;
        else
            return TRUE;
    }

    if (moo_file_test (file, MOO_FILE_IS_FOLDER))
        return TRUE;

    if (fileview->priv->current_filter && fileview->priv->use_current_filter)
    {
        GtkFileFilterInfo filter_info;
        GtkFileFilter *filter = fileview->priv->current_filter;

        filter_info.contains = gtk_file_filter_get_needed (filter);
        filter_info.filename = moo_file_get_basename (file);
        filter_info.uri = NULL;
        filter_info.display_name = moo_file_get_display_basename (file);
        filter_info.mime_type = moo_file_get_mime_type (file);

        return gtk_file_filter_filter (fileview->priv->current_filter,
                                       &filter_info);
    }

    return TRUE;
}


static gboolean     filter_visible_func (GtkTreeModel   *model,
                                         GtkTreeIter    *iter,
                                         MooFileView    *fileview)
{
    MooFile *file;
    gboolean visible = TRUE;

    gtk_tree_model_get (model, iter, COLUMN_FILE, &file, -1);

    if (file && fileview->priv->temp_visible &&
        !strcmp (moo_file_get_basename (file), fileview->priv->temp_visible))
    {
#if 1
        GtkTreePath *tmp = NULL, *tmp2 = NULL;
        g_assert (fileview->priv->temp_visible_row != NULL);
        g_assert (gtk_tree_row_reference_valid (fileview->priv->temp_visible_row));
        tmp = gtk_tree_row_reference_get_path (fileview->priv->temp_visible_row);
        tmp2 = gtk_tree_model_get_path (model, iter);
        g_assert (!gtk_tree_path_compare (tmp, tmp2));
        gtk_tree_path_free (tmp);
        gtk_tree_path_free (tmp2);
#endif
        visible = TRUE;
    }
    else
    {
        visible = moo_file_view_check_visible (fileview, file, FALSE, FALSE);
    }

    moo_file_unref (file);
    return visible;
}


static void icon_data_func  (G_GNUC_UNUSED GObject *column_or_iconview,
                             GtkCellRenderer    *cell,
                             GtkTreeModel       *model,
                             GtkTreeIter        *iter,
                             MooFileView        *fileview)
{
    MooFile *file = NULL;
    GdkPixbuf *pixbuf = NULL;

    gtk_tree_model_get (model, iter, COLUMN_FILE, &file, -1);

    if (file)
        pixbuf = moo_file_get_icon (file, GTK_WIDGET (fileview),
                                    fileview->priv->icon_size);

    g_object_set (cell, "pixbuf", pixbuf, NULL);
    moo_file_unref (file);
}


static void name_data_func  (G_GNUC_UNUSED GObject *column_or_iconview,
                             GtkCellRenderer    *cell,
                             GtkTreeModel       *model,
                             GtkTreeIter        *iter,
                             G_GNUC_UNUSED MooFileView *fileview)
{
    MooFile *file = NULL;
    const char *name = NULL;

    gtk_tree_model_get (model, iter, COLUMN_FILE, &file, -1);

    if (file)
        name = moo_file_get_display_basename (file);

    g_object_set (cell, "text", name, NULL);
    moo_file_unref (file);
}


#ifdef USE_SIZE_AND_STUFF
static void date_data_func  (G_GNUC_UNUSED GObject            *column_or_iconview,
                             G_GNUC_UNUSED GtkCellRenderer    *cell,
                             G_GNUC_UNUSED GtkTreeModel       *model,
                             G_GNUC_UNUSED GtkTreeIter        *iter,
                             G_GNUC_UNUSED MooFileView *fileview)
{
}

static void size_data_func  (G_GNUC_UNUSED GObject            *column_or_iconview,
                             G_GNUC_UNUSED GtkCellRenderer    *cell,
                             G_GNUC_UNUSED GtkTreeModel       *model,
                             G_GNUC_UNUSED GtkTreeIter        *iter,
                             G_GNUC_UNUSED MooFileView *fileview)
{
}
#endif


gboolean    moo_file_view_chdir             (MooFileView    *fileview,
                                             const char     *dir,
                                             GError        **error)
{
    gboolean result;

    g_return_val_if_fail (MOO_IS_FILE_VIEW (fileview), FALSE);

    g_signal_emit (fileview, signals[CHDIR], 0, dir, error, &result);

    return result;
}


static void         moo_file_view_go_up     (MooFileView    *fileview)
{
    MooFolder *parent;
    char *name;

    g_return_if_fail (fileview->priv->current_dir != NULL);

    parent = moo_folder_get_parent (fileview->priv->current_dir,
                                    MOO_FILE_HAS_ICON);

#if 0
    g_print ("current dir: '%s'\nparent dir: '%s'\n",
             moo_folder_get_path (fileview->priv->current_dir),
             moo_folder_get_path (parent));
#endif

    if (parent != fileview->priv->current_dir)
    {
        name = g_path_get_basename (
                moo_folder_get_path (fileview->priv->current_dir));
        moo_file_view_set_current_dir (fileview, parent);
        moo_file_view_select_file (fileview, name);
        g_free (name);
    }

    g_object_unref (parent);
}


static void         moo_file_view_go_home   (MooFileView    *fileview)
{
    const char *dir;
    GError *error = NULL;

    dir = g_get_home_dir ();

    if (!moo_file_view_chdir (fileview, dir, &error))
    {
        g_warning ("%s: could not go up", G_STRLOC);

        if (error)
        {
            g_warning ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }
    }
}


static void         tree_path_activated     (MooFileView    *fileview,
                                             GtkTreePath    *filter_path)
{
    MooFile *file = NULL;
    GtkTreeIter iter;

    if (!gtk_tree_model_get_iter (fileview->priv->filter_model, &iter, filter_path))
    {
        g_return_if_reached ();
    }

    gtk_tree_model_get (fileview->priv->filter_model,
                        &iter, COLUMN_FILE, &file, -1);
    g_return_if_fail (file != NULL);

    if (!strcmp (moo_file_get_basename (file), ".."))
    {
        g_signal_emit_by_name (fileview, "go-up");
    }
    else if (moo_file_test (file, MOO_FILE_IS_FOLDER))
    {
        GError *error = NULL;

        if (!moo_file_view_chdir (fileview, moo_file_get_basename (file), &error))
        {
            g_warning ("%s: could not go into '%s'",
                       G_STRLOC, moo_file_get_basename (file));

            if (error)
            {
                g_warning ("%s: %s", G_STRLOC, error->message);
                g_error_free (error);
            }
        }
    }
    else
    {
        char *path;
        g_assert (fileview->priv->current_dir != NULL);
        path = g_build_filename (moo_folder_get_path (fileview->priv->current_dir),
                                 moo_file_get_basename (file), NULL);
        g_signal_emit (fileview, signals[ACTIVATE], 0, path);
        g_free (path);
    }

    moo_file_unref (file);
}


static void         tree_row_activated      (G_GNUC_UNUSED GtkTreeView *treeview,
                                             GtkTreePath    *filter_treepath,
                                             G_GNUC_UNUSED GtkTreeViewColumn *column,
                                             MooFileView    *fileview)
{
    tree_path_activated (fileview, filter_treepath);
}


static void         icon_item_activated     (G_GNUC_UNUSED MooIconView *iconview,
                                             GtkTreePath    *filter_treepath,
                                             MooFileView    *fileview)
{
    tree_path_activated (fileview, filter_treepath);
}


static void         moo_file_view_go        (MooFileView    *fileview,
                                             GtkDirectionType where)
{
    const char *dir;
    GError *error = NULL;

    dir = history_go (fileview, where);

    if (dir)
    {
        if (!moo_file_view_chdir (fileview, dir, &error))
        {
            g_warning ("%s: could not go into '%s'",
                       G_STRLOC, dir);

            if (error)
            {
                g_warning ("%s: %s", G_STRLOC, error->message);
                g_error_free (error);
            }

            history_revert_go (fileview);
        }
    }
}


static void         moo_file_view_go_back   (MooFileView    *fileview)
{
    moo_file_view_go (fileview, GTK_DIR_LEFT);
}


static void         moo_file_view_go_forward(MooFileView    *fileview)
{
    moo_file_view_go (fileview, GTK_DIR_RIGHT);
}


struct _History {
    gboolean            done;
    GtkDirectionType    direction;

    GList              *list;
    GList              *current;
};


static void         history_init            (MooFileView    *fileview)
{
    fileview->priv->history = g_new0 (History, 1);
}


static void         history_free            (MooFileView    *fileview)
{
    g_list_foreach (fileview->priv->history->list, (GFunc) g_free, NULL);
    g_list_free (fileview->priv->history->list);
    g_free (fileview->priv->history);
    fileview->priv->history = NULL;
}


static void         history_clear           (MooFileView    *fileview)
{
    g_list_foreach (fileview->priv->history->list, (GFunc) g_free, NULL);
    g_list_free (fileview->priv->history->list);
    fileview->priv->history->current = NULL;
    fileview->priv->history->list = NULL;
}


static const char  *history_go              (MooFileView    *fileview,
                                             GtkDirectionType where)
{
    History *hist = fileview->priv->history;
    const char *dir;

    g_assert (where == GTK_DIR_LEFT || where == GTK_DIR_RIGHT);

    if (!hist->current)
        return NULL;

    if (where == GTK_DIR_RIGHT)
    {
        if (!hist->current->next)
            return NULL;

        dir = hist->current->next->data;
        hist->current = hist->current->next;
    }
    else
    {
        if (!hist->current->prev)
            return NULL;
        dir = hist->current->prev->data;
        hist->current = hist->current->prev;
    }

    hist->done = TRUE;
    hist->direction = where;
    return dir;
}


static void         history_revert_go       (MooFileView    *fileview)
{
    History *hist = fileview->priv->history;

    g_assert (hist->done);

    if (hist->direction == GTK_DIR_LEFT)
    {
        g_assert (hist->current && hist->current->next);
        hist->current = hist->current->next;
    }
    else
    {
        g_assert (hist->current && hist->current->prev);
        hist->current = hist->current->prev;
    }

    hist->done = FALSE;
}


static void         history_goto            (MooFileView    *fileview,
                                             const char     *dirname)
{
    History *hist = fileview->priv->history;

    g_return_if_fail (dirname != NULL);

    if (hist->done)
    {
        hist->done = FALSE;
        return;
    }

    if (hist->current && hist->current->next)
    {
        GList *l;
        for (l = hist->current->next; l != NULL; l = l->next)
            g_free (l->data);
        l = hist->current->next;
        hist->current->next = NULL;
        g_list_free (l);
    }

    if (!hist->current || strcmp (dirname, (char*) hist->current))
    {
        hist->list = g_list_append (hist->list, g_strdup (dirname));
        hist->current = g_list_last (hist->list);
    }
}


static void         moo_file_view_set_property  (GObject        *object,
                                                 guint           prop_id,
                                                 const GValue   *value,
                                                 GParamSpec     *pspec)
{
    MooFileView *fileview = MOO_FILE_VIEW (object);
    GError *error = NULL;
    const char *dir;

    switch (prop_id)
    {
        case PROP_CURRENT_DIRECTORY:
            dir = g_value_get_string (value);

            if (dir)
            {
                if (!moo_file_view_chdir (fileview, dir, &error))
                {
                    g_warning ("%s: could not chdir to '%s'",
                               G_STRLOC, dir);

                    if (error)
                    {
                        g_warning ("%s: %s", G_STRLOC, error->message);
                        g_error_free (error);
                    }
                }
            }
            else
            {
                moo_file_view_chdir (fileview, NULL, NULL);
            }

            break;

        case PROP_FILE_MGR:
            moo_file_view_set_file_mgr (fileview,
                                        g_value_get_object (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void         moo_file_view_get_property  (GObject        *object,
                                                 guint           prop_id,
                                                 GValue         *value,
                                                 GParamSpec     *pspec)
{
    MooFileView *fileview = MOO_FILE_VIEW (object);

    switch (prop_id)
    {
        case PROP_CURRENT_DIRECTORY:
            if (fileview->priv->current_dir)
                g_value_set_string (value, moo_folder_get_path (fileview->priv->current_dir));
            else
                g_value_set_string (value, NULL);
            break;

        case PROP_FILE_MGR:
            g_value_set_object (value, fileview->priv->mgr);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void         moo_file_view_set_file_mgr  (MooFileView    *fileview,
                                                 MooEditFileMgr *mgr)
{
    if (!mgr)
    {
        mgr = moo_edit_file_mgr_new ();
        moo_file_view_set_file_mgr (fileview, mgr);
        g_object_unref (mgr);
        return;
    }

    if (mgr == fileview->priv->mgr)
        return;

    if (fileview->priv->mgr)
        g_object_unref (fileview->priv->mgr);
    fileview->priv->mgr = mgr;
    g_object_ref (fileview->priv->mgr);

    init_filter_combo (fileview);
}


static void         block_filter_signals    (MooFileView    *fileview)
{
    g_signal_handlers_block_by_func (fileview->priv->filter_combo,
                                     (gpointer) filter_button_toggled,
                                     fileview);
    g_signal_handlers_block_by_func (fileview->priv->filter_combo,
                                     (gpointer) filter_combo_changed,
                                     fileview);
    g_signal_handlers_block_by_func (fileview->priv->filter_combo,
                                     (gpointer) filter_entry_activate,
                                     fileview);
}

static void         unblock_filter_signals  (MooFileView    *fileview)
{
    g_signal_handlers_unblock_by_func (fileview->priv->filter_combo,
                                       (gpointer) filter_button_toggled,
                                       fileview);
    g_signal_handlers_unblock_by_func (fileview->priv->filter_combo,
                                       (gpointer) filter_combo_changed,
                                       fileview);
    g_signal_handlers_unblock_by_func (fileview->priv->filter_combo,
                                       (gpointer) filter_entry_activate,
                                       fileview);
}


static void         init_filter_combo       (MooFileView    *fileview)
{
    MooEditFileMgr *mgr = fileview->priv->mgr;
    GtkFileFilter *filter;

    block_filter_signals (fileview);

    moo_edit_file_mgr_init_filter_combo (mgr, fileview->priv->filter_combo);
    if (fileview->priv->current_filter)
        g_object_unref (fileview->priv->current_filter);
    fileview->priv->current_filter = NULL;
    fileview->priv->use_current_filter = FALSE;
    gtk_toggle_button_set_active (fileview->priv->filter_button, FALSE);
    gtk_entry_set_text (fileview->priv->filter_entry, "");

    unblock_filter_signals (fileview);

    filter = moo_edit_file_mgr_get_last_filter (mgr);
    if (filter) fileview_set_filter (fileview, filter);
}


static void         filter_button_toggled   (MooFileView    *fileview)
{
    gboolean active =
            gtk_toggle_button_get_active (fileview->priv->filter_button);

    if (active == fileview->priv->use_current_filter)
        return;

    fileview_set_use_filter (fileview, active, TRUE);
    focus_to_file_view (fileview);
}


static void         filter_combo_changed    (MooFileView    *fileview)
{
    GtkTreeIter iter;
    GtkFileFilter *filter;
    MooEditFileMgr *mgr = fileview->priv->mgr;
    GtkComboBox *combo = fileview->priv->filter_combo;

    if (!gtk_combo_box_get_active_iter (combo, &iter))
        return;

    filter = moo_edit_file_mgr_get_filter (mgr, &iter);
    g_return_if_fail (filter != NULL);

    fileview_set_filter (fileview, filter);
    focus_to_file_view (fileview);
}


static void         filter_entry_activate   (MooFileView    *fileview)
{
    const char *text;
    GtkFileFilter *filter;
    MooEditFileMgr *mgr = fileview->priv->mgr;

    text = gtk_entry_get_text (fileview->priv->filter_entry);

    if (text && text[0])
        filter = moo_edit_file_mgr_new_user_filter (mgr, text);
    else
        filter = NULL;

    fileview_set_filter (fileview, filter);
    focus_to_file_view (fileview);
}


static void         fileview_set_filter     (MooFileView    *fileview,
                                             GtkFileFilter  *filter)
{
    GtkFileFilter *null_filter;

    if (filter && filter == fileview->priv->current_filter)
    {
        fileview_set_use_filter (fileview, TRUE, TRUE);
        return;
    }

    null_filter = moo_edit_file_mgr_get_null_filter (fileview->priv->mgr);
    if (filter == null_filter)
        return fileview_set_filter (fileview, NULL);

    if (filter && (gtk_file_filter_get_needed (filter) & GTK_FILE_FILTER_URI))
    {
        g_warning ("%s: The filter set wants uri, but i do not know "
                   "how to work with uri's. Ignoring", G_STRLOC);
        return;
    }

    block_filter_signals (fileview);

    if (fileview->priv->current_filter)
        g_object_unref (fileview->priv->current_filter);
    fileview->priv->current_filter = filter;

    if (filter)
    {
        const char *name;
        gtk_object_sink (GTK_OBJECT (g_object_ref (filter)));
        name = gtk_file_filter_get_name (filter);
        gtk_entry_set_text (fileview->priv->filter_entry, name);
        fileview_set_use_filter (fileview, TRUE, FALSE);
    }
    else
    {
        gtk_entry_set_text (fileview->priv->filter_entry, "");
        fileview_set_use_filter (fileview, FALSE, FALSE);
    }

    gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER
            (fileview->priv->filter_model));

    unblock_filter_signals (fileview);
}


static void         fileview_set_use_filter (MooFileView    *fileview,
                                             gboolean        use,
                                             gboolean        block_signals)
{
    if (block_signals)
        block_filter_signals (fileview);

    gtk_toggle_button_set_active (fileview->priv->filter_button, use);

    if (fileview->priv->use_current_filter != use)
    {
        if (fileview->priv->current_filter)
        {
            fileview->priv->use_current_filter = use;
            if (block_signals)
                gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER
                        (fileview->priv->filter_model));
        }
        else
        {
            fileview->priv->use_current_filter = FALSE;
            gtk_toggle_button_set_active (fileview->priv->filter_button, FALSE);
        }
    }

    if (block_signals)
        unblock_filter_signals (fileview);
}


static void create_goto_item (MooFileView   *fileview,
                              GtkWidget     *menu,
                              const char    *label,
                              const char    *stock_id,
                              const char    *signal)
{
    GtkWidget *item, *icon;

    icon = gtk_image_new_from_stock (stock_id,
                                     GTK_ICON_SIZE_MENU);
    gtk_widget_show (GTK_WIDGET (icon));
    item = gtk_image_menu_item_new_with_label (label);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), icon);
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    g_object_set_data (G_OBJECT (item), "moo-file-view-signal",
                       (gpointer) signal);
    g_signal_connect (item, "activate",
                      G_CALLBACK (goto_item_activated),
                      fileview);
}

static void add_separator_item (GtkWidget     *menu)
{
    GtkWidget *item = gtk_separator_menu_item_new ();
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
}


static void         view_type_item_toggled  (GtkWidget      *widget,
                                             MooFileView    *fileview)
{
    gboolean active = FALSE;

    g_object_get (widget, "active", &active, NULL);

    if (active)
    {
        MooFileViewType view_type = GPOINTER_TO_INT (
                g_object_get_data (G_OBJECT (widget), "moo-file-view-type"));
        moo_file_view_set_view_type (fileview, view_type);
    }
}


static void create_view_type_items (MooFileView   *fileview,
                                    GtkWidget     *menu)
{
    GtkWidget *icon_item, *list_item;
    GSList *group = NULL;

    icon_item = gtk_radio_menu_item_new_with_label (group, "Short View");
    gtk_widget_show (icon_item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), icon_item);
    g_object_set_data (G_OBJECT (icon_item), "moo-file-view-type",
                       GINT_TO_POINTER (MOO_FILE_VIEW_ICON));

    group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (icon_item));
    list_item = gtk_radio_menu_item_new_with_label (group, "Detailed View");
    gtk_widget_show (list_item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), list_item);
    g_object_set_data (G_OBJECT (list_item), "moo-file-view-type",
                       GINT_TO_POINTER (MOO_FILE_VIEW_LIST));

    if (fileview->priv->view_type == MOO_FILE_VIEW_ICON)
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (icon_item), TRUE);
    else
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (list_item), TRUE);

    g_signal_connect (icon_item, "toggled",
                      G_CALLBACK (view_type_item_toggled), fileview);
    g_signal_connect (list_item, "toggled",
                      G_CALLBACK (view_type_item_toggled), fileview);
}


static void create_view_submenu (MooFileView   *fileview,
                                 GtkWidget     *menu)
{
    GtkWidget *view_item, *item, *submenu;

    view_item = gtk_menu_item_new_with_label ("View");
    gtk_widget_show (view_item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), view_item);

    submenu = gtk_menu_new ();
    gtk_widget_show (submenu);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (view_item), submenu);

    item = gtk_check_menu_item_new_with_label ("Show Hidden Files");
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item),
                                    fileview->priv->show_hidden_files);
    g_signal_connect (item, "toggled",
                      G_CALLBACK (show_hidden_toggled), fileview);

    item = gtk_check_menu_item_new_with_label ("Show Parent Folder");
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item),
                                    fileview->priv->show_two_dots);
    g_signal_connect (item, "toggled",
                      G_CALLBACK (show_two_dots_toggled), fileview);

    add_separator_item (submenu);
    create_view_type_items (fileview, submenu);
}

static GtkWidget   *create_popup_menu       (MooFileView    *fileview)
{
    GtkWidget *menu;

    menu = gtk_menu_new ();

    create_goto_item (fileview, menu, "Parent Folder",
                      GTK_STOCK_GO_UP, "go-up");
    create_goto_item (fileview, menu, "Go Back",
                      GTK_STOCK_GO_BACK, "go-back");
    create_goto_item (fileview, menu, "Go Forward",
                      GTK_STOCK_GO_FORWARD, "go-forward");
    create_goto_item (fileview, menu, "Home Folder",
                      GTK_STOCK_HOME, "go-home");

    add_separator_item (menu);

    create_view_submenu (fileview, menu);

    return menu;
}


static gboolean really_destroy_menu (GtkWidget *menu)
{
    gtk_widget_destroy (menu);
    return FALSE;
}

static void destroy_menu    (GtkWidget *menu)
{
    g_idle_add ((GSourceFunc) really_destroy_menu, menu);
}

static void         do_popup                (MooFileView    *fileview,
                                             GdkEventButton *event,
                                             GtkTreePath    *filter_path)
{
    GtkWidget *menu;
    MooFile *file = NULL;
    char *path = NULL;

    if (filter_path)
    {
        GtkTreeIter iter;
        g_return_if_fail (gtk_tree_model_get_iter (fileview->priv->filter_model,
                          &iter, filter_path));

        gtk_tree_model_get (fileview->priv->filter_model, &iter,
                            COLUMN_FILE, &file, -1);
        g_return_if_fail (file != NULL);
    }

    menu = create_popup_menu (fileview);
    g_signal_connect (menu, "deactivate", G_CALLBACK (destroy_menu), NULL);

    if (file != NULL)
    {
        g_assert (fileview->priv->current_dir != NULL);
        path = g_build_filename (moo_folder_get_path (fileview->priv->current_dir),
                                 moo_file_get_basename (file), NULL);
    }
    g_signal_emit (fileview, signals[POPULATE_POPUP], 0, path, menu);

    gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
                    event->button, event->time);

    g_free (path);
    moo_file_unref (file);
}


static gboolean     tree_button_press       (GtkTreeView    *treeview,
                                             GdkEventButton *event,
                                             MooFileView    *fileview)
{
    GtkTreeSelection *selection;
    GtkTreePath *filter_path = NULL;

    if (event->button != 3)
        return FALSE;

    selection = gtk_tree_view_get_selection (treeview);

    if (gtk_tree_view_get_path_at_pos (treeview, event->x, event->y,
                                       &filter_path, NULL, NULL, NULL))
    {
        gtk_tree_selection_unselect_all (selection);
        gtk_tree_selection_select_path (selection, filter_path);
    }

    do_popup (fileview, event, filter_path);
    gtk_tree_path_free (filter_path);

    return TRUE;
}


static gboolean     icon_button_press       (MooIconView    *iconview,
                                             GdkEventButton *event,
                                             MooFileView    *fileview)
{
    GtkTreePath *filter_path = NULL;

    if (event->button != 3)
        return FALSE;

    filter_path = moo_icon_view_get_path (iconview, event->x, event->y);

    if (filter_path)
        moo_icon_view_select_path (iconview, filter_path);

    do_popup (fileview, event, filter_path);
    gtk_tree_path_free (filter_path);

    return TRUE;
}


static void         show_hidden_toggled     (GtkWidget      *widget,
                                             MooFileView    *fileview)
{
    gboolean active = fileview->priv->show_hidden_files;
    g_object_get (G_OBJECT (widget), "active", &active, NULL);
    moo_file_view_set_show_hidden (fileview, active);
}


static void         show_two_dots_toggled   (GtkWidget      *widget,
                                             MooFileView    *fileview)
{
    gboolean active = fileview->priv->show_two_dots;
    g_object_get (G_OBJECT (widget), "active", &active, NULL);
    moo_file_view_set_show_parent (fileview, active);
}


void        moo_file_view_set_show_hidden   (MooFileView    *fileview,
                                             gboolean        show)
{
    g_return_if_fail (MOO_IS_FILE_VIEW (fileview));

    if (fileview->priv->show_hidden_files != show)
    {
        fileview->priv->show_hidden_files = show;
        gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (
                fileview->priv->filter_model));
    }
}


void        moo_file_view_set_show_parent   (MooFileView    *fileview,
                                             gboolean        show)
{
    g_return_if_fail (MOO_IS_FILE_VIEW (fileview));

    if (fileview->priv->show_two_dots != show)
    {
        fileview->priv->show_two_dots = show;
        gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (
                fileview->priv->filter_model));
    }
}


static void         toggle_show_hidden      (MooFileView    *fileview)
{
    moo_file_view_set_show_hidden (fileview,
                                   !fileview->priv->show_hidden_files);
}


static GtkWidget   *get_file_view_widget        (MooFileView    *fileview)
{
    if (fileview->priv->view_type == MOO_FILE_VIEW_ICON)
        return GTK_WIDGET(fileview->priv->iconview);
    else
        return GTK_WIDGET(fileview->priv->treeview);
}


/****************************************************************************/
/* Auxiliary stuff
 */

static void         file_view_move_selection(MooFileView    *fileview,
                                             GtkTreeIter    *filter_iter)
{
    GtkTreePath *path = NULL;

    if (filter_iter)
    {
        path = gtk_tree_model_get_path (fileview->priv->filter_model,
                                        filter_iter);
        g_return_if_fail (path != NULL);
    }

    if (fileview->priv->view_type == MOO_FILE_VIEW_LIST)
    {
        GtkTreeSelection *selection;
        GtkTreeView *treeview;

        treeview = fileview->priv->treeview;
        selection = gtk_tree_view_get_selection (treeview);

        gtk_tree_selection_unselect_all (selection);

        if (path)
        {
            gtk_tree_view_set_cursor (treeview, path, NULL, FALSE);
            gtk_tree_view_scroll_to_cell (treeview, path, NULL,
                                          FALSE, 0, 0);
        }
    }
    else
    {
        moo_icon_view_move_cursor (fileview->priv->iconview, path);
    }

    gtk_tree_path_free (path);
}


/* returns path in the fileview->priv->filter_model */
static GtkTreePath *file_widget_get_selected    (MooFileView    *fileview)
{
    if (fileview->priv->view_type == MOO_FILE_VIEW_LIST)
    {
        GtkTreeSelection *selection;
        GtkTreeView *treeview;
        GtkTreeIter filter_iter;
        GtkTreeModel *filter_model;

        treeview = fileview->priv->treeview;
        selection = gtk_tree_view_get_selection (treeview);

        if (!gtk_tree_selection_get_selected (selection, &filter_model, &filter_iter))
            return NULL;

        return gtk_tree_model_get_path (fileview->priv->filter_model, &filter_iter);
    }
    else
    {
        return moo_icon_view_get_selected (fileview->priv->iconview);
    }
}


static gboolean find_match_visible  (MooFileView    *fileview,
                                     const char     *text,
                                     GtkTreeIter    *filter_iter,
                                     gboolean        exact_match)
{
    GtkTreeModel *model = fileview->priv->filter_model;
    guint len;

    g_return_val_if_fail (text && text[0], FALSE);

    if (!gtk_tree_model_get_iter_first (model, filter_iter))
        return FALSE;

    len = strlen (text);

    while (TRUE)
    {
        MooFile *file = NULL;
        gboolean match;

        gtk_tree_model_get (model, filter_iter,
                            COLUMN_FILE, &file, -1);

        if (file)
        {
            if (exact_match)
                match = !strcmp (text,
                                 moo_file_get_display_basename (file));
            else
                match = !strncmp (text,
                                  moo_file_get_display_basename (file),
                                  len);

            moo_file_unref (file);

            if (match)
                return TRUE;
        }

        if (!gtk_tree_model_iter_next (model, filter_iter))
            return FALSE;
    }
}


/****************************************************************************/
/* Search entry
 */

static gboolean try_completion              (MooFileView    *fileview,
                                             const char     *text);
static void     completion_clear            (MooFileView    *fileview);


static gboolean search_entry_key_press      (GtkWidget      *entry,
                                             GdkEventKey    *event,
                                             MooFileView    *fileview);
static gboolean search_entry_focus_out      (GtkWidget      *entry,
                                             GdkEventFocus  *event,
                                             MooFileView    *fileview);
static void     search_entry_activate       (GtkEntry       *entry,
                                             MooFileView    *fileview);
static void     search_entry_changed        (GtkEntry       *entry,
                                             MooFileView    *fileview);

static void         start_interactive_search    (MooFileView    *fileview,
                                                 GdkEventKey    *event)
{
    GdkEvent *copy;
    GtkWidget *entry = GTK_WIDGET (fileview->priv->search_entry);

    g_return_if_fail (event != NULL);

    copy = gdk_event_copy ((GdkEvent*) event);
    gtk_widget_realize (entry);
    g_object_unref (copy->key.window);
    copy->key.window = g_object_ref (entry->window);

    file_view_move_selection (fileview, NULL);
    gtk_widget_show (entry);
    gtk_widget_grab_focus (entry);

    fileview->priv->in_search = TRUE;

    g_signal_connect (entry, "key-press-event",
                      G_CALLBACK (search_entry_key_press),
                      fileview);
    g_signal_connect_after (entry, "focus-out-event",
                            G_CALLBACK (search_entry_focus_out),
                            fileview);
    g_signal_connect (entry, "activate",
                      G_CALLBACK (search_entry_activate),
                      fileview);
    g_signal_connect (entry, "changed",
                      G_CALLBACK (search_entry_changed),
                      fileview);

    gtk_widget_event (entry, copy);
    gdk_event_free (copy);
}


static void         stop_interactive_search     (MooFileView    *fileview,
                                                 gboolean        move_focus_to_file_view)
{
    GtkWidget *entry = GTK_WIDGET (fileview->priv->search_entry);

    fileview->priv->in_search = FALSE;

    g_signal_handlers_disconnect_by_func (entry,
                                          (gpointer) search_entry_key_press,
                                          fileview);
    g_signal_handlers_disconnect_by_func (entry,
                                          (gpointer) search_entry_focus_out,
                                          fileview);
    g_signal_handlers_disconnect_by_func (entry,
                                          (gpointer) search_entry_activate,
                                          fileview);
    g_signal_handlers_disconnect_by_func (entry,
                                          (gpointer) search_entry_changed,
                                          fileview);

    gtk_widget_hide (entry);
    gtk_entry_set_text (GTK_ENTRY (entry), "");

#ifdef DEBUG
    /* TODO: may it become invalid? */
    if (fileview->priv->temp_visible_row &&
        gtk_tree_row_reference_valid (fileview->priv->temp_visible_row))
    {
        GtkTreePath *temp_path =
                gtk_tree_row_reference_get_path (fileview->priv->temp_visible_row);
        g_assert (temp_path != NULL);
    }
#endif

    if (fileview->priv->temp_visible_row)
    {
        g_assert (gtk_tree_row_reference_valid (fileview->priv->temp_visible_row));

        gtk_tree_row_reference_free (fileview->priv->temp_visible_row);
        fileview->priv->temp_visible_row = NULL;
        g_free (fileview->priv->temp_visible);
        fileview->priv->temp_visible = NULL;

        gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER
                (fileview->priv->filter_model));
    }
    else
    {
        g_assert (fileview->priv->temp_visible == NULL);
    }

    completion_clear (fileview);

    if (move_focus_to_file_view)
        focus_to_file_view (fileview);
}


#define PRINT_KEY_EVENT(event)                                      \
    g_print ("%s%s%s%s\n",                                          \
             event->state & GDK_SHIFT_MASK ? "<Shift>" : "",        \
             event->state & GDK_CONTROL_MASK ? "<Control>" : "",    \
             event->state & GDK_MOD1_MASK ? "<Alt>" : "",           \
             gdk_keyval_name (event->keyval))


static gboolean     moo_file_view_key_press     (GtkWidget      *widget,
                                                 GdkEventKey    *event,
                                                 MooFileView    *fileview)
{
    if (fileview->priv->in_search)
    {
        g_warning ("%s: something wrong", G_STRLOC);
        stop_interactive_search (fileview, FALSE);
        return FALSE;
    }

    /* return immediately if event doesn't seem like text typed in */

    if (event->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK))
        return FALSE;

    switch (event->keyval)
    {
        case GDK_VoidSymbol:
        case GDK_BackSpace:
        case GDK_Tab:
        case GDK_Linefeed:
        case GDK_Clear:
        case GDK_Return:
        case GDK_Pause:
        case GDK_Scroll_Lock:
        case GDK_Sys_Req:
        case GDK_Escape:
        case GDK_Delete:
        case GDK_Multi_key:
        case GDK_Codeinput:
        case GDK_SingleCandidate:
        case GDK_MultipleCandidate:
        case GDK_PreviousCandidate:
        case GDK_Kanji:
        case GDK_Muhenkan:
        case GDK_Henkan_Mode:
        case GDK_Romaji:
        case GDK_Hiragana:
        case GDK_Katakana:
        case GDK_Hiragana_Katakana:
        case GDK_Zenkaku:
        case GDK_Hankaku:
        case GDK_Zenkaku_Hankaku:
        case GDK_Touroku:
        case GDK_Massyo:
        case GDK_Kana_Lock:
        case GDK_Kana_Shift:
        case GDK_Eisu_Shift:
        case GDK_Eisu_toggle:
        case GDK_Home:
        case GDK_Left:
        case GDK_Up:
        case GDK_Right:
        case GDK_Down:
        case GDK_Page_Up:
        case GDK_Page_Down:
        case GDK_End:
        case GDK_Begin:
        case GDK_Select:
        case GDK_Print:
        case GDK_Execute:
        case GDK_Insert:
        case GDK_Undo:
        case GDK_Redo:
        case GDK_Menu:
        case GDK_Find:
        case GDK_Cancel:
        case GDK_Help:
        case GDK_Break:
        case GDK_Mode_switch:
        case GDK_Num_Lock:
        case GDK_KP_Tab:
        case GDK_KP_Enter:
        case GDK_KP_F1:
        case GDK_KP_F2:
        case GDK_KP_F3:
        case GDK_KP_F4:
        case GDK_KP_Home:
        case GDK_KP_Left:
        case GDK_KP_Up:
        case GDK_KP_Right:
        case GDK_KP_Down:
        case GDK_KP_Page_Up:
        case GDK_KP_Page_Down:
        case GDK_KP_End:
        case GDK_KP_Begin:
        case GDK_KP_Insert:
        case GDK_KP_Delete:
        case GDK_F1:
        case GDK_F2:
        case GDK_F3:
        case GDK_F4:
        case GDK_F5:
        case GDK_F6:
        case GDK_F7:
        case GDK_F8:
        case GDK_F9:
        case GDK_F10:
        case GDK_F11:
        case GDK_F12:
        case GDK_F13:
        case GDK_F14:
        case GDK_F15:
        case GDK_F16:
        case GDK_F17:
        case GDK_F18:
        case GDK_F19:
        case GDK_F20:
        case GDK_F21:
        case GDK_F22:
        case GDK_F23:
        case GDK_F24:
        case GDK_F25:
        case GDK_F26:
        case GDK_F27:
        case GDK_F28:
        case GDK_F29:
        case GDK_F30:
        case GDK_F31:
        case GDK_F32:
        case GDK_F33:
        case GDK_F34:
        case GDK_F35:
        case GDK_Shift_L:
        case GDK_Shift_R:
        case GDK_Control_L:
        case GDK_Control_R:
        case GDK_Caps_Lock:
        case GDK_Shift_Lock:
        case GDK_Meta_L:
        case GDK_Meta_R:
        case GDK_Alt_L:
        case GDK_Alt_R:
        case GDK_Super_L:
        case GDK_Super_R:
        case GDK_Hyper_L:
        case GDK_Hyper_R:
        case GDK_ISO_Lock:
        case GDK_ISO_Level2_Latch:
        case GDK_ISO_Level3_Shift:
        case GDK_ISO_Level3_Latch:
        case GDK_ISO_Level3_Lock:
        case GDK_ISO_Group_Latch:
        case GDK_ISO_Group_Lock:
        case GDK_ISO_Next_Group:
        case GDK_ISO_Next_Group_Lock:
        case GDK_ISO_Prev_Group:
        case GDK_ISO_Prev_Group_Lock:
        case GDK_ISO_First_Group:
        case GDK_ISO_First_Group_Lock:
        case GDK_ISO_Last_Group:
        case GDK_ISO_Last_Group_Lock:
        case GDK_ISO_Left_Tab:
        case GDK_ISO_Move_Line_Up:
        case GDK_ISO_Move_Line_Down:
        case GDK_ISO_Partial_Line_Up:
        case GDK_ISO_Partial_Line_Down:
        case GDK_ISO_Partial_Space_Left:
        case GDK_ISO_Partial_Space_Right:
        case GDK_ISO_Set_Margin_Left:
        case GDK_ISO_Set_Margin_Right:
        case GDK_ISO_Release_Margin_Left:
        case GDK_ISO_Release_Margin_Right:
        case GDK_ISO_Release_Both_Margins:
        case GDK_ISO_Fast_Cursor_Left:
        case GDK_ISO_Fast_Cursor_Right:
        case GDK_ISO_Fast_Cursor_Up:
        case GDK_ISO_Fast_Cursor_Down:
        case GDK_ISO_Continuous_Underline:
        case GDK_ISO_Discontinuous_Underline:
        case GDK_ISO_Emphasize:
        case GDK_ISO_Center_Object:
        case GDK_ISO_Enter:
        case GDK_First_Virtual_Screen:
        case GDK_Prev_Virtual_Screen:
        case GDK_Next_Virtual_Screen:
        case GDK_Last_Virtual_Screen:
        case GDK_Terminate_Server:
        case GDK_AccessX_Enable:
        case GDK_AccessX_Feedback_Enable:
        case GDK_RepeatKeys_Enable:
        case GDK_SlowKeys_Enable:
        case GDK_BounceKeys_Enable:
        case GDK_StickyKeys_Enable:
        case GDK_MouseKeys_Enable:
        case GDK_MouseKeys_Accel_Enable:
        case GDK_Overlay1_Enable:
        case GDK_Overlay2_Enable:
        case GDK_AudibleBell_Enable:
        case GDK_Pointer_Left:
        case GDK_Pointer_Right:
        case GDK_Pointer_Up:
        case GDK_Pointer_Down:
        case GDK_Pointer_UpLeft:
        case GDK_Pointer_UpRight:
        case GDK_Pointer_DownLeft:
        case GDK_Pointer_DownRight:
        case GDK_Pointer_Button_Dflt:
        case GDK_Pointer_Button1:
        case GDK_Pointer_Button2:
        case GDK_Pointer_Button3:
        case GDK_Pointer_Button4:
        case GDK_Pointer_Button5:
        case GDK_Pointer_DblClick_Dflt:
        case GDK_Pointer_DblClick1:
        case GDK_Pointer_DblClick2:
        case GDK_Pointer_DblClick3:
        case GDK_Pointer_DblClick4:
        case GDK_Pointer_DblClick5:
        case GDK_Pointer_Drag_Dflt:
        case GDK_Pointer_Drag1:
        case GDK_Pointer_Drag2:
        case GDK_Pointer_Drag3:
        case GDK_Pointer_Drag4:
        case GDK_Pointer_Drag5:
        case GDK_Pointer_EnableKeys:
        case GDK_Pointer_Accelerate:
        case GDK_Pointer_DfltBtnNext:
        case GDK_Pointer_DfltBtnPrev:
        case GDK_3270_Duplicate:
        case GDK_3270_FieldMark:
        case GDK_3270_Right2:
        case GDK_3270_Left2:
        case GDK_3270_BackTab:
        case GDK_3270_EraseEOF:
        case GDK_3270_EraseInput:
        case GDK_3270_Reset:
        case GDK_3270_Quit:
        case GDK_3270_PA1:
        case GDK_3270_PA2:
        case GDK_3270_PA3:
        case GDK_3270_Test:
        case GDK_3270_Attn:
        case GDK_3270_CursorBlink:
        case GDK_3270_AltCursor:
        case GDK_3270_KeyClick:
        case GDK_3270_Jump:
        case GDK_3270_Ident:
        case GDK_3270_Rule:
        case GDK_3270_Copy:
        case GDK_3270_Play:
        case GDK_3270_Setup:
        case GDK_3270_Record:
        case GDK_3270_ChangeScreen:
        case GDK_3270_DeleteWord:
        case GDK_3270_ExSelect:
        case GDK_3270_CursorSelect:
        case GDK_3270_PrintScreen:
        case GDK_3270_Enter:
            return FALSE;
    }

    PRINT_KEY_EVENT (event);

    if (GTK_WIDGET_CLASS(G_OBJECT_GET_CLASS (widget))->key_press_event (widget, event))
        return TRUE;

    if (GTK_WIDGET_CLASS(G_OBJECT_GET_CLASS (fileview))->key_press_event (widget, event))
        return TRUE;

    if (event->string && event->length)
    {
        start_interactive_search (fileview, event);
        return TRUE;
    }

    return FALSE;
}


static gboolean search_entry_key_press      (G_GNUC_UNUSED GtkWidget *entry,
                                             GdkEventKey    *event,
                                             MooFileView    *fileview)
{
    GtkWidget *filewidget;
    switch (event->keyval)
    {
        case GDK_Escape:
            stop_interactive_search (fileview, TRUE);
            return TRUE;

        case GDK_Up:
        case GDK_KP_Up:
        case GDK_Down:
        case GDK_KP_Down:
            stop_interactive_search (fileview, TRUE);
            filewidget = get_file_view_widget (fileview);
            gtk_widget_grab_focus (filewidget);
            GTK_WIDGET_CLASS(G_OBJECT_GET_CLASS (filewidget))->
                    key_press_event (filewidget, event);
            return TRUE;

        default:
            return FALSE;
    }
}


static gboolean search_entry_focus_out      (GtkWidget *entry,
                                             G_GNUC_UNUSED GdkEventFocus *event,
                                             MooFileView    *fileview)
{
    /* focus may be lost due to switching to other window, focus file list then */
    stop_interactive_search (fileview, gtk_widget_is_focus (entry));
    return FALSE;
}


static void     process_entry_text          (MooFileView    *fileview,
                                             GtkEntry       *entry,
                                             const char     *text);
static void     search_entry_changed        (GtkEntry       *entry,
                                             MooFileView    *fileview)
{
    const char *text = gtk_entry_get_text (entry);
    g_return_if_fail (text != NULL);
    process_entry_text (fileview, entry, text);
}


static void     process_entry_text          (MooFileView    *fileview,
                                             GtkEntry       *entry,
                                             const char     *text)
{
    GtkTreeIter iter, filter_iter;
    gboolean need_to_refilter = FALSE;
    gboolean need_to_clear_completion = TRUE;

    g_return_if_fail (text != NULL);

    if (!strncmp (text, "./", 2))
        return process_entry_text (fileview, entry, text + 2);

    /* Check if some file was shown temporarily, and hide it */
    if (fileview->priv->temp_visible)
    {
        /* if still the same filename is entered, don't do anything */
        if (!strcmp (text, fileview->priv->temp_visible))
            goto out;

#ifdef DEBUG
        if (fileview->priv->temp_visible_row &&
            gtk_tree_row_reference_valid (fileview->priv->temp_visible_row))
        {
            GtkTreePath *temp_path =
                    gtk_tree_row_reference_get_path (fileview->priv->temp_visible_row);
            g_assert (temp_path != NULL);
        }
#endif

        gtk_tree_row_reference_free (fileview->priv->temp_visible_row);
        fileview->priv->temp_visible_row = NULL;
        g_free (fileview->priv->temp_visible);
        fileview->priv->temp_visible = NULL;
        need_to_refilter = TRUE;
    }

    if (!text[0])
    {
        file_view_move_selection (fileview, NULL);
        goto out;
    }

    /* first, try if this is a file in the list */
    if (find_match_visible (fileview, text, &filter_iter, FALSE))
    {
        file_view_move_selection (fileview, &filter_iter);
        goto out;
    }

    /* check if full name of hidden file was typed in */
    if (moo_folder_model_get_iter_by_display_name (
        MOO_FOLDER_MODEL (fileview->priv->model), text, &iter))
    {
        GtkTreePath *path, *filter_path;

        path = gtk_tree_model_get_path (fileview->priv->model, &iter);
        fileview->priv->temp_visible = g_strdup (text);
        fileview->priv->temp_visible_row =
                gtk_tree_row_reference_new (fileview->priv->model, path);
        gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER
                (fileview->priv->filter_model));
        need_to_refilter = FALSE;

        filter_path = gtk_tree_model_filter_convert_child_path_to_path (
                GTK_TREE_MODEL_FILTER (fileview->priv->filter_model), path);
        g_assert (filter_path != NULL);

        gtk_tree_model_get_iter (fileview->priv->filter_model,
                                 &filter_iter, filter_path);
        file_view_move_selection (fileview, &filter_iter);

        gtk_tree_path_free (filter_path);
        gtk_tree_path_free (path);
        goto out;
    }

    /* typed text is not a filename in current dir, unselect everything */
    file_view_move_selection (fileview, NULL);

    /* try completion */
    if (try_completion (fileview, text))
    {
        need_to_clear_completion = FALSE;
        goto out;
    }

out:
    if (need_to_refilter)
        gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER
            (fileview->priv->filter_model));
    if (need_to_clear_completion)
        completion_clear (fileview);
}


void        moo_file_view_select_file       (MooFileView    *fileview,
                                             const char     *basename)
{
    if (basename)
    {
        GtkTreeIter iter, filter_iter;

        if (moo_folder_model_get_iter_by_name (
            MOO_FOLDER_MODEL (fileview->priv->model), basename, &iter))
        {
            GtkTreePath *filter_path, *path;

            path = gtk_tree_model_get_path (fileview->priv->model, &iter);
            g_return_if_fail (path != NULL);

            filter_path = gtk_tree_model_filter_convert_child_path_to_path (
                    GTK_TREE_MODEL_FILTER (fileview->priv->filter_model), path);
            if (!filter_path)
            {
                g_assert (!find_match_visible (fileview, basename,
                           &filter_iter, TRUE));

                gtk_tree_path_free (path);
                if (gtk_tree_model_get_iter_first (fileview->priv->filter_model,
                    &filter_iter))
                    file_view_move_selection (fileview, &filter_iter);
            }
            else
            {
                gtk_tree_model_get_iter (fileview->priv->filter_model,
                                         &filter_iter, filter_path);
                file_view_move_selection (fileview, &filter_iter);
                gtk_tree_path_free (path);
                gtk_tree_path_free (filter_path);
            }
        }
        else
        {
            g_return_if_reached ();
        }
    }
    else
    {
        file_view_move_selection (fileview, NULL);
    }
}


/*********************************************************************/
/* Completion stuff
 */

enum {
    SEARCH_COLUMN_NAME,
    SEARCH_COLUMN_FILE
};

struct _Completion {
    GtkEntry *entry;
    GtkEntryCompletion *completion;
    GtkListStore *store;
    MooFolder *folder;
    char *text_utf8;    /* what was entered into search entry */
    char *basename;     /* real basename */
    char *basename_utf8;/* basename part of text_utf8 */
    char *dirname;      /* real dirname, like /home/username for ~ */
    char *dirname_utf8; /* dir part of text_utf8 */
};


static void     completion_connect_folder   (MooFileView    *fileview,
                                             MooFolder      *folder);
static void     completion_disconnect_folder(MooFileView    *fileview);


static void     completion_clear            (MooFileView    *fileview)
{
    Completion *cmpl = fileview->priv->completion;
    gtk_list_store_clear (cmpl->store);
    completion_disconnect_folder (fileview);
    g_free (cmpl->basename);
    g_free (cmpl->dirname);
    g_free (cmpl->text_utf8);
    g_free (cmpl->basename_utf8);
    g_free (cmpl->dirname_utf8);
    cmpl->basename = NULL;
    cmpl->dirname = NULL;
    cmpl->text_utf8 = NULL;
    cmpl->basename_utf8 = NULL;
    cmpl->dirname_utf8 = NULL;
}


static void         completion_init         (MooFileView    *fileview)
{
    Completion *cmpl;

    cmpl = fileview->priv->completion = g_new0 (Completion, 1);

    cmpl->entry = fileview->priv->search_entry;

    cmpl->completion = gtk_entry_completion_new ();
    gtk_entry_set_completion (cmpl->entry, cmpl->completion);

    cmpl->store = gtk_list_store_new (2,
                                      G_TYPE_STRING,
                                      MOO_TYPE_FILE);
    gtk_entry_completion_set_model (cmpl->completion,
                                    GTK_TREE_MODEL (cmpl->store));
    gtk_entry_completion_set_text_column (cmpl->completion,
                                          SEARCH_COLUMN_NAME);
}


static void         completion_free         (MooFileView    *fileview)
{
    Completion *cmpl = fileview->priv->completion;
    completion_clear (fileview);
    g_object_unref (cmpl->completion);
    g_object_unref (cmpl->store);
    g_free (cmpl);
    fileview->priv->completion = NULL;
}


static gboolean parse_partial_filename      (const char     *text_utf8,
                                             char          **dirname,
                                             char          **basename,
                                             char          **display_dirname,
                                             char          **display_basename,
                                             GError        **error);
static MooFolder *get_folder_for_dirname    (MooFileView    *fileview,
                                             const char     *dirname,
                                             GError        **error);
static char *filename_to_absolute           (MooFileView    *fileview,
                                             const char     *filename);

#if 0
#define PRINT_COMPLETION g_print
#else
static void PRINT_COMPLETION (const char *format, ...) G_GNUC_PRINTF (1, 2);
static void PRINT_COMPLETION (G_GNUC_UNUSED const char *format, ...)
{
}
#endif


static gboolean try_completion              (MooFileView    *fileview,
                                             const char     *text_utf8)
{
    Completion *cmpl = fileview->priv->completion;
    GError *error = NULL;
    char *basename, *dirname, *dirname_utf8, *basename_utf8;
    MooFolder *folder;

    g_return_val_if_fail (text_utf8 && text_utf8[0], FALSE);

    if (cmpl->text_utf8 && !strcmp (text_utf8, cmpl->text_utf8))
        return TRUE;

    if (!parse_partial_filename (text_utf8, &dirname, &basename,
                                 &dirname_utf8, &basename_utf8,
                                 &error))
    {
        g_message ("%s: could not parse '%s'", G_STRLOC, text_utf8);
        if (error)
        {
            g_message ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }
        completion_clear (fileview);
        return FALSE;
    }

    g_assert (dirname != NULL && basename != NULL);
    g_assert (dirname_utf8 != NULL && basename_utf8 != NULL);

    if (!cmpl->dirname || strcmp (cmpl->dirname, dirname))
    {
        folder = get_folder_for_dirname (fileview, dirname, &error);

        if (!folder)
        {
            g_message ("%s: could not get dir for '%s'",
                       G_STRLOC, dirname);

            if (error)
            {
                g_message ("%s: %s", G_STRLOC, error->message);
                g_error_free (error);
            }

            g_free (dirname);
            g_free (basename);
            g_free (dirname_utf8);
            g_free (basename_utf8);
            return FALSE;
        }

        /* we are playing interactive search,
           no completion in current dir */
        if (folder == fileview->priv->current_dir)
        {
            g_free (dirname);
            g_free (basename);
            g_free (dirname_utf8);
            g_free (basename_utf8);
            return FALSE;
        }
    }
    else
    {
        g_assert (cmpl->folder != NULL);
        folder = cmpl->folder;
        /* it's unref'ed in completion_clear() */
        g_object_ref (folder);
    }

    completion_clear (fileview);

    cmpl->text_utf8 = g_strdup (text_utf8);
    cmpl->basename = basename;
    cmpl->basename_utf8 = basename_utf8;
    cmpl->dirname = dirname;
    cmpl->dirname_utf8 = dirname_utf8;

    completion_connect_folder (fileview, folder);
    /* it's ref'ed in completion_connect_folder() */
    g_object_unref (folder);

    PRINT_COMPLETION ("dirname: %s\n", dirname ? dirname : "NONE");
    PRINT_COMPLETION ("basename: %s\n", basename ? basename : "NONE");
    PRINT_COMPLETION ("dirname_utf8: %s\n", dirname_utf8 ? dirname_utf8 : "NONE");
    PRINT_COMPLETION ("basename_utf8: %s\n", basename_utf8 ? basename_utf8 : "NONE");

    return TRUE;
}


static gboolean parse_partial_filename      (const char     *text_utf8,
                                             char          **dirnamep,
                                             char          **basenamep,
                                             char          **display_dirnamep,
                                             char          **display_basenamep,
                                             GError        **error)
{
    GError *error_here = NULL;
    char *text = NULL;
    const char *separator;
    char *dirname = NULL, *basename = NULL;
    char *display_dirname = NULL, *display_basename = NULL;
    const char *single = NULL;
    gsize len;

    g_return_val_if_fail (text_utf8 && text_utf8[0], FALSE);

    if (!strcmp (text_utf8, G_DIR_SEPARATOR_S))
        single = G_DIR_SEPARATOR_S;
    else if (!strcmp (text_utf8, "~"))
        single = "~";

    if (single)
    {
        display_dirname = g_strdup (single);
        dirname = g_strdup (single);
        basename = g_strdup ("");
        display_basename = g_strdup ("");
        goto success;
    }

    text = g_filename_from_utf8 (text_utf8, -1, NULL, &len, &error_here);

    if (error_here)
    {
        g_propagate_error (error, error_here);
        goto error_label;
    }

    if (len == 1 && text[0] == G_DIR_SEPARATOR)
    {
        display_dirname = g_strdup (G_DIR_SEPARATOR_S);
        dirname = g_strdup (G_DIR_SEPARATOR_S);
        basename = g_strdup ("");
        display_basename = g_strdup ("");
        text = NULL;
        goto success;
    }

    if (text[len-1] == G_DIR_SEPARATOR)
    {
        display_dirname = g_strndup (text_utf8, strlen (text_utf8) - 1);
        dirname = text;
        dirname[len-1] = 0;
        basename = g_strdup ("");
        display_basename = g_strdup ("");
        text = NULL;
        goto success;
    }

    if (!(separator = strrchr (text, G_DIR_SEPARATOR)))
    {
        display_basename = g_strdup (text_utf8);
        basename = text;
        dirname = g_strdup ("");
        display_dirname = g_strdup ("");
        text = NULL;
        goto success;
    }

    /* TODO TODO this all is broken, at least on windows */
    /* TODO TODO normalize dirname */

    if (separator == text)
    {
        basename = g_strdup (separator + 1);
        dirname = g_strdup (G_DIR_SEPARATOR_S);
        display_basename = g_filename_display_name (basename);
        display_dirname = g_filename_display_name (dirname);
        goto success;
    }
    else
    {
        basename = g_strdup (separator + 1);
        dirname = g_strndup (text, separator - text);
        display_basename = g_filename_display_name (basename);
        display_dirname = g_filename_display_name (dirname);
        goto success;
    }

    /* no fallthrough */
    g_assert_not_reached ();

error_label:
    g_free (text);
    g_free (dirname);
    g_free (basename);
    g_free (display_dirname);
    g_free (display_basename);
    return FALSE;

success:
    g_free (text);
    *dirnamep = dirname;
    *basenamep = basename;
    *display_dirnamep = display_dirname;
    *display_basenamep = display_basename;
    return TRUE;
}


static char *filename_to_absolute           (MooFileView    *fileview,
                                             const char     *filename)
{
    g_return_val_if_fail (filename && filename[0], g_strdup (filename));

    if (filename[0] == '~')
    {
        const char *home = g_get_home_dir ();
        g_return_val_if_fail (home != NULL, g_strdup (filename));

        if (filename[1])
            return g_build_filename (home, filename + 1, NULL);
        else
            return g_strdup (home);
    }

    if (g_path_is_absolute (filename))
        return g_strdup (filename);

    if (fileview->priv->current_dir)
        return g_build_filename (moo_folder_get_path (fileview->priv->current_dir),
                                 filename, NULL);

    g_return_val_if_reached (g_strdup (filename));
}


static MooFolder *get_folder_for_dirname    (MooFileView    *fileview,
                                             const char     *dirname,
                                             GError        **error)
{
    MooFolder *folder;
    char *path = NULL;

    g_assert (dirname != NULL);

    if (!dirname[0] && fileview->priv->current_dir)
        return g_object_ref (fileview->priv->current_dir);

    path = filename_to_absolute (fileview, dirname);
    g_return_val_if_fail (path != NULL, NULL);

    folder = moo_file_system_get_folder (fileview->priv->file_system,
                                         path, MOO_FILE_HAS_STAT, error);
    g_free (path);
    return folder;
}


static void     completion_update_files     (MooFileView    *fileview);
static void     completion_folder_deleted   (MooFolder      *folder,
                                             MooFileView    *fileview);
static void     completion_files_added      (MooFolder      *folder,
                                             GSList         *files,
                                             MooFileView    *fileview);
static void     completion_files_changed    (MooFolder      *folder,
                                             GSList         *files,
                                             MooFileView    *fileview);
static void     completion_files_removed    (MooFolder      *folder,
                                             GSList         *files,
                                             MooFileView    *fileview);

static void     completion_connect_folder   (MooFileView    *fileview,
                                             MooFolder      *folder)
{
    Completion *cmpl = fileview->priv->completion;

    g_assert (MOO_IS_FOLDER (folder));
    g_assert (cmpl->folder == NULL);

    PRINT_COMPLETION ("completion_connect_folder, folder '%s'\n",
                      moo_folder_get_path (folder));

    cmpl->folder = g_object_ref (folder);

    g_signal_connect (folder, "deleted",
                      G_CALLBACK (completion_folder_deleted), fileview);
    g_signal_connect (folder, "files_added",
                      G_CALLBACK (completion_files_added), fileview);
    g_signal_connect (folder, "files_removed",
                      G_CALLBACK (completion_files_removed), fileview);
    g_signal_connect (folder, "files_changed",
                      G_CALLBACK (completion_files_changed), fileview);

    completion_update_files (fileview);
}


static void     completion_disconnect_folder(MooFileView    *fileview)
{
    MooFolder *folder = fileview->priv->completion->folder;

    PRINT_COMPLETION ("completion_disconnect_folder, folder '%s'\n",
                      folder ? moo_folder_get_path (folder) : "NULL");

    if (folder)
    {
        fileview->priv->completion->folder = NULL;
        g_signal_handlers_disconnect_by_func (folder,
                                              (gpointer) completion_folder_deleted,
                                              fileview);
        g_signal_handlers_disconnect_by_func (folder,
                                              (gpointer) completion_files_added,
                                              fileview);
        g_signal_handlers_disconnect_by_func (folder,
                                              (gpointer) completion_files_removed,
                                              fileview);
        g_signal_handlers_disconnect_by_func (folder,
                                              (gpointer) completion_files_changed,
                                              fileview);
        g_object_unref (folder);
    }
}


static void     completion_update_files     (MooFileView    *fileview)
{
    Completion *cmpl = fileview->priv->completion;
    GtkTreeIter iter;
    GSList *files, *l;

    PRINT_COMPLETION ("completion_update_files, folder '%s'\n",
                      moo_folder_get_path (cmpl->folder));

    g_assert (cmpl->folder != NULL);
    g_assert (cmpl->basename != NULL);
    g_assert (cmpl->basename_utf8 != NULL);
    g_assert (cmpl->dirname != NULL);
    g_assert (cmpl->dirname_utf8 != NULL);

    files = moo_folder_list_files (cmpl->folder);

    for (l = files; l != NULL; l = l->next)
    {
        char *name;

        if (!cmpl->basename[0])
        {
            if (!moo_file_view_check_visible (fileview, l->data, FALSE, TRUE))
                continue;
        }
        else
        {
            if (!moo_file_view_check_visible (fileview, l->data, TRUE, TRUE))
                continue;
        }

        if (moo_file_test (l->data, MOO_FILE_IS_FOLDER))
            name = g_build_filename (cmpl->dirname_utf8,
                                    moo_file_get_display_basename (l->data),
                                    G_DIR_SEPARATOR_S, NULL);
        else
            name = g_build_filename (cmpl->dirname_utf8,
                                     moo_file_get_display_basename (l->data),
                                     NULL);

        gtk_list_store_append (cmpl->store, &iter);
        gtk_list_store_set (cmpl->store, &iter,
                            SEARCH_COLUMN_NAME, name,
                            SEARCH_COLUMN_FILE, l->data,
                            -1);
        g_free (name);
    }

    g_slist_foreach (files, (GFunc) moo_file_unref, NULL);
    g_slist_free (files);
}


static void     completion_folder_deleted   (G_GNUC_UNUSED MooFolder *folder,
                                             MooFileView    *fileview)
{
    g_assert (folder == fileview->priv->completion->folder);
    PRINT_COMPLETION ("completion_folder_deleted\n");
    completion_clear (fileview);
}


/* TODO */
static void     completion_files_added      (G_GNUC_UNUSED MooFolder *folder,
                                             G_GNUC_UNUSED GSList *files,
                                             MooFileView    *fileview)
{
    g_assert (folder == fileview->priv->completion->folder);
    PRINT_COMPLETION ("completion_files_added\n");
    completion_update_files (fileview);
}

static void     completion_files_changed    (G_GNUC_UNUSED MooFolder *folder,
                                             G_GNUC_UNUSED GSList *files,
                                             MooFileView    *fileview)
{
    g_assert (folder == fileview->priv->completion->folder);
    PRINT_COMPLETION ("completion_files_changed\n");
    completion_update_files (fileview);
}

static void     completion_files_removed    (G_GNUC_UNUSED MooFolder *folder,
                                             G_GNUC_UNUSED GSList *files,
                                             MooFileView    *fileview)
{
    g_assert (folder == fileview->priv->completion->folder);
    PRINT_COMPLETION ("completion_files_removed\n");
    completion_update_files (fileview);
}


static void     activate_filename           (MooFileView    *fileview,
                                             const char     *filename)
{
    GError *error = NULL;
    char *dirname, *basename;
    char *path = NULL;

    path = filename_to_absolute (fileview, filename);

    if (!path || !g_file_test (path, G_FILE_TEST_EXISTS))
    {
        g_free (path);
        g_return_if_reached ();
    }

    if (g_file_test (path, G_FILE_TEST_IS_DIR))
    {
        if (!moo_file_view_chdir (fileview, path, &error))
        {
            g_warning ("%s: could not chdir to %s",
                       G_STRLOC, path);
            if (error)
            {
                g_warning ("%s: %s", G_STRLOC, error->message);
                g_error_free (error);
            }
        }

        g_free (path);
        return;
    }

    dirname = g_path_get_dirname (filename);
    basename = g_path_get_basename (filename);

    if (!dirname || !basename)
    {
        g_free (dirname);
        g_free (basename);
        g_return_if_reached ();
    }

    if (!moo_file_view_chdir (fileview, dirname, &error))
    {
        g_warning ("%s: could not chdir to %s",
                    G_STRLOC, dirname);

        if (error)
        {
            g_warning ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }

        g_free (path);
        g_free (dirname);
        g_free (basename);
        return;
    }

    moo_file_view_select_file (fileview, basename);
    g_signal_emit (fileview, signals[ACTIVATE], 0, path);

    g_free (path);
    g_free (dirname);
    g_free (basename);
}


static void     search_entry_activate       (GtkEntry       *entry,
                                             MooFileView    *fileview)
{
    char *filename = NULL;
    GtkTreePath *selected;

    selected = file_widget_get_selected (fileview);

    if (selected)
    {
        GtkTreeIter iter;
        MooFile *file = NULL;
        gtk_tree_model_get_iter (fileview->priv->filter_model, &iter, selected);
        gtk_tree_model_get (fileview->priv->filter_model, &iter, COLUMN_FILE, &file, -1);
        gtk_tree_path_free (selected);
        g_return_if_fail (file != NULL);

        filename = g_strdup (moo_file_get_basename (file));
        moo_file_unref (file);
    }
    else
    {
        filename = g_strdup (gtk_entry_get_text (entry));
    }

    stop_interactive_search (fileview, TRUE);
    activate_filename (fileview, filename);
    g_free (filename);
}
