/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   moofileview.c
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

#define MOO_FILE_SYSTEM_COMPILATION
#include "moofileview.h"
#include "moofileview-dialogs.h"
#include "moobookmarkmgr.h"
#include "moofilesystem.h"
#include "moofoldermodel.h"
#include "moofileentry.h"
#include MOO_MARSHALS_H
#include "mooiconview.h"
#include "moofileview-private.h"
#include "mooutils/mooutils-gobject.h"
#include "mooutils/moofiltermgr.h"
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>


#ifndef __WIN32__
#define TYPEAHEAD_CASE_SENSITIVE_DEFAULT    FALSE
#define SORT_CASE_SENSITIVE_DEFAULT         MOO_FOLDER_MODEL_SORT_CASE_SENSITIVE_DEFAULT
#define COMPLETION_CASE_SENSITIVE_DEFAULT   TRUE
#else /* __WIN32__ */
#define TYPEAHEAD_CASE_SENSITIVE_DEFAULT    FALSE
#define SORT_CASE_SENSITIVE_DEFAULT         FALSE
#define COMPLETION_CASE_SENSITIVE_DEFAULT   FALSE
#endif /* __WIN32__ */


enum {
    TREEVIEW_PAGE = 0,
    ICONVIEW_PAGE = 1
};


#if 0
static GtkTargetEntry source_targets[] = {
    {(char*) "text/uri-list", 0, 0},
    {(char*) "text/plain", 0, 0}
};
#endif


typedef struct _History History;
typedef struct _Typeahead Typeahead;

struct _MooFileViewPrivate {
    GtkTreeModel    *model;
    GtkTreeModel    *filter_model;
    MooFolder       *current_dir;
    MooFileSystem   *file_system;

    guint            select_file_idle;

    GtkIconSize      icon_size;
    GtkNotebook     *notebook;
    MooFileViewType  view_type;

    GtkTreeView     *treeview;
    GtkTreeViewColumn *tree_name_column;
    MooIconView     *iconview;

    GtkMenu         *bookmarks_menu;
    MooBookmarkMgr  *bookmark_mgr;

    char            *home_dir;

    gboolean         show_hidden_files;
    gboolean         show_two_dots;
    History         *history;
    GString         *temp_visible;  /* temporary visible name, for interactive search */

    MooFilterMgr    *filter_mgr;
    GtkToggleButton *filter_button;
    MooCombo        *filter_combo;
    GtkEntry        *filter_entry;
    GtkFileFilter   *current_filter;
    gboolean         use_current_filter;

    GtkEntry        *entry;
    int              entry_state;   /* it can be one of three: nothing, typeahead, or completion,
                                       depending on text entered into the entry */
    Typeahead       *typeahead;
    gboolean         typeahead_case_sensitive;
    gboolean         sort_case_sensitive;
    gboolean         completion_case_sensitive;
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
static gboolean     moo_file_view_popup_menu    (GtkWidget      *widget);

static void         moo_file_view_set_filter_mgr    (MooFileView    *fileview,
                                                     MooFilterMgr   *mgr);
static void         moo_file_view_set_bookmark_mgr  (MooFileView    *fileview,
                                                     MooBookmarkMgr *mgr);
static void         destroy_bookmarks_menu          (MooFileView    *fileview);

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

static void         goto_item_activated     (GtkWidget      *widget,
                                             MooFileView    *fileview);
static void         show_hidden_toggled     (GtkWidget      *widget,
                                             MooFileView    *fileview);
static void         show_two_dots_toggled   (GtkWidget      *widget,
                                             MooFileView    *fileview);
static void         sort_case_toggled       (GtkWidget      *widget,
                                             MooFileView    *fileview);
static void         view_type_item_toggled  (GtkWidget      *widget,
                                             MooFileView    *fileview);
static void         boomarks_button_toggled (GtkToggleButton *button,
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
static gboolean     tree_button_press       (GtkTreeView    *treeview,
                                             GdkEventButton *event,
                                             MooFileView    *fileview);

static GtkWidget   *create_iconview         (MooFileView    *fileview);
static gboolean     icon_button_press       (MooIconView    *iconview,
                                             GdkEventButton *event,
                                             MooFileView    *fileview);

static void         tree_path_activated     (MooFileView    *fileview,
                                             GtkTreePath    *filter_path);

static GtkWidget   *get_file_view_widget    (MooFileView    *fileview);
static void         file_view_move_selection(MooFileView    *fileview,
                                             GtkTreeIter    *filter_iter);

static void         path_entry_init         (MooFileView    *fileview);
static void         path_entry_deinit       (MooFileView    *fileview);
static void         path_entry_set_text     (MooFileView    *fileview,
                                             const char     *text);
static void         stop_path_entry         (MooFileView    *fileview,
                                             gboolean        focus_file_list);
static void         path_entry_delete_to_cursor (MooFileView *fileview);
static void         file_view_activate_filename (MooFileView *fileview,
                                             const char     *display_name);

static void moo_file_view_populate_popup    (MooFileView    *fileview,
                                             GList          *selected,
                                             GtkMenu        *menu);

static void select_display_name_in_idle     (MooFileView    *fileview,
                                             const char     *display_name);
/* returns path in the fileview->priv->filter_model */
static GtkTreePath *file_view_get_selected  (MooFileView    *fileview);
static GList       *file_view_get_selected_rows (MooFileView *fileview);

static void file_view_delete_selected       (MooFileView    *fileview);
static void file_view_create_folder         (MooFileView    *fileview);
static void file_view_properties_dialog     (MooFileView    *fileview,
                                             gboolean        current_dir);


/* MOO_TYPE_FILE_VIEW */
G_DEFINE_TYPE (MooFileView, moo_file_view, GTK_TYPE_VBOX)

enum {
    PROP_0,
    PROP_CURRENT_DIRECTORY,
    PROP_HOME_DIRECTORY,
    PROP_FILTER_MGR,
    PROP_BOOKMARK_MGR,
    PROP_SORT_CASE_SENSITIVE,
    PROP_TYPEAHEAD_CASE_SENSITIVE,
    PROP_COMPLETION_CASE_SENSITIVE
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
    DELETE_TO_CURSOR,
    PROPERTIES_DIALOG,
    DELETE_SELECTED,
    CREATE_FOLDER,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

static void moo_file_view_class_init (MooFileViewClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    GtkBindingSet *binding_set;

    gobject_class->finalize = moo_file_view_finalize;
    gobject_class->set_property = moo_file_view_set_property;
    gobject_class->get_property = moo_file_view_get_property;

    widget_class->popup_menu = moo_file_view_popup_menu;

    klass->chdir = moo_file_view_chdir_real;
    klass->populate_popup = moo_file_view_populate_popup;

    g_object_class_install_property (gobject_class,
                                     PROP_CURRENT_DIRECTORY,
                                     g_param_spec_string ("current-directory",
                                             "current-directory",
                                             "current-directory",
                                             NULL,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_HOME_DIRECTORY,
                                     g_param_spec_string ("home-directory",
                                             "home-directory",
                                             "home-directory",
#ifndef __WIN32__
                                             g_get_home_dir (),
#else
#warning "Do something here"
                                             NULL,
#endif
                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_FILTER_MGR,
                                     g_param_spec_object ("filter-mgr",
                                             "filter-mgr",
                                             "filter-mgr",
                                             MOO_TYPE_FILTER_MGR,
                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_BOOKMARK_MGR,
                                     g_param_spec_object ("bookmark-mgr",
                                             "bookmark-mgr",
                                             "bookmark-mgr",
                                             MOO_TYPE_BOOKMARK_MGR,
                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_TYPEAHEAD_CASE_SENSITIVE,
                                     g_param_spec_boolean ("typeahead-case-sensitive",
                                             "typeahead-case-sensitive",
                                             "typeahead-case-sensitive",
                                             TYPEAHEAD_CASE_SENSITIVE_DEFAULT,
                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_SORT_CASE_SENSITIVE,
                                     g_param_spec_boolean ("sort-case-sensitive",
                                             "sort-case-sensitive",
                                             "sort-case-sensitive",
                                             SORT_CASE_SENSITIVE_DEFAULT,
                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_COMPLETION_CASE_SENSITIVE,
                                     g_param_spec_boolean ("completion-case-sensitive",
                                             "completion-case-sensitive",
                                             "completion-case-sensitive",
                                             COMPLETION_CASE_SENSITIVE_DEFAULT,
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
            g_signal_new ("populate-popup",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_FIRST,
                          G_STRUCT_OFFSET (MooFileViewClass, populate_popup),
                          NULL, NULL,
                          _moo_marshal_VOID__POINTER_OBJECT,
                          G_TYPE_NONE, 2,
                          G_TYPE_POINTER,
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

    signals[DELETE_TO_CURSOR] =
            moo_signal_new_cb ("delete-to-cursor",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                               G_CALLBACK (path_entry_delete_to_cursor),
                               NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);

    signals[DELETE_SELECTED] =
            moo_signal_new_cb ("delete-selected",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                               G_CALLBACK (file_view_delete_selected),
                               NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);

    signals[CREATE_FOLDER] =
            moo_signal_new_cb ("create-folder",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                               G_CALLBACK (file_view_create_folder),
                               NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);

    signals[PROPERTIES_DIALOG] =
             moo_signal_new_cb ("properties-dialog",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                               G_CALLBACK (file_view_properties_dialog),
                               NULL, NULL,
                               _moo_marshal_VOID__BOOLEAN,
                               G_TYPE_NONE, 1,
                               G_TYPE_BOOLEAN);

    binding_set = gtk_binding_set_by_class (klass);

    gtk_binding_entry_add_signal (binding_set,
                                  GDK_u, GDK_CONTROL_MASK,
                                  "delete-to-cursor", 0);

    gtk_binding_entry_add_signal (binding_set,
                                  GDK_Return, GDK_MOD1_MASK,
                                  "properties-dialog", 1,
                                  G_TYPE_BOOLEAN, FALSE);

    gtk_binding_entry_add_signal (binding_set,
                                  GDK_Delete, GDK_MOD1_MASK,
                                  "delete-selected", 0);

    gtk_binding_entry_add_signal (binding_set,
                                  GDK_Up, GDK_MOD1_MASK,
                                  "go-up", 0);
    gtk_binding_entry_add_signal (binding_set,
                                  GDK_KP_Up, GDK_MOD1_MASK,
                                  "go-up", 0);
    gtk_binding_entry_add_signal (binding_set,
                                  GDK_Left, GDK_MOD1_MASK,
                                  "go-back", 0);
    gtk_binding_entry_add_signal (binding_set,
                                  GDK_KP_Left, GDK_MOD1_MASK,
                                  "go-back", 0);
    gtk_binding_entry_add_signal (binding_set,
                                  GDK_Right, GDK_MOD1_MASK,
                                  "go-forward", 0);
    gtk_binding_entry_add_signal (binding_set,
                                  GDK_KP_Right, GDK_MOD1_MASK,
                                  "go-forward", 0);
    gtk_binding_entry_add_signal (binding_set,
                                  GDK_Home, GDK_MOD1_MASK,
                                  "go-home", 0);
    gtk_binding_entry_add_signal (binding_set,
                                  GDK_KP_Home, GDK_MOD1_MASK,
                                  "go-home", 0);

    gtk_binding_entry_add_signal (binding_set,
                                  GDK_f, GDK_MOD1_MASK | GDK_SHIFT_MASK,
                                  "focus-to-filter-entry", 0);
    gtk_binding_entry_add_signal (binding_set,
                                  GDK_b, GDK_MOD1_MASK | GDK_SHIFT_MASK,
                                  "focus-to-file-view", 0);
    gtk_binding_entry_add_signal (binding_set,
                                  GDK_h, GDK_MOD1_MASK | GDK_SHIFT_MASK,
                                  "toggle-show-hidden", 0);
}


static void moo_file_view_init      (MooFileView *fileview)
{
    fileview->priv = g_new0 (MooFileViewPrivate, 1);
    fileview->priv->show_hidden_files = FALSE;
    fileview->priv->view_type = MOO_FILE_VIEW_ICON;
    fileview->priv->use_current_filter = FALSE;
    fileview->priv->icon_size = GTK_ICON_SIZE_MENU;

    fileview->priv->typeahead_case_sensitive = TYPEAHEAD_CASE_SENSITIVE_DEFAULT;
    fileview->priv->sort_case_sensitive = SORT_CASE_SENSITIVE_DEFAULT;
    fileview->priv->completion_case_sensitive = COMPLETION_CASE_SENSITIVE_DEFAULT;

    history_init (fileview);

    fileview->priv->model = g_object_new (MOO_TYPE_FOLDER_MODEL,
                                          "sort-case-sensitive",
                                          fileview->priv->sort_case_sensitive,
                                          NULL);
    fileview->priv->filter_model =
            moo_folder_filter_new (MOO_FOLDER_MODEL (fileview->priv->model));
    gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (fileview->priv->filter_model),
                                            (GtkTreeModelFilterVisibleFunc) filter_visible_func,
                                            fileview, NULL);

    fileview->priv->file_system = moo_file_system_create ();

    init_gui (fileview);
    path_entry_init (fileview);
}


static void moo_file_view_finalize  (GObject      *object)
{
    MooFileView *fileview = MOO_FILE_VIEW (object);

    path_entry_deinit (fileview);

    if (fileview->priv->select_file_idle)
        g_source_remove (fileview->priv->select_file_idle);

    g_object_unref (fileview->priv->model);
    g_object_unref (fileview->priv->filter_model);
    history_free (fileview);

    destroy_bookmarks_menu (fileview);

    if (fileview->priv->bookmark_mgr)
    {
        g_signal_handlers_disconnect_by_func (fileview->priv->bookmark_mgr,
                                              (gpointer) destroy_bookmarks_menu,
                                              fileview);
        g_object_unref (fileview->priv->bookmark_mgr);
    }

    if (fileview->priv->filter_mgr)
        g_object_unref (fileview->priv->filter_mgr);
    if (fileview->priv->current_filter)
        g_object_unref (fileview->priv->current_filter);

    if (fileview->priv->temp_visible)
        g_string_free (fileview->priv->temp_visible, TRUE);

    if (fileview->priv->current_dir)
        g_object_unref (fileview->priv->current_dir);
    g_object_unref (fileview->priv->file_system);

    g_free (fileview->priv->home_dir);

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
    char *path;

    g_return_if_fail (MOO_IS_FILE_VIEW (fileview));
    g_return_if_fail (!folder || MOO_IS_FOLDER (folder));

    if (folder == fileview->priv->current_dir)
        return;

    if (fileview->priv->temp_visible)
    {
        g_string_free (fileview->priv->temp_visible, TRUE);
        fileview->priv->temp_visible = NULL;
    }

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

        path_entry_set_text (fileview, "");
        g_object_set (MOO_FILE_ENTRY (fileview->priv->entry),
                      "current-dir", NULL, NULL);
        g_object_notify (G_OBJECT (fileview), "current-directory");
        return;
    }

    if (fileview->priv->current_dir)
        g_object_unref (fileview->priv->current_dir);

    fileview->priv->current_dir = g_object_ref (folder);
    moo_folder_model_set_folder (MOO_FOLDER_MODEL (fileview->priv->model),
                                 folder);

    if (gtk_tree_model_get_iter_first (fileview->priv->filter_model, &filter_iter))
        file_view_move_selection (fileview, &filter_iter);

    if (gtk_widget_is_focus (GTK_WIDGET (fileview->priv->entry)))
        focus_to_file_view (fileview);

    path = g_filename_display_name (moo_folder_get_path (folder));
    path_entry_set_text (fileview, path);
    g_free (path);

    g_object_set (MOO_FILE_ENTRY(fileview->priv->entry)->completion,
                  "current-dir", moo_folder_get_path (folder), NULL);

    history_goto (fileview, moo_folder_get_path (folder));
    g_object_notify (G_OBJECT (fileview), "current-directory");
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
    GtkWidget *toolbar, *notebook, *filter_combo, *entry;

    box = GTK_BOX (fileview);

    toolbar = create_toolbar (fileview);
    gtk_widget_show (toolbar);
    gtk_box_pack_start (box, toolbar, FALSE, FALSE, 0);

    entry = moo_file_entry_new ();
    g_object_set_data (G_OBJECT (entry), "moo-file-view", fileview);
    gtk_widget_show (entry);
    gtk_box_pack_start (box, entry, FALSE, FALSE, 0);
    fileview->priv->entry = GTK_ENTRY (entry);

    notebook = create_notebook (fileview);
    gtk_widget_show (notebook);
    gtk_box_pack_start (box, notebook, TRUE, TRUE, 0);
    fileview->priv->notebook = GTK_NOTEBOOK (notebook);

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
        path = moo_icon_view_get_cursor (iconview);
        gtk_tree_selection_unselect_all (selection);
        if (path)
        {
            gtk_tree_view_set_cursor (treeview, path, NULL, FALSE);
            gtk_tree_path_free (path);
        }
    }
    else
    {
        gtk_tree_view_get_cursor (treeview, &path, NULL);
        moo_icon_view_unselect_all (iconview);
        if (path)
        {
            moo_icon_view_set_cursor (iconview, path, FALSE);
            gtk_tree_path_free (path);
        }
    }

    if (type == MOO_FILE_VIEW_ICON)
        gtk_notebook_set_current_page (fileview->priv->notebook,
                                       ICONVIEW_PAGE);
    else
        gtk_notebook_set_current_page (fileview->priv->notebook,
                                       TREEVIEW_PAGE);
}


GtkWidget*
moo_file_view_add_button (MooFileView  *fileview,
                          GType         type,
                          const char   *stock_id,
                          const char   *tip)
{
    GtkWidget *icon, *button;
    GtkTooltips *tooltips;

    g_return_val_if_fail (MOO_IS_FILE_VIEW (fileview), NULL);
    g_return_val_if_fail (g_type_is_a (type, GTK_TYPE_BUTTON), NULL);

    tooltips = g_object_get_data (G_OBJECT (fileview), "moo-file-view-tooltips");

    if (!tooltips)
    {
        tooltips = gtk_tooltips_new ();
        gtk_object_sink (GTK_OBJECT (g_object_ref (tooltips)));
        g_object_set_data_full (G_OBJECT (fileview),
                                "moo-file-view-tooltips",
                                tooltips, g_object_unref);
    }

    button = g_object_new (type, NULL);
    gtk_widget_show (button);
    gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
    gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);

    if (tip)
        gtk_tooltips_set_tip (tooltips, button, tip, tip);

    gtk_box_pack_start (GTK_BOX (fileview->toolbar), button,
                        FALSE, FALSE, 0);

    if (stock_id)
    {
        icon = gtk_image_new_from_stock (stock_id,
                                         GTK_ICON_SIZE_MENU);
        gtk_widget_show (icon);
        gtk_container_add (GTK_CONTAINER (button), icon);
    }

    return button;
}


static void
create_bookmarks_button (MooFileView  *fileview)
{
    GtkWidget *button;

    button = moo_file_view_add_button (fileview, GTK_TYPE_TOGGLE_BUTTON,
                                       GTK_STOCK_ABOUT, "Bookmarks");

    g_signal_connect (button, "toggled",
                      G_CALLBACK (boomarks_button_toggled),
                      fileview);
}

static void
create_button (MooFileView  *fileview,
               const char   *stock_id,
               const char   *tip,
               const char   *signal)
{
    GtkWidget *button = moo_file_view_add_button (fileview, GTK_TYPE_BUTTON, stock_id, tip);
    g_object_set_data (G_OBJECT (button), "moo-file-view-signal", (gpointer) signal);
    g_signal_connect (button, "clicked", G_CALLBACK (goto_item_activated), fileview);
}

static GtkWidget*
create_toolbar  (MooFileView    *fileview)
{
    fileview->toolbar = gtk_hbox_new (FALSE, 0);

    create_button (fileview, GTK_STOCK_GO_UP, "Up", "go-up");
    create_button (fileview, GTK_STOCK_GO_BACK, "Back", "go-back");
    create_button (fileview, GTK_STOCK_GO_FORWARD, "Forward", "go-forward");
    create_button (fileview, GTK_STOCK_HOME, "Home", "go-home");

    create_bookmarks_button (fileview);

    return fileview->toolbar;
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
    /* gtk+ #313719 */
    g_signal_connect_swapped (treeview, "popup-menu",
                              G_CALLBACK (moo_file_view_popup_menu),
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

    combo = moo_combo_new ();
    gtk_widget_show (combo);
    gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 0);

    fileview->priv->filter_button = GTK_TOGGLE_BUTTON (button);
    fileview->priv->filter_combo = MOO_COMBO (combo);
    fileview->priv->filter_entry = GTK_ENTRY (MOO_COMBO(combo)->entry);

    g_signal_connect_swapped (button, "toggled",
                              G_CALLBACK (filter_button_toggled),
                              fileview);
    g_signal_connect_data (combo, "changed",
                           G_CALLBACK (filter_combo_changed),
                           fileview, NULL,
                           G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    g_signal_connect_swapped (MOO_COMBO(combo)->entry, "activate",
                              G_CALLBACK (filter_entry_activate),
                              fileview);

    return hbox;
}


static GtkWidget   *create_treeview     (MooFileView    *fileview)
{
    GtkWidget *treeview;
    GtkTreeViewColumn *column;
    GtkTreeSelection *selection;
    GtkCellRenderer *cell;

    treeview = gtk_tree_view_new_with_model (fileview->priv->filter_model);

    g_signal_connect_swapped (treeview, "row-activated",
                              G_CALLBACK (tree_path_activated), fileview);
    g_signal_connect (treeview, "button-press-event",
                      G_CALLBACK (tree_button_press), fileview);

#if 0
    gtk_tree_view_enable_model_drag_source (GTK_TREE_VIEW (treeview),
                                            GDK_CONTROL_MASK,
                                            source_targets,
                                            G_N_ELEMENTS (source_targets),
                                            GDK_ACTION_COPY | GDK_ACTION_MOVE |
                                                    GDK_ACTION_LINK);
#endif

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);

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
    moo_icon_view_set_selection_mode (MOO_ICON_VIEW (iconview),
                                      GTK_SELECTION_MULTIPLE);

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

    g_signal_connect_swapped (iconview, "row-activated",
                              G_CALLBACK (tree_path_activated), fileview);
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

    if (!strcmp (moo_file_name (file), ".."))
    {
        if (!ignore_two_dots)
            return fileview->priv->show_two_dots;
        else
            return FALSE;
    }

    if (!ignore_hidden && MOO_FILE_IS_HIDDEN (file))
        return fileview->priv->show_hidden_files;

    if (MOO_FILE_IS_DIR (file))
        return TRUE;

    if (fileview->priv->current_filter && fileview->priv->use_current_filter)
    {
        GtkFileFilterInfo filter_info;
        GtkFileFilter *filter = fileview->priv->current_filter;

        filter_info.contains = gtk_file_filter_get_needed (filter);
        filter_info.filename = moo_file_name (file);
        filter_info.uri = NULL;
        filter_info.display_name = moo_file_display_name (file);
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
        !strncmp (moo_file_name (file),
                  fileview->priv->temp_visible->str,
                  fileview->priv->temp_visible->len))
    {
        visible = moo_file_view_check_visible (fileview, file, TRUE, TRUE);
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
        name = moo_file_display_name (file);

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


/* TODO */
static void         moo_file_view_go_up     (MooFileView    *fileview)
{
    MooFolder *parent = NULL;

    if (fileview->priv->entry_state)
        stop_path_entry (fileview, TRUE);

    if (fileview->priv->current_dir)
        parent = moo_folder_get_parent (fileview->priv->current_dir,
                                        MOO_FILE_HAS_ICON);
    if (!parent)
        parent = moo_file_system_get_root_folder (fileview->priv->file_system,
            MOO_FILE_HAS_ICON);

    g_return_if_fail (parent != NULL);

    if (parent != fileview->priv->current_dir)
    {
        char *name = g_path_get_basename (
                moo_folder_get_path (fileview->priv->current_dir));
        moo_file_view_set_current_dir (fileview, parent);
        moo_file_view_select_name (fileview, name);
        g_free (name);
    }

    g_object_unref (parent);
}


static void         moo_file_view_go_home   (MooFileView    *fileview)
{
    GError *error = NULL;

    if (fileview->priv->entry_state)
        stop_path_entry (fileview, TRUE);

    if (!moo_file_view_chdir (fileview, fileview->priv->home_dir, &error))
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

    if (!strcmp (moo_file_name (file), ".."))
    {
        g_signal_emit_by_name (fileview, "go-up");
    }
    else if (MOO_FILE_IS_DIR (file))
    {
        GError *error = NULL;

        if (!moo_file_view_chdir (fileview, moo_file_name (file), &error))
        {
            g_warning ("%s: could not go into '%s'",
                       G_STRLOC, moo_file_name (file));

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
                                 moo_file_name (file), NULL);
        g_signal_emit (fileview, signals[ACTIVATE], 0, path);
        g_free (path);
    }

    moo_file_unref (file);
}


static void         moo_file_view_go        (MooFileView    *fileview,
                                             GtkDirectionType where)
{
    const char *dir;
    GError *error = NULL;

    if (fileview->priv->entry_state)
        stop_path_entry (fileview, TRUE);

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

    switch (prop_id)
    {
        case PROP_CURRENT_DIRECTORY:
            moo_file_view_chdir (fileview, g_value_get_string (value), NULL);
            break;

        case PROP_HOME_DIRECTORY:
            g_free (fileview->priv->home_dir);
            fileview->priv->home_dir = g_strdup (g_value_get_string (value));
            g_object_notify (object, "home-directory");
            break;

        case PROP_FILTER_MGR:
            moo_file_view_set_filter_mgr (fileview, g_value_get_object (value));
            break;

        case PROP_BOOKMARK_MGR:
            moo_file_view_set_bookmark_mgr (fileview, g_value_get_object (value));
            break;

        case PROP_SORT_CASE_SENSITIVE:
            moo_file_view_set_sort_case_sensitive (fileview, g_value_get_boolean (value));
            break;

        case PROP_TYPEAHEAD_CASE_SENSITIVE:
            moo_file_view_set_typeahead_case_sensitive (fileview, g_value_get_boolean (value));
            break;

        case PROP_COMPLETION_CASE_SENSITIVE:
            g_object_set (MOO_FILE_ENTRY(fileview->priv->entry)->completion,
                          "case-sensitive", g_value_get_boolean (value), NULL);
            g_object_notify (object, "completion-case-sensitive");
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
    gboolean val;

    switch (prop_id)
    {
        case PROP_CURRENT_DIRECTORY:
            if (fileview->priv->current_dir)
                g_value_set_string (value, moo_folder_get_path (fileview->priv->current_dir));
            else
                g_value_set_string (value, NULL);
            break;

        case PROP_HOME_DIRECTORY:
            g_value_set_string (value, fileview->priv->home_dir);
            break;

        case PROP_FILTER_MGR:
            g_value_set_object (value, fileview->priv->filter_mgr);
            break;

        case PROP_BOOKMARK_MGR:
            g_value_set_object (value, fileview->priv->bookmark_mgr);
            break;

        case PROP_SORT_CASE_SENSITIVE:
            g_value_set_boolean (value, fileview->priv->sort_case_sensitive);
            break;

        case PROP_TYPEAHEAD_CASE_SENSITIVE:
            g_value_set_boolean (value, fileview->priv->typeahead_case_sensitive);
            break;

        case PROP_COMPLETION_CASE_SENSITIVE:
            g_object_get (MOO_FILE_ENTRY(fileview->priv->entry)->completion,
                          "case-sensitive", &val, NULL);
            g_value_set_boolean (value, val);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


void        moo_file_view_set_sort_case_sensitive       (MooFileView    *fileview,
                                                         gboolean        case_sensitive)
{
    g_return_if_fail (MOO_IS_FILE_VIEW (fileview));

    if (case_sensitive != fileview->priv->sort_case_sensitive)
    {
        fileview->priv->sort_case_sensitive = case_sensitive;
        g_object_set (fileview->priv->model,
                      "sort-case-sensitive",
                      case_sensitive, NULL);
        g_object_notify (G_OBJECT (fileview), "sort-case-sensitive");
    }
}


/*****************************************************************************/
/* Filters
 */

static void         moo_file_view_set_filter_mgr    (MooFileView    *fileview,
                                                     MooFilterMgr   *mgr)
{
    if (!mgr)
    {
        mgr = moo_filter_mgr_new ();
        moo_file_view_set_filter_mgr (fileview, mgr);
        g_object_unref (mgr);
        g_object_notify (G_OBJECT (fileview), "filter-mgr");
        return;
    }

    if (mgr == fileview->priv->filter_mgr)
        return;

    if (fileview->priv->filter_mgr)
        g_object_unref (fileview->priv->filter_mgr);
    fileview->priv->filter_mgr = g_object_ref (mgr);

    init_filter_combo (fileview);
    g_object_notify (G_OBJECT (fileview), "filter-mgr");
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
    MooFilterMgr *mgr = fileview->priv->filter_mgr;
    GtkFileFilter *filter;

    block_filter_signals (fileview);

    moo_filter_mgr_init_filter_combo (mgr, fileview->priv->filter_combo,
                                      "MooFileView");
    if (fileview->priv->current_filter)
        g_object_unref (fileview->priv->current_filter);
    fileview->priv->current_filter = NULL;
    fileview->priv->use_current_filter = FALSE;
    gtk_toggle_button_set_active (fileview->priv->filter_button, FALSE);
    gtk_entry_set_text (fileview->priv->filter_entry, "");

    unblock_filter_signals (fileview);

    filter = moo_filter_mgr_get_last_filter (mgr, "MooFileView");

    if (filter)
        fileview_set_filter (fileview, filter);
}


static void         filter_button_toggled   (MooFileView    *fileview)
{
    gboolean active =
            gtk_toggle_button_get_active (fileview->priv->filter_button);

    if (active == fileview->priv->use_current_filter)
        return;

    /* TODO check entry content */
    fileview_set_use_filter (fileview, active, TRUE);
    focus_to_file_view (fileview);
}


static void         filter_combo_changed    (MooFileView    *fileview)
{
    GtkTreeIter iter;
    GtkFileFilter *filter;
    MooFilterMgr *mgr = fileview->priv->filter_mgr;
    MooCombo *combo = fileview->priv->filter_combo;

    if (!moo_combo_get_active_iter (combo, &iter))
        return;

    filter = moo_filter_mgr_get_filter (mgr, &iter, "MooFileView");
    g_return_if_fail (filter != NULL);
    moo_filter_mgr_set_last_filter (mgr, &iter, "MooFileView");

    fileview_set_filter (fileview, filter);
    focus_to_file_view (fileview);
}


static void         filter_entry_activate   (MooFileView    *fileview)
{
    const char *text;
    GtkFileFilter *filter;
    MooFilterMgr *mgr = fileview->priv->filter_mgr;

    text = gtk_entry_get_text (fileview->priv->filter_entry);

    if (text && text[0])
        filter = moo_filter_mgr_new_user_filter (mgr, text, "MooFileView");
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

    null_filter = moo_filter_mgr_get_null_filter (fileview->priv->filter_mgr);
    g_return_if_fail (null_filter != NULL);

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


/*****************************************************************************/
/* File manager functionality
 */

static GList   *file_view_get_selected_files    (MooFileView    *fileview)
{
    GList *paths, *files, *l;
    paths = file_view_get_selected_rows (fileview);

    for (files = NULL, l = paths; l != NULL; l = l->next)
    {
        GtkTreeIter iter;
        MooFile *file = NULL;
        gtk_tree_model_get_iter (fileview->priv->filter_model, &iter, l->data);
        gtk_tree_model_get (fileview->priv->filter_model, &iter,
                            COLUMN_FILE, &file, -1);
        if (file)
            files = g_list_prepend (files, file);
    }

    g_list_foreach (paths, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (paths);
    return files;
}


static void     file_view_delete_selected       (MooFileView    *fileview)
{
    GError *error = NULL;
    GList *files, *l;
    gboolean one, folder;
    char *message, *path;
    GtkWidget *parent, *dialog;
    GtkWindow *parent_window;
    int response;

    if (!fileview->priv->current_dir)
        return;

    files = file_view_get_selected_files (fileview);
    if (!files)
        return;

    one = (files->next == NULL);
    if (one)
        folder = MOO_FILE_IS_DIR (files->data);

    if (one)
    {
        if (folder)
            message = g_strdup_printf ("Delete folder %s and all its content?",
                                       moo_file_display_name (files->data));
        else
            message = g_strdup_printf ("Delete file %s?",
                                       moo_file_display_name (files->data));
    }
    else
    {
        message = g_strdup ("Delete selected files?");
    }

    parent = gtk_widget_get_toplevel (GTK_WIDGET (fileview));
    parent_window = GTK_IS_WINDOW (parent) ? GTK_WINDOW (parent) : NULL;

    dialog = gtk_message_dialog_new (parent_window,
                                     GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_WARNING,
                                     GTK_BUTTONS_NONE,
                                     "%s", message);

    gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                            GTK_STOCK_DELETE, GTK_RESPONSE_OK, NULL);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);

    response = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    if (response == GTK_RESPONSE_OK)
    {
        for (l = files; l != NULL; l = l->next)
        {
            path = g_build_filename (moo_folder_get_path (fileview->priv->current_dir),
                                     moo_file_name (l->data), NULL);

            if (!moo_file_system_delete_file (fileview->priv->file_system, path, TRUE, &error))
            {
                dialog = gtk_message_dialog_new (parent_window,
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_ERROR,
                                                 GTK_BUTTONS_NONE,
                                                 "Could not delete %s '%s'",
                                                 MOO_FILE_IS_DIR (l->data) ? "folder" : "file",
                                                 path);
                if (error)
                {
                    gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                            "%s", error->message);
                    g_error_free (error);
                }

                gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                                        GTK_STOCK_CLOSE, GTK_RESPONSE_CANCEL, NULL);
                gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);
                gtk_dialog_run (GTK_DIALOG (dialog));
                gtk_widget_destroy (dialog);
            }

            g_free (path);
        }
    }

    g_list_foreach (files, (GFunc) moo_file_unref, NULL);
    g_list_free (files);
}


static void file_view_create_folder         (MooFileView    *fileview)
{
    GError *error = NULL;
    char *path, *name;

    if (!fileview->priv->current_dir)
        return;

    name = moo_create_folder_dialog (GTK_WIDGET (fileview),
                                     fileview->priv->current_dir);

    if (!name || !name[0])
    {
        g_free (name);
        return;
    }

    path = moo_file_system_make_path (fileview->priv->file_system,
                                      moo_folder_get_path (fileview->priv->current_dir),
                                      name, &error);

    if (!path)
    {
        g_message ("%s: could not make path for '%s'", G_STRLOC, name);

        if (error)
        {
            g_message ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }

        goto out;
    }

    if (!moo_file_system_create_folder (fileview->priv->file_system, path, &error))
    {
        g_message ("%s: could not create folder '%s'", G_STRLOC, name);

        if (error)
        {
            g_message ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }

        goto out;
    }

    select_display_name_in_idle (fileview, name);

out:
    g_free (path);
    g_free (name);
}


/* XXX */
static void file_view_properties_dialog     (MooFileView    *fileview,
                                             gboolean        current_dir)
{
    GtkWidget *dialog;
    GList *files;

    if (!fileview->priv->current_dir)
        return;

    files = file_view_get_selected_files (fileview);

    if (!files)
    {
        if (current_dir)
            g_print ("no files\n");
        return;
    }

    if (files->next)
    {
        g_print ("many files\n");
        g_list_foreach (files, (GFunc) moo_file_unref, NULL);
        g_list_free (files);
        return;
    }

    dialog = g_object_get_data (G_OBJECT (fileview),
                                "moo-file-view-properties-dialog");

    if (!dialog)
    {
        dialog = moo_file_props_dialog_new (GTK_WIDGET (fileview));
        gtk_object_sink (GTK_OBJECT (g_object_ref (dialog)));
        g_object_set_data_full (G_OBJECT (fileview),
                                "moo-file-view-properties-dialog",
                                dialog, g_object_unref);
        g_signal_connect (dialog, "delete-event",
                          G_CALLBACK (gtk_widget_hide_on_delete), NULL);
    }

    moo_file_props_dialog_set_file (dialog, files->data, fileview->priv->current_dir);
    gtk_window_present (GTK_WINDOW (dialog));

    g_list_foreach (files, (GFunc) moo_file_unref, NULL);
    g_list_free (files);
}


/*****************************************************************************/
/* Popup menu
 */

static gboolean really_destroy_menu (GtkWidget *menu)
{
    g_object_unref (menu);
    return FALSE;
}

static void destroy_menu    (GtkWidget *menu)
{
    g_idle_add ((GSourceFunc) really_destroy_menu, menu);
}

/* TODO */
static void menu_position_func  (G_GNUC_UNUSED GtkMenu *menu,
                                 gint       *x,
                                 gint       *y,
                                 gboolean   *push_in,
                                 gpointer    user_data)
{
    GdkWindow *window;

    struct {
        MooFileView *fileview;
        GList *rows;
    } *data = user_data;

    window = GTK_WIDGET(data->fileview)->window;
    gdk_window_get_origin (window, x, y);

    *push_in = TRUE;
}

static void         do_popup                (MooFileView    *fileview,
                                             GdkEventButton *event,
                                             GList          *selected)
{
    GtkWidget *menu;
    GList *files = NULL, *l;
    struct {
        MooFileView *fileview;
        GList *rows;
    } position_data = {fileview, selected};

    for (l = selected; l != NULL; l = l->next)
    {
        GtkTreeIter iter;
        MooFile *file = NULL;
        gtk_tree_model_get_iter (fileview->priv->filter_model, &iter, l->data);
        gtk_tree_model_get (fileview->priv->filter_model, &iter,
                            COLUMN_FILE, &file, -1);
        if (file)
            files = g_list_prepend (files, file);
    }

    menu = gtk_menu_new ();
    gtk_object_sink (GTK_OBJECT (g_object_ref (menu)));
    g_signal_connect (menu, "deactivate", G_CALLBACK (destroy_menu), NULL);

    g_signal_emit (fileview, signals[POPULATE_POPUP], 0, files, menu);

    if (event)
        gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
                        event->button, event->time);
    else
        gtk_menu_popup (GTK_MENU (menu), NULL, NULL,
                        menu_position_func,
                        &position_data, 0,
                        gtk_get_current_event_time ());

    g_list_foreach (files, (GFunc) moo_file_unref, NULL);
    g_list_free (files);
}


static gboolean     moo_file_view_popup_menu    (GtkWidget      *widget)
{
    GList *selected;
    MooFileView *fileview = MOO_FILE_VIEW (widget);

    selected = file_view_get_selected_rows (fileview);
    do_popup (fileview, NULL, selected);
    g_list_foreach (selected, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (selected);

    return TRUE;
}


static GtkWidget *create_menu_item  (const char        *icon_stock,
                                     const char        *label_text,
                                     GdkModifierType    mods,
                                     guint              key,
                                     GType              type)
{
    GtkWidget *item, *icon, *hbox, *accel_label, *label;
    char *accel_label_text;

    if (icon_stock)
    {
        icon = gtk_image_new_from_stock (icon_stock,
                                         GTK_ICON_SIZE_MENU);
        gtk_widget_show (icon);
        item = gtk_image_menu_item_new ();
        gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), icon);
    }
    else
    {
        if (type)
            item = g_object_new (type, NULL);
        else
            item = gtk_menu_item_new ();
    }

    hbox = gtk_hbox_new (FALSE, 6);
    gtk_container_add (GTK_CONTAINER (item), hbox);

    label = gtk_label_new (label_text);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

    if (key)
    {
        accel_label_text = gtk_accelerator_get_label (key, mods);
        if (accel_label_text)
        {
            accel_label = gtk_label_new (accel_label_text);
            gtk_misc_set_alignment (GTK_MISC (accel_label), 1.0, 0.5);
            gtk_box_pack_end (GTK_BOX (hbox), accel_label, FALSE, FALSE, 0);
            g_free (accel_label_text);
        }
    }

    gtk_widget_show_all (hbox);
    gtk_widget_show (item);
    return item;
}


static void create_goto_item (MooFileView       *fileview,
                              GtkMenu           *menu,
                              const char        *label_text,
                              const char        *stock_id,
                              const char        *signal,
                              guint              key,
                              GdkModifierType    mods)
{
    GtkWidget *item;

    item = create_menu_item (stock_id, label_text, mods, key, 0);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    g_object_set_data (G_OBJECT (item), "moo-file-view-signal",
                       (gpointer) signal);
    g_object_set_data (G_OBJECT (item), "moo-file-view-popup-item-id",
                       (gpointer) signal);
    g_signal_connect (item, "activate",
                      G_CALLBACK (goto_item_activated),
                      fileview);
}

static void add_separator_item (GtkMenu *menu)
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


static void create_view_submenu (MooFileView    *fileview,
                                 GtkMenu        *menu)
{
    GtkWidget *view_item, *item, *submenu;

    view_item = gtk_menu_item_new_with_label ("View");
    g_object_set_data (G_OBJECT (view_item),
                       "moo-file-view-popup-item-id",
                       (gpointer) "view");
    gtk_widget_show (view_item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), view_item);

    submenu = gtk_menu_new ();
    gtk_widget_show (submenu);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (view_item), submenu);

    item = create_menu_item (NULL, "Show Hidden Files",
                             GDK_MOD1_MASK | GDK_SHIFT_MASK,
                             GDK_h, GTK_TYPE_CHECK_MENU_ITEM);
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

    item = gtk_check_menu_item_new_with_label ("Case Sensitive Sort");
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item),
                                    fileview->priv->sort_case_sensitive);
    g_signal_connect (item, "toggled",
                      G_CALLBACK (sort_case_toggled), fileview);

    add_separator_item (GTK_MENU (submenu));
    create_view_type_items (fileview, submenu);
}


/* TODO */
static void create_new_folder_item          (MooFileView        *fileview,
                                             GtkMenu            *menu)
{
    GtkWidget *item;

    item = create_menu_item (GTK_STOCK_DIRECTORY,
                             "New Folder...",
                             0, 0, 0);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    g_object_set_data (G_OBJECT (item), "moo-file-view-popup-item-id",
                       (gpointer) "new-folder");
    if (fileview->priv->current_dir)
        g_signal_connect_swapped (item, "activate",
                                  G_CALLBACK (file_view_create_folder),
                                  fileview);
    else
        gtk_widget_set_sensitive (item, FALSE);
}


static void properties_item_activated   (MooFileView        *fileview)
{
    file_view_properties_dialog (fileview, TRUE);
}

static void create_properties_item      (MooFileView        *fileview,
                                         GList              *selected,
                                         GtkMenu            *menu)
{
    GtkWidget *item;

    item = create_menu_item (GTK_STOCK_PROPERTIES,
                             "Properties", GDK_MOD1_MASK,
                             GDK_Return, 0);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    g_object_set_data (G_OBJECT (item), "moo-file-view-popup-item-id",
                       (gpointer) "properties");

    if (selected && !selected->next)
        g_signal_connect_swapped (item, "activate",
                                  G_CALLBACK (properties_item_activated),
                                  fileview);
    else
        gtk_widget_set_sensitive (item, FALSE);
}


static void create_delete_item              (MooFileView        *fileview,
                                             GList              *selected,
                                             GtkMenu            *menu)
{
    GtkWidget *item;

    item = create_menu_item (GTK_STOCK_DELETE,
                             "Delete...", GDK_MOD1_MASK,
                             GDK_Delete, 0);
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    g_object_set_data (G_OBJECT (item), "moo-file-view-popup-item-id",
                       (gpointer) "delete");

    if (selected)
        g_signal_connect_swapped (item, "activate",
                                  G_CALLBACK (file_view_delete_selected),
                                  fileview);
    else
        gtk_widget_set_sensitive (item, FALSE);
}

static void moo_file_view_populate_popup    (MooFileView        *fileview,
                                             GList              *selected,
                                             GtkMenu            *menu)
{
    create_goto_item (fileview, menu, "Parent Folder",
                      GTK_STOCK_GO_UP, "go-up",
                      GDK_Up, GDK_MOD1_MASK);
    create_goto_item (fileview, menu, "Go Back",
                      GTK_STOCK_GO_BACK, "go-back",
                      GDK_Left, GDK_MOD1_MASK);
    create_goto_item (fileview, menu, "Go Forward",
                      GTK_STOCK_GO_FORWARD, "go-forward",
                      GDK_Right, GDK_MOD1_MASK);
    create_goto_item (fileview, menu, "Home Folder",
                      GTK_STOCK_HOME, "go-home",
                      GDK_Home, GDK_MOD1_MASK);

    add_separator_item (menu);

    create_new_folder_item (fileview, menu);

    create_delete_item (fileview, selected, menu);
    add_separator_item (menu);

    create_view_submenu (fileview, menu);
    add_separator_item (menu);

    create_properties_item (fileview, selected, menu);
}


static gboolean     tree_button_press       (GtkTreeView    *treeview,
                                             GdkEventButton *event,
                                             MooFileView    *fileview)
{
    GtkTreeSelection *selection;
    GtkTreePath *filter_path = NULL;
    GList *selected;

    if (event->button != 3)
        return FALSE;

    selection = gtk_tree_view_get_selection (treeview);

    if (gtk_tree_view_get_path_at_pos (treeview, event->x, event->y,
                                       &filter_path, NULL, NULL, NULL))
    {
        if (!gtk_tree_selection_path_is_selected (selection, filter_path))
        {
            gtk_tree_selection_unselect_all (selection);
            gtk_tree_view_set_cursor (treeview, filter_path,
                                      NULL, FALSE);
        }
    }
    else
    {
        gtk_tree_selection_unselect_all (selection);
    }

    selected = gtk_tree_selection_get_selected_rows (selection, NULL);
    do_popup (fileview, event, selected);
    gtk_tree_path_free (filter_path);
    g_list_foreach (selected, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (selected);

    return TRUE;
}


static gboolean     icon_button_press       (MooIconView    *iconview,
                                             GdkEventButton *event,
                                             MooFileView    *fileview)
{
    GtkTreePath *filter_path = NULL;
    GList *selected;

    if (event->button != 3)
        return FALSE;

    if (moo_icon_view_get_path_at_pos (iconview, event->x, event->y,
                                       &filter_path, NULL, NULL, NULL))
    {
        if (!moo_icon_view_path_is_selected (iconview, filter_path))
        {
            moo_icon_view_unselect_all (iconview);
            moo_icon_view_set_cursor (iconview, filter_path, FALSE);
        }
    }
    else
    {
        moo_icon_view_unselect_all (iconview);
    }

    selected = moo_icon_view_get_selected_rows (iconview);
    do_popup (fileview, event, selected);
    gtk_tree_path_free (filter_path);
    g_list_foreach (selected, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (selected);

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


static void         sort_case_toggled       (GtkWidget      *widget,
                                             MooFileView    *fileview)
{
    gboolean active = fileview->priv->sort_case_sensitive;
    g_object_get (G_OBJECT (widget), "active", &active, NULL);
    moo_file_view_set_sort_case_sensitive (fileview, active);
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


/*****************************************************************************/
/* Bookmarks menu
 */

static void         destroy_bookmarks_menu          (MooFileView    *fileview)
{
    if (fileview->priv->bookmarks_menu)
    {
        gtk_widget_destroy (GTK_WIDGET (fileview->priv->bookmarks_menu));
        g_object_unref (fileview->priv->bookmarks_menu);
        fileview->priv->bookmarks_menu = NULL;
    }
}


static void         moo_file_view_set_bookmark_mgr  (MooFileView    *fileview,
                                                     MooBookmarkMgr *mgr)
{
    if (!mgr)
    {
        mgr = moo_bookmark_mgr_new ();
        moo_file_view_set_bookmark_mgr (fileview, mgr);
        g_object_unref (mgr);
        return;
    }

    if (fileview->priv->bookmark_mgr == mgr)
        return;

    if (fileview->priv->bookmark_mgr)
    {
        g_signal_handlers_disconnect_by_func (fileview->priv->bookmark_mgr,
                                              (gpointer) destroy_bookmarks_menu,
                                              fileview);
        g_object_unref (fileview->priv->bookmark_mgr);
    }

    fileview->priv->bookmark_mgr = g_object_ref (mgr);
    g_signal_connect_swapped (fileview->priv->bookmark_mgr,
                              "changed",
                              G_CALLBACK (destroy_bookmarks_menu),
                              fileview);

    destroy_bookmarks_menu (fileview);

    g_object_set_data (G_OBJECT (fileview),
                       "moo-file-view-bookmarks-editor",
                       NULL);
    g_object_notify (G_OBJECT (fileview), "bookmark-mgr");
}


static void     bookmark_activated      (MooBookmark    *bookmark,
                                         MooFileView    *fileview)
{
    g_return_if_fail (bookmark != NULL && bookmark->path != NULL);
    moo_file_view_chdir (fileview, bookmark->path, NULL);
}


static void     add_bookmark            (MooFileView    *fileview)
{
    const char *path;
    char *display_path;
    MooBookmark *bookmark;

    g_return_if_fail (fileview->priv->current_dir != NULL);

    path = moo_folder_get_path (fileview->priv->current_dir);
    display_path = g_filename_display_name (path);
    bookmark = moo_bookmark_new (display_path, path,
                                 GTK_STOCK_DIRECTORY);

    moo_bookmark_mgr_add (fileview->priv->bookmark_mgr,
                          bookmark);

    moo_bookmark_free (bookmark);
    g_free (display_path);
}


static void     edit_bookmarks          (MooFileView    *fileview)
{
    GtkWidget *dialog, *parent;

    dialog = g_object_get_data (G_OBJECT (fileview),
                                "moo-file-view-bookmarks-editor");

    if (!dialog)
    {
        dialog = moo_bookmark_mgr_get_editor (fileview->priv->bookmark_mgr);
        gtk_object_sink (GTK_OBJECT (g_object_ref (dialog)));
        g_object_set_data_full (G_OBJECT (fileview),
                                "moo-file-view-bookmarks-editor",
                                dialog, g_object_unref);
        g_signal_connect (dialog, "delete-event",
                          G_CALLBACK (gtk_widget_hide_on_delete), NULL);
    }

    parent = gtk_widget_get_toplevel (GTK_WIDGET (fileview));
    if (GTK_IS_WINDOW (parent) && !GTK_WIDGET_VISIBLE (dialog))
        gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (parent));

    gtk_window_present (GTK_WINDOW (dialog));
}

static GtkMenu *create_bookmarks_menu   (MooFileView    *fileview)
{
    GtkWidget *item;
    GtkWidget *menu = gtk_menu_new ();

    if (!moo_bookmark_mgr_is_empty (fileview->priv->bookmark_mgr))
    {
        moo_bookmark_mgr_fill_menu (fileview->priv->bookmark_mgr, menu, -1,
                                    (MooBookmarkFunc) bookmark_activated,
                                    fileview);

        add_separator_item (GTK_MENU (menu));
    }

    item = create_menu_item (GTK_STOCK_ADD, "Add Bookmark", 0, 0, 0);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    g_signal_connect_swapped (item, "activate",
                              G_CALLBACK (add_bookmark),
                              fileview);

    item = create_menu_item (GTK_STOCK_EDIT, "Edit Bookmarks...", 0, 0, 0);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    g_signal_connect_swapped (item, "activate",
                              G_CALLBACK (edit_bookmarks),
                              fileview);

    return GTK_MENU (menu);
}

static void bookmarks_menu_position_func (GtkMenu           *menu,
                                          int               *x,
                                          int               *y,
                                          gboolean          *push_in,
                                          GtkToggleButton   *button)
{
    GtkRequisition req, menu_req;

    gdk_window_get_origin (GTK_BUTTON (button)->event_window, x, y);
    gtk_widget_size_request (GTK_WIDGET (button), &req);
    gtk_widget_size_request (GTK_WIDGET (menu), &menu_req);

    *x += GTK_WIDGET(button)->allocation.width - req.width;
    *y += GTK_WIDGET(button)->allocation.height;

    *push_in = TRUE;
}

static void toggle_bookmarks_button (GtkToggleButton *button)
{
    gtk_toggle_button_set_active (button, FALSE);
}

static void         boomarks_button_toggled (GtkToggleButton *button,
                                             MooFileView    *fileview)
{
    if (!gtk_toggle_button_get_active (button))
        return;

    if (!fileview->priv->bookmarks_menu)
    {
        fileview->priv->bookmarks_menu = create_bookmarks_menu (fileview);
        gtk_object_sink (GTK_OBJECT (g_object_ref (fileview->priv->bookmarks_menu)));
        g_signal_connect_swapped (fileview->priv->bookmarks_menu, "deactivate",
                                  G_CALLBACK (toggle_bookmarks_button), button);
    }

    gtk_menu_popup (fileview->priv->bookmarks_menu, NULL, NULL,
                    (GtkMenuPositionFunc) bookmarks_menu_position_func,
                    button, 0, gtk_get_current_event_time ());
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
        moo_icon_view_unselect_all (fileview->priv->iconview);

        if (path)
        {
            moo_icon_view_set_cursor (fileview->priv->iconview, path, FALSE);
            moo_icon_view_scroll_to_cell (fileview->priv->iconview, path);
        }
    }

    gtk_tree_path_free (path);
}


static void file_view_select_iter           (MooFileView    *fileview,
                                             GtkTreeIter    *iter)
{
    if (iter)
    {
        GtkTreeIter filter_iter;
        GtkTreePath *filter_path, *path;

        path = gtk_tree_model_get_path (fileview->priv->model, iter);
        g_return_if_fail (path != NULL);

        filter_path = gtk_tree_model_filter_convert_child_path_to_path (
                GTK_TREE_MODEL_FILTER (fileview->priv->filter_model), path);

        if (!filter_path)
        {
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
        file_view_move_selection (fileview, NULL);
    }
}


void        moo_file_view_select_name       (MooFileView    *fileview,
                                             const char     *name)
{
    GtkTreeIter iter;

    if (name && moo_folder_model_get_iter_by_name (
        MOO_FOLDER_MODEL (fileview->priv->model), name, &iter))
    {
        file_view_select_iter (fileview, &iter);
    }
    else
    {
        file_view_select_iter (fileview, NULL);
    }
}


void        moo_file_view_select_display_name   (MooFileView    *fileview,
                                                 const char     *name)
{
    GtkTreeIter iter;

    if (name && moo_folder_model_get_iter_by_display_name (
        MOO_FOLDER_MODEL (fileview->priv->model), name, &iter))
    {
        file_view_select_iter (fileview, &iter);
    }
    else
    {
        file_view_select_iter (fileview, NULL);
    }
}


static gboolean do_select_display_name      (MooFileView        *fileview)
{
    GtkTreeIter iter;
    const char *name;

    fileview->priv->select_file_idle = 0;

    name = g_object_get_data (G_OBJECT (fileview),
                              "moo-file-view-select-display-name");
    g_return_val_if_fail (name != NULL, FALSE);

    if (moo_folder_model_get_iter_by_display_name (
        MOO_FOLDER_MODEL (fileview->priv->model), name, &iter))
    {
        file_view_select_iter (fileview, &iter);
    }

    return FALSE;
}

/* TODO: this */
static void select_display_name_in_idle     (MooFileView        *fileview,
                                             const char         *display_name)
{
    GtkTreeIter iter;

    g_return_if_fail (display_name != NULL);

    if (fileview->priv->select_file_idle)
        g_source_remove (fileview->priv->select_file_idle);
    fileview->priv->select_file_idle = 0;

    if (moo_folder_model_get_iter_by_display_name (
        MOO_FOLDER_MODEL (fileview->priv->model), display_name, &iter))
    {
        file_view_select_iter (fileview, &iter);
    }
    else
    {
        g_object_set_data_full (G_OBJECT (fileview),
                                "moo-file-view-select-display-name",
                                g_strdup (display_name), g_free);
        g_idle_add_full (G_PRIORITY_LOW,
                         (GSourceFunc) do_select_display_name,
                         fileview, NULL);
    }
}


/* returns path in the fileview->priv->filter_model */
static GtkTreePath *file_view_get_selected      (MooFileView    *fileview)
{
    GList *rows;
    GtkTreePath *path;

    rows = file_view_get_selected_rows (fileview);

    if (!rows)
        return NULL;

    if (rows->next)
        g_warning ("%s: more than one row is selected", G_STRLOC);

    path = gtk_tree_path_copy (rows->data);
    g_list_foreach (rows, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (rows);

    return path;
}


static GList    *file_view_get_selected_rows    (MooFileView    *fileview)
{
    if (fileview->priv->view_type == MOO_FILE_VIEW_LIST)
    {
        GtkTreeSelection *selection;
        GtkTreeView *treeview;

        treeview = fileview->priv->treeview;
        selection = gtk_tree_view_get_selection (treeview);

        return gtk_tree_selection_get_selected_rows (selection, NULL);
    }
    else
    {
        return moo_icon_view_get_selected_rows (fileview->priv->iconview);
    }
}


static const char  *get_selected_display_name   (MooFileView    *fileview)
{
    GtkTreeModel *model = fileview->priv->filter_model;
    GtkTreePath *filter_path;
    GtkTreeIter filter_iter;
    const char *name;
    MooFile *file = NULL;

    filter_path = file_view_get_selected (fileview);

    if (!filter_path)
        return NULL;

    gtk_tree_model_get_iter (model, &filter_iter, filter_path);
    gtk_tree_path_free (filter_path);

    gtk_tree_model_get (model, &filter_iter, COLUMN_FILE, &file, -1);
    g_return_val_if_fail (file != NULL, NULL);
    name = moo_file_display_name (file);
    moo_file_unref (file);

    return name;
}


/****************************************************************************/
/* Path entry
 */

enum {
    ENTRY_STATE_NONE        = 0,
    ENTRY_STATE_TYPEAHEAD   = 1,
    ENTRY_STATE_COMPLETION  = 2
};

static void     typeahead_create        (MooFileView    *fileview);
static void     typeahead_destroy       (MooFileView    *fileview);
static void     typeahead_try           (MooFileView    *fileview,
                                         gboolean        need_to_refilter);
static void     typeahead_tab_key       (MooFileView    *fileview);
static gboolean typeahead_stop_tab_cycle(MooFileView    *fileview);

static void     entry_changed           (GtkEntry       *entry,
                                         MooFileView    *fileview);
static gboolean entry_key_press         (GtkEntry       *entry,
                                         GdkEventKey    *event,
                                         MooFileView    *fileview);
static gboolean entry_focus_out         (GtkWidget      *entry,
                                         GdkEventFocus  *event,
                                         MooFileView    *fileview);
static void     entry_activate          (GtkEntry       *entry,
                                         MooFileView    *fileview);

static gboolean entry_stop_tab_cycle    (MooFileView    *fileview);
static gboolean entry_tab_key           (GtkEntry       *entry,
                                         MooFileView    *fileview);

static gboolean looks_like_path         (const char     *text);


static void         path_entry_init         (MooFileView    *fileview)
{
    GtkEntry *entry = fileview->priv->entry;

    /* XXX after? */
    g_signal_connect (entry, "changed",
                      G_CALLBACK (entry_changed), fileview);
    g_signal_connect (entry, "key-press-event",
                      G_CALLBACK (entry_key_press), fileview);
    g_signal_connect (entry, "focus-out-event",
                      G_CALLBACK (entry_focus_out), fileview);
    g_signal_connect (entry, "activate",
                      G_CALLBACK (entry_activate), fileview);

    typeahead_create (fileview);
}


static void         path_entry_deinit       (MooFileView    *fileview)
{
    typeahead_destroy (fileview);
}


static void     entry_changed       (GtkEntry       *entry,
                                     MooFileView    *fileview)
{
    gboolean need_to_refilter = FALSE;

    const char *text = gtk_entry_get_text (entry);

    /*
    First, decide what's going on (doesn't look like something can be
    going on, but, anyway).
    Then:
    1) If text typed in is a beginning of some file in the list,
    select that file (like treeview interactive search).
    2) If text is a beginning of some hidden files (but not filtered
    out, those are always ignored), show those hidden files and
    select first of them.
    3) Otherwise, parse text as <path>/<file> (<file> may be empty),
    and do the entry completion stuff.

    Tab completion doesn't interfere with this code - tab completion sets
    entry text while this handler is blocked.
    */

    /* Check if some file was shown temporarily, and hide it */
    if (fileview->priv->temp_visible)
    {
        g_string_free (fileview->priv->temp_visible, TRUE);
        fileview->priv->temp_visible = NULL;
        need_to_refilter = TRUE;
    }

    if (!text[0])
    {
        if (need_to_refilter)
            gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER
                    (fileview->priv->filter_model));

        file_view_move_selection (fileview, NULL);
        return;
    }

    /* TODO take ~file into account */
    /* If entered text looks like path, do completion stuff */
    if (looks_like_path (text))
    {
        if (need_to_refilter)
            gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER
                    (fileview->priv->filter_model));

        file_view_move_selection (fileview, NULL);
        fileview->priv->entry_state = ENTRY_STATE_COMPLETION;
        /* XXX call complete() or something, for automatic popup */
        return;
    }
    else
    {
        fileview->priv->entry_state = ENTRY_STATE_TYPEAHEAD;
        return typeahead_try (fileview, need_to_refilter);
    }
}


static gboolean entry_key_press     (GtkEntry       *entry,
                                     GdkEventKey    *event,
                                     MooFileView    *fileview)
{
    GtkWidget *filewidget;
    const char *name;

    if (event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK))
        return FALSE;

    if (event->keyval != GDK_Tab)
    {
        if (entry_stop_tab_cycle (fileview))
            g_signal_emit_by_name (entry, "changed");
    }

    switch (event->keyval)
    {
        case GDK_Escape:
            stop_path_entry (fileview, TRUE);
            return TRUE;

        case GDK_Up:
        case GDK_KP_Up:
        case GDK_Down:
        case GDK_KP_Down:
            filewidget = get_file_view_widget (fileview);
            GTK_WIDGET_CLASS(G_OBJECT_GET_CLASS (filewidget))->
                    key_press_event (filewidget, event);
            name = get_selected_display_name (fileview);
            g_return_val_if_fail (name != NULL, TRUE);
            path_entry_set_text (fileview, name);
            gtk_editable_set_position (GTK_EDITABLE (entry), -1);
            return TRUE;

        case GDK_Tab:
            return entry_tab_key (entry, fileview);

        default:
            return FALSE;
    }
}


static gboolean entry_stop_tab_cycle    (MooFileView    *fileview)
{
    switch (fileview->priv->entry_state)
    {
        case ENTRY_STATE_TYPEAHEAD:
            return typeahead_stop_tab_cycle (fileview);
        case ENTRY_STATE_COMPLETION:
            fileview->priv->entry_state = 0;
            return FALSE;
        default:
            return FALSE;
    }
}


static gboolean entry_tab_key       (GtkEntry       *entry,
                                     MooFileView    *fileview)
{
    const char *text;

    if (!fileview->priv->entry_state)
    {
        text = gtk_entry_get_text (entry);

        if (text[0])
        {
            g_signal_emit_by_name (entry, "changed");
            g_return_val_if_fail (fileview->priv->entry_state != 0, FALSE);
            return entry_tab_key (entry, fileview);
        }
        else
        {
            return FALSE;
        }
    }

    if (fileview->priv->entry_state == ENTRY_STATE_TYPEAHEAD)
    {
        typeahead_tab_key (fileview);
        return TRUE;
    }

    return FALSE;
}


static void         path_entry_set_text     (MooFileView    *fileview,
                                             const char     *text)
{
    GtkEntry *entry = fileview->priv->entry;
    g_signal_handlers_block_by_func (entry, (gpointer) entry_changed,
                                     fileview);
    gtk_entry_set_text (entry, text);
    gtk_editable_set_position (GTK_EDITABLE (entry), -1);
    g_signal_handlers_unblock_by_func (entry, (gpointer) entry_changed,
                                       fileview);
}


static void         path_entry_delete_to_cursor (MooFileView *fileview)
{
    GtkEditable *entry = GTK_EDITABLE (fileview->priv->entry);
    if (gtk_widget_is_focus (GTK_WIDGET (entry)))
        gtk_editable_delete_text (entry, 0,
                                  gtk_editable_get_position (entry));
}


static void     entry_activate      (GtkEntry       *entry,
                                     MooFileView    *fileview)
{
    char *filename = NULL;
    GtkTreePath *selected;

    selected = file_view_get_selected (fileview);

    if (selected)
    {
        GtkTreeIter iter;
        MooFile *file = NULL;
        gtk_tree_model_get_iter (fileview->priv->filter_model, &iter, selected);
        gtk_tree_model_get (fileview->priv->filter_model, &iter, COLUMN_FILE, &file, -1);
        gtk_tree_path_free (selected);
        g_return_if_fail (file != NULL);

        /* XXX display_name() */
        filename = g_strdup (moo_file_display_name (file));
        moo_file_unref (file);
    }
    else
    {
        filename = g_strdup (gtk_entry_get_text (entry));
    }

    stop_path_entry (fileview, TRUE);
    file_view_activate_filename (fileview, filename);
    g_free (filename);
}


#if 0
#define PRINT_KEY_EVENT(event)                                      \
    g_print ("%s%s%s%s\n",                                          \
             event->state & GDK_SHIFT_MASK ? "<Shift>" : "",        \
             event->state & GDK_CONTROL_MASK ? "<Control>" : "",    \
             event->state & GDK_MOD1_MASK ? "<Alt>" : "",           \
             gdk_keyval_name (event->keyval))
#else
#define PRINT_KEY_EVENT(event)
#endif


static gboolean     moo_file_view_key_press     (GtkWidget      *widget,
                                                 GdkEventKey    *event,
                                                 MooFileView    *fileview)
{
    if (fileview->priv->entry_state)
    {
        g_warning ("%s: something wrong", G_STRLOC);
        stop_path_entry (fileview, FALSE);
        return FALSE;
    }

    /* return immediately if event doesn't look like text typed in */

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
        GdkEvent *copy;
        GtkWidget *entry = GTK_WIDGET (fileview->priv->entry);

        g_return_val_if_fail (event != NULL, FALSE);
        g_return_val_if_fail (GTK_WIDGET_REALIZED (entry), FALSE);

        copy = gdk_event_copy ((GdkEvent*) event);
        g_object_unref (copy->key.window);
        copy->key.window = g_object_ref (entry->window);

        gtk_widget_grab_focus (entry);

        path_entry_set_text (fileview, "");
        gtk_widget_event (entry, copy);

        gdk_event_free (copy);
        return TRUE;
    }

    return FALSE;
}


static gboolean entry_focus_out     (GtkWidget      *entry,
                                     G_GNUC_UNUSED GdkEventFocus *event,
                                     MooFileView    *fileview)
{
    /* focus may be lost due to switching to other window, do nothing then */
    if (!gtk_widget_is_focus (entry))
        stop_path_entry (fileview, TRUE);
    return FALSE;
}


static void     stop_path_entry     (MooFileView    *fileview,
                                     gboolean        focus_file_list)
{
    char *text;

    typeahead_stop_tab_cycle (fileview);

    fileview->priv->entry_state = 0;

    if (fileview->priv->current_dir)
        text = g_filename_display_name (moo_folder_get_path (fileview->priv->current_dir));
    else
        text = g_strdup ("");

    path_entry_set_text (fileview, text);

    if (focus_file_list)
        focus_to_file_view (fileview);

    g_free (text);
}


/* WIN32_XXX */
static void         file_view_activate_filename (MooFileView    *fileview,
                                                 const char     *display_name)
{
    GError *error = NULL;
    char *dirname, *basename;
    char *path = NULL;
    const char *current_dir = NULL;

    if (fileview->priv->current_dir)
        current_dir = moo_folder_get_path (fileview->priv->current_dir);

    path = moo_file_system_get_absolute_path (fileview->priv->file_system,
                                              display_name, current_dir);

    if (!path || !g_file_test (path, G_FILE_TEST_EXISTS))
    {
        g_free (path);
        return;
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

    dirname = g_path_get_dirname (path);
    basename = g_path_get_basename (path);

    if (!dirname || !basename)
    {
        g_free (path);
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

    moo_file_view_select_name (fileview, basename);
    g_signal_emit (fileview, signals[ACTIVATE], 0, path);

    g_free (path);
    g_free (dirname);
    g_free (basename);
}


static gboolean looks_like_path         (const char     *text)
{
    if (strchr (text, '/'))
        return TRUE;
#ifdef __WIN32__
    else if (strchr (text, '\\'))
        return TRUE;
#endif
    else
        return FALSE;
}


/*********************************************************************/
/* Typeahead
 */

struct _Typeahead {
    MooFileView *fileview;
    GtkEntry *entry;
    GString *matched_prefix;
    int matched_prefix_char_len;
    TextFuncs text_funcs;
    gboolean case_sensitive;
};

static gboolean typeahead_find_match_visible    (MooFileView    *fileview,
                                                 const char     *text,
                                                 GtkTreeIter    *filter_iter,
                                                 gboolean        exact_match);
static gboolean typeahead_find_match_hidden     (MooFileView    *fileview,
                                                 const char     *text,
                                                 GtkTreeIter    *iter,
                                                 gboolean        exact_match);


static void     typeahead_try       (MooFileView    *fileview,
                                     gboolean        need_to_refilter)
{
    const char *text;
    Typeahead *stuff = fileview->priv->typeahead;
    GtkEntry *entry = stuff->entry;
    GtkTreeIter filter_iter, iter;

    g_assert (fileview->priv->entry_state == ENTRY_STATE_TYPEAHEAD);

    if (stuff->matched_prefix)
    {
        g_string_free (stuff->matched_prefix, TRUE);
        stuff->matched_prefix = NULL;
    }

    text = gtk_entry_get_text (entry);
    g_assert (text[0] != 0);

    /* TODO windows */

    if (fileview->priv->show_hidden_files || text[0] != '.' || !text[1])
    {
        if (need_to_refilter)
            gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER
                    (fileview->priv->filter_model));

        if (typeahead_find_match_visible (fileview, text, &filter_iter, FALSE))
            file_view_move_selection (fileview, &filter_iter);
        else
            file_view_move_selection (fileview, NULL);

        return;
    }

    /* check if partial name of hidden file was typed in */
    if (typeahead_find_match_hidden (fileview, text, &iter, FALSE))
    {
        fileview->priv->temp_visible = g_string_new (text);
        gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER
                (fileview->priv->filter_model));

        gtk_tree_model_filter_convert_child_iter_to_iter (
                GTK_TREE_MODEL_FILTER (fileview->priv->filter_model),
        &filter_iter, &iter);
        file_view_move_selection (fileview, &filter_iter);
    }
    else
    {
        if (need_to_refilter)
            gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER
                    (fileview->priv->filter_model));

        file_view_move_selection (fileview, NULL);
    }
}


static void     typeahead_tab_key       (MooFileView    *fileview)
{
    const char *text;
    Typeahead *stuff = fileview->priv->typeahead;
    GtkEntry *entry = stuff->entry;
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreePath *path;
    MooFile *file = NULL;
    const char *name;

    g_assert (fileview->priv->entry_state == ENTRY_STATE_TYPEAHEAD);

    model = fileview->priv->filter_model;

    if (!gtk_tree_model_get_iter_first (model, &iter))
        return;

    /* see if it's cycling over matched names */
    if (stuff->matched_prefix)
    {
        gboolean found = FALSE;

        path = file_view_get_selected (fileview);

        /* check if there is next name in the list to jump to */
        if (path && gtk_tree_model_get_iter (model, &iter, path) &&
            gtk_tree_model_iter_next (model, &iter))
        {
            found = model_find_next_match (model, &iter,
                                           stuff->matched_prefix->str,
                                           stuff->matched_prefix->len,
                                           &stuff->text_funcs,
                                           FALSE);
        }

        /* if nothing found, start cycling again */
        if (!found)
            found = typeahead_find_match_visible (fileview,
                                                  stuff->matched_prefix->str,
                                                  &iter, FALSE);

        gtk_tree_path_free (path);

        if (!found)
            goto error;
        else
            file_view_move_selection (fileview, &iter);
    }
    else
    {
        gboolean unique;

        text = gtk_entry_get_text (entry);

        if (!text[0])
            return;

        stuff->matched_prefix =
                model_find_max_prefix (fileview->priv->filter_model,
                                       text, &stuff->text_funcs, &unique, &iter);

        if (!stuff->matched_prefix)
            return;

        stuff->matched_prefix_char_len =
                g_utf8_strlen (stuff->matched_prefix->str, stuff->matched_prefix->len);

        /* if match is unique and it's a directory, append a slash */
        if (unique)
        {
            file = NULL;
            gtk_tree_model_get (model, &iter, COLUMN_FILE, &file, -1);

            if (!file)
                goto error;

            name = moo_file_display_name (file);

//             if (!file || stuff->strcmp_func (stuff->matched_prefix->str, file))
//                 goto error;

            if (MOO_FILE_IS_DIR (file))
            {
                char *new_text = g_strdup_printf ("%s%c", name, G_DIR_SEPARATOR);
                g_string_free (stuff->matched_prefix, TRUE);
                stuff->matched_prefix = NULL;
                path_entry_set_text (fileview, new_text);
                g_free (new_text);
                moo_file_unref (file);
                g_signal_emit_by_name (entry, "changed");
                return;
            }

            moo_file_unref (file);
            file = NULL;
        }

        path = file_view_get_selected (fileview);

        if (!path && !typeahead_find_match_visible (fileview, stuff->matched_prefix->str, &iter, FALSE))
            goto error;

        if (path)
            gtk_tree_model_get_iter (model, &iter, path);

        gtk_tree_path_free (path);
    }

    gtk_tree_model_get (model, &iter, COLUMN_FILE, &file, -1);

    if (!file)
        goto error;

//     if (stuff->strncmp_func (stuff->matched_prefix->str, file, stuff->matched_prefix->len))
//         goto error;

    name = moo_file_display_name (file);
    path_entry_set_text (fileview, name);
    gtk_editable_select_region (GTK_EDITABLE (entry),
                                stuff->matched_prefix_char_len,
                                -1);

    moo_file_unref (file);
    return;

error:
    if (stuff->matched_prefix)
        g_string_free (stuff->matched_prefix, TRUE);
    stuff->matched_prefix = NULL;
    moo_file_unref (file);
    g_return_if_reached ();
}


static gboolean typeahead_stop_tab_cycle(MooFileView    *fileview)
{
    Typeahead *stuff = fileview->priv->typeahead;

    if (stuff->matched_prefix)
    {
        g_string_free (stuff->matched_prefix, TRUE);
        stuff->matched_prefix = NULL;
        stuff->matched_prefix_char_len = 0;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


static void     typeahead_create        (MooFileView    *fileview)
{
    Typeahead *stuff = g_new0 (Typeahead, 1);

    stuff->fileview = fileview;
    stuff->entry = fileview->priv->entry;
    stuff->case_sensitive = fileview->priv->typeahead_case_sensitive;

    if (stuff->case_sensitive)
    {
        stuff->text_funcs.strcmp_func = strcmp_func;
        stuff->text_funcs.strncmp_func = strncmp_func;
        stuff->text_funcs.normalize_func = normalize_func;
    }
    else
    {
        stuff->text_funcs.strcmp_func = case_strcmp_func;
        stuff->text_funcs.strncmp_func = case_strncmp_func;
        stuff->text_funcs.normalize_func = case_normalize_func;
    }

    fileview->priv->typeahead = stuff;
}


static void     typeahead_destroy       (MooFileView    *fileview)
{
    Typeahead *stuff = fileview->priv->typeahead;
    if (stuff->matched_prefix)
        g_string_free (stuff->matched_prefix, TRUE);
    g_free (stuff);
    fileview->priv->typeahead = NULL;
}


void        moo_file_view_set_typeahead_case_sensitive  (MooFileView    *fileview,
                                                         gboolean        case_sensitive)
{
    g_return_if_fail (MOO_IS_FILE_VIEW (fileview));

    if (case_sensitive != fileview->priv->typeahead_case_sensitive)
    {
        Typeahead *stuff = fileview->priv->typeahead;

        fileview->priv->typeahead_case_sensitive = case_sensitive;
        stuff->case_sensitive = case_sensitive;

        if (case_sensitive)
        {
            stuff->text_funcs.strcmp_func = strcmp_func;
            stuff->text_funcs.strncmp_func = strncmp_func;
            stuff->text_funcs.normalize_func = normalize_func;
        }
        else
        {
            stuff->text_funcs.strcmp_func = case_strcmp_func;
            stuff->text_funcs.strncmp_func = case_strncmp_func;
            stuff->text_funcs.normalize_func = case_normalize_func;
        }

        g_object_notify (G_OBJECT (fileview), "typeahead-case-sensitive");
    }
}


static gboolean typeahead_find_match_visible    (MooFileView    *fileview,
                                                 const char     *text,
                                                 GtkTreeIter    *filter_iter,
                                                 gboolean        exact_match)
{
    guint len;

    g_return_val_if_fail (text && text[0], FALSE);

    if (!gtk_tree_model_get_iter_first (fileview->priv->filter_model, filter_iter))
        return FALSE;

    len = strlen (text);

    return model_find_next_match (fileview->priv->filter_model,
                                  filter_iter, text, len,
                                  &fileview->priv->typeahead->text_funcs,
                                  exact_match);
}


static gboolean typeahead_find_match_hidden     (MooFileView    *fileview,
                                                 const char     *text,
                                                 GtkTreeIter    *iter,
                                                 gboolean        exact)
{
    guint len;
    GtkTreeModel *model = fileview->priv->model;

    g_return_val_if_fail (text && text[0], FALSE);

    if (!gtk_tree_model_get_iter_first (model, iter))
        return FALSE;

    len = strlen (text);

    while (TRUE)
    {
        MooFile *file = NULL;

        if (!model_find_next_match (model, iter, text, len,
                                    &fileview->priv->typeahead->text_funcs,
                                    exact))
            return FALSE;

        gtk_tree_model_get (model, iter, COLUMN_FILE, &file, -1);

        if (file && moo_file_view_check_visible (fileview, file, TRUE, TRUE))
        {
            moo_file_unref (file);
            return TRUE;
        }

        moo_file_unref (file);

        if (!gtk_tree_model_iter_next (model, iter))
            return FALSE;
    }
}
