/*
 *   mooui/mooshortcutsprefs.c
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

#include "mooui/mooshortcutsprefs.h"
#include "mooui/mooshortcutsprefs-glade.h"
#include "mooui/mooaccel.h"
#include "mooutils/mooaccelbutton.h"
#include "mooutils/moostock.h"
#include "mooutils/mooprefs.h"
#include "mooutils/mooglade.h"
#include <gtk/gtk.h>
#include <string.h>


typedef enum {
    NONE,
    DEFAULT,
    CUSTOM
} ChoiceType;

typedef struct {
    char *accel;
    ChoiceType choice;
} Shortcut;

static Shortcut *shortcut_new   (ChoiceType  choice,
                                 const char *accel)
{
    Shortcut *s = g_new (Shortcut, 1);
    s->choice = choice;
    s->accel = g_strdup (accel);
    return s;
}

static void      shortcut_free  (Shortcut   *s)
{
    g_free (s->accel);
    g_free (s);
}


typedef struct {
    MooAction *current_action;
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

    GHashTable *changed;    /* MooAction* -> Shortcut* */
    GPtrArray *actions;     /* MooActionGroup* */
    GHashTable *groups;     /* char* -> GtkTreeRowReference* */
} Stuff;

static Stuff    *stuff_new  (void)
{
    Stuff *s = g_new0 (Stuff, 1);

    s->changed = g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                        NULL,
                                        (GDestroyNotify) shortcut_free);
    s->actions = g_ptr_array_new ();
    s->groups = g_hash_table_new_full (g_str_hash, g_str_equal,
                                       (GDestroyNotify) g_free,
                                       (GDestroyNotify) gtk_tree_row_reference_free);

    return s;
}

static void      stuff_free (Stuff *s)
{
    g_hash_table_destroy (s->changed);
    g_hash_table_destroy (s->groups);
    g_ptr_array_free (s->actions, TRUE);
    g_free (s);
}


enum {
    COLUMN_ACTION_NAME,
    COLUMN_ACTION,
    COLUMN_ACCEL,
    N_COLUMNS
};


static void init                        (Stuff *stuff);
static void apply                       (Stuff *stuff);
static void tree_selection_changed      (Stuff *stuff);
static void accel_set                   (Stuff *stuff);
static void shortcut_none_toggled       (Stuff *stuff);
static void shortcut_default_toggled    (Stuff *stuff);
static void shortcut_custom_toggled     (Stuff *stuff);

static void row_activated (Stuff *stuff)
{
    if (GTK_WIDGET_IS_SENSITIVE (stuff->shortcut))
        gtk_button_clicked (GTK_BUTTON (stuff->shortcut));
}


inline static void block_accel_set (Stuff *stuff)
{
    g_signal_handlers_block_matched (stuff->shortcut, G_SIGNAL_MATCH_FUNC,
                                     0, 0, 0, (gpointer)accel_set, 0);
}
inline static void unblock_accel_set (Stuff *stuff)
{
    g_signal_handlers_unblock_matched (stuff->shortcut, G_SIGNAL_MATCH_FUNC,
                                       0, 0, 0, (gpointer)accel_set, 0);
}

inline static void block_radio (Stuff *stuff)
{
    g_signal_handlers_block_matched (stuff->shortcut_none, G_SIGNAL_MATCH_FUNC,
                                     0, 0, 0, (gpointer)shortcut_none_toggled, 0);
    g_signal_handlers_block_matched (stuff->shortcut_default, G_SIGNAL_MATCH_FUNC,
                                     0, 0, 0, (gpointer)shortcut_default_toggled, 0);
    g_signal_handlers_block_matched (stuff->shortcut_custom, G_SIGNAL_MATCH_FUNC,
                                     0, 0, 0, (gpointer)shortcut_custom_toggled, 0);
}
inline static void unblock_radio (Stuff *stuff)
{
    g_signal_handlers_unblock_matched (stuff->shortcut_none, G_SIGNAL_MATCH_FUNC,
                                       0, 0, 0, (gpointer)shortcut_none_toggled, 0);
    g_signal_handlers_unblock_matched (stuff->shortcut_default, G_SIGNAL_MATCH_FUNC,
                                       0, 0, 0, (gpointer)shortcut_default_toggled, 0);
    g_signal_handlers_unblock_matched (stuff->shortcut_custom, G_SIGNAL_MATCH_FUNC,
                                       0, 0, 0, (gpointer)shortcut_custom_toggled, 0);
}


GtkWidget *moo_shortcuts_prefs_page_new (MooActionGroup *actions)
{
    GtkWidget *page, *page_content;
    Stuff *stuff;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    MooGladeXML *xml;

    xml = moo_glade_xml_new_empty ();
    moo_glade_xml_map_class (xml, "GtkButton", MOO_TYPE_ACCEL_BUTTON);
    moo_glade_xml_parse_memory (xml, MOO_SHORTCUTS_PREFS_GLADE_UI, -1, "page");

    page = moo_prefs_dialog_page_new ("Shortcuts", MOO_STOCK_KEYBOARD);

    page_content = moo_glade_xml_get_widget (xml, "page");
    gtk_box_pack_start (GTK_BOX (page), page_content, TRUE, TRUE, 0);

    stuff = stuff_new ();
    if (actions)
        g_ptr_array_add (stuff->actions, actions);

    g_object_set_data_full (G_OBJECT (page), "ShortcutsPrefsStuff",
                            stuff, (GDestroyNotify) stuff_free);

    g_signal_connect_swapped (page, "apply",
                              G_CALLBACK (apply),
                              stuff);
    g_signal_connect_swapped (page, "init",
                              G_CALLBACK (init),
                              stuff);

    stuff->treeview = moo_glade_xml_get_widget (xml, "treeview");
    gtk_tree_view_set_headers_clickable (stuff->treeview, TRUE);
    gtk_tree_view_set_search_column (stuff->treeview, 0);

    g_signal_connect_swapped (stuff->treeview, "row-activated",
                              G_CALLBACK (row_activated),
                              stuff);

    stuff->shortcut_frame = moo_glade_xml_get_widget (xml, "shortcut_frame");
    stuff->shortcut_default = moo_glade_xml_get_widget (xml, "shortcut_default");
    stuff->shortcut_none = moo_glade_xml_get_widget (xml, "shortcut_none");
    stuff->shortcut_custom = moo_glade_xml_get_widget (xml, "shortcut_custom");
    stuff->shortcut = moo_glade_xml_get_widget (xml, "shortcut");
    stuff->default_label = moo_glade_xml_get_widget (xml, "default_label");

    moo_glade_xml_unref (xml);

    stuff->store = gtk_tree_store_new (N_COLUMNS,
                                       G_TYPE_STRING,
                                       MOO_TYPE_ACTION,
                                       G_TYPE_STRING);
    gtk_tree_view_set_model (stuff->treeview,
                             GTK_TREE_MODEL (stuff->store));
    g_object_unref (G_OBJECT (stuff->store));

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Action",
                                                       renderer,
                                                       "text", COLUMN_ACTION_NAME,
                                                       NULL);
    gtk_tree_view_append_column (stuff->treeview, column);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_ACTION_NAME);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Shortcut",
                                                       renderer,
                                                       "text", COLUMN_ACCEL,
                                                       NULL);
    gtk_tree_view_append_column (stuff->treeview, column);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_ACCEL);

    stuff->selection = gtk_tree_view_get_selection (stuff->treeview);
    gtk_tree_selection_set_mode (stuff->selection, GTK_SELECTION_SINGLE);

    gtk_tree_view_set_headers_clickable (stuff->treeview, TRUE);

    g_signal_connect_swapped (stuff->selection, "changed",
                              G_CALLBACK (tree_selection_changed),
                              stuff);

    g_signal_connect_swapped (stuff->shortcut, "accel-set",
                              G_CALLBACK (accel_set),
                              stuff);
    g_signal_connect_swapped (stuff->shortcut_none, "toggled",
                              G_CALLBACK (shortcut_none_toggled),
                              stuff);
    g_signal_connect_swapped (stuff->shortcut_default, "toggled",
                              G_CALLBACK (shortcut_default_toggled),
                              stuff);
    g_signal_connect_swapped (stuff->shortcut_custom, "toggled",
                              G_CALLBACK (shortcut_custom_toggled),
                              stuff);

    return page;
}


static void apply_one (MooAction    *action,
                       Shortcut     *shortcut,
                       G_GNUC_UNUSED Stuff        *stuff)
{
    const char *accel_path = moo_action_get_accel_path (action);
    const char *accel = moo_action_get_accel (action);
    const char *default_accel = moo_action_get_default_accel (action);

    switch (shortcut->choice) {
        case NONE:
            if (accel[0])
                moo_action_set_accel (action, "");
            moo_prefs_set_accel (accel_path, "");
            break;

        case CUSTOM:
            if (strcmp (accel, shortcut->accel))
                moo_action_set_accel (action, shortcut->accel);
            moo_prefs_set_accel (accel_path, shortcut->accel);
            break;

        case DEFAULT:
            if (strcmp (accel, default_accel))
                moo_action_set_accel (action, default_accel);
            moo_prefs_set_accel (accel_path, NULL);
            break;

        default:
            g_assert_not_reached ();
    }
}

static void apply (Stuff *stuff)
{
    g_hash_table_foreach (stuff->changed,
                          (GHFunc) apply_one,
                          stuff);
    g_hash_table_foreach_remove (stuff->changed,
                                 (GHRFunc) gtk_true,
                                 NULL);
}


static gboolean add_row (MooActionGroup *group,
                         MooAction      *action,
                         Stuff          *stuff)
{
    const char *group_name;
    char *accel;
    const char *name;
    GtkTreeIter iter;
    GtkTreeRowReference *row;

    if (moo_action_get_no_accel (action))
        return FALSE;

    group_name = moo_action_group_get_name (group);

    if (!group_name)
        group_name = "";

    row = g_hash_table_lookup (stuff->groups, group_name);
    if (row)
    {
        GtkTreeIter parent;
        GtkTreePath *path = gtk_tree_row_reference_get_path (row);
        if (!path || !gtk_tree_model_get_iter (GTK_TREE_MODEL (stuff->store), &parent, path))
        {
            g_critical ("%s: got invalid path for group %s",
                        G_STRLOC, group_name);
            gtk_tree_path_free (path);
            return FALSE;
        }
        gtk_tree_path_free (path);

        gtk_tree_store_append (stuff->store, &iter, &parent);
    }
    else
    {
        GtkTreeIter group;
        GtkTreePath *path;

        gtk_tree_store_append (stuff->store, &group, NULL);
        gtk_tree_store_set (stuff->store, &group,
                            COLUMN_ACTION_NAME, group_name,
                            COLUMN_ACTION, NULL,
                            COLUMN_ACCEL, NULL,
                            -1);
        path = gtk_tree_model_get_path (GTK_TREE_MODEL (stuff->store), &group);
        g_hash_table_insert (stuff->groups,
                             g_strdup (group_name),
                             gtk_tree_row_reference_new (GTK_TREE_MODEL (stuff->store), path));
        gtk_tree_path_free (path);

        gtk_tree_store_append (stuff->store, &iter, &group);
    }

    accel = moo_action_get_accel_label (action);
    name = moo_action_get_name (action);
    if (!name)
        name = moo_action_get_id (action);
    gtk_tree_store_set (stuff->store, &iter,
                        COLUMN_ACTION_NAME, name,
                        COLUMN_ACTION, action,
                        COLUMN_ACCEL, accel,
                        -1);
    g_free (accel);

    return FALSE;
}


static void init (Stuff *stuff)
{
    guint i;
    for (i = 0; i < stuff->actions->len; ++i)
        moo_action_group_foreach (g_ptr_array_index(stuff->actions, i),
                                  (MooActionGroupForeachFunc)add_row,
                                  stuff);
    gtk_tree_view_expand_all (stuff->treeview);
    tree_selection_changed (stuff);
}


static void tree_selection_changed (Stuff *stuff)
{
    gboolean selected_action = FALSE;
    GtkTreeIter iter;
    MooAction *action = NULL;
    GtkTreePath *path;
    char *default_label;
    const char *default_accel;
    Shortcut *shortcut;

    if (gtk_tree_selection_get_selected (stuff->selection, NULL, &iter))
    {
        gtk_tree_model_get (GTK_TREE_MODEL (stuff->store), &iter,
                            COLUMN_ACTION, &action, -1);
        if (action)
            selected_action = TRUE;
    }

    stuff->current_action = action;
    gtk_tree_row_reference_free (stuff->current_row);
    stuff->current_row = NULL;

    if (!selected_action)
    {
        gtk_label_set_text (stuff->default_label, "");
        gtk_widget_set_sensitive (stuff->shortcut_frame, FALSE);
        return;
    }

    path = gtk_tree_model_get_path (GTK_TREE_MODEL (stuff->store), &iter);
    stuff->current_row =
        gtk_tree_row_reference_new (GTK_TREE_MODEL (stuff->store), path);
    gtk_tree_path_free (path);

    gtk_widget_set_sensitive (stuff->shortcut_frame, TRUE);
    default_label = moo_action_get_default_accel_label (action);
    default_accel = moo_action_get_default_accel (action);
    if (!default_label || !default_label[0])
        gtk_label_set_text (stuff->default_label, "None");
    else
        gtk_label_set_text (stuff->default_label, default_label);

    block_radio (stuff);
    block_accel_set (stuff);

    shortcut = g_hash_table_lookup (stuff->changed, action);
    if (shortcut)
    {
        switch (shortcut->choice) {
            case NONE:
                gtk_toggle_button_set_active (stuff->shortcut_none, TRUE);
                moo_accel_button_set_accel (stuff->shortcut, NULL);
                break;

            case DEFAULT:
                gtk_toggle_button_set_active (stuff->shortcut_default, TRUE);
                moo_accel_button_set_accel (stuff->shortcut, default_accel);
                break;

            case CUSTOM:
                gtk_toggle_button_set_active (stuff->shortcut_custom, TRUE);
                moo_accel_button_set_accel (stuff->shortcut, shortcut->accel);
                break;

            default:
                g_assert_not_reached ();
        }
    }
    else
    {
        const char *accel = moo_action_get_accel (action);

        if (!accel[0]) {
            gtk_toggle_button_set_active (stuff->shortcut_none, TRUE);
            moo_accel_button_set_accel (stuff->shortcut, NULL);
        }
        else {
            if (!strcmp (accel, default_accel)) {
                gtk_toggle_button_set_active (stuff->shortcut_default, TRUE);
                moo_accel_button_set_accel (stuff->shortcut, default_accel);
            }
            else {
                gtk_toggle_button_set_active (stuff->shortcut_custom, TRUE);
                moo_accel_button_set_accel (stuff->shortcut, moo_action_get_accel (action));
            }
        }
    }

    unblock_radio (stuff);
    unblock_accel_set (stuff);

    g_free (default_label);
    g_object_unref (action);
}


static void accel_set (Stuff *stuff)
{
    GtkTreeIter iter;
    GtkTreePath *path;
    const char *accel;
    const char *label;

    g_return_if_fail (stuff->current_row != NULL && stuff->current_action != NULL);

    block_radio (stuff);
    gtk_toggle_button_set_active (stuff->shortcut_custom, TRUE);
    unblock_radio (stuff);

    path = gtk_tree_row_reference_get_path (stuff->current_row);
    if (!path || !gtk_tree_model_get_iter (GTK_TREE_MODEL (stuff->store), &iter, path))
    {
        g_critical ("%s: got invalid path", G_STRLOC);
        gtk_tree_path_free (path);
        return;
    }
    gtk_tree_path_free (path);

    accel = moo_accel_button_get_accel (stuff->shortcut);
    label = gtk_button_get_label (GTK_BUTTON (stuff->shortcut));
    gtk_tree_store_set (stuff->store, &iter, COLUMN_ACCEL, label, -1);

    if (accel && accel[0])
        g_hash_table_insert (stuff->changed,
                             stuff->current_action,
                             shortcut_new (CUSTOM, accel));
    else
        g_hash_table_insert (stuff->changed,
                             stuff->current_action,
                             shortcut_new (NONE, ""));
}


static void shortcut_none_toggled (Stuff *stuff)
{
    GtkTreeIter iter;
    GtkTreePath *path;

    g_return_if_fail (stuff->current_row != NULL &&
                      stuff->current_action != NULL);
    if (!gtk_toggle_button_get_active (stuff->shortcut_none))
        return;

    path = gtk_tree_row_reference_get_path (stuff->current_row);
    if (!path || !gtk_tree_model_get_iter (GTK_TREE_MODEL (stuff->store), &iter, path))
    {
        g_critical ("%s: got invalid path", G_STRLOC);
        gtk_tree_path_free (path);
        return;
    }
    gtk_tree_path_free (path);

    gtk_tree_store_set (stuff->store, &iter, COLUMN_ACCEL, NULL, -1);
    g_hash_table_insert (stuff->changed,
                         stuff->current_action,
                         shortcut_new (NONE, ""));
    block_accel_set (stuff);
    moo_accel_button_set_accel (stuff->shortcut, "");
    unblock_accel_set (stuff);
}


static void shortcut_default_toggled (Stuff *stuff)
{
    GtkTreeIter iter;
    GtkTreePath *path;
    Shortcut *current_shortcut;
    const char *default_accel;

    g_return_if_fail (stuff->current_row != NULL &&
                      stuff->current_action != NULL);
    if (!gtk_toggle_button_get_active (stuff->shortcut_default)) return;

    path = gtk_tree_row_reference_get_path (stuff->current_row);
    if (!path || !gtk_tree_model_get_iter (GTK_TREE_MODEL (stuff->store), &iter, path))
    {
        g_critical ("%s: got invalid path", G_STRLOC);
        gtk_tree_path_free (path);
        return;
    }
    gtk_tree_path_free (path);

    current_shortcut = g_hash_table_lookup (stuff->changed,
                                            stuff->current_action);
    if (!current_shortcut) {
        current_shortcut = shortcut_new (NONE, "");
        g_hash_table_insert (stuff->changed,
                             stuff->current_action,
                             current_shortcut);
    }

    default_accel = moo_action_get_default_accel (stuff->current_action);
    if (default_accel[0]) {
        char *label = moo_action_get_default_accel_label (stuff->current_action);
        gtk_tree_store_set (stuff->store, &iter, COLUMN_ACCEL, label, -1);
        g_free (label);
    }
    else {
        gtk_tree_store_set (stuff->store, &iter, COLUMN_ACCEL, NULL, -1);
    }

    current_shortcut->choice = DEFAULT;
    g_free (current_shortcut->accel);
    current_shortcut->accel = g_strdup (default_accel);
    block_accel_set (stuff);
    moo_accel_button_set_accel (stuff->shortcut, default_accel);
    unblock_accel_set (stuff);
}


static void shortcut_custom_toggled (Stuff *stuff)
{
    GtkTreeIter iter;
    GtkTreePath *path;
    Shortcut *shortcut;

    g_return_if_fail (stuff->current_row != NULL &&
                      stuff->current_action != NULL);
    if (!gtk_toggle_button_get_active (stuff->shortcut_custom)) return;

    path = gtk_tree_row_reference_get_path (stuff->current_row);
    if (!path || !gtk_tree_model_get_iter (GTK_TREE_MODEL (stuff->store), &iter, path))
    {
        g_critical ("%s: got invalid path", G_STRLOC);
        gtk_tree_path_free (path);
        return;
    }
    gtk_tree_path_free (path);

    shortcut = g_hash_table_lookup (stuff->changed, stuff->current_action);
    if (shortcut) {
        block_accel_set (stuff);
        moo_accel_button_set_accel (stuff->shortcut, shortcut->accel);
        unblock_accel_set (stuff);
        gtk_tree_store_set (stuff->store, &iter, COLUMN_ACCEL,
                            gtk_button_get_label (GTK_BUTTON (stuff->shortcut)),
                            -1);
        shortcut->choice = CUSTOM;
    }
    else {
        const char *accel = moo_action_get_accel (stuff->current_action);
        g_hash_table_insert (stuff->changed,
                             stuff->current_action,
                             shortcut_new (CUSTOM, accel));
        block_accel_set (stuff);
        moo_accel_button_set_accel (stuff->shortcut, accel);
        unblock_accel_set (stuff);
    }
}


static void dialog_response (GObject *page,
                             int response)
{
    switch (response) {
        case GTK_RESPONSE_OK:
            g_signal_emit_by_name (page, "apply", NULL);
            break;

        case GTK_RESPONSE_REJECT:
            g_signal_emit_by_name (page, "set_defaults", NULL);
            break;
    };
}


GtkWidget   *moo_shortcuts_prefs_dialog_new         (MooActionGroup *group)
{
    GtkWidget *page, *dialog, *page_holder;
    MooGladeXML *xml;

    xml = moo_glade_xml_new_from_buf (MOO_SHORTCUTS_PREFS_GLADE_UI,
                                      -1, "dialog", NULL);
    g_return_val_if_fail (xml != NULL, NULL);

    dialog = moo_glade_xml_get_widget (xml, "dialog");

    page = moo_shortcuts_prefs_page_new (group);
    gtk_widget_show (page);
    page_holder = moo_glade_xml_get_widget (xml, "page_holder");
    gtk_container_add (GTK_CONTAINER (page_holder), page);

    moo_glade_xml_unref (xml);

#if GTK_MINOR_VERSION >= 6
    gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                             GTK_RESPONSE_OK,
                                             GTK_RESPONSE_CANCEL,
                                             GTK_RESPONSE_REJECT,
                                             -1);
#endif /* GTK_MINOR_VERSION >= 6 */

    g_signal_connect_swapped (dialog, "response",
                              G_CALLBACK (dialog_response), page);
    g_signal_emit_by_name (page, "init", NULL);

    return dialog;
}


void         moo_shortcuts_prefs_dialog_run         (MooActionGroup *group,
                                                     GtkWidget      *parent)
{
    GtkWidget *dialog = moo_shortcuts_prefs_dialog_new (group);
    GtkWindow *parent_window = GTK_WINDOW (gtk_widget_get_toplevel (parent));

    gtk_window_set_transient_for (GTK_WINDOW (dialog), parent_window);

    while (TRUE)
    {
        int response = gtk_dialog_run (GTK_DIALOG (dialog));
        if (response != GTK_RESPONSE_REJECT)
            break;
    }

    gtk_widget_destroy (dialog);

#ifdef __WIN32__ /* TODO */
    gtk_window_present (parent_window);
#endif /* __WIN32__ */

    return;
}
