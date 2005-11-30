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

#include <gtk/gtk.h>
#include "mooutils/moodialogs.h"
#include "mooutils/mooprefs.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/moocompat.h"


static void message_dialog (GtkWidget       *parent,
                            const char      *text,
                            const char      *secondary_text,
                            GtkMessageType   type)
{
    GtkWindow *parent_window = NULL;
    GtkWidget *dialog;

    if (parent)
        parent = gtk_widget_get_toplevel (parent);

    if (GTK_IS_WINDOW (parent))
        parent_window = GTK_WINDOW (parent);
    else
        parent_window = moo_get_toplevel_window ();

#if GTK_CHECK_VERSION(2,6,0)
    dialog = gtk_message_dialog_new_with_markup (
        parent_window,
        GTK_DIALOG_MODAL,
        type,
        GTK_BUTTONS_NONE,
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
            GTK_BUTTONS_NONE,
            "<span weight=\"bold\" size=\"larger\">%s</span>\n%s",
            text, secondary_text ? secondary_text : "");
#else /* !GTK_CHECK_VERSION(2,4,0) */
    dialog = gtk_message_dialog_new (
            parent_window,
            GTK_DIALOG_MODAL,
            type,
            GTK_BUTTONS_NONE,
            "%s\n%s",
            text, secondary_text ? secondary_text : "");
#endif /* !GTK_CHECK_VERSION(2,4,0) */

    gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                            GTK_STOCK_OK, GTK_RESPONSE_CANCEL,
                            NULL);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog),
                                     GTK_RESPONSE_CANCEL);

    if (parent_window && parent_window->group)
        gtk_window_group_add_window (parent_window->group, GTK_WINDOW (dialog));

    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}


void
moo_error_dialog (GtkWidget  *parent,
                  const char *text,
                  const char *secondary_text)
{
    return message_dialog (parent, text, secondary_text, GTK_MESSAGE_ERROR);
}

void
moo_info_dialog (GtkWidget  *parent,
                 const char *text,
                 const char *secondary_text)
{
    return message_dialog (parent, text, secondary_text, GTK_MESSAGE_INFO);
}

void
moo_warning_dialog (GtkWidget  *parent,
                    const char *text,
                    const char *secondary_text)
{
    return message_dialog (parent, text, secondary_text, GTK_MESSAGE_WARNING);
}


gboolean
moo_overwrite_file_dialog (GtkWidget  *parent,
                           const char *display_name,
                           const char *display_dirname)
{
    int response;
    GtkWidget *dialog;
    GtkWidget *button;

    g_return_val_if_fail (display_name != NULL, FALSE);

    if (parent)
        parent = gtk_widget_get_toplevel (parent);
    if (parent && !GTK_WIDGET_TOPLEVEL (parent))
        parent = NULL;

    dialog = gtk_message_dialog_new (parent ? GTK_WINDOW (parent) : NULL,
                                     GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_WARNING,
                                     GTK_BUTTONS_NONE,
                                     "A file named \"%s\" already exists.  Do you want to replace it?",
                                     display_name);

#if GTK_CHECK_VERSION(2,6,0)
    gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                              "The file already exists in \"%s\".  Replacing it will "
                                              "overwrite its contents.",
                                              display_dirname);
#endif

    gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);

    button = gtk_button_new_with_mnemonic ("_Replace");
    gtk_button_set_image (GTK_BUTTON (button),
                          gtk_image_new_from_stock (GTK_STOCK_SAVE_AS, GTK_ICON_SIZE_BUTTON));
    gtk_widget_show (button);
    gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_YES);

#if GTK_CHECK_VERSION(2,6,0)
    gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                             GTK_RESPONSE_YES,
                                             GTK_RESPONSE_CANCEL,
                                             -1);
#endif /* GTK_CHECK_VERSION(2,6,0) */

    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);

    if (parent && GTK_WINDOW(parent)->group)
        gtk_window_group_add_window (GTK_WINDOW(parent)->group, GTK_WINDOW (dialog));

    response = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    return response == GTK_RESPONSE_YES;
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

#define file_chooser_set_select_multiple(dialog,multiple) \
    gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), multiple)
#define file_chooser_get_filename(dialog) \
    (gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog)))
#define file_chooser_get_filenames(dialog)  \
    (gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER (dialog)))

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

#define file_chooser_set_select_multiple(dialog,multiple) \
    gtk_file_selection_set_select_multiple (GTK_FILE_SELECTION (dialog), multiple)
#define file_chooser_get_filename(dialog) \
    g_strdup (gtk_file_selection_get_filename (GTK_FILE_SELECTION (dialog)))
#define file_chooser_get_filenames(dialog)  \
    (gtk_file_selection_get_selections (GTK_FILE_SELECTION (dialog)))

#endif /* !GTK_CHECK_VERSION(2,4,0) */


const char *moo_file_dialog (GtkWidget  *parent,
                             MooFileDialogType type,
                             const char *title,
                             const char *start_dir)
{
    static char *filename;
    GtkWidget *dialog;

    dialog = moo_file_dialog_create (parent, type, FALSE, title, start_dir);
    g_return_val_if_fail (dialog != NULL, NULL);

    moo_file_dialog_run (dialog);

    g_free (filename);
    filename = g_strdup (moo_file_dialog_get_filename (dialog));

    gtk_widget_destroy (dialog);
    return filename;
}


GtkWidget  *moo_file_dialog_create          (GtkWidget          *parent,
                                             MooFileDialogType   type,
                                             gboolean            multiple,
                                             const char         *title,
                                             const char         *start_dir)
{
    GtkWindow *parent_window = NULL;
    GtkFileChooserAction chooser_action;
    GtkWidget *dialog = NULL;

    if (parent)
        parent_window = GTK_WINDOW (gtk_widget_get_toplevel (parent));

    switch (type) {
        case MOO_DIALOG_FILE_OPEN_EXISTING:
        case MOO_DIALOG_FILE_OPEN_ANY:
        case MOO_DIALOG_DIR_OPEN:
            if (type == MOO_DIALOG_DIR_OPEN)
                chooser_action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
            else
                chooser_action = GTK_FILE_CHOOSER_ACTION_OPEN;

            dialog = file_chooser_dialog_new (title, parent_window, chooser_action,
                                              GTK_STOCK_OPEN, start_dir);
            file_chooser_set_select_multiple (dialog, multiple);
            gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
            break;

        case MOO_DIALOG_FILE_SAVE:
            chooser_action = GTK_FILE_CHOOSER_ACTION_SAVE;

            dialog = file_chooser_dialog_new (title, parent_window, chooser_action,
                                              GTK_STOCK_SAVE, start_dir);
            file_chooser_set_select_multiple (dialog, multiple);
            gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
            break;

        default:
            g_critical ("%s: incorrect dialog type specified", G_STRLOC);
            return NULL;
    }

    g_object_set_data (G_OBJECT (dialog),
                       "moo-file-dialog-action",
                       GINT_TO_POINTER (type));
    g_object_set_data (G_OBJECT (dialog),
                       "moo-file-dialog",
                       GINT_TO_POINTER (1));
    g_object_set_data (G_OBJECT (dialog),
                       "moo-file-dialog-multiple",
                       GINT_TO_POINTER (multiple));
    g_object_set_data_full (G_OBJECT (dialog),
                            "moo-file-dialog-filename", NULL,
                            g_free);

    return dialog;
}


static GSList *string_slist_copy (GSList *list)
{
    GSList *copy = NULL;
    GSList *l;

    for (l = list; l != NULL; l = l->next)
        copy = g_slist_prepend (copy, g_strdup (l->data));

    return g_slist_reverse (copy);
}


static void    string_slist_free (GSList *list)
{
    g_slist_foreach (list, (GFunc) g_free, NULL);
    g_slist_free (list);
}


gboolean    moo_file_dialog_run             (GtkWidget          *dialog)
{
    char *filename;
    MooFileDialogType type;

    g_return_val_if_fail (GPOINTER_TO_INT (g_object_get_data
            (G_OBJECT (dialog), "moo-file-dialog")) == 1, FALSE);
    g_return_val_if_fail (dialog != NULL, FALSE);

    g_object_set_data_full (G_OBJECT (dialog),
                            "moo-file-dialog-filename", NULL,
                            g_free);

    type = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (dialog),
                            "moo-file-dialog-action"));

    switch (type)
    {
        case MOO_DIALOG_FILE_OPEN_EXISTING:
        case MOO_DIALOG_FILE_OPEN_ANY:
        case MOO_DIALOG_DIR_OPEN:
            if (gtk_dialog_run (GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
            {
                g_object_set_data_full (G_OBJECT (dialog),
                                        "moo-file-dialog-filename",
                                        file_chooser_get_filename (dialog),
                                        g_free);
                if (g_object_get_data (G_OBJECT (dialog), "moo-file-dialog-multiple"))
                    g_object_set_data_full (G_OBJECT (dialog),
                                            "moo-file-dialog-filenames",
                                            file_chooser_get_filenames (dialog),
                                            (GDestroyNotify) string_slist_free);
                return TRUE;
            }
            else
            {
                return FALSE;
            }

        case MOO_DIALOG_FILE_SAVE:
            while (TRUE)
            {
                if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (dialog)))
                {
                    filename = file_chooser_get_filename (dialog);

                    if (g_file_test (filename, G_FILE_TEST_EXISTS) &&
                        ! g_file_test (filename, G_FILE_TEST_IS_REGULAR))
                    {
                        moo_error_dialog (dialog,
                                          "Choosen file is not a regular file",
                                          NULL);
                        g_free (filename);
                    }
                    else if (g_file_test (filename, G_FILE_TEST_EXISTS) &&
                             g_file_test (filename, G_FILE_TEST_IS_REGULAR))
                    {
                        char *basename = g_path_get_basename (filename);
                        char *dirname = g_path_get_dirname (filename);
                        char *display_name = g_filename_display_name (basename);
                        char *display_dirname = g_filename_display_name (dirname);
                        gboolean overwrite;

                        overwrite = moo_overwrite_file_dialog (dialog, display_name, display_dirname);

                        g_free (basename);
                        g_free (dirname);
                        g_free (display_name);
                        g_free (display_dirname);

                        if (overwrite)
                        {
                            g_object_set_data_full (G_OBJECT (dialog), "moo-file-dialog-filename",
                                                    filename, g_free);
                            return TRUE;
                        }
                    }
                    else
                    {
                        g_object_set_data_full (G_OBJECT (dialog),
                                                "moo-file-dialog-filename", filename,
                                                g_free);
                        return TRUE;
                    }
                }
                else
                {
                    return FALSE;
                }
            }

        default:
            g_critical ("%s: incorrect dialog type specified", G_STRLOC);
            return FALSE;
    }
}


const char  *moo_file_dialog_get_filename    (GtkWidget          *dialog)
{
    g_return_val_if_fail (dialog != NULL, NULL);
    g_return_val_if_fail (GPOINTER_TO_INT (g_object_get_data
            (G_OBJECT (dialog), "moo-file-dialog")) == 1, NULL);
    return g_object_get_data (G_OBJECT (dialog), "moo-file-dialog-filename");
}


GSList     *moo_file_dialog_get_filenames   (GtkWidget          *dialog)
{
    g_return_val_if_fail (dialog != NULL, NULL);
    g_return_val_if_fail (GPOINTER_TO_INT (g_object_get_data
            (G_OBJECT (dialog), "moo-file-dialog")) == 1, NULL);
    g_return_val_if_fail (g_object_get_data (G_OBJECT (dialog),
                          "moo-file-dialog-multiple"), NULL);
    return string_slist_copy (g_object_get_data (G_OBJECT (dialog),
                              "moo-file-dialog-filenames"));
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

    if (prefs_key && moo_prefs_get (prefs_key))
        start = moo_prefs_get_string (prefs_key);

    if (!start && alternate_prefs_key && moo_prefs_get (alternate_prefs_key))
        start = moo_prefs_get_string (alternate_prefs_key);

    filename = moo_file_dialog (parent, type, title, start);

    if (filename && prefs_key)
    {
        char *new_start = g_path_get_dirname (filename);
        moo_prefs_new_key_string (prefs_key, NULL);
        moo_prefs_set_filename (prefs_key, new_start);
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
