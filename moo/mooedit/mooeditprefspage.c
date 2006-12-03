/*
 *   mooeditprefspage.c
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

#define MOOEDIT_COMPILATION
#include "mooedit/mooedit-private.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mooeditprefs-glade.h"
#include "mooedit/moolangmgr.h"
#include "mooedit/mooeditfiltersettings.h"
#include "mooutils/mooprefsdialog.h"
#include "mooutils/moocompat.h"
#include "mooutils/moostock.h"
#include "mooutils/mooglade.h"
#include "mooutils/moofontsel.h"
#include "mooutils/mooutils-treeview.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooi18n.h"
#include <string.h>


static void     prefs_page_init             (MooPrefsDialogPage *page);
static void     prefs_page_apply            (MooPrefsDialogPage *page);
static void     prefs_page_apply_lang_prefs (MooPrefsDialogPage *page);
static void     apply_filter_settings       (MooPrefsDialogPage *page);

static void     scheme_combo_init           (GtkComboBox        *combo,
                                             MooEditor          *editor);
static void     scheme_combo_data_func      (GtkCellLayout      *layout,
                                             GtkCellRenderer    *cell,
                                             GtkTreeModel       *model,
                                             GtkTreeIter        *iter);
static void     scheme_combo_set_scheme     (GtkComboBox        *combo,
                                             MooTextStyleScheme *scheme);

static void     default_lang_combo_init     (GtkComboBox        *combo,
                                             MooPrefsDialogPage *page);
static void     default_lang_combo_set_lang (GtkComboBox        *combo,
                                             const char         *id);

static void     lang_combo_init             (GtkComboBox        *combo,
                                             MooPrefsDialogPage *page);

static void     filter_treeview_init        (MooGladeXML        *xml);

static GtkTreeModel *create_lang_model      (MooEditor          *editor);

static MooEditor *page_get_editor           (MooPrefsDialogPage *page);
static GtkTreeModel *page_get_lang_model    (MooPrefsDialogPage *page);
static MooTextStyleScheme *page_get_scheme  (MooPrefsDialogPage *page);
static char    *page_get_default_lang       (MooPrefsDialogPage *page);


GtkWidget *
moo_edit_prefs_page_new (MooEditor *editor)
{
    MooPrefsDialogPage *page;
    GtkComboBox *scheme_combo, *default_lang_combo, *lang_combo;
    MooGladeXML *xml;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    _moo_edit_init_config ();

    xml = moo_glade_xml_new_empty (GETTEXT_PACKAGE);
    moo_glade_xml_map_id (xml, "fontbutton", MOO_TYPE_FONT_BUTTON);
    moo_glade_xml_set_property (xml, "fontbutton", "monospace", "True");
    page = moo_prefs_dialog_page_new_from_xml ("Editor", GTK_STOCK_EDIT, xml,
                                               MOO_EDIT_PREFS_GLADE_UI, "page",
                                               MOO_EDIT_PREFS_PREFIX);
    g_object_unref (xml);

    g_object_set_data_full (G_OBJECT (page), "moo-editor",
                            g_object_ref (editor), g_object_unref);
    g_object_set_data_full (G_OBJECT (page), "moo-lang-mgr",
                            g_object_ref (moo_editor_get_lang_mgr (editor)),
                            g_object_unref);

    g_signal_connect (page, "init", G_CALLBACK (prefs_page_init), NULL);
    g_signal_connect (page, "apply", G_CALLBACK (prefs_page_apply), NULL);

    scheme_combo = moo_glade_xml_get_widget (page->xml, "color_scheme_combo");
    scheme_combo_init (scheme_combo, editor);
    default_lang_combo = moo_glade_xml_get_widget (page->xml, "default_lang_combo");
    default_lang_combo_init (default_lang_combo, page);
    lang_combo = moo_glade_xml_get_widget (page->xml, "lang_combo");
    lang_combo_init (lang_combo, page);

    filter_treeview_init (page->xml);

    return GTK_WIDGET (page);
}


static void
scheme_combo_init (GtkComboBox *combo,
                   MooEditor   *editor)
{
    GtkListStore *store;
    MooLangMgr *mgr;
    GSList *list, *l;
    GtkCellRenderer *cell;

    mgr = moo_editor_get_lang_mgr (editor);
    list = _moo_lang_mgr_list_schemes (mgr);
    g_return_if_fail (list != NULL);

    store = gtk_list_store_new (1, MOO_TYPE_TEXT_STYLE_SCHEME);

    for (l = list; l != NULL; l = l->next)
    {
        GtkTreeIter iter;
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 0, l->data, -1);
    }

    gtk_combo_box_set_model (combo, GTK_TREE_MODEL (store));

    cell = gtk_cell_renderer_text_new ();
    gtk_cell_layout_clear (GTK_CELL_LAYOUT (combo));
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, TRUE);
    gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (combo), cell,
                                        (GtkCellLayoutDataFunc) scheme_combo_data_func,
                                        NULL, NULL);

    g_object_unref (store);
    g_slist_foreach (list, (GFunc) g_object_unref, NULL);
    g_slist_free (list);
}


static void
scheme_combo_data_func (G_GNUC_UNUSED GtkCellLayout *layout,
                        GtkCellRenderer    *cell,
                        GtkTreeModel       *model,
                        GtkTreeIter        *iter)
{
    MooTextStyleScheme *scheme = NULL;
    gtk_tree_model_get (model, iter, 0, &scheme, -1);
    g_return_if_fail (scheme != NULL);
    g_object_set (cell, "text", moo_text_style_scheme_get_name (scheme), NULL);
    g_object_unref (scheme);
}


static void
prefs_page_init (MooPrefsDialogPage *page)
{
    MooEditor *editor;
    MooLangMgr *mgr;
    MooTextStyleScheme *scheme;
    GtkComboBox *scheme_combo, *default_lang_combo;
    MooTreeHelper *helper;
    const char *lang;

    editor = page_get_editor (page);
    mgr = moo_editor_get_lang_mgr (editor);
    scheme = _moo_lang_mgr_get_active_scheme (mgr);
    g_return_if_fail (scheme != NULL);

    scheme_combo = moo_glade_xml_get_widget (page->xml, "color_scheme_combo");
    scheme_combo_set_scheme (scheme_combo, scheme);

    default_lang_combo = moo_glade_xml_get_widget (page->xml, "default_lang_combo");
    lang = moo_prefs_get_string (moo_edit_setting (MOO_EDIT_PREFS_DEFAULT_LANG));
    default_lang_combo_set_lang (default_lang_combo, lang);

    helper = g_object_get_data (G_OBJECT (page), "moo-tree-helper");
    _moo_tree_helper_update_widgets (helper);
}


static MooEditor*
page_get_editor (MooPrefsDialogPage *page)
{
    return g_object_get_data (G_OBJECT (page), "moo-editor");
}


static GtkTreeModel *
page_get_lang_model (MooPrefsDialogPage *page)
{
    GtkTreeModel *model;

    model = g_object_get_data (G_OBJECT (page), "moo-lang-model");

    if (!model)
    {
        model = create_lang_model (page_get_editor (page));
        g_object_set_data_full (G_OBJECT (page), "moo-lang-model",
                                model, g_object_unref);
    }

    return model;
}


static void
scheme_combo_set_scheme (GtkComboBox        *combo,
                         MooTextStyleScheme *scheme)
{
    GtkTreeModel *model = gtk_combo_box_get_model (combo);
    gboolean found = FALSE;
    GtkTreeIter iter;

    gtk_tree_model_get_iter_first (model, &iter);
    do {
        MooTextStyleScheme *s;
        gtk_tree_model_get (model, &iter, 0, &s, -1);
        g_object_unref (s);
        if (scheme == s)
        {
            found = TRUE;
            break;
        }
    }
    while (gtk_tree_model_iter_next (model, &iter));

    g_return_if_fail (found);

    gtk_combo_box_set_active_iter (combo, &iter);
}


static MooTextStyleScheme *
page_get_scheme (MooPrefsDialogPage *page)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    MooTextStyleScheme *scheme = NULL;
    GtkComboBox *combo;

    combo = moo_glade_xml_get_widget (page->xml, "color_scheme_combo");
    g_return_val_if_fail (combo != NULL, NULL);

    if (!gtk_combo_box_get_active_iter (combo, &iter))
        g_return_val_if_reached (NULL);

    model = gtk_combo_box_get_model (combo);
    gtk_tree_model_get (model, &iter, 0, &scheme, -1);

    return scheme;
}


static void
prefs_page_apply (MooPrefsDialogPage *page)
{
    MooTextStyleScheme *scheme;
    char *lang;
    MooEditor *editor;

    editor = page_get_editor (page);
    g_return_if_fail (editor != NULL);

    scheme = page_get_scheme (page);
    g_return_if_fail (scheme != NULL);
    moo_prefs_set_string (moo_edit_setting (MOO_EDIT_PREFS_COLOR_SCHEME),
                          moo_text_style_scheme_get_id (scheme));
    g_object_unref (scheme);

    lang = page_get_default_lang (page);
    moo_prefs_set_string (moo_edit_setting (MOO_EDIT_PREFS_DEFAULT_LANG), lang);
    g_free (lang);

    prefs_page_apply_lang_prefs (page);
    apply_filter_settings (page);
    _moo_editor_apply_prefs (editor);
}


/*********************************************************************/
/* Language combo
 */

static void
fix_style (G_GNUC_UNUSED GtkWidget *combo)
{
#ifdef __WIN32__
    static gboolean been_here = FALSE;

    gtk_widget_set_name (combo, "moo-lang-combo");

    if (!been_here)
    {
        been_here = TRUE;
        gtk_rc_parse_string ("style \"moo-lang-combo\"\n"
                             "{\n"
                             "   GtkComboBox::appears-as-list = 0\n"
                             "}\n"
                             "widget \"*.moo-lang-combo\" style \"moo-lang-combo\"\n");
    }
#endif
}


enum {
    COLUMN_ID,
    COLUMN_NAME,
    COLUMN_LANG,
    COLUMN_EXTENSIONS,
    COLUMN_MIMETYPES,
    COLUMN_CONFIG
};


static char *
list_to_string (GSList  *list,
                gboolean free_list)
{
    GSList *l;
    GString *string = g_string_new (NULL);

    for (l = list; l != NULL; l = l->next)
    {
        g_string_append (string, l->data);

        if (l->next)
            g_string_append_c (string, ';');
    }

    if (free_list)
    {
        g_slist_foreach (list, (GFunc) g_free, NULL);
        g_slist_free (list);
    }

    return g_string_free (string, FALSE);
}


static int
lang_cmp (MooLang *lang1,
          MooLang *lang2)
{
    const char *name1, *name2;

    name1 = _moo_lang_display_name (lang1);
    name2 = _moo_lang_display_name (lang2);

    return g_utf8_collate (name1, name2);
}

static GtkTreeModel *
create_lang_model (MooEditor *editor)
{
    GtkTreeStore *store;
    MooLangMgr *mgr;
    GSList *langs, *sections, *l;
    GtkTreeIter iter;
    const char *config;
    char *ext, *mime;

    mgr = moo_editor_get_lang_mgr (editor);
    langs = g_slist_sort (moo_lang_mgr_get_available_langs (mgr), (GCompareFunc) lang_cmp);
    sections = moo_lang_mgr_get_sections (mgr);

    store = gtk_tree_store_new (6, G_TYPE_STRING, G_TYPE_STRING, MOO_TYPE_LANG,
                                G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    gtk_tree_store_append (store, &iter, NULL);
    config = _moo_lang_mgr_get_config (mgr, MOO_LANG_NONE);
    ext = list_to_string (_moo_lang_mgr_get_globs (mgr, NULL), TRUE);
    mime = list_to_string (_moo_lang_mgr_get_mime_types (mgr, NULL), TRUE);
    gtk_tree_store_set (store, &iter, COLUMN_ID, MOO_LANG_NONE,
                        COLUMN_NAME, "None",
                        COLUMN_CONFIG, config,
                        COLUMN_MIMETYPES, mime,
                        COLUMN_EXTENSIONS, ext,
                        -1);
    g_free (ext);
    g_free (mime);

    /* separator */
    gtk_tree_store_append (store, &iter, NULL);

    while (sections)
    {
        char *section = sections->data;

        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, COLUMN_NAME, section, -1);

        for (l = langs; l != NULL; l = l->next)
        {
            MooLang *lang = l->data;

            if (!_moo_lang_get_hidden (lang) &&
                !strcmp (_moo_lang_get_section (lang), section))
            {
                GtkTreeIter child;

                ext = list_to_string (_moo_lang_mgr_get_globs (mgr, _moo_lang_id (lang)), TRUE);
                mime = list_to_string (_moo_lang_mgr_get_mime_types (mgr, _moo_lang_id (lang)), TRUE);

                config = _moo_lang_mgr_get_config (mgr, _moo_lang_id (lang));
                gtk_tree_store_append (store, &child, &iter);
                gtk_tree_store_set (store, &child,
                                    COLUMN_ID, _moo_lang_id (lang),
                                    COLUMN_NAME, _moo_lang_display_name (lang),
                                    COLUMN_LANG, lang,
                                    COLUMN_CONFIG, config,
                                    COLUMN_MIMETYPES, mime,
                                    COLUMN_EXTENSIONS, ext,
                                    -1);

                g_free (ext);
                g_free (mime);
            }
        }

        g_free (section);
        sections = g_slist_delete_link (sections, sections);
    }

    g_slist_foreach (langs, (GFunc) g_object_unref, NULL);
    g_slist_free (langs);
    return GTK_TREE_MODEL (store);
}


static gboolean
separator_func (GtkTreeModel *model,
                GtkTreeIter  *iter,
                G_GNUC_UNUSED gpointer data)
{
    char *name = NULL;

    gtk_tree_model_get (model, iter, COLUMN_NAME, &name, -1);

    if (!name)
        return TRUE;

    g_free (name);
    return FALSE;
}

static void
set_sensitive (G_GNUC_UNUSED GtkCellLayout *cell_layout,
               GtkCellRenderer *cell,
               GtkTreeModel    *model,
               GtkTreeIter     *iter,
               G_GNUC_UNUSED gpointer data)
{
    g_object_set (cell, "sensitive",
                  !gtk_tree_model_iter_has_child (model, iter),
                  NULL);
}

static void
default_lang_combo_init (GtkComboBox        *combo,
                         MooPrefsDialogPage *page)
{
    GtkTreeModel *model;
    GtkCellRenderer *cell;

    fix_style (GTK_WIDGET (combo));

    model = page_get_lang_model (page);
    g_return_if_fail (model != NULL);

    gtk_combo_box_set_model (combo, model);
    gtk_combo_box_set_row_separator_func (combo, separator_func, NULL, NULL);

    cell = gtk_cell_renderer_text_new ();
    gtk_cell_layout_clear (GTK_CELL_LAYOUT (combo));
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), cell,
                                    "text", COLUMN_NAME, NULL);
    gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (combo), cell,
                                        set_sensitive, NULL, NULL);
}


static gboolean
find_lang_by_id (GtkTreeModel *model,
                 G_GNUC_UNUSED GtkTreePath *path,
                 GtkTreeIter  *iter,
                 gpointer      user_data)
{
    struct {
        gboolean found;
        GtkTreeIter iter;
        const char *id;
    } *data = user_data;

    char *lang_id = NULL;

    gtk_tree_model_get (model, iter, COLUMN_ID, &lang_id, -1);

    if (_moo_str_equal(data->id, lang_id))
    {
        data->found = TRUE;
        data->iter = *iter;
        g_free (lang_id);
        return TRUE;
    }

    g_free (lang_id);
    return FALSE;
}


static void
default_lang_combo_set_lang (GtkComboBox *combo,
                             const char  *id)
{
    GtkTreeModel *model;

    struct {
        gboolean found;
        GtkTreeIter iter;
        const char *id;
    } data;

    g_return_if_fail (GTK_IS_COMBO_BOX (combo));

    model = gtk_combo_box_get_model (combo);
    data.found = FALSE;
    data.id = id ? id : MOO_LANG_NONE;

    gtk_tree_model_foreach (model,
                            (GtkTreeModelForeachFunc) find_lang_by_id,
                            &data);

    g_return_if_fail (data.found);
    gtk_combo_box_set_active_iter (combo, &data.iter);
}


static char *
page_get_default_lang (MooPrefsDialogPage *page)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    char *lang = NULL;
    GtkComboBox *combo;

    combo = moo_glade_xml_get_widget (page->xml, "default_lang_combo");
    g_return_val_if_fail (combo != NULL, NULL);

    if (!gtk_combo_box_get_active_iter (combo, &iter))
        g_return_val_if_reached (NULL);

    model = gtk_combo_box_get_model (combo);
    gtk_tree_model_get (model, &iter, COLUMN_ID, &lang, -1);

    return lang;
}


/*********************************************************************/
/* Per-lang settings
 */

static void
helper_update_widgets (MooPrefsDialogPage *page,
                       GtkTreeModel       *model,
                       G_GNUC_UNUSED GtkTreePath *path,
                       GtkTreeIter        *iter)
{
    GtkEntry *extensions, *mimetypes, *config;
    GtkWidget *label_mimetypes;
    MooLang *lang = NULL;
    char *ext = NULL, *mime = NULL, *id = NULL, *conf = NULL;

    g_return_if_fail (iter != NULL);

    extensions = moo_glade_xml_get_widget (page->xml, "extensions");
    mimetypes = moo_glade_xml_get_widget (page->xml, "mimetypes");
    label_mimetypes = moo_glade_xml_get_widget (page->xml, "label_mimetypes");
    config = moo_glade_xml_get_widget (page->xml, "config");

    gtk_tree_model_get (model, iter,
                        COLUMN_LANG, &lang,
                        COLUMN_MIMETYPES, &mime,
                        COLUMN_EXTENSIONS, &ext,
                        COLUMN_CONFIG, &conf,
                        COLUMN_ID, &id,
                        -1);
    g_return_if_fail (id != NULL);

    gtk_entry_set_text (extensions, ext ? ext : "");
    gtk_entry_set_text (mimetypes, mime ? mime : "");
    gtk_entry_set_text (config, conf ? conf : "");
    gtk_widget_set_sensitive (GTK_WIDGET (mimetypes), lang != NULL);
    gtk_widget_set_sensitive (label_mimetypes, lang != NULL);

    if (lang)
        g_object_unref (lang);

    g_free (conf);
    g_free (ext);
    g_free (mime);
    g_free (id);
}


static void
helper_update_model (MooPrefsDialogPage *page,
                     GtkTreeModel       *model,
                     G_GNUC_UNUSED GtkTreePath *path,
                     GtkTreeIter        *iter)
{
    GtkEntry *extensions, *mimetypes, *config;
    const char *ext, *mime, *conf;

    extensions = moo_glade_xml_get_widget (page->xml, "extensions");
    ext = gtk_entry_get_text (extensions);
    mimetypes = moo_glade_xml_get_widget (page->xml, "mimetypes");
    mime = gtk_entry_get_text (mimetypes);
    config = moo_glade_xml_get_widget (page->xml, "config");
    conf = gtk_entry_get_text (config);

    gtk_tree_store_set (GTK_TREE_STORE (model), iter,
                        COLUMN_MIMETYPES, mime,
                        COLUMN_EXTENSIONS, ext,
                        COLUMN_CONFIG, conf, -1);
}


static void
lang_combo_init (GtkComboBox        *combo,
                 MooPrefsDialogPage *page)
{
    GtkTreeModel *model;
    GtkCellRenderer *cell;
    MooTreeHelper *helper;

    fix_style (GTK_WIDGET (combo));

    model = page_get_lang_model (page);
    g_return_if_fail (model != NULL);

    gtk_combo_box_set_model (combo, model);
    gtk_combo_box_set_row_separator_func (combo, separator_func, NULL, NULL);

    cell = gtk_cell_renderer_text_new ();
    gtk_cell_layout_clear (GTK_CELL_LAYOUT (combo));
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), cell,
                                    "text", COLUMN_NAME, NULL);
    gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (combo), cell,
                                        set_sensitive, NULL, NULL);

    _moo_combo_box_select_first (combo);
    helper = _moo_tree_helper_new (GTK_WIDGET (combo), NULL, NULL, NULL, NULL);
    g_return_if_fail (helper != NULL);

    g_object_set_data_full (G_OBJECT (page), "moo-tree-helper",
                            helper, g_object_unref);
    g_signal_connect_swapped (helper, "update-widgets",
                              G_CALLBACK (helper_update_widgets), page);
    g_signal_connect_swapped (helper, "update-model",
                              G_CALLBACK (helper_update_model), page);
    _moo_tree_helper_update_widgets (helper);
}


static gboolean
apply_one_lang (GtkTreeModel *model,
                G_GNUC_UNUSED GtkTreePath *path,
                GtkTreeIter  *iter,
                MooLangMgr   *mgr)
{
    MooLang *lang = NULL;
    char *config = NULL, *id = NULL;
    char *mime, *ext;

    gtk_tree_model_get (model, iter,
                        COLUMN_LANG, &lang,
                        COLUMN_ID, &id,
                        COLUMN_CONFIG, &config, -1);

    if (!id)
        return FALSE;

    gtk_tree_model_get (model, iter,
                        COLUMN_MIMETYPES, &mime,
                        COLUMN_EXTENSIONS, &ext, -1);

    _moo_lang_mgr_set_mime_types (mgr, id, mime);
    _moo_lang_mgr_set_globs (mgr, id, ext);
    _moo_lang_mgr_set_config (mgr, id, config);

    if (lang)
        g_object_unref (lang);

    g_free (mime);
    g_free (ext);
    g_free (config);
    g_free (id);
    return FALSE;
}


static void
prefs_page_apply_lang_prefs (MooPrefsDialogPage *page)
{
    GtkTreeModel *model;
    MooTreeHelper *helper;
    MooLangMgr *mgr;

    helper = g_object_get_data (G_OBJECT (page), "moo-tree-helper");
    _moo_tree_helper_update_model (helper, NULL, NULL);

    model = page_get_lang_model (page);
    g_return_if_fail (model != NULL);

    mgr = moo_editor_get_lang_mgr (page_get_editor (page));
    gtk_tree_model_foreach (model, (GtkTreeModelForeachFunc) apply_one_lang, mgr);
    _moo_lang_mgr_save_config (mgr);
    _moo_edit_update_lang_config ();
}


/*********************************************************************/
/* Filters
 */

enum {
    FILTER_COLUMN_FILTER,
    FILTER_COLUMN_CONFIG,
    FILTER_NUM_COLUMNS
};


static void
filter_store_set_modified (gpointer store,
                           gboolean modified)
{
    g_return_if_fail (GTK_IS_LIST_STORE (store));
    g_object_set_data (store, "filter-store-modified",
                       GINT_TO_POINTER (modified));
}

static gboolean
filter_store_get_modified (gpointer store)
{
    g_return_val_if_fail (GTK_IS_LIST_STORE (store), FALSE);
    return g_object_get_data (store, "filter-store-modified") != NULL;
}

static void
populate_filter_settings_store (GtkListStore *store)
{
    GSList *strings, *l;

    _moo_edit_filter_settings_load ();

    l = strings = _moo_edit_filter_settings_get_strings ();

    while (l)
    {
        GtkTreeIter iter;

        g_return_if_fail (l->next != NULL);

        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter,
                            FILTER_COLUMN_FILTER, l->data,
                            FILTER_COLUMN_CONFIG, l->next->data,
                            -1);

        l = l->next->next;
    }

    g_slist_foreach (strings, (GFunc) g_free, NULL);
    g_slist_free (strings);
}


static void
filter_cell_edited (GtkCellRendererText *cell,
                    const char          *path_string,
                    const char          *text,
                    GtkListStore        *store)
{
    GtkTreeIter iter;
    GtkTreePath *path;
    int column;
    char *old_text;

    g_return_if_fail (GTK_IS_LIST_STORE (store));

    column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (cell), "filter-store-column-id"));
    g_return_if_fail (column >= 0 && column < FILTER_NUM_COLUMNS);

    path = gtk_tree_path_new_from_string (path_string);

    if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter, path))
    {
        gtk_tree_path_free (path);
        return;
    }

    gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, column, &old_text, -1);

    if (!_moo_str_equal (old_text, text))
    {
        gtk_list_store_set (store, &iter, column, text, -1);
        filter_store_set_modified (store, TRUE);
    }

    g_free (old_text);
    gtk_tree_path_free (path);
}


static void
create_filter_cell (GtkTreeView  *treeview,
                    GtkListStore *store,
                    const char   *title,
                    int           column_id)
{
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;

    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (title, cell, "text", column_id, NULL);
    gtk_tree_view_append_column (treeview, column);

    g_object_set (cell, "editable", TRUE, NULL);
    g_object_set_data (G_OBJECT (cell), "filter-store-column-id", GINT_TO_POINTER (column_id));
    g_signal_connect (cell, "edited", G_CALLBACK (filter_cell_edited), store);
}


static void
filter_treeview_init (MooGladeXML *xml)
{
    GtkTreeView *filter_treeview;
    GtkListStore *store;
    MooTreeHelper *helper;

    filter_treeview = moo_glade_xml_get_widget (xml, "filter_treeview");
    g_return_if_fail (filter_treeview != NULL);

    store = gtk_list_store_new (FILTER_NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);
    populate_filter_settings_store (store);
    gtk_tree_view_set_model (filter_treeview, GTK_TREE_MODEL (store));

    create_filter_cell (filter_treeview, store, "Filter", FILTER_COLUMN_FILTER);
    create_filter_cell (filter_treeview, store, "Options", FILTER_COLUMN_CONFIG);

    helper = _moo_tree_helper_new (GTK_WIDGET (filter_treeview),
                                   moo_glade_xml_get_widget (xml, "new_filter_setting"),
                                   moo_glade_xml_get_widget (xml, "delete_filter_setting"),
                                   moo_glade_xml_get_widget (xml, "filter_setting_up"),
                                   moo_glade_xml_get_widget (xml, "filter_setting_down"));
    gtk_object_sink (g_object_ref (helper));
    g_object_set_data_full (G_OBJECT (filter_treeview), "tree-helper", helper, g_object_unref);

    g_object_unref (store);
}


static gboolean
prepend_filter_and_config (GtkTreeModel *model,
                           G_GNUC_UNUSED GtkTreePath *path,
                           GtkTreeIter  *iter,
                           GSList      **list)
{
    char *filter = NULL, *config = NULL;

    gtk_tree_model_get (model, iter,
                        FILTER_COLUMN_FILTER, &filter,
                        FILTER_COLUMN_CONFIG, &config,
                        -1);

    *list = g_slist_prepend (*list, filter ? filter : g_strdup (""));
    *list = g_slist_prepend (*list, config ? config : g_strdup (""));

    return FALSE;
}

static void
apply_filter_settings (MooPrefsDialogPage *page)
{
    GtkTreeView *filter_treeview;
    GSList *strings = NULL;
    GtkTreeModel *model;

    filter_treeview = moo_glade_xml_get_widget (page->xml, "filter_treeview");
    g_return_if_fail (filter_treeview != NULL);

    model = gtk_tree_view_get_model (filter_treeview);

    if (!filter_store_get_modified (model))
        return;

    gtk_tree_model_foreach (model,
                            (GtkTreeModelForeachFunc) prepend_filter_and_config,
                            &strings);
    strings = g_slist_reverse (strings);

    _moo_edit_filter_settings_set_strings (strings);
    filter_store_set_modified (model, FALSE);

    g_slist_foreach (strings, (GFunc) g_free, NULL);
    g_slist_free (strings);
}
