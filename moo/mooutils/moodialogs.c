/*
 *   mooutils/moodialogs.c
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

#include "mooutils/moodialogs.h"
#include "mooutils/mooprefs.h"
#include <gtk/gtk.h>


static void message_dialog (GtkWidget       *parent,
                            const char      *text,
                            const char      *secondary_text,
                            GtkMessageType   type)
{
    GtkWindow *parent_window = NULL;
    GtkWidget *dialog;

    if (parent) parent_window = GTK_WINDOW (gtk_widget_get_toplevel (parent));

#if GTK_CHECK_VERSION(2,6,0)
    dialog = gtk_message_dialog_new_with_markup (
        parent_window,
        GTK_DIALOG_MODAL,
        type,
        GTK_BUTTONS_OK,
        "<span weight=\"bold\" size=\"larger\">%s</span>", text);
    if (secondary_text)
        gtk_message_dialog_format_secondary_text (
            GTK_MESSAGE_DIALOG (dialog),
            "%s", secondary_text);
#elif GTK_CHECK_VERSION(2,4,0)
    dialog = gtk_message_dialog_new_with_markup (
            parent_window,
            GTK_DIALOG_MODAL,
            type,
            GTK_BUTTONS_OK,
            "<span weight=\"bold\" size=\"larger\">%s</span>\n%s",
            text, secondary_text ? secondary_text : "");
#else /* !GTK_CHECK_VERSION(2,4,0) */
    dialog = gtk_message_dialog_new (
            parent_window,
            GTK_DIALOG_MODAL,
            type,
            GTK_BUTTONS_OK,
            "%s\n%s",
            text, secondary_text ? secondary_text : "");
#endif /* !GTK_CHECK_VERSION(2,4,0) */

    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}


void        moo_error_dialog    (GtkWidget  *parent,
                                 const char *text,
                                 const char *secondary_text)
{
    return message_dialog (parent, text, secondary_text, GTK_MESSAGE_ERROR);
}

void        moo_info_dialog     (GtkWidget  *parent,
                                 const char *text,
                                 const char *secondary_text)
{
    return message_dialog (parent, text, secondary_text, GTK_MESSAGE_INFO);
}

void        moo_warning_dialog  (GtkWidget  *parent,
                                 const char *text,
                                 const char *secondary_text)
{
    return message_dialog (parent, text, secondary_text, GTK_MESSAGE_WARNING);
}


static gboolean overwrite_dialog (GtkWindow  *parent)
{
    int res;

#if GTK_CHECK_VERSION(2,4,0)
    GtkWidget *dialog = gtk_message_dialog_new_with_markup (
        parent,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_NONE,
        "<span weight=\"bold\" size=\"larger\">Overwrite existing file?</span>");
#else /* !GTK_CHECK_VERSION(2,4,0) */
    GtkWidget *dialog = gtk_message_dialog_new (
        parent,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_NONE,
        "Overwrite existing file?");
#endif /* !GTK_CHECK_VERSION(2,4,0) */

    gtk_dialog_add_buttons (GTK_DIALOG (dialog),
        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
        "_Overwrite", GTK_RESPONSE_YES,
        NULL);

#if GTK_CHECK_VERSION(2,6,0)
    gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                             GTK_RESPONSE_YES,
                                             GTK_RESPONSE_CANCEL,
                                             -1);
#endif /* GTK_CHECK_VERSION(2,6,0) */

    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);

    res = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    return res == GTK_RESPONSE_YES;
}


#if GTK_CHECK_VERSION(2,4,0)

inline static
GtkWidget *file_chooser_dialog_new (const char *title,
                                    GtkWindow *parent,
                                    GtkFileChooserAction action,
                                    const char *okbtn,
                                    const char *start_dir)
{
    GtkWidget *dialog = gtk_file_chooser_dialog_new (
        title, parent, action,
        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
        okbtn, GTK_RESPONSE_OK,
        NULL);

#if GTK_CHECK_VERSION(2,6,0)
    gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                             GTK_RESPONSE_OK,
                                             GTK_RESPONSE_CANCEL,
                                             -1);
#endif /* GTK_CHECK_VERSION(2,6,0) */

    if (start_dir)
        gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog),
                                             start_dir);
    return dialog;
}

#define file_chooser_get_filename(dialog) \
    (gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog)))

#else /* !GTK_CHECK_VERSION(2,4,0) */

#define GtkFileChooserAction int
#define GTK_FILE_CHOOSER_ACTION_SAVE            1
#define GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER   2
#define GTK_FILE_CHOOSER_ACTION_OPEN            3

inline static
GtkWidget *file_chooser_dialog_new (const char              *title,
                                    GtkWindow               *parent,
                                    G_GNUC_UNUSED GtkFileChooserAction action,
                                    G_GNUC_UNUSED const char *okbtn,
                                    const char              *start_dir)
{
    GtkWidget *dialog = gtk_file_selection_new (title);
    gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);
    if (start_dir) {
        char *dir = g_strdup_printf ("%s/", start_dir);
        gtk_file_selection_set_filename (GTK_FILE_SELECTION (dialog), dir);
        g_free (dir);
    }

    return dialog;
}

#define file_chooser_get_filename(dialog) \
    g_strdup (gtk_file_selection_get_filename (GTK_FILE_SELECTION (dialog)))

#endif /* !GTK_CHECK_VERSION(2,4,0) */


const char *moo_file_dialog (GtkWidget  *parent,
                             MooFileDialogType type,
                             const char *title,
                             const char *start_dir)
{
    static char *filename = NULL;
    GtkWindow *parent_window = NULL;
    GtkFileChooserAction chooser_action;
    GtkWidget *dialog = NULL;

    if (filename) g_free (filename);
    filename = NULL;

    parent_window = NULL;
    if (parent) parent_window = GTK_WINDOW (gtk_widget_get_toplevel (parent));

    switch (type) {
    case MOO_DIALOG_FILE_OPEN_EXISTING:
    case MOO_DIALOG_FILE_OPEN_ANY:
    case MOO_DIALOG_DIR_OPEN:
    {
        if (type == MOO_DIALOG_DIR_OPEN)
            chooser_action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
        else
            chooser_action = GTK_FILE_CHOOSER_ACTION_OPEN;

        dialog = file_chooser_dialog_new (title, parent_window, chooser_action,
                                          GTK_STOCK_OPEN, start_dir);
        gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

        if (gtk_dialog_run (GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
        {
            filename = file_chooser_get_filename (dialog);
            gtk_widget_destroy (dialog);
            return filename;
        }
        else {
            gtk_widget_destroy (dialog);
            return NULL;
        }
    }

    case MOO_DIALOG_FILE_SAVE:
    {
        chooser_action = GTK_FILE_CHOOSER_ACTION_SAVE;

        dialog = file_chooser_dialog_new (title, parent_window, chooser_action,
                                          GTK_STOCK_SAVE, start_dir);
        gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

        while (TRUE) {
            if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (dialog))) {
                filename = file_chooser_get_filename (dialog);
                if (g_file_test (filename, G_FILE_TEST_EXISTS) &&
                    ! g_file_test (filename, G_FILE_TEST_IS_REGULAR))
                {
                    moo_error_dialog (dialog,
                                      "Choosen file is not a regular file",
                                      NULL);
                    g_free (filename);
                    filename = NULL;
                }
                else if (g_file_test (filename, G_FILE_TEST_EXISTS) &&
                        g_file_test (filename, G_FILE_TEST_IS_REGULAR))
                {
                    if (overwrite_dialog (GTK_WINDOW (dialog))) {
                        gtk_widget_destroy (dialog);
                        return filename;
                    }
                    else {
                        g_free (filename);
                        filename = NULL;
                    }
                }
                else { /* file doesn't exist */
                    gtk_widget_destroy (dialog);
                    return filename;
                }
            }
            else {
                gtk_widget_destroy (dialog);
                return NULL;
            }
        }
    }

    default:
        g_critical ("%s: incorrect dialog type specified", G_STRLOC);
        return NULL;
    }
}


const char *moo_file_dialogp(GtkWidget          *parent,
                             MooFileDialogType   type,
                             const char         *title,
                             const char         *prefs_key,
                             const char         *alternate_prefs_key)
{
    const char *start = NULL;
    const char *filename = NULL;

    if (!title) title = "Choose File";

    if (prefs_key)
        start = moo_prefs_get_string (prefs_key);

    if (!start && alternate_prefs_key)
        start = moo_prefs_get_string (alternate_prefs_key);

    filename = moo_file_dialog (parent, type, title, start);

    if (filename && prefs_key)
    {
        char *new_start = g_path_get_dirname (filename);
        moo_prefs_set_string (prefs_key, new_start);
        g_free (new_start);
    }

    return filename;
}


const char *moo_font_dialog (GtkWidget  *parent,
                             const char *title,
                             const char *start_font,
                             gboolean fixed_width)
{
    GtkWindow *parent_window = NULL;
    GtkWidget *dialog;
    const char *fontname = NULL;

    if (fixed_width)
        g_warning ("%s: choosing fixed width fonts "
                   "only is not implemented", G_STRLOC);

    if (parent) parent_window = GTK_WINDOW (gtk_widget_get_toplevel (parent));

    dialog = gtk_font_selection_dialog_new (title);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
    if (parent_window)
        gtk_window_set_transient_for (GTK_WINDOW (dialog), parent_window);
    if (start_font)
        gtk_font_selection_dialog_set_font_name (
            GTK_FONT_SELECTION_DIALOG (dialog), start_font);

    if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (dialog)))
        fontname = gtk_font_selection_dialog_get_font_name (
            GTK_FONT_SELECTION_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    return fontname;
}


GType moo_file_dialog_type_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GEnumValue values[] = {
            { MOO_DIALOG_FILE_OPEN_EXISTING, (char*)"MOO_DIALOG_FILE_OPEN_EXISTING", (char*)"file-open-existing" },
            { MOO_DIALOG_FILE_OPEN_ANY, (char*)"MOO_DIALOG_FILE_OPEN_ANY", (char*)"file-open-any" },
            { MOO_DIALOG_FILE_SAVE, (char*)"MOO_DIALOG_FILE_SAVE", (char*)"file-save" },
            { MOO_DIALOG_DIR_OPEN, (char*)"MOO_DIALOG_DIR_OPEN", (char*)"dir-open" },
            { 0, NULL, NULL }
        };
        type = g_enum_register_static ("MooFileDialogType", values);
    }

    return type;
}
