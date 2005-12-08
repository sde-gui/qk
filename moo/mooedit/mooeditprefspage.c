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
#include "mooutils/mooprefsdialog.h"
#include "mooutils/moocompat.h"
#include "mooutils/moostock.h"
#include "mooutils/mooglade.h"
#include "mooutils/moocellrenderercolor.h"
#include <string.h>


static void     prefs_page_init             (MooPrefsDialogPage *page);
static void     prefs_page_apply            (MooPrefsDialogPage *page);

static void     scheme_combo_init           (GtkWidget          *combo,
                                             MooEditor          *editor);
static void     scheme_combo_data_func      (GtkCellLayout      *layout,
                                             GtkCellRenderer    *cell,
                                             GtkTreeModel       *model,
                                             GtkTreeIter        *iter);
static void     scheme_combo_set_scheme     (GtkComboBox        *combo,
                                             MooTextStyleScheme *scheme);

static MooEditor *page_get_editor           (MooPrefsDialogPage *page);
static MooTextStyleScheme *page_get_scheme  (MooPrefsDialogPage *page);


GtkWidget *
moo_edit_prefs_page_new (MooEditor *editor)
{
    MooPrefsDialogPage *page;
    GtkWidget *page_widget, *scheme_combo;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    _moo_edit_init_settings ();

    page_widget = moo_prefs_dialog_page_new_from_xml ("Editor", GTK_STOCK_EDIT,
                                                      MOO_EDIT_PREFS_GLADE_UI, -1, "page",
                                                      MOO_EDIT_PREFS_PREFIX);

    page = MOO_PREFS_DIALOG_PAGE (page_widget);

    g_object_set_data_full (G_OBJECT (page), "moo-editor",
                            g_object_ref (editor), g_object_unref);
    g_object_set_data_full (G_OBJECT (page), "moo-lang-mgr",
                            g_object_ref (moo_editor_get_lang_mgr (editor)),
                            g_object_unref);

    g_signal_connect (page, "init", G_CALLBACK (prefs_page_init), NULL);
    g_signal_connect (page, "apply", G_CALLBACK (prefs_page_apply), NULL);

    scheme_combo = moo_glade_xml_get_widget (page->xml, "color_scheme_combo");
    scheme_combo_init (scheme_combo, editor);

    return page_widget;
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


static void
prefs_page_init (MooPrefsDialogPage *page)
{
    MooEditor *editor;
    MooLangMgr *mgr;
    MooTextStyleScheme *scheme;
    GtkComboBox *scheme_combo;

    editor = page_get_editor (page);
    mgr = moo_editor_get_lang_mgr (editor);
    scheme = moo_lang_mgr_get_active_scheme (mgr);
    g_return_if_fail (scheme != NULL);

    scheme_combo = moo_glade_xml_get_widget (page->xml, "color_scheme_combo");
    scheme_combo_set_scheme (scheme_combo, scheme);
}


static MooEditor*
page_get_editor (MooPrefsDialogPage *page)
{
    return g_object_get_data (G_OBJECT (page), "moo-editor");
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

    scheme = page_get_scheme (page);
    g_return_if_fail (scheme != NULL);

    moo_prefs_set_string (moo_edit_setting (MOO_EDIT_PREFS_COLOR_SCHEME), scheme->name);
    g_object_unref (scheme);
}
