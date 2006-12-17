/*
 *   mooeditdialogs.c
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
#include "mooedit/mooeditdialogs.h"
#include "mooedit/mooedit-private.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mootextfind-glade.h"
#include "mooedit/mooeditfileops.h"
#include "mooedit/mooeditsavemultiple-glade.h"
#include "mooutils/moodialogs.h"
#include "mooutils/moostock.h"
#include "mooutils/mooglade.h"
#include "mooutils/eggregex.h"
#include "mooutils/mooi18n.h"
#include "mooutils/mooencodings.h"
#include <gtk/gtk.h>
#include <string.h>


static void
open_dialog_created (G_GNUC_UNUSED MooFileDialog *fd,
                     GtkWidget *widget)
{
    _moo_window_set_remember_size (GTK_WINDOW (widget),
                                   moo_edit_setting (MOO_EDIT_PREFS_DIALOG_OPEN),
                                   TRUE);
}

GSList *
_moo_edit_open_dialog (GtkWidget      *widget,
                       MooFilterMgr   *mgr)
{
    MooFileDialog *dialog;
    const char *start, *encoding;
    char *new_start;
    GSList *filenames, *infos = NULL, *l;

    moo_prefs_create_key (moo_edit_setting (MOO_EDIT_PREFS_LAST_DIR), MOO_PREFS_STATE, G_TYPE_STRING, NULL);
    start = moo_prefs_get_filename (moo_edit_setting (MOO_EDIT_PREFS_LAST_DIR));

    dialog = moo_file_dialog_new (MOO_FILE_DIALOG_OPEN, widget,
                                  TRUE, "Open", start, NULL);
    g_object_set (dialog, "enable-encodings", TRUE, NULL);
    g_signal_connect (dialog, "dialog-created", G_CALLBACK (open_dialog_created), NULL);

    if (mgr)
        moo_file_dialog_set_filter_mgr (dialog, mgr, "MooEdit");

    if (!moo_file_dialog_run (dialog))
    {
        g_object_unref (dialog);
        return NULL;
    }

    encoding = moo_file_dialog_get_encoding (dialog);

    if (encoding && !strcmp (encoding, MOO_ENCODING_AUTO))
        encoding = NULL;

    filenames = moo_file_dialog_get_filenames (dialog);
    g_return_val_if_fail (filenames != NULL, NULL);

    for (l = filenames; l != NULL; l = l->next)
        infos = g_slist_prepend (infos, moo_edit_file_info_new (l->data, encoding));
    infos = g_slist_reverse (infos);

    new_start = g_path_get_dirname (filenames->data);
    moo_prefs_set_filename (moo_edit_setting (MOO_EDIT_PREFS_LAST_DIR), new_start);
    g_free (new_start);

    g_object_unref (dialog);
    g_slist_foreach (filenames, (GFunc) g_free, NULL);
    return infos;
}


MooEditFileInfo *
_moo_edit_save_as_dialog (MooEdit        *edit,
                          MooFilterMgr   *mgr,
                          const char     *display_basename)
{
    /* Save dialog title */
    const char *title = _("Save As");
    const char *start = NULL;
    const char *filename = NULL;
    const char *encoding;
    char *new_start;
    MooFileDialog *dialog;
    MooEditFileInfo *file_info;

    moo_prefs_create_key (moo_edit_setting (MOO_EDIT_PREFS_LAST_DIR),
                          MOO_PREFS_STATE, G_TYPE_STRING, NULL);
    start = moo_prefs_get_filename (moo_edit_setting (MOO_EDIT_PREFS_LAST_DIR));

    dialog = moo_file_dialog_new (MOO_FILE_DIALOG_SAVE, GTK_WIDGET (edit),
                                  FALSE, title, start, display_basename);
    g_object_set (dialog, "enable-encodings", TRUE, NULL);
    moo_file_dialog_set_encoding (dialog, moo_edit_get_encoding (edit));

    if (mgr)
        moo_file_dialog_set_filter_mgr (dialog, mgr, "MooEdit");

    if (!moo_file_dialog_run (dialog))
    {
        g_object_unref (dialog);
        return NULL;
    }

    encoding = moo_file_dialog_get_encoding (dialog);
    filename = moo_file_dialog_get_filename (dialog);
    g_return_val_if_fail (filename != NULL, NULL);
    file_info = moo_edit_file_info_new (filename, encoding);

    new_start = g_path_get_dirname (filename);
    moo_prefs_set_filename (moo_edit_setting (MOO_EDIT_PREFS_LAST_DIR), new_start);
    g_free (new_start);

    g_object_unref (dialog);
    return file_info;
}


MooEditDialogResponse
_moo_edit_save_changes_dialog (MooEdit *edit)
{
    GtkDialog *dialog = NULL;
    int response;
    const char *name = moo_edit_get_display_basename (edit);

    g_return_val_if_fail (MOO_IS_EDIT (edit), MOO_EDIT_RESPONSE_CANCEL);
    name = moo_edit_get_display_basename (edit);

#if GTK_CHECK_VERSION(2,6,0)
    dialog = GTK_DIALOG (gtk_message_dialog_new (
        GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (edit))),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_NONE,
        name ?
            _("Save changes to document \"%s\" before closing?") :
            _("Save changes to document before closing?"),
        name));

    gtk_message_dialog_format_secondary_text (
        GTK_MESSAGE_DIALOG (dialog),
        _("If you don't save, changes will be discarded"));
#elif GTK_CHECK_VERSION(2,4,0)
    {
        char *question, *markup;

        question = name ?
            g_strdup_printf (_("Save changes to document \"%s\" before closing?"), name) :
            g_strdup (_("Save changes to document before closing?"));
        markup = g_markup_printf_escaped ("<span weight=\"bold\" size=\"larger\">%s</span>\n%s",
                                          question,
                                          _("If you don't save, changes will be discarded"));

        dialog = GTK_DIALOG (gtk_message_dialog_new_with_markup (
            GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (edit))),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_NONE,
            "%s", markup);

        g_free (markup);
        g_free (question);
    }
#else /* !GTK_CHECK_VERSION(2,4,0) */
    {
        char *markup;

        question = name ?
            g_strdup_printf (_("Save changes to document \"%s\" before closing?"), name) :
            g_strdup (_("Save changes to document before closing?"));

        dialog = GTK_DIALOG (gtk_message_dialog_new (
            GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (edit))),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_NONE,
            "%s\n%s", question,
            _("If you don't save, changes will be discarded"));

        g_free (question);
    }
#endif /* !GTK_CHECK_VERSION(2,4,0) */

    gtk_dialog_add_buttons (dialog,
        _("Close _without Saving"), GTK_RESPONSE_NO,
        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
        GTK_STOCK_SAVE, GTK_RESPONSE_YES,
        NULL);

#if GTK_CHECK_VERSION(2,6,0)
    gtk_dialog_set_alternative_button_order (dialog,
        GTK_RESPONSE_YES, GTK_RESPONSE_NO, GTK_RESPONSE_CANCEL, -1);
#endif /* GTK_CHECK_VERSION(2,6,0) */

    gtk_dialog_set_default_response (dialog, GTK_RESPONSE_YES);

    moo_window_set_parent (GTK_WIDGET (dialog), GTK_WIDGET (edit));
    response = gtk_dialog_run (dialog);
    if (response == GTK_RESPONSE_DELETE_EVENT)
        response = GTK_RESPONSE_CANCEL;
    gtk_widget_destroy (GTK_WIDGET (dialog));

    switch (response)
    {
        case GTK_RESPONSE_NO:
            return MOO_EDIT_RESPONSE_DONT_SAVE;
        case GTK_RESPONSE_CANCEL:
            return MOO_EDIT_RESPONSE_CANCEL;
        case GTK_RESPONSE_YES:
            return MOO_EDIT_RESPONSE_SAVE;
    }

    g_return_val_if_reached (MOO_EDIT_RESPONSE_CANCEL);
}


/****************************************************************************/
/* Save multiple
 */

enum {
    COLUMN_SAVE = 0,
    COLUMN_EDIT,
    NUM_COLUMNS
};

static void
name_data_func (G_GNUC_UNUSED GtkTreeViewColumn *column,
                GtkCellRenderer   *cell,
                GtkTreeModel      *model,
                GtkTreeIter       *iter)
{
    MooEdit *edit = NULL;

    gtk_tree_model_get (model, iter, COLUMN_EDIT, &edit, -1);
    g_return_if_fail (MOO_IS_EDIT (edit));

    g_object_set (cell, "text", moo_edit_get_display_basename (edit), NULL);
    g_object_unref (edit);
}


static void
save_toggled (GtkCellRendererToggle *cell,
              gchar                 *path,
              GtkTreeModel          *model)
{
    GtkTreePath *tree_path;
    GtkTreeIter iter;
    gboolean save = TRUE;
    gboolean active;
    gboolean sensitive;
    GtkDialog *dialog;

    g_return_if_fail (GTK_IS_LIST_STORE (model));

    tree_path = gtk_tree_path_new_from_string (path);
    g_return_if_fail (tree_path != NULL);

    gtk_tree_model_get_iter (model, &iter, tree_path);
    gtk_tree_model_get (model, &iter, COLUMN_SAVE, &save, -1);

    active = gtk_cell_renderer_toggle_get_active (cell);

    if (active == save)
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_SAVE, !save, -1);

    gtk_tree_path_free (tree_path);

    dialog = g_object_get_data (G_OBJECT (model), "moo-dialog");
    g_return_if_fail (dialog != NULL);

    if (!save)
    {
        sensitive = TRUE;
    }
    else
    {
        sensitive = FALSE;
        gtk_tree_model_get_iter_first (model, &iter);

        do
        {
            gtk_tree_model_get (model, &iter, COLUMN_SAVE, &save, -1);
            if (save)
            {
                sensitive = TRUE;
                break;
            }
        }
        while (gtk_tree_model_iter_next (model, &iter));
    }

    gtk_dialog_set_response_sensitive (dialog, GTK_RESPONSE_YES, sensitive);
}

static void
files_treeview_init (GtkTreeView *treeview, GtkWidget *dialog, GSList  *docs)
{
    GtkListStore *store;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GSList *l;

    store = gtk_list_store_new (NUM_COLUMNS, G_TYPE_BOOLEAN, MOO_TYPE_EDIT);

    for (l = docs; l != NULL; l = l->next)
    {
        GtkTreeIter iter;
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter,
                            COLUMN_SAVE, TRUE,
                            COLUMN_EDIT, l->data, -1);
    }

    gtk_tree_view_set_model (treeview, GTK_TREE_MODEL (store));

    column = gtk_tree_view_column_new ();
    gtk_tree_view_append_column (treeview, column);
    cell = gtk_cell_renderer_toggle_new ();
    gtk_tree_view_column_pack_start (column, cell, FALSE);
    g_object_set (cell, "activatable", TRUE, NULL);
    gtk_tree_view_column_add_attribute (column, cell, "active", COLUMN_SAVE);
    g_signal_connect (cell, "toggled", G_CALLBACK (save_toggled), store);

    column = gtk_tree_view_column_new ();
    gtk_tree_view_append_column (treeview, column);
    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, cell, TRUE);
    gtk_tree_view_column_set_cell_data_func (column, cell,
                                             (GtkTreeCellDataFunc) name_data_func,
                                             NULL, NULL);

    g_object_set_data (G_OBJECT (store), "moo-dialog", dialog);

    g_object_unref (store);
}


static GSList *
files_treeview_get_to_save (GtkWidget *treeview)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    GSList *list = NULL;

    g_return_val_if_fail (GTK_IS_TREE_VIEW (treeview), NULL);

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
    g_return_val_if_fail (model != NULL, NULL);

    gtk_tree_model_get_iter_first (model, &iter);

    do
    {
        MooEdit *edit = NULL;
        gboolean save = TRUE;

        gtk_tree_model_get (model, &iter,
                            COLUMN_SAVE, &save,
                            COLUMN_EDIT, &edit, -1);
        g_return_val_if_fail (MOO_IS_EDIT (edit), list);

        if (save)
            list = g_slist_prepend (list, edit);

        g_object_unref (edit);
    }
    while (gtk_tree_model_iter_next (model, &iter));

    return g_slist_reverse (list);
}


MooEditDialogResponse
_moo_edit_save_multiple_changes_dialog (GSList  *docs,
                                        GSList **to_save)
{
    GSList *l;
    GtkWidget *dialog, *label, *treeview;
    char *msg, *question;
    int response;
    MooEditDialogResponse retval;
    MooGladeXML *xml;

    g_return_val_if_fail (docs != NULL, MOO_EDIT_RESPONSE_CANCEL);
    g_return_val_if_fail (docs->next != NULL, MOO_EDIT_RESPONSE_CANCEL);
    g_return_val_if_fail (to_save != NULL, MOO_EDIT_RESPONSE_CANCEL);

    for (l = docs; l != NULL; l = l->next)
        g_return_val_if_fail (MOO_IS_EDIT (l->data), MOO_EDIT_RESPONSE_CANCEL);

    xml = moo_glade_xml_new_from_buf (MOO_EDIT_SAVE_MULTIPLE_GLADE_UI, -1,
                                      "dialog", GETTEXT_PACKAGE, NULL);
    dialog = moo_glade_xml_get_widget (xml, "dialog");

    moo_window_set_parent (dialog, docs->data);

    gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                            MOO_STOCK_SAVE_NONE, GTK_RESPONSE_NO,
                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                            MOO_STOCK_SAVE_SELECTED, GTK_RESPONSE_YES,
                            NULL);

    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);

#if GTK_CHECK_VERSION(2,6,0)
    gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                             GTK_RESPONSE_YES,
                                             GTK_RESPONSE_NO,
                                             GTK_RESPONSE_CANCEL, -1);
#endif /* GTK_CHECK_VERSION(2,6,0) */

    label = moo_glade_xml_get_widget (xml, "label");
    /* %u is number of documents greater than 1 */
    question = g_strdup_printf (_("There are %u documents with unsaved changes. "
                                  "Save changes before closing?"), g_slist_length (docs));
    msg = g_markup_printf_escaped ("<span weight=\"bold\" size=\"larger\">%s</span>",
                                   question);
    gtk_label_set_markup (GTK_LABEL (label), msg);

    treeview = moo_glade_xml_get_widget (xml, "treeview");
    files_treeview_init (GTK_TREE_VIEW (treeview), dialog, docs);

    response = gtk_dialog_run (GTK_DIALOG (dialog));

    switch (response)
    {
        case GTK_RESPONSE_NO:
            retval = MOO_EDIT_RESPONSE_DONT_SAVE;
            break;
        case GTK_RESPONSE_YES:
            *to_save = files_treeview_get_to_save (treeview);
            retval = MOO_EDIT_RESPONSE_SAVE;
            break;
        default:
            retval = MOO_EDIT_RESPONSE_CANCEL;
    }

    g_free (question);
    g_free (msg);
    gtk_widget_destroy (dialog);
    g_object_unref (xml);
    return retval;
}


/*****************************************************************************/
/* Error dialogs
 */

void
_moo_edit_save_error_dialog (GtkWidget      *widget,
                             const char     *filename,
                             GError         *error)
{
    char *filename_utf8, *msg = NULL;

    g_return_if_fail (filename != NULL);

    filename_utf8 = g_filename_to_utf8 (filename, -1, NULL, NULL, NULL);

    if (!filename_utf8)
        g_critical ("%s: could not convert filename '%s' to utf8", G_STRLOC, filename);

    if (filename_utf8)
        /* Could not save file foo.txt */
        msg = g_strdup_printf (_("Could not save file\n%s"), filename_utf8);
    else
        msg = g_strdup (_("Could not save file"));

    moo_error_dialog (widget, msg, error ? error->message : NULL);

    g_free (msg);
    g_free (filename_utf8);
}

void
_moo_edit_save_error_enc_dialog (GtkWidget  *widget,
                                 const char *filename,
                                 const char *encoding)
{
    char *filename_utf8, *msg = NULL;
    char *secondary;

    g_return_if_fail (filename != NULL);
    g_return_if_fail (encoding != NULL);

    filename_utf8 = g_filename_to_utf8 (filename, -1, NULL, NULL, NULL);

    if (!filename_utf8)
        g_critical ("%s: could not convert filename '%s' to utf8", G_STRLOC, filename);

    if (filename_utf8)
        /* Could not save file foo.txt */
        msg = g_strdup_printf (_("Error saving file\n%s"), filename_utf8);
    else
        msg = g_strdup (_("Error saving file"));

    secondary = g_strdup_printf (_("Could not convert file to requested character "
                                   "encoding %s. File was saved in UTF-8 encoding."),
                                 encoding);

    moo_error_dialog (widget, msg, secondary);

    g_free (msg);
    g_free (secondary);
    g_free (filename_utf8);
}


void
_moo_edit_open_error_dialog (GtkWidget      *widget,
                             const char     *filename,
                             const char     *encoding,
                             GError         *error)
{
    char *filename_utf8, *msg = NULL;
    char *secondary;

    g_return_if_fail (filename != NULL);

    filename_utf8 = g_filename_to_utf8 (filename, -1, NULL, NULL, NULL);

    if (!filename_utf8)
        g_critical ("%s: could not convert filename '%s' to utf8", G_STRLOC, filename);

    if (filename_utf8)
        /* Could not open file foo.txt */
        msg = g_strdup_printf (_("Could not open file\n%s"), filename_utf8);
    else
        msg = g_strdup (_("Could not open file"));

    if (error && error->domain == MOO_EDIT_FILE_ERROR &&
        error->code == MOO_EDIT_FILE_ERROR_ENCODING)
    {
        if (encoding)
            secondary = g_strdup_printf (_("Could not open file using character encoding %s. "
                                           "The file may be binary or encoding may be specified "
                                           "incorrectly."), encoding);
        else
            secondary = g_strdup_printf (_("Could not detect file character encoding. "
                                           "Please make sure the file is not binary and try to select "
                                           "encoding in the Open dialog."));
    }
    else
    {
        secondary = error ? g_strdup (error->message) : NULL;
    }

    moo_error_dialog (widget, msg, secondary);

    g_free (msg);
    g_free (secondary);
    g_free (filename_utf8);
}


void
_moo_edit_reload_error_dialog (MooEdit *doc,
                               GError  *error)
{
    const char *filename;
    char *msg = NULL;

    g_return_if_fail (MOO_IS_EDIT (doc));

    filename = moo_edit_get_display_basename (doc);

    if (!filename)
    {
        g_critical ("%s: oops", G_STRLOC);
        filename = "";
    }

    /* Could not reload file foo.txt */
    msg = g_strdup_printf (_("Could not reload file\n%s"), filename);
    /* XXX */
    moo_error_dialog (GTK_WIDGET (doc), msg, error ? error->message : NULL);

    g_free (msg);
}


/*****************************************************************************/
/* Confirmation and alerts
 */

static gboolean
moo_edit_question_dialog (MooEdit    *edit,
                          const char *text,
                          const char *secondary,
                          const char *button)
{
    int res;
    GtkWindow *parent = GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (edit)));
    GtkWidget *dialog;

#if !GTK_CHECK_VERSION(2,6,0)
#warning "Implement me: moo_edit_question_dialog"
    dialog = gtk_message_dialog_new (parent, GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_WARNING,
                                     GTK_BUTTONS_NONE,
                                     text);
#else
    dialog = gtk_message_dialog_new (parent, GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_WARNING,
                                     GTK_BUTTONS_NONE,
                                     text);
    gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                              "%s", secondary);
#endif

    gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                            button, GTK_RESPONSE_YES,
                            NULL);

#if GTK_CHECK_VERSION(2,6,0)
    gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                             GTK_RESPONSE_YES,
                                             GTK_RESPONSE_CANCEL,
                                             -1);
#endif /* GTK_CHECK_VERSION(2,6,0) */

    res = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    return res == GTK_RESPONSE_YES;
}

gboolean
_moo_edit_reload_modified_dialog (MooEdit    *edit)
{
    const char *name;
    char *question;
    gboolean result;

    name = moo_edit_get_display_basename (edit);

    if (!name)
    {
        g_critical ("%s: oops", G_STRLOC);
        name = "";
    }

    question = g_strdup_printf (_("Discard changes in file '%s'?"), name);
    result = moo_edit_question_dialog (edit, question,
                                       _("If you reload the document, changes will be discarded"),
                                       _("_Reload"));

    g_free (question);
    return result;
}

gboolean
_moo_edit_overwrite_modified_dialog (MooEdit    *edit)
{
    const char *name;
    char *question, *secondary;
    gboolean result;

    name = moo_edit_get_display_basename (edit);

    if (!name)
    {
        g_critical ("%s: oops", G_STRLOC);
        name = "";
    }

    question = g_strdup_printf (_("Overwrite modified file '%s'?"), name);
    secondary = g_strdup_printf (_("File '%s' was modified on disk by another process. If you save it, "
                                   "changes on disk will be lost."), name);
    result = moo_edit_question_dialog (edit, question, secondary, _("Over_write"));

    g_free (question);
    g_free (secondary);
    return result;
}


/***************************************************************************/
/* Search dialogs
 */

gboolean
_moo_text_search_from_start_dialog (GtkWidget *widget,
                                    gboolean   backwards)
{
    GtkWidget *dialog;
    int response;
    const char *msg;

    if (backwards)
        msg = _("Beginning of document reached.\n"
                "Continue from the end?");
    else
        msg = _("End of document reached.\n"
                "Continue from the beginning?");

    dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
                                     msg);
    moo_window_set_parent (dialog, widget);

    gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                            GTK_STOCK_NO, GTK_RESPONSE_CANCEL,
                            GTK_STOCK_YES, GTK_RESPONSE_YES,
                            NULL);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);
#if GTK_CHECK_VERSION(2,6,0)
    gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                             GTK_RESPONSE_YES,
                                             GTK_RESPONSE_CANCEL,
                                             -1);
#endif /* GTK_CHECK_VERSION(2,6,0) */

    response = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    return response == GTK_RESPONSE_YES;
}


void
_moo_text_regex_error_dialog (GtkWidget  *parent,
                              GError     *error)
{
    GtkWidget *dialog;
    char *msg_text = NULL;

    if (error)
    {
        if (error->domain != EGG_REGEX_ERROR)
        {
            g_warning ("%s: unknown error domain", G_STRLOC);
        }
        else if (error->code != EGG_REGEX_ERROR_COMPILE &&
                 error->code != EGG_REGEX_ERROR_OPTIMIZE &&
                 error->code != EGG_REGEX_ERROR_REPLACE)
        {
            g_warning ("%s: unknown error code", G_STRLOC);
        }

        msg_text = g_strdup (error->message);
    }
    else
    {
        msg_text = g_strdup_printf ("Invalid regular expression");
    }

    dialog = gtk_message_dialog_new (NULL,
                                     GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_ERROR, GTK_BUTTONS_NONE,
                                     msg_text);
    moo_window_set_parent (dialog, parent);
    gtk_dialog_add_buttons (GTK_DIALOG (dialog), GTK_STOCK_CLOSE,
                            GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);

    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    g_free (msg_text);
}


GtkWidget*
_moo_text_prompt_on_replace_dialog (GtkWidget *parent)
{
    GtkWidget *dialog;
    MooGladeXML *xml;

    xml = moo_glade_xml_new_from_buf (MOO_TEXT_FIND_GLADE_UI, -1,
                                      "prompt_on_replace_dialog",
                                      GETTEXT_PACKAGE, NULL);
    dialog = moo_glade_xml_get_widget (xml, "prompt_on_replace_dialog");
    g_object_unref (xml);

    moo_window_set_parent (dialog, parent);

    return dialog;
}
