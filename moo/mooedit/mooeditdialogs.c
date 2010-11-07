/*
 *   mooeditdialogs.c
 *
 *   Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@sourceforge.net>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#define MOOEDIT_COMPILATION
#include "mooedit/mooeditdialogs.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mooedit-fileops.h"
#include "mooutils/moodialogs.h"
#include "mooutils/moocompat.h"
#include "mooutils/moostock.h"
#include "mooutils/mooi18n.h"
#include "mooutils/mooencodings.h"
#include "mootextfind-prompt-gxml.h"
#include "mooeditsavemult-gxml.h"
#include <gtk/gtk.h>
#include <glib/gregex.h>
#include <string.h>


GSList *
_moo_edit_open_dialog (GtkWidget *widget,
                       MooEdit   *current_doc)
{
    MooFileDialog *dialog;
    const char *start_dir = NULL;
    const char *encoding;
    char *new_start;
    char *freeme = NULL;
    char **filenames = NULL, **p;
    GSList *infos = NULL;

    moo_prefs_create_key (moo_edit_setting (MOO_EDIT_PREFS_LAST_DIR), MOO_PREFS_STATE, G_TYPE_STRING, NULL);

    if (current_doc && moo_prefs_get_bool (moo_edit_setting (MOO_EDIT_PREFS_DIALOGS_OPEN_FOLLOWS_DOC)))
    {
        char *filename = moo_edit_get_filename (current_doc);

        if (filename)
        {
            freeme = g_path_get_dirname (filename);
            start_dir = freeme;
            g_free (filename);
        }
    }

    if (!start_dir)
        start_dir = moo_prefs_get_filename (moo_edit_setting (MOO_EDIT_PREFS_LAST_DIR));

    dialog = moo_file_dialog_new (MOO_FILE_DIALOG_OPEN, widget,
                                  TRUE, GTK_STOCK_OPEN, start_dir,
                                  NULL);
    g_object_set (dialog, "enable-encodings", TRUE, NULL);
    moo_file_dialog_set_help_id (dialog, "dialog-open");
    moo_file_dialog_set_remember_size (dialog, moo_edit_setting (MOO_EDIT_PREFS_DIALOG_OPEN));

    moo_file_dialog_set_filter_mgr_id (dialog, "MooEdit");

    if (!moo_file_dialog_run (dialog))
        goto out;

    encoding = moo_file_dialog_get_encoding (dialog);

    if (encoding && !strcmp (encoding, MOO_ENCODING_AUTO))
        encoding = NULL;

    filenames = moo_file_dialog_get_filenames (dialog);
    g_return_val_if_fail (filenames != NULL, NULL);

    for (p = filenames; *p != NULL; ++p)
        infos = g_slist_prepend (infos, moo_edit_file_info_new_path (*p, encoding));
    infos = g_slist_reverse (infos);

    new_start = g_path_get_dirname (filenames[0]);
    moo_prefs_set_filename (moo_edit_setting (MOO_EDIT_PREFS_LAST_DIR), new_start);
    g_free (new_start);

out:
    g_free (freeme);
    g_object_unref (dialog);
    g_strfreev (filenames);
    return infos;
}


MooEditFileInfo *
_moo_edit_save_as_dialog (MooEdit    *edit,
                          const char *display_basename)
{
    const char *start = NULL;
    char *filename = NULL;
    char *freeme = NULL;
    const char *encoding;
    char *new_start;
    MooFileDialog *dialog;
    MooEditFileInfo *file_info;

    moo_prefs_create_key (moo_edit_setting (MOO_EDIT_PREFS_LAST_DIR),
                          MOO_PREFS_STATE, G_TYPE_STRING, NULL);

    if (moo_prefs_get_bool (moo_edit_setting (MOO_EDIT_PREFS_DIALOGS_OPEN_FOLLOWS_DOC)))
    {
        char *this_filename = moo_edit_get_filename (edit);

        if (this_filename)
        {
            freeme = g_path_get_dirname (this_filename);
            start = freeme;
            g_free (this_filename);
        }
    }

    if (!start)
        start = moo_prefs_get_filename (moo_edit_setting (MOO_EDIT_PREFS_LAST_DIR));

    dialog = moo_file_dialog_new (MOO_FILE_DIALOG_SAVE, GTK_WIDGET (edit),
                                  FALSE, GTK_STOCK_SAVE_AS, start, display_basename);
    g_object_set (dialog, "enable-encodings", TRUE, NULL);
    moo_file_dialog_set_encoding (dialog, moo_edit_get_encoding (edit));
    moo_file_dialog_set_help_id (dialog, "dialog-save");

    moo_file_dialog_set_filter_mgr_id (dialog, "MooEdit");

    if (!moo_file_dialog_run (dialog))
    {
        g_object_unref (dialog);
        g_free (freeme);
        return NULL;
    }

    encoding = moo_file_dialog_get_encoding (dialog);
    filename = moo_file_dialog_get_filename (dialog);
    g_return_val_if_fail (filename != NULL, NULL);
    file_info = moo_edit_file_info_new_path (filename, encoding);

    new_start = g_path_get_dirname (filename);
    moo_prefs_set_filename (moo_edit_setting (MOO_EDIT_PREFS_LAST_DIR), new_start);
    g_free (new_start);

    g_free (filename);
    g_object_unref (dialog);
    g_free (freeme);
    return file_info;
}


MooSaveChangesDialogResponse
_moo_edit_save_changes_dialog (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), MOO_SAVE_CHANGES_RESPONSE_CANCEL);
    return moo_save_changes_dialog (moo_edit_get_display_basename (edit),
                                    GTK_WIDGET (edit));
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
files_treeview_get_to_save (GtkTreeView *treeview)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    GSList *list = NULL;

    model = gtk_tree_view_get_model (treeview);
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


static GtkWidget *
find_widget_for_response (GtkDialog *dialog,
                          int        response)
{
    GList *l, *children;
    GtkWidget *ret = NULL;

    children = gtk_container_get_children (GTK_CONTAINER (dialog->action_area));

    for (l = children; ret == NULL && l != NULL; l = l->next)
    {
        GtkWidget *widget = l->data;
        int response_here = gtk_dialog_get_response_for_widget (dialog, widget);
        if (response_here == response)
            ret = widget;
    }

    g_list_free (children);
    return ret;
}

MooSaveChangesDialogResponse
_moo_edit_save_multiple_changes_dialog (GSList  *docs,
                                        GSList **to_save)
{
    GSList *l;
    GtkWidget *dialog;
    char *msg, *question;
    int response;
    MooSaveChangesDialogResponse retval;
    SaveMultDialogXml *xml;

    g_return_val_if_fail (docs != NULL, MOO_SAVE_CHANGES_RESPONSE_CANCEL);
    g_return_val_if_fail (docs->next != NULL, MOO_SAVE_CHANGES_RESPONSE_CANCEL);
    g_return_val_if_fail (to_save != NULL, MOO_SAVE_CHANGES_RESPONSE_CANCEL);

    for (l = docs; l != NULL; l = l->next)
        g_return_val_if_fail (MOO_IS_EDIT (l->data), MOO_SAVE_CHANGES_RESPONSE_CANCEL);

    xml = save_mult_dialog_xml_new ();
    dialog = GTK_WIDGET (xml->SaveMultDialog);

    moo_window_set_parent (dialog, docs->data);

    gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                            MOO_STOCK_SAVE_NONE, GTK_RESPONSE_NO,
                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                            MOO_STOCK_SAVE_SELECTED, GTK_RESPONSE_YES,
                            NULL);

    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);

    gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                             GTK_RESPONSE_YES,
                                             GTK_RESPONSE_NO,
                                             GTK_RESPONSE_CANCEL, -1);

    question = g_strdup_printf (dngettext (GETTEXT_PACKAGE,
                                           /* Translators: number of documents here is always greater than one, so
                                              ignore singular form (which is simply copy of the plural here) */
                                           "There are %u documents with unsaved changes. "
                                            "Save changes before closing?",
                                           "There are %u documents with unsaved changes. "
                                            "Save changes before closing?",
                                           g_slist_length (docs)),
                                g_slist_length (docs));
    msg = g_markup_printf_escaped ("<span weight=\"bold\" size=\"larger\">%s</span>",
                                   question);
    gtk_label_set_markup (xml->label, msg);

    files_treeview_init (xml->treeview, dialog, docs);

    {
        GtkWidget *button;
        button = find_widget_for_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);
        gtk_widget_grab_focus (button);
    }

    response = gtk_dialog_run (GTK_DIALOG (dialog));

    switch (response)
    {
        case GTK_RESPONSE_NO:
            retval = MOO_SAVE_CHANGES_RESPONSE_DONT_SAVE;
            break;
        case GTK_RESPONSE_YES:
            *to_save = files_treeview_get_to_save (xml->treeview);
            retval = MOO_SAVE_CHANGES_RESPONSE_SAVE;
            break;
        default:
            retval = MOO_SAVE_CHANGES_RESPONSE_CANCEL;
    }

    g_free (question);
    g_free (msg);
    gtk_widget_destroy (dialog);
    return retval;
}


/*****************************************************************************/
/* Error dialogs
 */

void
_moo_edit_save_error_dialog (GtkWidget *widget,
                             GFile     *file,
                             GError    *error)
{
    char *filename, *msg = NULL;

    filename = _moo_file_get_display_name (file);

    if (filename)
        /* Could not save file foo.txt */
        msg = g_strdup_printf (_("Could not save file\n%s"), filename);
    else
        msg = g_strdup (_("Could not save file"));

    moo_error_dialog (widget, msg, error ? error->message : NULL);

    g_free (msg);
    g_free (filename);
}

void
_moo_edit_save_error_enc_dialog (GtkWidget  *widget,
                                 GFile      *file,
                                 const char *encoding)
{
    char *filename, *msg = NULL;
    char *secondary;

    g_return_if_fail (encoding != NULL);

    filename = _moo_file_get_display_name (file);

    if (filename)
        /* Error saving file foo.txt */
        msg = g_strdup_printf (_("Error saving file\n%s"), filename);
    else
        msg = g_strdup (_("Error saving file"));

    secondary = g_strdup_printf (_("Could not convert file to encoding %s. "
                                   "File was saved in UTF-8 encoding."),
                                 encoding);

    moo_error_dialog (widget, msg, secondary);

    g_free (msg);
    g_free (secondary);
    g_free (filename);
}


void
_moo_edit_open_error_dialog (GtkWidget  *widget,
                             GFile      *file,
                             const char *encoding,
                             GError     *error)
{
    char *filename, *msg = NULL;
    char *secondary;

    filename = _moo_file_get_display_name (file);

    if (filename)
        /* Could not open file foo.txt */
        msg = g_strdup_printf (_("Could not open file\n%s"), filename);
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
    g_free (filename);
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

    dialog = gtk_message_dialog_new (parent, GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_WARNING,
                                     GTK_BUTTONS_NONE,
                                     "%s", text);
    gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                              "%s", secondary);

    gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                            button, GTK_RESPONSE_YES,
                            NULL);

    gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                             GTK_RESPONSE_YES,
                                             GTK_RESPONSE_CANCEL,
                                             -1);

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
                                     "%s", msg);
    moo_window_set_parent (dialog, widget);

    gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                            GTK_STOCK_NO, GTK_RESPONSE_CANCEL,
                            GTK_STOCK_YES, GTK_RESPONSE_YES,
                            NULL);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);
    gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                             GTK_RESPONSE_YES,
                                             GTK_RESPONSE_CANCEL,
                                             -1);

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
        if (error->domain != G_REGEX_ERROR)
        {
            g_warning ("%s: unknown error domain", G_STRLOC);
        }
        else if (error->code != G_REGEX_ERROR_COMPILE &&
                 error->code != G_REGEX_ERROR_OPTIMIZE &&
                 error->code != G_REGEX_ERROR_REPLACE &&
                 error->code != G_REGEX_ERROR_MATCH)
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
                                     "%s", msg_text);
    moo_window_set_parent (dialog, parent);
    gtk_dialog_add_buttons (GTK_DIALOG (dialog), GTK_STOCK_CLOSE,
                            GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);

    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    g_free (msg_text);
}


GtkWidget *
_moo_text_prompt_on_replace_dialog (GtkWidget *parent)
{
    FindPromptDialogXml *xml;
    xml = find_prompt_dialog_xml_new ();
    moo_window_set_parent (GTK_WIDGET (xml->FindPromptDialog), parent);
    return GTK_WIDGET (xml->FindPromptDialog);
}
