/*
 *   moocellrenderercolor.h
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

#ifndef __MOO_CELL_RENDERER_COLOR_H__
#define __MOO_CELL_RENDERER_COLOR_H__

#include <gtk/gtkcellrenderer.h>

G_BEGIN_DECLS


#define MOO_TYPE_CELL_RENDERER_COLOR            (moo_cell_renderer_color_get_type ())
#define MOO_CELL_RENDERER_COLOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_CELL_RENDERER_COLOR, MooCellRendererColor))
#define MOO_CELL_RENDERER_COLOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_CELL_RENDERER_COLOR, MooCellRendererColorClass))
#define MOO_IS_CELL_RENDERER_COLOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_CELL_RENDERER_COLOR))
#define MOO_IS_CELL_RENDERER_COLOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_CELL_RENDERER_COLOR))
#define MOO_CELL_RENDERER_COLOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_CELL_RENDERER_COLOR, MooCellRendererColorClass))

typedef struct _MooCellRendererColor MooCellRendererColor;
typedef struct _MooCellRendererColorClass MooCellRendererColorClass;

struct _MooCellRendererColor
{
    GtkCellRenderer parent;

    GdkColor color;
    guint color_set : 1;
    guint activatable : 1;
};

struct _MooCellRendererColorClass
{
    GtkCellRendererClass parent_class;

    void (*color_set) (MooCellRendererColor *cell,
                       GdkColor             *color);
};

GType            moo_cell_renderer_color_get_type (void) G_GNUC_CONST;
GtkCellRenderer *moo_cell_renderer_color_new      (void);


G_END_DECLS

#endif /* __MOO_CELL_RENDERER_COLOR_H__ */
