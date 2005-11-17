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
// #include "mooedit/mooeditprefs-glade.h"
#include "mooedit/mooeditcolorsprefs-glade.h"
#include "mooutils/mooprefsdialog.h"
#include "mooutils/moocompat.h"
#include "mooutils/moostock.h"
#include "mooutils/mooglade.h"
#include "mooutils/moocellrenderercolor.h"
#include <string.h>


static void     color_page_init             (MooPrefsDialogPage *page);
static void     color_page_apply            (MooPrefsDialogPage *page);

static void     scheme_combo_init           (GtkWidget          *combo,
                                             MooEditor          *editor);
static void     scheme_combo_data_func      (GtkCellLayout      *layout,
                                             GtkCellRenderer    *cell,
                                             GtkTreeModel       *model,
                                             GtkTreeIter        *iter);
static void     scheme_combo_set_scheme     (GtkComboBox        *combo,
                                             MooTextStyleScheme *scheme);
static void     scheme_combo_changed        (GtkComboBox        *combo,
                                             MooPrefsDialogPage *page);

static void     syntax_combo_init           (GtkWidget          *combo,
                                             MooEditor          *editor);
static void     syntax_combo_data_func      (GtkCellLayout      *layout,
                                             GtkCellRenderer    *cell,
                                             GtkTreeModel       *model,
                                             GtkTreeIter        *iter);
static void     syntax_combo_set_lang       (GtkComboBox        *combo,
                                             MooLang            *lang);
static void     syntax_combo_changed        (GtkComboBox        *combo,
                                             MooPrefsDialogPage *page);

static MooLangMgr *page_get_mgr             (MooPrefsDialogPage *page);
static MooEditor *page_get_editor           (MooPrefsDialogPage *page);
static void     page_set_scheme             (MooPrefsDialogPage *page,
                                             MooTextStyleScheme *scheme);
static void     page_set_lang               (MooPrefsDialogPage *page,
                                             MooLang            *lang);
static GtkWidget *page_get_preview          (MooPrefsDialogPage *page);
static void     preview_set_lang            (GtkWidget          *preview,
                                             MooLang            *lang);
static MooTextStyleScheme *page_get_scheme  (MooPrefsDialogPage *page);
static MooLang *page_get_lang               (MooPrefsDialogPage *page);


static void     default_treeview_init       (GtkTreeView        *treeview,
                                             MooPrefsDialogPage *page);
static void     default_treeview_set_scheme (GtkTreeView        *treeview,
                                             MooTextStyleScheme *scheme);

static void     lang_treeview_init          (GtkTreeView        *treeview,
                                             MooPrefsDialogPage *page);

static void     treeview_apply_scheme       (GtkWidget          *widget,
                                             MooTextStyleScheme *scheme);


GtkWidget*
moo_edit_colors_prefs_page_new (MooEditor      *editor)
{
    MooPrefsDialogPage *page;
    GtkWidget *scheme_combo, *syntax_combo;
    GtkTreeView *default_treeview;//, *lang_treeview;
    MooGladeXML *xml;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    xml = moo_glade_xml_new_empty ();
    moo_glade_xml_map_id (xml, "page", MOO_TYPE_PREFS_DIALOG_PAGE);
    moo_glade_xml_map_id (xml, "sample_view", MOO_TYPE_TEXT_VIEW);
    moo_glade_xml_parse_memory (xml, MOO_EDIT_COLORS_PREFS_GLADE_UI, -1, "page");

    page = moo_glade_xml_get_widget (xml, "page");
    page->xml = xml;

    g_object_set (page,
                  "label", "Font and Colors",
                  "icon-stock-id", GTK_STOCK_EDIT,
                  NULL);

    g_object_set_data_full (G_OBJECT (page), "moo-editor",
                            g_object_ref (editor), g_object_unref);
    g_object_set_data_full (G_OBJECT (page), "moo-lang-mgr",
                            g_object_ref (moo_editor_get_lang_mgr (editor)),
                            g_object_unref);

    g_signal_connect (page, "init", G_CALLBACK (color_page_init), NULL);
//     g_signal_connect (page, "apply", G_CALLBACK (color_page_apply), NULL);

    scheme_combo = moo_glade_xml_get_widget (xml, "scheme_combo");
    scheme_combo_init (scheme_combo, editor);
    g_signal_connect (scheme_combo, "changed",
                      G_CALLBACK (scheme_combo_changed), page);

    syntax_combo = moo_glade_xml_get_widget (xml, "syntax_combo");
    syntax_combo_init (syntax_combo, editor);
    g_signal_connect (syntax_combo, "changed",
                      G_CALLBACK (syntax_combo_changed), page);

    default_treeview = moo_glade_xml_get_widget (xml, "default_treeview");
    default_treeview_init (default_treeview, page);

//     lang_treeview = moo_glade_xml_get_widget (xml, "lang_treeview");
//     lang_treeview_init (lang_treeview, page, editor);

    return GTK_WIDGET (page);
}


static void
scheme_combo_init (GtkWidget          *combo,
                   MooEditor          *editor)
{
    GtkListStore *store;
    MooLangMgr *mgr;
    GSList *list, *l;
    GtkCellRenderer *cell;

    mgr = moo_editor_get_lang_mgr (editor);
    list = moo_lang_mgr_list_schemes (mgr);
    g_return_if_fail (list != NULL);

    store = gtk_list_store_new (1, MOO_TYPE_TEXT_STYLE_SCHEME);

    for (l = list; l != NULL; l = l->next)
    {
        GtkTreeIter iter;
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 0, l->data, -1);
    }

    gtk_combo_box_set_model (GTK_COMBO_BOX (combo), GTK_TREE_MODEL (store));

    cell = gtk_cell_renderer_text_new ();
    gtk_cell_layout_clear (GTK_CELL_LAYOUT (combo));
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, TRUE);
    gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (combo), cell,
                                        (GtkCellLayoutDataFunc) scheme_combo_data_func,
                                        NULL, NULL);

    g_object_unref (store);
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
    g_object_set (cell, "text", scheme->name, NULL);
    g_object_unref (scheme);
}


static GtkWidget*
page_get_preview (MooPrefsDialogPage *page)
{
    return moo_glade_xml_get_widget (page->xml, "sample_view");
}


static void
color_page_init (MooPrefsDialogPage *page)
{
    MooEditor *editor;
    MooLangMgr *mgr;
    MooTextStyleScheme *scheme;
    GtkComboBox *scheme_combo, *syntax_combo;

    editor = page_get_editor (page);
    mgr = moo_editor_get_lang_mgr (editor);
    scheme = moo_lang_mgr_get_active_scheme (mgr);
    g_return_if_fail (scheme != NULL);

    scheme_combo = moo_glade_xml_get_widget (page->xml, "scheme_combo");
    scheme_combo_set_scheme (scheme_combo, scheme);

    syntax_combo = moo_glade_xml_get_widget (page->xml, "syntax_combo");
    syntax_combo_set_lang (syntax_combo, NULL);
}


static MooEditor*
        page_get_editor (MooPrefsDialogPage *page)
{
    return g_object_get_data (G_OBJECT (page), "moo-editor");
}


static MooLangMgr*
page_get_mgr (MooPrefsDialogPage *page)
{
    return g_object_get_data (G_OBJECT (page), "moo-lang-mgr");
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


static void
scheme_combo_changed (GtkComboBox        *combo,
                      MooPrefsDialogPage *page)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    MooTextStyleScheme *scheme = NULL;

    if (!gtk_combo_box_get_active_iter (combo, &iter))
        g_return_if_reached ();

    model = gtk_combo_box_get_model (combo);
    gtk_tree_model_get (model, &iter, 0, &scheme, -1);
    g_return_if_fail (scheme != NULL);

    page_set_scheme (page, scheme);
    g_object_unref (scheme);
}


static void
page_set_scheme (MooPrefsDialogPage *page,
                 MooTextStyleScheme *scheme)
{
    GtkColorButton *fg, *bg, *sel_fg, *sel_bg, *cur_line;
    GdkColor color;
    GdkScreen *screen;

    g_return_if_fail (scheme != NULL);

    g_object_set_data (G_OBJECT (page), "moo-text-style-scheme", scheme);

    if (gtk_widget_has_screen (GTK_WIDGET (page)))
        screen = gtk_widget_get_screen (GTK_WIDGET (page));
    else
        screen = gdk_screen_get_default ();

    fg = moo_glade_xml_get_widget (page->xml, "foreground");
    bg = moo_glade_xml_get_widget (page->xml, "background");
    sel_fg = moo_glade_xml_get_widget (page->xml, "selected_foreground");
    sel_bg = moo_glade_xml_get_widget (page->xml, "selected_background");
    cur_line = moo_glade_xml_get_widget (page->xml, "current_line");

    _moo_text_style_scheme_get_color (scheme, MOO_TEXT_COLOR_FG, &color, screen);
    gtk_color_button_set_color (fg, &color);
    _moo_text_style_scheme_get_color (scheme, MOO_TEXT_COLOR_BG, &color, screen);
    gtk_color_button_set_color (bg, &color);
    _moo_text_style_scheme_get_color (scheme, MOO_TEXT_COLOR_SEL_FG, &color, screen);
    gtk_color_button_set_color (sel_fg, &color);
    _moo_text_style_scheme_get_color (scheme, MOO_TEXT_COLOR_SEL_BG, &color, screen);
    gtk_color_button_set_color (sel_bg, &color);
    _moo_text_style_scheme_get_color (scheme, MOO_TEXT_COLOR_CUR_LINE, &color, screen);
    gtk_color_button_set_color (cur_line, &color);

    moo_text_view_apply_scheme (MOO_TEXT_VIEW (page_get_preview (page)), scheme);
    default_treeview_set_scheme (moo_glade_xml_get_widget (page->xml, "default_treeview"), scheme);
}


static int
cmp_langs (MooLang *lang1,
           MooLang *lang2)
{
    int result;

    if (!lang1 || !lang2)
        return lang1 < lang2 ? -1 : (lang1 > lang2 ? 1 : 0);

    result = strcmp (lang1->section, lang2->section);
    if (!result)
        result = strcmp (lang1->display_name, lang2->display_name);

    return result;
}


static void
syntax_combo_init (GtkWidget          *combo,
                   MooEditor          *editor)
{
    GtkListStore *store;
    MooLangMgr *mgr;
    GSList *list, *l;
    GtkCellRenderer *cell;

    mgr = moo_editor_get_lang_mgr (editor);

    list = moo_lang_mgr_get_available_langs (mgr);
    list = g_slist_prepend (list, NULL);
    list = g_slist_sort (list, (GCompareFunc) cmp_langs);

    store = gtk_list_store_new (1, MOO_TYPE_LANG);

    for (l = list; l != NULL; l = l->next)
    {
        GtkTreeIter iter;
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 0, l->data, -1);
    }

    gtk_combo_box_set_model (GTK_COMBO_BOX (combo), GTK_TREE_MODEL (store));

    cell = gtk_cell_renderer_text_new ();
    gtk_cell_layout_clear (GTK_CELL_LAYOUT (combo));
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, TRUE);
    gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (combo), cell,
                                        (GtkCellLayoutDataFunc) syntax_combo_data_func,
                                        NULL, NULL);

    g_object_unref (store);
    g_slist_free (list);
}


static void
syntax_combo_data_func (G_GNUC_UNUSED GtkCellLayout *layout,
                        GtkCellRenderer    *cell,
                        GtkTreeModel       *model,
                        GtkTreeIter        *iter)
{
    MooLang *lang = NULL;

    gtk_tree_model_get (model, iter, 0, &lang, -1);

    if (lang)
    {
        char *text = g_strdup_printf ("%s/%s", lang->section, lang->display_name);
        g_object_set (cell, "text", text, NULL);
        g_free (text);
        moo_lang_unref (lang);
    }
    else
    {
        g_object_set (cell, "text", "None", NULL);
    }
}


static void
syntax_combo_changed (GtkComboBox        *combo,
                      MooPrefsDialogPage *page)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    MooLang *lang = NULL;

    if (!gtk_combo_box_get_active_iter (combo, &iter))
        g_return_if_reached ();

    model = gtk_combo_box_get_model (combo);
    gtk_tree_model_get (model, &iter, 0, &lang, -1);

    page_set_lang (page, lang);

    if (lang)
        moo_lang_unref (lang);
}


static void
syntax_combo_set_lang (GtkComboBox        *combo,
                       MooLang            *lang)
{
    GtkTreeModel *model = gtk_combo_box_get_model (combo);
    gboolean found = FALSE;
    GtkTreeIter iter;

    gtk_tree_model_get_iter_first (model, &iter);

    do {
        MooLang *l;

        gtk_tree_model_get (model, &iter, 0, &l, -1);

        if (l)
            moo_lang_unref (l);

        if (lang == l)
        {
            found = TRUE;
            break;
        }
    }
    while (gtk_tree_model_iter_next (model, &iter));

    g_return_if_fail (found);

    gtk_combo_box_set_active_iter (combo, &iter);
}


static void
preview_set_lang (GtkWidget *preview,
                  MooLang   *lang)
{
    if (lang && lang->sample)
    {
        GtkTextBuffer *buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (preview));
        gtk_text_buffer_set_text (buf, lang->sample, -1);
    }

    moo_text_view_set_lang (MOO_TEXT_VIEW (preview), lang);
}


static void
page_set_lang (MooPrefsDialogPage *page,
               MooLang            *lang)
{
    g_object_set_data (G_OBJECT (page), "moo-lang", lang);
    preview_set_lang (page_get_preview (page), lang);
}


static MooTextStyleScheme*
page_get_scheme (MooPrefsDialogPage *page)
{
    return g_object_get_data (G_OBJECT (page), "moo-text-style-scheme");
}


static MooLang*
page_get_lang (MooPrefsDialogPage *page)
{
    return g_object_get_data (G_OBJECT (page), "moo-lang");
}


/********************************************************************/
/* Default styles list
 */

static void default_treeview_style_func     (GtkTreeViewColumn  *column,
                                             GtkCellRenderer    *cell,
                                             GtkTreeModel       *model,
                                             GtkTreeIter        *iter,
                                             MooPrefsDialogPage *page);
static void default_treeview_toggle_func    (GtkTreeViewColumn  *column,
                                             GtkCellRenderer    *cell,
                                             GtkTreeModel       *model,
                                             GtkTreeIter        *iter,
                                             MooPrefsDialogPage *page);
static void default_treeview_color_func     (GtkTreeViewColumn  *column,
                                             GtkCellRenderer    *cell,
                                             GtkTreeModel       *model,
                                             GtkTreeIter        *iter,
                                             MooPrefsDialogPage *page);


static void
default_treeview_create_toggle (GtkTreeView        *treeview,
                                MooPrefsDialogPage *page,
                                MooTextStyleMask    mask,
                                const char         *title)
{
    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;
    GtkWidget *label;

    cell = gtk_cell_renderer_toggle_new ();
    g_object_set (cell, "activatable", TRUE, NULL);
    g_object_set_data (G_OBJECT (cell), "moo-text-style-mask", GUINT_TO_POINTER (mask));

    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_pack_start (column, cell, FALSE);
    gtk_tree_view_column_set_cell_data_func (column, cell,
                                             (GtkTreeCellDataFunc) default_treeview_toggle_func,
                                             page, NULL);
    gtk_tree_view_column_set_expand (column, FALSE);
    gtk_tree_view_append_column (treeview, column);

    label = gtk_label_new (title);
    gtk_widget_show (label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_tree_view_column_set_widget (column, label);
}

static void
default_treeview_create_color (GtkTreeView        *treeview,
                               MooPrefsDialogPage *page,
                               MooTextStyleMask    mask,
                               const char         *title)
{
    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;

    cell = moo_cell_renderer_color_new ();
    g_object_set (cell, "activatable", TRUE, NULL);
    g_object_set_data (G_OBJECT (cell), "moo-text-style-mask", GUINT_TO_POINTER (mask));

    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_title (column, title);
    gtk_tree_view_column_pack_start (column, cell, TRUE);
    gtk_tree_view_column_set_cell_data_func (column, cell,
                                             (GtkTreeCellDataFunc) default_treeview_color_func,
                                             page, NULL);
    gtk_tree_view_column_set_expand (column, FALSE);
    gtk_tree_view_append_column (treeview, column);
}

static void
default_treeview_init (GtkTreeView        *treeview,
                       MooPrefsDialogPage *page)
{
    GtkCellRenderer *cell;
    GtkListStore *store;
    GtkTreeViewColumn *column;

    gtk_tree_selection_set_mode (gtk_tree_view_get_selection (treeview),
                                 GTK_SELECTION_NONE);

    store = gtk_list_store_new (1, G_TYPE_STRING);
    gtk_tree_view_set_model (treeview, GTK_TREE_MODEL (store));
    g_object_unref (store);

    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_data_func (treeview, -1, "Style", cell,
            (GtkTreeCellDataFunc) default_treeview_style_func, page, NULL);
    column = gtk_tree_view_get_column (treeview, 0);

    default_treeview_create_color (treeview, page, MOO_TEXT_STYLE_FOREGROUND, "Foreground");
    default_treeview_create_color (treeview, page, MOO_TEXT_STYLE_BACKGROUND, "Background");

    default_treeview_create_toggle (treeview, page, MOO_TEXT_STYLE_BOLD, "<b>B</b>");
    default_treeview_create_toggle (treeview, page, MOO_TEXT_STYLE_ITALIC, "<i>I</i>");
    default_treeview_create_toggle (treeview, page, MOO_TEXT_STYLE_UNDERLINE, "<u>U</u>");
    default_treeview_create_toggle (treeview, page, MOO_TEXT_STYLE_STRIKETHROUGH, "<s>S</s>");
}


static void
text_cell_set_props_from_style (GtkCellRenderer  *cell,
                                MooTextStyle     *style)
{
    g_return_if_fail (style != NULL);

    if ((style->mask & MOO_TEXT_STYLE_BOLD) && style->bold)
        g_object_set (cell, "weight", PANGO_WEIGHT_BOLD, NULL);
    else
        g_object_set (cell, "weight-set", FALSE, NULL);

    if ((style->mask & MOO_TEXT_STYLE_ITALIC) && style->italic)
        g_object_set (cell, "style", PANGO_STYLE_ITALIC, NULL);
    else
        g_object_set (cell, "style-set", FALSE, NULL);

    if ((style->mask & MOO_TEXT_STYLE_UNDERLINE) && style->underline)
        g_object_set (cell, "underline", PANGO_UNDERLINE_SINGLE, NULL);
    else
        g_object_set (cell, "underline", PANGO_UNDERLINE_NONE, NULL);

    if ((style->mask & MOO_TEXT_STYLE_STRIKETHROUGH) && style->strikethrough)
        g_object_set (cell, "strikethrough", TRUE, NULL);
    else
        g_object_set (cell, "strikethrough", FALSE, NULL);

    if (style->mask & MOO_TEXT_STYLE_FOREGROUND)
        g_object_set (cell, "foreground-gdk", &style->foreground, NULL);
    else
        g_object_set (cell, "foreground-set", FALSE, NULL);

    if (style->mask & MOO_TEXT_STYLE_BACKGROUND)
        g_object_set (cell, "background-gdk", &style->background, NULL);
    else
        g_object_set (cell, "background-set", FALSE, NULL);
}


static void
default_treeview_style_func (G_GNUC_UNUSED GtkTreeViewColumn *column,
                             GtkCellRenderer    *cell,
                             GtkTreeModel       *model,
                             GtkTreeIter        *iter,
                             G_GNUC_UNUSED MooPrefsDialogPage *page)
{
    char *style_name;
    MooTextStyle *style;

    gtk_tree_model_get (model, iter, 0, &style_name, -1);
    g_return_if_fail (style_name != NULL);

    style = moo_lang_mgr_get_style (page_get_mgr (page),
                                    NULL, style_name,
                                    page_get_scheme (page));
    g_return_if_fail (style != NULL);

    text_cell_set_props_from_style (cell, style);
    g_object_set (cell, "text", style_name, NULL);

    moo_text_style_free (style);
    g_free (style_name);
}


static void
default_treeview_toggle_func (G_GNUC_UNUSED GtkTreeViewColumn *column,
                              GtkCellRenderer    *cell,
                              GtkTreeModel       *model,
                              GtkTreeIter        *iter,
                              MooPrefsDialogPage *page)
{
    char *style_name;
    MooTextStyle *style;
    MooTextStyleMask mask;
    gboolean active = FALSE;

    gtk_tree_model_get (model, iter, 0, &style_name, -1);
    g_return_if_fail (style_name != NULL);

    style = moo_lang_mgr_get_style (page_get_mgr (page),
                                    NULL, style_name,
                                    page_get_scheme (page));
    g_return_if_fail (style != NULL);

    mask = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (cell), "moo-text-style-mask"));

    /* XXX */
    if (mask & style->mask)
    {
        switch (mask)
        {
            case MOO_TEXT_STYLE_BOLD:
                active = style->bold ? TRUE : FALSE;
                break;
            case MOO_TEXT_STYLE_ITALIC:
                active = style->italic ? TRUE : FALSE;
                break;
            case MOO_TEXT_STYLE_UNDERLINE:
                active = style->underline ? TRUE : FALSE;
                break;
            case MOO_TEXT_STYLE_STRIKETHROUGH:
                active = style->strikethrough ? TRUE : FALSE;
                break;
            default:
                g_return_if_reached ();
        }
    }

    g_object_set (cell, "active", active, NULL);
    moo_text_style_free (style);
    g_free (style_name);
}


static void
default_treeview_color_func (G_GNUC_UNUSED GtkTreeViewColumn *column,
                             GtkCellRenderer    *cell,
                             GtkTreeModel       *model,
                             GtkTreeIter        *iter,
                             MooPrefsDialogPage *page)
{
    char *style_name;
    MooTextStyle *style;
    MooTextStyleScheme *scheme;
    MooTextStyleMask mask;
    GdkScreen *screen;

    if (!gtk_widget_has_screen (GTK_WIDGET (page)))
        return;

    screen = gtk_widget_get_screen (GTK_WIDGET (page));

    gtk_tree_model_get (model, iter, 0, &style_name, -1);
    g_return_if_fail (style_name != NULL);

    scheme = page_get_scheme (page);
    style = moo_lang_mgr_get_style (page_get_mgr (page),
                                    NULL, style_name,
                                    scheme);
    g_return_if_fail (style != NULL);

    mask = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (cell), "moo-text-style-mask"));

    switch (mask)
    {
        case MOO_TEXT_STYLE_FOREGROUND:
            if (style->mask & MOO_TEXT_STYLE_FOREGROUND)
                g_object_set (cell, "color", &style->foreground, NULL);
            else
                g_object_set (cell, "color-set", FALSE, NULL);
            break;
        case MOO_TEXT_STYLE_BACKGROUND:
            if (style->mask & MOO_TEXT_STYLE_BACKGROUND)
                g_object_set (cell, "color", &style->background, NULL);
            else
                g_object_set (cell, "color-set", FALSE, NULL);
            break;
        default:
            g_return_if_reached ();
    }

    moo_text_style_free (style);
    g_free (style_name);
}


static void
default_treeview_set_scheme (GtkTreeView        *treeview,
                             MooTextStyleScheme *scheme)
{
    GtkListStore *store;
    GtkTreeModel *model;
    GSList *styles, *l;
    GtkTreeIter iter;

    treeview_apply_scheme (GTK_WIDGET (treeview), scheme);

    model = gtk_tree_view_get_model (treeview);
    store = GTK_LIST_STORE (model);
    gtk_list_store_clear (store);

    styles = moo_text_style_scheme_list_default (scheme);

    for (l = styles; l != NULL; l = l->next)
    {
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 0, l->data, -1);
    }

    g_slist_foreach (styles, (GFunc) g_free, NULL);
    g_slist_free (styles);
}


static void
treeview_apply_scheme (GtkWidget          *widget,
                       MooTextStyleScheme *scheme)
{
    GdkColor color;
    GdkColor *color_ptr;

    g_return_if_fail (scheme != NULL);

    gtk_widget_ensure_style (widget);

    color_ptr = NULL;
    if (scheme->text_colors[MOO_TEXT_COLOR_FG])
    {
        if (gdk_color_parse (scheme->text_colors[MOO_TEXT_COLOR_FG], &color))
            color_ptr = &color;
        else
            g_warning ("%s: could not parse color '%s'", G_STRLOC,
                       scheme->text_colors[MOO_TEXT_COLOR_FG]);
    }
    gtk_widget_modify_text (widget, GTK_STATE_NORMAL, color_ptr);
    gtk_widget_modify_text (widget, GTK_STATE_ACTIVE, color_ptr);
    gtk_widget_modify_text (widget, GTK_STATE_PRELIGHT, color_ptr);
    gtk_widget_modify_text (widget, GTK_STATE_INSENSITIVE, color_ptr);

    color_ptr = NULL;
    if (scheme->text_colors[MOO_TEXT_COLOR_BG])
    {
        if (gdk_color_parse (scheme->text_colors[MOO_TEXT_COLOR_BG], &color))
            color_ptr = &color;
        else
            g_warning ("%s: could not parse color '%s'", G_STRLOC,
                       scheme->text_colors[MOO_TEXT_COLOR_BG]);
    }
    gtk_widget_modify_base (widget, GTK_STATE_NORMAL, color_ptr);
    gtk_widget_modify_base (widget, GTK_STATE_ACTIVE, color_ptr);
    gtk_widget_modify_base (widget, GTK_STATE_PRELIGHT, color_ptr);
    gtk_widget_modify_base (widget, GTK_STATE_INSENSITIVE, color_ptr);

    color_ptr = NULL;
    if (scheme->text_colors[MOO_TEXT_COLOR_SEL_FG])
    {
        if (gdk_color_parse (scheme->text_colors[MOO_TEXT_COLOR_SEL_FG], &color))
            color_ptr = &color;
        else
            g_warning ("%s: could not parse color '%s'", G_STRLOC,
                       scheme->text_colors[MOO_TEXT_COLOR_SEL_FG]);
    }
    gtk_widget_modify_text (widget, GTK_STATE_SELECTED, color_ptr);

    color_ptr = NULL;
    if (scheme->text_colors[MOO_TEXT_COLOR_SEL_BG])
    {
        if (gdk_color_parse (scheme->text_colors[MOO_TEXT_COLOR_SEL_BG], &color))
            color_ptr = &color;
        else
            g_warning ("%s: could not parse color '%s'", G_STRLOC,
                       scheme->text_colors[MOO_TEXT_COLOR_SEL_BG]);
    }
    gtk_widget_modify_base (widget, GTK_STATE_SELECTED, color_ptr);
}
