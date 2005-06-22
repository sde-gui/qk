/*
 *   mooedit/mooeditdialogs.c
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

#include "mooedit/mooeditdialogs.h"
#include "mooedit/mooedit-private.h"
#include "mooedit/mooeditprefs.h"
#include "mooutils/moodialogs.h"


GtkWidget *_moo_edit_create_prompt_on_replace_dialog (void); /* in mooeditfind-glade.c */


int              moo_edit_save_changes_dialog   (MooEdit *edit)
{
    GtkDialog *dialog;
    int response;
    const char *name = edit->priv->display_basename;

#if GTK_CHECK_VERSION(2,6,0)
    dialog = GTK_DIALOG (gtk_message_dialog_new (
        GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (edit))),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_NONE,
        name ?
            "Save changes to document \"%s\" before closing?" :
            "Save changes to document before closing?",
        name));

    gtk_message_dialog_format_secondary_text (
        GTK_MESSAGE_DIALOG (dialog),
        "If you don't save, changes will be discarded");
#elif GTK_CHECK_VERSION(2,4,0)
    dialog = NULL;
    if (name)
        dialog = GTK_DIALOG (gtk_message_dialog_new_with_markup (
            GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (edit))),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_NONE,
            "<span weight=\"bold\" size=\"larger\">Save changes to "
            "document \"%s\" before closing?</span>\n"
            "If you don't save, changes will be discarded",
            name));
    else
        dialog = GTK_DIALOG (gtk_message_dialog_new_with_markup (
            GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (edit))),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_NONE,
            "<span weight=\"bold\" size=\"larger\">Save changes to "
            "document before closing?</span>\n"
            "If you don't save, changes will be discarded"));
#else /* !GTK_CHECK_VERSION(2,4,0) */
    dialog = NULL;
    if (name)
        dialog = GTK_DIALOG (gtk_message_dialog_new (
            GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (edit))),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_NONE,
            "Save changes to document \"%s\" before closing?\n"
            "If you don't save, changes will be discarded",
            name));
    else
        dialog = GTK_DIALOG (gtk_message_dialog_new (
            GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (edit))),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_NONE,
            "Save changes to document before closing?\n"
            "If you don't save, changes will be discarded"));
#endif /* !GTK_CHECK_VERSION(2,4,0) */

    gtk_dialog_add_buttons (dialog,
        "Close _without Saving", GTK_RESPONSE_NO,
        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
        GTK_STOCK_SAVE, GTK_RESPONSE_YES,
        NULL);
#if GTK_CHECK_VERSION(2,6,0)
    gtk_dialog_set_alternative_button_order (dialog,
        GTK_RESPONSE_YES, GTK_RESPONSE_NO, GTK_RESPONSE_CANCEL, -1);
#endif /* GTK_CHECK_VERSION(2,6,0) */
    gtk_dialog_set_default_response (dialog, GTK_RESPONSE_YES);

    response = gtk_dialog_run (dialog);
    if (response == GTK_RESPONSE_DELETE_EVENT)
        response = GTK_RESPONSE_CANCEL;
    gtk_widget_destroy (GTK_WIDGET (dialog));
    return response;
}


void             moo_edit_save_error_dialog     (GtkWidget  *widget,
                                                 const char *err_msg)
{
    moo_error_dialog (widget,
                      "Could not save file",
                      err_msg);
}

void             moo_edit_open_error_dialog     (GtkWidget *widget,
                                                 const char *err_msg)
{
    moo_error_dialog (widget,
                      "Could not open file",
                      err_msg);
}

void             moo_edit_load_error_dialog     (GtkWidget *widget,
                                                 const char *err_msg)
{
    moo_error_dialog (widget,
                      "Could not load file",
                      err_msg);
}


MooEditFileInfo *moo_edit_save_as_dialog        (MooEdit *edit)
{
    const char *title = "Save File";
    const char *start = NULL;
    const char *filename = NULL;
    MooEditFileInfo *file = NULL;

    start = moo_prefs_get (moo_edit_setting (MOO_EDIT_PREFS_DIALOGS_SAVE));
    if (!start)
        start = moo_prefs_get (moo_edit_setting (MOO_EDIT_PREFS_DIALOGS_OPEN));

    filename = moo_file_dialog (GTK_WIDGET (edit), MOO_DIALOG_FILE_SAVE, title, start);
    if (filename) {
        char *new_start = g_path_get_dirname (filename);
        moo_prefs_set (moo_edit_setting (MOO_EDIT_PREFS_DIALOGS_SAVE), new_start);
        g_free (new_start);
        file = moo_edit_file_info_new (filename, NULL);
    }

    return file;
}


MooEditFileInfo *moo_edit_open_dialog           (GtkWidget *widget)
{
    const char *title = "Open File";
    const char *start = NULL;
    const char *filename = NULL;
    MooEditFileInfo *file = NULL;

    start = moo_prefs_get (moo_edit_setting (MOO_EDIT_PREFS_DIALOGS_OPEN));
    filename = moo_file_dialog (widget, MOO_DIALOG_FILE_OPEN_EXISTING, title, start);
    if (filename) {
        char *new_start = g_path_get_dirname (filename);
        moo_prefs_set (moo_edit_setting (MOO_EDIT_PREFS_DIALOGS_OPEN), new_start);
        g_free (new_start);
        file = moo_edit_file_info_new (filename, NULL);
    }

    return file;
}


void             moo_edit_nothing_found_dialog  (MooEdit    *edit,
                                                 const char *text,
                                                 gboolean    regex)
{
    GtkWindow *parent_window;
    GtkWidget *dialog;
    char *msg_text;

    g_return_if_fail (MOO_IS_EDIT (edit) && text != NULL);

    if (regex)
        msg_text = g_strdup_printf ("Search pattern '%s' not found!", text);
    else
        msg_text = g_strdup_printf ("Search string '%s' not found!", text);

    parent_window = GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (edit)));
    dialog = gtk_message_dialog_new (parent_window, GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_INFO, GTK_BUTTONS_NONE,
                                     msg_text);
    gtk_dialog_add_buttons (GTK_DIALOG (dialog), GTK_STOCK_CLOSE,
                            GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);

    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    g_free (msg_text);
}


gboolean         moo_edit_search_from_beginning_dialog (MooEdit    *edit,
                                                        gboolean    backwards)
{
    GtkWindow *parent_window;
    GtkWidget *dialog;
    int response;
    const char *msg;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);

    if (backwards)
        msg = "Beginning of document reached.\n"
              "Continue from the end?";
    else
        msg = "End of document reached.\n"
              "Continue from the beginning?";

    parent_window = GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (edit)));
    dialog = gtk_message_dialog_new (parent_window, GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
                                     msg);
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


void             moo_edit_regex_error_dialog    (MooEdit    *edit,
                                                 GError     *err)
{
    GtkWindow *parent_window;
    GtkWidget *dialog;
    char *msg_text = NULL;

    g_return_if_fail (MOO_IS_EDIT (edit));

    if (err) {
        if (err->domain != EGG_REGEX_ERROR)
            g_warning ("%s: unknown error domain", G_STRLOC);
        else if (err->code != EGG_REGEX_ERROR_COMPILE &&
                 err->code != EGG_REGEX_ERROR_OPTIMIZE &&
                 err->code != EGG_REGEX_ERROR_REPLACE)
                    g_warning ("%s: unknown error code", G_STRLOC);

        msg_text = g_strdup (err->message);
    }

    if (!msg_text)
        msg_text = g_strdup_printf ("Invalid regular expression");

    parent_window = GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (edit)));
    dialog = gtk_message_dialog_new (parent_window, GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_ERROR, GTK_BUTTONS_NONE,
                                     msg_text);
    gtk_dialog_add_buttons (GTK_DIALOG (dialog), GTK_STOCK_CLOSE,
                            GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);

    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    g_free (msg_text);
}


void             moo_edit_replaced_n_dialog     (MooEdit    *edit,
                                                 guint       n)
{
    GtkWindow *parent_window;
    GtkWidget *dialog;
    char *msg_text;

    g_return_if_fail (MOO_IS_EDIT (edit));

    if (!n)
        msg_text = g_strdup_printf ("No replacement made");
    else if (n == 1)
        msg_text = g_strdup_printf ("1 replacement made");
    else
        msg_text = g_strdup_printf ("%d replacements made", n);

    parent_window = GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (edit)));
    dialog = gtk_message_dialog_new (parent_window, GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_INFO, GTK_BUTTONS_NONE,
                                     msg_text);
    gtk_dialog_add_buttons (GTK_DIALOG (dialog), GTK_STOCK_CLOSE,
                            GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);

    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    g_free (msg_text);
}


GtkWidget       *moo_edit_prompt_on_replace_dialog  (MooEdit    *edit)
{
    GtkWidget *dialog;
    GtkWindow *parent_window;

    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);

    parent_window = GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (edit)));
    dialog = _moo_edit_create_prompt_on_replace_dialog ();
    gtk_window_set_transient_for (GTK_WINDOW (dialog), parent_window);

    return dialog;
}


static gboolean  moo_edit_question_dialog       (MooEdit    *edit,
                                                 const char *text,
                                                 const char *button)
{
    int res;
    GtkWindow *parent = GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (edit)));

#if GTK_CHECK_VERSION(2,4,0)
    GtkWidget *dialog = gtk_message_dialog_new_with_markup (
        parent,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_NONE,
        "<span weight=\"bold\" size=\"larger\">%s</span>",
        text);
#else /* !GTK_CHECK_VERSION(2,4,0) */
    GtkWidget *dialog = gtk_message_dialog_new (
        parent,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_NONE,
        text);
#endif /* !GTK_CHECK_VERSION(2,4,0) */

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

//     gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);

    res = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    return res == GTK_RESPONSE_YES;
}

gboolean         moo_edit_reload_dialog             (MooEdit    *edit)
{
    return moo_edit_question_dialog (edit, "Reload?", "Reload");
}

gboolean         moo_edit_overwrite_modified_dialog (MooEdit    *edit)
{
    return moo_edit_question_dialog (edit, "Overwrite modified?", "Overwrite");
}

gboolean         moo_edit_overwrite_deleted_dialog  (MooEdit    *edit)
{
    return moo_edit_question_dialog (edit, "Overwrite deleted?", "Overwrite");
}


void             moo_edit_file_deleted_dialog       (MooEdit    *edit)
{
    moo_error_dialog (GTK_WIDGET (edit),
                      "File deleted",
                      "File deleted");
}


int              moo_edit_file_modified_on_disk_dialog (MooEdit *edit)
{
    moo_error_dialog (GTK_WIDGET (edit),
                      "File modified on disk",
                      "File modified on disk");
    return GTK_RESPONSE_CANCEL;
}
