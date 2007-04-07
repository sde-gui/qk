/*
 *   mooutils/moostock.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
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
#include "mooutils/stock-moo.h"
#include "mooutils/mooi18n.h"
#include <gtk/gtk.h>

#if !GTK_CHECK_VERSION(2,6,0)
#include "mooutils/stock-about-16.h"
#include "mooutils/stock-about-24.h"
#include "mooutils/stock-edit-16.h"
#include "mooutils/stock-edit-24.h"
#endif

#if !GTK_CHECK_VERSION(2,10,0)
#include "mooutils/stock-select-all-16.h"
#include "mooutils/stock-select-all-24.h"
#endif

#define REAL_SMALL 6


static GtkStockItem stock_items[] = {
    {(char*) MOO_STOCK_SAVE_NONE, (char*) N_("Save _None"), 0, 0, (char*) GETTEXT_PACKAGE},
    {(char*) MOO_STOCK_SAVE_SELECTED, (char*) N_("Save _Selected"), 0, 0, (char*) GETTEXT_PACKAGE},
    {(char*) MOO_STOCK_FILE_COPY, (char*) "_Copy", 0, 0, (char*) "gtk20"},
    {(char*) MOO_STOCK_FILE_MOVE, (char*) N_("Move"), 0, 0, (char*) GETTEXT_PACKAGE},
    {(char*) MOO_STOCK_FILE_LINK, (char*) N_("Link"), 0, 0, (char*) GETTEXT_PACKAGE},
    {(char*) MOO_STOCK_FILE_SAVE_AS, (char*) "Save _As", 0, 0, (char*) "gtk20"},
    {(char*) MOO_STOCK_FILE_SAVE_COPY, (char*) N_("Save Copy"), 0, 0, (char*) GETTEXT_PACKAGE}
};

#if !GTK_CHECK_VERSION(2,10,0)
static GtkStockItem stock_items_2_10[] = {
    {(char*) GTK_STOCK_SELECT_ALL, (char*) N_("Select _All"), 0, 0, (char*) GETTEXT_PACKAGE},
};
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

#if !GTK_CHECK_VERSION(2,10,0)
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
#endif /* !GTK_CHECK_VERSION(2,10,0) */


#else /* !GTK_CHECK_VERSION(2,4,0) */

/* TODO: take code from gtk */


static void add_icon    (GtkIconFactory *factory,
                         const gchar    *stock_id,
                         G_GNUC_UNUSED gint size,
                         const guchar   *data)
{
    GtkIconSet *set = NULL;
    GdkPixbuf *pixbuf = NULL;

    pixbuf = gdk_pixbuf_new_from_inline (-1, data, FALSE, NULL);

    if (pixbuf)
    {
        set = gtk_icon_set_new_from_pixbuf (pixbuf);
        gdk_pixbuf_unref (pixbuf);
    }
    else
    {
        set = gtk_icon_set_new ();
    }

    gtk_icon_factory_add (factory, stock_id, set);
    gtk_icon_set_unref (set);
}

static void add_icon2   (GtkIconFactory *factory,
                         const gchar    *stock_id,
                         gint            size1,
                         const guchar   *data1,
                         G_GNUC_UNUSED gint size2,
                         G_GNUC_UNUSED const guchar *data2)
{
    add_icon (factory, stock_id, size1, data1);
}


#endif /* !GTK_CHECK_VERSION(2,4,0) */


GtkIconSize
_moo_get_icon_size_real_small (void)
{
    static GtkIconSize size = 0;

    if (!size)
        size = gtk_icon_size_register ("moo-real-small", 4, 4);

    return size;
}


static void register_stock_icon_alias (GtkIconFactory *factory,
                                       const gchar    *stock_id,
                                       const gchar    *new_stock_id)
{
    /* must use gtk_icon_factory_lookup_default() to initialize gtk icons */
    GtkIconSet *set = gtk_icon_factory_lookup_default (stock_id);
    g_return_if_fail (set != NULL);
    gtk_icon_factory_add (factory, new_stock_id, set);
}


void
_moo_stock_init (void)
{
    static gboolean created = FALSE;
    GtkIconFactory *factory;

    if (created)
        return;

    created = TRUE;

    factory = gtk_icon_factory_new ();

    gtk_icon_factory_add_default (factory);

    add_icon (factory, MOO_STOCK_TERMINAL,
              24, MOO_GNOME_TERMINAL_ICON);
    add_icon (factory, MOO_STOCK_MEDIT,
              24, MEDIT_ICON);
    add_icon (factory, MOO_STOCK_CLOSE,
              REAL_SMALL, MOO_CLOSE_ICON);
    add_icon (factory, MOO_STOCK_STICKY,
              REAL_SMALL, MOO_STICKY_ICON);
    add_icon (factory, MOO_STOCK_DETACH,
              REAL_SMALL, MOO_DETACH_ICON);
    add_icon (factory, MOO_STOCK_ATTACH,
              REAL_SMALL, MOO_ATTACH_ICON);
    add_icon (factory, MOO_STOCK_KEEP_ON_TOP,
              REAL_SMALL, MOO_KEEP_ON_TOP_ICON);

#if !GTK_CHECK_VERSION(2,6,0)
    add_icon2 (factory, GTK_STOCK_ABOUT,
               24, STOCK_ABOUT_24,
               16, STOCK_ABOUT_16);
    add_icon2 (factory, GTK_STOCK_ABOUT,
               24, STOCK_EDIT_24,
               16, STOCK_EDIT_16);
#endif

#if !GTK_CHECK_VERSION(2,10,0)
    add_icon2 (factory, GTK_STOCK_SELECT_ALL,
               24, STOCK_SELECT_ALL_24,
               16, STOCK_SELECT_ALL_16);
    gtk_stock_add_static (stock_items_2_10, G_N_ELEMENTS (stock_items_2_10));
#endif

    gtk_stock_add_static (stock_items, G_N_ELEMENTS (stock_items));
    register_stock_icon_alias (factory, GTK_STOCK_NO, MOO_STOCK_SAVE_NONE);
    register_stock_icon_alias (factory, GTK_STOCK_SAVE, MOO_STOCK_SAVE_SELECTED);

    register_stock_icon_alias (factory, GTK_STOCK_COPY, MOO_STOCK_FILE_COPY);
    register_stock_icon_alias (factory, GTK_STOCK_SAVE, MOO_STOCK_FILE_SAVE_COPY);
    register_stock_icon_alias (factory, GTK_STOCK_SAVE_AS, MOO_STOCK_FILE_SAVE_AS);

    register_stock_icon_alias (factory, GTK_STOCK_NEW, MOO_STOCK_NEW_PROJECT);
    register_stock_icon_alias (factory, GTK_STOCK_OPEN, MOO_STOCK_OPEN_PROJECT);
    register_stock_icon_alias (factory, GTK_STOCK_CLOSE, MOO_STOCK_CLOSE_PROJECT);
    register_stock_icon_alias (factory, GTK_STOCK_PREFERENCES, MOO_STOCK_PROJECT_OPTIONS);
    register_stock_icon_alias (factory, GTK_STOCK_GOTO_BOTTOM, MOO_STOCK_BUILD);
    register_stock_icon_alias (factory, GTK_STOCK_GO_DOWN, MOO_STOCK_COMPILE);
    register_stock_icon_alias (factory, GTK_STOCK_EXECUTE, MOO_STOCK_EXECUTE);

    register_stock_icon_alias (factory, GTK_STOCK_FIND, MOO_STOCK_FIND_IN_FILES);
    register_stock_icon_alias (factory, GTK_STOCK_FIND, MOO_STOCK_FIND_FILE);

    register_stock_icon_alias (factory, GTK_STOCK_ABOUT, MOO_STOCK_EDIT_BOOKMARK);

    add_default_image ("medit", 48, MEDIT_ICON);

    g_object_unref (G_OBJECT (factory));
}
