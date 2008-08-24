/*
 *   mooaccelprefs.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mooutils/mooaccelprefs.h"
#include "mooutils/mooaccel.h"
#include "mooaccelprefs-glade.h"
#include "mooutils/mooprefsdialog.h"
#include "mooutils/mooaccelbutton.h"
#include "mooutils/mooglade.h"
#include "mooutils/moodialogs.h"
#include "mooutils/mooi18n.h"
#include "mooutils/moostock.h"
#include "mooutils/mooaction-private.h"
#include "mooutils/mooactiongroup.h"
#include "mooutils/mootype-macros.h"
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>


typedef struct {
    MooPrefsDialogPage base;

    GtkAction *current_action;
    GtkTreeRowReference *current_row;

    GtkWidget *shortcut_frame;
    GtkTreeView *treeview;
    GtkTreeSelection *selection;
    GtkTreeStore *store;
    GtkToggleButton *shortcut_default;
    GtkToggleButton *shortcut_none;
    GtkToggleButton *shortcut_custom;
    MooAccelButton *shortcut;
    GtkLabel *default_label;

    GHashTable *changed;    /* Gtkction* -> Shortcut* */
    GPtrArray *actions;     /* GtkActionGroup* */
    GHashTable *groups;     /* char* -> GtkTreeRowReference* */
} MooAccelPrefsPage;

typedef MooPrefsDialogPageClass MooAccelPrefsPageClass;

#define MOO_TYPE_ACCEL_PREFS_PAGE    (_moo_accel_prefs_page_get_type ())
#define MOO_ACCEL_PREFS_PAGE(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_ACCEL_PREFS_PAGE, MooAccelPrefsPage))
MOO_DEFINE_TYPE_STATIC (MooAccelPrefsPage, _moo_accel_prefs_page, MOO_TYPE_PREFS_DIALOG_PAGE)


typedef enum {
    NONE,
    DEFAULT,
    CUSTOM
} ChoiceType;

typedef struct {
    char *accel;
    ChoiceType choice;
} Shortcut;


enum {
    COLUMN_ACTION_NAME,
    COLUMN_ACTION,
    COLUMN_ACCEL,
    N_COLUMNS
};


static void moo_accel_prefs_page_init   (MooPrefsDialogPage *page);
static void moo_accel_prefs_page_apply  (MooPrefsDialogPage *page);
static void tree_selection_changed      (MooAccelPrefsPage  *page);
static void accel_set                   (MooAccelPrefsPage  *page);
static void shortcut_none_toggled       (MooAccelPrefsPage  *page);
static void shortcut_default_toggled    (MooAccelPrefsPage  *page);
static void shortcut_custom_toggled     (MooAccelPrefsPage  *page);




static Shortcut *
shortcut_new (ChoiceType  choice,
              const char *accel)
{
    Shortcut *s = g_new (Shortcut, 1);
    s->choice = choice;
    s->accel = g_strdup (accel);
    return s;
}

static void
shortcut_free (Shortcut *s)
{
    g_free (s->accel);
    g_free (s);
}


static void
_moo_accel_prefs_page_finalize (GObject *object)
{
    MooAccelPrefsPage *page = MOO_ACCEL_PREFS_PAGE (object);

    g_hash_table_destroy (page->changed);
    g_hash_table_destroy (page->groups);
    g_ptr_array_free (page->actions, TRUE);

    G_OBJECT_CLASS (_moo_accel_prefs_page_parent_class)->finalize (object);
}


static void
_moo_accel_prefs_page_class_init (MooAccelPrefsPageClass *klass)
{
    G_OBJECT_CLASS (klass)->finalize = _moo_accel_prefs_page_finalize;
    MOO_PREFS_DIALOG_PAGE_CLASS (klass)->init = moo_accel_prefs_page_init;
    MOO_PREFS_DIALOG_PAGE_CLASS (klass)->apply = moo_accel_prefs_page_apply;
}


static void
row_activated (MooAccelPrefsPage *page)
{
    if (GTK_WIDGET_IS_SENSITIVE (page->shortcut))
        gtk_button_clicked (GTK_BUTTON (page->shortcut));
}


static void
block_accel_set (MooAccelPrefsPage *page)
{
    g_signal_handlers_block_matched (page->shortcut, G_SIGNAL_MATCH_FUNC,
                                     0, 0, 0, (gpointer) accel_set, 0);
}

static void
unblock_accel_set (MooAccelPrefsPage *page)
{
    g_signal_handlers_unblock_matched (page->shortcut, G_SIGNAL_MATCH_FUNC,
                                       0, 0, 0, (gpointer) accel_set, 0);
}

static void
block_radio (MooAccelPrefsPage *page)
{
    g_signal_handlers_block_matched (page->shortcut_none, G_SIGNAL_MATCH_FUNC,
                                     0, 0, 0, (gpointer) shortcut_none_toggled, 0);
    g_signal_handlers_block_matched (page->shortcut_default, G_SIGNAL_MATCH_FUNC,
                                     0, 0, 0, (gpointer) shortcut_default_toggled, 0);
    g_signal_handlers_block_matched (page->shortcut_custom, G_SIGNAL_MATCH_FUNC,
                                     0, 0, 0, (gpointer) shortcut_custom_toggled, 0);
}

static void
unblock_radio (MooAccelPrefsPage *page)
{
    g_signal_handlers_unblock_matched (page->shortcut_none, G_SIGNAL_MATCH_FUNC,
                                       0, 0, 0, (gpointer) shortcut_none_toggled, 0);
    g_signal_handlers_unblock_matched (page->shortcut_default, G_SIGNAL_MATCH_FUNC,
                                       0, 0, 0, (gpointer) shortcut_default_toggled, 0);
    g_signal_handlers_unblock_matched (page->shortcut_custom, G_SIGNAL_MATCH_FUNC,
                                       0, 0, 0, (gpointer) shortcut_custom_toggled, 0);
}


static void
_moo_accel_prefs_page_init (MooAccelPrefsPage *page)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    MooGladeXML *xml;

    page->changed = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL,
                                           (GDestroyNotify) shortcut_free);
    page->actions = g_ptr_array_new ();
    page->groups = g_hash_table_new_full (g_str_hash, g_str_equal,
                                          (GDestroyNotify) g_free,
                                          (GDestroyNotify) gtk_tree_row_reference_free);

    xml = moo_glade_xml_new_empty (GETTEXT_PACKAGE);
    moo_glade_xml_fill_widget (xml, GTK_WIDGET (page),
                               mooaccelprefs_glade_xml, -1,
                               "page", NULL);
    g_object_set (page, "label", "Shortcuts", "icon-stock-id", MOO_STOCK_KEYBOARD, NULL);

    page->treeview = moo_glade_xml_get_widget (xml, "treeview");
    gtk_tree_view_set_search_column (page->treeview, 0);

    g_signal_connect_swapped (page->treeview, "row-activated",
                              G_CALLBACK (row_activated),
                              page);

    page->shortcut_frame = moo_glade_xml_get_widget (xml, "shortcut_frame");
    page->shortcut_default = moo_glade_xml_get_widget (xml, "shortcut_default");
    page->shortcut_none = moo_glade_xml_get_widget (xml, "shortcut_none");
    page->shortcut_custom = moo_glade_xml_get_widget (xml, "shortcut_custom");
    page->shortcut = moo_glade_xml_get_widget (xml, "shortcut");
    page->default_label = moo_glade_xml_get_widget (xml, "default_label");

    g_object_unref (xml);

    page->store = gtk_tree_store_new (N_COLUMNS,
                                      G_TYPE_STRING,
                                      GTK_TYPE_ACTION,
                                      G_TYPE_STRING);
    gtk_tree_view_set_model (page->treeview, GTK_TREE_MODEL (page->store));
    g_object_unref (page->store);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Action", renderer,
                                                       "text", COLUMN_ACTION_NAME,
                                                       NULL);
    gtk_tree_view_append_column (page->treeview, column);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_ACTION_NAME);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Shortcut", renderer,
                                                       "text", COLUMN_ACCEL,
                                                       NULL);
    gtk_tree_view_append_column (page->treeview, column);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_ACCEL);

    page->selection = gtk_tree_view_get_selection (page->treeview);
    gtk_tree_selection_set_mode (page->selection, GTK_SELECTION_SINGLE);

    g_signal_connect_swapped (page->selection, "changed",
                              G_CALLBACK (tree_selection_changed),
                              page);

    g_signal_connect_swapped (page->shortcut, "accel-set",
                              G_CALLBACK (accel_set),
                              page);
    g_signal_connect_swapped (page->shortcut_none, "toggled",
                              G_CALLBACK (shortcut_none_toggled),
                              page);
    g_signal_connect_swapped (page->shortcut_default, "toggled",
                              G_CALLBACK (shortcut_default_toggled),
                              page);
    g_signal_connect_swapped (page->shortcut_custom, "toggled",
                              G_CALLBACK (shortcut_custom_toggled),
                              page);
}


static const char *
get_action_accel (GtkAction *action)
{
    const char *accel_path = _moo_action_get_accel_path (action);
    return _moo_get_accel (accel_path);
}


static void
apply_one (GtkAction *action,
           Shortcut  *shortcut)
{
    const char *accel_path = _moo_action_get_accel_path (action);
    const char *default_accel = _moo_get_default_accel (accel_path);
    const char *new_accel = "";

    switch (shortcut->choice)
    {
        case NONE:
            new_accel = "";
            break;
        case CUSTOM:
            new_accel = shortcut->accel;
            break;
        case DEFAULT:
            new_accel = default_accel;
            break;
        default:
            g_return_if_reached ();
    }

    _moo_modify_accel (accel_path, new_accel);
}

static void
moo_accel_prefs_page_apply (MooPrefsDialogPage *prefs_page)
{
    MooAccelPrefsPage *page = MOO_ACCEL_PREFS_PAGE (prefs_page);
    g_hash_table_foreach (page->changed, (GHFunc) apply_one, NULL);
    g_hash_table_foreach_remove (page->changed, (GHRFunc) gtk_true, NULL);
}


static char *
get_accel_label_for_path (const char *accel_path)
{
    g_return_val_if_fail (accel_path != NULL, g_strdup (""));
    return _moo_get_accel_label (_moo_get_accel (accel_path));
}


static gboolean
add_row (GtkActionGroup    *group,
         GtkAction         *action,
         MooAccelPrefsPage *page)
{
    const char *group_name;
    char *accel;
    const char *name;
    GtkTreeIter iter;
    GtkTreeRowReference *row;

    if (_moo_action_get_no_accel (action) || !_moo_action_get_accel_editable (action))
        return FALSE;

    group_name = _moo_action_group_get_display_name (MOO_ACTION_GROUP (group));

    if (!group_name)
        group_name = "";

    row = g_hash_table_lookup (page->groups, group_name);

    if (row)
    {
        GtkTreeIter parent;
        GtkTreePath *path = gtk_tree_row_reference_get_path (row);
        if (!path || !gtk_tree_model_get_iter (GTK_TREE_MODEL (page->store), &parent, path))
        {
            g_critical ("%s: got invalid path for group %s",
                        G_STRLOC, group_name);
            gtk_tree_path_free (path);
            return FALSE;
        }
        gtk_tree_path_free (path);

        gtk_tree_store_append (page->store, &iter, &parent);
    }
    else
    {
        GtkTreeIter group_iter;
        GtkTreePath *path;

        gtk_tree_store_append (page->store, &group_iter, NULL);
        gtk_tree_store_set (page->store, &group_iter,
                            COLUMN_ACTION_NAME, group_name,
                            COLUMN_ACTION, NULL,
                            COLUMN_ACCEL, NULL,
                            -1);
        path = gtk_tree_model_get_path (GTK_TREE_MODEL (page->store), &group_iter);
        g_hash_table_insert (page->groups,
                             g_strdup (group_name),
                             gtk_tree_row_reference_new (GTK_TREE_MODEL (page->store), path));
        gtk_tree_path_free (path);

        gtk_tree_store_append (page->store, &iter, &group_iter);
    }

    accel = get_accel_label_for_path (_moo_action_get_accel_path (action));
    name = _moo_action_get_display_name (action);

    gtk_tree_store_set (page->store, &iter,
                        COLUMN_ACTION_NAME, name,
                        COLUMN_ACTION, action,
                        COLUMN_ACCEL, accel,
                        -1);
    g_free (accel);

    return FALSE;
}


static void
moo_accel_prefs_page_init (MooPrefsDialogPage *prefs_page)
{
    guint i;
    MooAccelPrefsPage *page = MOO_ACCEL_PREFS_PAGE (prefs_page);

    for (i = 0; i < page->actions->len; ++i)
    {
        GtkActionGroup *group = g_ptr_array_index (page->actions, i);
        GList *list = gtk_action_group_list_actions (group);

        while (list)
        {
            add_row (group, list->data, page);
            list = g_list_delete_link (list, list);
        }
    }

    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (page->store),
                                          COLUMN_ACTION_NAME,
                                          GTK_SORT_ASCENDING);
    gtk_tree_view_expand_all (page->treeview);
    tree_selection_changed (page);
}


static void
tree_selection_changed (MooAccelPrefsPage *page)
{
    gboolean selected_action = FALSE;
    GtkTreeIter iter;
    GtkAction *action = NULL;
    GtkTreePath *path;
    char *default_label;
    const char *default_accel, *new_accel = "";
    GtkToggleButton *new_button = NULL;
    Shortcut *shortcut;

    if (gtk_tree_selection_get_selected (page->selection, NULL, &iter))
    {
        gtk_tree_model_get (GTK_TREE_MODEL (page->store), &iter,
                            COLUMN_ACTION, &action, -1);
        if (action)
            selected_action = TRUE;
    }

    page->current_action = action;
    gtk_tree_row_reference_free (page->current_row);
    page->current_row = NULL;

    if (!selected_action)
    {
        gtk_label_set_text (page->default_label, "");
        gtk_widget_set_sensitive (page->shortcut_frame, FALSE);
        return;
    }

    path = gtk_tree_model_get_path (GTK_TREE_MODEL (page->store), &iter);
    page->current_row = gtk_tree_row_reference_new (GTK_TREE_MODEL (page->store), path);
    gtk_tree_path_free (path);

    gtk_widget_set_sensitive (page->shortcut_frame, TRUE);

    default_accel = _moo_get_default_accel (_moo_action_get_accel_path (action));
    default_label = _moo_get_accel_label (default_accel);

    if (!default_label || !default_label[0])
        gtk_label_set_text (page->default_label, "None");
    else
        gtk_label_set_text (page->default_label, default_label);

    block_radio (page);
    block_accel_set (page);

    shortcut = g_hash_table_lookup (page->changed, action);

    if (shortcut)
    {
        switch (shortcut->choice)
        {
            case NONE:
                new_button = page->shortcut_none;
                new_accel = NULL;
                break;

            case DEFAULT:
                new_button = page->shortcut_default;
                new_accel = default_accel;
                break;

            case CUSTOM:
                new_button = page->shortcut_custom;
                new_accel = shortcut->accel;
                break;

            default:
                g_assert_not_reached ();
        }
    }
    else
    {
        const char *accel = get_action_accel (action);

        if (!strcmp (accel, default_accel))
        {
            new_button = page->shortcut_default;
            new_accel = default_accel;
        }
        else if (!accel[0])
        {
            new_button = page->shortcut_none;
            new_accel = NULL;
        }
        else
        {
            new_button = page->shortcut_custom;
            new_accel = accel;
        }
    }

    gtk_toggle_button_set_active (new_button, TRUE);
    _moo_accel_button_set_accel (page->shortcut, new_accel);

    unblock_radio (page);
    unblock_accel_set (page);

    g_free (default_label);
    g_object_unref (action);
}


static void
accel_set (MooAccelPrefsPage *page)
{
    GtkTreeIter iter;
    GtkTreePath *path;
    const char *accel;
    const char *label;

    g_return_if_fail (page->current_row != NULL && page->current_action != NULL);

    block_radio (page);
    gtk_toggle_button_set_active (page->shortcut_custom, TRUE);
    unblock_radio (page);

    path = gtk_tree_row_reference_get_path (page->current_row);
    if (!path || !gtk_tree_model_get_iter (GTK_TREE_MODEL (page->store), &iter, path))
    {
        g_critical ("%s: got invalid path", G_STRLOC);
        gtk_tree_path_free (path);
        return;
    }
    gtk_tree_path_free (path);

    accel = _moo_accel_button_get_accel (page->shortcut);
    label = gtk_button_get_label (GTK_BUTTON (page->shortcut));
    gtk_tree_store_set (page->store, &iter, COLUMN_ACCEL, label, -1);

    if (accel && accel[0])
        g_hash_table_insert (page->changed,
                             page->current_action,
                             shortcut_new (CUSTOM, accel));
    else
        g_hash_table_insert (page->changed,
                             page->current_action,
                             shortcut_new (NONE, ""));
}


static void
shortcut_none_toggled (MooAccelPrefsPage *page)
{
    GtkTreeIter iter;
    GtkTreePath *path;

    g_return_if_fail (page->current_row != NULL && page->current_action != NULL);

    if (!gtk_toggle_button_get_active (page->shortcut_none))
        return;

    path = gtk_tree_row_reference_get_path (page->current_row);
    if (!path || !gtk_tree_model_get_iter (GTK_TREE_MODEL (page->store), &iter, path))
    {
        g_critical ("%s: got invalid path", G_STRLOC);
        gtk_tree_path_free (path);
        return;
    }
    gtk_tree_path_free (path);

    gtk_tree_store_set (page->store, &iter, COLUMN_ACCEL, NULL, -1);
    g_hash_table_insert (page->changed,
                         page->current_action,
                         shortcut_new (NONE, ""));
    block_accel_set (page);
    _moo_accel_button_set_accel (page->shortcut, "");
    unblock_accel_set (page);
}


static void
shortcut_default_toggled (MooAccelPrefsPage *page)
{
    GtkTreeIter iter;
    GtkTreePath *path;
    Shortcut *current_shortcut;
    const char *default_accel;

    g_return_if_fail (page->current_row != NULL && page->current_action != NULL);

    if (!gtk_toggle_button_get_active (page->shortcut_default))
        return;

    path = gtk_tree_row_reference_get_path (page->current_row);

    if (!path || !gtk_tree_model_get_iter (GTK_TREE_MODEL (page->store), &iter, path))
    {
        g_critical ("%s: got invalid path", G_STRLOC);
        gtk_tree_path_free (path);
        return;
    }

    gtk_tree_path_free (path);

    current_shortcut = g_hash_table_lookup (page->changed, page->current_action);

    if (!current_shortcut)
    {
        current_shortcut = shortcut_new (NONE, "");
        g_hash_table_insert (page->changed,
                             page->current_action,
                             current_shortcut);
    }

    default_accel = _moo_action_get_default_accel (page->current_action);

    if (default_accel[0])
    {
        char *label = _moo_get_accel_label (default_accel);
        gtk_tree_store_set (page->store, &iter, COLUMN_ACCEL, label, -1);
        g_free (label);
    }
    else
    {
        gtk_tree_store_set (page->store, &iter, COLUMN_ACCEL, NULL, -1);
    }

    current_shortcut->choice = DEFAULT;
    g_free (current_shortcut->accel);
    current_shortcut->accel = g_strdup (default_accel);

    block_accel_set (page);
    _moo_accel_button_set_accel (page->shortcut, default_accel);
    unblock_accel_set (page);
}


static void
shortcut_custom_toggled (MooAccelPrefsPage *page)
{
    GtkTreeIter iter;
    GtkTreePath *path;
    Shortcut *shortcut;

    g_return_if_fail (page->current_row != NULL && page->current_action != NULL);

    if (!gtk_toggle_button_get_active (page->shortcut_custom))
        return;

    path = gtk_tree_row_reference_get_path (page->current_row);

    if (!path || !gtk_tree_model_get_iter (GTK_TREE_MODEL (page->store), &iter, path))
    {
        g_critical ("%s: got invalid path", G_STRLOC);
        gtk_tree_path_free (path);
        return;
    }

    gtk_tree_path_free (path);

    shortcut = g_hash_table_lookup (page->changed, page->current_action);

    if (shortcut)
    {
        block_accel_set (page);
        _moo_accel_button_set_accel (page->shortcut, shortcut->accel);
        unblock_accel_set (page);

        gtk_tree_store_set (page->store, &iter, COLUMN_ACCEL,
                            gtk_button_get_label (GTK_BUTTON (page->shortcut)),
                            -1);

        shortcut->choice = CUSTOM;
    }
    else
    {
        gtk_button_clicked (GTK_BUTTON (page->shortcut));
    }
}


static void
dialog_response (GObject *page,
                 int response)
{
    switch (response)
    {
        case GTK_RESPONSE_OK:
            g_signal_emit_by_name (page, "apply", NULL);
            break;

        case GTK_RESPONSE_REJECT:
            g_signal_emit_by_name (page, "set_defaults", NULL);
            break;
    };
}


static MooAccelPrefsPage *
_moo_accel_prefs_page_new (MooActionCollection *collection)
{
    const GSList *groups;
    MooAccelPrefsPage *page;

    page = g_object_new (MOO_TYPE_ACCEL_PREFS_PAGE, NULL);
    groups = moo_action_collection_get_groups (collection);

    while (groups)
    {
        g_ptr_array_add (page->actions, groups->data);
        groups = groups->next;
    }

    return page;
}


static GtkWidget*
_moo_accel_prefs_dialog_new (MooActionCollection *collection)
{
    MooAccelPrefsPage *page;
    GtkWidget *dialog, *page_holder;
    MooGladeXML *xml;

    xml = moo_glade_xml_new_from_buf (mooaccelprefs_glade_xml, -1,
                                      "dialog", GETTEXT_PACKAGE, NULL);
    g_return_val_if_fail (xml != NULL, NULL);

    dialog = moo_glade_xml_get_widget (xml, "dialog");

    page = _moo_accel_prefs_page_new (collection);
    gtk_widget_show (GTK_WIDGET (page));
    page_holder = moo_glade_xml_get_widget (xml, "page_holder");
    gtk_container_add (GTK_CONTAINER (page_holder), GTK_WIDGET (page));

    g_object_unref (xml);

    gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                             GTK_RESPONSE_OK,
                                             GTK_RESPONSE_CANCEL,
                                             -1);

    g_signal_connect_swapped (dialog, "response",
                              G_CALLBACK (dialog_response), page);
    g_signal_emit_by_name (page, "init", NULL);

    return dialog;
}


void
_moo_accel_prefs_dialog_run (MooActionCollection *collection,
                             GtkWidget           *parent)
{
    GtkWidget *dialog = _moo_accel_prefs_dialog_new (collection);

    moo_window_set_parent (dialog, parent);

    while (TRUE)
    {
        int response = gtk_dialog_run (GTK_DIALOG (dialog));

        if (response != GTK_RESPONSE_REJECT)
            break;
    }

    gtk_widget_destroy (dialog);
}
