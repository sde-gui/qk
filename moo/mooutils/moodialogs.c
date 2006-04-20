/*
 *   mooutils/moodialogs.c
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

#include <gtk/gtk.h>
#include "mooutils/moodialogs.h"
#include "mooutils/mooprefs.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/moocompat.h"


static GtkWidget *
create_message_dialog (GtkWindow  *parent,
                       GtkMessageType type,
                       const char *text,
                       const char *secondary_text)
{
    GtkWidget *dialog;

#if GTK_CHECK_VERSION(2,6,0)
    dialog = gtk_message_dialog_new_with_markup (parent,
                                                 GTK_DIALOG_MODAL,
                                                 type,
                                                 GTK_BUTTONS_NONE,
                                                 "<span weight=\"bold\" size=\"larger\">%s</span>", text);
    if (secondary_text)
        gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                                  "%s", secondary_text);
#elif GTK_CHECK_VERSION(2,4,0)
    dialog = gtk_message_dialog_new_with_markup (parent,
                                                 GTK_DIALOG_MODAL,
                                                 type,
                                                 GTK_BUTTONS_NONE,
                                                 "<span weight=\"bold\" size=\"larger\">%s</span>\n%s",
                                                 text, secondary_text ? secondary_text : "");
#else /* !GTK_CHECK_VERSION(2,4,0) */
    dialog = gtk_message_dialog_new (parent,
                                     GTK_DIALOG_MODAL,
                                     type,
                                     GTK_BUTTONS_NONE,
                                     "%s\n%s",
                                     text, secondary_text ? secondary_text : "");
#endif /* !GTK_CHECK_VERSION(2,4,0) */

    gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                            GTK_STOCK_CLOSE, GTK_RESPONSE_CANCEL,
                            NULL);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog),
                                     GTK_RESPONSE_CANCEL);

    if (parent && parent->group)
        gtk_window_group_add_window (parent->group, GTK_WINDOW (dialog));

    return dialog;
}


/* gtkwindow.c */
static void
clamp_window_to_rectangle (gint               *x,
                           gint               *y,
                           gint                w,
                           gint                h,
                           const GdkRectangle *rect)
{
    /* if larger than the screen, center on the screen. */
    if (w > rect->width)
        *x = rect->x - (w - rect->width) / 2;
    else if (*x < rect->x)
        *x = rect->x;
    else if (*x + w > rect->x + rect->width)
        *x = rect->x + rect->width - w;

    if (h > rect->height)
        *y = rect->y - (h - rect->height) / 2;
    else if (*y < rect->y)
        *y = rect->y;
    else if (*y + h > rect->y + rect->height)
        *y = rect->y + rect->height - h;
}


static void
position_window (GtkWindow *dialog)
{
    GdkPoint *coord;
    int screen_width, screen_height, monitor_num;
    GdkRectangle monitor;
    GdkScreen *screen;
    GtkRequisition req;

    g_signal_handlers_disconnect_by_func (dialog,
                                          (gpointer) position_window,
                                          NULL);

    coord = g_object_get_data (G_OBJECT (dialog), "moo-coords");
    g_return_if_fail (coord != NULL);

    screen = gtk_widget_get_screen (GTK_WIDGET (dialog));
    g_return_if_fail (screen != NULL);

    screen_width = gdk_screen_get_width (screen);
    screen_height = gdk_screen_get_height (screen);
    monitor_num = gdk_screen_get_monitor_at_point (screen, coord->x, coord->y);

    gtk_widget_size_request (GTK_WIDGET (dialog), &req);

    coord->x = coord->x - req.width / 2;
    coord->y = coord->y - req.height / 2;
    coord->x = CLAMP (coord->x, 0, screen_width - req.width);
    coord->y = CLAMP (coord->y, 0, screen_height - req.height);

    gdk_screen_get_monitor_geometry (screen, monitor_num, &monitor);
    clamp_window_to_rectangle (&coord->x, &coord->y, req.width, req.height, &monitor);

    gtk_window_move (dialog, coord->x, coord->y);
}


#ifdef __WIN32__
static void
on_hide (GtkWindow *window)
{
    GtkWindow *parent = gtk_window_get_transient_for (window);

    if (parent && GTK_WIDGET_DRAWABLE (parent))
        gtk_window_present (parent);

    g_signal_handlers_disconnect_by_func (window, (gpointer) on_hide, NULL);
}
#endif


void
moo_position_window (GtkWidget  *window,
                     GtkWidget  *parent,
                     gboolean    at_mouse,
                     gboolean    at_coords,
                     int         x,
                     int         y)
{
    GtkWidget *toplevel = NULL;

    g_return_if_fail (GTK_IS_WINDOW (window));
    g_return_if_fail (!GTK_WIDGET_REALIZED (window));

    if (parent)
        toplevel = gtk_widget_get_toplevel (parent);
    if (toplevel && !GTK_WIDGET_TOPLEVEL (toplevel))
        toplevel = NULL;

    if (toplevel)
    {
        gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (toplevel));
#ifdef __WIN32__
        g_signal_connect (window, "unmap", G_CALLBACK (on_hide), NULL);
#endif
    }

    if (toplevel && GTK_WINDOW(toplevel)->group)
        gtk_window_group_add_window (GTK_WINDOW(toplevel)->group, GTK_WINDOW (window));

    if (!at_mouse && !at_coords && parent && GTK_WIDGET_REALIZED (parent))
    {
        if (GTK_WIDGET_TOPLEVEL (parent))
        {
            gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER_ON_PARENT);
        }
        else
        {
            GdkWindow *parent_window = gtk_widget_get_parent_window (parent);
            gdk_window_get_origin (parent_window, &x, &y);
            x += parent->allocation.x;
            y += parent->allocation.y;
            x += parent->allocation.width / 2;
            y += parent->allocation.height / 2;
            at_coords = TRUE;
        }
    }

    if (at_mouse)
    {
        gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_MOUSE);
    }
    else if (at_coords)
    {
        GdkPoint *coord = g_new (GdkPoint, 1);
        coord->x = x;
        coord->y = y;
        gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_NONE);
        g_object_set_data_full (G_OBJECT (window), "moo-coords", coord, g_free);
        g_signal_connect (window, "realize",
                          G_CALLBACK (position_window),
                          NULL);
    }
}


void
moo_message_dialog (GtkWidget  *parent,
                    GtkMessageType type,
                    const char *text,
                    const char *secondary_text,
                    gboolean    at_mouse,
                    gboolean    at_coords,
                    int         x,
                    int         y)
{
    GtkWidget *dialog, *toplevel = NULL;

    if (parent)
        toplevel = gtk_widget_get_toplevel (parent);
    if (!toplevel || !GTK_WIDGET_TOPLEVEL (toplevel))
        toplevel = NULL;

    dialog = create_message_dialog (toplevel ? GTK_WINDOW (toplevel) : NULL,
                                    type, text, secondary_text);
    g_return_if_fail (dialog != NULL);

    moo_position_window (dialog, parent, at_mouse, at_coords, x, y);

    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}


void
moo_error_dialog (GtkWidget  *parent,
                  const char *text,
                  const char *secondary_text)
{
    moo_message_dialog (parent,
                        GTK_MESSAGE_ERROR,
                        text, secondary_text,
                        FALSE, FALSE, 0, 0);
}

void
moo_info_dialog (GtkWidget  *parent,
                 const char *text,
                 const char *secondary_text)
{
    moo_message_dialog (parent,
                        GTK_MESSAGE_INFO,
                        text, secondary_text,
                        FALSE, FALSE, 0, 0);
}

void
moo_warning_dialog (GtkWidget  *parent,
                    const char *text,
                    const char *secondary_text)
{
    moo_message_dialog (parent,
                        GTK_MESSAGE_WARNING,
                        text, secondary_text,
                        FALSE, FALSE, 0, 0);
}


gboolean
moo_overwrite_file_dialog (GtkWidget  *parent,
                           const char *display_name,
                           const char *display_dirname)
{
    int response;
    GtkWidget *dialog, *button, *toplevel = NULL;

    g_return_val_if_fail (display_name != NULL, FALSE);

    if (parent)
        parent = gtk_widget_get_toplevel (parent);

    dialog = gtk_message_dialog_new (toplevel ? GTK_WINDOW (toplevel) : NULL,
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

    moo_position_window (dialog, parent, FALSE, FALSE, 0, 0);

    response = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    return response == GTK_RESPONSE_YES;
}


const char *
moo_font_dialog (GtkWidget  *parent,
                 const char *title,
                 const char *start_font,
                 gboolean fixed_width)
{
    GtkWidget *dialog;
    const char *fontname = NULL;

    if (fixed_width)
        g_warning ("%s: choosing fixed width fonts "
                   "only is not implemented", G_STRLOC);

    dialog = gtk_font_selection_dialog_new (title);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
    if (start_font)
        gtk_font_selection_dialog_set_font_name (
                GTK_FONT_SELECTION_DIALOG (dialog), start_font);

    moo_position_window (dialog, parent, FALSE, FALSE, 0, 0);

    if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (dialog)))
        fontname = gtk_font_selection_dialog_get_font_name (
            GTK_FONT_SELECTION_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    return fontname;
}
