/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   as-plugin-prefs.c
 *
 *   Copyright (C) 2004-2006 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "as-plugin.h"
#include "mooedit/plugins/activestrings/as-plugin-glade.h"
#include "mooutils/mooprefsdialogpage.h"
#include "mooutils/mooconfig.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooutils-fs.h"
#include <string.h>


static void prefs_page_apply    (MooGladeXML        *xml);
static void prefs_page_init     (MooGladeXML        *xml);
static void prefs_page_destroy  (MooGladeXML        *xml);
static void selection_changed   (GtkTreeSelection   *selection,
                                 MooGladeXML        *xml);
static void set_from_widgets    (MooGladeXML        *xml,
                                 GtkTreeModel       *model,
                                 GtkTreePath        *path);
static void set_from_model      (MooGladeXML        *xml,
                                 GtkTreeModel       *model,
                                 GtkTreePath        *path);

static void pattern_data_func   (GtkTreeViewColumn  *column,
                                 GtkCellRenderer    *cell,
                                 GtkTreeModel       *model,
                                 GtkTreeIter        *iter);
static void button_new          (MooGladeXML        *xml);
static void button_delete       (MooGladeXML        *xml);
static void button_up           (MooGladeXML        *xml);
static void button_down         (MooGladeXML        *xml);
static void pattern_changed     (MooGladeXML        *xml);
static void enabled_changed     (MooGladeXML        *xml);


static MooPlugin *
get_plugin (MooGladeXML *xml)
{
    return g_object_get_data (G_OBJECT (xml), "as-plugin");
}


static void
setup_script_view (MooTextView *script)
{
    MooLangMgr *mgr;
    MooLang *lang;
    MooIndenter *indent;

    g_object_set (script, "highlight-current-line", FALSE, NULL);

    mgr = moo_editor_get_lang_mgr (moo_editor_instance ());
    lang = moo_lang_mgr_get_lang (mgr, "MooScript");

    if (lang)
        moo_text_view_set_lang (script, lang);

    moo_text_view_set_font_from_string (script, "Monospace");

    indent = moo_indenter_new (NULL, NULL);
    moo_text_view_set_indenter (script, indent);
    g_object_set (indent, "use-tabs", FALSE, "indent", 2, NULL);
    g_object_unref (indent);
}


GtkWidget *
_as_plugin_prefs_page (MooPlugin *plugin)
{
    GtkWidget *page;
    MooGladeXML *xml;
    GtkTreeSelection *selection;
    GtkTreeView *treeview;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;

    xml = moo_glade_xml_new_empty ();
    moo_glade_xml_map_id (xml, "script", MOO_TYPE_TEXT_VIEW);
    page = moo_prefs_dialog_page_new_from_xml ("Active Strings", GTK_STOCK_CONVERT,
                                               xml, AS_PLUGIN_GLADE_UI, -1,
                                               "page", NULL);

    g_object_set_data_full (G_OBJECT (xml), "as-plugin",
                            g_object_ref (plugin), (GDestroyNotify) g_object_unref);

    g_signal_connect_swapped (page, "apply", G_CALLBACK (prefs_page_apply), xml);
    g_signal_connect_swapped (page, "init", G_CALLBACK (prefs_page_init), xml);
    g_signal_connect_swapped (page, "destroy", G_CALLBACK (prefs_page_destroy), xml);

    setup_script_view (moo_glade_xml_get_widget (xml, "script"));

    treeview = moo_glade_xml_get_widget (xml, "treeview");
    selection = gtk_tree_view_get_selection (treeview);
    g_signal_connect (selection, "changed",
                      G_CALLBACK (selection_changed), xml);

    column = gtk_tree_view_column_new ();
    gtk_tree_view_append_column (treeview, column);

    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, cell, TRUE);
    gtk_tree_view_column_set_cell_data_func (column, cell,
                                             (GtkTreeCellDataFunc) pattern_data_func,
                                             NULL, NULL);

    g_signal_connect_swapped (moo_glade_xml_get_widget (xml, "new"),
                              "clicked", G_CALLBACK (button_new), xml);
    g_signal_connect_swapped (moo_glade_xml_get_widget (xml, "delete"),
                              "clicked", G_CALLBACK (button_delete), xml);
    g_signal_connect_swapped (moo_glade_xml_get_widget (xml, "up"),
                              "clicked", G_CALLBACK (button_up), xml);
    g_signal_connect_swapped (moo_glade_xml_get_widget (xml, "down"),
                              "clicked", G_CALLBACK (button_down), xml);
    g_signal_connect_swapped (moo_glade_xml_get_widget (xml, "pattern"),
                              "changed", G_CALLBACK (pattern_changed), xml);
    g_signal_connect_swapped (moo_glade_xml_get_widget (xml, "enabled"),
                              "toggled", G_CALLBACK (enabled_changed), xml);

    g_object_unref (xml);
    return page;
}


static void
pattern_data_func (G_GNUC_UNUSED GtkTreeViewColumn *column,
                   GtkCellRenderer    *cell,
                   GtkTreeModel       *model,
                   GtkTreeIter        *iter)
{
    gboolean enabled;
    const char *pattern;
    MooConfigItem *item = NULL;

    gtk_tree_model_get (model, iter, 0, &item, -1);
    g_return_if_fail (item != NULL);

    pattern = moo_config_item_get_value (item, AS_KEY_PATTERN);
    enabled = moo_config_item_get_bool (item, AS_KEY_ENABLED, TRUE);

    g_object_set (cell, "text", pattern,
                  "foreground", enabled ? NULL : "grey",
                  "style", enabled ? PANGO_STYLE_NORMAL : PANGO_STYLE_ITALIC,
                  NULL);
}


static gboolean
is_empty_string (const char *string)
{
    if (!string)
        return TRUE;

#define IS_SPACE(c) (c == ' ' || c == '\t' || c == '\r' || c == '\n')
    while (*string)
    {
        if (*string && !IS_SPACE (*string))
            return FALSE;
        string++;
    }
#undef IS_SPACE

    return TRUE;
}


static void
prefs_page_apply (MooGladeXML *xml)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    MooConfig *config;
    GError *error = NULL;

    selection = gtk_tree_view_get_selection (moo_glade_xml_get_widget (xml, "treeview"));

    if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        GtkTreePath *path = gtk_tree_model_get_path (model, &iter);
        set_from_widgets (xml, model, path);
        gtk_tree_path_free (path);
    }

    config = MOO_CONFIG (model);

    if (!moo_config_get_modified (config))
        return;

    _as_plugin_save_config (config, &error);

    if (error)
    {
        g_critical ("%s: could not save config: %s",
                    G_STRLOC, error->message);
        g_error_free (error);
    }

    _as_plugin_reload (get_plugin (xml));
}


static void
prefs_page_init (MooGladeXML *xml)
{
    MooConfig *config;

    config = _as_plugin_load_config ();

    if (!config)
        config = moo_config_new ();

    gtk_tree_view_set_model (moo_glade_xml_get_widget (xml, "treeview"),
                             GTK_TREE_MODEL (config));

    set_from_model (xml, GTK_TREE_MODEL (config), NULL);
    g_object_unref (config);
}


static gboolean
get_selected (MooGladeXML   *xml,
              GtkTreeModel **model,
              GtkTreeIter   *iter)
{
    GtkTreeSelection *selection;
    selection = gtk_tree_view_get_selection (moo_glade_xml_get_widget (xml, "treeview"));
    return gtk_tree_selection_get_selected (selection, model, iter);
}


static gboolean
strings_equal (const char *str1,
               const char *str2)
{
    str1 = str1 ? str1 : "";
    str2 = str2 ? str2 : "";
    return ! strcmp (str1, str2);
}


static void
set_from_widgets (MooGladeXML  *xml,
                  GtkTreeModel *model,
                  GtkTreePath  *path)
{
    GtkTreeIter iter;
    GtkToggleButton *button;
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    const char *pattern, *lang;
    char *script;
    gboolean enabled;
    MooConfigItem *item = NULL;

    gtk_tree_model_get_iter (model, &iter, path);

    pattern = gtk_entry_get_text (moo_glade_xml_get_widget (xml, "pattern"));
    lang = gtk_entry_get_text (moo_glade_xml_get_widget (xml, "lang"));
    button = moo_glade_xml_get_widget (xml, "enabled");
    enabled = gtk_toggle_button_get_active (button);

    buffer = gtk_text_view_get_buffer (moo_glade_xml_get_widget (xml, "script"));
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    script = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

    gtk_tree_model_get (model, &iter, 0, &item, -1);
    g_return_if_fail (item != NULL);

    moo_config_set_value (MOO_CONFIG (model), item,
                          AS_KEY_PATTERN, pattern, TRUE);
    moo_config_set_value (MOO_CONFIG (model), item,
                          AS_KEY_LANG, lang, TRUE);

    if (!enabled)
        moo_config_set_bool (MOO_CONFIG (model), item,
                             AS_KEY_ENABLED, enabled, TRUE);
    else
        moo_config_set_value (MOO_CONFIG (model), item,
                              AS_KEY_ENABLED, NULL, TRUE);

    moo_config_set_item_content (MOO_CONFIG (model), item,
                                 script, TRUE);

    g_free (script);
}


static void
script_set_text (gpointer    view,
                 const char *text)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    moo_text_view_begin_not_undoable_action (view);
    gtk_text_buffer_set_text (gtk_text_view_get_buffer (view), text, -1);
    moo_text_view_end_not_undoable_action (view);
}


static void
set_from_model (MooGladeXML  *xml,
                GtkTreeModel *model,
                GtkTreePath  *path)
{
    GtkTreeIter iter;
    GtkWidget *button_new, *button_delete, *button_up, *button_down;
    GtkWidget *table;
    GtkToggleButton *button_enabled;
    GtkEntry *entry_pattern, *entry_lang;
    GtkTextView *script_view;
    GtkTextBuffer *buffer;
    MooConfigItem *item = NULL;

    if (path)
    {
        gtk_tree_model_get_iter (model, &iter, path);
        gtk_tree_model_get (model, &iter, 0, &item, -1);
        g_return_if_fail (item != NULL);
    }

    g_signal_handlers_block_by_func (moo_glade_xml_get_widget (xml, "pattern"),
                                     (gpointer) pattern_changed, xml);
    g_signal_handlers_block_by_func (moo_glade_xml_get_widget (xml, "enabled"),
                                     (gpointer) enabled_changed, xml);

    button_new = moo_glade_xml_get_widget (xml, "new");
    button_delete = moo_glade_xml_get_widget (xml, "delete");
    button_up = moo_glade_xml_get_widget (xml, "up");
    button_down = moo_glade_xml_get_widget (xml, "down");
    table = moo_glade_xml_get_widget (xml, "table");
    entry_pattern = moo_glade_xml_get_widget (xml, "pattern");
    entry_lang = moo_glade_xml_get_widget (xml, "lang");
    button_enabled = moo_glade_xml_get_widget (xml, "enabled");
    script_view = moo_glade_xml_get_widget (xml, "script");
    buffer = gtk_text_view_get_buffer (script_view);

    if (!item)
    {
        gtk_widget_set_sensitive (button_delete, FALSE);
        gtk_widget_set_sensitive (button_up, FALSE);
        gtk_widget_set_sensitive (button_down, FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (button_enabled), FALSE);
        gtk_widget_set_sensitive (table, FALSE);
        gtk_entry_set_text (entry_pattern, "");
        gtk_entry_set_text (entry_lang, "");
        script_set_text (script_view, "");
        gtk_toggle_button_set_active (button_enabled, FALSE);
    }
    else
    {
        const char *pattern, *lang, *script;
        gboolean enabled;
        int *indices;

        pattern = moo_config_item_get_value (item, AS_KEY_PATTERN);
        lang = moo_config_item_get_value (item, AS_KEY_LANG);
        script = moo_config_item_get_content (item);
        enabled = moo_config_item_get_bool (item, AS_KEY_ENABLED, TRUE);

        gtk_widget_set_sensitive (button_delete, TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET (button_enabled), TRUE);
        gtk_widget_set_sensitive (table, TRUE);
        gtk_entry_set_text (entry_pattern, pattern ? pattern : "");
        gtk_entry_set_text (entry_lang, lang ? lang : "");
        script_set_text (script_view, script ? script : "");
        gtk_toggle_button_set_active (button_enabled, enabled);

        indices = gtk_tree_path_get_indices (path);
        gtk_widget_set_sensitive (button_up, indices[0] != 0);
        gtk_widget_set_sensitive (button_down, indices[0] !=
                gtk_tree_model_iter_n_children (model, NULL) - 1);

    }

    g_signal_handlers_unblock_by_func (moo_glade_xml_get_widget (xml, "pattern"),
                                       (gpointer) pattern_changed, xml);
    g_signal_handlers_unblock_by_func (moo_glade_xml_get_widget (xml, "enabled"),
                                       (gpointer) enabled_changed, xml);
}


static void
selection_changed (GtkTreeSelection *selection,
                   MooGladeXML      *xml)
{
    GtkTreeRowReference *old_row;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreePath *path, *old_path;

    old_row = g_object_get_data (G_OBJECT (selection), "current-row");
    old_path = old_row ? gtk_tree_row_reference_get_path (old_row) : NULL;

    if (old_row && !old_path)
    {
        g_object_set_data (G_OBJECT (selection), "current-row", NULL);
        old_row = NULL;
    }

    if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        path = gtk_tree_model_get_path (model, &iter);

        if (old_path && !gtk_tree_path_compare (old_path, path))
        {
            gtk_tree_path_free (old_path);
            gtk_tree_path_free (path);
            return;
        }
    }
    else
    {
        if (!old_path)
            return;

        path = NULL;
    }

    if (old_path)
        set_from_widgets (xml, model, old_path);

    set_from_model (xml, model, path);

    if (path)
    {
        GtkTreeRowReference *row;
        row = gtk_tree_row_reference_new (model, path);
        g_object_set_data_full (G_OBJECT (selection), "current-row", row,
                                (GDestroyNotify) gtk_tree_row_reference_free);
    }
    else
    {
        g_object_set_data (G_OBJECT (selection), "current-row", NULL);
    }

    gtk_tree_path_free (path);
    gtk_tree_path_free (old_path);
}


static int
iter_get_index (GtkTreeModel *model,
                GtkTreeIter  *iter)
{
    int index;
    GtkTreePath *path;

    path = gtk_tree_model_get_path (model, iter);

    if (!path)
        return -1;

    index = gtk_tree_path_get_indices(path)[0] + 1;

    gtk_tree_path_free (path);
    return index;
}


static void
button_new (MooGladeXML *xml)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    GtkTreeView *treeview;
    MooConfigItem *item;
    int index;

    treeview = moo_glade_xml_get_widget (xml, "treeview");

    if (!get_selected (xml, &model, &iter))
        index = -1;
    else
        index = iter_get_index (model, &iter);

    item = moo_config_new_item (MOO_CONFIG (model), index, TRUE);
    moo_config_set_value (MOO_CONFIG (model), item, AS_KEY_PATTERN, "?", TRUE);

    selection = gtk_tree_view_get_selection (treeview);
    moo_config_get_item_iter (MOO_CONFIG (model), item, &iter);
    gtk_tree_selection_select_iter (selection, &iter);
}


static void
button_delete (MooGladeXML *xml)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    GtkTreeView *treeview;
    int index;

    if (!get_selected (xml, &model, &iter))
        g_return_if_reached ();

    index = iter_get_index (model, &iter);
    moo_config_delete_item (MOO_CONFIG (model), index, TRUE);

    if (gtk_tree_model_iter_nth_child (model, &iter, NULL, index))
    {
        treeview = moo_glade_xml_get_widget (xml, "treeview");
        selection = gtk_tree_view_get_selection (treeview);
        gtk_tree_selection_select_iter (selection, &iter);
    }
}


static void
button_up (MooGladeXML *xml)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreePath *path, *new_path;
    int *indices;

    if (!get_selected (xml, &model, &iter))
        g_return_if_reached ();

    path = gtk_tree_model_get_path (model, &iter);
    indices = gtk_tree_path_get_indices (path);

    if (!indices[0])
        g_return_if_reached ();

    moo_config_move_item (MOO_CONFIG (model), indices[0], indices[0] - 1, TRUE);
    new_path = gtk_tree_path_new_from_indices (indices[0] - 1, -1);
    set_from_model (xml, model, new_path);

    gtk_tree_path_free (new_path);
    gtk_tree_path_free (path);
}


static void
button_down (MooGladeXML *xml)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreePath *path, *new_path;
    int *indices;
    int n_children;

    if (!get_selected (xml, &model, &iter))
        g_return_if_reached ();

    path = gtk_tree_model_get_path (model, &iter);
    indices = gtk_tree_path_get_indices (path);
    n_children = gtk_tree_model_iter_n_children (model, NULL);

    if (indices[0] == n_children - 1)
        g_return_if_reached ();

    moo_config_move_item (MOO_CONFIG (model), indices[0], indices[0] + 1, TRUE);
    new_path = gtk_tree_path_new_from_indices (indices[0] + 1, -1);
    set_from_model (xml, model, new_path);

    gtk_tree_path_free (new_path);
    gtk_tree_path_free (path);
}


static void
pattern_changed (MooGladeXML *xml)
{
    GtkEntry *entry;
    GtkTreeModel *model;
    GtkTreeIter iter;
    const char *pattern;
    MooConfigItem *item = NULL;

    if (!get_selected (xml, &model, &iter))
        return;

    entry = moo_glade_xml_get_widget (xml, "pattern");
    pattern = gtk_entry_get_text (entry);
    gtk_tree_model_get (model, &iter, 0, &item, -1);
    g_return_if_fail (item != NULL);

    moo_config_set_value (MOO_CONFIG (model), item,
                          AS_KEY_PATTERN, pattern, TRUE);
}


static void
enabled_changed (MooGladeXML *xml)
{
    GtkToggleButton *button;
    GtkTreeModel *model;
    GtkTreeIter iter;
    MooConfigItem *item = NULL;

    if (!get_selected (xml, &model, &iter))
        return;

    button = moo_glade_xml_get_widget (xml, "enabled");
    gtk_tree_model_get (model, &iter, 0, &item, -1);
    g_return_if_fail (item != NULL);

    moo_config_set_bool (MOO_CONFIG (model), item, AS_KEY_ENABLED,
                         gtk_toggle_button_get_active (button), TRUE);
}


static void
prefs_page_destroy (MooGladeXML *xml)
{
    GtkTreeSelection *selection;

    selection = gtk_tree_view_get_selection (moo_glade_xml_get_widget (xml, "treeview"));
    g_signal_handlers_disconnect_by_func (selection, (gpointer) selection_changed, xml);

    g_signal_handlers_disconnect_by_func (moo_glade_xml_get_widget (xml, "new"),
                                          (gpointer) button_new, xml);
    g_signal_handlers_disconnect_by_func (moo_glade_xml_get_widget (xml, "delete"),
                                          (gpointer) button_delete, xml);
    g_signal_handlers_disconnect_by_func (moo_glade_xml_get_widget (xml, "up"),
                                          (gpointer) button_up, xml);
    g_signal_handlers_disconnect_by_func (moo_glade_xml_get_widget (xml, "down"),
                                          (gpointer) button_down, xml);
    g_signal_handlers_disconnect_by_func (moo_glade_xml_get_widget (xml, "pattern"),
                                          (gpointer) pattern_changed, xml);
    g_signal_handlers_disconnect_by_func (moo_glade_xml_get_widget (xml, "enabled"),
                                          (gpointer) enabled_changed, xml);
}
