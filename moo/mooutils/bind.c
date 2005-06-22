/*
 *   mooutils/bind.c
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

#include "mooutils/bind.h"


typedef struct {
    GtkWidget  *wid;
    guint       invert : 1;
} WBPair;


static void button_destroyed (GArray *array)
{
    if (array)
        g_array_free (array, TRUE);
}


static void button_toggled (GtkToggleButton *btn)
{
    gboolean active = gtk_toggle_button_get_active (btn);
    GArray *dep_ctls = (GArray*) g_object_get_data (G_OBJECT (btn), "moo_bind_sensitive_ctls");
    guint i;

    if (!dep_ctls) return;
    for (i = 0; i < dep_ctls->len; ++i) {
        WBPair *pair = &g_array_index (dep_ctls, WBPair, i);
        gtk_widget_set_sensitive (pair->wid, pair->invert ? !active : active);
    }
}


void        moo_bind_sensitive  (GtkToggleButton    *btn,
                                 GtkWidget         **dependent,
                                 int                 num,
                                 gboolean            invert)
{
    GArray *dep_ctls;
    int i;
    GtkWidget **w;

    g_return_if_fail (btn != NULL && dependent != NULL);
    if (num == 0) return;

    dep_ctls = (GArray*) g_object_get_data (G_OBJECT (btn), "moo_bind_sensitive_ctls");
    if (!dep_ctls) {
        dep_ctls = g_array_new (FALSE, FALSE, sizeof(WBPair));
        g_object_set_data_full (G_OBJECT (btn), "moo_bind_sensitive_ctls",
                                dep_ctls, (GDestroyNotify) button_destroyed);
    }

    if (num > 0)
        for (i = 0; i < num; ++i)
        {
            WBPair p = {dependent[i], invert};
            g_array_append_val (dep_ctls, p);
        }
    else
        for (w = dependent; *w != NULL; ++w)
        {
            WBPair p = {*w, invert};
            g_array_append_val (dep_ctls, p);
        }

    button_toggled (btn);

    g_signal_handlers_disconnect_matched (btn, G_SIGNAL_MATCH_FUNC,
                                          0, 0, 0, (gpointer)button_toggled, 0);
    g_signal_connect (G_OBJECT (btn), "toggled", G_CALLBACK (button_toggled), NULL);
}


/*****************************************************************************/


static void button_clicked (GtkButton *btn, GtkEntry *entry)
{
    MooQueryTextFunc func = (MooQueryTextFunc)g_object_get_data (G_OBJECT (btn), "moo_query_text_func");
    MooTransformTextFunc text_func = (MooTransformTextFunc)g_object_get_data (G_OBJECT (btn), "moo_transform_text_func");
    void *data = g_object_get_data (G_OBJECT (btn), "moo_bind_button_data");
    const char *text = func (data, GTK_WIDGET (btn));
    if (!text) return;
    if (text_func) {
        char *transformed = text_func (text, data);
        gtk_entry_set_text (entry, transformed);
        g_free (transformed);
    }
    else
        gtk_entry_set_text (entry, text);
}


void        moo_bind_button     (GtkButton                  *button,
                                 GtkEntry                   *entry,
                                 MooQueryTextFunc            func,
                                 MooTransformTextFunc        text_func,
                                 gpointer                    data)
{
    g_return_if_fail (button != NULL && entry != NULL && func != NULL);
    g_object_set_data (G_OBJECT (button), "moo_query_text_func", (gpointer)func);
    g_object_set_data (G_OBJECT (button), "moo_transform_text_func", (gpointer)text_func);
    g_object_set_data (G_OBJECT (button), "moo_bind_button_data", data);
    g_signal_connect (button, "clicked", G_CALLBACK (button_clicked), entry);
}


char       *moo_quote_text      (const char *text,
                                 G_GNUC_UNUSED gpointer data)
{
    return text ? g_strdup_printf ("\"%s\"", text) : NULL;
}

