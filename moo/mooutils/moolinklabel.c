/*
 *   moolinklabel.c
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

#include "mooutils/moolinklabel.h"
#include "mooutils/moomarshals.h"
#include "mooutils/mooutils-misc.h"
#include <gtk/gtk.h>


G_DEFINE_TYPE (MooLinkLabel, moo_link_label, GTK_TYPE_EVENT_BOX)

enum {
    PROP_0,
    PROP_TEXT,
    PROP_URL
};

enum {
    ACTIVATE,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};


static void
moo_link_label_init (MooLinkLabel *label)
{
    label->label = gtk_label_new (NULL);
    gtk_widget_show (label->label);
    gtk_container_add (GTK_CONTAINER (label), label->label);
}


static void
set_cursor (GtkWidget *widget,
            gboolean   hand)
{
    if (!GTK_WIDGET_REALIZED (widget))
        return;

    if (hand)
    {
        GdkCursor *cursor = gdk_cursor_new (GDK_HAND2);
        gdk_window_set_cursor (widget->window, cursor);
        gdk_cursor_unref (cursor);
    }
    else
    {
        gdk_window_set_cursor (widget->window, NULL);
    }
}


static void
moo_link_label_realize (GtkWidget *widget)
{
    MooLinkLabel *label = MOO_LINK_LABEL (widget);
    GTK_WIDGET_CLASS(moo_link_label_parent_class)->realize (widget);
    set_cursor (widget, label->url && label->text);
}


static void
moo_link_label_activate (G_GNUC_UNUSED MooLinkLabel *label,
                         const char   *url)
{
    moo_open_url (url);
}


static void
open_activated (MooLinkLabel *label)
{
    g_return_if_fail (label->url != NULL);
    g_signal_emit (label, signals[ACTIVATE], 0, label->url);
}

static void
copy_activated (MooLinkLabel *label)
{
    GtkClipboard *clipboard;

    g_return_if_fail (label->url != NULL);

    clipboard = gtk_widget_get_clipboard (GTK_WIDGET (label), GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text (clipboard, label->url, -1);
}

static gboolean
moo_link_label_button_press (GtkWidget      *widget,
                             GdkEventButton *event)
{
    MooLinkLabel *label = MOO_LINK_LABEL (widget);
    GtkWidget *menu, *item, *image;

    if (event->button == 1)
    {
        if (label->url)
            g_signal_emit (label, signals[ACTIVATE], 0, label->url);
        return TRUE;
    }

    if (event->button != 3 || !label->url)
        return FALSE;

    menu = gtk_menu_new ();

    item = gtk_image_menu_item_new_with_label ("Copy Link");
    image = gtk_image_new_from_stock (GTK_STOCK_COPY, GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);
    g_signal_connect_swapped (item, "activate", G_CALLBACK (copy_activated), label);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    gtk_widget_show_all (item);

    item = gtk_image_menu_item_new_with_label ("Open Link");
    g_signal_connect_swapped (item, "activate", G_CALLBACK (open_activated), label);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    gtk_widget_show_all (item);

    gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
                    event->button, event->time);

    return TRUE;
}


static void
moo_link_label_finalize (GObject *object)
{
    MooLinkLabel *label = MOO_LINK_LABEL (object);
    g_free (label->url);
    g_free (label->text);
    G_OBJECT_CLASS(moo_link_label_parent_class)->finalize (object);
}


static void
moo_link_label_set_property (GObject        *object,
                             guint           param_id,
                             const GValue   *value,
                             GParamSpec     *pspec)
{
    MooLinkLabel *label = MOO_LINK_LABEL (object);

    switch (param_id)
    {
        case PROP_TEXT:
            moo_link_label_set_text (label, g_value_get_string (value));
            break;

        case PROP_URL:
            moo_link_label_set_url (label, g_value_get_string (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
            break;
    }
}


static void
moo_link_label_get_property (GObject        *object,
                             guint           param_id,
                             GValue         *value,
                             GParamSpec     *pspec)
{
    MooLinkLabel *label = MOO_LINK_LABEL (object);

    switch (param_id)
    {
        case PROP_TEXT:
            g_value_set_string (value, label->text);
            break;

        case PROP_URL:
            g_value_set_string (value, label->url);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
            break;
    }
}


static void
moo_link_label_class_init (MooLinkLabelClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    object_class->finalize = moo_link_label_finalize;
    object_class->set_property = moo_link_label_set_property;
    object_class->get_property = moo_link_label_get_property;
    widget_class->button_press_event = moo_link_label_button_press;
    widget_class->realize = moo_link_label_realize;
    klass->activate = moo_link_label_activate;

    g_object_class_install_property (object_class,
                                     PROP_TEXT,
                                     g_param_spec_string ("text",
                                             "text",
                                             "text",
                                             NULL,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (object_class,
                                     PROP_URL,
                                     g_param_spec_string ("url",
                                             "url",
                                             "url",
                                             NULL,
                                             G_PARAM_READWRITE));

    signals[ACTIVATE] =
            g_signal_new ("activate",
                          G_TYPE_FROM_CLASS (object_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooLinkLabelClass, activate),
                          NULL, NULL,
                          _moo_marshal_VOID__STRING,
                          G_TYPE_NONE, 1,
                          G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);
}


const char *
moo_link_label_get_url (MooLinkLabel *label)
{
    g_return_val_if_fail (MOO_IS_LINK_LABEL (label), NULL);
    return label->url;
}


const char *
moo_link_label_get_text (MooLinkLabel *label)
{
    g_return_val_if_fail (MOO_IS_LINK_LABEL (label), NULL);
    return label->text;
}


static void
moo_link_label_update (MooLinkLabel *label)
{
    if (!label->text || !label->url)
    {
        gtk_label_set_text (label->label, label->text ? label->text : "");
        set_cursor (GTK_WIDGET (label), FALSE);
    }
    else
    {
        char *markup;
        markup = g_markup_printf_escaped ("<span foreground=\"#0000EE\">%s</span>",
                                          label->text);
        gtk_label_set_markup (label->label, markup);
        set_cursor (GTK_WIDGET (label), TRUE);
        g_free (markup);
    }
}


void
moo_link_label_set_url (MooLinkLabel   *label,
                        const char     *url)
{
    g_return_if_fail (MOO_IS_LINK_LABEL (label));
    g_free (label->url);
    label->url = url && *url ? g_strdup (url) : NULL;
    moo_link_label_update (label);
    g_object_notify (G_OBJECT (label), "url");
}


void
moo_link_label_set_text (MooLinkLabel   *label,
                         const char     *text)
{
    g_return_if_fail (MOO_IS_LINK_LABEL (label));
    g_free (label->text);
    label->text = text && *text ? g_strdup (text) : NULL;
    moo_link_label_update (label);
    g_object_notify (G_OBJECT (label), "text");
}
