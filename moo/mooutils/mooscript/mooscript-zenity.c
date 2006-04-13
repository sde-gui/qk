/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooscript-zenity.c
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

#include "mooscript-zenity.h"
#include "mooscript-context.h"
#include "mooutils/moodialogs.h"
#include <gtk/gtk.h>


#define CHECK_GTK(ctx)                                              \
G_STMT_START {                                                      \
    if (!gtk_init_check (&ctx->argc, &ctx->argv))                   \
        return ms_context_format_error (ctx, MS_ERROR_RUNTIME,      \
                                        "could not open display");  \
} G_STMT_END


static MSValue*
entry_func (MSValue   **args,
            guint       n_args,
            MSContext  *ctx)
{
    char *dialog_text = NULL, *entry_text = NULL;
    gboolean hide_text = FALSE;
    int response;
    GtkWidget *dialog, *entry;
    MSValue *result;

    CHECK_GTK (ctx);

    if (n_args > 0 && !ms_value_is_none (args[0]))
        entry_text = ms_value_print (args[0]);
    if (n_args > 1 && !ms_value_is_none (args[1]))
        dialog_text = ms_value_print (args[1]);
    if (n_args > 2 && !ms_value_is_none (args[2]))
        hide_text = ms_value_get_bool (args[2]);

    dialog = gtk_dialog_new_with_buttons (NULL, ctx->window,
                                          GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OK, GTK_RESPONSE_OK,
                                          NULL);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

    if (dialog_text)
    {
        GtkWidget *label;
        label = gtk_label_new (dialog_text);
        gtk_widget_show (label);
        gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), label, FALSE, FALSE, 0);
    }

    entry = gtk_entry_new ();
    gtk_widget_show (entry);
    gtk_entry_set_visibility (GTK_ENTRY (entry), !hide_text);
    gtk_entry_set_text (GTK_ENTRY (entry), entry_text ? entry_text : "");
    gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), entry, FALSE, FALSE, 0);

    response = gtk_dialog_run (GTK_DIALOG (dialog));

    if (response == GTK_RESPONSE_OK)
        result = ms_value_string (gtk_entry_get_text (GTK_ENTRY (entry)));
    else
        result = ms_value_none ();

    gtk_widget_destroy (dialog);
    g_free (dialog_text);
    g_free (entry_text);

    return result;
}

MSFunc *
ms_zenity_entry (void)
{
    return ms_cfunc_new_var (entry_func);
}


static MSValue*
text_func (MSValue   **args,
           guint       n_args,
           MSContext  *ctx)
{
    char *dialog_text = NULL, *text = NULL;
    int response;
    GtkWidget *dialog, *textview, *swin;
    GtkTextBuffer *buffer;
    MSValue *result;

    CHECK_GTK (ctx);

    if (n_args > 0)
        text = ms_value_print (args[0]);
    if (n_args > 1)
        dialog_text = ms_value_print (args[1]);

    dialog = gtk_dialog_new_with_buttons (NULL, ctx->window,
                                          GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OK, GTK_RESPONSE_OK,
                                          NULL);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

    if (dialog_text)
    {
        GtkWidget *label;
        label = gtk_label_new (dialog_text);
        gtk_widget_show (label);
        gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), label, FALSE, FALSE, 0);
    }

    swin = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (swin),
                                         GTK_SHADOW_ETCHED_IN);

    textview = gtk_text_view_new ();
    gtk_container_add (GTK_CONTAINER (swin), textview);
    gtk_widget_show_all (swin);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
    gtk_text_buffer_set_text (buffer, text ? text : "", -1);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), swin, FALSE, FALSE, 0);

    response = gtk_dialog_run (GTK_DIALOG (dialog));

    if (response == GTK_RESPONSE_OK)
    {
        GtkTextIter start, end;
        char *content;
        gtk_text_buffer_get_bounds (buffer, &start, &end);
        content = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
        result = ms_value_take_string (content);
    }
    else
    {
        result = ms_value_none ();
    }

    gtk_widget_destroy (dialog);
    g_free (dialog_text);
    g_free (text);

    if (!gtk_main_level ())
        while (gtk_events_pending ())
            gtk_main_iteration ();

    return result;
}

MSFunc*
ms_zenity_text (void)
{
    return ms_cfunc_new_var (text_func);
}


static MSValue*
message_dialog (MSValue      **args,
                guint          n_args,
                MSContext     *ctx,
                GtkMessageType type,
                gboolean       question)
{
    char *dialog_text = NULL;
    GtkWidget *dialog;
    int response;

    CHECK_GTK (ctx);

    if (n_args > 0)
        dialog_text = ms_value_print (args[0]);

    dialog = gtk_message_dialog_new (ctx->window,
                                     GTK_DIALOG_MODAL,
                                     type, GTK_BUTTONS_NONE,
                                     "%s", dialog_text ? dialog_text : "");

    if (question)
    {
        gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                GTK_STOCK_OK, GTK_RESPONSE_OK,
                                NULL);
        gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
    }
    else
    {
        gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                                GTK_STOCK_CLOSE, GTK_RESPONSE_CANCEL,
                                NULL);
        gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);
    }

    response = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    if (!gtk_main_level ())
        while (gtk_events_pending ())
            gtk_main_iteration ();

    g_free (dialog_text);

    if (!question)
        return ms_value_none ();
    else if (response == GTK_RESPONSE_OK)
        return ms_value_true ();
    else
        return ms_value_false ();
}


static MSValue*
info_func (MSValue   **args,
           guint       n_args,
           MSContext  *ctx)
{
    return message_dialog (args, n_args, ctx, GTK_MESSAGE_INFO, FALSE);
}

MSFunc *
ms_zenity_info (void)
{
    return ms_cfunc_new_var (info_func);
}


static MSValue*
error_func (MSValue   **args,
            guint       n_args,
            MSContext  *ctx)
{
    return message_dialog (args, n_args, ctx, GTK_MESSAGE_ERROR, FALSE);
}

MSFunc *
ms_zenity_error (void)
{
    return ms_cfunc_new_var (error_func);
}


static MSValue*
question_func (MSValue   **args,
               guint       n_args,
               MSContext  *ctx)
{
    return message_dialog (args, n_args, ctx, GTK_MESSAGE_QUESTION, TRUE);
}

MSFunc *
ms_zenity_question (void)
{
    return ms_cfunc_new_var (question_func);
}


static MSValue*
warning_func (MSValue   **args,
              guint       n_args,
              MSContext  *ctx)
{
    return message_dialog (args, n_args, ctx, GTK_MESSAGE_WARNING, TRUE);
}

MSFunc *
ms_zenity_warning (void)
{
    return ms_cfunc_new_var (warning_func);
}


static MSValue*
file_selector_func (MSValue   **args,
                    guint       n_args,
                    MSContext  *ctx,
                    MooFileDialogType type,
                    gboolean    multiple)
{
    MooFileDialog *dialog;
    MSValue *ret;
    char *start = NULL, *title = NULL;

    CHECK_GTK (ctx);

    if (n_args > 0)
        title = ms_value_print (args[0]);

    if (n_args > 1)
        start = ms_value_print (args[1]);

    dialog = moo_file_dialog_new (type, ctx->window ? GTK_WIDGET (ctx->window) : NULL,
                                  multiple, title, start, NULL);

    g_free (title);
    g_free (start);

    if (!moo_file_dialog_run (dialog))
    {
        g_object_unref (dialog);
        return ms_value_none ();
    }

    if (!multiple)
    {
        ret = ms_value_string (moo_file_dialog_get_filename (dialog));
    }
    else
    {
        GSList *names, *l;
        guint n_names, i;

        names = moo_file_dialog_get_filenames (dialog);
        n_names = g_slist_length (names);
        ret = ms_value_list (n_names);

        for (i = 0, l = names; i < n_names; ++i, l = l->next)
        {
            MSValue *n = ms_value_take_string (l->data);
            ms_value_list_set_elm (ret, i, n);
            ms_value_unref (n);
        }

        g_slist_free (names);
    }

    g_object_unref (dialog);
    return ret;
}


static MSValue*
choose_file_func (MSValue   **args,
                  guint       n_args,
                  MSContext  *ctx)
{
    return file_selector_func (args, n_args, ctx,
                               MOO_DIALOG_FILE_OPEN_EXISTING,
                               FALSE);
}

MSFunc*
ms_zenity_choose_file (void)
{
    return ms_cfunc_new_var (choose_file_func);
}


static MSValue*
choose_files_func (MSValue   **args,
                   guint       n_args,
                   MSContext  *ctx)
{
    return file_selector_func (args, n_args, ctx,
                               MOO_DIALOG_FILE_OPEN_EXISTING,
                               TRUE);
}

MSFunc*
ms_zenity_choose_files (void)
{
    return ms_cfunc_new_var (choose_files_func);
}


static MSValue*
choose_dir_func (MSValue   **args,
                 guint       n_args,
                 MSContext  *ctx)
{
    return file_selector_func (args, n_args, ctx,
                               MOO_DIALOG_DIR_OPEN,
                               FALSE);
}

MSFunc*
ms_zenity_choose_dir (void)
{
    return ms_cfunc_new_var (choose_dir_func);
}


static MSValue*
choose_file_save_func (MSValue   **args,
                       guint       n_args,
                       MSContext  *ctx)
{
    return file_selector_func (args, n_args, ctx,
                               MOO_DIALOG_FILE_SAVE,
                               FALSE);
}

MSFunc*
ms_zenity_choose_file_save (void)
{
    return ms_cfunc_new_var (choose_file_save_func);
}
