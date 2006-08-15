/*
 *   mooterm/mootermwindow.c
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

#define MOOTERM_COMPILATION
#include "mooterm/mootermwindow.h"
#include "mooterm/mooterm-prefs.h"
#include "mooterm/mooterm-private.h"
#include "mooutils/moocompat.h"
#include "mooutils/moodialogs.h"
#include "mooutils/mooutils-fs.h"
#include "mooutils/moostock.h"
#include "mooutils/mooprefs.h"


static void         moo_term_window_class_init  (MooTermWindowClass *klass);
static void         moo_term_window_init        (MooTermWindow      *window);
static GObject     *moo_term_window_constructor (GType               type,
                                                 guint               n_props,
                                                 GObjectConstructParam *props);
static void         moo_term_window_destroy     (GtkObject          *object);

static void         copy_clipboard              (MooTerm        *term);
static void         paste_clipboard             (MooTerm        *term);

static void         prefs_notify                (const char     *key,
                                                 const GValue   *newval,
                                                 MooTermWindow  *window);


/* MOO_TYPE_TERM_WINDOW */
G_DEFINE_TYPE (MooTermWindow, moo_term_window, MOO_TYPE_WINDOW)


static void moo_term_window_class_init (MooTermWindowClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    MooWindowClass *window_class = MOO_WINDOW_CLASS (klass);

    gobject_class->constructor = moo_term_window_constructor;
    GTK_OBJECT_CLASS(klass)->destroy = moo_term_window_destroy;

    moo_window_class_set_id (window_class, "Terminal", "Terminal");

    moo_window_class_new_action (window_class, "Copy", NULL,
                                 "display-name", GTK_STOCK_COPY,
                                 "label", GTK_STOCK_COPY,
                                 "tooltip", GTK_STOCK_COPY,
                                 "stock-id", GTK_STOCK_COPY,
                                 "accel", "<alt>C",
                                 "closure-callback", copy_clipboard,
                                 "closure-proxy-func", moo_term_window_get_term,
                                 NULL);

    moo_window_class_new_action (window_class, "Paste", NULL,
                                 "display-name", GTK_STOCK_PASTE,
                                 "label", GTK_STOCK_PASTE,
                                 "tooltip", GTK_STOCK_PASTE,
                                 "stock-id", GTK_STOCK_PASTE,
                                 "accel", "<alt>V",
                                 "closure-callback", paste_clipboard,
                                 "closure-proxy-func", moo_term_window_get_term,
                                 NULL);

    moo_window_class_new_action (window_class, "SelectAll", NULL,
                                 "display-name", "Select All",
                                 "label", "Select _All",
                                 "tooltip", "Select all",
                                 "accel", "<alt>A",
                                 "closure-callback", moo_term_select_all,
                                 "closure-proxy-func", moo_term_window_get_term,
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

    scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (scrolledwindow);
    gtk_box_pack_start (GTK_BOX (MOO_WINDOW(window)->vbox), scrolledwindow, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
                                    GTK_POLICY_NEVER,
                                    GTK_POLICY_ALWAYS);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow),
                                         GTK_SHADOW_ETCHED_OUT);

    if (window->term_type)
    {
        if (!g_type_is_a (window->term_type, MOO_TYPE_TERM))
        {
            g_critical ("%s: oops", G_STRLOC);
            window->term_type = MOO_TYPE_TERM;
        }
    }
    else
    {
        window->term_type = MOO_TYPE_TERM;
    }

    terminal = g_object_new (window->term_type, NULL);
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

    _moo_term_apply_settings (window->terminal);

    window->prefs_notify_id =
            moo_prefs_notify_connect (MOO_TERM_PREFS_PREFIX,
                                      MOO_PREFS_MATCH_PREFIX,
                                      (MooPrefsNotify) prefs_notify,
                                      window, NULL);

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


static void
moo_term_window_destroy (GtkObject *object)
{
    MooTermWindow *window = MOO_TERM_WINDOW (object);

    if (window->prefs_notify_id)
        moo_prefs_notify_disconnect (window->prefs_notify_id);
    if (window->apply_prefs_idle)
        g_source_remove (window->apply_prefs_idle);

    window->prefs_notify_id = 0;
    window->apply_prefs_idle = 0;

    GTK_OBJECT_CLASS (moo_term_window_parent_class)->destroy (object);
}


static gboolean
apply_prefs (MooTermWindow *window)
{
    window->apply_prefs_idle = 0;
    _moo_term_apply_settings (window->terminal);
    return FALSE;
}


static void
prefs_notify (G_GNUC_UNUSED const char *key,
              G_GNUC_UNUSED const GValue *newval,
              MooTermWindow  *window)
{
    g_return_if_fail (MOO_IS_TERM_WINDOW (window));

    if (!window->apply_prefs_idle)
        window->apply_prefs_idle =
                g_idle_add ((GSourceFunc) apply_prefs, window);
}


GtkWidget *
moo_term_window_new (void)
{
    return g_object_new (MOO_TYPE_TERM_WINDOW, NULL);
}


MooTerm *
moo_term_window_get_term (MooTermWindow *window)
{
    g_return_val_if_fail (MOO_IS_TERM_WINDOW (window), NULL);
    return window->terminal;
}


void
moo_term_window_set_term_type (MooTermWindow  *window,
                               GType           type)
{
    g_return_if_fail (MOO_IS_TERM_WINDOW (window));
    g_return_if_fail (g_type_is_a (type, MOO_TYPE_TERM));
    g_return_if_fail (window->terminal == NULL);
    window->term_type = type;
}


static void
copy_clipboard (MooTerm *term)
{
    moo_term_copy_clipboard (term, GDK_SELECTION_CLIPBOARD);
}


static void
paste_clipboard (MooTerm *term)
{
    moo_term_paste_clipboard (term, GDK_SELECTION_CLIPBOARD);
}
