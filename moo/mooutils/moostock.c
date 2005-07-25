/*
 *   mooutils/moostock.c
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

#include "mooutils/moostock.h"
#include "mooutils/moocompat.h"
#include "mooutils/stock-terminal-24.h"
#include "mooutils/stock-app.h"
#include <gtk/gtk.h>

#if !GTK_CHECK_VERSION(2,6,0)
#include "mooutils/stock-about-16.h"
#include "mooutils/stock-about-24.h"
#include "mooutils/stock-edit-16.h"
#include "mooutils/stock-edit-24.h"
#endif


#if GTK_CHECK_VERSION(2,4,0)

static void register_stock_icon     (GtkIconFactory *factory,
                                     const gchar    *stock_id)
{
    GtkIconSet *set = gtk_icon_set_new ();
    GtkIconSource *source = gtk_icon_source_new ();

    gtk_icon_source_set_icon_name (source, stock_id);
    gtk_icon_set_add_source (set, source);

    gtk_icon_factory_add (factory, stock_id, set);
    gtk_icon_set_unref (set);
    gtk_icon_source_free (source);
}


static void add_default_image (const gchar  *stock_id,
                               gint          size,
                               const guchar *inline_data)
{
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_inline (-1, inline_data, FALSE, NULL);
    g_return_if_fail (pixbuf != NULL);

    gtk_icon_theme_add_builtin_icon (stock_id, size, pixbuf);

    g_object_unref (pixbuf);
}


static void add_icon    (GtkIconFactory *factory,
                         const gchar    *stock_id,
                         gint            size,
                         const guchar   *data)
{
    add_default_image (stock_id, size, data);
    register_stock_icon (factory, stock_id);
}

#if !GTK_CHECK_VERSION(2,6,0)
static void add_icon2   (GtkIconFactory *factory,
                         const gchar    *stock_id,
                         gint            size1,
                         const guchar   *data1,
                         gint            size2,
                         const guchar   *data2)
{
    add_default_image (stock_id, size1, data1);
    add_default_image (stock_id, size2, data2);
    register_stock_icon (factory, stock_id);
}
#endif /* !GTK_CHECK_VERSION(2,6,0) */


void moo_create_stock_items (void)
{
    static gboolean created = FALSE;
    GtkIconFactory *factory;

    if (created) return;
    else created = TRUE;

    factory = gtk_icon_factory_new ();

    gtk_icon_factory_add_default (factory);

    add_icon (factory, MOO_STOCK_TERMINAL,
              24, MOO_GNOME_TERMINAL_ICON);
    add_icon (factory, MOO_STOCK_APP,
              24, GGAP_ICON);
    add_icon (factory, MOO_STOCK_GAP,
              24, GAP_ICON);

#if !GTK_CHECK_VERSION(2,6,0)
    add_icon2 (factory, GTK_STOCK_ABOUT,
               24, STOCK_ABOUT_24,
               16, STOCK_ABOUT_16);
    add_icon2 (factory, GTK_STOCK_ABOUT,
               24, STOCK_EDIT_24,
               16, STOCK_EDIT_16);
#endif

    g_object_unref (G_OBJECT (factory));
}


#else /* !GTK_CHECK_VERSION(2,4,0) */

/* TODO: take code from gtk */

void moo_create_stock_items (void)
{
    static gboolean created = FALSE;
    GtkIconSet *set = NULL;
    GdkPixbuf *pixbuf = NULL;
    GtkIconFactory *factory;

    if (created) return;
    else created = TRUE;

    factory = gtk_icon_factory_new ();
    gtk_icon_factory_add_default (factory);

    pixbuf = gdk_pixbuf_new_from_inline (-1, MOO_GNOME_TERMINAL_ICON, FALSE, NULL);
    if (pixbuf) {
        set = gtk_icon_set_new_from_pixbuf (pixbuf);
        gdk_pixbuf_unref (pixbuf);
    }
    else
        set = gtk_icon_set_new ();
    gtk_icon_factory_add (factory, MOO_STOCK_TERMINAL, set);
    gtk_icon_set_unref (set);

    pixbuf = gdk_pixbuf_new_from_inline (-1, STOCK_ABOUT_24, FALSE, NULL);
    if (pixbuf) {
        set = gtk_icon_set_new_from_pixbuf (pixbuf);
        gdk_pixbuf_unref (pixbuf);
    }
    else
        set = gtk_icon_set_new ();
    gtk_icon_factory_add (factory, GTK_STOCK_ABOUT, set);
    gtk_icon_set_unref (set);

    pixbuf = gdk_pixbuf_new_from_inline (-1, STOCK_EDIT_24, FALSE, NULL);
    if (pixbuf) {
        set = gtk_icon_set_new_from_pixbuf (pixbuf);
        gdk_pixbuf_unref (pixbuf);
    }
    else
        set = gtk_icon_set_new ();
    gtk_icon_factory_add (factory, GTK_STOCK_EDIT, set);
    gtk_icon_set_unref (set);

    g_object_unref (G_OBJECT (factory));
}

#endif /* !GTK_CHECK_VERSION(2,4,0) */
