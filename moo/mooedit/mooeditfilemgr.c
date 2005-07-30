/*
 *   mooedit/mooeditfilemgr.c
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

#include "mooedit/mooeditfilemgr.h"
#include "mooedit/mooeditprefs.h"
#include "mooutils/moodialogs.h"
#include "mooutils/moomarshals.h"
#include <string.h>

#define PREFS_FILTERS       "filters"
#define PREFS_LAST_FILTER   PREFS_FILTERS "/last"
#define PREFS_USER          "user"
#define PREFS_RECENT        "recent_files"
#define PREFS_ENTRY         "entry"
#define NUM_USER_FILTERS    3
#define NUM_RECENT_FILES    5


typedef struct {
    GtkFileFilter   *filter;
    GtkFileFilter   *aux;
    char            *description;
    char            *glob;
} Filter;

static Filter   *filter_new     (const char *description,
                                 const char *glob);
static void      filter_free    (Filter     *filter);

static GtkFileFilter    *filter_get_gtk_filter  (Filter *filter);
static const char       *filter_get_glob        (Filter *filter);
static const char       *filter_get_description (Filter *filter);


typedef struct _RecentStuff RecentStuff;

struct _MooEditFileMgrPrivate {
    RecentStuff     *recent;
    GtkListStore    *filters;
    Filter          *last_filter;
    Filter          *null_filter;
    guint            num_user_filters;
};

enum {
    COLUMN_DESCRIPTION,
    COLUMN_FILTER,
    NUM_COLUMNS
};


static void moo_edit_file_mgr_class_init    (MooEditFileMgrClass    *klass);
static void moo_edit_file_mgr_init          (MooEditFileMgr         *mgr);
static void moo_edit_file_mgr_finalize      (GObject                *object);

static void mgr_recent_stuff_init           (MooEditFileMgr *mgr);
static void mgr_recent_stuff_free           (MooEditFileMgr *mgr);

static void      mgr_load_filter_prefs      (MooEditFileMgr *mgr);
static void      mgr_save_filter_prefs      (MooEditFileMgr *mgr);
static Filter   *mgr_new_user_filter        (MooEditFileMgr *mgr,
                                             const char     *glob);
static void      mgr_set_last_filter        (MooEditFileMgr *mgr,
                                             Filter         *filter);
static Filter   *mgr_get_last_filter        (MooEditFileMgr *mgr);
static Filter   *mgr_get_null_filter        (MooEditFileMgr *mgr);
static void      mgr_set_filter             (MooEditFileMgr *mgr,
                                             GtkFileChooser *dialog,
                                             Filter         *filter);
static void      list_store_init            (MooEditFileMgr *mgr);
static void      list_store_destroy         (MooEditFileMgr *mgr);
static void      list_store_append_filter   (MooEditFileMgr *mgr,
                                             Filter         *filter);
static Filter   *list_store_find_filter     (MooEditFileMgr *mgr,
                                             const char     *text);


/* MOO_TYPE_EDIT_FILE_MGR */
G_DEFINE_TYPE (MooEditFileMgr, moo_edit_file_mgr, G_TYPE_OBJECT)

enum {
    OPEN_RECENT,
    NUM_SIGNALS
};

static guint signals[NUM_SIGNALS];

static void moo_edit_file_mgr_class_init    (MooEditFileMgrClass   *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->finalize = moo_edit_file_mgr_finalize;

    signals[OPEN_RECENT] =
            g_signal_new ("open-recent",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooEditFileMgrClass, open_recent),
                          NULL, NULL,
                          _moo_marshal_VOID__BOXED_POINTER_OBJECT,
                          G_TYPE_NONE, 3,
                          MOO_TYPE_EDIT_FILE_INFO | G_SIGNAL_TYPE_STATIC_SCOPE,
                          G_TYPE_POINTER,
                          GTK_TYPE_MENU_ITEM);
}


static void moo_edit_file_mgr_init          (MooEditFileMgr        *mgr)
{
    mgr->priv = g_new0 (MooEditFileMgrPrivate, 1);
    list_store_init (mgr);
    mgr_recent_stuff_init (mgr);
}


static void moo_edit_file_mgr_finalize      (GObject            *object)
{
    MooEditFileMgr *mgr = MOO_EDIT_FILE_MGR (object);
    list_store_destroy (mgr);
    mgr_recent_stuff_free (mgr);
    G_OBJECT_CLASS (moo_edit_file_mgr_parent_class)->finalize (object);
}


MooEditFileMgr  *moo_edit_file_mgr_new              (void)
{
    return MOO_EDIT_FILE_MGR (g_object_new (MOO_TYPE_EDIT_FILE_MGR, NULL));
}


static gboolean row_is_separator (GtkTreeModel  *model,
                                  GtkTreeIter   *iter,
                                  G_GNUC_UNUSED gpointer data)
{
    Filter *filter;
    gtk_tree_model_get (model, iter, COLUMN_FILTER, &filter, -1);
    return filter == NULL;
}


static void     filter_entry_activated  (GtkEntry       *entry,
                                         GtkFileChooser *dialog);
static void     combo_changed           (GtkComboBox    *combo,
                                         GtkFileChooser *dialog);

static void setup_open_dialog (MooEditFileMgr   *mgr,
                               GtkWidget        *dialog)
{
    GtkWidget *alignment;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *combo;
    GtkWidget *entry;
    Filter *filter;

    alignment = gtk_alignment_new (1, 0.5, 0, 1);
    gtk_widget_show (alignment);
    gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER (dialog), alignment);

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox);
    gtk_container_add (GTK_CONTAINER (alignment), hbox);

    label = gtk_label_new ("Filter:");
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

    combo = gtk_combo_box_entry_new_with_model (GTK_TREE_MODEL (mgr->priv->filters),
            COLUMN_DESCRIPTION);
    gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo),
                                          row_is_separator,
                                          NULL, NULL);
    gtk_widget_show (combo);
    gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 0);

    entry = GTK_WIDGET (GTK_BIN(combo)->child);

    g_signal_connect (entry, "activate",
                      G_CALLBACK (filter_entry_activated),
                      dialog);
    g_signal_connect (combo, "changed",
                      G_CALLBACK (combo_changed),
                      dialog);

    g_object_set_data (G_OBJECT (dialog), "filter-combo", combo);
    g_object_set_data (G_OBJECT (dialog), "filter-entry", entry);
    g_object_set_data (G_OBJECT (dialog), "file-mgr", mgr);

    filter = mgr_get_last_filter (mgr);

    if (filter)
        mgr_set_filter (mgr, GTK_FILE_CHOOSER (dialog), filter);
}


MooEditFileInfo *moo_edit_file_mgr_open_dialog      (MooEditFileMgr *mgr,
                                                     GtkWidget      *parent)
{
    const char *filename;
    const char *title = "Open File";
    const char *start = NULL;
    MooEditFileInfo *file = NULL;
    GtkWidget *dialog;

    g_return_val_if_fail (MOO_IS_EDIT_FILE_MGR (mgr), NULL);

    start = moo_prefs_get_string (moo_edit_setting (MOO_EDIT_PREFS_DIALOGS_OPEN));

    dialog = moo_file_dialog_create (parent, MOO_DIALOG_FILE_OPEN_EXISTING,
                                     title, start);

    setup_open_dialog (mgr, dialog);
    moo_file_dialog_run (dialog);

    filename = moo_file_dialog_get_filename (dialog);

    if (filename)
    {
        char *new_start = g_path_get_dirname (filename);
        moo_prefs_set_string (moo_edit_setting (MOO_EDIT_PREFS_DIALOGS_OPEN), new_start);
        g_free (new_start);
        file = moo_edit_file_info_new (filename, NULL);
    }

    gtk_widget_destroy (dialog);
    return file;
}


MooEditFileInfo *moo_edit_file_mgr_save_as_dialog   (G_GNUC_UNUSED MooEditFileMgr *mgr,
                                                     MooEdit        *edit)
{
    const char *title = "Save File";
    const char *start = NULL;
    const char *filename = NULL;
    MooEditFileInfo *file = NULL;
    GtkWidget *dialog;

    start = moo_prefs_get_string (moo_edit_setting (MOO_EDIT_PREFS_DIALOGS_SAVE));
    if (!start)
        start = moo_prefs_get_string (moo_edit_setting (MOO_EDIT_PREFS_DIALOGS_OPEN));

    dialog = moo_file_dialog_create (GTK_WIDGET (edit), MOO_DIALOG_FILE_SAVE,
                                     title, start);

    moo_file_dialog_run (dialog);

    filename = moo_file_dialog_get_filename (dialog);

    if (filename)
    {
        char *new_start = g_path_get_dirname (filename);
        moo_prefs_set_string (moo_edit_setting (MOO_EDIT_PREFS_DIALOGS_SAVE), new_start);
        g_free (new_start);
        file = moo_edit_file_info_new (filename, NULL);
    }

    gtk_widget_destroy (dialog);
    return file;
}


static Filter   *mgr_get_null_filter        (MooEditFileMgr     *mgr)
{
    if (!mgr->priv->null_filter)
    {
        Filter *filter = filter_new ("All Files", "*");
        list_store_append_filter (mgr, filter);
        mgr->priv->null_filter = filter;
    }
    return mgr->priv->null_filter;
}

static Filter   *mgr_get_last_filter        (MooEditFileMgr     *mgr)
{
    mgr_load_filter_prefs (mgr);
    if (!mgr->priv->last_filter)
        mgr->priv->last_filter = mgr_get_null_filter (mgr);
    return mgr->priv->last_filter;
}


static void      list_store_init            (MooEditFileMgr     *mgr)
{
    mgr->priv->filters = gtk_list_store_new (NUM_COLUMNS,
                                             G_TYPE_STRING,
                                             G_TYPE_POINTER);
    mgr_get_null_filter (mgr);
}


static gboolean filter_free_func (GtkTreeModel  *model,
                                  G_GNUC_UNUSED GtkTreePath *path,
                                  GtkTreeIter   *iter,
                                  G_GNUC_UNUSED gpointer data)
{
    Filter *filter;
    gtk_tree_model_get (model, iter, COLUMN_FILTER, &filter, -1);
    if (filter)
        filter_free (filter);
    return FALSE;
}

static void      list_store_destroy         (MooEditFileMgr     *mgr)
{
    gtk_tree_model_foreach (GTK_TREE_MODEL (mgr->priv->filters),
                            filter_free_func, NULL);
    g_object_unref (mgr->priv->filters);
    mgr->priv->filters = NULL;
    mgr->priv->last_filter = NULL;
    mgr->priv->null_filter = NULL;
}


static void      list_store_append_filter   (MooEditFileMgr     *mgr,
                                             Filter             *filter)
{
    GtkTreeIter iter;
    gtk_list_store_append (mgr->priv->filters, &iter);
    if (filter)
        gtk_list_store_set (mgr->priv->filters, &iter,
                            COLUMN_DESCRIPTION, filter_get_description (filter),
                            COLUMN_FILTER, filter, -1);
}


#define NEGATE_CHAR     '!'
#define GLOB_SEPARATOR  ";"

static gboolean neg_filter_func (const GtkFileFilterInfo *filter_info,
                                 Filter *filter)
{
    return !gtk_file_filter_filter (filter->aux, filter_info);
}


static Filter   *filter_new     (const char *description,
                                 const char *glob)
{
    Filter *filter;
    char **globs, **p;
    gboolean negative;

    g_return_val_if_fail (description != NULL, NULL);
    g_return_val_if_fail (glob != NULL && glob[0] != 0, NULL);
    g_return_val_if_fail (glob[0] != NEGATE_CHAR || glob[1] != 0, NULL);

    if (glob[0] == NEGATE_CHAR)
    {
        negative = TRUE;
        globs = g_strsplit (glob + 1, GLOB_SEPARATOR, 0);
    }
    else
    {
        negative = FALSE;
        globs = g_strsplit (glob, GLOB_SEPARATOR, 0);
    }

    g_return_val_if_fail (globs != NULL, NULL);

    filter = g_new0 (Filter, 1);

    filter->description = g_strdup (description);
    filter->glob = g_strdup (glob);

    filter->filter = gtk_file_filter_new ();
    gtk_object_sink (gtk_object_ref (GTK_OBJECT (filter->filter)));
    gtk_file_filter_set_name (filter->filter, description);

    if (negative)
    {
        filter->aux = gtk_file_filter_new ();
        gtk_object_sink (gtk_object_ref (GTK_OBJECT (filter->aux)));

        for (p = globs; *p != NULL; p++)
            gtk_file_filter_add_pattern (filter->aux, *p);

        gtk_file_filter_add_custom (filter->filter,
                                    gtk_file_filter_get_needed (filter->aux),
                                    (GtkFileFilterFunc) neg_filter_func,
                                    filter, NULL);
    }
    else
    {
        for (p = globs; *p != NULL; p++)
            gtk_file_filter_add_pattern (filter->filter, *p);
    }

    g_strfreev (globs);
    return filter;
}


static void      filter_free    (Filter *filter)
{
    if (filter)
    {
        if (filter->filter)
            g_object_unref (filter->filter);
        filter->filter = NULL;
        if (filter->aux)
            g_object_unref (filter->aux);
        filter->aux = NULL;
        g_free (filter->description);
        filter->description = NULL;
        g_free (filter->glob);
        filter->glob = NULL;
        g_free (filter);
    }
}


static GtkFileFilter    *filter_get_gtk_filter  (Filter *filter)
{
    return filter->filter;
}

static const char       *filter_get_glob        (Filter *filter)
{
    return filter->glob;
}

static const char       *filter_get_description (Filter *filter)
{
    return filter->description;
}


static void     filter_entry_activated  (GtkEntry       *entry,
                                         GtkFileChooser *dialog)
{
    const char *text;
    Filter *filter;
    MooEditFileMgr *mgr;

    mgr = g_object_get_data (G_OBJECT (dialog), "file-mgr");
    g_return_if_fail (mgr != NULL);

    text = gtk_entry_get_text (entry);

    if (text && text[0])
        filter = mgr_new_user_filter (mgr, text);

    if (!filter)
        filter = mgr_get_null_filter (mgr);

    mgr_set_filter (mgr, dialog, filter);
}


static void     combo_changed           (GtkComboBox    *combo,
                                         GtkFileChooser *dialog)
{
    GtkTreeIter iter;
    Filter *filter;
    MooEditFileMgr *mgr;

    if (!gtk_combo_box_get_active_iter (combo, &iter))
        return;

    mgr = g_object_get_data (G_OBJECT (dialog), "file-mgr");
    g_return_if_fail (mgr != NULL);

    gtk_tree_model_get (GTK_TREE_MODEL (mgr->priv->filters), &iter,
                        COLUMN_FILTER, &filter, -1);
    g_return_if_fail (filter != NULL);

    mgr_set_filter (mgr, dialog, filter);
}


static void      mgr_set_filter             (MooEditFileMgr *mgr,
                                             GtkFileChooser *dialog,
                                             Filter         *filter)
{
    GtkEntry *entry = g_object_get_data (G_OBJECT (dialog),
                                         "filter-entry");
    gtk_entry_set_text (entry, filter_get_description (filter));
    gtk_file_chooser_set_filter (dialog, filter_get_gtk_filter (filter));
    mgr_set_last_filter (mgr, filter);
}


static void      mgr_set_last_filter    (MooEditFileMgr *mgr,
                                         Filter         *filter)
{
    mgr->priv->last_filter = filter;

    if (filter == mgr->priv->null_filter)
        moo_prefs_set_string (moo_edit_setting (PREFS_LAST_FILTER),
                              NULL);
    else
        moo_prefs_set_string (moo_edit_setting (PREFS_LAST_FILTER),
                              filter_get_glob (filter));
}


typedef gboolean (*ListFoundFunc) (GtkTreeModel *model,
                                   GtkTreeIter  *iter,
                                   gpointer      data);

typedef struct {
    GtkTreeIter     *iter;
    ListFoundFunc    func;
    gpointer         user_data;
    gboolean         found;
} ListStoreFindData;

static gboolean list_store_find_check_func (GtkTreeModel        *model,
                                            G_GNUC_UNUSED GtkTreePath *path,
                                            GtkTreeIter         *iter,
                                            ListStoreFindData   *data)
{
    if (data->func (model, iter, data->user_data))
    {
        data->found = TRUE;
        if (data->iter)
            *data->iter = *iter;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static gboolean list_store_find     (GtkListStore   *store,
                                     GtkTreeIter    *iter,
                                     ListFoundFunc   func,
                                     gpointer        user_data)
{
    ListStoreFindData data = {iter, func, user_data, FALSE};
    gtk_tree_model_foreach (GTK_TREE_MODEL (store),
                            (GtkTreeModelForeachFunc) list_store_find_check_func,
                            &data);
    return data.found;
}


static gboolean check_filter_match (GtkTreeModel   *model,
                                    GtkTreeIter    *iter,
                                    const char     *text)
{
    Filter *filter = NULL;

    gtk_tree_model_get (model, iter, COLUMN_FILTER, &filter, -1);

    if (!filter)
        return FALSE;

    if (!strcmp (text, filter_get_description (filter)) ||
         !strcmp (text, filter_get_glob (filter)))
            return TRUE;

    return FALSE;
}

static Filter   *list_store_find_filter     (MooEditFileMgr *mgr,
                                             const char     *text)
{
    GtkTreeIter iter;

    g_return_val_if_fail (text != NULL, NULL);

    if (list_store_find (mgr->priv->filters, &iter,
                         (ListFoundFunc)check_filter_match,
                         (gpointer)text))
    {
        Filter *filter;
        gtk_tree_model_get (GTK_TREE_MODEL (mgr->priv->filters),
                            &iter, COLUMN_FILTER, &filter, -1);
        return filter;
    }
    else
    {
        return NULL;
    }
}


static void      mgr_load_filter_prefs      (MooEditFileMgr *mgr)
{
    guint i;
    char *key;
    char *glob;
    Filter *filter;

    for (i = 0; i < NUM_USER_FILTERS; ++i)
    {
        key = g_strdup_printf (MOO_EDIT_PREFS_PREFIX "/"
                               PREFS_FILTERS "/" PREFS_USER "%d", i);

        glob = g_strdup (moo_prefs_get_string (key));

        if (glob && glob[0])
            mgr_new_user_filter (mgr, glob);

        g_free (key);
        g_free (glob);
    }

    glob = g_strdup (moo_prefs_get_string (moo_edit_setting (PREFS_LAST_FILTER)));

    if (glob && glob[0])
    {
        filter = mgr_new_user_filter (mgr, glob);

        if (filter)
            mgr_set_last_filter (mgr, filter);
    }

    g_free (glob);
}


static void set_user_filter_prefs (guint        num,
                                   const char  *val)
{
    char *key = g_strdup_printf (MOO_EDIT_PREFS_PREFIX "/"
                                 PREFS_FILTERS "/" PREFS_USER "%d", num);
    moo_prefs_set_string (key, val);
    g_free (key);
}


static void      mgr_save_filter_prefs      (MooEditFileMgr *mgr)
{
    GtkTreeIter iter;
    gboolean user_present;
    guint i = 0;

    user_present = list_store_find (mgr->priv->filters, &iter,
                                    row_is_separator, NULL);

    if (user_present)
    {
        while (i < NUM_USER_FILTERS &&
               gtk_tree_model_iter_next (GTK_TREE_MODEL (mgr->priv->filters), &iter))
        {
            Filter *filter = NULL;
            gtk_tree_model_get (GTK_TREE_MODEL (mgr->priv->filters), &iter,
                                COLUMN_FILTER, &filter, -1);
            g_assert (filter != NULL);
            set_user_filter_prefs (i, filter_get_glob (filter));
            i++;
        }
    }

    for ( ; i < NUM_USER_FILTERS; ++i)
        set_user_filter_prefs (i, NULL);
}


static Filter   *mgr_new_user_filter        (MooEditFileMgr *mgr,
                                             const char     *glob)
{
    Filter *filter;

    g_return_val_if_fail (glob && glob[0], NULL);

    filter = list_store_find_filter (mgr, glob);

    if (filter)
    {
        GtkTreeIter iter;
        gboolean user_present;

        user_present = list_store_find (mgr->priv->filters, &iter,
                                        row_is_separator, NULL);

        g_return_val_if_fail (user_present, filter);
        g_return_val_if_fail (gtk_tree_model_iter_next
                (GTK_TREE_MODEL (mgr->priv->filters), &iter), filter);
        gtk_list_store_move_before (mgr->priv->filters, &iter, NULL);
    }
    else
    {
        filter = filter_new (glob, glob);

        if (filter)
        {
            GtkTreeIter iter;
            gboolean user_present;

            user_present = list_store_find (mgr->priv->filters, &iter,
                                            row_is_separator, NULL);

            if (!user_present)
                gtk_list_store_append (mgr->priv->filters, &iter);

            if (mgr->priv->num_user_filters == NUM_USER_FILTERS)
            {
                Filter *old = NULL;

                --mgr->priv->num_user_filters;

                if (!gtk_tree_model_iter_next (GTK_TREE_MODEL (mgr->priv->filters),
                                                &iter))
                {
                    filter_free (filter);
                    g_return_val_if_reached (NULL);
                }

                gtk_tree_model_get (GTK_TREE_MODEL (mgr->priv->filters),
                                    &iter, COLUMN_FILTER, &old, -1);
                g_assert (old != NULL && old != mgr->priv->last_filter);
                gtk_list_store_remove (mgr->priv->filters, &iter);
                filter_free (old);
            }

            gtk_list_store_append (mgr->priv->filters, &iter);
            gtk_list_store_set (mgr->priv->filters, &iter,
                                COLUMN_FILTER, filter,
                                COLUMN_DESCRIPTION, filter_get_description (filter),
                                -1);
            ++mgr->priv->num_user_filters;
        }
    }

    mgr_save_filter_prefs (mgr);

    return filter;
}


/************************************************************************/
/* Recent files
 */

struct _RecentStuff {
    GSList  *files;  /* RecentEntry* */
    GSList  *menus;  /* RecentMenu* */
    gboolean prefs_loaded;
};

typedef struct {
    MooEditFileInfo *info;
} RecentEntry;

typedef struct {
    GtkMenuItem *parent;
    GtkMenu     *menu;
    gpointer     data;
    GSList      *items;     /* GtkMenuItem*, synchronized with recent files list */
} RecentMenu;

static RecentEntry  *recent_entry_new       (const MooEditFileInfo  *info);
static void          recent_entry_free      (RecentEntry            *entry);

static RecentEntry  *recent_list_find       (MooEditFileMgr     *mgr,
                                             MooEditFileInfo    *info);
static void          recent_list_delete_tail(MooEditFileMgr     *mgr);

static GtkWidget    *recent_menu_item_new   (MooEditFileMgr         *mgr,
                                             const MooEditFileInfo  *info,
                                             gpointer                data);

static void          mgr_add_recent         (MooEditFileMgr         *mgr,
                                             const MooEditFileInfo  *info);
static void          mgr_reorder_recent     (MooEditFileMgr     *mgr,
                                             RecentEntry        *top);
static void          mgr_load_recent        (MooEditFileMgr     *mgr);
static void          mgr_save_recent        (MooEditFileMgr     *mgr);

static void          save_recent            (guint               n,
                                             RecentEntry        *entry);
static RecentEntry  *load_recent            (guint               n);

static void          menu_item_activated    (GtkMenuItem        *item,
                                             MooEditFileMgr     *mgr);
static void          menu_destroyed         (GtkMenuItem        *parent,
                                             MooEditFileMgr     *mgr);


static void          mgr_recent_stuff_init  (MooEditFileMgr *mgr)
{
    mgr->priv->recent = g_new0 (RecentStuff, 1);
}


static void          mgr_recent_stuff_free  (MooEditFileMgr *mgr)
{
    GSList *l;

    for (l = mgr->priv->recent->files; l != NULL; l = l->next)
        recent_entry_free (l->data);
    g_slist_free (mgr->priv->recent->files);
    mgr->priv->recent->files = NULL;

    for (l = mgr->priv->recent->menus; l != NULL; l = l->next)
    {
        RecentMenu *menu = l->data;
        GSList *i;

        g_signal_handlers_disconnect_by_func (menu->parent,
                                              (gpointer) menu_destroyed,
                                              mgr);

        for (i = menu->items; i != NULL; i = i->next)
        {
            if (GTK_IS_MENU_ITEM (i->data))
                g_signal_handlers_disconnect_by_func (i->data,
                    (gpointer) menu_item_activated, mgr);
        }

        g_slist_free (menu->items);
        menu->items = NULL;
        g_free (menu);
    }
}


void             moo_edit_file_mgr_add_recent       (MooEditFileMgr *mgr,
                                                     MooEditFileInfo *info)
{
    RecentEntry *entry;

    g_return_if_fail (MOO_IS_EDIT_FILE_MGR (mgr));
    g_return_if_fail (info != NULL);
    g_return_if_fail (info->filename != NULL);

    mgr_load_recent (mgr);

    entry = recent_list_find (mgr, info);

    if (!entry)
        mgr_add_recent (mgr, info);
    else
        mgr_reorder_recent (mgr, entry);

    mgr_save_recent (mgr);
}


static void          mgr_add_recent     (MooEditFileMgr         *mgr,
                                         const MooEditFileInfo  *info)
{
    GSList *l;
    RecentEntry *entry = recent_entry_new (info);
    gboolean delete_last = FALSE;

    if (g_slist_length (mgr->priv->recent->files) == NUM_RECENT_FILES)
        delete_last = TRUE;

    mgr->priv->recent->files =
            g_slist_prepend (mgr->priv->recent->files, entry);

    if (delete_last)
        recent_list_delete_tail (mgr);

    for (l = mgr->priv->recent->menus; l != NULL; l = l->next)
    {
        RecentMenu *menu;
        GtkWidget *item;

        menu = l->data;

        item = recent_menu_item_new (mgr, info, menu->data);
        gtk_menu_shell_prepend (GTK_MENU_SHELL (menu->menu), item);
        menu->items = g_slist_prepend (menu->items, item);

        if (delete_last)
        {
            GSList *tail;
            tail = g_slist_last (menu->items);
            g_signal_handlers_disconnect_by_func (tail->data,
                    (gpointer) menu_item_activated, mgr);
            gtk_container_remove (GTK_CONTAINER (menu->menu), tail->data);
            menu->items = g_slist_delete_link (menu->items, tail);
        }
    }
}


static void          mgr_reorder_recent     (MooEditFileMgr     *mgr,
                                             RecentEntry        *top)
{
    int index;
    GSList *l;

    index = g_slist_index (mgr->priv->recent->files, top);
    g_return_if_fail (index >= 0);

    if (index == 0)
        return;

    mgr->priv->recent->files =
            g_slist_delete_link (mgr->priv->recent->files,
                                 g_slist_nth (mgr->priv->recent->files, index));
    mgr->priv->recent->files =
            g_slist_prepend (mgr->priv->recent->files, top);

    for (l = mgr->priv->recent->menus; l != NULL; l = l->next)
    {
        RecentMenu *menu;
        GtkWidget *item;

        menu = l->data;
        item = g_slist_nth_data (menu->items, index);
        g_assert (item != NULL);

        g_object_ref (item);

        gtk_container_remove (GTK_CONTAINER (menu->menu), item);
        gtk_menu_shell_prepend (GTK_MENU_SHELL (menu->menu), item);

        g_object_unref (item);

        menu->items = g_slist_delete_link (menu->items,
                                           g_slist_nth (menu->items, index));
        menu->items = g_slist_prepend (menu->items, item);
    }
}


static void          mgr_save_recent        (MooEditFileMgr     *mgr)
{
    guint i, num;

    num = g_slist_length (mgr->priv->recent->files);

    for (i = 0; i < num; ++i)
        save_recent (i, g_slist_nth_data (mgr->priv->recent->files, i));
    for (; i < NUM_RECENT_FILES; ++i)
        save_recent (i, NULL);
}


static void          mgr_load_recent        (MooEditFileMgr     *mgr)
{
    guint i;

    if (mgr->priv->recent->prefs_loaded)
        return;

    mgr->priv->recent->prefs_loaded = TRUE;

    for (i = 0; i < NUM_RECENT_FILES; ++i)
    {
        RecentEntry *entry = load_recent (i);
        if (entry)
            mgr->priv->recent->files =
                    g_slist_append (mgr->priv->recent->files, entry);
    }
}


static gboolean file_cmp (RecentEntry       *entry,
                          MooEditFileInfo   *info)
{
    return strcmp (entry->info->filename, info->filename);
}

static RecentEntry  *recent_list_find       (MooEditFileMgr     *mgr,
                                             MooEditFileInfo    *info)
{
    GSList *l = g_slist_find_custom (mgr->priv->recent->files,
                                     info, (GCompareFunc) file_cmp);
    if (l)
        return l->data;
    else
        return NULL;
}


static void          recent_list_delete_tail(MooEditFileMgr     *mgr)
{
    GSList *l = g_slist_last (mgr->priv->recent->files);
    g_return_if_fail (l != NULL);
    recent_entry_free (l->data);
    g_slist_delete_link (mgr->priv->recent->files, l);
}


static GtkWidget    *recent_menu_item_new   (MooEditFileMgr         *mgr,
                                             const MooEditFileInfo  *info,
                                             gpointer                data)
{
    GtkWidget *item = gtk_menu_item_new_with_label (info->filename);
    gtk_widget_show (item);
    g_signal_connect (item, "activate",
                      G_CALLBACK (menu_item_activated), mgr);
    g_object_set_data_full (G_OBJECT (item), "moo-edit-file-mgr-recent-file",
                            moo_edit_file_info_copy (info),
                            (GDestroyNotify) moo_edit_file_info_free);
    g_object_set_data (G_OBJECT (item),
                       "moo-edit-file-mgr-recent-file-data", data);
    return item;
}


static void          menu_item_activated    (GtkMenuItem        *item,
                                             MooEditFileMgr     *mgr)
{
    MooEditFileInfo *info = g_object_get_data (G_OBJECT (item),
                                               "moo-edit-file-mgr-recent-file");
    gpointer data = g_object_get_data (G_OBJECT (item),
                                       "moo-edit-file-mgr-recent-file-data");
    g_return_if_fail (info != NULL);
    g_signal_emit (mgr, signals[OPEN_RECENT], 0, info, data, item);
}


static RecentEntry  *recent_entry_new       (const MooEditFileInfo  *info)
{
    RecentEntry *entry = g_new0 (RecentEntry, 1);
    entry->info = moo_edit_file_info_copy (info);
    return entry;
}


static void          recent_entry_free      (RecentEntry        *entry)
{
    if (entry)
    {
        moo_edit_file_info_free (entry->info);
        g_free (entry);
    }
}


#define RECENT_PREFIX MOO_EDIT_PREFS_PREFIX "/" PREFS_RECENT "/" PREFS_ENTRY
#define RECENT_FILENAME RECENT_PREFIX "%d/filename"
#define RECENT_ENCODING RECENT_PREFIX "%d/encoding"

static void          save_recent            (guint               n,
                                             RecentEntry        *entry)
{
    char *key;
    const char *filename, *encoding;

    filename = entry ? entry->info->filename : NULL;
    encoding = entry ? entry->info->encoding : NULL;

    key = g_strdup_printf (RECENT_FILENAME, n);
    moo_prefs_set_string (key, filename);
    g_free (key);

    key = g_strdup_printf (RECENT_ENCODING, n);
    moo_prefs_set_string (key, encoding);
    g_free (key);
}


static RecentEntry  *load_recent            (guint               n)
{
    RecentEntry *entry = NULL;
    char *filename, *encoding;
    char *key;

    key = g_strdup_printf (RECENT_FILENAME, n);
    filename = g_strdup (moo_prefs_get_string (key));
    g_free (key);

    key = g_strdup_printf (RECENT_ENCODING, n);
    encoding = g_strdup (moo_prefs_get_string (key));
    g_free (key);

    if (filename)
    {
        MooEditFileInfo *info = moo_edit_file_info_new (filename, encoding);
        entry = recent_entry_new (info);
        moo_edit_file_info_free (info);
    }

    g_free (filename);
    g_free (encoding);

    return entry;
}

#undef RECENT_PREFIX
#undef RECENT_FILENAME
#undef RECENT_ENCODING


GtkMenuItem *moo_edit_file_mgr_create_recent_files_menu (MooEditFileMgr *mgr,
                                                         gpointer        data)
{
    RecentMenu *menu;
    GSList *l;

    mgr_load_recent (mgr);

    menu = g_new0 (RecentMenu, 1);
    menu->data = data;
    menu->parent = GTK_MENU_ITEM (gtk_menu_item_new_with_label ("Open Recent"));
    menu->menu = GTK_MENU (gtk_menu_new ());
    gtk_menu_item_set_submenu (menu->parent, GTK_WIDGET (menu->menu));

    gtk_widget_show (GTK_WIDGET (menu->parent));
    gtk_widget_show (GTK_WIDGET (menu->menu));

    g_signal_connect (menu->parent, "destroy",
                      G_CALLBACK (menu_destroyed), mgr);

    for (l = mgr->priv->recent->files; l != NULL; l = l->next)
    {
        RecentEntry *entry = l->data;
        GtkWidget *item = recent_menu_item_new (mgr, entry->info, data);
        gtk_menu_shell_append (GTK_MENU_SHELL (menu->menu), item);
        menu->items = g_slist_append (menu->items, item);
    }

    mgr->priv->recent->menus = g_slist_prepend (mgr->priv->recent->menus, menu);
    return menu->parent;
}


static gboolean menu_cmp (RecentMenu    *menu,
                          GtkMenuItem   *parent)
{
    return menu->parent != parent;
}

static void          menu_destroyed         (GtkMenuItem        *parent,
                                             MooEditFileMgr     *mgr)
{
    RecentMenu *menu;
    GSList *l = g_slist_find_custom (mgr->priv->recent->menus,
                                     parent,
                                     (GCompareFunc) menu_cmp);

    g_return_if_fail (l != NULL);

    menu = l->data;

    g_slist_free (menu->items);
    g_free (menu);

    mgr->priv->recent->menus =
            g_slist_remove (mgr->priv->recent->menus, menu);
}
