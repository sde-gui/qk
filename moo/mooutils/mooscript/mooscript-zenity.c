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
#include <gtk/gtk.h>


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

    if (n_args > 0)
        dialog_text = ms_value_print (args[0]);
    if (n_args > 1)
        entry_text = ms_value_print (args[1]);
    if (n_args > 2)
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
message_dialog (MSValue      **args,
                guint          n_args,
                MSContext     *ctx,
                GtkMessageType type)
{
    char *dialog_text = NULL;
    GtkWidget *dialog;

    if (n_args > 0)
        dialog_text = ms_value_print (args[0]);

    dialog = gtk_message_dialog_new (ctx->window,
                                     GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR,
                                     type, GTK_BUTTONS_NONE,
                                     "%s", dialog_text ? dialog_text : "");
    gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                            GTK_STOCK_CLOSE, GTK_RESPONSE_CANCEL,
                            NULL);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);

    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    g_free (dialog_text);
    return ms_value_none ();
}


static MSValue*
info_func (MSValue   **args,
           guint       n_args,
           MSContext  *ctx)
{
    return message_dialog (args, n_args, ctx, GTK_MESSAGE_INFO);
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
    return message_dialog (args, n_args, ctx, GTK_MESSAGE_ERROR);
}

MSFunc *
ms_zenity_error (void)
{
    return ms_cfunc_new_var (error_func);
}
