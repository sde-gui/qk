/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooeditprefspage.c
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

#define MOOEDIT_COMPILATION
#include "mooedit/mooedit-private.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mooeditprefs-glade.h"
#include "mooedit/mooeditlangmgr.h"
#include "mooutils/mooprefsdialog.h"
#include "mooutils/moocompat.h"
#include "mooutils/moostock.h"
#include "mooutils/mooglade.h"
#include <string.h>


typedef struct _Settings Settings;
static Settings *highlighting_settings_new  (MooPrefsDialogPage *page,
                                             GtkListStore       *styles_list);
static void highighting_settings_free       (Settings           *hs);
static void highlighting_settings_set_lang  (Settings           *hs,
                                             MooEditLang        *lang);
static void highlighting_settings_set_style (Settings           *hs,
                                             const char         *style_id);


static void styles_list_selection_changed   (GtkTreeSelection   *selection,
                                             Settings           *set);
static void hookup_highlighting_prefs       (MooPrefsDialogPage *page,
                                             MooEditor          *editor);
static void setup_language_combo            (GtkWidget          *combo,
                                             Settings           *set);
#if GTK_CHECK_VERSION(2,4,0)
static void language_combo_changed          (GtkComboBox        *combo,
                                             Settings           *set);
#else /* !GTK_CHECK_VERSION(2,4,0) */
static void language_menu_changed           (GtkOptionMenu      *menu,
                                             Settings           *set);
#endif /* !GTK_CHECK_VERSION(2,4,0) */
static Settings *setup_styles_list          (GtkTreeView        *list,
                                             MooPrefsDialogPage *page);
static void styles_list_selection_changed   (GtkTreeSelection   *selection,
                                             Settings           *set);
static void moo_edit_prefs_page_init        (MooPrefsDialogPage *page,
                                             MooEditor          *editor);


GtkWidget*
moo_edit_prefs_page_new (MooEditor *editor)
{
    GtkWidget *page;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    page = moo_prefs_dialog_page_new_from_xml ("Editor",
                                               GTK_STOCK_EDIT,
                                               MOO_EDIT_PREFS_GLADE_UI,
                                               -1,
                                               "page",
                                               MOO_EDIT_PREFS_PREFIX);
    g_return_val_if_fail (page != NULL, NULL);

    hookup_highlighting_prefs (MOO_PREFS_DIALOG_PAGE (page), editor);
    return page;
}


static void hookup_highlighting_prefs (MooPrefsDialogPage *page,
                                       MooEditor          *editor)
{
    GtkTreeView *styles_list;
    Settings *set;

    styles_list = moo_glade_xml_get_widget (page->xml, "styles_list");
    set = setup_styles_list (styles_list, page);
    g_return_if_fail (set != NULL);

    setup_language_combo (moo_glade_xml_get_widget (page->xml, "language_combo"), set);

    g_signal_connect (page, "init",
                      G_CALLBACK (moo_edit_prefs_page_init),
                      editor);
}


enum {
   LANGNAME_COLUMN,
   LANG_COLUMN
};
enum {
   STYLE_ID_COLUMN,
   STYLE_NAME_COLUMN
};


#if GTK_CHECK_VERSION(2,4,0)

static void setup_language_combo   (GtkWidget          *c,
                                    Settings           *set)
{
    GtkComboBox *combo;
    GtkListStore *store;
    GtkCellRenderer *cell;

    combo = GTK_COMBO_BOX (c);
    store = gtk_list_store_new (2, G_TYPE_STRING,
                                   MOO_TYPE_EDIT_LANG);
    gtk_combo_box_set_model (combo, GTK_TREE_MODEL (store));
    g_object_unref (G_OBJECT (store));

    cell = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo),
                                cell, FALSE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combo),
                                   cell,
                                   "text", LANGNAME_COLUMN);

    g_signal_connect (G_OBJECT (combo), "changed",
                      G_CALLBACK (language_combo_changed), set);
}


static void language_combo_changed (GtkComboBox *combo,
                                    Settings    *set)
{
    GtkTreeIter iter;
    GtkTreeModel *store;
    MooEditLang *lang = NULL;

    g_return_if_fail (gtk_combo_box_get_active_iter (combo, &iter));

    store = gtk_combo_box_get_model (combo);
    gtk_tree_model_get (store, &iter, LANG_COLUMN, &lang, -1);
    g_return_if_fail (lang != NULL);
    highlighting_settings_set_lang (set, lang);
    g_object_unref (lang);
}

#else /* !GTK_CHECK_VERSION(2,4,0) */

static void setup_language_combo   (GtkWidget   *c,
                                    Settings    *set)
{
    GtkOptionMenu *menu = GTK_OPTION_MENU (c);

    GtkListStore *store = gtk_list_store_new (2,
                                              G_TYPE_STRING,
                                              MOO_TYPE_EDIT_LANG);
    g_object_set_data_full (G_OBJECT (menu), "lang-list", store,
    (GDestroyNotify)g_object_unref);

    g_signal_connect (G_OBJECT (menu), "changed",
                      G_CALLBACK (language_menu_changed), set);
}


static void language_menu_changed   (GtkOptionMenu      *menu,
                                     Settings           *set)
{
    int current = gtk_option_menu_get_history (menu);
    MooEditLang *lang = NULL;
    GtkTreeModel *store = GTK_TREE_MODEL (g_object_get_data (G_OBJECT (menu), "lang-list"));
    GtkTreePath *path = gtk_tree_path_new_from_indices (current, -1);
    GtkTreeIter iter;

    if (!gtk_tree_model_get_iter (store, &iter, path))
    {
        gtk_tree_path_free (path);
        g_return_if_fail (FALSE);
    }
    gtk_tree_path_free (path);

    gtk_tree_model_get (store, &iter, LANG_COLUMN, &lang, -1);
    g_return_if_fail (lang != NULL);
    highlighting_settings_set_lang (set, lang);
    g_object_unref (lang);
}
#endif /* !GTK_CHECK_VERSION(2,4,0) */


static Settings *setup_styles_list (GtkTreeView        *list,
                                    MooPrefsDialogPage *page)
{
    GtkListStore *store;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkTreeSelection *selection;
    Settings *set;

    store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_view_set_model (list, GTK_TREE_MODEL (store));
    g_object_unref (G_OBJECT (store));

    gtk_tree_view_set_headers_visible (list, FALSE);
    gtk_tree_view_set_enable_search (list, FALSE);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Style",
                                                       renderer,
                                                       "text", STYLE_NAME_COLUMN,
                                                       NULL);
    gtk_tree_view_append_column (list, column);

    selection = gtk_tree_view_get_selection (list);
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

    set = highlighting_settings_new (page, store);

    g_signal_connect (G_OBJECT (selection), "changed",
                      G_CALLBACK (styles_list_selection_changed),
                      set);

    return set;
}


static void styles_list_selection_changed    (GtkTreeSelection  *selection,
                                              Settings          *set)
{
    GtkTreeIter iter;
    GtkTreeModel *model;

    if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        char *style_id = NULL;
        gtk_tree_model_get (model, &iter, STYLE_ID_COLUMN, &style_id, -1);
        g_return_if_fail (style_id != NULL);
        highlighting_settings_set_style (set, style_id);
        g_free (style_id);
    }
    else {
        g_critical ("%s: nothing selected", G_STRLOC);
        if (gtk_tree_model_get_iter_first (model, &iter)) {
            gtk_tree_selection_select_iter (selection, &iter);
        }
        else {
            g_critical ("%s: list is empty", G_STRLOC);
        }
    }
}


static void
moo_edit_prefs_page_init (MooPrefsDialogPage *page,
                          MooEditor          *editor)
{
    GtkListStore *store;
    const GSList *langs, *l;
    GtkTreeIter iter;
    GtkWidget *language_combo = moo_glade_xml_get_widget (page->xml, "language_combo");
    g_return_if_fail (language_combo != NULL);

#if GTK_CHECK_VERSION(2,4,0)
    store = GTK_LIST_STORE (gtk_combo_box_get_model (GTK_COMBO_BOX (language_combo)));
#else /* !GTK_CHECK_VERSION(2,4,0) */
    store = GTK_LIST_STORE (g_object_get_data (G_OBJECT (language_combo), "lang-list"));
#endif /* !GTK_CHECK_VERSION(2,4,0) */
    gtk_list_store_clear (store);

    g_return_if_fail (editor != NULL);

    langs = moo_edit_lang_mgr_get_available_languages (moo_editor_get_lang_mgr (editor));

    for (l = langs; l != NULL; l = l->next)
    {
        MooEditLang *lang = MOO_EDIT_LANG (l->data);
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter,
                            LANGNAME_COLUMN, moo_edit_lang_get_name (lang),
                            LANG_COLUMN, lang, -1);
    }

#if GTK_CHECK_VERSION(2,4,0)
    gtk_combo_box_set_active (GTK_COMBO_BOX (language_combo), 0);
#else /* !GTK_CHECK_VERSION(2,4,0) */
    G_STMT_START {
        GtkWidget *menu;
        GtkWidget *item = NULL;
        char *label = NULL;

        menu = gtk_menu_new ();
        gtk_widget_show (menu);

        g_return_if_fail (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter));
        gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, LANGNAME_COLUMN, &label, -1);
        item = gtk_menu_item_new_with_label (label);
        gtk_widget_show (item);
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
        g_free (label);

        item = gtk_separator_menu_item_new ();
        gtk_widget_show (item);
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
        gtk_widget_set_sensitive (item, FALSE);

        if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter))
        do {
            gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, LANGNAME_COLUMN, &label, -1);
            item = gtk_menu_item_new_with_label (label);
            gtk_widget_show (item);
            gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
            g_free (label);
        } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (store), &iter));

        gtk_option_menu_set_menu (GTK_OPTION_MENU (language_combo), menu);
    } G_STMT_END;
#endif /* !GTK_CHECK_VERSION(2,4,0) */
}


/****************************************************************************/
/****************************************************************************/

typedef struct {
    MooEditLang *lang;
    char        *style_id;
} LangStylePair;

static LangStylePair    *lang_style_pair_copy   (const LangStylePair *p);
static void              lang_style_pair_free   (LangStylePair  *pair);
static guint             lang_style_pair_hash   (LangStylePair  *pair);
static gboolean          lang_style_pair_compare(LangStylePair  *a,
                                                 LangStylePair  *b);

static void              highlighting_settings_init     (Settings  *hs);
static void              highlighting_settings_apply    (Settings  *hs);
static void              highlighting_settings_set      (Settings  *hs);
static void              highlighting_settings_unset    (Settings  *hs);


struct _Settings {
    GHashTable          *styles; /* LangStylePair* -> GtkSourceTagStyle* */
    MooPrefsDialogPage  *page;

    GtkToggleButton     *bold;
    GtkToggleButton     *italic;
    GtkToggleButton     *underline;
    GtkToggleButton     *strikethrough;
    GtkToggleButton     *foreground_toggle;
    GtkToggleButton     *background_toggle;
    GtkColorButton      *foreground_color;
    GtkColorButton      *background_color;

    GtkListStore        *styles_list;
    GtkTreeSelection    *styles_selection;
    GtkWidget           *style_vbox;
    LangStylePair       *current;
    MooEditLang         *current_lang;
};


static Settings*
highlighting_settings_new (MooPrefsDialogPage *page,
                           GtkListStore       *styles_list)
{
    Settings *hs = g_new0 (Settings, 1);

    hs->page = page;
    hs->styles_list = styles_list;
    hs->styles_selection = gtk_tree_view_get_selection (moo_glade_xml_get_widget (page->xml, "styles_list"));
    hs->style_vbox = moo_glade_xml_get_widget (page->xml, "style_vbox");
    g_assert (GTK_IS_LIST_STORE (hs->styles_list));

    hs->bold = moo_glade_xml_get_widget (page->xml, "bold");
    hs->italic = moo_glade_xml_get_widget (page->xml, "italic");
    hs->underline = moo_glade_xml_get_widget (page->xml, "underline");
    hs->strikethrough = moo_glade_xml_get_widget (page->xml, "strikethrough");
    hs->foreground_toggle = moo_glade_xml_get_widget (page->xml, "foreground_toggle");
    hs->foreground_color = moo_glade_xml_get_widget (page->xml, "foreground_color");
    hs->background_toggle = moo_glade_xml_get_widget (page->xml, "background_toggle");
    hs->background_color = moo_glade_xml_get_widget (page->xml, "background_color");

    g_signal_connect_swapped (page, "apply", G_CALLBACK (highlighting_settings_apply), hs);
    g_signal_connect_swapped (page, "init", G_CALLBACK (highlighting_settings_init), hs);
    g_signal_connect_swapped (page, "destroy", G_CALLBACK (highighting_settings_free), hs);

    hs->styles = g_hash_table_new_full ((GHashFunc) lang_style_pair_hash,
                                        (GEqualFunc) lang_style_pair_compare,
                                        (GDestroyNotify) lang_style_pair_free,
                                        (GDestroyNotify) gtk_source_tag_style_free);

    return hs;
}


static void highighting_settings_free       (Settings           *hs)
{
    g_hash_table_destroy (hs->styles);
    g_free (hs);
}


static void highlighting_settings_set_lang  (Settings           *hs,
                                             MooEditLang        *lang)
{
    GPtrArray *styles;
    guint i;

    g_return_if_fail (lang != NULL);

    if (hs->current) highlighting_settings_unset (hs);
    hs->current = NULL;

    hs->current_lang = lang;
    g_signal_handlers_block_matched (hs->styles_selection,
                                     G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC,
                                     0, 0, 0, (gpointer)styles_list_selection_changed,
                                     hs);

    gtk_list_store_clear (hs->styles_list);
    styles = moo_edit_lang_get_style_names (lang);
    for (i = 0; i < styles->len && styles->pdata[i]; i += 2) {
        GtkTreeIter iter;
        gtk_list_store_append (hs->styles_list, &iter);
        gtk_list_store_set (hs->styles_list, &iter,
                            STYLE_ID_COLUMN, styles->pdata[i],
                            STYLE_NAME_COLUMN, styles->pdata[i+1],
                            -1);
    }
    g_strfreev ((char**)styles->pdata);
    g_ptr_array_free (styles, FALSE);

    g_signal_handlers_unblock_matched (hs->styles_selection,
                                       G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC,
                                       0, 0, 0, (gpointer)styles_list_selection_changed,
                                       hs);

    GtkTreeIter iter;
    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (hs->styles_list), &iter)) {
        gtk_tree_selection_select_iter (hs->styles_selection, &iter);
    }
    else {
        gtk_widget_set_sensitive (hs->style_vbox, FALSE);
    }
}


static void highlighting_settings_set_style (Settings           *hs,
                                             const char         *style_id)
{
    LangStylePair p = {hs->current_lang, (char*)style_id};
    LangStylePair *key = NULL;
    GtkSourceTagStyle *s;

    g_return_if_fail (hs->current_lang != NULL && style_id != NULL);

    if (hs->current) highlighting_settings_unset (hs);
    hs->current = NULL;
    gtk_widget_set_sensitive (hs->style_vbox, TRUE);

    g_hash_table_lookup_extended (hs->styles, &p, (gpointer*)&key, (gpointer*)&s);
    if (!key) {
        LangStylePair *np = lang_style_pair_copy (&p);
        GtkSourceTagStyle *style =
            gtk_source_tag_style_copy (moo_edit_lang_get_style (hs->current_lang,
                                                                style_id));
        g_hash_table_insert (hs->styles, np, style);
        hs->current = np;
    }
    else
        hs->current = key;

    highlighting_settings_set (hs);
}


static void              highlighting_settings_init     (Settings  *hs)
{
    if (hs->current) highlighting_settings_set (hs);
}


static void apply_style (LangStylePair *p, GtkSourceTagStyle *s)
{
    g_return_if_fail (p != NULL && s != NULL);
    g_return_if_fail (p->lang != NULL && p->style_id != NULL);
    moo_edit_lang_set_style (p->lang, p->style_id, s);
}


static void              highlighting_settings_apply    (Settings  *hs)
{
    if (hs->current) highlighting_settings_unset (hs);
    g_hash_table_foreach (hs->styles, (GHFunc) apply_style, NULL);
}


static void              highlighting_settings_set      (Settings  *hs)
{
    GtkSourceTagStyle *s = NULL;

    g_return_if_fail (hs->current != NULL);

    s = g_hash_table_lookup (hs->styles, hs->current);
    g_return_if_fail (s != NULL);

    gtk_toggle_button_set_active (hs->bold, s->bold);
    gtk_toggle_button_set_active (hs->italic, s->italic);
    gtk_toggle_button_set_active (hs->underline, s->underline);
    gtk_toggle_button_set_active (hs->strikethrough, s->strikethrough);
    gtk_toggle_button_set_active (hs->foreground_toggle, s->mask & GTK_SOURCE_TAG_STYLE_USE_FOREGROUND);
    gtk_toggle_button_set_active (hs->background_toggle, s->mask & GTK_SOURCE_TAG_STYLE_USE_BACKGROUND);
    gtk_color_button_set_color (hs->foreground_color, &(s->foreground));
    gtk_color_button_set_color (hs->background_color, &(s->background));
}


static void              highlighting_settings_unset    (Settings  *hs)
{
    GtkSourceTagStyle *s = NULL;
    GdkColor c;

    g_return_if_fail (hs->current != NULL);
    s = g_hash_table_lookup (hs->styles, hs->current);
    g_return_if_fail (s != NULL);

    s->bold = gtk_toggle_button_get_active (hs->bold);
    s->italic = gtk_toggle_button_get_active (hs->italic);
    s->underline = gtk_toggle_button_get_active (hs->underline);
    s->strikethrough = gtk_toggle_button_get_active (hs->strikethrough);

    if (gtk_toggle_button_get_active (hs->foreground_toggle)) {
        s->mask |= GTK_SOURCE_TAG_STYLE_USE_FOREGROUND;
        gtk_color_button_get_color (hs->foreground_color, &c);
        s->foreground = c;
    }
    else
        s->mask &= ~GTK_SOURCE_TAG_STYLE_USE_FOREGROUND;

    if (gtk_toggle_button_get_active (hs->background_toggle)) {
        s->mask |= GTK_SOURCE_TAG_STYLE_USE_BACKGROUND;
        gtk_color_button_get_color (hs->background_color, &c);
        s->background = c;
    }
    else
        s->mask &= ~GTK_SOURCE_TAG_STYLE_USE_BACKGROUND;
}


static LangStylePair    *lang_style_pair_copy   (const LangStylePair *p)
{
    LangStylePair *np = g_new (LangStylePair, 1);
    np->lang = p->lang;
    np->style_id = g_strdup (p->style_id);
    return np;
}


static void              lang_style_pair_free   (LangStylePair  *pair)
{
    g_return_if_fail (pair != NULL);
    g_free (pair->style_id);
    g_free (pair);
}


static guint             lang_style_pair_hash   (LangStylePair  *pair)
{
    if (!pair) return 0;
    else return (g_direct_hash(pair->lang)) ^ (g_str_hash(pair->style_id));
}


static gboolean          lang_style_pair_compare(LangStylePair  *a,
                                                 LangStylePair  *b)
{
    g_return_val_if_fail (a != NULL && b != NULL, a == b);
    return a->lang == b->lang && !strcmp (a->style_id, b->style_id);
}
