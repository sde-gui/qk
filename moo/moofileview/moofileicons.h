/*
 *   moofileicons.h
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

#ifndef MOO_FILE_ICONS_H
#define MOO_FILE_ICONS_H

#include <gtk/gtkwidget.h>

G_BEGIN_DECLS


typedef enum {
    MOO_ICON_EMBLEM_LINK = 1 << 0
} MooIconEmblem;

typedef enum {
    MOO_ICON_MIME = 0,
    MOO_ICON_HOME,
    MOO_ICON_DESKTOP,
    MOO_ICON_TRASH,
    MOO_ICON_DIRECTORY,
    MOO_ICON_BROKEN_LINK,
    MOO_ICON_NONEXISTENT,
    MOO_ICON_BLOCK_DEVICE,
    MOO_ICON_CHARACTER_DEVICE,
    MOO_ICON_FIFO,
    MOO_ICON_SOCKET,
    MOO_ICON_FILE,
    MOO_ICON_BLANK,
    MOO_ICON_INVALID
} MooIconType;


GdkPixbuf   *_moo_get_icon  (GtkWidget      *widget,
                             MooIconType     type,
                             const char     *mime_type,
                             MooIconEmblem   emblem,
                             GtkIconSize     size);


G_END_DECLS

#endif /* MOO_FILE_ICONS_H */
