/*
 *   mooterm/mootermwindow.c
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

#include "mooterm/mootermwindow.h"
#include "mooterm/mooterm-prefs.h"
#include "mooui/moouiobject-impl.h"
#include "mooutils/moocompat.h"
#include "mooutils/moodialogs.h"
#include "mooutils/moofileutils.h"
#include "mooutils/moostock.h"
#include "mooutils/mooprefs.h"


static void         moo_term_window_class_init          (MooTermWindowClass *klass);
static void         moo_term_window_init                (MooTermWindow     *window);
static GObject     *moo_term_window_constructor         (GType                  type,
                                                         guint                  n_props,
                                                         GObjectConstructParam *props);

static void         moo_term_window_save_selection      (MooTermWindow *window);


/* MOO_TYPE_TERM_WINDOW */
G_DEFINE_TYPE (MooTermWindow, moo_term_window, MOO_TYPE_WINDOW)


static void moo_term_window_class_init (MooTermWindowClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->constructor = moo_term_window_constructor;

    moo_ui_object_class_init (gobject_class, "Terminal", "Terminal");

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "SaveSelection",
                                    "name", "Save Selection",
                                    "label", "_Save Selection",
                                    "tooltip", "Save selected text to a file",
                                    "icon-stock-id", GTK_STOCK_SAVE,
                                    "accel", "<alt>S",
                                    "closure::callback", moo_term_window_save_selection,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "Copy",
                                    "name", "Copy",
                                    "label", "_Copy",
                                    "tooltip", "Copy",
                                    "icon-stock-id", GTK_STOCK_COPY,
                                    "accel", "<alt>C",
                                    "default-accel", "<alt>C",
                                    "closure::callback", moo_term_copy_clipboard,
                                    "closure::proxy-func", moo_term_window_get_term,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "Paste",
                                    "name", "Paste",
                                    "label", "_Paste",
                                    "tooltip", "Paste",
                                    "icon-stock-id", GTK_STOCK_PASTE,
                                    "accel", "<alt>V",
                                    "default-accel", "<alt>V",
                                    "closure::callback", moo_term_paste_clipboard,
                                    "closure::proxy-func", moo_term_window_get_term,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "SelectAll",
                                    "name", "Select All",
                                    "label", "Select _All",
                                    "tooltip", "Select all",
                                    "accel", "<alt>A",
                                    "closure::callback", moo_term_select_all,
                                    "closure::proxy-func", moo_term_window_get_term,
                                    NULL);
}


static void moo_term_window_init (MooTermWindow *window)
{
    g_object_set (G_OBJECT (window),
                  "menubar-ui-name", "Terminal/Menubar",
                  "toolbar-ui-name", "Terminal/Toolbar",
                  NULL);
}


static GObject     *moo_term_window_constructor         (GType                  type,
                                                         guint                  n_props,
                                                         GObjectConstructParam *props)
{
    MooTermWindow *window;
    GtkWidget *scrolledwindow;
    GtkWidget *terminal;

    GObject *object =
            G_OBJECT_CLASS(moo_term_window_parent_class)->constructor (type, n_props, props);

    window = MOO_TERM_WINDOW (object);

    /************************************************************************/
    /* Gui                                                                  */
    /***/
    gtk_window_set_default_size (GTK_WINDOW (window), 500, 450);

    scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (scrolledwindow);
    gtk_box_pack_start (GTK_BOX (MOO_WINDOW(window)->vbox), scrolledwindow, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
                                    GTK_POLICY_NEVER,
                                    GTK_POLICY_ALWAYS);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow),
                                         GTK_SHADOW_ETCHED_OUT);

    terminal = g_object_new (MOO_TYPE_TERM, NULL);
    gtk_widget_show (terminal);
    gtk_container_add (GTK_CONTAINER (scrolledwindow), terminal);
    GTK_WIDGET_SET_FLAGS (terminal, GTK_CAN_FOCUS);
    GTK_WIDGET_SET_FLAGS (terminal, GTK_CAN_DEFAULT);

    gtk_widget_grab_focus (terminal);
    gtk_widget_grab_default (terminal);

    /************************************************************************/
    /* MOO_TERM_WINDOW stuff                                                */
    /***/
    window->terminal = MOO_TERM (terminal);
    gtk_widget_show (MOO_WINDOW(window)->vbox);

#if 0
TODO TODO
//     GtkWidget *popup =
//         moo_ui_xml_create_widget (xml, "Terminal/Popup",
//                                   moo_ui_object_get_actions (MOO_UI_OBJECT (window)),
//                                   MOO_WINDOW (window)->accel_group,
//                                   MOO_WINDOW (window)->tooltips);
//     if (popup) moo_term_set_popup_menu (window->terminal, popup);
#endif

    return object;
}


void        moo_term_window_apply_settings  (MooTermWindow     *window)
{
    g_return_if_fail (MOO_IS_TERM_WINDOW (window));
    g_signal_emit_by_name (window->terminal, "apply_settings", NULL);
}


GtkWidget *moo_term_window_new (void)
{
    return GTK_WIDGET (g_object_new (MOO_TYPE_TERM_WINDOW, NULL));
}


static void moo_term_window_save_selection (MooTermWindow *self)
{
    char *text = moo_term_get_selection (self->terminal);

    if (!text)
        text = moo_term_get_content (self->terminal);

    if (text)
    {
        const char *filename =
                moo_file_dialogp (GTK_WIDGET (self),
                                  MOO_DIALOG_FILE_SAVE,
                                  "Save As",
                                  MOO_TERM_PREFS_SAVE_SELECTION_DIR,
                                  NULL);

        if (filename)
        {
            char *new_start = g_path_get_dirname (filename);
            moo_prefs_set (MOO_TERM_PREFS_SAVE_SELECTION_DIR, new_start);
            g_free (new_start);
            moo_save_file_utf8 (filename, text, -1, NULL);
        }

        g_free (text);
    }
}


MooTerm         *moo_term_window_get_term       (MooTermWindow  *window)
{
    g_return_val_if_fail (MOO_IS_TERM_WINDOW (window), NULL);
    return window->terminal;
}
