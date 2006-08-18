/*
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
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooutils-fs.h"
#include "mooutils/mooutils-treeview.h"
#include "mooutils/mooi18n.h"
#include <string.h>


static void prefs_page_apply    (MooGladeXML        *xml);
static void prefs_page_init     (MooGladeXML        *xml);

static void pattern_data_func   (GtkTreeViewColumn  *column,
                                 GtkCellRenderer    *cell,
                                 GtkTreeModel       *model,
                                 GtkTreeIter        *iter);
static void new_item            (MooConfigHelper    *helper,
                                 MooConfig          *config,
                                 MooConfigItem      *item);


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
    MooPrefsDialogPage *page;
    MooGladeXML *xml;
    GtkWidget *treeview;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    MooConfigHelper *helper;

    xml = moo_glade_xml_new_empty (GETTEXT_PACKAGE);
    moo_glade_xml_map_id (xml, "script", MOO_TYPE_TEXT_VIEW);
    page = moo_prefs_dialog_page_new_from_xml ("Active Strings", GTK_STOCK_CONVERT,
                                               xml, AS_PLUGIN_GLADE_UI,
                                               "page", NULL);

    g_object_set_data_full (G_OBJECT (xml), "as-plugin",
                            g_object_ref (plugin), (GDestroyNotify) g_object_unref);

    g_signal_connect_swapped (page, "apply", G_CALLBACK (prefs_page_apply), xml);
    g_signal_connect_swapped (page, "init", G_CALLBACK (prefs_page_init), xml);

    setup_script_view (moo_glade_xml_get_widget (xml, "script"));

    treeview = moo_glade_xml_get_widget (xml, "treeview");

    column = gtk_tree_view_column_new ();
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, cell, TRUE);
    gtk_tree_view_column_set_cell_data_func (column, cell,
                                             (GtkTreeCellDataFunc) pattern_data_func,
                                             NULL, NULL);

    helper = _moo_config_helper_new (treeview,
                                     moo_glade_xml_get_widget (xml, "new"),
                                     moo_glade_xml_get_widget (xml, "delete"),
                                     moo_glade_xml_get_widget (xml, "up"),
                                     moo_glade_xml_get_widget (xml, "down"));

    _moo_config_helper_add_widget (helper, moo_glade_xml_get_widget (xml, "pattern"),
                                   AS_KEY_PATTERN, TRUE);
    _moo_config_helper_add_widget (helper, moo_glade_xml_get_widget (xml, "lang"),
                                   AS_KEY_LANG, FALSE);
    _moo_config_helper_add_widget (helper, moo_glade_xml_get_widget (xml, "enabled"),
                                   AS_KEY_ENABLED, TRUE);
    _moo_config_helper_add_widget (helper, moo_glade_xml_get_widget (xml, "word_boundary"),
                                   AS_KEY_WORD_BOUNDARY, FALSE);
    _moo_config_helper_add_widget (helper, moo_glade_xml_get_widget (xml, "script"),
                                   NULL, FALSE);

    g_signal_connect (helper, "new-item",
                      G_CALLBACK (new_item), NULL);
    g_object_set_data_full (G_OBJECT (treeview),
                            "as-plugin-config-helper",
                            helper, g_object_unref);

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

    pattern = moo_config_item_get (item, AS_KEY_PATTERN);
    enabled = moo_config_get_bool (MOO_CONFIG (model), item, AS_KEY_ENABLED);

    g_object_set (cell, "text", pattern,
                  "foreground", enabled ? NULL : "grey",
                  "style", enabled ? PANGO_STYLE_NORMAL : PANGO_STYLE_ITALIC,
                  NULL);
}


static void
prefs_page_apply (MooGladeXML *xml)
{
    GtkWidget *treeview;
    MooConfig *config;
    MooConfigHelper *helper;
    GError *error = NULL;

    treeview = moo_glade_xml_get_widget (xml, "treeview");
    helper = g_object_get_data (G_OBJECT (treeview), "as-plugin-config-helper");
    _moo_config_helper_update_model (helper, NULL, NULL);

    config = MOO_CONFIG (gtk_tree_view_get_model (GTK_TREE_VIEW (treeview)));

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
    GtkWidget *treeview;

    config = _as_plugin_load_config ();

    if (!config)
        config = moo_config_new ();

    moo_config_set_default_bool (config, AS_KEY_ENABLED, TRUE);

    treeview = moo_glade_xml_get_widget (xml, "treeview");
    gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (config));
    _moo_tree_view_select_first (GTK_TREE_VIEW (treeview));

    g_object_unref (config);
}


static void
new_item (G_GNUC_UNUSED MooConfigHelper *helper,
          MooConfig          *config,
          MooConfigItem      *item)
{
    moo_config_set (config, item, AS_KEY_PATTERN, "?", TRUE);
}
