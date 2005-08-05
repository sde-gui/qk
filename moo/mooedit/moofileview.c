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

#include "mooedit/moofileview.h"
#include "mooutils/moomarshals.h"
#include <gdk/gdkkeysyms.h>
#include <glib/gstdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef USE_XDGMIME
#include "mooedit/xdgmime/xdgmime.h"
#endif

#define TREEVIEW_UPDATE_TIMEOUT 0.5

enum {
    TREEVIEW_PAGE = 0,
    ICONVIEW_PAGE = 1
};


typedef struct _History History;

struct _MooFileViewFile {
    char        *basename;
    char        *fullname;
    char        *uri;
    char        *display_name;
    const char  *mime_type;
    GdkPixbuf   *pixbuf;
    gboolean     is_dir;
    struct stat  statbuf;
    gboolean     exists;
    gpointer     time;  /* struct tm* */
    gpointer     date_string;
    guint        size;
    guint        ref_count;
};

struct _MooFileViewPrivate {
    GtkListStore    *store;
    GtkTreeModel    *filter_model;
    MooFileViewType  view_type;
    GtkWidget       *treeview;
    GtkWidget       *iconview;
    char            *current_dir;
    gboolean         show_hidden_files;
    History         *history;
    guint            populate_idle;
    GDir            *populate_dir;
};


static MooFileViewFile  *file_new   (MooFileView        *fileview,
                                     const char         *basename,
                                     const char         *fullname);
static MooFileViewFile  *file_ref   (MooFileViewFile    *file);
static void              file_unref (MooFileViewFile    *file);


static void         moo_file_view_finalize  (GObject        *object);

static gboolean     moo_file_view_chdir_real(MooFileView    *fileview,
                                             const char     *dir,
                                             GError        **error);
static void         moo_file_view_go_up     (MooFileView    *fileview);
static void         moo_file_view_go_home   (MooFileView    *fileview);
static void         moo_file_view_go_back   (MooFileView    *fileview);
static void         moo_file_view_go_forward(MooFileView    *fileview);

static void         history_init            (MooFileView    *fileview);
static void         history_free            (MooFileView    *fileview);
static void         history_goto            (MooFileView    *fileview,
                                             const char     *dirname);
static const char  *history_go              (MooFileView    *fileview,
                                             GtkDirectionType where);
static void         history_revert_go       (MooFileView    *fileview);

static gboolean     populate_tree           (MooFileView        *fileview,
                                             GError            **error);
static gboolean     filter_visible_func     (GtkTreeModel       *model,
                                             GtkTreeIter        *iter,
                                             MooFileView        *fileview);
static int          tree_compare_func       (GtkTreeModel       *model,
                                             GtkTreeIter        *a,
                                             GtkTreeIter        *b);

static void         init_gui                (MooFileView    *fileview);
static GtkWidget   *create_toolbar          (MooFileView    *fileview);
static void         toolbar_button_clicked  (GtkToolButton  *button,
                                             MooFileView    *fileview);
static GtkWidget   *create_notebook         (MooFileView    *fileview);
static GtkWidget   *create_filter_combo     (MooFileView    *fileview);

static GtkWidget   *create_treeview         (MooFileView    *fileview);
static void         tree_row_activated      (GtkTreeView    *treeview,
                                             GtkTreePath    *treepath,
                                             GtkTreeViewColumn *column,
                                             MooFileView    *fileview);

static GtkWidget   *create_iconview         (MooFileView    *fileview);
static void         icon_item_activated     (GtkIconView    *iconview,
                                             GtkTreePath    *treepath,
                                             MooFileView    *fileview);

static void         get_icon                (MooFileView    *fileview,
                                             MooFileViewFile*file);


/* MOO_TYPE_FILE_VIEW */
G_DEFINE_TYPE (MooFileView, moo_file_view, GTK_TYPE_VBOX)

enum {
    PROP_0,
    PROP_DIRECTORY
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
    GO_UP,
    GO_BACK,
    GO_FORWARD,
    GO_HOME,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

static void moo_file_view_class_init (MooFileViewClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkBindingSet *binding_set;

    gobject_class->finalize = moo_file_view_finalize;

    klass->chdir = moo_file_view_chdir_real;
    klass->go_up = moo_file_view_go_up;
    klass->go_home = moo_file_view_go_home;
    klass->go_back = moo_file_view_go_back;
    klass->go_forward = moo_file_view_go_forward;

    signals[CHDIR] = g_signal_new ("chdir",
                                   G_OBJECT_CLASS_TYPE (klass),
                                   G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                   G_STRUCT_OFFSET (MooFileViewClass, chdir),
                                   NULL, NULL,
                                   _moo_marshal_BOOLEAN__STRING_POINTER,
                                   G_TYPE_BOOLEAN, 2,
                                   G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE,
                                   G_TYPE_POINTER);

    signals[ACTIVATE] = g_signal_new ("activate",
                                      G_OBJECT_CLASS_TYPE (klass),
                                      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                      G_STRUCT_OFFSET (MooFileViewClass, activate),
                                      NULL, NULL,
                                      _moo_marshal_VOID__BOXED,
                                      G_TYPE_NONE, 1,
                                      MOO_TYPE_FILE_VIEW_FILE | G_SIGNAL_TYPE_STATIC_SCOPE);

    signals[GO_UP] = g_signal_new ("go-up",
                                   G_OBJECT_CLASS_TYPE (klass),
                                   G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                   G_STRUCT_OFFSET (MooFileViewClass, go_up),
                                   NULL, NULL,
                                   _moo_marshal_VOID__VOID,
                                   G_TYPE_NONE, 0);

    signals[GO_FORWARD] = g_signal_new ("go-forward",
                                        G_OBJECT_CLASS_TYPE (klass),
                                        G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                        G_STRUCT_OFFSET (MooFileViewClass, go_forward),
                                        NULL, NULL,
                                        _moo_marshal_VOID__VOID,
                                        G_TYPE_NONE, 0);

    signals[GO_BACK] = g_signal_new ("go-back",
                                     G_OBJECT_CLASS_TYPE (klass),
                                     G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                     G_STRUCT_OFFSET (MooFileViewClass, go_back),
                                     NULL, NULL,
                                     _moo_marshal_VOID__VOID,
                                     G_TYPE_NONE, 0);

    signals[GO_HOME] = g_signal_new ("go-home",
                                     G_OBJECT_CLASS_TYPE (klass),
                                     G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                     G_STRUCT_OFFSET (MooFileViewClass, go_home),
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
                                  "home-folder",
                                  0);
    gtk_binding_entry_add_signal (binding_set,
                                  GDK_KP_Home, GDK_MOD1_MASK,
                                  "home-folder",
                                  0);
}


static void moo_file_view_init      (MooFileView *fileview)
{
    fileview->priv = g_new0 (MooFileViewPrivate, 1);

    fileview->priv->current_dir = NULL;
    fileview->priv->show_hidden_files = FALSE;

    fileview->priv->view_type = MOO_FILE_VIEW_LIST;

    history_init (fileview);

    fileview->priv->store =
            gtk_list_store_new (5,
                                MOO_TYPE_FILE_VIEW_FILE,
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
}


static void moo_file_view_finalize  (GObject      *object)
{
    MooFileView *fileview = MOO_FILE_VIEW (object);

    g_object_unref (fileview->priv->filter_model);
    g_object_unref (fileview->priv->store);

    history_free (fileview);

    g_free (fileview->priv->current_dir);

    g_free (fileview->priv);

    G_OBJECT_CLASS (moo_file_view_parent_class)->finalize (object);
}


GtkWidget   *moo_file_view_new              (void)
{
    return GTK_WIDGET (g_object_new (MOO_TYPE_FILE_VIEW, NULL));
}


static gboolean     moo_file_view_chdir_real(MooFileView    *fileview,
                                             const char     *new_dir,
                                             GError        **error)
{
    char *real_new_dir;

    g_return_val_if_fail (MOO_IS_FILE_VIEW (fileview), FALSE);
    g_return_val_if_fail (new_dir != NULL, FALSE);

    if (fileview->priv->current_dir && !strcmp (fileview->priv->current_dir, new_dir))
        return TRUE;

    if (g_path_is_absolute (new_dir))
    {
        real_new_dir = g_strdup (new_dir);
    }
    else
    {
        char *current_dir = g_get_current_dir ();
        real_new_dir = g_build_filename (current_dir, new_dir);
    }

    if (!g_file_test (real_new_dir, G_FILE_TEST_IS_DIR))
    {
        g_set_error (error,
                     G_FILE_ERROR,
                     G_FILE_ERROR_NOTDIR,
                     "'%s' is not a directory",
                     real_new_dir);
        g_free (real_new_dir);
        return FALSE;
    }

    g_free (fileview->priv->current_dir);
    fileview->priv->current_dir = real_new_dir;

    history_goto (fileview, real_new_dir);

    gtk_list_store_clear (fileview->priv->store);
    return populate_tree (fileview, error);
}


static void         init_gui        (MooFileView    *fileview)
{
    GtkBox *box;
    GtkWidget *toolbar, *notebook, *filter_combo;

    box = GTK_BOX (fileview);

    toolbar = create_toolbar (fileview);
    gtk_widget_show (toolbar);
    gtk_box_pack_start (box, toolbar, FALSE, FALSE, 0);

    notebook = create_notebook (fileview);
    gtk_widget_show (notebook);
    gtk_box_pack_start (box, notebook, TRUE, TRUE, 0);

    filter_combo = create_filter_combo (fileview);
    gtk_widget_show (filter_combo);
    gtk_box_pack_start (box, filter_combo, FALSE, FALSE, 0);

    if (fileview->priv->view_type == MOO_FILE_VIEW_ICON)
    {
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook),
                                       ICONVIEW_PAGE);
        gtk_widget_grab_focus (fileview->priv->iconview);
    }
    else
    {
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook),
                                       TREEVIEW_PAGE);
        gtk_widget_grab_focus (fileview->priv->treeview);
    }
}


static GtkWidget   *create_toolbar  (MooFileView    *fileview)
{
    GtkWidget *toolbar, *icon;
    GtkToolItem *item;
    GtkTooltips *tooltips;

    tooltips = gtk_tooltips_new ();

    /*********************************************************/
    /* Navigation toolbar                                    */

    toolbar = gtk_toolbar_new ();
    gtk_toolbar_set_show_arrow (GTK_TOOLBAR (toolbar), TRUE);
    gtk_toolbar_set_icon_size (GTK_TOOLBAR (toolbar),
                               GTK_ICON_SIZE_MENU);
    gtk_toolbar_set_style (GTK_TOOLBAR (toolbar),
                           GTK_TOOLBAR_ICONS);

    /* Up */
    icon = gtk_image_new_from_stock (GTK_STOCK_GO_UP,
                                     GTK_ICON_SIZE_MENU);
    item = gtk_tool_button_new (icon, NULL);
    gtk_tool_item_set_tooltip (item, tooltips, "Up", "Up");
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
    g_object_set_data (G_OBJECT (item), "moo-file-view-signal",
                       (gpointer) "go-up");
    g_signal_connect (item, "clicked",
                      G_CALLBACK (toolbar_button_clicked),
                      fileview);

    /* Back */
    icon = gtk_image_new_from_stock (GTK_STOCK_GO_BACK,
                                     GTK_ICON_SIZE_MENU);
    item = gtk_tool_button_new (icon, NULL);
    gtk_tool_item_set_tooltip (item, tooltips, "Back", "Back");
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
    g_object_set_data (G_OBJECT (item), "moo-file-view-signal",
                       (gpointer) "go-back");
    g_signal_connect (item, "clicked",
                      G_CALLBACK (toolbar_button_clicked),
                      fileview);

    /* Forward */
    icon = gtk_image_new_from_stock (GTK_STOCK_GO_FORWARD,
                                     GTK_ICON_SIZE_MENU);
    item = gtk_tool_button_new (icon, NULL);
    gtk_tool_item_set_tooltip (item, tooltips, "Forward", "Forward");
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
    g_object_set_data (G_OBJECT (item), "moo-file-view-signal",
                       (gpointer) "go-forward");
    g_signal_connect (item, "clicked",
                      G_CALLBACK (toolbar_button_clicked),
                      fileview);

    /* Home */
    icon = gtk_image_new_from_stock (GTK_STOCK_HOME,
                                     GTK_ICON_SIZE_MENU);
    item = gtk_tool_button_new (icon, NULL);
    gtk_tool_item_set_tooltip (item, tooltips, "Home", "Home");
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
    g_object_set_data (G_OBJECT (item), "moo-file-view-signal",
                       (gpointer) "go-home");
    g_signal_connect (item, "clicked",
                      G_CALLBACK (toolbar_button_clicked),
                      fileview);

    return toolbar;
}


static void         toolbar_button_clicked  (GtkToolButton  *button,
                                             MooFileView    *fileview)
{
    const char *signal = g_object_get_data (G_OBJECT (button),
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
    fileview->priv->treeview = treeview = create_treeview (fileview);
    gtk_widget_show (treeview);
    gtk_container_add (GTK_CONTAINER (swin), treeview);

    swin = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (swin);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), swin, NULL);
    fileview->priv->iconview = iconview = create_iconview (fileview);
    gtk_widget_show (iconview);
    gtk_container_add (GTK_CONTAINER (swin), iconview);

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

    return hbox;
}


static GtkWidget   *create_treeview     (MooFileView    *fileview)
{
    GtkWidget *treeview;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;

    treeview = gtk_tree_view_new_with_model (fileview->priv->filter_model);
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
    gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview), COLUMN_DISPLAY_NAME);

    g_signal_connect (treeview, "row-activated",
                      G_CALLBACK (tree_row_activated), fileview);

    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_title (column, "Name");
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

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

#if 0
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


static void         get_icon            (MooFileView    *fileview,
                                         MooFileViewFile*file)
{
    GdkPixbuf *pixbuf = NULL;

    g_return_if_fail (file != NULL);

    pixbuf = moo_get_icon_for_file (GTK_WIDGET (fileview), file,
                                    GTK_ICON_SIZE_MENU);

    if (file->pixbuf)
        g_object_unref (file->pixbuf);

    if (pixbuf)
        file->pixbuf = g_object_ref (pixbuf);
    else
        file->pixbuf = NULL;
}


#define MAX_DATE_LEN 1024
static gboolean populate_a_bit          (MooFileView    *fileview)
{
    const char *path;
    const char *name;
    GDir *dir;
    GtkTreeIter iter;
    GTimer *timer;
    gboolean done = FALSE;

    dir = fileview->priv->populate_dir;
    path = fileview->priv->current_dir;

    if (!dir || !path)
    {
        fileview->priv->populate_idle = 0;
        g_dir_close (fileview->priv->populate_dir);
        fileview->priv->populate_dir = NULL;
        g_return_val_if_reached (FALSE);
    }

    timer = g_timer_new ();

    while (TRUE)
    {
        char *fullname;
        char *size = NULL;
        MooFileViewFile *file;

        name = g_dir_read_name (dir);

        if (!name)
        {
            done = TRUE;
            break;
        }

        fullname = g_build_filename (path, name, NULL);

        file = file_new (fileview, name, fullname);
        size = g_strdup_printf ("%d", file->size);

        gtk_list_store_append (fileview->priv->store, &iter);
        gtk_list_store_set (fileview->priv->store, &iter,
                            COLUMN_FILE, file,
                            COLUMN_DISPLAY_NAME, file->display_name,
                            COLUMN_PIXBUF, file->pixbuf,
                            COLUMN_SIZE, size,
                            COLUMN_DATE, file->date_string,
                            -1);

        file_unref (file);
        g_free (fullname);
        g_free (size);

        if (g_timer_elapsed (timer, NULL) > TREEVIEW_UPDATE_TIMEOUT)
            break;
    }

    g_timer_destroy (timer);

    if (done)
    {
        fileview->priv->populate_idle = 0;
        g_dir_close (fileview->priv->populate_dir);
        fileview->priv->populate_dir = NULL;
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
#undef MAX_DATE_LEN


static gboolean populate_tree           (MooFileView    *fileview,
                                         GError        **error)
{
    const char *path = fileview->priv->current_dir;
    GtkTreeIter iter;

    g_return_val_if_fail (path != NULL, FALSE);

    if (fileview->priv->populate_idle)
    {
        g_source_remove (fileview->priv->populate_idle);
        fileview->priv->populate_idle = 0;
        g_dir_close (fileview->priv->populate_dir);
    }

    fileview->priv->populate_dir = g_dir_open (path, 0, error);
    if (!fileview->priv->populate_dir) return FALSE;

    if (populate_a_bit (fileview))
        fileview->priv->populate_idle =
                g_idle_add ((GSourceFunc) populate_a_bit, fileview);

    if (gtk_tree_model_get_iter_first (fileview->priv->filter_model, &iter))
    {
        GtkTreeSelection *selection;
        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (fileview->priv->treeview));
        gtk_tree_selection_select_iter (selection, &iter);
    }

    return TRUE;
}


static gboolean     filter_visible_func (GtkTreeModel   *model,
                                         GtkTreeIter    *iter,
                                         MooFileView    *fileview)
{
    MooFileViewFile *file;
    gboolean visible = TRUE;

    gtk_tree_model_get (model, iter, COLUMN_FILE, &file, -1);

    if (!file)
    {
        visible = FALSE;
        goto out;
    }

    if (!fileview->priv->show_hidden_files && file->basename[0] == '.')
    {
        visible = FALSE;
        goto out;
    }

out:
    file_unref (file);
    return visible;
}


static int          tree_compare_func   (GtkTreeModel   *model,
                                         GtkTreeIter    *a,
                                         GtkTreeIter    *b)
{
    MooFileViewFile *f1, *f2;
    gboolean result = 0;

    gtk_tree_model_get (model, a, COLUMN_FILE, &f1, -1);
    gtk_tree_model_get (model, b, COLUMN_FILE, &f2, -1);

    if (!f1 || !f2)
    {
        if (f1 < f2)
            result = -1;
        else if (f1 == f2)
            result = 0;
        else
            result = 1;
        goto out;
    }

    if (f1->is_dir != f2->is_dir)
    {
        if (f1->is_dir && !f2->is_dir)
            result = -1;
        else
            result = 1;
        goto out;
    }

    result = strcmp (f1->basename, f2->basename);

out:
    file_unref (f1);
    file_unref (f2);
    return result;
}


static MooFileViewFile  *file_new   (MooFileView    *fileview,
                                     const char     *basename,
                                     const char     *fullname)
{
    MooFileViewFile *file;

    g_return_val_if_fail (basename != NULL, NULL);
    g_return_val_if_fail (fullname != NULL, NULL);

    file = g_new0 (MooFileViewFile, 1);
    file->ref_count = 1;

    file->basename = g_strdup (basename);
    file->fullname = g_strdup (fullname);
    file->uri = g_strdup_printf ("file://%s", fullname);
    file->display_name = g_filename_display_basename (basename);

#ifdef USE_XDGMIME
    file->mime_type = xdg_mime_get_mime_type_for_file (fullname);
#else
    file->mime_type = NULL;
#endif


    file->time = NULL;  /* struct tm* */
    file->date_string = NULL;
    file->size = 0;

    file->is_dir = FALSE;
    file->exists = TRUE;

    if (g_stat (fullname, &file->statbuf) != 0)
    {
        if (errno == ENOENT)
        {
            gchar *display_name = g_filename_display_name (fullname);
            g_warning ("%s: file '%s' doesn't exist",
                       G_STRLOC, display_name);
            g_free (display_name);
            file->exists = FALSE;
        }
        else if (g_lstat (fullname, &file->statbuf) != 0)
        {
            int save_errno = errno;
            gchar *display_name = g_filename_display_name (fullname);
            g_warning ("%s: error getting information for '%s': %s",
                       G_STRLOC, display_name,
                       g_strerror (save_errno));
            g_free (display_name);
            file->exists = FALSE;
        }
    }

    if (file->exists)
    {
        if (S_ISDIR (file->statbuf.st_mode))
            file->is_dir = TRUE;
    }

    file->pixbuf = NULL;
    get_icon (fileview, file);

    return file;
}


static MooFileViewFile  *file_ref   (MooFileViewFile  *file)
{
    g_return_val_if_fail (file != NULL, NULL);
    file->ref_count++;
    return file;
}


static void              file_unref (MooFileViewFile  *file)
{
    if (file && !--file->ref_count)
    {
        g_free (file->basename);
        g_free (file->fullname);
        g_free (file->uri);
        g_free (file->display_name);
        if (file->pixbuf) g_object_unref (file->pixbuf);
        g_free (file->time);  /* struct tm* */
        g_free (file->date_string);
        g_free (file);
    }
}


GType moo_file_view_file_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        type = g_boxed_type_register_static ("MooFileViewFile",
                                             (GBoxedCopyFunc) file_ref,
                                             (GBoxedFreeFunc) file_unref);
    }

    return type;
}


gboolean    moo_file_view_chdir             (MooFileView    *fileview,
                                             const char     *dir,
                                             GError        **error)
{
    gboolean result;

    g_return_val_if_fail (MOO_IS_FILE_VIEW (fileview), FALSE);
    g_return_val_if_fail (dir != NULL, FALSE);

    g_signal_emit (fileview, signals[CHDIR], 0, dir, error, &result);

    return result;
}


static void         moo_file_view_go_up     (MooFileView    *fileview)
{
    char *dirname;
    GError *error = NULL;

    dirname = g_path_get_dirname (fileview->priv->current_dir);

    if (!moo_file_view_chdir (fileview, dirname, &error))
    {
        g_warning ("%s: could not go up", G_STRLOC);

        if (error)
        {
            g_warning ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }
    }

    g_free (dirname);
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
                                             GtkTreePath    *filter_treepath)
{
    GtkTreePath *treepath = NULL;
    MooFileViewFile *file = NULL;
    GtkTreeIter iter;

    treepath = gtk_tree_model_filter_convert_path_to_child_path (
            GTK_TREE_MODEL_FILTER (fileview->priv->filter_model), filter_treepath);
    g_return_if_fail (treepath != NULL);

    if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (fileview->priv->store),
         &iter, treepath))
    {
        gtk_tree_path_free (treepath);
        return;
    }

    gtk_tree_model_get (GTK_TREE_MODEL (fileview->priv->store),
                        &iter, COLUMN_FILE, &file, -1);
    if (!file)
    {
        gtk_tree_path_free (treepath);
        g_return_if_reached ();
    }

    if (file->is_dir)
    {
        GError *error = NULL;

        if (!moo_file_view_chdir (fileview, file->fullname, &error))
        {
            g_warning ("%s: could not go into '%s'",
                       G_STRLOC, file->fullname);

            if (error)
            {
                g_warning ("%s: %s", G_STRLOC, error->message);
                g_error_free (error);
            }
        }
    }
    else
    {
        g_signal_emit (fileview, signals[ACTIVATE], 0, file);
    }

    gtk_tree_path_free (treepath);
    file_unref (file);
}


static void         tree_row_activated      (G_GNUC_UNUSED GtkTreeView *treeview,
                                             GtkTreePath    *filter_treepath,
                                             G_GNUC_UNUSED GtkTreeViewColumn *column,
                                             MooFileView    *fileview)
{
    tree_path_activated (fileview, filter_treepath);
}


static void         icon_item_activated     (G_GNUC_UNUSED GtkIconView *iconview,
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
    History *hist;

    fileview->priv->history = hist = g_new0 (History, 1);
}


static void         history_free            (MooFileView    *fileview)
{
    g_list_foreach (fileview->priv->history->list, (GFunc) g_free, NULL);
    g_list_free (fileview->priv->history->list);
    g_free (fileview->priv->history);
    fileview->priv->history = NULL;
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

    iconview = gtk_icon_view_new_with_model (fileview->priv->filter_model);
    gtk_icon_view_set_text_column (GTK_ICON_VIEW (iconview),
                                   COLUMN_DISPLAY_NAME);
    gtk_icon_view_set_pixbuf_column (GTK_ICON_VIEW (iconview),
                                     COLUMN_PIXBUF);
    gtk_icon_view_set_orientation (GTK_ICON_VIEW (iconview),
                                   GTK_ORIENTATION_HORIZONTAL);

    g_signal_connect (iconview, "item-activated",
                      G_CALLBACK (icon_item_activated), fileview);

    return iconview;
}


gconstpointer moo_file_view_file_get_stat   (MooFileViewFile *file)
{
    g_return_val_if_fail (file != NULL, NULL);
    if (file->exists)
        return &file->statbuf;
    else
        return NULL;
}


const char *moo_file_view_file_path         (MooFileViewFile *file)
{
    g_return_val_if_fail (file != NULL, NULL);
    return file->fullname;
}


const char *moo_file_view_file_mime_type    (MooFileViewFile *file)
{
    g_return_val_if_fail (file != NULL, NULL);
    return file->mime_type;
}
