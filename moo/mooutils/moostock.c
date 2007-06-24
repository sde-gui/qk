/*
 *   moostock.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mooutils/moostock.h"
#include "mooutils/moocompat.h"
#include "mooutils/stock-terminal-24.h"
#include "mooutils/stock-file-selector-24.h"
#include "mooutils/stock-file-24.h"
#include "mooutils/stock-moo.h"
#include "mooutils/mooi18n.h"
#include <gtk/gtk.h>
#include <string.h>

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
    {(char*) MOO_STOCK_FILE_SAVE_COPY, (char*) N_("Save Copy"), 0, 0, (char*) GETTEXT_PACKAGE},
    {(char*) MOO_STOCK_NEW_FOLDER, (char*) N_("_New Folder"), 0, 0, (char*) GETTEXT_PACKAGE},
    {(char*) MOO_STOCK_NEW_WINDOW, (char*) N_("New _Window"), 0, 0, (char*) GETTEXT_PACKAGE}
};

#if !GTK_CHECK_VERSION(2,10,0)
static GtkStockItem stock_items_2_10[] = {
    {(char*) GTK_STOCK_SELECT_ALL, (char*) N_("Select _All"), 0, 0, (char*) GETTEXT_PACKAGE}
};
#endif


#if GTK_CHECK_VERSION(2,4,0)

static void
add_icon_name_if_present (GtkIconSet *set,
                          const char *icon_name)
{
    GtkIconTheme *theme;

    g_return_if_fail (icon_name);

    theme = gtk_icon_theme_get_default ();

    if (gtk_icon_theme_has_icon (theme, icon_name))
    {
        GtkIconSource *source;
        source = gtk_icon_source_new ();
        gtk_icon_source_set_icon_name (source, icon_name);
        gtk_icon_set_add_source (set, source);
        gtk_icon_source_free (source);
    }
#if 0
    else
    {
        g_message ("no icon '%s'", icon_name);
    }
#endif
}

static void
register_stock_icon (GtkIconFactory *factory,
                     const char     *stock_id,
                     const char     *icon_name)
{
    GtkIconSet *set;

    set = gtk_icon_set_new ();

    add_icon_name_if_present (set, stock_id);

    if (icon_name)
        add_icon_name_if_present (set, icon_name);

    gtk_icon_factory_add (factory, stock_id, set);
    gtk_icon_set_unref (set);
}


static void
add_default_image (gint          size,
                   const guchar *inline_data,
                   const char   *name1,
                   const char   *name2)
{
    GdkPixbuf *pixbuf;

    g_return_if_fail (name1 != NULL);
    g_return_if_fail (inline_data != NULL);

    pixbuf = gdk_pixbuf_new_from_inline (-1, inline_data, FALSE, NULL);
    g_return_if_fail (pixbuf != NULL);

    gtk_icon_theme_add_builtin_icon (name1, size, pixbuf);

    if (name2)
        gtk_icon_theme_add_builtin_icon (name2, size, pixbuf);

    g_object_unref (pixbuf);
}


static void
add_icon (GtkIconFactory *factory,
          const char     *stock_id,
          const char     *icon_name,
          gint            size,
          const guchar   *data)
{
    if (data)
        add_default_image (size, data, stock_id, icon_name);
    register_stock_icon (factory, stock_id, icon_name);
}

static void
add_icon_name (GtkIconFactory *factory,
               const char     *stock_id,
               const char     *icon_name)
{
    GtkIconSet *set;

    g_return_if_fail (icon_name != NULL);

    set = gtk_icon_factory_lookup (factory, stock_id);
    g_return_if_fail (set != NULL);

    add_icon_name_if_present (set, icon_name);
}

#if !GTK_CHECK_VERSION(2,10,0)
static void add_icon2   (GtkIconFactory *factory,
                         const char     *stock_id,
                         const char     *icon_name,
                         gint            size1,
                         const guchar   *data1,
                         gint            size2,
                         const guchar   *data2)
{
    add_default_image (size1, data1, stock_id, NULL);
    add_default_image (size2, data2, stock_id, NULL);
    register_stock_icon (factory, stock_id, icon_name);
}
#endif /* !GTK_CHECK_VERSION(2,10,0) */


#else /* !GTK_CHECK_VERSION(2,4,0) */

/* TODO: take code from gtk */


static void
add_icon (GtkIconFactory *factory,
          const char     *stock_id,
          G_GNUC_UNUSED const char *icon_name,
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

static void
add_icon2 (GtkIconFactory *factory,
           const char     *stock_id,
           const char     *icon_name,
           gint            size1,
           const guchar   *data1,
           G_GNUC_UNUSED gint size2,
           G_GNUC_UNUSED const guchar *data2)
{
    add_icon (factory, stock_id, icon_name, size1, data1);
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


static void
register_stock_icon_alias (GtkIconFactory *factory,
                           const char     *stock_id,
                           const char     *new_stock_id,
                           const char     *icon_name)
{
    GtkIconSet *set;

    /* must use gtk_icon_factory_lookup_default() to initialize gtk icons */
    set = gtk_icon_factory_lookup_default (stock_id);
    g_return_if_fail (set != NULL);

    set = gtk_icon_set_copy (set);
    gtk_icon_factory_add (factory, new_stock_id, set);

    if (icon_name)
        add_icon_name (factory, new_stock_id, icon_name);

    gtk_icon_set_unref (set);
}


void
_moo_stock_init (void)
{
    static gboolean created = FALSE;
    GtkIconFactory *factory;
    GtkSettings *settings;
    char *icon_theme_name;

    if (created)
        return;

    created = TRUE;

    factory = gtk_icon_factory_new ();

    gtk_icon_factory_add_default (factory);

    settings = gtk_settings_get_default ();
    g_object_get (settings, "gtk-icon-theme-name", &icon_theme_name, NULL);

    /* XXX */
    if (icon_theme_name && !strcmp (icon_theme_name, "gnome"))
    {
        add_icon (factory, MOO_STOCK_FILE_SELECTOR, "gnome-fs-directory", 24, MOO_FILE_SELECTOR_ICON);
    }
    else
    {
        add_icon (factory, MOO_STOCK_FILE_SELECTOR, NULL, 24, MOO_FILE_SELECTOR_ICON);
        add_icon_name (factory, MOO_STOCK_FILE_SELECTOR, "folder");
        add_icon_name (factory, MOO_STOCK_FILE_SELECTOR, "file-manager");
    }

    add_icon (factory, MOO_STOCK_FILE, "unknown", 24, MOO_FILE_ICON);
    add_icon_name (factory, MOO_STOCK_FILE, "gnome-fs-regular");

    add_icon (factory, MOO_STOCK_FOLDER, "gnome-fs-directory", 24, MOO_FILE_SELECTOR_ICON);
    add_icon_name (factory, MOO_STOCK_FOLDER, "folder");

    add_icon (factory, MOO_STOCK_TERMINAL, "terminal", 24, MOO_GNOME_TERMINAL_ICON);

    add_icon (factory, MOO_STOCK_MEDIT, "medit", 24, MEDIT_ICON);
    add_icon (factory, MOO_STOCK_HIDE, NULL, REAL_SMALL, MOO_HIDE_ICON);
    add_icon (factory, MOO_STOCK_CLOSE, NULL, REAL_SMALL, MOO_CLOSE_ICON);
    add_icon (factory, MOO_STOCK_STICKY, NULL, REAL_SMALL, MOO_STICKY_ICON);
    add_icon (factory, MOO_STOCK_DETACH, NULL, REAL_SMALL, MOO_DETACH_ICON);
    add_icon (factory, MOO_STOCK_ATTACH, NULL, REAL_SMALL, MOO_ATTACH_ICON);
    add_icon (factory, MOO_STOCK_KEEP_ON_TOP, NULL, REAL_SMALL, MOO_KEEP_ON_TOP_ICON);

#if !GTK_CHECK_VERSION(2,6,0)
    add_icon2 (factory, GTK_STOCK_ABOUT, "stock_about",
               24, STOCK_ABOUT_24, 16, STOCK_ABOUT_16);
    add_icon2 (factory, GTK_STOCK_EDIT, "stock_edit",
               24, STOCK_EDIT_24, 16, STOCK_EDIT_16);
#endif

#if !GTK_CHECK_VERSION(2,10,0)
    add_icon2 (factory, GTK_STOCK_SELECT_ALL, "edit-select-all",
               24, STOCK_SELECT_ALL_24, 16, STOCK_SELECT_ALL_16);
    gtk_stock_add_static (stock_items_2_10, G_N_ELEMENTS (stock_items_2_10));
#endif

    gtk_stock_add_static (stock_items, G_N_ELEMENTS (stock_items));

    register_stock_icon_alias (factory, GTK_STOCK_DIRECTORY, MOO_STOCK_NEW_FOLDER, "folder_new");
    add_icon_name (factory, MOO_STOCK_NEW_FOLDER, "folder-new");
    register_stock_icon_alias (factory, GTK_STOCK_NEW, MOO_STOCK_NEW_WINDOW, "window_new");
    add_icon_name (factory, MOO_STOCK_NEW_WINDOW, "window-new");

    register_stock_icon_alias (factory, GTK_STOCK_NO, MOO_STOCK_SAVE_NONE, "no");
    register_stock_icon_alias (factory, GTK_STOCK_SAVE, MOO_STOCK_SAVE_SELECTED, "filesave");

    register_stock_icon_alias (factory, GTK_STOCK_COPY, MOO_STOCK_FILE_COPY, "editcopy");
    register_stock_icon_alias (factory, GTK_STOCK_SAVE, MOO_STOCK_FILE_SAVE_COPY, "filesave");
    register_stock_icon_alias (factory, GTK_STOCK_SAVE_AS, MOO_STOCK_FILE_SAVE_AS, "filesaveas");

    register_stock_icon_alias (factory, GTK_STOCK_NEW, MOO_STOCK_NEW_PROJECT, NULL);
    register_stock_icon_alias (factory, GTK_STOCK_OPEN, MOO_STOCK_OPEN_PROJECT, NULL);
    register_stock_icon_alias (factory, GTK_STOCK_CLOSE, MOO_STOCK_CLOSE_PROJECT, NULL);
    register_stock_icon_alias (factory, GTK_STOCK_PREFERENCES, MOO_STOCK_PROJECT_OPTIONS, NULL);
    register_stock_icon_alias (factory, GTK_STOCK_GOTO_BOTTOM, MOO_STOCK_BUILD, NULL);
    register_stock_icon_alias (factory, GTK_STOCK_GO_DOWN, MOO_STOCK_COMPILE, NULL);
    register_stock_icon_alias (factory, GTK_STOCK_EXECUTE, MOO_STOCK_EXECUTE, NULL);

    register_stock_icon_alias (factory, GTK_STOCK_FIND, MOO_STOCK_FIND_IN_FILES, NULL);
    register_stock_icon_alias (factory, GTK_STOCK_FIND, MOO_STOCK_FIND_FILE, NULL);

    register_stock_icon_alias (factory, GTK_STOCK_ABOUT, MOO_STOCK_EDIT_BOOKMARK, "bookmark");
    register_stock_icon_alias (factory, GTK_STOCK_ABOUT, MOO_STOCK_FILE_BOOKMARK, "gnome-fs-bookmark");
    add_icon_name (factory, MOO_STOCK_FILE_BOOKMARK, "bookmark");

    g_free (icon_theme_name);
    g_object_unref (G_OBJECT (factory));
}
