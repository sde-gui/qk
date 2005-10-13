/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   moofind.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef MOO_VERSION
#define MOO_VERSION NULL
#endif

#include "mooedit/mooplugin-macro.h"
#include "mooedit/plugins/mooeditplugins.h"
#include "mooterm/mooterm.h"
#include "mooutils/moostock.h"
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#define CONSOLE_PLUGIN_ID "Console"

enum {
    CMD_GREP = 1,
    CMD_FIND
};

typedef struct {
    MooPlugin parent;
    guint ui_merge_id;
} ConsolePlugin;

typedef struct {
    MooWinPlugin parent;
    MooTerm *terminal;
    MooEditWindow *window;
} ConsoleWindowPlugin;

#define WindowStuff ConsoleWindowPlugin


static void
console_start (WindowStuff *stuff)
{
    if (!moo_term_child_alive (stuff->terminal))
    {
        moo_term_soft_reset (stuff->terminal);
        moo_term_start_default_profile (stuff->terminal, NULL);
    }
}


static void
console_window_plugin_create (WindowStuff *stuff)
{
    GtkWidget *swin;
    MooPaneLabel *label;
    MooEditWindow *window = MOO_WIN_PLUGIN(stuff)->window;

    stuff->window = window;

    label = moo_pane_label_new (MOO_STOCK_TERMINAL, NULL, NULL, "Console");

    stuff->terminal = g_object_new (MOO_TYPE_TERM, NULL);
    /* XXX */
    g_signal_connect_swapped (stuff->terminal, "child_died",
                              G_CALLBACK (console_start), stuff);
    console_start (stuff);

    swin = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (swin),
                                         GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (swin), GTK_WIDGET (stuff->terminal));
    gtk_widget_show_all (swin);

    moo_edit_window_add_pane (window, CONSOLE_PLUGIN_ID,
                              swin, label, MOO_PANE_POS_BOTTOM);
}


static void
show_console_cb (MooEditWindow *window)
{
    GtkWidget *pane;
    pane = moo_edit_window_get_pane (window, CONSOLE_PLUGIN_ID);
    moo_big_paned_present_pane (window->paned, pane);
}


static gboolean
console_plugin_init (ConsolePlugin *plugin)
{
    MooWindowClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);
    MooEditor *editor = moo_editor_instance ();
    MooUIXML *xml = moo_editor_get_ui_xml (editor);

    g_return_val_if_fail (klass != NULL, FALSE);
    g_return_val_if_fail (editor != NULL, FALSE);

    moo_window_class_new_action (klass, "ShowConsole",
                                 "name", "Show Console",
                                 "label", "Show Console",
                                 "tooltip", "Show Console",
                                 "icon-stock-id", MOO_STOCK_TERMINAL,
                                 "closure::callback", show_console_cb,
                                 NULL);

    plugin->ui_merge_id = moo_ui_xml_new_merge_id (xml);

    moo_ui_xml_add_item (xml, plugin->ui_merge_id,
                         "Editor/Menubar/View",
                         "ShowConsole",
                         "ShowConsole", -1);

    g_type_class_unref (klass);
    return TRUE;
}


static void
console_plugin_deinit (ConsolePlugin *plugin)
{
    MooWindowClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);
    MooEditor *editor = moo_editor_instance ();
    MooUIXML *xml = moo_editor_get_ui_xml (editor);

    moo_window_class_remove_action (klass, "ShowConsole");

    if (plugin->ui_merge_id)
        moo_ui_xml_remove_ui (xml, plugin->ui_merge_id);
    plugin->ui_merge_id = 0;

    g_type_class_unref (klass);
}


static void
console_window_plugin_destroy (WindowStuff *stuff)
{
    moo_edit_window_remove_pane (stuff->window, CONSOLE_PLUGIN_ID);
}


MOO_WIN_PLUGIN_DEFINE (Console, console,
                       console_window_plugin_create,
                       console_window_plugin_destroy);
MOO_PLUGIN_DEFINE_INFO (console, CONSOLE_PLUGIN_ID,
                        "Console", "Console",
                        "Yevgen Muntyan <muntyan@tamu.edu>",
                        MOO_VERSION);
MOO_PLUGIN_DEFINE_FULL (Console, console,
                        console_plugin_init, console_plugin_deinit,
                        NULL, NULL, NULL, NULL, NULL,
                        console_window_plugin_get_type (), 0);


gboolean
moo_console_plugin_init (void)
{
    return moo_plugin_register (console_plugin_get_type ());
}
