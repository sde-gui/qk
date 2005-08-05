/*
 *   mooutils/moofileview-icons.c
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

#include "mooutils/moofileview.h"
#include <sys/stat.h>
#include <errno.h>
#include <string.h>


/* Icon type, supplemented by MIME type
 */
typedef enum {
    ICON_NOENT,
    ICON_NONE,      /* "Could not compute the icon type" */
    ICON_REGULAR,	  /* Use mime type for icon */
    ICON_BLOCK_DEVICE,
    ICON_BROKEN_SYMBOLIC_LINK,
    ICON_CHARACTER_DEVICE,
    ICON_DIRECTORY,
    ICON_EXECUTABLE,
    ICON_FIFO,
    ICON_SOCKET
} IconType;


typedef struct {
    gint       size;
    GdkPixbuf *pixbuf;
} IconCacheElement;

static void icon_cache_element_free (IconCacheElement *element)
{
    if (element->pixbuf)
        g_object_unref (element->pixbuf);
    g_free (element);
}

static void icon_theme_changed (GtkIconTheme *icon_theme)
{
    GHashTable *cache;

    /* Difference from the initial creation is that we don't
        * reconnect the signal */
    cache = g_hash_table_new_full (g_str_hash, g_str_equal,
                                   (GDestroyNotify)g_free,
                                   (GDestroyNotify)icon_cache_element_free);
    g_object_set_data_full (G_OBJECT (icon_theme), "gtk-file-icon-cache",
                            cache, (GDestroyNotify)g_hash_table_destroy);
}


static IconType get_icon_type_from_stat (const struct stat *statp)
{
    if (S_ISBLK (statp->st_mode))
        return ICON_BLOCK_DEVICE;
    else if (S_ISLNK (statp->st_mode))
        return ICON_BROKEN_SYMBOLIC_LINK; /* See get_icon_type */
    else if (S_ISCHR (statp->st_mode))
        return ICON_CHARACTER_DEVICE;
    else if (S_ISDIR (statp->st_mode))
        return ICON_DIRECTORY;
#ifdef S_ISFIFO
    else if (S_ISFIFO (statp->st_mode))
        return  ICON_FIFO;
#endif
#ifdef S_ISSOCK
    else if (S_ISSOCK (statp->st_mode))
        return ICON_SOCKET;
#endif
    else
        return ICON_REGULAR;
}


static IconType get_icon_type (MooFileViewFile *file)
{
    const struct stat *statp = moo_file_view_file_get_stat (file);
    if (statp)
        return get_icon_type_from_stat (statp);
    else
        return ICON_NOENT;
}


static GdkPixbuf *get_cached_icon (GtkWidget   *widget,
                                   const gchar *name,
                                   gint         pixel_size)
{
    GtkIconTheme *icon_theme = gtk_icon_theme_get_for_screen (gtk_widget_get_screen (widget));
    GHashTable *cache = g_object_get_data (G_OBJECT (icon_theme), "gtk-file-icon-cache");
    IconCacheElement *element;

    if (!cache)
    {
        cache = g_hash_table_new_full (g_str_hash, g_str_equal,
                                       (GDestroyNotify)g_free,
                                       (GDestroyNotify)icon_cache_element_free);

        g_object_set_data_full (G_OBJECT (icon_theme), "gtk-file-icon-cache",
                                cache, (GDestroyNotify)g_hash_table_destroy);
        g_signal_connect (icon_theme, "changed",
                          G_CALLBACK (icon_theme_changed), NULL);
    }

    element = g_hash_table_lookup (cache, name);
    if (!element)
    {
        element = g_new0 (IconCacheElement, 1);
        g_hash_table_insert (cache, g_strdup (name), element);
    }

    if (element->size != pixel_size)
    {
        if (element->pixbuf)
            g_object_unref (element->pixbuf);
        element->size = pixel_size;
        element->pixbuf = gtk_icon_theme_load_icon (icon_theme, name,
                pixel_size, 0, NULL);
    }

    return element->pixbuf;
}


static GdkPixbuf *get_icon_for_mime_type (GtkWidget  *widget,
                                          const char *mime_type,
                                          gint        pixel_size)
{
    const char *separator;
    GString *icon_name;
    GdkPixbuf *pixbuf;

    separator = strchr (mime_type, '/');
    if (!separator)
        return NULL; /* maybe we should return a GError with "invalid MIME-type" */

    icon_name = g_string_new ("gnome-mime-");
    g_string_append_len (icon_name, mime_type, separator - mime_type);
    g_string_append_c (icon_name, '-');
    g_string_append (icon_name, separator + 1);
    pixbuf = get_cached_icon (widget, icon_name->str, pixel_size);
    g_string_free (icon_name, TRUE);
    if (pixbuf)
        return pixbuf;

    icon_name = g_string_new ("gnome-mime-");
    g_string_append_len (icon_name, mime_type, separator - mime_type);
    pixbuf = get_cached_icon (widget, icon_name->str, pixel_size);
    g_string_free (icon_name, TRUE);

    return pixbuf;
}


/* Returns the name of the icon to be used for a path which is known to be a
 * directory.  This can vary for Home, Desktop, etc.
 */
static const char *get_icon_name_for_directory (const char *path)
{
    static char *desktop_path = NULL;

    if (!g_get_home_dir ())
        return "gnome-fs-directory";

    if (!desktop_path)
        desktop_path = g_build_filename (g_get_home_dir (), "Desktop", NULL);

    if (strcmp (g_get_home_dir (), path) == 0)
        return "gnome-fs-home";
    else if (strcmp (desktop_path, path) == 0)
        return "gnome-fs-desktop";
    else
        return "gnome-fs-directory";
}


/* Renders an icon for a non-ICON_REGULAR file */
static GdkPixbuf *get_special_icon (IconType         icon_type,
                                    MooFileViewFile *file,
                                    GtkWidget       *widget,
                                    gint             pixel_size)
{
    const char *name;

    g_assert (icon_type != ICON_REGULAR);

    switch (icon_type)
    {
        case ICON_BLOCK_DEVICE:
            name = "gnome-fs-blockdev";
            break;
        case ICON_BROKEN_SYMBOLIC_LINK:
            name = "gnome-fs-symlink";
            break;
        case ICON_CHARACTER_DEVICE:
            name = "gnome-fs-chardev";
            break;
        case ICON_DIRECTORY:
            name = get_icon_name_for_directory
                    (moo_file_view_file_path (file));
            break;
        case ICON_EXECUTABLE:
            name ="gnome-fs-executable";
            break;
        case ICON_FIFO:
            name = "gnome-fs-fifo";
            break;
        case ICON_SOCKET:
            name = "gnome-fs-socket";
            break;
        default:
            g_assert_not_reached ();
            return NULL;
    }

    return get_cached_icon (widget, name, pixel_size);
}


/* Renders a fallback icon from the stock system */
static GdkPixbuf *get_fallback_icon (GtkWidget     *widget,
                                     IconType       icon_type,
                                     GtkIconSize    size)
{
    const char *stock_name;
    GdkPixbuf *pixbuf;

    switch (icon_type)
    {
        case ICON_BLOCK_DEVICE:
            stock_name = GTK_STOCK_HARDDISK;
            break;

        case ICON_DIRECTORY:
            stock_name = GTK_STOCK_DIRECTORY;
            break;

        case ICON_EXECUTABLE:
            stock_name = GTK_STOCK_EXECUTE;
            break;

        case ICON_NOENT:
            stock_name = GTK_STOCK_MISSING_IMAGE;
            break;

        default:
            stock_name = GTK_STOCK_FILE;
            break;
    }

    pixbuf = gtk_widget_render_icon (widget, stock_name, size, NULL);

    if (!pixbuf)
    {
        g_warning ("%s: could not get a stock icon for %s",
                   G_STRLOC, stock_name);
    }

    return pixbuf;
}


GdkPixbuf  *moo_get_icon_for_file           (GtkWidget         *widget,
                                             MooFileViewFile   *file,
                                             GtkIconSize        size)
{
    IconType icon_type;
    GdkPixbuf *pixbuf;
    int pixel_size;

    icon_type = get_icon_type (file);

    if (icon_type == ICON_REGULAR && !moo_file_view_file_mime_type (file))
        icon_type = ICON_NONE;

    if (!gtk_icon_size_lookup (size, &pixel_size, NULL))
        if (!gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &pixel_size, NULL))
            pixel_size = 16;

    switch (icon_type)
    {
        case ICON_NONE:
            goto fallback;

        case ICON_NOENT:
            pixbuf = get_fallback_icon (widget, icon_type, size);
            break;

        case ICON_REGULAR:
            pixbuf = get_icon_for_mime_type (widget,
                                             moo_file_view_file_mime_type (file),
                                             pixel_size);
            break;

        default:
            pixbuf = get_special_icon (icon_type, file, widget, pixel_size);
    }

    if (pixbuf)
        goto out;

fallback:
    pixbuf = get_cached_icon (widget, "gnome-fs-regular", pixel_size);
    if (!pixbuf)
        pixbuf = get_fallback_icon (widget, icon_type, size);

out:
    return pixbuf;
}
