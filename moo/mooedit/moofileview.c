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

struct _MooFileViewPrivate {
    GtkListStore    *store;
    GHashTable      *files; /* File* -> GtkTreeRowReference* */
    gboolean         need_to_clear; /* next add_file_in_idle will clear list store */
    gboolean         process_changes_now; /* first bunch of files in a dir, should be
                                             put into liststore immediately */

    GHashTable      *files_to_add;  /* File* */
    GHashTable      *files_to_remove;
    GHashTable      *files_to_change;
    guint            add_files_idle;
    guint            remove_files_idle;
    guint            change_files_idle;

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
    char            *name_to_select;

    MooEditFileMgr  *mgr;
    GtkToggleButton *filter_button;
    GtkComboBox     *filter_combo;
    GtkEntry        *filter_entry;
    GtkFileFilter   *current_filter;
    gboolean         use_current_filter;

    GtkEntry        *search_entry;
    gboolean         in_search;
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

static gboolean     moo_file_view_chdir_real(MooFileView    *fileview,
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
static int          tree_compare_func       (GtkTreeModel       *model,
                                             GtkTreeIter        *a,
                                             GtkTreeIter        *b);

static void         folder_deleted          (MooFolder      *folder,
                                             MooFileView    *fileview);
static void         files_added             (MooFolder      *folder,
                                             GSList         *files,
                                             MooFileView    *fileview);
static void         files_changed           (MooFolder      *folder,
                                             GSList         *files,
                                             MooFileView    *fileview);
static void         files_removed           (MooFolder      *folder,
                                             GSList         *files,
                                             MooFileView    *fileview);
static void         connect_folder          (MooFileView    *fileview);
static void         disconnect_folder       (MooFileView    *fileview);

static void         fileview_add_files      (MooFileView    *fileview,
                                             GSList         *files);
static void         fileview_remove_files   (MooFileView    *fileview,
                                             GSList         *files);
static void         fileview_change_files   (MooFileView    *fileview,
                                             GSList         *files);

static void         init_gui                (MooFileView    *fileview);
static void         focus_to_file_view      (MooFileView    *fileview);
static void         focus_to_filter_entry   (MooFileView    *fileview);
static GtkWidget   *create_toolbar          (MooFileView    *fileview);
static GtkWidget   *create_notebook         (MooFileView    *fileview);
static GtkWidget   *create_search_entry     (MooFileView    *fileview);

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


/* MOO_TYPE_FILE_VIEW */
G_DEFINE_TYPE (MooFileView, moo_file_view, GTK_TYPE_VBOX)

enum {
    PROP_0,
    PROP_CURRENT_DIRECTORY,
    PROP_FILE_MGR
};

enum {
    COLUMN_FILE         = 0,
    COLUMN_PIXBUF       = 1,
    COLUMN_DISPLAY_NAME = 2,
    COLUMN_SIZE         = 3,
    COLUMN_DATE         = 4
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

static guint signals[LAST_SIGNAL];

static void moo_file_view_class_init (MooFileViewClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
//     GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
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

    fileview->priv->store =
            gtk_list_store_new (5,
                                MOO_TYPE_FILE,
                                GDK_TYPE_PIXBUF,
                                G_TYPE_STRING,
                                G_TYPE_STRING,
                                G_TYPE_STRING);
    gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (fileview->priv->store),
                                     COLUMN_FILE,
                                     (GtkTreeIterCompareFunc) tree_compare_func,
                                     fileview, NULL);
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (fileview->priv->store),
                                          COLUMN_FILE, GTK_SORT_ASCENDING);

    fileview->priv->filter_model =
            gtk_tree_model_filter_new (GTK_TREE_MODEL (fileview->priv->store),
                                       NULL);
    gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (fileview->priv->filter_model),
                                            (GtkTreeModelFilterVisibleFunc) filter_visible_func,
                                            fileview, NULL);

    init_gui (fileview);

    fileview->priv->name_to_select = NULL;

    fileview->priv->current_dir = NULL;
    fileview->priv->file_system = moo_file_system_create ();

    fileview->priv->icon_size = GTK_ICON_SIZE_MENU;
}


static void moo_file_view_finalize  (GObject      *object)
{
    MooFileView *fileview = MOO_FILE_VIEW (object);

    if (fileview->priv->files)
        g_hash_table_destroy (fileview->priv->files);

    g_object_unref (fileview->priv->filter_model);
    g_object_unref (fileview->priv->store);
    history_free (fileview);
    g_free (fileview->priv->name_to_select);
    fileview->priv->name_to_select = NULL;

    if (fileview->priv->mgr)
        g_object_unref (fileview->priv->mgr);
    if (fileview->priv->current_filter)
        g_object_unref (fileview->priv->current_filter);

    disconnect_folder (fileview);
    g_object_unref (fileview->priv->file_system);

    g_free (fileview->priv);

    G_OBJECT_CLASS (moo_file_view_parent_class)->finalize (object);
}


GtkWidget   *moo_file_view_new              (void)
{
    return GTK_WIDGET (g_object_new (MOO_TYPE_FILE_VIEW, NULL));
}


static void         connect_folder          (MooFileView    *fileview)
{
    g_return_if_fail (fileview->priv->current_dir != NULL);
    g_signal_connect (fileview->priv->current_dir, "files-added",
                      G_CALLBACK (files_added), fileview);
    g_signal_connect (fileview->priv->current_dir, "files-changed",
                      G_CALLBACK (files_changed), fileview);
    g_signal_connect (fileview->priv->current_dir, "files-removed",
                      G_CALLBACK (files_removed), fileview);
    g_signal_connect (fileview->priv->current_dir, "deleted",
                      G_CALLBACK (folder_deleted), fileview);

    fileview->priv->files_to_add =
            g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                   (GDestroyNotify) moo_file_unref,
                                   NULL);
    fileview->priv->files_to_remove =
            g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                   (GDestroyNotify) moo_file_unref,
                                   NULL);
    fileview->priv->files_to_change =
            g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                   (GDestroyNotify) moo_file_unref,
                                   NULL);
}

static void         disconnect_folder       (MooFileView    *fileview)
{
    if (fileview->priv->current_dir)
    {
        g_signal_handlers_disconnect_by_func (fileview->priv->current_dir,
                                              (gpointer)files_added,
                                              fileview);
        g_signal_handlers_disconnect_by_func (fileview->priv->current_dir,
                                              (gpointer)files_changed,
                                              fileview);
        g_signal_handlers_disconnect_by_func (fileview->priv->current_dir,
                                              (gpointer)files_removed,
                                              fileview);
        g_signal_handlers_disconnect_by_func (fileview->priv->current_dir,
                                              (gpointer)folder_deleted,
                                              fileview);
    }

    if (fileview->priv->files_to_add)
        g_hash_table_destroy (fileview->priv->files_to_add);
    fileview->priv->files_to_add = NULL;
    if (fileview->priv->add_files_idle)
        g_source_remove (fileview->priv->add_files_idle);
    fileview->priv->add_files_idle = 0;

    if (fileview->priv->files_to_remove)
        g_hash_table_destroy (fileview->priv->files_to_remove);
    fileview->priv->files_to_remove = NULL;
    if (fileview->priv->remove_files_idle)
        g_source_remove (fileview->priv->remove_files_idle);
    fileview->priv->remove_files_idle = 0;

    if (fileview->priv->files_to_change)
        g_hash_table_destroy (fileview->priv->files_to_change);
    fileview->priv->files_to_change = NULL;
    if (fileview->priv->change_files_idle)
        g_source_remove (fileview->priv->change_files_idle);
    fileview->priv->change_files_idle = 0;
}


static gboolean     clear_list_in_idle      (MooFileView    *fileview)
{
    if (fileview->priv->need_to_clear)
        gtk_list_store_clear (fileview->priv->store);
    return FALSE;
}

static gboolean     moo_file_view_chdir_real(MooFileView    *fileview,
                                             const char     *new_dir,
                                             GError        **error)
{
    char *real_new_dir;
    MooFolder *folder;
    GSList *files;

    g_return_val_if_fail (MOO_IS_FILE_VIEW (fileview), FALSE);

    if (!new_dir)
    {
        if (fileview->priv->current_dir)
        {
            disconnect_folder (fileview);
            g_object_unref (fileview->priv->current_dir);
            fileview->priv->current_dir = NULL;
            history_clear (fileview);
            gtk_list_store_clear (fileview->priv->store);
        }

        return TRUE;
    }

    if (fileview->priv->current_dir &&
        !strcmp (moo_folder_get_path (fileview->priv->current_dir), new_dir))
            return TRUE;

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

    disconnect_folder (fileview);
    if (fileview->priv->current_dir)
        g_object_unref (fileview->priv->current_dir);

    fileview->priv->current_dir = folder;

    history_goto (fileview, moo_folder_get_path (folder));

    fileview->priv->need_to_clear = TRUE;
    g_idle_add_full (G_PRIORITY_LOW,
                     (GSourceFunc) clear_list_in_idle,
                     fileview, NULL);
    fileview->priv->process_changes_now = TRUE;

    if (fileview->priv->files)
        g_hash_table_destroy (fileview->priv->files);
    fileview->priv->files =
            g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                   (GDestroyNotify) moo_file_unref,
                                   (GDestroyNotify) gtk_tree_row_reference_free);

    connect_folder (fileview);

    files = moo_folder_list_files (folder);
    fileview_add_files (fileview, files);
    g_slist_foreach (files, (GFunc) moo_file_unref, NULL);
    g_slist_free (files);

    return TRUE;
}


static void         folder_deleted          (MooFolder      *folder,
                                             MooFileView    *fileview)
{
    g_assert (folder == fileview->priv->current_dir);
    disconnect_folder (fileview);
    gtk_list_store_clear (fileview->priv->store);
}


static void         files_added             (MooFolder      *folder,
                                             GSList         *files,
                                             MooFileView    *fileview)
{
    g_assert (folder == fileview->priv->current_dir);
    fileview_add_files (fileview, files);
}


static void         files_changed           (MooFolder      *folder,
                                             GSList         *files,
                                             MooFileView    *fileview)
{
    g_assert (folder == fileview->priv->current_dir);
    fileview_change_files (fileview, files);
}


static void         files_removed           (MooFolder      *folder,
                                             GSList         *files,
                                             MooFileView    *fileview)
{
    g_assert (folder == fileview->priv->current_dir);
    fileview_remove_files (fileview, files);
}


typedef void (*ForEachFileFunc) (MooFileView    *fileview,
                                 MooFile        *file);

static gboolean do_timed_func (MooFile  *file,
                               G_GNUC_UNUSED gpointer  dummy,
                               gpointer user_data)
{
    struct {
        GTimer *timer;
        gboolean stop;
        MooFileView *fileview;
        ForEachFileFunc func;
    }* data = user_data;

    if (data->stop)
        return FALSE;

    data->func (data->fileview, file);

    if (g_timer_elapsed (data->timer, NULL) > TIMEOUT)
        data->stop = TRUE;

    return TRUE;
}

static void files_table_do_timed    (GHashTable         *files_table,
                                     ForEachFileFunc     func,
                                     MooFileView        *fileview)
{
    struct {
        GTimer *timer;
        gboolean stop;
        MooFileView *fileview;
        ForEachFileFunc func;
    } data;

    data.timer = g_timer_new ();
    data.stop = FALSE;
    data.fileview = fileview;
    data.func = func;

    g_hash_table_foreach_remove (files_table,
                                 (GHRFunc) do_timed_func,
                                 &data);

    g_timer_destroy (data.timer);
}


static void start_add       (MooFileView    *fileview);
static void start_change    (MooFileView    *fileview);
static void start_remove    (MooFileView    *fileview);
static void add_a_bit       (MooFileView    *fileview);
static void change_a_bit    (MooFileView    *fileview);
static void remove_a_bit    (MooFileView    *fileview);

static void add_one_file (MooFileView    *fileview,
                          MooFile        *file)
{
    GtkTreeIter iter;
    GtkTreePath *path;
    GtkTreeRowReference *old;

    g_assert (file != NULL);

    old = g_hash_table_lookup (fileview->priv->files, file);
    g_assert (old == NULL);

    if (old && gtk_tree_row_reference_valid (old))
    {
        g_warning ("readding file");
        path = gtk_tree_row_reference_get_path (old);
        gtk_tree_model_get_iter (GTK_TREE_MODEL (fileview->priv->store),
                                    &iter, path);
        gtk_tree_path_free (path);
    }
    else
    {
        gtk_list_store_prepend (fileview->priv->store, &iter);
        path = gtk_tree_model_get_path (GTK_TREE_MODEL (fileview->priv->store),
                                        &iter);
        g_hash_table_insert (fileview->priv->files,
                             moo_file_ref (file),
                             gtk_tree_row_reference_new (GTK_TREE_MODEL (fileview->priv->store),
                                     path));
        gtk_tree_path_free (path);
    }

    gtk_list_store_set (fileview->priv->store, &iter,
                        COLUMN_FILE, file,
                        COLUMN_PIXBUF,
                        moo_file_get_icon (file,
                                           GTK_WIDGET (fileview),
                                           GTK_ICON_SIZE_MENU),
                        COLUMN_DISPLAY_NAME,
                        moo_file_get_display_basename (file),
                        COLUMN_SIZE, NULL,
                        COLUMN_DATE, NULL, -1);
}


static gboolean     fileview_add_files_in_idle  (MooFileView    *fileview)
{
    if (fileview->priv->need_to_clear)
        gtk_list_store_clear (fileview->priv->store);
    fileview->priv->need_to_clear = FALSE;

    if (fileview->priv->remove_files_idle)
        remove_a_bit (fileview);
    else
        add_a_bit (fileview);

    gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (fileview->priv->store),
                                     COLUMN_FILE,
                                     (GtkTreeIterCompareFunc) tree_compare_func,
                                     fileview, NULL);

    if (!g_hash_table_size (fileview->priv->files_to_add))
    {
        if (fileview->priv->add_files_idle)
            g_source_remove (fileview->priv->add_files_idle);
        fileview->priv->add_files_idle = 0;
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


static void         fileview_add_files      (MooFileView    *fileview,
                                             GSList         *files)
{
    GSList *l;
    gboolean add = FALSE;
    gboolean change = FALSE;

    for (l = files; l != NULL; l = l->next)
    {
        MooFile *file = l->data;
        g_assert (file != NULL);

        if (g_hash_table_lookup (fileview->priv->files_to_remove, file))
        {
            g_assert (g_hash_table_lookup (fileview->priv->files, file));
            g_assert (!g_hash_table_lookup (fileview->priv->files_to_change, file));
            g_assert (!g_hash_table_lookup (fileview->priv->files_to_add, file));
            g_hash_table_remove (fileview->priv->files_to_remove, file);
            g_hash_table_insert (fileview->priv->files_to_change,
                                 moo_file_ref (file), NULL);
            change = TRUE;
        }
        else
        {
            g_assert (!g_hash_table_lookup (fileview->priv->files, file));
            g_assert (!g_hash_table_lookup (fileview->priv->files_to_add, file));
            g_assert (!g_hash_table_lookup (fileview->priv->files_to_change, file));
            g_hash_table_insert (fileview->priv->files_to_add,
                                 moo_file_ref (file), NULL);
            add = TRUE;
        }
    }

    if (add)
        start_add (fileview);

    if (change)
        start_change (fileview);
}


static void remove_one_file (MooFileView    *fileview,
                             MooFile        *file)
{
    GtkTreeIter iter;
    GtkTreePath *path;
    GtkTreeRowReference *ref;

    g_assert (file != NULL);

    ref = g_hash_table_lookup (fileview->priv->files, file);
    g_assert (ref != NULL && gtk_tree_row_reference_valid (ref));
    path = gtk_tree_row_reference_get_path (ref);

    gtk_tree_model_get_iter (GTK_TREE_MODEL (fileview->priv->store), &iter, path);
    gtk_list_store_remove (fileview->priv->store, &iter);
    g_hash_table_remove (fileview->priv->files, file);
}


static gboolean     fileview_remove_files_in_idle   (MooFileView    *fileview)
{
    remove_a_bit (fileview);

    if (!g_hash_table_size (fileview->priv->files_to_remove))
    {
        g_source_remove (fileview->priv->remove_files_idle);
        fileview->priv->remove_files_idle = 0;
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


static void         fileview_remove_files   (MooFileView    *fileview,
                                             GSList         *files)
{
    GSList *l;
    gboolean remove = FALSE;

    for (l = files; l != NULL; l = l->next)
    {
        MooFile *file = l->data;
        g_assert (file != NULL);
        g_hash_table_remove (fileview->priv->files_to_add, file);
        g_hash_table_remove (fileview->priv->files_to_change, file);
        if (g_hash_table_lookup (fileview->priv->files, file))
        {
            remove = TRUE;
            g_hash_table_insert (fileview->priv->files_to_remove,
                                 moo_file_ref (file), NULL);
        }
    }

    if (remove)
        start_remove (fileview);
}


static void change_one_file (MooFileView    *fileview,
                             MooFile        *file)
{
    GtkTreeIter iter;
    GtkTreePath *path;
    GtkTreeRowReference *ref;

    g_assert (file != NULL);

    ref = g_hash_table_lookup (fileview->priv->files, file);

    g_assert (ref != NULL);
    g_assert (gtk_tree_row_reference_valid (ref));

    path = gtk_tree_row_reference_get_path (ref);

    gtk_tree_model_get_iter (GTK_TREE_MODEL (fileview->priv->store), &iter, path);
    gtk_list_store_set (fileview->priv->store, &iter,
                        COLUMN_FILE, file,
                        COLUMN_PIXBUF,
                        moo_file_get_icon (file,
                                           GTK_WIDGET (fileview),
                                           GTK_ICON_SIZE_MENU),
                        COLUMN_DISPLAY_NAME,
                        moo_file_get_display_basename (file),
                        COLUMN_SIZE, NULL,
                        COLUMN_DATE, NULL, -1);
}


static gboolean     fileview_change_files_in_idle   (MooFileView    *fileview)
{
    if (fileview->priv->remove_files_idle)
        remove_a_bit (fileview);
    else if (fileview->priv->add_files_idle)
        add_a_bit (fileview);
    else
        change_a_bit (fileview);

    gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (fileview->priv->store),
                                     COLUMN_FILE,
                                     (GtkTreeIterCompareFunc) tree_compare_func,
                                     fileview, NULL);

    if (!g_hash_table_size (fileview->priv->files_to_change))
    {
        if (fileview->priv->change_files_idle)
            g_source_remove (fileview->priv->change_files_idle);
        fileview->priv->change_files_idle = 0;
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


static void         fileview_change_files   (MooFileView    *fileview,
                                             GSList         *files)
{
    GSList *l;
    gboolean add = FALSE;
    gboolean change = FALSE;

    for (l = files; l != NULL; l = l->next)
    {
        MooFile *file = l->data;
        g_assert (file != NULL);

        g_hash_table_remove (fileview->priv->files_to_remove, file);

        if (!g_hash_table_lookup (fileview->priv->files, file))
        {
            g_hash_table_insert (fileview->priv->files_to_add,
                                 moo_file_ref (file), NULL);
            add = TRUE;
        }
        else
        {
            g_hash_table_insert (fileview->priv->files_to_change,
                                 moo_file_ref (file), NULL);
            change = TRUE;
        }
    }

    if (add)
        start_add (fileview);

    if (change)
        start_change (fileview);
}


static void     start_add       (MooFileView    *fileview)
{
    if (!fileview->priv->add_files_idle)
    {
        if (!fileview->priv->process_changes_now || fileview_add_files_in_idle (fileview))
            fileview->priv->add_files_idle =
                    g_idle_add ((GSourceFunc) fileview_add_files_in_idle,
                                 fileview);
        else
            start_change (fileview);
    }

    fileview->priv->process_changes_now = FALSE;
}

static void     start_change    (MooFileView    *fileview)
{
    if (!fileview->priv->change_files_idle)
    {
        if (!fileview->priv->process_changes_now || fileview_change_files_in_idle (fileview))
            fileview->priv->change_files_idle =
                    g_idle_add ((GSourceFunc) fileview_change_files_in_idle,
                                 fileview);
    }

    fileview->priv->process_changes_now = FALSE;
}

static void     start_remove    (MooFileView    *fileview)
{
    if (!fileview->priv->remove_files_idle)
        fileview->priv->remove_files_idle =
                g_idle_add ((GSourceFunc) fileview_remove_files_in_idle,
                             fileview);
}

static void remove_a_bit    (MooFileView    *fileview)
{
    files_table_do_timed (fileview->priv->files_to_remove,
                          remove_one_file,
                          fileview);
}

static void add_a_bit       (MooFileView    *fileview)
{
    files_table_do_timed (fileview->priv->files_to_add,
                          add_one_file,
                          fileview);
}

static void change_a_bit    (MooFileView    *fileview)
{
    files_table_do_timed (fileview->priv->files_to_change,
                          change_one_file,
                          fileview);
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

    search_entry = create_search_entry (fileview);
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
//     gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
    gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview), COLUMN_DISPLAY_NAME);

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
    gtk_tree_view_column_set_attributes (column, cell,
                                         "pixbuf", COLUMN_PIXBUF,
                                         NULL);

    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, cell, TRUE);
    gtk_tree_view_column_set_attributes (column, cell,
                                         "text", COLUMN_DISPLAY_NAME,
                                         NULL);

#if 1
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_title (column, "Size");
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, cell, FALSE);
    gtk_tree_view_column_set_attributes (column, cell,
                                         "text", COLUMN_SIZE,
                                         NULL);

    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_title (column, "Date");
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, cell, FALSE);
    gtk_tree_view_column_set_attributes (column, cell,
                                         "text", COLUMN_DATE,
                                         NULL);
#endif

    return treeview;
}


static gboolean     filter_visible_func (GtkTreeModel   *model,
                                         GtkTreeIter    *iter,
                                         MooFileView    *fileview)
{
    MooFile *file;
    gboolean visible = TRUE;

    gtk_tree_model_get (model, iter, COLUMN_FILE, &file, -1);

    /* file == NULL when row was just added */
    if (!file)
    {
        visible = FALSE;
        goto out;
    }

    if (!strcmp (moo_file_get_basename (file), ".."))
    {
        visible = fileview->priv->show_two_dots;
        goto out;
    }

    if (!fileview->priv->show_hidden_files && moo_file_test (file, MOO_FILE_IS_HIDDEN))
    {
        visible = FALSE;
        goto out;
    }

    if (moo_file_test (file, MOO_FILE_IS_FOLDER))
        goto out;

    if (fileview->priv->current_filter && fileview->priv->use_current_filter)
    {
        GtkFileFilterInfo filter_info;
        GtkFileFilter *filter = fileview->priv->current_filter;

        filter_info.contains = gtk_file_filter_get_needed (filter);
        filter_info.filename = moo_file_get_basename (file);
        filter_info.uri = NULL;
        filter_info.display_name = moo_file_get_display_basename (file);
        filter_info.mime_type = moo_file_get_mime_type (file);

        visible = gtk_file_filter_filter (fileview->priv->current_filter,
                                          &filter_info);
    }

out:
    moo_file_unref (file);
    return visible;
}


static int          tree_compare_func   (GtkTreeModel   *model,
                                         GtkTreeIter    *a,
                                         GtkTreeIter    *b)
{
    MooFile *f1, *f2;
    gboolean is_dir1, is_dir2;
    gboolean result = 0;

    gtk_tree_model_get (model, a, COLUMN_FILE, &f1, -1);
    gtk_tree_model_get (model, b, COLUMN_FILE, &f2, -1);
    g_assert (f1 != NULL && f2 != NULL);

    if (f1 == f2)
    {
        result = 0;
        goto out;
    }

    if (!f1 || !f2)
    {
        if (f1 < f2)
            result = -1;
        else
            result = 1;
        goto out;
    }

    is_dir1 = moo_file_test (f1, MOO_FILE_IS_FOLDER);
    is_dir2 = moo_file_test (f2, MOO_FILE_IS_FOLDER);

    if (is_dir1 != is_dir2)
    {
        if (is_dir1 && !is_dir2)
            result = -1;
        else
            result = 1;
        goto out;
    }

    if (is_dir1)
    {
        if (!strcmp (moo_file_get_basename (f1), ".."))
            result = -1;
        else if (!strcmp (moo_file_get_basename (f2), ".."))
            result = 1;
        else
            result = strcmp (moo_file_get_basename (f1),
                             moo_file_get_basename (f2));
        goto out;
    }

    result = strcmp (moo_file_get_basename (f1),
                     moo_file_get_basename (f2));

out:
    moo_file_unref (f1);
    moo_file_unref (f2);
    return result;
}


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
    char *dirname, *basename;
    GError *error = NULL;

    g_return_if_fail (fileview->priv->current_dir != NULL);

    basename = g_path_get_basename (moo_folder_get_path (fileview->priv->current_dir));
    dirname = g_path_get_dirname (moo_folder_get_path (fileview->priv->current_dir));

    /* TODO TODO do something about top-level directories */
    if (!strcmp (basename, dirname))
        goto out;

    if (!moo_file_view_chdir (fileview, dirname, &error))
    {
        g_warning ("%s: could not go up", G_STRLOC);

        if (error)
        {
            g_warning ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }

        goto out;
    }

    moo_file_view_select_file (fileview, basename);

out:
    g_free (dirname);
    g_free (basename);
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


static GtkWidget   *create_iconview         (MooFileView    *fileview)
{
    GtkWidget *iconview;
    GtkCellRenderer *cell;

    iconview = moo_icon_view_new_with_model (fileview->priv->filter_model);
    g_signal_connect (iconview, "key-press-event",
                      G_CALLBACK (moo_file_view_key_press),
                      fileview);
    moo_icon_view_set_attributes (MOO_ICON_VIEW (iconview),
                                  MOO_ICON_VIEW_CELL_PIXBUF,
                                  "pixbuf", COLUMN_PIXBUF,
                                  NULL);
    moo_icon_view_set_attributes (MOO_ICON_VIEW (iconview),
                                  MOO_ICON_VIEW_CELL_TEXT,
                                  "text", COLUMN_DISPLAY_NAME,
                                  NULL);

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
/* Interactive search
 */

enum {
    SEARCH_COLUMN_NAME,
    SEARCH_COLUMN_FILE
};

static GtkWidget   *create_search_entry     (MooFileView    *fileview)
{
    GtkWidget *entry;
    GtkEntryCompletion *completion;
    GtkListStore *store;

    entry = gtk_entry_new ();
    g_object_set_data (G_OBJECT (entry), "moo-file-view", fileview);
    gtk_widget_set_no_show_all (entry, TRUE);

    completion = gtk_entry_completion_new ();
    gtk_entry_set_completion (GTK_ENTRY (entry), completion);

    store = gtk_list_store_new (2, G_TYPE_STRING,
                                MOO_TYPE_FILE);
    gtk_entry_completion_set_model (completion,
                                    GTK_TREE_MODEL (store));
    gtk_entry_completion_set_text_column (completion, SEARCH_COLUMN_NAME);

    g_object_unref (store);
    g_object_unref (completion);
    return entry;
}


/****************************************************************************/
/* Auxiliary stuff
 */

static GtkTreePath *tree_view_get_selected  (GtkTreeView    *treeview)
{
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    GtkTreeModel *model;

    selection = gtk_tree_view_get_selection (treeview);

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        return NULL;
    else
        return gtk_tree_model_get_path (model, &iter);
}


static void     activate_file_widget        (MooFileView    *fileview)
{
    if (fileview->priv->view_type == MOO_FILE_VIEW_LIST)
    {
        GtkTreePath *path;
        path = tree_view_get_selected (fileview->priv->treeview);
        if (path)
        {
            gtk_tree_view_row_activated (fileview->priv->treeview, path,
                                         fileview->priv->tree_name_column);
            gtk_tree_path_free (path);
        }
    }
    else
    {
        moo_icon_view_activate_selected (fileview->priv->iconview);
    }
}


static void     file_widget_move_selection  (MooFileView    *fileview,
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
            gtk_tree_selection_select_path (selection, path);
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


static gboolean find_match                  (MooFileView    *fileview,
                                             const char     *text,
                                             GtkTreeIter    *iter,
                                             gboolean        exact_match)
{
    GtkTreeModel *model = GTK_TREE_MODEL (fileview->priv->store);
    guint len;

    g_return_val_if_fail (text != NULL, FALSE);
    if (!text[0])
        return FALSE;

    if (!gtk_tree_model_get_iter_first (model, iter))
        return FALSE;

    len = strlen (text);

    while (TRUE)
    {
        char *val;
        gboolean match;

        gtk_tree_model_get (model, iter,
                            COLUMN_DISPLAY_NAME, &val, -1);

        if (val)
        {
            if (exact_match)
                match = !strcmp (text, val);
            else
                match = !strncmp (text, val, len);

            g_free (val);

            if (match)
                return TRUE;
        }

        if (!gtk_tree_model_iter_next (model, iter))
            return FALSE;
    }
}


/****************************************************************************/
/* Search entry
 */

static gboolean search_entry_key_press      (GtkWidget      *entry,
                                             GdkEventKey    *event,
                                             MooFileView    *fileview);
static gboolean search_entry_focus_out      (GtkWidget      *entry,
                                             GdkEventFocus  *event,
                                             MooFileView    *fileview);
static void     search_entry_activate       (GtkWidget      *entry,
                                             MooFileView    *fileview);
static void     search_entry_changed        (GtkEntry       *entry,
                                             MooFileView    *fileview);

static void         start_interactive_search    (MooFileView    *fileview)
{
    GtkWidget *entry = GTK_WIDGET (fileview->priv->search_entry);

    file_widget_move_selection (fileview, NULL);
    gtk_widget_show (entry);
    gtk_widget_grab_focus (entry);

    fileview->priv->in_search = TRUE;

    g_signal_connect (entry, "key-press-event",
                      G_CALLBACK (search_entry_key_press),
                      fileview);
    g_signal_connect (entry, "focus-out-event",
                      G_CALLBACK (search_entry_focus_out),
                      fileview);
    g_signal_connect (entry, "activate",
                      G_CALLBACK (search_entry_activate),
                      fileview);
    g_signal_connect (entry, "changed",
                      G_CALLBACK (search_entry_changed),
                      fileview);
}


static void         stop_interactive_search     (MooFileView    *fileview)
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

    gtk_widget_grab_focus (get_file_view_widget (fileview));
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
    GtkWidget *entry = GTK_WIDGET (fileview->priv->search_entry);

    if (fileview->priv->in_search)
    {
        g_warning ("%s: something wrong", G_STRLOC);
        stop_interactive_search (fileview);
        return FALSE;
    }

    if (GTK_WIDGET_CLASS(G_OBJECT_GET_CLASS (widget))->key_press_event (widget, event))
        return TRUE;

    if (GTK_WIDGET_CLASS(G_OBJECT_GET_CLASS (fileview))->key_press_event (widget, event))
        return TRUE;

    if (event->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK))
        return FALSE;

    if (event->string && event->length)
    {
        GdkEvent *copy = gdk_event_copy ((GdkEvent*) event);
        gtk_widget_realize (entry);
        g_object_unref (copy->key.window);
        copy->key.window = g_object_ref (entry->window);

        start_interactive_search (fileview);
        gtk_widget_event (entry, copy);

        gdk_event_free (copy);
        return TRUE;
    }

    return FALSE;
}


static gboolean search_entry_key_press      (G_GNUC_UNUSED GtkWidget *entry,
                                             GdkEventKey    *event,
                                             MooFileView    *fileview)
{
    switch (event->keyval)
    {
        case GDK_Escape:
            stop_interactive_search (fileview);
            return TRUE;

        default:
            return FALSE;
    }
}


static gboolean search_entry_focus_out      (G_GNUC_UNUSED GtkWidget *entry,
                                             G_GNUC_UNUSED GdkEventFocus *event,
                                             MooFileView    *fileview)
{
    stop_interactive_search (fileview);
    return FALSE;
}


static void     search_entry_activate       (GtkWidget      *entry,
                                             MooFileView    *fileview)
{
    stop_interactive_search (fileview);
    activate_file_widget (fileview);
}


static void     search_entry_changed        (GtkEntry       *entry,
                                             MooFileView    *fileview)
{
    GtkTreeIter iter, filter_iter;
    const char *text;

    text = gtk_entry_get_text (entry);

    if (find_match (fileview, text, &iter, FALSE))
    {
        GtkTreePath *filter_path, *path;

        path = gtk_tree_model_get_path (GTK_TREE_MODEL (fileview->priv->store),
                                        &iter);
        g_return_if_fail (path != NULL);

        filter_path = gtk_tree_model_filter_convert_child_path_to_path (
                GTK_TREE_MODEL_FILTER (fileview->priv->filter_model), path);
        if (!filter_path)
        {
            gtk_tree_path_free (path);
            file_widget_move_selection (fileview, NULL);
        }
        else
        {
            gtk_tree_model_get_iter (fileview->priv->filter_model,
                                     &filter_iter, filter_path);
            file_widget_move_selection (fileview, &filter_iter);
            gtk_tree_path_free (path);
            gtk_tree_path_free (filter_path);
        }
    }
    else
    {
        file_widget_move_selection (fileview, NULL);
    }
}


void        moo_file_view_select_file       (MooFileView    *fileview,
                                             const char     *basename)
{
    if (basename)
    {
        GtkTreeIter iter, filter_iter;

        if (find_match (fileview, basename, &iter, FALSE))
        {
            GtkTreePath *filter_path, *path;

            path = gtk_tree_model_get_path (GTK_TREE_MODEL (fileview->priv->store),
                                            &iter);
            g_return_if_fail (path != NULL);

            filter_path = gtk_tree_model_filter_convert_child_path_to_path (
                    GTK_TREE_MODEL_FILTER (fileview->priv->filter_model), path);
            if (!filter_path)
            {
                gtk_tree_path_free (path);
                file_widget_move_selection (fileview, NULL);
            }
            else
            {
                gtk_tree_model_get_iter (fileview->priv->filter_model,
                                         &filter_iter, filter_path);
                file_widget_move_selection (fileview, &filter_iter);
                gtk_tree_path_free (path);
                gtk_tree_path_free (filter_path);
            }
        }
        else
        {
            g_free (fileview->priv->name_to_select);
            fileview->priv->name_to_select = g_strdup (basename);
        }
    }
    else
    {
        file_widget_move_selection (fileview, NULL);
    }
}
